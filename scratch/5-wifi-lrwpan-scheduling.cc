/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include <ns3/constant-position-mobility-model.h>
#include "ns3/netanim-module.h"


#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>

//lr-pan
#include <ns3/lr-wpan-module.h>

//wifi
#include <ns3/spectrum-wifi-helper.h>
#include "ns3/ssid.h"

//internet
#include "ns3/internet-module.h"

#include "ns3/applications-module.h"

#include "ns3/mobility-module.h"

#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/ap-wifi-mac.h"

#include <memory>
using namespace ns3;




/****************************************************
 * Hwn header start
 * 
 */
namespace ns3
{

class Hwn: public ns3::Object
{
public:
  
  typedef struct  ScheduleItem_s: public SimpleRefCount<struct  ScheduleItem_s>
  {
      uint32_t id;
      Time lrwpanPeriod;
      Time wifiPeriod;
      Time enqueueTime;
      Time ctsInjectTime;
      Time ctsStartSendingTime;
      Time ctsSentTime;
      Time ctsRealDuration;
      Time anSentTime;
      Time wifiSlotEndTime;
  }ScheduleItem;
 typedef enum
  { 
    HWN_SUCCESS,               //!< Operation success
    HWN_TIMEOUT,
    HWN_LR_WPAN_FAILURE,
    HWN_CSMA_COMPENSATION_FAIL
  }HwnStatus;
  struct ScheduleConfirmParameters: public SimpleRefCount<ScheduleConfirmParameters>
  {
    HwnStatus status;
  };

  typedef enum
  { 
    HWN_CS,                    //!< HWN_CP Heterogeneous Wireless Network Coexist state
    HWN_MS,                    //!< MAC_MS management state. CTS and AN is sending
    HWN_WS,                    //!< MAC_WS wifi state 
    HWN_LS                     //!< HWN_LS Lr-Wpan state

  }HwnState;
  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  Hwn ();
  virtual ~Hwn ();
  void SetLrWpanMac(Ptr<LrWpanMac>);
  void SetApWiFiMac(Ptr<ApWifiMac>);
  /**
   * schedule time slots for lrwpanPeriod and wifiPeriod.
   * 
   * the schedule is put into a queue, if the queue is empty, the schedule will be excehdule inmediately.
   * Otherwise, execute orderly.
   * \param lrwpanPeriod desired Lr-wpan period, the real scheduled period may be shorter, due to Lr-Wpan CSMA random delay.
   * \param wifiPeriod desired WiFi period, the reall wifi period may be longer or shorter, due to WiFi CSMA random delay and delay compensation
   * \handleId handle id of the schedule, can be used to identify the schedule. provided by application
   */
  void Schedule(Time lrwpanPeriod, Time wifiPeriod, uint32_t id);

  /**
   * Get maximum lr-wpan period or wifi period supported
   */
  Time GetMaxLrwpanPeriod(void);
  Time GetMaxWifiPeriod(void);

protected:
  // Inherited from Object.
  virtual void DoInitialize (void);
  virtual void DoDispose (void);
private:
  /**
   * Modified LR-WPAN mac used to schedule AN 
   */
  Ptr<LrWpanMac> m_lrWpanMac;
  /**
   * Modified WiFi AP mac used to schedule CTS
   */
  Ptr<ApWifiMac> m_apWiFiMac;
  /**
   * queue for schedule
   */
  std::deque<Ptr<ScheduleItem>> m_scheduleQueue;
  EventId m_scheduleEventId;
  /**
   * The item is currently scheduling
   */
  Ptr<ScheduleItem> m_currentScheduleItem;
  /**
   * CTS maximum waiting time
   */
  Time m_ctsWaitMax;
  const Time m_ctsDurationMax;
  Time m_maxLrwpanPeriod;
  /**
   * lrWpan traffic will keep being suppressed even if the WiFi duartion has ended.
   * due to CSMA sending is random, excess suppressed period can help following scheduling is guaranteed. 
   */
  const Time m_LrwpanExcessSP;
  /**
   * The current Hwn state.
   */
  TracedValue<HwnState> m_hwnState;
  /**
   * used by check queue, to see if the next item can be schedule. 
   */
  bool m_canScheduleNext;

  /**
   * shchedule confirm, if the schedule sucessed or fail
   */
  Callback<void, Ptr<ScheduleConfirmParameters>> m_scheduleConfirmCb;
  
  /**
   * check the m_scheduleQueue to if schedule is necessary
   */
  void CheckScheduleQueue(void);
  void Enqueue(Ptr<ScheduleItem> scheduleItem);
  Ptr<ScheduleItem> Dequeue(void);
  /**
   * call back from wifi Maclow when Cts is successfully injected and sent
   * 
   */
  void WiFiMaclowCtsInjectSentCallback(Time duration);
  /**
   * call back from wifi Maclow when Cts start sending
   * 
   */
  void WiFiMaclowCtsInjectStartSendingCallback(Time duration);
  /**
   * call back from lr-wpan to confirm An sending status
   */
  void LrwpanMcpsAnConfirm(struct McpsAnConfirmParams);
  void CtsWaitTimeout(void);
  void WiFiSlotTimeUp(void); //invoked when scheduled wifi slot finishes
  void ChangeState(HwnState state);
  /**
   * Estimate CTS period according to desired lr-wpan period
   */
  Time EstimateCtsPeriodByLrwpanPeriod(Time lrwpanPeriod);

};





};



/**
 * Hwn header end
 *********************************************************/






NS_LOG_COMPONENT_DEFINE ("LrWpanTestByXiao");

namespace xiao
{
class Helper{
  std::shared_ptr<ns3::AnimationInterface> anim = 0;
  std::shared_ptr<ns3::GtkConfigStore> config= 0;
  public:

  void
  ConfigStorShow(void)
  {
    //ns3::GtkConfigStore config;
    //config.ConfigureDefaults ();
    //config.ConfigureAttributes ();
    config = std::make_shared<ns3::GtkConfigStore>();
    config->ConfigureDefaults ();
    config->ConfigureAttributes ();
  }

  void makeAnim()
  {
    std::string animFile = "xiao-animation.xml" ;  // Name of file for animation
    // Create the animation object and configure for specified output
    //AnimationInterface anim (animFile);
    // anim.EnablePacketMetadata (); // Optional
    // anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
    anim = std::make_shared<ns3::AnimationInterface>(animFile);
    anim->EnablePacketMetadata (); // Optional
    anim->EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
  }

  std::shared_ptr<ns3::NodeContainer> pSpectrumAnalyzerNodes= 0;
  //std::shared_ptr<ns3::SpectrumAnalyzerHelper> pSpectrumAnalyzerHelper= 0;
  void 
  PlaceSpectrum(const ns3::Ptr<ns3::SpectrumChannel> & channel, const Vector & position, ns3::Time start, ns3::Time stop,ns3::Time resolution)
  {

  pSpectrumAnalyzerNodes = std::make_shared< NodeContainer >();
  NodeContainer & spectrumAnalyzerNodes = *pSpectrumAnalyzerNodes.get();
  //pSpectrumAnalyzerHelper  = std::make_shared< SpectrumAnalyzerHelper >();
  //SpectrumAnalyzerHelper & spectrumAnalyzerHelper = *pSpectrumAnalyzerHelper.get();


  /////////////////////////////////
  // Configure spectrum analyzer
  /////////////////////////////////
  Ptr<ConstantPositionMobilityModel> spectrumAnalyzer_mob = CreateObject<ConstantPositionMobilityModel>();
  spectrumAnalyzer_mob->SetPosition(position);
  //NodeContainer spectrumAnalyzerNodes;
  spectrumAnalyzerNodes.Create (1);
  spectrumAnalyzerNodes.Get(0)->AggregateObject(spectrumAnalyzer_mob);
  SpectrumAnalyzerHelper spectrumAnalyzerHelper;
  spectrumAnalyzerHelper.SetChannel (channel);
  spectrumAnalyzerHelper.SetRxSpectrumModel (SpectrumModelIsm2400MhzRes1Mhz);
  spectrumAnalyzerHelper.SetPhyAttribute ("Resolution", TimeValue (resolution));
  spectrumAnalyzerHelper.SetPhyAttribute ("NoisePowerSpectralDensity", DoubleValue (1e-18));  // -150 dBm/Hz -90dBm/Mhz
  spectrumAnalyzerHelper.SetPhyAttribute ("StartTime", TimeValue (start));  // -150 dBm/Hz -90dBm/Mhz
  spectrumAnalyzerHelper.SetPhyAttribute ("StopTime", TimeValue (stop));  // -150 dBm/Hz -90dBm/Mhz
  spectrumAnalyzerHelper.EnableAsciiAll ("spectrum-analyzer-output");
  NetDeviceContainer spectrumAnalyzerDevices = spectrumAnalyzerHelper.Install (spectrumAnalyzerNodes);
 /*
    you can get a nice plot of the output of SpectrumAnalyzer with this gnuplot script:

    unset surface
    set pm3d at s 
    set palette
    set key off
    set view 50,50
    set xlabel "time (ms)"
    set ylabel "freq (MHz)"
    set zlabel "PSD (dBW/Hz)" offset 15,0,0
    splot "./spectrum-analyzer-output-2-0.tr" using ($1*1000.0):($2/1e6):(10*log10($3))
  */
 /*
    set pm3d map
    set palette
    set key off
    set xlabel "time (ms)"
    set ylabel "freq (MHz)"
    set zlabel "PSD (dBW/Hz)" offset 15,0,0
    splot "./spectrum-analyzer-output-4-0.tr" using ($1*1000.0):($2/1e6):(10*log10($3))


    set xrange [90 to 110] 
    set yrange [2400 to 2420] 
    replot
  ref:http://valavanis-research.blogspot.com/2012/06/plotting-spectral-maps-or-spectrograms.html

  set grid ztics lc rgb "#bbbbbb" lw 1 lt 0

 */
  }


  //LrWpanShowTraceRxDrop
  uint64_t lrWpanShowMacTraceRxDropCounter =0;
  void 
  _LrWpanShowMacTraceRxDropRx(ns3::Ptr<const ns3::Packet> p)
  {
    lrWpanShowMacTraceRxDropCounter++;
    NS_LOG_INFO("Mac Received:" + std::to_string(lrWpanShowMacTraceRxDropCounter));
  }
  void 
  _LrWpanShowMacTraceRxDropRxDrop(ns3::Ptr<const ns3::Packet> p)
  {
    lrWpanShowMacTraceRxDropCounter++;
    NS_LOG_INFO("Mac Droped:" + std::to_string(lrWpanShowMacTraceRxDropCounter));
  }
  void 
  EnableLrWpanShowMacTraceRxDrop(Ptr<NetDevice> netDev)
  {
    netDev->GetObject<LrWpanNetDevice>()->GetMac()->TraceConnectWithoutContext
      ("MacRx",ns3::MakeCallback(&Helper::_LrWpanShowMacTraceRxDropRx, this));
    netDev->GetObject<LrWpanNetDevice>()->GetMac()->TraceConnectWithoutContext
      ("MacRxDrop",ns3::MakeCallback(&Helper::_LrWpanShowMacTraceRxDropRxDrop, this));
  }
  
  //LrWpanShowPhyTraceRxDrop
  uint64_t lrWpanShowPhyTraceRxDropCounterDrop =0;
  uint64_t lrWpanShowPhyTraceRxDropCounterRx =0;

  void 
  _LrWpanShowPhyTraceRxDropRx(ns3::Ptr<const ns3::Packet> p)
  {
    lrWpanShowPhyTraceRxDropCounterRx++;
    NS_LOG_INFO("Phy Received(n/id):" 
              + std::to_string(lrWpanShowPhyTraceRxDropCounterRx)
              + "/" + std::to_string(lrWpanShowPhyTraceRxDropCounterRx+lrWpanShowPhyTraceRxDropCounterDrop));
  }
  void 
  _LrWpanShowPhyTraceRxDropRxDrop(ns3::Ptr<const ns3::Packet> p)
  {
    lrWpanShowPhyTraceRxDropCounterDrop++;
    NS_LOG_INFO("Phy Droped(n/id):" 
              + std::to_string(lrWpanShowPhyTraceRxDropCounterDrop)
              + "/" + std::to_string(lrWpanShowPhyTraceRxDropCounterRx+lrWpanShowPhyTraceRxDropCounterDrop));
  }
  void 
  EnableLrWpanShowPhyTraceRxDrop(Ptr<NetDevice> netDev)
  {
    netDev->GetObject<LrWpanNetDevice>()->GetPhy()->TraceConnectWithoutContext
      ("PhyRx",ns3::MakeCallback(&Helper::_LrWpanShowPhyTraceRxDropRx, this));
    netDev->GetObject<LrWpanNetDevice>()->GetPhy()->TraceConnectWithoutContext
      ("PhyRxDrop",ns3::MakeCallback(&Helper::_LrWpanShowPhyTraceRxDropRxDrop, this));
  }
  



};


//typedef Callback< bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address & > ReceiveCallback;
bool
NetDevCb(Ptr<NetDevice> netDev, Ptr<const Packet> p, uint16_t, const Address & addr)
{
  static uint32_t counter =0;
  counter ++;
  NS_LOG_INFO("NetDevice received a packet! count:"+std::to_string(counter));
  //p->Print (std::cout);
  //std::cout << std::endl;
  return true;
}

void
LrWpanSendSchedule(ns3::Ptr<ns3::Node> &sender,ns3::Ptr<ns3::Node> &receiver, 
                  const ns3::Time &start, const ns3::Time &end, const ns3::Time &interval)
{

  Address addr(receiver->GetDevice(0)->GetAddress());
  ns3::Time i;
  uint32_t j=0;
  for(i= start;  i<end; i += interval){
    Ptr<Packet> p = Create<Packet>(20); //20 byte packet
    Simulator::Schedule(i,&NetDevice::Send,
                      sender->GetDevice(0),
                      p,
                      addr,0);
    j++;
  }
  NS_LOG_INFO(std::to_string(j) + " lr-wpan packets scheduled");
}
void
LrWpanSendScheduleBroadcast(ns3::Ptr<ns3::Node> &sender,
                  const ns3::Time &start, const ns3::Time &end, const ns3::Time &interval)
{
  ns3::Time i;
  uint32_t j=0;
  for(i= start;  i<end; i += interval){
    Ptr<Packet> p = Create<Packet>(20); //20 byte packet
    Simulator::Schedule(i,&NetDevice::Send,
                      sender->GetDevice(0),
                      p,
                      ns3::Mac16Address("ff:ff"),0); //broadcast without mac
    j++;
  }
  NS_LOG_INFO(std::to_string(j) + " lr-wpan packets scheduled");
}


  //random variable
  ns3::Ptr<ns3::UniformRandomVariable> rand =0;
void
LrWpanSendScheduleBroadcastRandom(ns3::Ptr<ns3::Node> &sender,
                  const ns3::Time &start, const ns3::Time &end, const ns3::Time &maxInterval)
{
  ns3::Time i;
  uint32_t j=0;
  if(rand == 0){
    rand = CreateObject<UniformRandomVariable> ();
    rand->SetAttribute ("Min", DoubleValue (0.0));
    rand->SetAttribute ("Max", DoubleValue (1.0));
  }
  ns3::Time interval = maxInterval * (rand->GetValue() * 1e6)/1e6;
  for(i= start;  i<end; i += interval){
    Ptr<Packet> p = Create<Packet>(20); //20 byte packet
    Simulator::Schedule(i,&NetDevice::Send,
                      sender->GetDevice(0),
                      p,
                      ns3::Mac16Address("ff:ff"),0); //broadcast without mac
    j++;
    interval = maxInterval * (rand->GetValue() * 1e6)/1e6;
  }
  NS_LOG_INFO(std::to_string(j) + " lr-wpan packets scheduled");
}

void HwnStaticScheduleStart(Ptr<Hwn> hwn, Time end ){
  ns3::Time i;
  for(i= Now();  i<end; i += MilliSeconds(64))
  {
    hwn->Schedule(MilliSeconds(30),MilliSeconds(30),1);
    //hwn->Schedule(hwn->GetMaxLrwpanPeriod(),hwn->GetMaxWifiPeriod(),1);
  }
}
void HwnStaticSchedule(Ptr<Hwn> hwn, const Time &startTime, Time end )
{
  Simulator::Schedule(startTime, &HwnStaticScheduleStart,hwn,end);
}


};








void
ServerConnectionEstablished (Ptr<const ThreeGppHttpServer>, Ptr<Socket>)
{
  NS_LOG_INFO ("Client has established a connection to the server.");
}

void
MainObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated a main object of " << size << " bytes.");
}

void
EmbeddedObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated an embedded object of " << size << " bytes.");
}

void
ServerTx (Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Server sent a packet of " << packet->GetSize () << " bytes.");
}

void
ClientRx (Ptr<const Packet> packet, const Address &address)
{
  NS_LOG_INFO ("Client received a packet of " << packet->GetSize () << " bytes from " << address);
}

void
ClientMainObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::MAIN_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received a main object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse a main object. ");
    }
}

void
ClientEmbeddedObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::EMBEDDED_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received an embedded object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse an embedded object. ");
    }
}

void InjectCtsCallback(ns3::Time duration)
{
  NS_LOG_INFO(duration);
}










int 
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("My first hello word!");
  xiao::Helper xiao_helper;
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("LrWpanTestByXiao", LOG_INFO);
  
  Packet::EnablePrinting ();
  Packet::EnableChecking();

  /////////////////////////////////
  // Configure LR-WPAN
  /////////////////////////////////
  ns3::NodeContainer lrPandNodes;
  lrPandNodes.Create(2);

  LrWpanHelper lrWpanHelper(true);
  /*if (verbose)
  {
      lrWpanHelper.EnableLogComponents ();
  }*/
  
  //install Mobility to lrPandDnodes;
  Ptr<Node> sender = lrPandNodes.Get(0);
  Ptr<Node> recver = lrPandNodes.Get(1);
  Ptr<ConstantPositionMobilityModel> sender_mob = CreateObject<ConstantPositionMobilityModel>();
  Ptr<ConstantPositionMobilityModel> recver_mob = CreateObject<ConstantPositionMobilityModel>();
  sender->AggregateObject(sender_mob);
  recver->AggregateObject(recver_mob);
  sender_mob->SetPosition(Vector(0,0,0));
  recver_mob->SetPosition(Vector(10,0,0));

  //Create and install LrWpanNetDevices into nodes in nodes container by using LrWpanHelper
  ns3::NetDeviceContainer lrDevices = lrWpanHelper.Install(lrPandNodes);

  LrWpanPhyPibAttributes lrWpanPibAttribute;
  lrWpanPibAttribute.phyCurrentChannel = 17;
  lrDevices.Get(0)->GetObject<LrWpanNetDevice>()->GetPhy()->PlmeSetAttributeRequest(ns3::LrWpanPibAttributeIdentifier::phyCurrentChannel,&lrWpanPibAttribute);
  lrDevices.Get(1)->GetObject<LrWpanNetDevice>()->GetPhy()->PlmeSetAttributeRequest(ns3::LrWpanPibAttributeIdentifier::phyCurrentChannel,&lrWpanPibAttribute);

  //setup MAC addres
  lrDevices.Get(0)->SetAddress(ns3::Mac16Address("00:00"));
  lrDevices.Get(1)->SetAddress(ns3::Mac16Address("00:01"));
  //lrWpanHelper.AssociateToPan()

  //set Netdevice receiver
  recver->GetDevice(0)->SetReceiveCallback(MakeCallback(&xiao::NetDevCb));

  //stupid way scheduling packet seding
  xiao::LrWpanSendScheduleBroadcast(sender, MilliSeconds(3102),MilliSeconds(4100),MilliSeconds(6));
  //xiao::LrWpanSendScheduleBroadcastRandom(sender, MilliSeconds(150),MilliSeconds(300),MilliSeconds(10));
  //xiao::LrWpanSendSchedule(sender,recver, MilliSeconds(150),MilliSeconds(300),MilliSeconds(10));
 
  //std::cout << recver->GetDevice(0)->GetAddress() << " -- " << sender->GetDevice(0)->GetAddress() << std::endl;
  lrWpanHelper.EnablePcapAll("lrpwan_test",true);
  //xiao_helper.EnableLrWpanShowMacTraceRxDrop(recver->GetDevice(0));
  xiao_helper.EnableLrWpanShowPhyTraceRxDrop(recver->GetDevice(0));



  /////////////////////////////////
  // Configure WiFi -- prepare
  /////////////////////////////////

  ns3::Ptr<ns3::SpectrumChannel> channel = lrWpanHelper.GetChannel();


  /////////////////////////////////
  // Configure WiFi
  /////////////////////////////////
  uint32_t wifiStaNodesN = 10;
  NodeContainer wifiNodes;
  NodeContainer wifiApNode;
  wifiApNode.Create(1);
  wifiNodes.Add(wifiApNode);
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(wifiStaNodesN);
  wifiNodes.Add(wifiStaNodes);

  //set nodes location
  ns3::Ptr<ListPositionAllocator> wifiLocationAllocator = ns3::CreateObject<ListPositionAllocator>();
  wifiLocationAllocator->Add(ns3::Vector(10,2,0));     //wifi AP
  
  wifiLocationAllocator->Add(ns3::Vector(0,2,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(7,8,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(19,3,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(6,5,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(5,8,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(15,6,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(6,4,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(6,7,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(14,11,0));    //wifi Station
  wifiLocationAllocator->Add(ns3::Vector(7,10,0));    //wifi Station
  MobilityHelper mobility;
  mobility.SetPositionAllocator(wifiLocationAllocator);
  mobility.Install(wifiNodes);


  ns3::SpectrumWifiPhyHelper wifiPhy = ns3::SpectrumWifiPhyHelper::Default();
  wifiPhy.SetChannel(channel);
  wifiPhy.Set("ChannelNumber",UintegerValue(6));

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");
  //wifi.SetRemoteStationManager("ns3::AmrrWifiManager");
  //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","DataMode", StringValue ("OfdmRate27MbpsBW10MHz"))
  
  //config station nodes
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices = wifi.Install (wifiPhy, mac, wifiStaNodes);
  //config ap node
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevices = wifi.Install (wifiPhy, mac, wifiApNode);

  

  //intall internet statck
  InternetStackHelper internetStackHelper;
  internetStackHelper .Install(wifiNodes);

  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase ("192.168.1.0", "255.255.255.0");
  ns3::Ipv4InterfaceContainer wifiStaIpv4Interfaces = addressHelper.Assign (staDevices);
  ns3::Ipv4InterfaceContainer wifiApIpv4Interfaces =  addressHelper.Assign (apDevices);
  





  Ipv4Address serverAddress = wifiApIpv4Interfaces.GetAddress (0);
  // Create HTTP server helper
  ThreeGppHttpServerHelper serverHelper (serverAddress);

  // Install HTTP server
  ApplicationContainer serverApps = serverHelper.Install (wifiApNode);
  Ptr<ThreeGppHttpServer> httpServer = serverApps.Get (0)->GetObject<ThreeGppHttpServer> ();

  // // Example of connecting to the trace sources
  // httpServer->TraceConnectWithoutContext ("ConnectionEstablished",
  //                                         MakeCallback (&ServerConnectionEstablished));
  // httpServer->TraceConnectWithoutContext ("MainObject", MakeCallback (&MainObjectGenerated));
  // httpServer->TraceConnectWithoutContext ("EmbeddedObject", MakeCallback (&EmbeddedObjectGenerated));
  // httpServer->TraceConnectWithoutContext ("Tx", MakeCallback (&ServerTx));

  // Setup HTTP variables for the server
  PointerValue varPtr;
  httpServer->GetAttribute ("Variables", varPtr);
  Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables> ();
  httpVariables->SetMainObjectSizeMean (102400); // 100kB
  httpVariables->SetMainObjectSizeStdDev (40960); // 40kB


  // Create HTTP client helper
  ThreeGppHttpClientHelper clientHelper (serverAddress);

  // Install HTTP client
  ApplicationContainer clientApps = clientHelper.Install (wifiStaNodes);
  for(uint32_t i=0;i<wifiStaNodesN;i++){
    Ptr<ThreeGppHttpClient> httpClient = clientApps.Get (i)->GetObject<ThreeGppHttpClient> ();
    httpClient->GetAttribute("Variables",varPtr) ;
    httpVariables=varPtr.Get<ThreeGppHttpVariables>();
    httpVariables->SetAttribute("ReadingTimeMean",TimeValue(Seconds(0)));
  }

  // // Example of connecting to the trace sources
  // httpClient->TraceConnectWithoutContext ("RxMainObject", MakeCallback (&ClientMainObjectReceived));
  // httpClient->TraceConnectWithoutContext ("RxEmbeddedObject", MakeCallback (&ClientEmbeddedObjectReceived));
  // httpClient->TraceConnectWithoutContext ("Rx", MakeCallback (&ClientRx));

  

  // Stop browsing after 30 minutes
  clientApps.Stop (Seconds (60));


  









/*
  //install application
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer echoServerApp = echoServer.Install(wifiApNode);
  echoServerApp.Start(MilliSeconds(5));
  echoServerApp.Stop (Seconds (3.0));

  //Ptr<ns3::Ipv4> apIpv4 = wifiApNode.Get(0)->GetObject<ns3::Ipv4>();
  //apIpv4->GetInterfaceForDevice(apDevices.Get(0));
  //apIpv4->GetAddress(apIpv4->GetInterfaceForDevice(apDevices.Get(0)),0);
  UdpEchoClientHelper echoClient (wifiApIpv4Interfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient.SetAttribute ("Interval", TimeValue (MicroSeconds (2099)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (500));
  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get(0));
  clientApps.Start (MilliSeconds(10));
  clientApps.Stop (Seconds (0.7));


  //install application1
  UdpEchoServerHelper echoServer1 (9);
  ApplicationContainer echoServerApp1 = echoServer1.Install(wifiStaNodes.Get(0));
  echoServerApp1.Start(MilliSeconds(200));
  echoServerApp1.Stop (Seconds (3.0));

  UdpEchoClientHelper echoClient1 (wifiStaIpv4Interfaces.GetAddress (0), 9);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient1.SetAttribute ("Interval", TimeValue (MicroSeconds (1786)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (500));
  ApplicationContainer clientApps1 = 
    echoClient1.Install (wifiApNode.Get(0));
  clientApps1.Start (MilliSeconds(220));
  clientApps1.Stop (Seconds (0.7));
*/


  /********************************************************
   * configuration Hwn
   */
  ns3::Ptr<ns3::LrWpanMac> lrwpanMac = lrDevices.Get(1)->GetObject<LrWpanNetDevice>()->GetMac();

  //apDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>()->InjectCts(Seconds(0.03));
  Ptr<ApWifiMac> apMac = apDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>();
  apMac->SetCtsInjectSentCallback(MakeCallback (&InjectCtsCallback));

  Ptr<Hwn> hwn = Create<Hwn>();
  hwn->SetLrWpanMac(lrwpanMac);
  hwn->SetApWiFiMac(apMac);
  xiao::HwnStaticSchedule(hwn,Seconds(3),Seconds(7));
  


  Simulator::Stop (MilliSeconds (4500));
  xiao_helper.PlaceSpectrum(channel,Vector(5,1,0),Seconds(3),Seconds(0),MicroSeconds(1000));
  //xiao_helper.ConfigStorShow();
  xiao_helper.makeAnim();
  Simulator::Run ();
  Simulator::Destroy ();
}



















/****************************************************
 * Hwn source start
 * 
 * 
 * 
 */
namespace ns3
{

/* static */
TypeId
Hwn::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Hwn")
    .SetParent<Object> ()
    .SetGroupName ("Hwn")
    .AddConstructor<Hwn> ()
    .AddTraceSource ("HwnStateValue",
                     "The state of HWN, indicating current HWN working state",
                     MakeTraceSourceAccessor (&Hwn::m_hwnState),
                     "ns3::TracedValueCallback::HwnState")
  ;
  return tid;
}

Hwn::Hwn()
: m_lrWpanMac(0),
m_apWiFiMac(0),
m_currentScheduleItem(0),
m_ctsWaitMax(MilliSeconds(20)),
m_ctsDurationMax(MicroSeconds(0x7fff)),
m_maxLrwpanPeriod(MilliSeconds(0)),
m_LrwpanExcessSP(MilliSeconds(10)),
m_hwnState(HWN_CS),
m_canScheduleNext(true)
{
  NS_LOG_FUNCTION (this);
}

Hwn::~Hwn ()
{
  NS_LOG_FUNCTION (this);
}


void
Hwn::DoInitialize ()
{
  Object::DoInitialize ();
}

void
Hwn::DoDispose ()
{
  Object::DoDispose ();
}

void
Hwn::SetLrWpanMac(Ptr<LrWpanMac> lrWpanMac)
{
  m_lrWpanMac = lrWpanMac;
  m_lrWpanMac->SetMcpsAnConfirmCallback(MakeCallback(&Hwn::LrwpanMcpsAnConfirm, this));
}
void
Hwn::SetApWiFiMac(Ptr<ApWifiMac> apWiFiMac)
{
  m_apWiFiMac = apWiFiMac;
  m_apWiFiMac->SetCtsInjectSentCallback(MakeCallback(&Hwn::WiFiMaclowCtsInjectSentCallback, this));
  m_apWiFiMac->SetCtsInjectStartSendingCallback(MakeCallback(&Hwn::WiFiMaclowCtsInjectStartSendingCallback, this));
}

void
Hwn::Schedule(Time lrwpanPeriod, Time wifiPeriod, uint32_t id)
{
  NS_ASSERT(m_lrWpanMac && m_apWiFiMac);
  //TODO.. provid a more friendly checker whether lrwpanPeriod and WifiPeriod is valid or not
  NS_ASSERT(lrwpanPeriod <= GetMaxLrwpanPeriod() && wifiPeriod <= GetMaxWifiPeriod());
  Ptr<ScheduleItem> si = Create<ScheduleItem>();
  si->id = id;
  si->lrwpanPeriod = lrwpanPeriod;
  si->wifiPeriod = wifiPeriod;
  si->enqueueTime = ns3::Now();
  Enqueue(si);
  CheckScheduleQueue();
}

void
Hwn::CheckScheduleQueue(void)
{
  if(!m_scheduleQueue.empty() && m_canScheduleNext)
  {
    //can start sending CTS and AN
    m_canScheduleNext = false;
    m_currentScheduleItem = Dequeue();
    m_currentScheduleItem->ctsInjectTime = ns3::Now();
    m_apWiFiMac->InjectCts(EstimateCtsPeriodByLrwpanPeriod(m_currentScheduleItem->lrwpanPeriod));
    m_scheduleEventId.Cancel();
    m_scheduleEventId = Simulator::Schedule(m_ctsWaitMax,&Hwn::CtsWaitTimeout, this);
  }
}


void
Hwn::Enqueue(Ptr<ScheduleItem> scheduleItem)
{
  m_scheduleQueue.push_back(scheduleItem);
}

Ptr<Hwn::ScheduleItem>
Hwn::Dequeue(void)
{
  Ptr<ScheduleItem> si = m_scheduleQueue.front();
  m_scheduleQueue.pop_front();
  return si;
}
void
Hwn::WiFiMaclowCtsInjectStartSendingCallback(Time duration)
{
  NS_LOG_FUNCTION(this << duration);
  //CTS starts sending, change state
  m_hwnState = HWN_MS;
  m_currentScheduleItem->ctsStartSendingTime = ns3::Now();
}
void
Hwn::WiFiMaclowCtsInjectSentCallback(Time duration)
{
  NS_LOG_FUNCTION(this << duration);
  //CTS successed injected, then sending AN frame
  m_currentScheduleItem->ctsSentTime = ns3::Now();
  m_currentScheduleItem->ctsRealDuration = duration;

  Time csmaCompensation = m_currentScheduleItem->ctsInjectTime - m_currentScheduleItem->ctsStartSendingTime;
  m_currentScheduleItem->wifiSlotEndTime = m_currentScheduleItem->ctsSentTime + duration + m_currentScheduleItem->wifiPeriod - csmaCompensation;
  if(csmaCompensation <= m_currentScheduleItem->wifiPeriod)
  {
    //wifi Period is enough for compensation, thus OK
    McpsAnRequestParams anParams;
    Time SpfDuration = m_currentScheduleItem->wifiPeriod + m_LrwpanExcessSP - csmaCompensation;
    anParams.m_GpExpire = (uint16_t) m_lrWpanMac->TimeToSymbol(duration);
    anParams.m_SPF = (uint8_t) (m_lrWpanMac->TimeToSymbol(SpfDuration)/64);
    m_lrWpanMac->McpsAnRequestImmediate(anParams);
    m_scheduleEventId.Cancel();
    m_scheduleEventId = Simulator::Schedule(duration,&Hwn::ChangeState,this,HWN_WS); // schedule to send next schedule item
  }else{
    //Error. Not engough wifi period to compensate csma delay;
    NS_LOG_WARN("CSMA compensation fail for HWN scheduling, thus return HWN_CS state");
    m_hwnState = HWN_CS;
    Ptr<ScheduleConfirmParameters> param = Create<ScheduleConfirmParameters>();
    param->status = HWN_CSMA_COMPENSATION_FAIL;
    if(!m_scheduleConfirmCb.IsNull()) m_scheduleConfirmCb(param);
    //cancel event and start next schedule if needed, by check schedule queue
    m_scheduleEventId.Cancel();
    m_canScheduleNext = true;
    CheckScheduleQueue();
  }
}

void
Hwn::LrwpanMcpsAnConfirm(struct McpsAnConfirmParams anParam)
{
  NS_LOG_FUNCTION(this << anParam.m_status);
  if(anParam.m_status == IEEE802154_SUCCESS)
  {
    //schedule success
    m_currentScheduleItem->anSentTime = ns3::Now();
    m_hwnState = HWN_LS;
    //callback scheduling successed
    Ptr<ScheduleConfirmParameters> param = Create<ScheduleConfirmParameters>();
    param->status = HWN_SUCCESS;
    if(!m_scheduleConfirmCb.IsNull()) m_scheduleConfirmCb(param);
  }else{
    NS_LOG_WARN("Send AN fail and fail code is:"+anParam.m_status);
    m_hwnState = HWN_CS;
    Ptr<ScheduleConfirmParameters> param = Create<ScheduleConfirmParameters>();
    param->status = HWN_LR_WPAN_FAILURE;
    if(!m_scheduleConfirmCb.IsNull()) m_scheduleConfirmCb(param);
    //cancel event and start next schedule if needed, by check schedule queue
    m_scheduleEventId.Cancel();
    m_canScheduleNext=true;
    CheckScheduleQueue();
  }
}

void
Hwn::CtsWaitTimeout(void)
{
  NS_LOG_FUNCTION(this);
  //CTS wait timeout, means scheduling fail, the CTS is not sent out
  m_hwnState = HWN_CS;
  Ptr<ScheduleConfirmParameters> param = Create<ScheduleConfirmParameters>();
  param->status = HWN_TIMEOUT;
  if(!m_scheduleConfirmCb.IsNull()) m_scheduleConfirmCb(param);
  m_scheduleEventId.Cancel();//may not necessary
  m_canScheduleNext = true;
  CheckScheduleQueue();
}

Time
Hwn::GetMaxLrwpanPeriod(void)
{
  if(m_maxLrwpanPeriod == Seconds(0)){
    //calculate maximum LrwpanPeriod
    Time lrTime = m_lrWpanMac->GetMinAnSendingTimeRequired();
    m_maxLrwpanPeriod = m_ctsDurationMax - lrTime;
    return m_maxLrwpanPeriod;
  }else{
    return m_maxLrwpanPeriod;
  }
}
Time
Hwn::EstimateCtsPeriodByLrwpanPeriod(Time lrwpanPeriod)
{
  return m_ctsDurationMax - GetMaxLrwpanPeriod() + lrwpanPeriod;
}
Time
Hwn::GetMaxWifiPeriod(void)
{
  return m_lrWpanMac->GetMaxAnSp() - m_LrwpanExcessSP;
}

void
Hwn::WiFiSlotTimeUp(void)
{
  NS_ASSERT(m_hwnState == HWN_WS);
  m_scheduleEventId.Cancel();
  m_scheduleEventId = ns3::Simulator::Schedule(m_LrwpanExcessSP, &Hwn::ChangeState, this, HWN_CS );
  m_canScheduleNext = true;
  CheckScheduleQueue();
}

void
Hwn::ChangeState(HwnState state)
{
  if(m_hwnState == HWN_LS && state == HWN_WS)
  {
    m_hwnState = HWN_WS;
    Time delay = m_currentScheduleItem->wifiSlotEndTime - ns3::Now();
    NS_ASSERT(delay >= 0);
    m_scheduleEventId.Cancel();
    m_scheduleEventId = ns3::Simulator::Schedule(delay, &Hwn::WiFiSlotTimeUp, this ); 
  }
  else if(m_hwnState == HWN_WS && state == HWN_CS)
  {
    //Excess SP has passed, thus change hwn state into HWN_CS state
    //the queue must be empty and no more shedule item in the queue
    m_hwnState = HWN_CS;
  }
  else
  {
    NS_LOG_ERROR("unexpected state");
    NS_ASSERT(false);
  }
  
}
};



/**
 * Hwn source end
 *********************************************************/
