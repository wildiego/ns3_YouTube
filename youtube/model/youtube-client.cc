/* -*-	Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Orange Labs Network
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

#include <limits>
#include <deque>
#include <math.h>       /* round, floor, ceil, trunc */
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>       /* exp */


#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"

#include "youtube-client.h"
#include "youtube-controller.h"


NS_LOG_COMPONENT_DEFINE ("YoutubeClient");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (YoutubeClient);
TypeId
YoutubeClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YoutubeClient")
    .SetParent<Application> ()
    .AddConstructor<YoutubeClient> ()
    .AddAttribute ("YoutubeController", "The controller",
                   PointerValue (0),
                   MakePointerAccessor (&YoutubeClient::SetController,
                                        &YoutubeClient::GetController),
                   MakePointerChecker<YoutubeController> ())
    .AddAttribute ("Timeout", "Time out value for each one web page",
                   UintegerValue (40),
                   MakeUintegerAccessor (&YoutubeClient::m_Timeout),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Server", "The peer address for the client",
                   AddressValue (),
                   MakeAddressAccessor (&YoutubeClient::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("User", "The local address for client",
                   AddressValue (),
                   MakeAddressAccessor (&YoutubeClient::m_addClient),
                   MakeAddressChecker ())
    .AddAttribute ("TransportId", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&YoutubeClient::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Persistent", "Set if the connection is persistent connection or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&YoutubeClient::m_persistent),
                   MakeBooleanChecker ())
    .AddAttribute ("Pipelining", "Set if the connection is doing pipelining or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&YoutubeClient::m_pipelining),
                   MakeBooleanChecker ())
    .AddAttribute ("UserID", "User ID used for result file",
                   UintegerValue (0),
                   MakeUintegerAccessor (&YoutubeClient::m_userID),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UserServerDelay", "User defined server delay",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&YoutubeClient::m_userServerDelay),
                   MakeDoubleChecker<double> ())
     .AddAttribute ("VideoSize", "Video Youtube size in seconds",
                    DoubleValue (60),
                    MakeDoubleAccessor (&YoutubeClient::m_videoSize),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("UserRequestSize", "User defined size of the request",
                   UintegerValue (100),
                   MakeUintegerAccessor (&YoutubeClient::m_userRequestSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ChunkSize", "User defined size chunks (seconds)",
                   UintegerValue (5),
                   MakeUintegerAccessor (&YoutubeClient::m_chunkSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

YoutubeClient::YoutubeClient ()
{
  /// Only test for if this is the first connection or not
  //m_realFirstObject = true;
	NS_LOG_FUNCTION (this);
	  m_connected = false;
	  m_lastItem =false;
	  m_tagPause =false;
	  m_firstChunk = true;
	  m_timeBuffer = 0;
	  m_timeVideo = 0;
	  m_timeStartVideo = 0;
	  m_pauseTime = 0;
	  m_pauseTimes = 0;
	  m_endDl = 0;
	  m_firstWrite = true;
	  m_pauseTotal =0;
	  m_endVideo =0;
	  m_itag1 = 0;
	  m_itag2 = 0;
	  m_itag3 = 0;
	  m_itag4 = 0;
	  m_chunkNumber = ceil(m_videoSize/m_chunkSize)+1;

}

YoutubeClient::~YoutubeClient ()
{
	NS_LOG_FUNCTION (this);
}

void
YoutubeClient::SetController (Ptr<YoutubeController> controller)
{
  NS_LOG_FUNCTION (this);
  m_controller = controller;
}

Ptr<YoutubeController>
YoutubeClient::GetController () const
{
  NS_LOG_FUNCTION (this);
  return m_controller;
}


void
YoutubeClient::SetServer (Ptr<YoutubeServer> server)
{
  NS_LOG_FUNCTION (this);
  m_server = server;
}

Ptr<YoutubeServer>
YoutubeClient::GetServer() const
{
  NS_LOG_FUNCTION (this);
  return m_server;
}

Ptr<Socket>
YoutubeClient::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}


void
YoutubeClient::StartApplication ()     // Called at time specified by Start
{
	  NS_LOG_FUNCTION (this);

	  // Create the socket if not already
	  if (!m_socket)
	    {
	      m_socket = Socket::CreateSocket (GetNode (), m_tid);
	      m_socket->Bind (m_addClient);
	      uint32_t ipTos = 56;
	      m_socket->SetIpTos (ipTos);
	      bool ipRecvTos = true;
	      m_socket->SetIpTos(ipRecvTos);
	      m_socket->Connect (m_peer);
	      m_socket->SetConnectCallback (MakeCallback (&YoutubeClient::ConnectionComplete, this),
	                                    MakeCallback (&YoutubeClient::ConnectionFailed, this));
	     m_socket->SetSendCallback ( MakeCallback (&YoutubeClient::DataSend, this));

	    }

	  NS_LOG_DEBUG ("Start navigation at : "<<Simulator::Now ());

		  m_controller->SetYoutubeVersion (m_pipelining, m_persistent);

		  /// If it is not a persistent connection, there should not be pipelining
		  if (!m_persistent)
		    {
		      m_pipelining = false;
		    }

		  m_controller->SetClientSocket (m_socket);

			  m_connected = false;
			  m_lastItem =false;
			  m_tagPause =false;
			  m_firstChunk = true;
			  m_timeBuffer = 0;
			  m_timeVideo = 0;
			  m_timeStartVideo = 0;
			  m_pauseTime = 0;
			  m_pauseTimes = 0;
			  m_endDl = 0;
			  m_firstWrite = true;
			  m_pauseTotal =0;
			  m_chunkNumber = ceil(m_videoSize/m_chunkSize)+1;
			  m_itag1 = 0;
			  m_itag2 = 0;
			  m_itag3 = 0;
			  m_itag4 = 0;
			  m_timeClick=0;
			  m_endVideo =0;

	  TrafficGeneration ();

}




void
YoutubeClient::TrafficGeneration ()
{
	NS_LOG_FUNCTION (this);

	  Ptr<ExponentialRandomVariable> gab_re = CreateObject<ExponentialRandomVariable>();
	  gab_re->SetAttribute ("Mean", DoubleValue (10));

      double serverDelay = m_userServerDelay;
      double requestVideoGap = 0;
      double requestObjectGap = 0;

      NS_LOG_DEBUG (" TimeStartVideo "<< m_timeStartVideo << " m_endDl "<< m_endDl <<" m_itag4 "<<m_itag4);

	  if (!m_chunkNumber)
	  {
		  Stadistics ();
	  }


NS_LOG_DEBUG ("Start Video generation at : "<<Simulator::Now ());

  uint32_t responseSize;
  m_lastItem = false;
  float requestSize, chunkSize, schedChunk;
  float t_out = 0;
  schedChunk = 0;

  if(m_firstChunk)
  	  {
	  requestVideoGap = gab_re->GetValue();
	  chunkSize = 40000;
	  m_firstChunk=false;
	  m_itag4 +=1;
      if (m_firstVideo)   {requestVideoGap = 0;}
	  m_timeClick = Simulator::Now ().GetSeconds ();// + requestVideoGap;
  	  }

  else
  {

	 float rate =8*m_controller->GetClientSite ().totalRecived/ (m_controller->GetClientSite ().timeReceptionChunk - m_controller->GetClientSite ().timeSendChunk);
	 	 // Rate = 8 x data(bytes)/time

	 	 if(rate > 758000*2) // itag = 93
	 {
		 chunkSize = 475000;
		 m_itag1 +=1;
	 }
	 else if(rate > 395000*2)// itag = 92
	 {
		 chunkSize = 250000;
		 m_itag2 +=1;
	 }

	 else // itag = 132
	 {
		 chunkSize = 167000;
		 m_itag3 +=1;
	 }

	 /*
	 else if(rate > 266000*2)// itag = 132
	 {
		 chunkSize = 167000;
		 m_itag3 +=1;
	 }
	 else // itag = 151
	 {
	 chunkSize = 40000;
	 	 m_itag4 +=1;
	 }
	*/

	 NS_LOG_DEBUG ("---> Buffer Time = "<< m_timeBuffer <<" Start Time: "<< m_timeStartVideo);
	 if(m_timeBuffer > 30 && !(m_timeStartVideo))
	 {
		 NS_LOG_DEBUG ("[3] Start Play Video: "<< Simulator::Now ().GetSeconds ());
		 m_timeStartVideo = Simulator::Now ().GetSeconds ();
		 m_lastNow = Simulator::Now ().GetSeconds ();
	 }

	 if(m_timeStartVideo>0)
	 {

		 if (!m_tagPause) {
			 NS_LOG_DEBUG (" >> ?? = "<< m_timeVideo << " and last  "<< m_lastNow);
			 m_timeVideo= Simulator::Now ().GetSeconds () -  m_lastNow;}

		 NS_LOG_DEBUG (" >> Consumed = "<< m_timeBuffer - m_timeVideo);



		 if ((m_timeBuffer - m_timeVideo) <= 0 )	 {

			 if (!m_tagPause)
			 {  m_tagPause = true;
			 	m_pauseTime+=1;
			 	m_pauseStart = Simulator::Now ().GetSeconds () + (m_timeBuffer - m_timeVideo);
			 	m_timeVideo = 0;
			 	m_timeBuffer = 0;
			 }

			 m_pauseTimes = Simulator::Now ().GetSeconds () - m_pauseStart;

			 NS_LOG_DEBUG ("[7] Pause Video Time at: "<< m_pauseStart<<"  # of pause: " << m_pauseTimes <<"  and total time: "<<  m_pauseTime<< " Total Pause: "<< m_pauseTotal);
		 }


		 if ((m_timeBuffer - m_timeVideo)> 0)
		 {
			 if(m_tagPause)
			 {
				 if(m_timeBuffer>30)
				 {
					 m_pauseTimes = Simulator::Now ().GetSeconds () - m_pauseStart;
					 m_lastNow = Simulator::Now ().GetSeconds ();
					 m_tagPause = false;
					 m_pauseStart = 0;
					 m_pauseTotal += m_pauseTimes;
					 NS_LOG_DEBUG ("Total Pauses time: "<< m_pauseTotal);
				 }



			 }

			 else{

			 m_tagPause = false;
			 NS_LOG_DEBUG ("[5] Watching : "<< m_timeVideo);
			 NS_LOG_DEBUG ("[**] Buffer Time Before "<< m_timeBuffer);
			 m_timeBuffer -= m_timeVideo;
			 NS_LOG_DEBUG ("[**] Buffer Time After "<< m_timeBuffer);
			 m_pauseStart = 0;
			 m_lastNow = Simulator::Now ().GetSeconds ();
			 NS_LOG_DEBUG (" >> ??? = "<< m_timeVideo << " and last  "<< m_lastNow);
			 }
		 }

//		 if(!m_tagPause)
//		 {
//		 NS_LOG_DEBUG ("[**] Buffer Time Before "<< m_timeBuffer);
//		 m_timeBuffer -= m_timeVideo;
//		 NS_LOG_DEBUG ("[**] Buffer Time After "<< m_timeBuffer);
//		 }

		 if (m_timeBuffer > 100 && (m_chunkNumber > 1))
			{
			 NS_LOG_INFO ("Buffer > 110s : Client is closing the socket ");
				 schedChunk = 60;//m_timeBuffer - 40;
				 NS_LOG_DEBUG ("[4] Stop TX and close TCP connection for "<< schedChunk);


			}

		 if(m_chunkNumber==1 && m_timeBuffer > 0)
		 {
			 m_endDl = Simulator::Now ().GetSeconds ();
			 m_endVideo =m_endDl+m_timeBuffer;
			 t_out = m_timeBuffer;
		 }
		 NS_LOG_DEBUG ("[c*c*] Stop TX and close TCP connection for "<< schedChunk);
	 }
  }
  	  	  	  	YoutubeSite site;
              	responseSize = chunkSize;
                requestSize = m_userRequestSize;
                requestObjectGap = 0;
                YoutubeMessage request;
                request.size = requestSize;
                request.messageType = YoutubeMessage::REQUEST;
                request.requestObjectGapTime = requestObjectGap;
                YoutubeMessage response;
                response.size = responseSize;
                response.messageType = YoutubeMessage::RESPONSE;
                response.serverDelayTime = serverDelay;

                m_chunkNumber-=1;


          	  site.request.push_back (request);
          	  site.objects.push_back (response);


          if (m_firstVideo)
            {
        	  NS_LOG_DEBUG (" --> This is the first video");
        	  requestVideoGap = 0;
              m_firstVideo = false;

            }

          else if (!m_chunkNumber )

          {

        	  requestVideoGap = gab_re->GetValue();

        	  if (m_socket != 0)
          	    {
          	      NS_LOG_INFO ("END Video: Client is closing the socket: " << m_socket);
          	      m_socket->Close ();
          	      Simulator::Schedule (Seconds(requestVideoGap + m_timeBuffer), &YoutubeClient::StartNewSocket, this);
          	      Simulator::Schedule (Seconds(requestVideoGap + m_timeBuffer), &YoutubeClient::TrafficGeneration, this);
          	    }

          }


          else{
        	  requestVideoGap = 0;
          	  }

          NS_LOG_DEBUG ("The request size " << requestSize << " The response Size " << responseSize);
          /// When this is the first object in first web page, we generate the request gap time between
          /// different web pages
          NS_LOG_DEBUG ("The request object gap here " << requestObjectGap);



          site.requestPageGaps = requestVideoGap;
          site.totalPageSize= chunkSize;
          site.remainingPageSize = chunkSize;
          site.firstObject = true;
          site.windowSize = 8;
          NS_LOG_DEBUG ("The video gap here " << requestVideoGap);

          m_timeoutEvent.Cancel ();
          m_timeoutEvent = Simulator::Schedule (Seconds (m_Timeout+schedChunk+ requestVideoGap+t_out), &YoutubeClient::CallTimeOut, this, m_socket);



        if(schedChunk>0)
        {
        	 NS_LOG_INFO ("Buffer > 110s : Client is closing the socket: " << m_socket);
        	 if (m_socket != 0)
        	  	 {
        		 m_socket->Close ();
        	  	 Simulator::Schedule (Seconds (schedChunk), &YoutubeClient::StartNewSocket, this);
        	  	 Simulator::Schedule (Seconds (schedChunk), &YoutubeClient::StartVideo, this,site);
        	  	 }

        	 schedChunk = 0;
        }

        else
        	{
        	StartVideo (site);
        	}



}

void
YoutubeClient::StartVideo(YoutubeSite site)
{

		  NS_LOG_FUNCTION_NOARGS ();
		  NS_LOG_FUNCTION (this);

//if  (!site.firstObject){TrafficGeneration();}


		  YoutubeSite clientSide = site;
		  if (m_firstChunk){site.startTime = Simulator::Now ().GetSeconds ();}

		  //site.timeSendChunk = Simulator::Now ().GetSeconds ();

		  clientSide.remainingPageSize = clientSide.totalPageSize;

	         NS_LOG_LOGIC ("Start web page navigation !!!");
	          m_controller->SetClientSite (site);
	          m_controller->SetServerSite  (site);
	          //m_controller->ClientSend (m_socket);

	          /// This is the first object inside of one web page, schedule the next client send using the request page gap time
		      double nextPageGap = clientSide.requestPageGaps;
		      // time at which the next data shall be sent

		      NS_LOG_LOGIC ("Start at " << nextPageGap);
		      //m_startStopEvent = Simulator::Schedule (Seconds(nextPageGap), &YoutubeClient::StartSending, this);
		      m_controller->StartVideo (m_socket);//, m_server);

	         // m_controller->FirstObjectClientSend (m_socket, sendTime);
	       	 // m_controller->ClientSend (m_socket);



}



void
YoutubeClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (m_socket << " " << Simulator::Now () << " Stop Application");
  if (m_socket != 0)
    {
      NS_LOG_INFO ("Client is closing the socket: " << m_socket);
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  m_connected = false;
  Simulator::Cancel (m_timeoutEvent);
}

void
YoutubeClient::ConnectionComplete (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // Get ready to receive.
  socket->SetRecvCallback (MakeCallback (&YoutubeClient::ClientReceive, this));
  NS_LOG_LOGIC ("Break Client Receive");
  m_controller->SetClientSocket (socket);
  m_connected = true;
}

void
YoutubeClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // Retry the connection I assume? This isn't really supposed to happen.
  NS_LOG_WARN ("Client failed to open connection. Retrying.");
  m_socket->Connect (m_peer);
}


void
YoutubeClient::SetIndex (uint32_t index)
{
	m_index = index;
}
void
YoutubeClient::SetClientApplication ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = GetNode ();
/*
 for (uint32_t i = 0; i < node->GetNApplications (); ++i)
   {
      if (node->GetApplication (i) == this)
        {
          m_controller->SetClientApplication (node, i);
        }
    }
 */
 m_controller->SetClientApplication (node, m_index);

}

void
YoutubeClient::StartNewSocket ()
{
  NS_LOG_FUNCTION (this);
  m_socket = Socket::CreateSocket (GetNode (), m_tid);
  uint32_t ipTos = 56;
  m_socket->SetIpTos(ipTos);
  bool ipRecvTos = true;
  m_socket->SetIpTos(ipRecvTos);
  m_socket->Bind (m_addClient);
  m_socket->Connect (m_peer);

  //m_socket->ShutdownRecv ();
  m_socket->SetConnectCallback (MakeCallback (&YoutubeClient::ConnectionComplete, this),
                                MakeCallback (&YoutubeClient::ConnectionFailed, this));

  m_socket->SetSendCallback ( MakeCallback (&YoutubeClient::DataSend, this));

  m_socket->SetRecvCallback (MakeCallback (&YoutubeClient::ClientReceive, this));

  m_controller->SetClientSocket (m_socket);


}



///


void
YoutubeClient::ClientReceive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  //NS_LOG_DEBUG ("Client Receive start");
  /*
   * Get the Adu container for client and server for the Youtube controller
   * The controller is maintaining both the client and server side containers
   */
  bool ipRecvTos = true;
  socket->SetIpTos(ipRecvTos);



  YoutubeSite serverSite = m_controller->GetClientSite ();

  NS_LOG_LOGIC (" Objects:  " <<  m_controller->GetClientSite ().objects.size() );
  Ptr<Packet> packet = socket->Recv();
  NS_LOG_LOGIC ("*+* "<<" RX availavility "<< socket->GetRxAvailable()<<"  Reference Count: "<< socket->GetReferenceCount());
  uint32_t bytesToRecv = packet->GetSize ();
  NS_LOG_DEBUG ("Client Receive Packet: "<< bytesToRecv << " bytes of an Object. "<< serverSite.objects.front ().size << " Site size: "<< serverSite.remainingPageSize);

  while (bytesToRecv)
    {
      NS_LOG_DEBUG ("The object size " << serverSite.objects.front ().size);
      NS_LOG_LOGIC ("*+* "<<" RX availavility "<< socket->GetRxAvailable()<<"  Reference Count: "<< socket->GetReferenceCount());
      if (serverSite.objects.front ().size > bytesToRecv)
        {
          // Received less than a whole ADU. Reduce its size by the number of bytes received.
    	  serverSite.remainingPageSize -= bytesToRecv;
    	  serverSite.objects.front ().size -= bytesToRecv;
    	  serverSite.totalRecived += bytesToRecv;
          m_controller->SetClientSite(serverSite);
          NS_LOG_LOGIC ("Go : "<< serverSite.objects.size() <<" Objects and recived: " << serverSite.totalRecived << " of "<<serverSite.totalPageSize );
          //TrafficGeneration ();

          NS_LOG_LOGIC ("Server has send :" << m_controller->GetServerSite().remainingPageSize<<" and " << m_controller->GetServerSite().objects.size());

          if(((serverSite.totalRecived)>=serverSite.objects.front ().size))
          {

        	  m_controller->GetClientSite ().timeReceptionChunk=Simulator::Now ().GetSeconds ();
        	  NS_LOG_LOGIC ("Finished Chunk");
        	  m_timeBuffer+=5;
        	  TrafficGeneration ();

          }
          break;
        }
      else
        {

    	  NS_LOG_LOGIC ("Remove the whole thing and continue receiving");
    	  serverSite.totalRecived+=bytesToRecv;
    	  NS_LOG_LOGIC ("Go : "<< serverSite.objects.size() <<" Objects and received: " << serverSite.totalRecived << " of "<<serverSite.totalPageSize );
    	  // Front ADU size <= bytesToRecv: remove the whole thing and continue receiving
          bytesToRecv -= serverSite.objects.front ().size;
          serverSite.remainingPageSize -= serverSite.objects.front ().size;

          NS_LOG_LOGIC ("Finished receiving an Object. " << serverSite.objects.size () << " to go. Site size: "<< serverSite.remainingPageSize);

          //serverSite.objects.pop_front ();

          //m_controller->Cleanup ();
          m_controller->SetClientSite(serverSite);
          NS_LOG_LOGIC ("Finished receiving an Object. " << serverSite.objects.size () << " to go. Site size: "<< serverSite.remainingPageSize);




          if (((serverSite.totalRecived)>=serverSite.objects.front ().size))
            {
        	  m_controller->GetClientSite ().timeReceptionChunk=Simulator::Now ().GetSeconds ();
        	  NS_LOG_LOGIC ("Finished Chunk");
        	  m_timeBuffer+=5;
        	  // We generate next web site
        	  //m_controller->Cleanup ();
        	  TrafficGeneration ();

            }
          else if (!m_pipelining)
            {
              NS_LOG_LOGIC ("This is the non-pipelining connection, and working with the next web request");
              // The next adu is request or the terminator, schedule the next send
              // We would send the request immediately if past due of the request gap time
              // If this is not persistent connection, right after the next connection is established,
              // we will send the next web request
              m_controller->ScheduleNextClientSend (socket);
            }
          else
            {
        	  NS_LOG_LOGIC ("Same connection due to pipelining");
              NS_ASSERT (m_pipelining);
              //serverSite.objects.front ().size -= bytesToRecv;
              //m_controller->SetClientSite(serverSite);
            }
          continue;
        }
    }



}



void
YoutubeClient::CallTimeOut (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("Time: " << Simulator::Now () << " Socket: " << socket << " We have a transaction timeout");


  m_endDl = 0;
  m_chunkNumber = 0;
  m_controller->Cleanup ();
	 m_socket->Close ();
	 StartNewSocket();
  TrafficGeneration ();



}

void
YoutubeClient::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

  if (m_connected)
    {
    // Only send new data if the connection has completed
      Simulator::ScheduleNow (&YoutubeClient::CallController, this);
    }

}

void YoutubeClient::CallController (void)
{

	m_controller->ClientSend(m_socket);
}


void YoutubeClient::Stadistics ()
{
	NS_LOG_INFO ("Writing statistics ... ");

			  	  	  	  if (m_endDl==0){m_endDl=Simulator::Now ().GetSeconds (); }

			  	  	  	  std::ofstream os_lte;
			  	  	  	  std::ostringstream  name_queue;
			  		  	  name_queue<<"KPI_youtube_"<<m_userID<<".txt";
			  		  	  std::string fileName = name_queue.str ();

			  	if (m_firstWrite == true )
			  		  	    	    {
			  		  	    		os_lte.open (fileName.c_str ());
			  		  	    		m_firstWrite = false;
			  		  	    		os_lte << "UserID\tStartClick\tStartPlay\tStartBuffer\tEndDL\tEndTime\tPausesNum\tPausesTotal\tMOS\tItag_93\tItag_94\tItag_132\tItag_151";
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

			  		  	    		float MOS= 3.5*exp(-(0.15*m_pauseTime+0.19)*m_pauseTotal)+1.5;

			  		  	    		os_lte <<m_userID<< "\t";
			  		  	    	    os_lte <<m_timeClick<< "\t";
			  		  	    		os_lte <<m_timeStartVideo<< "\t";
			  		  	    		os_lte <<(m_timeStartVideo - m_timeClick)<< "\t";
			  		  	    	    os_lte <<m_endDl<< "\t";
			  		  	    		os_lte <<m_endVideo<< "\t";
			  		  	    		os_lte <<m_pauseTime<< "\t";
			  		  	    		os_lte <<m_pauseTotal<< "\t";
			  		  	    		os_lte << MOS<< "\t";
			  		  	    		os_lte <<m_itag1<< "\t";
			  		  	    		os_lte <<m_itag2<< "\t";
			  		  	    		os_lte <<m_itag3<< "\t";
			  		  	    		os_lte <<m_itag4<< "\n";
			  		  	    		os_lte.close ();
			  		  	    	    name_queue.str("");

			  		  	    		}
			  ///===================================================

			  NS_LOG_LOGIC ("ALL sessions are done, stopping the TCP connection for this node pair");
			  // Call stop application
			  	m_timeStartVideo =0;
			  	m_chunkNumber = ceil(m_videoSize/5)+1;
			  	m_firstChunk = true;
			  	m_controller->Cleanup ();
			  	m_tagPause = false;
			  	m_pauseTime = 0;
			  	m_pauseTimes= 0;
			  	m_timeBuffer = 0;
			  	m_lastNow=0;
			  	m_pauseStart=0;
			  	m_timeVideo=0;
			  	m_pauseTotal =0;
				  m_itag1 = 0;
				  m_itag2 = 0;
				  m_itag3 = 0;
				  m_itag4 = 0;
				  m_timeClick=0;
				  m_endVideo =0;





}

} // namespace ns3
