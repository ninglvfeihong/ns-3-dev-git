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


#include <memory>
using namespace ns3;

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
  PlaceSpectrum(const ns3::Ptr<ns3::SpectrumChannel> & channel, const Vector & position)
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
  spectrumAnalyzerHelper.SetPhyAttribute ("Resolution", TimeValue (ns3::MicroSeconds (500)));
  spectrumAnalyzerHelper.SetPhyAttribute ("NoisePowerSpectralDensity", DoubleValue (1e-18));  // -150 dBm/Hz -90dBm/Mhz
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
void
LrWpanSendScheduleAn(ns3::Ptr<ns3::NetDevice> senderDev,
                  const ns3::Time &start, const ns3::Time &end, const ns3::Time &interval)
{
  ns3::Time i;
  uint32_t j=0;
  ns3::McpsAnRequestParams params;
  params.m_GpExpire = 30*1000/16;
  params.m_SPF = 40;
  for(i= start;  i<end; i += interval){
    ns3::Ptr<ns3::LrWpanMac> mac = senderDev->GetObject<LrWpanNetDevice>()->GetMac();
    Simulator::Schedule(i,&LrWpanMac::McpsAnRequest,mac,params); //broadcast without mac
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
  //xiao::LrWpanSendScheduleBroadcast(sender, MilliSeconds(150),MilliSeconds(200),MilliSeconds(1));
  xiao::LrWpanSendScheduleBroadcast(recver, MilliSeconds(150),MilliSeconds(200),MilliSeconds(1));
  //xiao::LrWpanSendScheduleBroadcastRandom(sender, MilliSeconds(150),MilliSeconds(300),MilliSeconds(10));
  //xiao::LrWpanSendSchedule(sender,recver, MilliSeconds(150),MilliSeconds(300),MilliSeconds(2));
 
  //std::cout << recver->GetDevice(0)->GetAddress() << " -- " << sender->GetDevice(0)->GetAddress() << std::endl;
  lrWpanHelper.EnablePcapAll("lrpwan_test",true);
  //xiao_helper.EnableLrWpanShowMacTraceRxDrop(recver->GetDevice(0));
  xiao_helper.EnableLrWpanShowPhyTraceRxDrop(recver->GetDevice(0));

  //schedule AN
  xiao::LrWpanSendScheduleAn(lrDevices.Get(0), MilliSeconds(150),MilliSeconds(500),MilliSeconds(60));


  ns3::Ptr<ns3::SpectrumChannel> channel = lrWpanHelper.GetChannel();



  Simulator::Stop (MilliSeconds (500));
  xiao_helper.PlaceSpectrum(channel,Vector(5,1,0));
  //xiao_helper.ConfigStorShow();
  xiao_helper.makeAnim();
  Simulator::Run ();
  Simulator::Destroy ();
}
