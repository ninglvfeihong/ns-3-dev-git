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

#include <ns3/gnuplot.h>

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

  struct ScheduleReportParameters: public SimpleRefCount<ScheduleReportParameters>
  {
    Time managementPeriod;    //!<scheduling managemnt period
    Time lrwpanPeriod;
    Time wifiPeriod;
    Time csmaDelay;
    Time csmaCompensation;
    Time wifiCBT;         //!< wifi channel busy time, exclude HWN management frame
    Time lrwpanCBT;       //!< Lr-Wpan channel busy time, exclude HWN management frame
    uint64_t wifiPacketCount;  //wifi packet number
    uint64_t lrwpanPacketCount; //Lr-Wpan packet number
  };
  /**
   * Report statics
   */
  struct ScheduleReportStatics: public SimpleRefCount<ScheduleReportParameters>
  {
    Time wifiCBT;         //!< wifi channel busy time, exclude HWN management frame
    Time lrwpanCBT;       //!< Lr-Wpan channel busy time, exclude HWN management frame
    uint64_t wifiPacketCount;  //wifi packet number
    uint64_t lrwpanPacketCount; //Lr-Wpan packet number
  };
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
   * \param id id of the schedule, can be used to identify the schedule. provided by application
   */
  void Schedule(Time lrwpanPeriod, Time wifiPeriod, uint32_t id);

  void SetScheduleConfirmCallback(Callback<void, Ptr<ScheduleConfirmParameters>> cb);
  void SetScheduleReportCallback(Callback<void, Ptr<ScheduleReportParameters>> cb);
  void  ScheduleReportStatisticStart(void);
  void  ScheduleReportStatisticEnd(void);

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
   * current scheduling report statistics
   */
  Ptr<ScheduleReportStatics> m_reportStatics;
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
   * shchedule confirm, if the schedule sucessed or fail
   */
  Callback<void, Ptr<ScheduleReportParameters>> m_scheduleReportCb;
  
  /**
   * check the m_scheduleQueue to if schedule is necessary
   */
  void StartScheduleIfNeeded(void);
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

  /**
   * Lrwpan and wifi traces report statistics
   */
  void ReportLrwpanMacPromiscSniffer(Ptr<const Packet> p);
  void ReportWifiSniffRx( Ptr<const Packet>, uint16_t, WifiTxVector, MpduInfo, SignalNoiseDbm);
  void ReportWifiSniffTx( Ptr<const Packet>, uint16_t, WifiTxVector, MpduInfo);


};





};



/**
 * Hwn header end
 *********************************************************/




/****************************************************
 * LrWpanTestHeader header start
 * 
 */

namespace xiao
{

class LrWpanTestHeader : public Header
{
public:
  LrWpanTestHeader();
  LrWpanTestHeader(uint32_t id);
  ~LrWpanTestHeader();

  void SetId (uint32_t id);
  uint32_t GetId (void)const; 
  Time GetTs (void)const; 
  
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
private:
  uint32_t m_id; 
  uint64_t m_ts; //time step
};


};


/**
 * LrWpanTestHeader header end
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
  Time m_GenerateWiFiTrafficStartTime;
  Time m_GenerateWiFiTrafficTxCbReportTime;
  Time m_GenerateWiFiTrafficRxCbReportTime;
  uint64_t m_GenerateWiFiTrafficTxCbBytes = 0;
  void _GenerateWiFiTrafficTxCb(Ptr<const Packet> p){
    m_GenerateWiFiTrafficTxCbBytes+=p->GetSize();
    if(ns3::Now()-m_GenerateWiFiTrafficTxCbReportTime > Seconds(1)){
      m_GenerateWiFiTrafficTxCbReportTime = ns3::Now();
      NS_LOG_INFO("Scheduled" << m_GenerateWiFiTrafficTxCbBytes*8*2/(Now() - m_GenerateWiFiTrafficStartTime).GetSeconds() /1e6 << "Mbps");
    }
  }
  uint64_t m_GenerateWiFiTrafficRxCbBytes = 0;
  void _GenerateWiFiTrafficRxCb(Ptr<const Packet> p){
    m_GenerateWiFiTrafficRxCbBytes+=p->GetSize();
    if(ns3::Now()-m_GenerateWiFiTrafficRxCbReportTime > Seconds(1)){
      m_GenerateWiFiTrafficRxCbReportTime = ns3::Now();
      NS_LOG_INFO("UDP throughput" << m_GenerateWiFiTrafficRxCbBytes*8/(Now() - m_GenerateWiFiTrafficStartTime).GetSeconds() /1e6 << "Mbps");
    }
  }
  void GenerateWiFiTraffic(Ptr<Node> server, ns3::Ipv4Address addr, uint16_t port, Ptr<Node> client, double speedIn_mbps, Time start, Time stop){
    uint64_t pcketSize = 1472;
    double interval = (pcketSize*8*2/*send and echo data*/)/(1e6*speedIn_mbps);
    UdpEchoServerHelper echoServer (port);

    ApplicationContainer echoServerApp = echoServer.Install(server);
    echoServerApp.Start(Seconds(0));
    echoServerApp.Stop (Seconds(0));

    //Ptr<ns3::Ipv4> apIpv4 = wifiApNode.Get(0)->GetObject<ns3::Ipv4>();
    //apIpv4->GetInterfaceForDevice(apDevices.Get(0));
    //apIpv4->GetAddress(apIpv4->GetInterfaceForDevice(apDevices.Get(0)),0);
    UdpEchoClientHelper echoClient (addr, port);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (UINT32_MAX));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (interval)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (pcketSize));
    ApplicationContainer clientApps = 
      echoClient.Install (client);
    clientApps.Start (start);
    clientApps.Stop (stop);
    clientApps.Get(0)->TraceConnectWithoutContext("Tx",MakeCallback(&Helper::_GenerateWiFiTrafficTxCb, this));
    clientApps.Get(0)->TraceConnectWithoutContext("Rx",MakeCallback(&Helper::_GenerateWiFiTrafficRxCb, this));
    echoServerApp.Get(0)->TraceConnectWithoutContext("Rx",MakeCallback(&Helper::_GenerateWiFiTrafficRxCb, this));
    m_GenerateWiFiTrafficStartTime= start;
    m_GenerateWiFiTrafficTxCbReportTime = start;
    m_GenerateWiFiTrafficRxCbReportTime = start;
  }

  //UDP client server test
  Time m_GenerateWiFiTrafficUdpStartTime;
  Time m_GenerateWiFiTrafficUdpEndTime;
  Time m_GenerateWiFiTrafficUdpRxCbReportTime;
  uint64_t m_GenerateWiFiTrafficUdpRxCbBytes = 0;
  uint64_t m_GenerateWiFiTrafficUdpRxCbByPacket=0;
  Time m_m_GenerateWiFiTrafficUdp_allDelay = Seconds(0);
  void _GenerateWiFiTrafficUdpRxCb(Ptr<const Packet> p){
    if(Now() > m_GenerateWiFiTrafficUdpEndTime) return;
    m_GenerateWiFiTrafficUdpRxCbBytes+=p->GetSize();
    m_GenerateWiFiTrafficUdpRxCbByPacket++;
    SeqTsHeader seqTs;
    p->PeekHeader (seqTs);
    m_m_GenerateWiFiTrafficUdp_allDelay += Now() - seqTs.GetTs();
    if(ns3::Now()-m_GenerateWiFiTrafficUdpRxCbReportTime > Seconds(1)){
      m_GenerateWiFiTrafficUdpRxCbReportTime = ns3::Now();
      NS_LOG_INFO("UDP throughput:" << m_GenerateWiFiTrafficUdpRxCbBytes*8/(Now() - m_GenerateWiFiTrafficUdpStartTime).GetSeconds() /1e6 << "Mbps");
      NS_LOG_INFO("UDP average delay:" <<(m_m_GenerateWiFiTrafficUdp_allDelay / (m_GenerateWiFiTrafficUdpRxCbBytes/p->GetSize())).GetSeconds()*1e3 << "ms");
    }
  }
  //throughtput in Mbps
  double GetUdpThroughtput(){
    if(m_GenerateWiFiTrafficUdpEndTime == m_GenerateWiFiTrafficUdpStartTime) return 0;
    return m_GenerateWiFiTrafficUdpRxCbBytes*8/(m_GenerateWiFiTrafficUdpEndTime - m_GenerateWiFiTrafficUdpStartTime).GetSeconds() /1e6 ;
  }
  Time GetUdpDelay(){
    if(m_GenerateWiFiTrafficUdpRxCbByPacket == 0){
      return Seconds(0);
    }else{
      return m_m_GenerateWiFiTrafficUdp_allDelay/m_GenerateWiFiTrafficUdpRxCbByPacket;
    }
  }
  void GenerateWiFiTrafficUdp(Ptr<Node> node1, ns3::Ipv4Address addr1, Ptr<Node> node2,ns3::Ipv4Address addr2, double speedIn_mbps, Time start, Time stop){
    if(speedIn_mbps==0){
      //not wifi generate needed 
      return;
    }
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
    rand->SetAttribute ("Min", DoubleValue (0.0));
    rand->SetAttribute ("Max", DoubleValue (0.001));

    uint64_t pcketSize = 1472;
    double interval = (pcketSize*8*2/*send and echo data*/)/(1e6*speedIn_mbps);
    UdpServerHelper udpServer;
    NodeContainer nodes;
    nodes.Add(node1);
    nodes.Add(node2);
    ApplicationContainer udpServerApps = udpServer.Install(nodes);
    udpServerApps.Start(Seconds(0));
    udpServerApps.Stop (Seconds(0));


    UdpClientHelper echoClient (addr1);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (UINT32_MAX));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (interval)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (pcketSize));
    ApplicationContainer clientApps = echoClient.Install (node2);
    clientApps.Start (start);
    clientApps.Stop (stop);
    echoClient.SetAttribute("RemoteAddress",AddressValue(addr2));
    clientApps = echoClient.Install (node1);
    clientApps.Start (start+ Seconds (interval/2));
    clientApps.Stop (stop);
    udpServerApps.Get(0)->TraceConnectWithoutContext("Rx",MakeCallback(&Helper::_GenerateWiFiTrafficUdpRxCb, this));
    udpServerApps.Get(1)->TraceConnectWithoutContext("Rx",MakeCallback(&Helper::_GenerateWiFiTrafficUdpRxCb, this));
    m_GenerateWiFiTrafficUdpStartTime= start;
    m_GenerateWiFiTrafficUdpEndTime = stop;
    m_GenerateWiFiTrafficUdpRxCbReportTime = start;
  }


  //LR-WPAN data collection
  uint32_t m_lrwpanScheduleCounter_tx =0;
  uint32_t m_NetDevCb_counter=0;
  Time m_NetDevCb_allDelay = Seconds(0);
  Time m_NetDevCb_reportTime =  Seconds(0);
  bool NetDevCb(Ptr<NetDevice> netDev, Ptr<const Packet> p, uint16_t protocol, const Address & addr)
  {
    m_NetDevCb_counter ++;
    LrWpanTestHeader hdr;
    p->PeekHeader(hdr);
    m_NetDevCb_allDelay += Now()-hdr.GetTs();
    //NS_LOG_INFO("NetDevice received a packet! count:"+std::to_string(m_NetDevCb_counter));

    
    if(ns3::Now()-m_NetDevCb_reportTime > Seconds(1)){
      m_NetDevCb_reportTime = ns3::Now();
       NS_LOG_INFO("LR-WPAN rx/ScheduledTx:" << m_NetDevCb_counter << "/" << m_lrwpanScheduleCounter_tx);
       NS_LOG_INFO("LR-WPAN delay:" << (m_NetDevCb_allDelay/m_NetDevCb_counter).GetSeconds()*1e3 << "ms");
    }
    return true;
  }
  double GetLrWpanPLR(){
    return 1 - 1.0*m_NetDevCb_counter/m_lrwpanScheduleCounter_tx;
  }
  Time GetLrWpanDelay(){
    if(m_NetDevCb_counter == 0){
      return Seconds(0);
    }else{
      return m_NetDevCb_allDelay/m_NetDevCb_counter;
    }
  }
  void _LrWpanSendScheduleBroadcastSend(ns3::Ptr<ns3::Node> &sender,  const ns3::Time &end, const ns3::Time &interval){
    m_lrwpanScheduleCounter_tx++;
    LrWpanTestHeader hdr;
    hdr.SetId(m_lrwpanScheduleCounter_tx);
    Ptr<Packet> p = Create<Packet>(20-hdr.GetSerializedSize()); //20 byte packet
    p->AddHeader(hdr);
    sender->GetDevice(0)->Send(p, ns3::Mac16Address("ff:ff"),0);
    if(interval + Now() <= end)
      Simulator::Schedule(interval, &Helper::_LrWpanSendScheduleBroadcastSend, this, sender, end, interval);
  }
  void
  LrWpanSendScheduleBroadcast(ns3::Ptr<ns3::Node> &sender,
                    const ns3::Time &start, const ns3::Time &end, const ns3::Time &interval)
  {
    m_NetDevCb_reportTime = start;
    Simulator::Schedule(start, &Helper::_LrWpanSendScheduleBroadcastSend, this,sender, end, interval);
  }


  Time m_hwnStaticSchedule_lrwpanSlot = MilliSeconds(30);
  Time m_hwnStaticSchedule_wifiSlot = MilliSeconds(30);
  Time m_hwnStaticSchedule_end;
  Ptr<Hwn> m_hwnStaticSchedule_hwn;
  void HwnStaticScheduleCb(Ptr<Hwn::ScheduleConfirmParameters> scheduleConfirm){
    if(Now()>m_hwnStaticSchedule_end) return;
    if(scheduleConfirm->status == Hwn::HWN_SUCCESS){
      m_hwnStaticSchedule_hwn->Schedule(m_hwnStaticSchedule_lrwpanSlot,m_hwnStaticSchedule_wifiSlot,1);
    }else{
      NS_LOG_WARN("Schedule failed:"<< scheduleConfirm->status);
      m_hwnStaticSchedule_hwn->Schedule(m_hwnStaticSchedule_lrwpanSlot,m_hwnStaticSchedule_wifiSlot,1);
    }
  }
  struct {
    uint64_t schedulingCount;
    Time csmaDelaySum;
    Time managementPeriodSum;
    Time lrwpanPeriodSum;
    Time wifiPeriodSum;
    Time lrwpanCBTSum;
    Time wifiCBTSum;
    uint64_t lrwpanPacketCount;
    uint64_t wifiPacketCount;

  } hwnStatistic;

  void
  HwnStatisticInit(void){
    hwnStatistic.schedulingCount = 0;
    hwnStatistic.csmaDelaySum = Seconds(0);
    hwnStatistic.managementPeriodSum = Seconds(0);
    hwnStatistic.lrwpanCBTSum = Seconds(0);
    hwnStatistic.wifiPeriodSum = Seconds(0);
    hwnStatistic.lrwpanCBTSum = Seconds(0);
    hwnStatistic.wifiCBTSum = Seconds(0);
    hwnStatistic.lrwpanPacketCount = 0;
    hwnStatistic.wifiPacketCount = 0;
    
  }
  void HwnStatisticReportCb(Ptr<Hwn::ScheduleReportParameters> param){
    // NS_LOG_INFO("CsmaDelay" << param->csmaDelay);
    // NS_LOG_INFO("Manaement Period " << param->managementPeriod);
    // NS_LOG_INFO("LR-WPAN CBT:" <<param->lrwpanCBT);
    // NS_LOG_INFO("WiFi CBT:" <<param->wifiCBT);
    // NS_LOG_INFO("LR-WPAN Count:" <<param->lrwpanPacketCount);
    // NS_LOG_INFO("WiFi Count:" <<param->wifiPacketCount);
    if(param->wifiCBT >= param->wifiPeriod){
      //scheduling fail;
      return;
    }
    hwnStatistic.schedulingCount++;
    hwnStatistic.csmaDelaySum += param->csmaDelay;
    hwnStatistic.managementPeriodSum += param->managementPeriod;
    hwnStatistic.lrwpanPeriodSum += param->lrwpanPeriod;
    hwnStatistic.wifiPeriodSum += param->wifiPeriod;
    hwnStatistic.lrwpanCBTSum += param->lrwpanCBT;
    hwnStatistic.wifiCBTSum += param->wifiCBT;
    hwnStatistic.lrwpanPacketCount += param->lrwpanPacketCount;
    hwnStatistic.wifiPacketCount += param->wifiPacketCount;
  }
  Time HwnGetManagementDelay(void){
    return hwnStatistic.csmaDelaySum/hwnStatistic.schedulingCount;
  }
  double HwnGetManagementOverhead(void){
    return hwnStatistic.managementPeriodSum.GetSeconds() / (hwnStatistic.wifiPeriodSum + hwnStatistic.lrwpanPeriodSum + hwnStatistic.managementPeriodSum).GetSeconds();
  }
  double HwnGetSlotUsageLrwpan(void){
    return hwnStatistic.lrwpanCBTSum.GetSeconds() / hwnStatistic.lrwpanPeriodSum.GetSeconds();
  }
  double HwnGetSlotUsageWifi(void){
    return hwnStatistic.wifiCBTSum.GetSeconds() / hwnStatistic.wifiPeriodSum.GetSeconds();
  }
  void HwnStaticScheduleStart(){
    HwnStatisticInit();
    m_hwnStaticSchedule_hwn->Schedule(m_hwnStaticSchedule_lrwpanSlot,m_hwnStaticSchedule_wifiSlot,0);
  }
  void HwnStaticSchedule(Ptr<Hwn> hwn, const Time &startTime, Time end )
  {
    m_hwnStaticSchedule_end = end;
    m_hwnStaticSchedule_hwn = hwn;
    hwn->SetScheduleConfirmCallback(MakeCallback(&Helper::HwnStaticScheduleCb, this));
    hwn->SetScheduleReportCallback(MakeCallback(&Helper::HwnStatisticReportCb, this));
    Simulator::Schedule(startTime, &Helper::HwnStaticScheduleStart, this);
  }
};




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






};






int 
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("My first hello word!");
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnable ("LrWpanTestByXiao", LOG_INFO);
  
  //Packet::EnablePrinting ();
  //Packet::EnableChecking();

  double desiredWiFiSpeed =20; 
  double desiredWiFiSpeedMax = 32; //Mbps
  double desiredWiFiSpeedStep = 100; //Mbps
  Time simulationTimePerRound = Seconds(9);

  std::ofstream lrWpanPlrPlotFile ("cim-static-simple-lrwpan-plr.plt");
  //Gnuplot lrWpanPlrPlot = Gnuplot ("cim-static-simple-lrwpan-plr.eps");
  Gnuplot lrWpanPlrPlot = Gnuplot ();
  lrWpanPlrPlot.SetTitle("LR-WPAN Packet Loss Rate (PLR) vs desired WiFi speed");
  Gnuplot2dDataset lrWpanPlrNodataset ("Original");
  Gnuplot2dDataset lrWpanPlrSchdataset ("Proposed Algorithm");

  std::ofstream lrWpanDelayPlotFile ("cim-static-simple-lrwpan-delay.plt");
  Gnuplot lrWpanDelayPlot = Gnuplot ();
  lrWpanDelayPlot.SetTitle("LR-WPAN packet delay vs desired WiFi speed");
  Gnuplot2dDataset lrWpanDelayNodataset ("Original");
  Gnuplot2dDataset lrWpanDelaySchdataset ("Proposed Algorithm");

  std::ofstream wifiThroughputPlotFile ("cim-static-simple-wifi-throughput.plt");
  Gnuplot wifiThroughputPlot = Gnuplot ();
  wifiThroughputPlot.SetTitle("WiFi real speed vs desired WiFi speed");
  Gnuplot2dDataset wifiThroughputNodataset ("Original");
  Gnuplot2dDataset wifiThroughputSchdataset ("Proposed Algorithm");

  std::ofstream wifiDelayPlotFile ("cim-static-simple-wifi-delay.plt");
  Gnuplot wifiDelayPlot = Gnuplot ();
  wifiDelayPlot.SetTitle("WiFi packet delay vs desired WiFi speed");
  Gnuplot2dDataset wifiDelayNodataset ("Original");
  Gnuplot2dDataset wifiDelaySchdataset ("Proposed Algorithm");
  
  for(; desiredWiFiSpeed <= desiredWiFiSpeedMax; desiredWiFiSpeed += desiredWiFiSpeedStep)
  {
    for(int i=1;i<2;i++){
      bool isWithScheduling;
      switch (i){
        case 0:
          isWithScheduling = false;
          break;
        case 1:
          isWithScheduling = true;
          break;
        default:
          NS_ASSERT(false);//no such case;
          break;
      }

      xiao::Helper xiao_helper;
      /////////////////////////////////
      // Configure LR-WPAN
      /////////////////////////////////
      ns3::NodeContainer nodes;
      nodes.Create(3);

      LrWpanHelper lrWpanHelper(true);
      /*if (verbose)
      {
          lrWpanHelper.EnableLogComponents ();
      }*/
      
      //install Mobility to nodes;
      Ptr<Node> gateway = nodes.Get(0);
      Ptr<Node> lrwpanNode = nodes.Get(1);
      Ptr<Node> wifiStation = nodes.Get(1);
      ns3::Ptr<ListPositionAllocator> locationAllocator = ns3::CreateObject<ListPositionAllocator>();
      locationAllocator->Add(ns3::Vector(0,5,0));     //gateway
      locationAllocator->Add(ns3::Vector(10,0,0));  //lrwpanNode
      locationAllocator->Add(ns3::Vector(10,10,0));  //wifi station
      MobilityHelper mobility;
      mobility.SetPositionAllocator(locationAllocator);
      mobility.Install(nodes);


      ns3::NodeContainer lrPandNodes;
      lrPandNodes.Add(gateway);
      lrPandNodes.Add(lrwpanNode);
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
      lrDevices.Get(0)->SetReceiveCallback(MakeCallback(&xiao::Helper::NetDevCb,&xiao_helper));



      /////////////////////////////////
      // Configure WiFi -- prepare
      /////////////////////////////////

      ns3::Ptr<ns3::SpectrumChannel> channel = lrWpanHelper.GetChannel();


      /////////////////////////////////
      // Configure WiFi
      /////////////////////////////////
      NodeContainer wifiNodes;
      wifiNodes.Add(gateway);
      wifiNodes.Add(wifiStation);

      ns3::SpectrumWifiPhyHelper wifiPhy = ns3::SpectrumWifiPhyHelper::Default();
      wifiPhy.SetChannel(channel);
      wifiPhy.Set("ChannelNumber",UintegerValue(6));



      WifiHelper wifi;
      //wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
      //wifi.SetRemoteStationManager("ns3::AarfWifiManager");
      //wifi.SetRemoteStationManager("ns3::AmrrWifiManager");
      //wifi.SetRemoteStationManager("ns3::CaraWifiManager");
      //wifi.SetRemoteStationManager("ns3::IdealWifiManager");
      //wifi.SetRemoteStationManager("ns3::RraaWifiManager");
      //wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");
      wifi.SetRemoteStationManager("ns3::MinstrelWifiManager");
      //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","DataMode", StringValue ("OfdmRate27MbpsBW10MHz"));
      //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","DataMode", StringValue ("OfdmRate54Mbps"));
      // wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
      //                                      "DataMode", StringValue ("HtMcs7"),
      //                                      "ControlMode", StringValue ("HtMcs0"));
      
      //config station nodes
      WifiMacHelper mac;
      Ssid ssid = Ssid ("ns-3-ssid");
      mac.SetType ("ns3::StaWifiMac",
                  "Ssid", SsidValue (ssid),
                  "ActiveProbing", BooleanValue (false));
      NetDeviceContainer staDevices = wifi.Install (wifiPhy, mac, wifiStation);
      //config ap node
      mac.SetType ("ns3::ApWifiMac",
                  "Ssid", SsidValue (ssid));
      NetDeviceContainer apDevices = wifi.Install (wifiPhy, mac, gateway);

      

      //intall internet statck
      InternetStackHelper internetStackHelper;
      internetStackHelper .Install(wifiNodes);

      Ipv4AddressHelper addressHelper;
      addressHelper.SetBase ("192.168.1.0", "255.255.255.0");
      ns3::Ipv4InterfaceContainer wifiStaIpv4Interfaces = addressHelper.Assign (staDevices);
      ns3::Ipv4InterfaceContainer wifiApIpv4Interfaces =  addressHelper.Assign (apDevices);

      //schedule wifi traffic
      xiao_helper.GenerateWiFiTraffic(gateway,wifiApIpv4Interfaces.GetAddress (0),9,wifiStation,1,Seconds(0.2),Seconds(0.3)); //send packet allow ARP build, before real experiment
      //xiao_helper.GenerateWiFiTraffic(gateway,wifiApIpv4Interfaces.GetAddress (0),9,wifiStation,30,Seconds(1),Seconds(6)); //send packet allow ARP build, before real experiment
      //xiao_helper.GenerateWiFiTraffic(wifiStation,wifiStaIpv4Interfaces.GetAddress (0),9,gateway,30,Seconds(1),Seconds(6));

      xiao_helper.GenerateWiFiTrafficUdp(gateway,wifiApIpv4Interfaces.GetAddress (0),wifiStation,wifiStaIpv4Interfaces.GetAddress (0),
                                          desiredWiFiSpeed,Seconds(1),simulationTimePerRound);
      //stupid way scheduling lr-wpan packet seding
      xiao_helper.LrWpanSendScheduleBroadcast(lrwpanNode, Seconds(1.12),simulationTimePerRound-Seconds(0.5),MilliSeconds(300));

      /********************************************************
       * configuration Hwn
       */
      ns3::Ptr<ns3::LrWpanMac> lrwpanMac = lrDevices.Get(0)->GetObject<LrWpanNetDevice>()->GetMac();
      Ptr<ApWifiMac> apMac = apDevices.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>();

      Ptr<Hwn> hwn = Create<Hwn>();
      hwn->SetLrWpanMac(lrwpanMac);
      hwn->SetApWiFiMac(apMac);
      
      if(isWithScheduling) xiao_helper.HwnStaticSchedule(hwn,Seconds(1),simulationTimePerRound);
      


      Simulator::Stop (simulationTimePerRound+Seconds(0.5));
      xiao_helper.PlaceSpectrum(channel,Vector(5,5,0),Seconds(0.1),Seconds(3),MicroSeconds(1000));
      //xiao_helper.ConfigStorShow();
      //xiao_helper.makeAnim();
      Simulator::Run ();

      NS_LOG_INFO("Round Overall:");
      NS_LOG_INFO("LR-WPAN PLR:" << xiao_helper.GetLrWpanPLR()*1e2 <<"%");
      NS_LOG_INFO("LR-WPAN Delay:" << xiao_helper.GetLrWpanDelay().GetSeconds()*1e3 <<" ms");
      NS_LOG_INFO("WiFi ThrouthPut:" << xiao_helper.GetUdpThroughtput() << " Mbps");
      NS_LOG_INFO("WiFi Delay:"<< xiao_helper.GetUdpDelay().GetSeconds()*1e3 <<" ms");
      NS_LOG_INFO("Management Average Delay" << xiao_helper.HwnGetManagementDelay().GetSeconds()*1e3 <<" ms");
      NS_LOG_INFO("Management Overhead " << xiao_helper.HwnGetManagementOverhead()*1e2 <<"%");
      NS_LOG_INFO("Lrwpan slot usage " << xiao_helper.HwnGetSlotUsageLrwpan()*1e2 <<"%");
      NS_LOG_INFO("WiFi slot usage " << xiao_helper.HwnGetSlotUsageWifi()*1e2 <<"%");
      if(isWithScheduling){
        lrWpanPlrSchdataset.Add(desiredWiFiSpeed, xiao_helper.GetLrWpanPLR());
        lrWpanDelaySchdataset.Add(desiredWiFiSpeed, xiao_helper.GetLrWpanDelay().GetSeconds()*1e3);
        wifiThroughputSchdataset.Add(desiredWiFiSpeed, xiao_helper.GetUdpThroughtput() );
        wifiDelaySchdataset.Add(desiredWiFiSpeed,xiao_helper.GetUdpDelay().GetSeconds()*1e3);
      }else{
        lrWpanPlrNodataset.Add(desiredWiFiSpeed, xiao_helper.GetLrWpanPLR());
        lrWpanDelayNodataset.Add(desiredWiFiSpeed, xiao_helper.GetLrWpanDelay().GetSeconds()*1e3);
        wifiThroughputNodataset.Add(desiredWiFiSpeed, xiao_helper.GetUdpThroughtput() );
        wifiDelayNodataset.Add(desiredWiFiSpeed,xiao_helper.GetUdpDelay().GetSeconds()*1e3);
      }
      Simulator::Destroy ();
    }
  }
  lrWpanPlrPlot.AddDataset(lrWpanPlrNodataset);
  lrWpanPlrPlot.AddDataset(lrWpanPlrSchdataset);
  lrWpanDelayPlot.AddDataset(lrWpanDelayNodataset);
  lrWpanDelayPlot.AddDataset(lrWpanDelaySchdataset);
  wifiThroughputPlot.AddDataset(wifiThroughputNodataset);
  wifiThroughputPlot.AddDataset(wifiThroughputSchdataset);
  wifiDelayPlot.AddDataset(wifiDelayNodataset);
  wifiDelayPlot.AddDataset(wifiDelaySchdataset);
  //wifiDelayNodataset.SetExtra()

  
  //psrplot.SetTitle (os.str ());
  //lrWpanPlrPlot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  //lrWpanPlrPlot.SetLegend ("distance (m)", "Packet Success Rate (PSR)");
  //set terminal wxt size 350,262 enhanced font 'Verdana,10' persist
  //lrWpanPlrPlot.SetTerminal ("wxt size 350,262 enhanced font 'Verdana,10' persist");
  
  lrWpanPlrSchdataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  lrWpanPlrPlot.SetLegend ("Desired WiFi Speed (Mbps)", "Packet Loss Rate (PLR)");
  lrWpanPlrPlot.SetExtra  (
"set xrange [0:32]\n\
set grid\n\
set style line 1 lw 2 ps 2\n\
set style line 2 lw 2 ps 2\n\
set style increment user");
  lrWpanPlrPlot.GenerateOutput (lrWpanPlrPlotFile);
  lrWpanPlrPlotFile.close ();
  
  lrWpanDelaySchdataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  lrWpanDelayPlot.SetLegend ("Desired WiFi Speed (Mbps)", "LR-WPAN Packet Delay (ms)");
  lrWpanDelayPlot.SetExtra  ("\
set xrange [0:32]\n\
set grid\n\
set style line 1 lw 2 ps 2\n\
set style line 2 lw 2 ps 2\n\
set style increment user");
  lrWpanDelayPlot.GenerateOutput (lrWpanDelayPlotFile);
  lrWpanDelayPlotFile.close ();

  wifiThroughputSchdataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  wifiThroughputPlot.SetLegend ("Desired WiFi Speed (Mbps)", "Real WiFi Speed (Mbps)");
  wifiThroughputPlot.SetExtra  ("\
set xrange [0:32]\n\
set grid\n\
set style line 1 lw 2 ps 2\n\
set style line 2 lw 2 ps 2\n\
set style increment user");
  wifiThroughputPlot.GenerateOutput (wifiThroughputPlotFile);
  wifiThroughputPlotFile.close ();

  wifiDelaySchdataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  wifiDelayPlot.SetLegend ("Desired WiFi Speed (Mbps)", "WiFi Packet Delay (ms)");
  wifiDelayPlot.SetExtra  ("\
set xrange [0:32]\n\
set logscale y 10\n\
set grid\n\
set style line 1 lw 2 ps 2\n\
set style line 2 lw 2 ps 2\n\
set style increment user");
  wifiDelayPlot.GenerateOutput (wifiDelayPlotFile);
  wifiDelayPlotFile.close ();


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
m_reportStatics(Create<ScheduleReportStatics>()),
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
Hwn::SetScheduleConfirmCallback(Callback<void, Ptr<ScheduleConfirmParameters>> cb)
{
  m_scheduleConfirmCb = cb;
}

void
Hwn::SetScheduleReportCallback(Callback<void, Ptr<ScheduleReportParameters>> cb)
{
  m_scheduleReportCb = cb;
}


void
Hwn::ReportLrwpanMacPromiscSniffer(Ptr<const Packet> p)
{
  //NS_LOG_INFO("LR-wpan packet..................");
  LrWpanMacHeader macHdr;
  p->PeekHeader(macHdr);
  if(macHdr.GetCmdIdentifier() == LrWpanMacHeader::LRWPAN_MAC_CMD_ASSESS_NOTIFICATION)
  {
    //management frame.
    //do nothing
  }else{
    //normal frame
    double SPO = m_lrWpanMac->GetPhy()->GetPhySymbolsPerOctet();
    int32_t symbolN = SPO*(1/*PHY header*/ + p->GetSize()) + m_lrWpanMac->GetPhy()->GetPhySHRDuration();
    Time txTime = m_lrWpanMac->SymbolToTime(symbolN);
    m_reportStatics->lrwpanCBT += txTime;
    m_reportStatics->lrwpanPacketCount ++;
  }
}
void
Hwn::ReportWifiSniffRx( Ptr<const Packet> p, uint16_t, WifiTxVector txVector, MpduInfo mpduInfo, SignalNoiseDbm snr)
{
  Time txDuration = m_apWiFiMac->GetWifiPhy()->CalculateTxDuration (p->GetSize(),txVector, m_apWiFiMac->GetWifiPhy()->GetFrequency ());
  //NS_LOG_INFO("wifi packet.................." << txDuration);
  //p->Print(std::cout);
  m_reportStatics->wifiCBT += txDuration;
  m_reportStatics->wifiPacketCount ++;
}
void
Hwn::ReportWifiSniffTx( Ptr<const Packet> p, uint16_t, WifiTxVector txVector, MpduInfo mpduInfo)
{
  Time txDuration = m_apWiFiMac->GetWifiPhy()->CalculateTxDuration (p->GetSize(),txVector, m_apWiFiMac->GetWifiPhy()->GetFrequency ());
  //NS_LOG_INFO("wifi packet.................." << txDuration);
  //p->Print(std::cout);
  m_reportStatics->wifiCBT += txDuration;
  m_reportStatics->wifiPacketCount ++;
}

void
Hwn::ScheduleReportStatisticStart(void)
{
  //static Callback<void, Ptr<const Packet>> rptMacPromiscRx;
  //static Callback<void, Ptr<const Packet>, uint16_t, WifiTxVector, MpduInfo, SignalNoiseDbm> phyMonitorSniffRxTrace;
  static bool firstTime = true;
  //using ns3 trace source to get CBT (channel busy time) information 
  if(firstTime){
    firstTime = false;
    m_lrWpanMac->TraceConnectWithoutContext("PromiscSniffer", MakeCallback(&Hwn::ReportLrwpanMacPromiscSniffer, this));
    m_apWiFiMac->GetWifiPhy()->TraceConnectWithoutContext("MonitorSnifferRx", MakeCallback(&Hwn::ReportWifiSniffRx, this));
    m_apWiFiMac->GetWifiPhy()->TraceConnectWithoutContext("MonitorSnifferTx", MakeCallback(&Hwn::ReportWifiSniffTx, this));
  }
  m_reportStatics->lrwpanCBT = Seconds(0);
  m_reportStatics->wifiCBT = Seconds(0);
  m_reportStatics->lrwpanPacketCount = 0;
  m_reportStatics->wifiPacketCount = 0;
}


void
Hwn::ScheduleReportStatisticEnd(void)
{
  if(m_hwnState == HWN_WS){
    Ptr<ScheduleReportParameters> param = Create<ScheduleReportParameters>();
    //not work if CTS is not received by others nodes. 
    //param->managementPeriod = m_currentScheduleItem->anSentTime - m_currentScheduleItem->ctsStartSendingTime;
    //param->managementPeriod =  NanoSeconds(1004000); //seting using experiment const vaule for managementPeriod.
    //param->managementPeriod = NanoSeconds(44000) /*WiFi CTS*/ + m_lrWpanMac->GetMinAnSendingTimeRequired();
    //m_lrWpanMac->GetMinAnSendingTimeRequired() may optimized by cache static vaule to speed up AnSedning Time Reuqired caculation
    Time wifiCtsDuration = m_currentScheduleItem->ctsSentTime - m_currentScheduleItem->ctsStartSendingTime /*WiFi CTS*/ ;
    param->managementPeriod = wifiCtsDuration + m_lrWpanMac->GetMinAnSendingTimeRequired();
    param->lrwpanPeriod = m_currentScheduleItem->lrwpanPeriod;
    param->wifiPeriod = m_currentScheduleItem ->wifiPeriod;
    param->csmaDelay = m_currentScheduleItem->ctsStartSendingTime - m_currentScheduleItem->ctsInjectTime;
    param->csmaCompensation = param->csmaDelay; 
    param->lrwpanCBT = m_reportStatics->lrwpanCBT;          //the management frame has already removed during statistic collection
    param->lrwpanPacketCount = m_reportStatics->lrwpanPacketCount;
    param->wifiCBT = m_reportStatics->wifiCBT - wifiCtsDuration; //remove management frame : wifi CTS
    param->wifiPacketCount = m_reportStatics->wifiPacketCount -1; //minus one wifi management frame: wifi CTS
    if(! m_scheduleReportCb.IsNull()) m_scheduleReportCb(param);
  }else{
    NS_LOG_ERROR("Unexpected m_hwnState state for statistic End");
    NS_ASSERT(m_hwnState == HWN_WS);
  }
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
  StartScheduleIfNeeded();
}

void
Hwn::StartScheduleIfNeeded(void)
{
  if(!m_scheduleQueue.empty() && m_canScheduleNext)
  {
    //report statistic start
    ScheduleReportStatisticStart();
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

  Time csmaCompensation = m_currentScheduleItem->ctsStartSendingTime - m_currentScheduleItem->ctsInjectTime;
  m_currentScheduleItem->wifiSlotEndTime = m_currentScheduleItem->ctsSentTime + duration + m_currentScheduleItem->wifiPeriod - csmaCompensation;
  if(csmaCompensation <= m_currentScheduleItem->wifiPeriod)
  {
    //wifi Period is enough for compensation, thus OK
    McpsAnRequestParams anParams;
    Time SpfDuration = m_currentScheduleItem->wifiPeriod + m_LrwpanExcessSP - csmaCompensation;
    //minus NanoSeconds(1) ensure the LrwpanMcpsAnConfirm is called before  Simulator::Schedule(duration,&Hwn::ChangeState,this,HWN_WS);
    anParams.m_GpExpire = (uint16_t) m_lrWpanMac->TimeToSymbol(duration - NanoSeconds(1)); 
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
    StartScheduleIfNeeded();
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
    NS_LOG_WARN("Send AN fail and fail code is:" << anParam.m_status);
    m_hwnState = HWN_CS;
    Ptr<ScheduleConfirmParameters> param = Create<ScheduleConfirmParameters>();
    param->status = HWN_LR_WPAN_FAILURE;
    if(!m_scheduleConfirmCb.IsNull()) m_scheduleConfirmCb(param);
    //cancel event and start next schedule if needed, by check schedule queue
    m_scheduleEventId.Cancel();
    m_canScheduleNext=true;
    StartScheduleIfNeeded();
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
  StartScheduleIfNeeded();
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
  ScheduleReportStatisticEnd(); //statistic end
  StartScheduleIfNeeded();
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













/****************************************************
 * LrWpanTestHeader source start
 * 
 */
namespace xiao
{
LrWpanTestHeader::LrWpanTestHeader()
  :
  m_id(0),
  m_ts(Simulator::Now().GetTimeStep())
{
}
LrWpanTestHeader::LrWpanTestHeader(uint32_t id)
{
  m_id = id;
}
LrWpanTestHeader::~LrWpanTestHeader(){}

TypeId
LrWpanTestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("xiao::LrWpanTestHeader")
    .SetParent<Header> ()
    .SetGroupName ("xiao")
    .AddConstructor<LrWpanTestHeader> ();
  return tid;
}
TypeId

LrWpanTestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void LrWpanTestHeader::SetId (uint32_t id)
{
  m_id = id;
}

uint32_t LrWpanTestHeader::GetId (void) const
{
  return m_id;
}
Time LrWpanTestHeader::GetTs (void) const
{
  return TimeStep(m_ts);
}

void
LrWpanTestHeader::Print (std::ostream &os) const
{
  os << " ID = " << m_id << "TimeStemp = " << m_ts;
}

uint32_t
LrWpanTestHeader::GetSerializedSize (void) const
{
  return sizeof(m_id) + sizeof(m_ts);
}

void
LrWpanTestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU32(m_id);
  i.WriteU64(m_ts);
}


uint32_t
LrWpanTestHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  m_id = i.ReadU32();
  m_ts = i.ReadU64();
  return i.GetDistanceFrom (start);
}

};
/**
 * LrWpanTestHeader source end
 *********************************************************/