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


#include <ns3/lr-wpan-module.h>

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
};


//typedef Callback< bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address & > ReceiveCallback;
bool
NetDevCb(Ptr<NetDevice> netDev, Ptr<const Packet> p, uint16_t, const Address & addr)
{
  NS_LOG_UNCOND("I received a packet!");
  p->Print (std::cout);
  std::cout << std::endl;
  return true;
}

};


int 
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("My first hello word!");
  xiao::Helper xiao_helper;

  Packet::EnablePrinting ();
  Packet::EnableChecking();

  ns3::NodeContainer lrPandNodes;
  lrPandNodes.Create(2);

  LrWpanHelper lrWpanHelper(false);
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
  lrWpanHelper.Install(lrPandNodes);
  //lrWpanHelper.AssociateToPan()

  //set Netdevice receiver
  recver->GetDevice(0)->SetReceiveCallback(MakeCallback(&xiao::NetDevCb));

  Ptr<Packet> p = Create<Packet>(20); //20 byte packet
  Address addr(recver->GetDevice(0)->GetAddress());
  Simulator::Schedule(Seconds(1),&NetDevice::Send,
                      sender->GetDevice(0),
                      p,
                      addr,0);
  p = Create<Packet>(20);
  Simulator::Schedule(Seconds(2),&NetDevice::Send,
                      sender->GetDevice(0),
                      p,
                      addr,0);
  std::cout << recver->GetDevice(0)->GetAddress() << " -- " << sender->GetDevice(0)->GetAddress() << std::endl;
  lrWpanHelper.EnablePcapAll("lrpwan_test",true);


  //xiao_helper.ConfigStorShow();
  xiao_helper.makeAnim();
  Simulator::Run ();
  Simulator::Destroy ();
}
