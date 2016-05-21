/* -*-	Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Orange Labs Network
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


//COMMAND: ./waf --run "scratch/LTE_IP_AWARE_A --SimTime=39 --TCP=1 --VoipBearer=0 --VideoRate=2Mbps --FTPRate=15Mbps  --IpAware=1 --CoreDataRate=10Gbps"
// nohup ./waf --run "scratch/LTE_random --SimTime=30 --TCP=1 --VoipBearer=0 --VideoRate=1Mbps --FTPRate=5Mbps  --IpAware=1 --CoreDataRate=10Gbps --numberOfUE=10"
//nohup ./waf --run "LTE_random --SimTime=10 --VideoSize=500000 --runt=1 --Scenario=1 --FTPRate=1Mbps  --CoreDataRate=1000Gbps --numberOfUE=3 --run=Hola --nRB=6"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <ns3/log.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <set>

#include "ns3/random-variable.h"
#include <stdlib.h>
#include <algorithm>


#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/error-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/enum.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/object-factory.h"
#include "ns3/srtcm.h"
#include "ns3/trtcm.h"
#include "ns3/wred.h"
#include "ns3/packet-sink.h"
// Bulding library


#include "ns3/diff-serv-queue.h"
#include "ns3/srtcm.h"
#include "ns3/trtcm.h"
#include "ns3/wred.h"
#include "ns3/token-bucket.h"
#include "ns3/diff-serv-sla.h"
#include "ns3/diff-serv-flow.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"

#include "ns3/youtube-module.h"
#include "ns3/http-module.h"

//#include "ns3/simulator-module.h"


/*
#include "ns3/buildings-mobility-model.h"
#include "ns3/buildings-propagation-loss-model.h"
#include "ns3/building.h"
#include "ns3/buildings-helper.h"
*/
#include "ns3/propagation-environment.h"


//NETANIM
#include "ns3/netanim-module.h"

using namespace ns3;
using namespace std;
/**
 * Simulation script for LTE/EPC IP-Aware - vesion 1.0:
 */
NS_LOG_COMPONENT_DEFINE ("LTE_IP-Aware");


/*
static void
CwndTracer (Ptr<OutputStreamWrapper>stream, uint32_t oldval, uint32_t newval)
{
  *stream->GetStream () << oldval << " " << newval << std::endl;
}

static void
TraceCwnd (std::string cwndTrFileName)
{
  AsciiTraceHelper ascii;
  if (cwndTrFileName.compare ("") == 0)
    {
      NS_LOG_DEBUG ("No trace file for cwnd provided");
      return;
    }
  else
    {
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (cwndTrFileName.c_str ());
      Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",MakeBoundCallback (&CwndTracer, stream));
    }
}

*/

//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//*****************				  [Flow Monitor]		   ***********************************
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


void
printStats (FlowMonitor::FlowStats st, std::string fileName, bool m_firstWrite, int UE, uint16_t port)
{

	 ofstream os_lte;
	     std::cout << "	Tx Bytes: " << st.txBytes << std::endl;
		 std::cout << "	 Rx Bytes: " << st.rxBytes << std::endl;
		 std::cout << "	 Tx Packets: " << st.txPackets << std::endl;
		 std::cout << "	 Rx Packets: " << st.rxPackets << std::endl;
		 std::cout << "	 Lost Packets: " << st.lostPackets << std::endl;
		 //std::cout << "  Drop Packets: " <<	st.packetsDropped << std::endl;

	 if (st.rxPackets > 0)
	 {		float m_throug = st.rxBytes * 8.0/ (st.timeLastRxPacket.GetSeconds() - st.timeFirstRxPacket.GetSeconds())/ 1000;
			string ordre = " Kbps";
			if(m_throug>=1000) {m_throug=m_throug/1000; ordre = " Mbps";}

			std::cout << "	 Mean{Throughput}: " << m_throug<< ordre << endl;
			 std::cout << "	 Mean{Delay}: " << (st.delaySum.GetSeconds() / st.rxPackets*1000)<< "ms" << std::endl;
			 std::cout << "	 Mean{Jitter}: " << (st.jitterSum.GetSeconds() /(st.rxPackets-1)*1000)<< " ms" << std::endl;
			 //std::cout << "  Mean{Hop Count}: " << st.timesForwarded / st.rxPackets + 1 << std::endl;

	 }


	 if (false)
	 {
		 std::cout << "Delay Histogram" << std::endl;
		 for (uint32_t i=0; i<st.delayHistogram.GetNBins (); i++)
		if (st.delayHistogram.GetBinCount (i)>0){ std::cout << " " << i << "(" << st.delayHistogram.GetBinStart (i) << "-" << st.delayHistogram.GetBinEnd (i) << "): " << st.delayHistogram.GetBinCount (i) << std::endl;}
		 std::cout << "Jitter Histogram" << std::endl;
		 for (uint32_t i=0; i<st.jitterHistogram.GetNBins (); i++ )
		if (st.delayHistogram.GetBinCount (i)>0){ std::cout << " " << i << "(" << st.jitterHistogram.GetBinStart (i) << "-" << st.jitterHistogram.GetBinEnd (i) << "): " << st.jitterHistogram.GetBinCount (i) << std::endl;}
				//std::cout << "PacketSize Histogram	"<< std::endl;
		 //for (uint32_t i=0; i<st.packetSizeHistogram.GetNBins (); i++ )
						// std::cout << " " << i << "(" << st.packetSizeHistogram.GetBinStart (i) << "-" << st.packetSizeHistogram.GetBinEnd (i) << "): " << st.packetSizeHistogram.GetBinCount (i) << std::endl;
	 }

	 for (uint32_t i=0; i<st.packetsDropped.size (); i++)
		{ if (st.packetsDropped[i]!=0)
		std::cout << "	Packets dropped by reason " << i << ": " << st.packetsDropped[i] << std::endl;
		}
	 for (uint64_t i=0; i<st.bytesDropped.size(); i++)
		{ if (st.bytesDropped[i]!=0)
		std::cout << "	|-->Bytes dropped by reason " << i << ": " << st.bytesDropped[i] << std::endl;
		}


             if(port >= 4300 && port >= 4350){

		if (m_firstWrite == true)
		    {
			os_lte.open (fileName.c_str ());
			m_firstWrite = false;
			os_lte << "% Start\tTxBytes\tTxTime\tThBPS\tPacketLoss\tUE\tDPort";
			os_lte << std::endl;

		    }
		else
		{
			os_lte.open (fileName.c_str (), std::ios_base::app);
		}

		if (!os_lte.is_open ())
		      {
		        NS_LOG_ERROR ("Can't open file " << fileName.c_str ());
		        return;
		      }
		else
		{
			os_lte <<st.timeFirstRxPacket.GetSeconds()<< "\t";
			os_lte <<st.txBytes<< "\t";
			os_lte <<(st.timeLastRxPacket.GetSeconds() - st.timeFirstRxPacket.GetSeconds())<< "\t";
			os_lte <<(st.rxBytes * 8.0/ (st.timeLastRxPacket.GetSeconds() - st.timeFirstRxPacket.GetSeconds()))<< "\t";
			os_lte <<st.lostPackets<< "\t";
			os_lte <<(UE-1)<< "\t";
			os_lte <<port ;
			os_lte << std::endl;
			os_lte.close ();
		}
               }

}


/*
static void
VoIP_GBR (Ptr<LteHelper>& lteHelper, Ptr<NetDevice>& UE)
{

	  Ptr<EpcTft> tft_voip = Create<EpcTft> ();
	  EpcTft::PacketFilter pf_voip;
	  pf_voip.localPortStart = 1234;
	  pf_voip.localPortEnd = 1234;
	  tft_voip->Add (pf_voip);
	  GbrQosInformation qos_voip;
	  qos_voip.gbrDl = 80000; // Downlink GBR
	  qos_voip.gbrUl = 80000; // Uplink GBR
	  qos_voip.mbrDl = qos_voip.gbrDl; // Downlink MBR
	  qos_voip.mbrUl = qos_voip.gbrUl; // Uplink MBR
	  EpsBearer bearer_voip (EpsBearer::GBR_CONV_VOICE, qos_voip);
	  lteHelper->ActivateDedicatedEpsBearer (UE, bearer_voip, tft_voip);

}

static void
Video_GBR (Ptr<LteHelper>& lteHelper, Ptr<NetDevice>& UE)

{

	  Ptr<EpcTft> tft_video = Create<EpcTft> ();
		  EpcTft::PacketFilter pf_video;
		  pf_video.localPortStart = 4321;
		  pf_video.localPortEnd = 4321;
		  tft_video->Add (pf_video);
	  GbrQosInformation qos_video;
	  qos_video.gbrDl = 1200000; // Downlink GBR
	  qos_video.gbrUl = 1200000; // Uplink GBR
	  qos_video.mbrDl = qos_video.gbrDl; // Downlink MBR
	  qos_video.mbrUl = qos_video.gbrUl; // Uplink MBR
	  EpsBearer bearer_video (EpsBearer::GBR_CONV_VIDEO, qos_video);
	  lteHelper->ActivateDedicatedEpsBearer (UE, bearer_video, tft_video);

}
*/
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>



int
main (int argc, char *argv[])
{

	 bool  m_firstWrite = true;

   // cout<<"<===== HERE !!";
  uint16_t numberOfeNB = 1;
  uint16_t numberOfUE = 15;
  uint16_t nRB = 100;
  double simTime = 10;
  //double interPacketInterval = 100;

  int pkt_voip_t=44; // G.722.2 + RTP (32 Bytes + 12 Bytes) --> VoLTE
  int pkt_video_t=1440;//172;//
  string voip_rate("44000bps");
  string video_rate("4000bps");
  string FTP_rate("58000bps");
  bool enableFlowMonitor = true; //	 false;//
  //bool Plot = true;
  int bearer = 0;
  bool tcp=true;
  bool IpAware=false;
  DataRate core_Rate("10Gb/s");

  uint32_t maxBytes = 10000000; // 5MB
  int Q_AF = 150000000;
  int Q_EF = 150000000;
  ////////////////// MARKING DSCP  /////////////////////////////////////
  enum{drop = 256 };
  enum{ AF11 = 40, AF12 = 48, AF13 = 56, AF21 = 72, AF22 = 80, AF23 = 88, AF31 = 104, AF32 = 112, AF33 = 120, AF41 = 136, AF42 = 144, AF43 = 152, EF = 184, BE = 1};

  std::string pcapFileNamePrefix = "LTE-DropTail";
  std::string cwndTrFileName = "LTE-DropTail.tr";


  int scenario = 1;
   string runID;
   string Scenario = "BE";
   {
     stringstream sstr;
     sstr << "Run-" << time (NULL);
     runID = sstr.str ();
   }


   int runt = 1;
#if 0
   LogComponentEnable ("CoDelQueue", LOG_LEVEL_ALL);
   LogComponentEnable ("IpAwareQueue", LOG_LEVEL_ALL);
   LogComponentEnable ("LteRlcUmIpAware", LOG_LEVEL_ALL);

  LogComponentEnable ("YoutubeClient", LOG_LEVEL_ALL);
  LogComponentEnable ("YoutubeServer", LOG_LEVEL_ALL);
  LogComponentEnable ("YoutubeController", LOG_LEVEL_ALL);
#endif


 // Command line arguments
   CommandLine cmd;


   cmd.AddValue("numberOfUE", "Number of UE (10)", numberOfUE);
   cmd.AddValue("SimTime", "Total duration of the simulation [s] ())", simTime);
   cmd.AddValue("VideoSize","Set Video size (Bytes)",maxBytes);
   cmd.AddValue("FTPRate","Set Video rate (1Mbps) ",FTP_rate);
   cmd.AddValue("Scenario","(1): IP-Aware (2):  BE  (3): CQA  (4): PSS ",scenario);
   cmd.AddValue("CoreDataRate"," Core Data Rate (1Gbps)",core_Rate);
   cmd.AddValue ("run", "Identifier for run.", runID);
   cmd.AddValue("runt","Run time",runt);
   cmd.AddValue("nRB","RB number: 100, 75, 50, 25, 15, 6",nRB);

   cmd.Parse(argc, argv);




   switch (scenario)
 	{
 		  case 1:  {IpAware=true;
  	  	   	   	   bearer=false;
  	  	   	   	   Scenario = "IP-Aware";
  	  	   	   	   break;}
 		  case 2:  {IpAware=false;
  	  	   	   	   bearer=false;
  	  	   	   	   Scenario = "BE";
  	  	   	   	   break;}
 		  case 3:  {IpAware=false;
 		  	  	   bearer=true;
 		  	  	   Scenario = "CQA";
 		  	  	   break;}
 		  case 4:  {IpAware=false;
 		  	  	   bearer=true;
 		  	  	   Scenario = "PSS";
 		  	  	   break;}

 	}


 SeedManager::SetRun (runt);
 //srand(time(NULL)) ;

  //MCS Methode: [Piro2011] based on the GSoC module  and works per RB basis
  	  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  	  //Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));
  	Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration",TimeValue (Seconds (1)));
  	Config::SetDefault ("ns3::BulkSendApplication::MaxBytes",UintegerValue(maxBytes));
  	Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (80));  // Increase UEs capacity in simulation
  	Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
	 Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue(1024*8000));//(1024*3000));
	 Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(1024*8000));//(1024*3000));

	 //========================================
	 //CODEL and Drop Tail Queue Configuration
	 //========================================
	  // Queue defaults
	 uint32_t queueSize = 1000;              // in packets
	  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (queueSize));
	  Config::SetDefault ("ns3::CoDelQueue::MaxPackets", UintegerValue (queueSize));
	  Config::SetDefault ("ns3::DropTailQueue::Mode", StringValue ("QUEUE_MODE_PACKETS"));
	  Config::SetDefault ("ns3::CoDelQueue::Mode", StringValue ("QUEUE_MODE_PACKETS"));


  // Some others atributes
  /* eNodeB's transmit power, in Watts.
  	  Recommended by TS.36.814 are:
  	  43 dBm for 1.25, 5 MHz carrier
  	  46/49 dBm for 10, 20 MHz carrier
  	*/
  	//Config::SetDefault ("ns3::LteHelper::Scheduler" "ns3::PfFfMacScheduler")
  	Config::SetDefault ("ns3::LteHelper::PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));
  	Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46));
  	Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (5)); // Receiver noise figure in dB
  	Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (24)); //==> dBm iPhone 5
  	Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (9));  // Receiver noise figure in dB


    //	+------------------------+
       //	| RAN elements creation: |
       //	+------------------------+

       NodeContainer ueNodes;
       NodeContainer enbNodes;
       ueNodes.Create(numberOfUE);
       enbNodes.Create(numberOfeNB);
       // Creation of LTE and EPC helper
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> (); // Declaration for ns 3.18
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));
  //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  epcHelper->SetAttribute("S1uLinkDataRate",DataRateValue (core_Rate));
  //lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  if(bearer==1)
  {
	  if(scenario==2) { lteHelper->SetSchedulerType ("ns3::CqaFfMacScheduler");
		  	  	  	    lteHelper->SetSchedulerAttribute ("CqaMetric",StringValue("CqaPf")); }
	  if(scenario==3) { lteHelper->SetSchedulerType("ns3::PssFfMacScheduler");
	  	  	 	 	 	lteHelper->SetSchedulerAttribute("nMux", UintegerValue(10)); } // the maximum number of UE selected by TD scheduler

  }
  if(IpAware){

	  lteHelper->SetAttribute ("RlcMode",EnumValue(LteHelper::RLC_UM_IP_AWARE));

	  Config::SetDefault ("ns3::LteHelper::RlcMode",EnumValue(LteHelper::RLC_UM_IP_AWARE));
	  std::cout << "************** IP-AWARE *****************" <<std::endl;
	  //***************
	  	  	 // lteHelper->SetSchedulerType("ns3::PssFfMacScheduler");
			  //lteHelper->SetSchedulerAttribute("nMux", UintegerValue(2)); // the maximum number of UE selected by TD scheduler
			 // lteHelper->SetSchedulerAttribute("PssFdSchedulerType", StringValue("CoItA")); // PF scheduler type in PSS, default value "PFsch"
	  //***************

  }


  lteHelper->SetSchedulerAttribute ("CqiTimerThreshold", UintegerValue (1)); //compute CQI each TTI (1)

  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

//if(RlcMode==true){Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping",EnumValue(LteHelper::RLC_AM_ALWAYS));}

  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (100 + 18000));
  // Number of RB configuration ==> 100 RB UL/DL (INFO: Only Resource allocation type 0 is available -  RBG =)
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (nRB));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (nRB));

  lteHelper->SetEnbAntennaModelType("ns3::IsotropicAntennaModel");
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


  //***************
  	  	 // lteHelper->SetSchedulerType("ns3::PssFfMacScheduler");
		  //lteHelper->SetSchedulerAttribute("nMux", UintegerValue(2)); // the maximum number of UE selected by TD scheduler
		 // lteHelper->SetSchedulerAttribute("PssFdSchedulerType", StringValue("CoItA")); // PF scheduler type in PSS, default value "PFsch"
  //***************

		//


  // Number of RB configuration ==> 100 RB UL/DL (INFO: Only Resource allocation type 0 is available -  RBG =)




  lteHelper->SetFadingModel("ns3::TraceFadingLossModel");

  /// Excerpt of the fading trace included in the simulator for an urban scenario (speed of 3 kmph). >>>> urbanETU
  //lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
  /// Excerpt of the fading trace included in the simulator for a pedestrian scenario (speed of 3 kmph). >>>> pedestrianEPA
  lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (simTime)));
  lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
  lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.001)));
  lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (nRB));

  //lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  //lteHelper->SetAttribute ("RlcMode",EnumValue(LteHelper::RLC_UM_ALWAYS));




  //
/*
  // PF configuration
  lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("CqiTimerThreshold", UintegerValue (1)); //compute CQI each TTI (1)
*/

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);


  DataRateValue voip_rate_ns3  = DataRate (voip_rate);
  DataRateValue video_rate_ns3 = DataRate (video_rate);
  DataRateValue ftp_rate_ns3 = DataRate (FTP_rate);

  UintegerValue pkt_voip=pkt_voip_t;
  UintegerValue pkt_video=pkt_video_t;



  //	+------------------------+
   //	|declaration PGW on EPC: |
   //	+------------------------+

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create two RemoteHosts (SERVER)
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (2);
  Ptr<Node> server_1 = remoteHostContainer.Get (0);
  Ptr<Node> server_2 = remoteHostContainer.Get (1);




  InternetStackHelper internet;

  internet.Install (remoteHostContainer.Get (0));

  if(tcp==true){  internet.SetTcp ("ns3::NscTcpL4Protocol","Library",StringValue ("liblinux2.6.26.so"));}
  internet.Install (remoteHostContainer.Get (1));
  // Install the IP stack on the UEs
  internet.Install (ueNodes);

   //	+------------------------+
    //	|      S1 <-> PGW :      |
    //	+------------------------+
    ///

  PointToPointHelper PP1;
  Ptr<DiffServQueue> q1 = CreateObject<DiffServQueue> ();
  Ptr<DropTailQueue> q2 = CreateObject<DropTailQueue> ();

  PP1.SetDeviceAttribute ("DataRate", DataRateValue (core_Rate));
  //PP1.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  PP1.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));
  NetDeviceContainer s1_pgw =  PP1.Install(pgw,server_1);


  //dev_s1_t  =  PP1.Install(pgw,"PGW");
 // Ptr<PointToPointNetDevice> dev_s1 = StaticCast<PointToPointNetDevice> (dev_s1_t);

  //Ptr<NetDevice> dev_s2_t = CreateObject<PointToPointNetDevice> ();

 // dev_s2_t  = PP1.Install ("Server1", server_1);
  Ptr<PointToPointNetDevice> dev_pgw1 = s1_pgw.Get(0)->GetObject<PointToPointNetDevice> ();
  Names::Add ("PGW/Server_1", dev_pgw1);
  Ptr<PointToPointNetDevice> dev_s1 = s1_pgw.Get(1)->GetObject<PointToPointNetDevice> ();
  Names::Add ("Server_1/PGW", dev_s1);

   dev_s1->SetQueue(q1);
   dev_pgw1->SetQueue(q2);


   Ipv4AddressHelper address1;
   address1.SetBase("10.1.0.0","255.255.255.0") ;
   Ipv4InterfaceContainer internetIpIfaces1 = address1.Assign (s1_pgw);

//	==================================================================================//
      //	+------------------------+
      //	|      S2 <-> PGW :      |
      //	+------------------------+

   PointToPointHelper PP2;
   Ptr<DiffServQueue> q3 = CreateObject<DiffServQueue> ();
   Ptr<DropTailQueue> q4 = CreateObject<DropTailQueue> ();

   PP2.SetDeviceAttribute ("DataRate", DataRateValue (core_Rate));
   //PP2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
   PP2.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));
   NetDeviceContainer s2_pgw =  PP2.Install(pgw,server_2);


   //dev_s1_t  =  PP1.Install(pgw,"PGW");
  // Ptr<PointToPointNetDevice> dev_s1 = StaticCast<PointToPointNetDevice> (dev_s1_t);

   //Ptr<NetDevice> dev_s2_t = CreateObject<PointToPointNetDevice> ();

  // dev_s2_t  = PP1.Install ("Server1", server_1);

   Ptr<PointToPointNetDevice> dev_pgw2 = s2_pgw.Get(0)->GetObject<PointToPointNetDevice> ();
   Names::Add ("PGW/Server_2", dev_pgw2);
   Ptr<PointToPointNetDevice> dev_s2 = s2_pgw.Get(1)->GetObject<PointToPointNetDevice> ();
   Names::Add ("Server_2/PGW", dev_s2);

    dev_s2->SetQueue(q3);
    dev_pgw2->SetQueue(q4);


    Ipv4AddressHelper address2;
    address2.SetBase("10.2.0.0","255.255.255.0") ;
    Ipv4InterfaceContainer internetIpIfaces2 = address2.Assign (s2_pgw);

//	==================================================================================//
      //	+------------------------+
      //	|     Static routing:    |
      //	+------------------------+
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> s1_tStaticRouting = ipv4RoutingHelper.GetStaticRouting(server_1->GetObject<Ipv4> ());
  s1_tStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  Ptr<Ipv4StaticRouting> s2_tStaticRouting = ipv4RoutingHelper.GetStaticRouting(server_2->GetObject<Ipv4> ());
  s2_tStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

 // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  //	+------------------------+
  //	| Mobility Configuration:|
  //	+------------------------+

  // Position of eNB(s)


  float r_cell= 500;




   // Position of UEs attached to eNB
  /* MobilityHelper uemobility;
   Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();

   // Position of UEs attached to eNB
   ObjectFactory pos;
   pos.SetTypeId ( "ns3::RandomDiscPositionAllocator");
   pos.Set ("X", StringValue ("500.0"));
   pos.Set ("Y", StringValue ("500.0"));
   pos.Set ("Rho", StringValue ("ns3::UniformRandomVariable[Min=30|Max=500]"));

   Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
    uemobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
		   "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"),
		   "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=0.84]"),
		   "PositionAllocator", PointerValue (taPositionAlloc));
   uemobility.SetPositionAllocator (taPositionAlloc);
   uemobility.Install (ueNodes);
*/


    // Position of eNB(s)
     Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
     positionAlloc->Add (Vector (r_cell, r_cell, 0.0));
     MobilityHelper enbMobility;
     enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     enbMobility.SetPositionAllocator (positionAlloc);
     enbMobility.Install (enbNodes);
   AnimationInterface::SetConstantPosition (remoteHostContainer.Get (0), (r_cell - 10), (r_cell + 30));
   AnimationInterface::SetConstantPosition (remoteHostContainer.Get (1), (r_cell + 10), (r_cell + 30));
   AnimationInterface::SetConstantPosition (pgw, r_cell, (r_cell + 10));

   std::string traceFile = "scratch/scenario2.ns_movements";
   Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);


   MobilityHelper ueMobility;
   ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
   ueMobility.Install (ueNodes);

    ns2.Install ();


   // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);


  Ptr<LteEnbNetDevice> lteEnbDev = enbLteDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Names::Add ("eNB/UE", lteEnbDev);


  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));




  // Install and start applications (UDP/TCP) on UEs and remote host
   // uint16_t app1_dlPort = 1234;
   // uint16_t app2_dlPort = 4321;
    uint16_t VoIPPort = 1234;
    //uint16_t VideoPort = 4321;
    //uint16_t HttpPort = 80;
    uint16_t secPort = 4300;

    //		+---------------------------+
    //		|  DIFF_SERV CONFIGURATION  |
    //		+---------------------------+

    // if the packet flow isn't conformant to the SLA, we mark it with a lower priority. But in my case, I will suppose that the flow is always conformant, so the DSCP marking corresponds always to the Initial Code Point.

    ConformanceSpec cSpec1 ;                       // We declare a ConformanceSpec object
    cSpec1.initialCodePoint = EF ;                 // The initial DSCP marking is "Expedited Forwarding"
    cSpec1.nonConformantActionI = AF21 ;           //if the packet is declared by the metering module as "non-conformant level 1", the action to be taken is to mark it with AF21 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafic is beyond the subscriber profil up to a certain point
    cSpec1.nonConformantActionII = AF31 ;          //if the packet is declared by the metering module as "non-conformant level 2", the action to be taken is to mark it with AF31 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafics cross the non-conformant-level 1 point
    MeterSpec mSpec1;                             // We declare a MeterSpec object
    mSpec1.meterID = "SRTCM" ;                     // The metric to be used is "Single Rate Three Color Marker"
    mSpec1.cIR = 600000 ;                          // The committed Information Rate
    mSpec1.cBS = 2000 ;                            // The number of tokens the bucket can hold
    mSpec1.eBS = 30000 ;                           // The number of tokens the excess bucket can hold
    mSpec1.pIR = 0 ;                               // not used for the SRTCM metric
    mSpec1.pBS = 0 ;

    //***** Set up service level agreement for flow 2 (server2 => UEs)  ***********************
    ///*******                        Video with WRR               ***********************
    ///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    ///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    ConformanceSpec cSpec2 ;                       // We declare a ConformanceSpec object
    cSpec2.initialCodePoint = AF13 ;                 // The initial DSCP marking is "Expedited Forwarding"
    cSpec2.nonConformantActionI = AF11 ;           //if the packet is declared by the metering module as "non-conformant level 1", the action to be taken is to mark it with AF21 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafic is beyond the subscriber profil up to a certain point
    cSpec2.nonConformantActionII = BE ;          //if the packet is declared by the metering module as "non-conformant level 2", the action to be taken is to mark it with AF31 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafics cross the non-conformant-level 1 point
    MeterSpec mSpec2;                             // We declare a MeterSpec object
    mSpec2.meterID = "SRTCM" ;                     // The metric to be used is "Single Rate Three Color Marker"
    mSpec2.cIR = 600000 ;                          // The committed Information Rate
    mSpec2.cBS = 2000 ;                            // The number of tokens the bucket can hold
    mSpec2.eBS = 30000 ;                           // The number of tokens the excess bucket can hold
    mSpec2.pIR = 0 ;                               // not used for the SRTCM metric
    mSpec2.pBS = 0 ;
    //***** Set up service level agreement for flow 2 (server2 => UEs)  ***********************
    ///*******                        Video with WRR               ***********************
    ///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    ///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    ConformanceSpec cSpec4 ;                       // We declare a ConformanceSpec object
    cSpec4.initialCodePoint = AF21 ;                 // The initial DSCP marking is "Expedited Forwarding"
    cSpec4.nonConformantActionI = AF11 ;           //if the packet is declared by the metering module as "non-conformant level 1", the action to be taken is to mark it with AF21 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafic is beyond the subscriber profil up to a certain point
    cSpec4.nonConformantActionII = BE ;          //if the packet is declared by the metering module as "non-conformant level 2", the action to be taken is to mark it with AF31 code point. This level of "conformance" is used only by srTCM and trTCM metrics when the trafics cross the non-conformant-level 1 point
    MeterSpec mSpec4;                             // We declare a MeterSpec object
    mSpec4.meterID = "SRTCM" ;                     // The metric to be used is "Single Rate Three Color Marker"
    mSpec4.cIR = 600000 ;                          // The committed Information Rate
    mSpec4.cBS = 2000 ;                            // The number of tokens the bucket can hold
    mSpec4.eBS = 30000 ;                           // The number of tokens the excess bucket can hold
    mSpec4.pIR = 0 ;                               // not used for the SRTCM metric
    mSpec4.pBS = 0 ;
    //***** Set up service level agreement for flow 3 (server2 => UEs)  ***********************
    ///*******                              FTP with WRR               ***********************

    //***** Set up service level agreement for flow 2 (server1 => UEs)  ***********************
    //*******  video or just a simple background CBR, both flows      ***********************
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    ConformanceSpec cSpec3;
    cSpec3.initialCodePoint = BE ;
    cSpec3.nonConformantActionI = drop ;
    cSpec3.nonConformantActionII = drop ;
    MeterSpec mSpec3 ;
    mSpec3.meterID = "SRTCM" ;
    mSpec3.cIR = 600000 ;
    mSpec3.cBS = 2000 ;
    mSpec3.eBS = 30000 ;
    mSpec3.pIR = 0 ;
    mSpec3.pBS = 0 ;

    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    //           *****************   [SLA SET UP]  ***********************
    // we define the conformance specifications and the metering specifications for both SLAs
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    Ptr<DiffServSla> sla1Ptr = CreateObject<DiffServSla> (001, cSpec1, mSpec1) ;
    Ptr<DiffServSla> sla2Ptr = CreateObject<DiffServSla> (002, cSpec2, mSpec2) ;
    Ptr<DiffServSla> sla3Ptr = CreateObject<DiffServSla> (003, cSpec3, mSpec3) ;
    Ptr<DiffServSla> sla4Ptr = CreateObject<DiffServSla> (003, cSpec4, mSpec4) ;
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    //*****************   [FLOW CONFIGURATION]  ***********************
    //  Flows' [source, destination] addresses and [source , destination] ports set up
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

    vector< Ptr<DiffServFlow> > myFlowVector ;
    vector< Ptr< DiffServMeter> > myMeterVector ;
    vector< Ptr< DiffServAQM> > myAQMVector;

    // WRR Configuration
    int AF1Weight = 1 ;
    int AF2Weight = 1 ;
    int AF3Weight = 1 ;
    int AF4Weight = 1 ;
    int BEWeight = 1 ;

    // Queue Size Configuration
    int AF1queue = Q_AF ;
    int AF2queue = Q_AF ;
    int AF3queue = Q_AF ;
    int AF4queue = Q_AF ;
    int BEqueue = Q_AF ;
    int EFqueue = Q_EF ;

    ///<><><><><><><><><><><><><><><><><>
    // DiffServ configuration for each UE:
    ///<><><><><><><><><><><><><><><><><>
    ostringstream adress_string;
    int Nj=100;//numberOfUE;

    for(int i=0; i<numberOfUE;i++)
    {
    adress_string <<"7.0.0."<<i+2;
    //Ptr<DiffServFlow> flow_s1_ue = CreateObject<DiffServFlow> (i*3+1,"10.1.0.2",adress_string.str().c_str(),0,VoIPPort);
    //Ptr<DiffServFlow> flow_s2_ue = CreateObject<DiffServFlow> (i*3+2,"10.2.0.2",adress_string.str().c_str(),0,VideoPort);
    //Ptr<DiffServFlow> flow_s3_ue = CreateObject<DiffServFlow> (i*3+3,"10.2.0.2",adress_string.str().c_str(),0,21);

    Ptr<DiffServFlow> flow_s1_ue = CreateObject<DiffServFlow> (i*Nj+1,"10.1.0.2",adress_string.str().c_str(),0,VoIPPort);
    Ptr<DiffServFlow> flow_s3_ue = CreateObject<DiffServFlow> (i*Nj+2,"10.2.0.2",adress_string.str().c_str(),0,21);

    for(int j =0; j<Nj/2;j++)
    {
    Ptr<DiffServFlow> YoutubeFlow = CreateObject<DiffServFlow> (i*Nj+3+j,"10.2.0.2",adress_string.str().c_str(),(secPort+j),0);
    YoutubeFlow->SetSla(sla2Ptr) ;
    myFlowVector.push_back(YoutubeFlow) ;
    }


    for(int ji = Nj/2; ji<Nj;ji++)
    {
    Ptr<DiffServFlow> WebFlow = CreateObject<DiffServFlow> (i*Nj+3+ji,"10.2.0.2",adress_string.str().c_str(),(secPort+ji),0);
    WebFlow->SetSla(sla4Ptr) ;
    myFlowVector.push_back(WebFlow) ;
    }

    	adress_string.str("");

    // Traffic marking rules
     flow_s1_ue->SetSla(sla1Ptr) ;
     //flow_s2_ue->SetSla(sla2Ptr) ;
     flow_s3_ue->SetSla(sla3Ptr) ;

    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    // Create a DiffServflow vector onto which the different flows are inserted
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    myFlowVector.push_back(flow_s1_ue) ;
    myFlowVector.push_back(flow_s3_ue) ;
    }
    // The "SetDiffServFlows" method is called here to link the flow vector above to all "DiffServQueues" created in the program
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Metering Set up. Here we create a vector containing the metering algorithms that could be used by the edge diff_serv_queues (Token Bucket, SRTCM, TRTCM). If the vector is empty, then no metering can be performed and all the packets are considered as conformant. Should this occurs, all the packets are marked with their inital code point.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // There are two modes for diff_serv queues: edge mode (performing the metering and Behavior Aggregate(BA) classification) and core mode (performing the BA classification only)
    // In our case,
    //we consider that the router does both tasks, it's in edge mode
    q1->SetQueueMode("Edge");
    q3->SetQueueMode("Edge");
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Setting the Round Robin Weights: the EF queue is scheduled with high priority, then the AF21 is scheduled. In this scenario, we do not need to set different weights to the AF queues since they're not used all together, all the queues will have for example a weight of 1.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    q1->SetWRRWeights(AF1Weight, AF2Weight, AF3Weight, AF4Weight, BEWeight) ;
    q3->SetWRRWeights(AF1Weight, AF2Weight, AF3Weight, AF4Weight, BEWeight) ;
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Setting the queues' max size (expressed in terms of packets)
    ///////////////////////////////////////////////////////////////////////////////////////////////
    q1->SetQueueSize(AF1queue, AF2queue, AF3queue, AF4queue, BEqueue, EFqueue) ;
    q3->SetQueueSize(AF1queue, AF2queue, AF3queue, AF4queue, BEqueue, EFqueue) ;

    DiffServQueue::SetDiffServFlows(myFlowVector) ;
    DiffServQueue::SetDiffServMeters(myMeterVector) ;
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    //*****************           [END DIFF_SERV CONFIGURATION]          ***************************
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    //*****************           [Applications' SET UP]          ***************************
    //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
    NS_LOG_INFO("Setup applications ...");

    // TCP Cubic Configuration

    		string tcpCong = "cubic";
    		//string tcpCong = "hybla";
      Config::Set ("/NodeList/*/$ns3::Ns3NscStack<linux2.6.26>/net.ipv4.tcp_congestion_control", StringValue (tcpCong));
      PacketSinkHelper app1_dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), VoIPPort));
      //PacketSinkHelper app2_dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), VideoPort));
      PacketSinkHelper app3_dlPacketSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 21));


      // Service t=0s

/*
      int VoIP_max = 0;
      int Video_max = 9;
      int FTP_max = 8;
      int Mix_max = 7;
*/
      ostringstream  name_UE;

     // Ramdom exponential time
     /* vector<float> tiapp;
      tiapp.push_back(1);
      for(int jk =1; jk<40; jk++)
      {

      	tiapp.push_back(tiapp[jk-1]+ExponentialVariable(1.5).GetValue());
      }
      random_shuffle(&tiapp[0], &tiapp[39]);
		*/
		float ran_t;
      ///====================================================
      ///====================================================
     /* int UE_list[numberOfUE];
      // Random UE allocation
      for(int j =0; j<numberOfUE; j++)
      {
    	  UE_list[j] = j+1;
      }
      random_shuffle(&UE_list[0], &UE_list[numberOfUE-1]);
      */

      int UE_i;
      int UE_sel;

	  //====================================================
      ///====================================================

       ///LTE Random starting data transfer
      	  ofstream stat_LTE;

         ///===================================================




      for(int i=0; i<numberOfUE;i++)
      //for(int i=nbre_UE-1; i>=0;i--)
    	{

    	UE_i = i;
    	UE_sel = i;//UE_list[i];
    	UE_sel = UE_sel +1;
    	//cout << UE_list[i]<< ""<<endl;

    	name_UE <<"UE-"<<UE_i<<"eNB";
    	lteHelper->Attach (ueLteDevs.Get(UE_i), enbLteDevs.Get(0));
    	Names::Add (name_UE.str (), ueLteDevs.Get(UE_i));
    	name_UE.str("");
  		Ptr<Node> ueNode = ueNodes.Get (UE_i);
  		// Set the default gateway for the UE
  		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);


 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>
 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>
 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>
  		//               +------------------------+
  	    //				|     Traffic SETUP      |
  	    //				+------------------------+
 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>
 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>
 	    // <*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*><*>

  		 //================//
		  //  Traffic  FTP  //
		  //================//
///====================================================
///====================================================
  		if (true)// ((UE_sel >= 3))
		    {

		      ApplicationContainer clientApps;
		      ApplicationContainer serverApps;

			ran_t = UniformVariable(1,5).GetValue();
			serverApps.Add (app3_dlPacketSinkHelper.Install (ueNode));

		/// TCP fond traffic
			BulkSendHelper app3_dlClient("ns3::TcpSocketFactory",InetSocketAddress(ueIpIface.GetAddress(i), 21)) ;
			// Set the amount of data to send in bytes.  Zero is unlimited.
		     app3_dlClient.SetAttribute ("Random", BooleanValue (true));
		     app3_dlClient.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=5]")) ;
		     app3_dlClient.SetAttribute("FileSize", StringValue("ns3::UniformRandomVariable[Min=0.1|Max=1]"));
			 clientApps.Add (app3_dlClient.Install (server_2));

			serverApps.Start(Seconds(ran_t));
			clientApps.Start(Seconds(ran_t));
			serverApps.Stop(Seconds(simTime));
			clientApps.Stop(Seconds(simTime));

		    }


		  //================//
		  //   Traffic VoIP //
		  //================//

		if (false)//((UE_sel == 0) ||(UE_sel == 4)|| (UE_sel == 5)|| (UE_sel == 6)|| (UE_sel == 7)||(UE_sel == 16) || (UE_sel == 17)||(UE_sel == 18)||(UE_sel == 19))
			  {

			ran_t = UniformVariable(1,5).GetValue();
			float on_Time = ran_t;
			float off_Time= on_Time + LogNormalVariable(3.8945309,1.0041).GetValue();
			while(on_Time<simTime)
					{

					ApplicationContainer server_voip = app1_dlPacketSinkHelper.Install (ueNode);
					///====================================================
					/// VoIP connection from: server 1 => UEs
		    		OnOffHelper app1_dlClient("ns3::UdpSocketFactory",InetSocketAddress(ueIpIface.GetAddress(UE_i), VoIPPort)) ; // precise the protocol by which the traffic is transported and its destination port address
		    		app1_dlClient.SetAttribute("PacketSize",pkt_voip) ; // the packet size in byte
		    		app1_dlClient.SetAttribute("DataRate",voip_rate_ns3) ; // the data rate in bits per second
		    		app1_dlClient.SetAttribute("OnTime", StringValue("ns3::WeibullRandomVariable[Shape=1.423|Scale=0.824]")) ;  // the on time is a random variable with mean of 0.352
		    		app1_dlClient.SetAttribute("OffTime", StringValue("ns3::WeibullRandomVariable[Shape=0.899|Scale=1.089]")) ;  // the off time is a random variable with mean of 0.65
		    		ApplicationContainer client_voip = app1_dlClient.Install (server_1);

	    		 	client_voip.Start(Seconds(on_Time));
  				server_voip.Start(Seconds(on_Time));
  				client_voip.Stop(Seconds(off_Time));
  				server_voip.Stop(Seconds(off_Time));

  				on_Time = off_Time+ ExponentialVariable(7.101).GetValue();
  				off_Time = on_Time + LogNormalVariable(3.8945309,1.0041).GetValue();

  				if(off_Time>simTime){off_Time = simTime;}

					}

			  if(bearer==1)
			  	  {
				  //=============//
				  // VoIP Bearer//
				  //=============//
				  // Simulator::Schedule (Seconds (24.0), &Video_GBR, lteHelper, ueLteDevs.Get(i));
				  Ptr<EpcTft> tft_voip = Create<EpcTft> ();
				  EpcTft::PacketFilter pf_voip;
				  pf_voip.localPortStart = VoIPPort;
				  pf_voip.localPortEnd = VoIPPort;
				  tft_voip->Add (pf_voip);
				  GbrQosInformation qos_voip;
				  qos_voip.gbrDl = 44000; // Downlink GBR
		  	  	  qos_voip.gbrUl = 44000; // Uplink GBR
		  	  	  qos_voip.mbrDl = qos_voip.gbrDl; // Downlink MBR
		  	  	  qos_voip.mbrUl = qos_voip.gbrUl; // Uplink MBR
		  	  	  EpsBearer bearer_voip (EpsBearer::GBR_CONV_VOICE, qos_voip);
		  	  	  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(UE_i), bearer_voip, tft_voip);
			    	}


			  }
		  //================//
		  //   Traffic Web  //
		  //================//

			  if (false)//( (UE_sel == 2) || (UE_sel == 12)|| (UE_sel == 13)|| (UE_sel == 14)|| (UE_sel == 15)||(UE_sel == 16) || (UE_sel == 17)||(UE_sel == 18)||(UE_sel == 19))
		    					 {
		  						ran_t = UniformVariable(1,5).GetValue();

		  						  HttpHelper httpHelper;
		  						  HttpServerHelper httpServer;
		  						  NS_LOG_LOGIC ("Install Http in server");
		  						  httpServer.SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), (secPort + i + 50))));
		  						  httpServer.SetAttribute ("HttpController", PointerValue (httpHelper.GetController ()));
		  				      	  ApplicationContainer server_http = httpServer.Install (server_2);

		  				      	  HttpClientHelper httpClient;
		  				      	  httpClient.SetAttribute("UserID",UintegerValue (UE_i));
		  				      	  httpClient.SetAttribute("UserRequestSize",UintegerValue (200));
		  				      	  httpClient.SetAttribute("UserServerDelay",DoubleValue (0.1));
		  				      	  httpClient.SetAttribute("UserObjectRequestGap",DoubleValue (0.01));
		  				      	  httpClient.SetAttribute("PageTimeout",UintegerValue (30));
		  				      	  httpClient.SetAttribute ("Server", AddressValue (InetSocketAddress (internetIpIfaces2.GetAddress(1),  (secPort + i + 50))));
		  				      	  httpClient.SetAttribute ("HttpController", PointerValue (httpHelper.GetController ()));
		  				      	  ApplicationContainer client_http = httpClient.Install (ueNode);


		  				      	client_http.Start(Seconds(ran_t));
		    				    server_http.Start(Seconds(ran_t));
		    				    client_http.Stop(Seconds(simTime));
		    				    server_http.Stop(Seconds(simTime));

		    					  if(bearer==1)
		    					  	  {
		    						  //=============//
		    						  // web Bearer//
		    						  //=============//
		    						  Ptr<EpcTft> tft_web = Create<EpcTft> ();
		    						  EpcTft::PacketFilter pf_web;
		    						  pf_web.remotePortStart=(secPort + 50);
		    						  pf_web.remotePortEnd= (secPort +50 + 50);
		    						  tft_web->Add (pf_web);
		    				  	  	  EpsBearer bearer_web (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
		    				  	  	  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(UE_i), bearer_web, tft_web);
		    					    	}



				}

    					  //================//
    					  // Traffic YouTube//
    					  //================//

		    	if (true)//( (UE_sel == 1) || (UE_sel == 8)|| (UE_sel == 9)|| (UE_sel == 10)|| (UE_sel == 11)||(UE_sel == 19) )
    					 {
  						ran_t = UniformVariable(1,5).GetValue();
  						  YoutubeHelper youtubeHelper;
  						  YoutubeServerHelper youtubeServer;
  						  NS_LOG_LOGIC ("Install Youtube in server");
  				      	  youtubeServer.SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), (secPort + i))));
  				      	  youtubeServer.SetAttribute ("YoutubeController", PointerValue (youtubeHelper.GetController ()));
  				      	  ApplicationContainer server_youtube = youtubeServer.Install (server_2);

  				      	  YoutubeClientHelper youtubeClient;
  				      	  youtubeClient.SetAttribute("UserID",UintegerValue (UE_i));
  				      	  youtubeClient.SetAttribute("UserRequestSize",UintegerValue (200));
  				      	  youtubeClient.SetAttribute("UserServerDelay",DoubleValue (0.1));
  				      	  youtubeClient.SetAttribute("VideoSize",DoubleValue (5*25));
  				      	  youtubeClient.SetAttribute("PageTimeout",UintegerValue (100));
  				      	  youtubeClient.SetAttribute ("Server", AddressValue (InetSocketAddress (internetIpIfaces2.GetAddress(1), (secPort + i))));
  				      	  youtubeClient.SetAttribute ("YoutubeController", PointerValue (youtubeHelper.GetController ()));
  				      	  ApplicationContainer client_youtube = youtubeClient.Install (ueNode);


  				      	client_youtube.Start(Seconds(ran_t));
    				    server_youtube.Start(Seconds(ran_t));
    				    client_youtube.Stop(Seconds(simTime));
    				    server_youtube.Stop(Seconds(simTime));

    				    if(bearer==1)
    				       				 {

    				       						//========================//
    				       				  		// YouTube and Web Bearer//
    				       				  		//=======================//
    				       				//Simulator::Schedule (Seconds (24.0), &VoIP_GBR, lteHelper, ueLteDevs.Get(i));
    				       				  Ptr<EpcTft> tft_video = Create<EpcTft> ();
    				       			      EpcTft::PacketFilter pf_video;
    				       			      pf_video.remotePortStart=(secPort);
    				       			      pf_video.remotePortEnd= (secPort + 49);
    				       			      tft_video->Add (pf_video);
    				       				  GbrQosInformation qos_video;
    				       				  qos_video.gbrDl = 1500000; // Downlink GBR
    				       				  qos_video.gbrUl = 1500000; // Uplink GBR
    				       				  qos_video.mbrDl = 2000000; // Downlink MBR
    				       				  qos_video.mbrUl = 2000000; // Uplink MBR
    				       				  //EpsBearer bearer_video (EpsBearer::GBR_CONV_VOICE, qos_video);
    				       		          EpsBearer bearer_video (EpsBearer::GBR_NON_CONV_VIDEO, qos_video);
    				       				  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(UE_i), bearer_video, tft_video);
    				       				 }

    					  }





    	}

 //	==================================================================================//





  // Uncomment to enable PCAP tracing
		//PP1.EnablePcapAll(pcapFileNamePrefix);
 PP2.EnablePcapAll("LTE_YB");

  lteHelper->EnableTraces();

   Ptr<FlowMonitor> flowmon;
   FlowMonitorHelper flowmonhelper;
	if (enableFlowMonitor)
	{
	  NS_LOG_INFO("Creating the flow monitor probes...");
		flowmon = flowmonhelper.Install (ueNodes);
		flowmon = flowmonhelper.Install (enbNodes);
		flowmon = flowmonhelper.Install (remoteHostContainer);
		flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.00001));
		flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.00001));
		flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20));
	}

	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();



 // Output config store to txt format
	Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("LTE-attributes.txt"));
	Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
	Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
	ConfigStore outputConfig;
	outputConfig.ConfigureDefaults ();
	outputConfig.ConfigureAttributes ();




  Simulator::Stop(Seconds(simTime));

//ANIMMMMM




  	// anim.UpdateNodeDescription (enbNodes.Get(0),"eNB"); // Obligue

    /*  AnimationInterface::UpdateNodeDescription (ueNodes.Get(0), "UE"); // Optional
    AnimationInterface::UpdateNodeDescription (remoteHostContainer, "SA"); // Optional
    AnimationInterface::UpdateNodeDescription (pgw, "EPC"); // Optional
    AnimationInterface::UpdateNodeColor (pgw, 0, 255, 0); // Optional
    AnimationInterface::UpdateNodeColor (enbNodes.Get (0), 0, 255, 0); // Optional
    AnimationInterface::UpdateNodeColor (ueNodes.Get(), 255, 0, 0); // Optional
    AnimationInterface::UpdateNodeColor (remoteHostContainer.Get(), 0, 0, 255); // Optional
*/

  /*
   // Animation part
   //=================
  	  	  AnimationInterface anim ("LTE-animation.xml"); // Mandatory
  	  	  anim.SetMobilityPollInterval(Seconds(0.01));
    	  anim.EnablePacketMetadata (true); // Optional
    	  anim.EnableIpv4RouteTracking ("routingtable-LTE.xml", Seconds(0), Seconds(5), Seconds(0.25)); //Optional
*/

     uint16_t t =lteEnbDev->GetUlBandwidth();
     uint16_t t2 =lteEnbDev->GetDlBandwidth();
     std:: cout<<"UlBandwidth/DlBandwidth = "<<t<<"/"<<t2<<std::endl;


  Simulator::Run();


  if (enableFlowMonitor)
{
	string type_flow;
	flowmon->CheckForLostPackets();
	 ostringstream    stat_q;
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonhelper.GetClassifier());
	cout<<endl;
	NS_LOG_INFO("Writing statistical results in Simu_gen.xml file...");

	string proto;
              std::map< FlowId, FlowMonitor::FlowStats > stats = flowmon->GetFlowStats();
				 cout<<endl;
              for (std::map< FlowId, FlowMonitor::FlowStats >::iterator flow=stats.begin(); flow!=stats.end(); flow++)
              {
                      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow->first);
                      switch(t.protocol)
                      {
                      case(6):
                              proto = "TCP";
                              break;
                      case(17):
                              proto = "UDP";
                              break;
                      default:
                              exit(1);
                      }

                 //if(t.destinationPort==VoIPPort || t.destinationPort==21 || t.destinationPort== VideoPort)
        			//{



                	 //uint8_t y = (((t.destinationAddress.Get() & 255) >> 16) & 0xff);
                	 	 uint8_t y = (t.destinationAddress.Get() & 255) ;
                      if(t.destinationPort== VoIPPort){type_flow=" VoIP ";}
                      else if(t.destinationPort== 21){type_flow=" FTP ";}
                      else{type_flow=" VIDEO ";}
                      	std::cout << "****************[ "<<type_flow<<" UE-"<<int(y)-1<<" ]*******************" <<std::endl;
                        std::cout << "FlowID: " << flow->first << " (" << proto << " " << t.sourceAddress << "/" << t.sourcePort << " ==> " << t.destinationAddress << "/" << t.destinationPort << ")" << std::endl;
                        std::cout << "**********************************************" <<std::endl;
                        stat_q <<"Gen_stats.txt";
                        printStats(flow->second, stat_q.str(),m_firstWrite, y , t.destinationPort);
                         if(t.destinationPort== 4300){m_firstWrite = false;}
						stat_q.str("");
                      					}
              //}
	flowmon->SerializeToXmlFile("LTE_stas.xml",true,true);
	std::cout << "**********************************************" <<std::endl;

	}




  Simulator::Destroy();
 //return 0;
  //Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (client_video.Get(0));
//  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
}

