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



#include "youtube-controller.h"
#include "youtube-client.h"
#include "youtube-server.h"
//#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include <deque>
#include "ns3/core-module.h"
#include "ns3/boolean.h"
#include "ns3/packet.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("YoutubeController");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (YoutubeController);

TypeId
YoutubeController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YoutubeController")
    .SetParent<Object> ()
    .AddConstructor<YoutubeController> ()
    .AddAttribute ("SegmentSize", "The largest segment size",
                   UintegerValue (1440),
                   MakeUintegerAccessor (&YoutubeController::m_segmentSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

// \brief Constructor
YoutubeController::YoutubeController()

{
	NS_LOG_FUNCTION (this);
	m_persistent= true;
}

// \brief Destructor
YoutubeController::~YoutubeController ()
{
	NS_LOG_FUNCTION (this);
}

void
YoutubeController::SetServerSite (const YoutubeSite & site)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC("Size Request before: " << m_serverSite.request.front ().size);
  NS_LOG_LOGIC("Size Request after : " << site.request.front ().size );
  m_serverSite = site;
}

void
YoutubeController::SetClientSite (const YoutubeSite & site)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC("Size Object before: " << m_clientSite.objects.front ().size);
  NS_LOG_LOGIC("Size Object after : " << site.objects.front ().size );
  m_clientSite = site;
}

YoutubeSite &
YoutubeController::GetServerSite ()
{
	NS_LOG_FUNCTION (this);
  return m_serverSite;
}

YoutubeSite &
YoutubeController::GetClientSite ()
{
	NS_LOG_FUNCTION (this);
	return m_clientSite;
}

void
YoutubeController::SetYoutubeVersion (bool pipelining, bool persistent)
{
  NS_LOG_FUNCTION (this << pipelining << persistent);
  m_persistent = persistent;
  if (m_persistent)
    {
      m_pipelining = pipelining;
    }
  else
    {
      m_pipelining = false;
    }
}

void
YoutubeController::Cleanup ()
{
  NS_LOG_FUNCTION (this);
  m_serverSite.Clear ();
  m_clientSite.Clear ();
}

void
YoutubeController::StartVideo (Ptr<Socket> socket)
{
	  NS_LOG_FUNCTION (this << socket);
	  NS_LOG_DEBUG ("The second now " << Simulator::Now ().GetSeconds ()<<" - >Web Navigation in: "<< m_serverSite.requestPageGaps << "s");

	  if (m_persistent)
	    {

		  Simulator::Schedule (Seconds (m_serverSite.requestPageGaps), &YoutubeController::CallClientSend, this);
	    }
	  else
	    {
	      Simulator::Schedule (Seconds (m_serverSite.requestPageGaps), &YoutubeController::StartNewClientSocket, this);
	    }

}


void
YoutubeController::ClientSend (Ptr<Socket> socket)
{
  //socket = m_socket;
	NS_LOG_FUNCTION (this << socket);


	  //if (CheckNextRequest(m_clientSite))
	  //{
		  YoutubeMessage requestMessage = m_clientSite.request.front();
 // Stuff the whole ADU into the send buffer, one packet at a time.
	uint32_t bytesToSend = requestMessage.size;
	  uint32_t bytesQueued = 0;

	  //m_clientSite.request.pop_front();
      // When we call Client Send function, the first ADU should be request
      NS_ASSERT (requestMessage.messageType == YoutubeMessage::REQUEST);



      while (bytesToSend)
        {
          int bytesAccepted = 0;

          //Segmentation part
          //======================================================
          if (bytesToSend > m_segmentSize)
            {
        	 Ptr<Packet> packet = Create<Packet> (m_segmentSize);
        	  //Tag tag = "Hola";
        	  //packet->AddByteTag()
        	 bytesAccepted = socket->Send (packet);
              //bytesAccepted = socket->Send(m_segmentSize);

            }
          else
            {
        	  Ptr<Packet> packet = Create<Packet> (bytesToSend);
              bytesAccepted = socket->Send (packet);

              break;

            }

          //Information about sending process
          //======================================================
          if (bytesAccepted > 0)
            {
              bytesToSend -= bytesAccepted;
              bytesQueued += bytesAccepted;
            }

          else if (socket->GetErrno () == TcpSocket::ERROR_MSGSIZE)
            {
              // This part might have better way
             // NS_FATAL_ERROR ("Send buffer filled while sending ADU bytes already queued). Increase TcpSocket's SndBufSize.");
        	  break;
            }
          else if (socket->GetErrno () == TcpSocket::ERROR_NOTCONN)
            {
              // This means that the socket was already closed by us for sending, somehow.
              //NS_FATAL_ERROR ("ERROR_NOTCONN while sending - this is a bug");
        	  break;
            }
          else
            {
              NS_FATAL_ERROR ("Error sending " << bytesToSend << " bytes of data (" << bytesQueued << " bytes already queued). TcpSocket errno == " << socket->GetErrno ());
            }
        }


      /// This last send time record the time to send out the previous ADU, this is the case for both client and server send
      m_clientSite.clientSentTime = Simulator::Now ().GetSeconds ();
      NS_LOG_DEBUG ("The clientContaienr send time " << m_clientSite.clientSentTime);


          //NS_LOG_LOGIC ("This is the pipelining request option");
          //Simulator::ScheduleNow (&YoutubeController::ScheduleNextClientSend, this, socket);

      /// When it is not pipelining, we wait for the previous response Message to arrive before sending next request
    //}

  // Wait for send
 /* else
    {
      NS_LOG_LOGIC ("No request to work on now, waiting for the next web page");
    }
   */
}

bool
YoutubeController::CheckNextRequest (YoutubeSite & site)
{
  NS_LOG_FUNCTION (this);
  for (deque<YoutubeMessage>::iterator i = site.request.begin (); i != site.request.end (); ++i)
    {
      if (i->messageType == YoutubeMessage::REQUEST)
        {
          return true;
        }
    }
  return false;
}


void
YoutubeController::ScheduleNextClientSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  // It's expected that this won't be called if m_serverContainer is
  // empty, since the last ADU should have size==0, serve as the FIN message
  NS_ASSERT (!m_clientSite.objects.empty ());

//============================
// Pippeling Case
//============================
  if (m_pipelining)
    {
	  NS_LOG_LOGIC ("This is the pipelining request option");
      m_delayTime = m_clientSite.objects.front ().requestObjectGapTime;


      if (m_delayTime > 0)
        {
          if (m_clientSite.clientSentTime < Seconds (0))
            {
              NS_FATAL_ERROR ("client_send_wait: haven't sent anything yet. Adu container corrupted or wrong?");
            }
          else
            {
              // (m_lastSendTime + adu.requestGapTime) is the earliest
              // time at which the data shall be sent.
              double sendTime = m_clientSite.clientSentTime + m_delayTime;
              NS_LOG_DEBUG ("The send time " << sendTime << " Now time " << Simulator::Now ().GetSeconds ());
              if (sendTime >= Simulator::Now ().GetSeconds ())
                {
                  NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend at " << sendTime << "s");
                  Simulator::Schedule (Seconds (sendTime - Simulator::Now ().GetSeconds ()),
                                       &YoutubeController::ClientSend, this, socket);
                }
              else
                {
                  NS_LOG_LOGIC ("client_send_wait: Scheduling ClientSend immediately (was overdue by " << Simulator::Now ().GetSeconds () - sendTime << "s");
                  Simulator::ScheduleNow (&YoutubeController::ClientSend, this, socket);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("neither client_recv_wait nor client_send_wait: calling ClientSend");
          Simulator::ScheduleNow (&YoutubeController::ClientSend, this, socket);
        }
    }
//============================
// No nPippeling Case
//============================

  else
    {
      // Non-pipelining

      NS_LOG_LOGIC ("This is the non-pipelining option, need to determine if it is persistent connection or not");

      /// For the non-persistent option to work, we need to first have the pipelining as false
      if (!m_persistent)
        {
          NS_LOG_DEBUG ("Start a new connection for next client sent");
          StartNewClientSocket ();
        }
      else
        {
          NS_LOG_DEBUG ("This is the persistent connection without pipelining");
          Simulator::ScheduleNow (&YoutubeController::ClientSend, this, socket);
        }
    }
}

void
YoutubeController::CallClientSend ()
{
  NS_LOG_FUNCTION (this);
  ClientSend (m_clientSocket);
}

void
YoutubeController::SetClientApplication (Ptr<Node> node, uint32_t id)
{
  NS_LOG_FUNCTION (this);
  m_clientNode = node;
  m_clientId = id;
}

void
YoutubeController::SetServerApplication (Ptr<Node> node, uint32_t id)
{
  NS_LOG_FUNCTION (this);
  m_serverNode = node;
  m_serverId = id;
}

void
YoutubeController::SetClientSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  m_clientSocket = socket;
}

void
YoutubeController::SetServerSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  m_serverSocket = socket;
}

void
YoutubeController::StartNewClientSocket ()
{
  NS_LOG_FUNCTION (this);

  Ptr<YoutubeClient> youtubeClient = DynamicCast<YoutubeClient>
      (m_clientNode->GetApplication (m_clientId));

  youtubeClient->StartNewSocket ();
}




void
YoutubeController::ScheduleNextServerSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  // It's expected that this won't be called if m_serverContainer is
  // empty, since the last ADU should have size==0, serve as the FIN message
  //NS_ASSERT (!m_serverSite.objects.empty ());
  socket =m_serverSocket;

  m_delayTime = m_serverSite.objects.front ().serverDelayTime;

  NS_LOG_DEBUG ("The server delay time " << m_delayTime);
  if (m_delayTime > 0)
    {
      // time at which the data may be sent.
      double sendTime = Simulator::Now ().GetSeconds () + m_delayTime;

      NS_LOG_DEBUG ("The send time here " << sendTime << " The now " << Simulator::Now ().GetSeconds ());

      if (sendTime >= Simulator::Now ().GetSeconds ())
        {
          NS_LOG_LOGIC ("SCHEDULE TX: server send wait: Scheduling DoSend at " << sendTime << "s");
          Simulator::Schedule (Seconds (m_delayTime), &YoutubeController::ServerSend, this, socket);
        }
      else
        {
          // past due because that we are doing pipelining
          NS_LOG_LOGIC ("NOW TX: server_send_wait: Scheduling ServerSend immediately (was overdue by " << Simulator::Now ().GetSeconds () - sendTime << "s");
          Simulator::ScheduleNow (&YoutubeController::ServerSend, this, socket);
        }
    }
  else
    {
      NS_LOG_LOGIC ("NOW TX: neither server_recv_wait nor server_send_wait: calling ServerSend");
      // When both the server_recv_wait and server_recv_wait times are zero, it
      // means to send those bytes immediately.
      Simulator::ScheduleNow (&YoutubeController::ServerSend, this, socket);
    }
}




void
YoutubeController::ServerSend (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  //NS_ASSERT (!m_serverContainer.objects.empty ());
    YoutubeMessage objectMessage = m_serverSite.objects.front ();

  // When we call Server Send function, the first ADU should be response or the terminator
  //NS_ASSERT (objectMessage.messageType == YoutubeMessage::RESPONSE);
    uint32_t bytesToSend;
    uint32_t bytesQueued = 0;
    uint32_t ipTos = 56;
    socket->SetIpTos(ipTos);
    bytesToSend = objectMessage.size;
  // remove the ADU we are about to send

    NS_LOG_DEBUG (" **** TX Availavility "<< socket->GetTxAvailable());
  while (bytesToSend)
    {


	  int bytesAccepted;
      if (bytesToSend > m_segmentSize)
        {
    	  Ptr<Packet> packet = Create<Packet> (m_segmentSize);
          bytesAccepted = socket->Send (packet);
        }
      else
        {
    	  Ptr<Packet> packet = Create<Packet> (bytesToSend);
          bytesAccepted = socket->Send (packet);        }

      if (bytesAccepted > 0)
        {

    	  bytesToSend -= bytesAccepted;
          bytesQueued += bytesAccepted;
          m_serverSite.remainingPageSize-= bytesAccepted;

        }
      else if (socket->GetErrno () == TcpSocket::ERROR_MSGSIZE)
        {
          // This part might have better way
    	  //NS_LOG_LOGIC ("Send buffer filled while sending ADU  bytes already queued). Increase TcpSocket's SndBufSize.");
         // NS_FATAL_ERROR ("Send buffer filled while sending ADU  bytes already queued). Increase TcpSocket's SndBufSize.");
    	  socket->SetSendCallback ( MakeCallback (&YoutubeController::DataSendServer, this));

    	  break;

        }
      else if (socket->GetErrno () == TcpSocket::ERROR_NOTCONN)
        {
          // This means that the socket was already closed by us for sending, somehow.
          //NS_FATAL_ERROR ("ERROR_NOTCONN while sending - this is a bug");
    	  break;
        }
      else
        {
          NS_FATAL_ERROR ("ERROR sending " << bytesToSend << " bytes of data (" << bytesQueued << " bytes already queued). TcpSocket errno == " << socket->GetErrno ());
          break;
        }
    }

}

void
YoutubeController::DataSendUser (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

    // Only send new data if the connection has completed
      //Simulator::ScheduleNow (&YoutubeController::CallControllerUser, this);
  Simulator::ScheduleNow (&YoutubeController::ClientSend, this, m_clientSocket);
}

void
YoutubeController::CallControllerUser (void)
{

	ClientSend(m_clientSocket);
}



void
YoutubeController::DataSendServer (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("=======================================================>>>>>>>>>>>>>>>>>>>>>>>>>>>> Buffer SEND");
    // Only send new data if the connection has completed
      //Simulator::ScheduleNow (&YoutubeController::CallControllerServer, this);
  Simulator::ScheduleNow (&YoutubeController::ServerSend, this, m_serverSocket);
}

void
YoutubeController::CallControllerServer (void)
{

	ServerSend(m_serverSocket);
}



void
YoutubeSite::Clear ()
{
  NS_LOG_FUNCTION (this);
  startTime = 0;
  serverReceiveTime = 0;
  clientSentTime = 0;
  requestPageGaps= 0;
  objects.clear();
  request.clear();
  remainingPageSize = 0;
  totalPageSize= 0;
  firstObject = true;
  pageNumber = 0;


}







} // namespace ns3
