/* -*-	Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Orange Labs Network
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307	 USA
 *
 * Author: William DIEGO <william.diego@orange.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/youtube-module.h"

NS_LOG_COMPONENT_DEFINE ("YoutubeTest");

using namespace ns3;

int main (int argc, char *argv[])
{
  //
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //
	SeedManager::SetRun (3);
	 Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue(1024*8000));//(1024*3000));
	 Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(1024*8000));//(1024*3000));


	// Config::SetDefault ("ns3::PacketSocket::RcvBufSize", UintegerValue(1024*8000));
	// Config::SetDefault ("ns3::PacketSocketClient::MaxPackets", UintegerValue(1000));
	 // Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue(1460));
	 //Config::SetDefault ("ns3::NodeListPriv/NodeList/*/$ns3::Node/$ns3::Ns3NscStack<linux2.6.26>/net.ipv4.tcp_fin_timeout", UintegerValue(200));
	 //Config::Set ("/NodeList/*/$ns3::Ns3NscStack<linux2.6.26>/net.ipv4.tcp_fin_timeout", StringValue ("200"));
	 
	 //TcpSocket::SetSndBufSize=UintegerValue(1024*700*100);
#if 0
  LogComponentEnable ("YoutubeClient", LOG_LEVEL_ALL);
  LogComponentEnable ("YoutubeServer", LOG_LEVEL_ALL);
  LogComponentEnable ("YoutubeController", LOG_LEVEL_ALL);
#endif
  //LogComponentEnable ("BulkSendApplication", LOG_LEVEL_ALL);


  LogComponentEnable ("YoutubeTest", LOG_LEVEL_ALL);

  uint32_t totalTime = 60*10;
  uint32_t dataStart = 5;
  uint32_t nNodes = 5;
  std::string delay = "5ms";
  std::string dataRate0 = "200Kbps";
  std::string dataRate1 = "1Mbps";
  std::string dataRate2 = "1Mbps";
  std::string dataRate3 = "1Mbps";
  // Allow users to override the default parameters and set it to new ones from CommandLine.
  CommandLine cmd;
  /*
   * The parameter for the p2p link
   */
  cmd.AddValue ("DataRate", "The data rate for the link", dataRate1);
  cmd.AddValue ("Delay", "The delay for the link", delay);
  cmd.Parse (argc, argv);

  SeedManager::SetSeed (3);
  SeedManager::SetRun (1);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (nNodes);
  NodeContainer clients;

  Ptr<Node> server = nodes.Get (0);
  Ptr<Node> swt = nodes.Get (1);
  Ptr<Node> Client_1 = nodes.Get (2);
  Ptr<Node> Client_2 = nodes.Get (3);
  Ptr<Node> Client_3 = nodes.Get (4);
clients.Add(Client_1);
clients.Add(Client_2);
clients.Add(Client_3);

  NS_LOG_INFO ("Create channel 0");
  PointToPointHelper PP;
  PP.SetDeviceAttribute ("DataRate", StringValue (dataRate0));
  PP.SetChannelAttribute ("Delay", StringValue (delay));
  NetDeviceContainer s_s =  PP.Install(server,swt);

  NS_LOG_INFO ("Create channel 1");
  PointToPointHelper PP1;
  PP1.SetDeviceAttribute ("DataRate", StringValue (dataRate1));
  PP1.SetChannelAttribute ("Delay", StringValue (delay));
  NetDeviceContainer s_c1 =  PP1.Install(swt,Client_1);

  NS_LOG_INFO ("Create channel 2");
  PointToPointHelper PP2;
  PP2.SetDeviceAttribute ("DataRate", StringValue (dataRate2));
  PP2.SetChannelAttribute ("Delay", StringValue (delay));
  NetDeviceContainer s_c2 =  PP2.Install(swt,Client_2);

  NS_LOG_INFO ("Create channel 3");
  PointToPointHelper PP3;
  PP3.SetDeviceAttribute ("DataRate", StringValue (dataRate3));
  PP3.SetChannelAttribute ("Delay", StringValue (delay));
  NetDeviceContainer s_c3 =  PP3.Install(swt,Client_3);

  NS_LOG_INFO ("Install Internet");
///==============
  InternetStackHelper internet;
  internet.Install (swt);
  internet.SetTcp ("ns3::NscTcpL4Protocol","Library",StringValue ("liblinux2.6.26.so"));
  internet.Install (server);
  internet.Install (Client_1);
  internet.Install (Client_2);
  internet.Install (Client_3);

  NS_LOG_INFO ("Setting the address 0");
  Ipv4AddressHelper address0;
  address0.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces0;
  interfaces0 = address0.Assign (s_s);

  NS_LOG_INFO ("Setting the address 1");
  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1;
  interfaces1 = address1.Assign (s_c1);

  NS_LOG_INFO ("Setting the address 2");
  Ipv4AddressHelper address2;
  address2.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2;
  interfaces2 = address2.Assign (s_c2);

  NS_LOG_INFO ("Setting the address 3");
  Ipv4AddressHelper address3;
  address3.SetBase ("10.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces3;
  interfaces3 = address3.Assign (s_c3);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
//Config::Set ("/NodeList/*/$ns3::Ns3NscStack<linux2.6.26>/net.ipv4.tcp_fin_timeout", StringValue ("200"));
 // double randomStartTime = (1.0 / 4) / (nNodes);





  uint16_t port = 80;
   //uint32_t count = 0;
  YoutubeHelper youtubeHelper1;
  YoutubeHelper youtubeHelper2;
  YoutubeHelper youtubeHelper3;
  //Configuring HTTP

//====================================
/// Server 1
//====================================
  	  YoutubeServerHelper youtubeServer1;
      NS_LOG_LOGIC ("Install app in server");
      youtubeServer1.SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), port)));
      youtubeServer1.SetAttribute ("YoutubeController", PointerValue (youtubeHelper1.GetController ()));
      //youtubeServer1.SetAttribute ("Persistent",BooleanValue (true));
	  ApplicationContainer serverApps1 = youtubeServer1.Install (server);


//====================================
/// Lien 1
//====================================
      NS_LOG_LOGIC ("Install app in Client 1");
      YoutubeClientHelper youtubeClient1;
      youtubeClient1.SetAttribute("Persistent",BooleanValue (true));
      youtubeClient1.SetAttribute("UserID",UintegerValue (1));
      youtubeClient1.SetAttribute("User",AddressValue (InetSocketAddress (interfaces1.GetAddress(1), port*10)));
      youtubeClient1.SetAttribute("UserRequestSize",UintegerValue (200));
      youtubeClient1.SetAttribute("UserServerDelay",DoubleValue (0.1));
      youtubeClient1.SetAttribute("Timeout",UintegerValue (100));
	  youtubeClient1.SetAttribute("VideoSize",DoubleValue (60*2.5));
      //=====================================================
	  youtubeClient1.SetAttribute ("Server", AddressValue (InetSocketAddress (interfaces0.GetAddress(0), port)));
	  youtubeClient1.SetAttribute ("YoutubeController", PointerValue (youtubeHelper1.GetController ()));
      ApplicationContainer clientApps1 = youtubeClient1.Install (Client_1);

     // UniformVariable var;
      serverApps1.Start (Seconds (0.0));
      serverApps1.Stop (Seconds (totalTime));
      clientApps1.Start (Seconds (dataStart));
      clientApps1.Stop (Seconds (totalTime));
      //httpHelper1.GetController()->SetAttribute("SegmentSize",UintegerValue (2048*5));

//====================================
/// Server 2
//====================================
    //====================================
      /// Server 1
      //====================================
           YoutubeServerHelper youtubeServer2;
            NS_LOG_LOGIC ("Install app in server");
            youtubeServer2.SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), (port+1))));
            youtubeServer2.SetAttribute ("YoutubeController", PointerValue (youtubeHelper2.GetController ()));
			//httpServer2.SetAttribute ("Persistent",BooleanValue (true));
            ApplicationContainer serverApps2 = youtubeServer2.Install (server);


////====================================
///// Lien 2
////====================================
      NS_LOG_LOGIC ("Install app in Client 2");
      YoutubeClientHelper youtubeClient2;
      youtubeClient2.SetAttribute("Persistent",BooleanValue (true));
      youtubeClient2.SetAttribute("UserID",UintegerValue (2));
      youtubeClient2.SetAttribute("User",AddressValue (InetSocketAddress (interfaces2.GetAddress(1), port*10)));
      youtubeClient2.SetAttribute("UserRequestSize",UintegerValue (200));
      youtubeClient2.SetAttribute("UserServerDelay",DoubleValue (0.1));
      youtubeClient1.SetAttribute("Timeout",UintegerValue (100));
	  youtubeClient2.SetAttribute("VideoSize",DoubleValue (60*3));
      youtubeClient2.SetAttribute ("Server", AddressValue (InetSocketAddress (interfaces0.GetAddress(0), port+1)));
      youtubeClient2.SetAttribute ("YoutubeController", PointerValue (youtubeHelper2.GetController ()));
      ApplicationContainer clientApps2 = youtubeClient2.Install (Client_2);
      //UniformVariable var;
      serverApps2.Start (Seconds (0.0));
      serverApps2.Stop (Seconds (totalTime));
      clientApps2.Start (Seconds (dataStart));
      clientApps2.Stop (Seconds (totalTime));
      //youtubeHelper1.GetController()->SetAttribute("SegmentSize",UintegerValue (2048*5));


      //====================================
      /// Server 3
      //====================================
          //====================================
            /// Server 3
            //====================================
                 YoutubeServerHelper youtubeServer3;
                  NS_LOG_LOGIC ("Install app in server");
                  youtubeServer3.SetAttribute ("Local", AddressValue (InetSocketAddress (interfaces2.GetAddress(3), (port+2))));
                  youtubeServer3.SetAttribute ("YoutubeController", PointerValue (youtubeHelper3.GetController ()));
      			//youtubeServer2.SetAttribute ("Persistent",BooleanValue (true));
                  ApplicationContainer serverApps3 = youtubeServer3.Install (server);


      ////====================================
      ///// Lien 3
      ////====================================
            NS_LOG_LOGIC ("Install app in Client 3");
            YoutubeClientHelper youtubeClient3;
            youtubeClient3.SetAttribute("Persistent",BooleanValue (true));
            youtubeClient3.SetAttribute("UserID",UintegerValue (3));
            youtubeClient3.SetAttribute("User",AddressValue (InetSocketAddress (Ipv4Address::GetAny (), port*10)));
            youtubeClient3.SetAttribute("UserRequestSize",UintegerValue (200));
            youtubeClient3.SetAttribute("UserServerDelay",DoubleValue (0.1));
            youtubeClient1.SetAttribute("Timeout",UintegerValue (100));
            youtubeClient3.SetAttribute("VideoSize",DoubleValue (60*3));
            youtubeClient3.SetAttribute ("Server", AddressValue (InetSocketAddress (interfaces0.GetAddress(0), port+2)));
            youtubeClient3.SetAttribute ("YoutubeController", PointerValue (youtubeHelper3.GetController ()));
            ApplicationContainer clientApps3 = youtubeClient3.Install (Client_3);
            //UniformVariable var;
            serverApps3.Start (Seconds (0.0));
            serverApps3.Stop (Seconds (totalTime));
            clientApps3.Start (Seconds (dataStart));
            clientApps3.Stop (Seconds (totalTime));
            //youtubeHelper1.GetController()->SetAttribute("SegmentSize",UintegerValue (2048*5));


      ApplicationContainer clientApps;
      ApplicationContainer serverApps;

      PacketSinkHelper app3_dlPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 21));
      serverApps.Add (app3_dlPacketSinkHelper.Install (Client_3));
      /*
      	  	  	  	  	  	  	 OnOffHelper app3_dlClient("ns3::TcpSocketFactory",InetSocketAddress(interfaces3.GetAddress(1), 21)) ; // precise the protocol by which the traffic is transported and its destination port address
          						app3_dlClient.SetAttribute("PacketSize",UintegerValue(1440)) ; // the packet size in byte
          						app3_dlClient.SetAttribute("DataRate",DataRateValue(DataRate(1000000))) ; // the data rate in bits per second
          						app3_dlClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=20]")) ;  // the on time is a random variable with mean of 0.352
          						app3_dlClient.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=10]")) ;  // the off time is a random variable with mean of 0.65
          						clientApps.Add (app3_dlClient.Install (server));

            					serverApps.Start(Seconds(1));
            					clientApps.Start(Seconds(1));
            					serverApps.Stop(Seconds(totalTime));
            					clientApps.Stop(Seconds(totalTime));
*/



     BulkSendHelper app3_dlClient("ns3::TcpSocketFactory",InetSocketAddress(interfaces3.GetAddress(1),21)) ;
     // Set the amount of data to send in bytes.  Zero is unlimited.
     //app3_dlClient.SetAttribute ("MaxBytes", UintegerValue (1000000));
     app3_dlClient.SetAttribute ("Random", BooleanValue (true));
     app3_dlClient.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=10]")) ;
     app3_dlClient.SetAttribute("FileSize", StringValue("ns3::UniformRandomVariable[Min=1|Max=5]"));
     clientApps.Add (app3_dlClient.Install (server));

     	 	 	 	 	 	 	 	serverApps.Start(Seconds (dataStart));
                 					clientApps.Start(Seconds (dataStart));
                 					serverApps.Stop(Seconds(totalTime));
                 					clientApps.Stop(Seconds(totalTime));

  NS_LOG_INFO ("Run Simulation.");



  PP.EnablePcapAll("Youtube");
  //PP2.EnablePcapAll("Html2");


  // Output config store to txt format
 	Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("Youtube-attributes.txt"));
 	Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
 	Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
 	ConfigStore outputConfig;
 	outputConfig.ConfigureDefaults ();
 	outputConfig.ConfigureAttributes ();

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
