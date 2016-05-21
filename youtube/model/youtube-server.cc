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

#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/pointer.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/boolean.h"

//#include "Youtube-client.h"
#include "youtube-server.h"
#include "youtube-controller.h"

#include "ns3/tcp-socket.h"

#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"


NS_LOG_COMPONENT_DEFINE ("YoutubeServer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (YoutubeServer);

TypeId
YoutubeServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YoutubeServer")
    .SetParent<Application> ()
    .AddConstructor<YoutubeServer> ()
    .AddAttribute ("YoutubeController", "The controller",
                   PointerValue (0),
                   MakePointerAccessor (&YoutubeServer::SetController,
                                        &YoutubeServer::GetController),
                   MakePointerChecker<YoutubeController> ())
    .AddAttribute ("PageTimeout", "Time out value for each one web page",
                   UintegerValue (1),
                   MakeUintegerAccessor (&YoutubeServer::m_pageTimeout),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Local", "The local address for server",
                   AddressValue (),
                   MakeAddressAccessor (&YoutubeServer::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("TransportId", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&YoutubeServer::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Persistent", "Set if the connection is persistent connection or not.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&YoutubeServer::m_persistent),
                   MakeBooleanChecker ())
  ;
  return tid;
}

YoutubeServer::YoutubeServer ()

{
NS_LOG_FUNCTION (this);
m_connected = false;
 m_firstConnection = false;

}

YoutubeServer::~YoutubeServer ()
{
	 NS_LOG_FUNCTION (this);
  m_firstConnection = false;
}

void
YoutubeServer::SetController (Ptr<YoutubeController> controller)
{
  NS_LOG_FUNCTION (this);
  m_controller = controller;
}

Ptr<YoutubeController>
YoutubeServer::GetController () const
{
  NS_LOG_FUNCTION (this);
  return m_controller;
}

void YoutubeServer::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  // Called at time specified by Start
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);

      uint32_t ipTos = 56;
      m_socket->SetIpTos(ipTos);
      bool ipRecvTos = true;
      m_socket->SetIpTos(ipRecvTos);
      m_socket->Bind (m_local);
      //m_socket->Bind ();
      m_socket->Listen ();

    }
  NS_ASSERT (m_socket != 0);
  m_socket->SetAcceptCallback (MakeCallback (&YoutubeServer::ConnectionRequested, this),
                               MakeCallback (&YoutubeServer::ConnectionAccepted, this));
  m_socket->SetSendCallback ( MakeCallback (&YoutubeServer::DataSend, this));
  m_controller->SetServerSocket (m_socket);
  /// By this time save the serve application information in the controller
  SetServerApplication ();

  NS_LOG_LOGIC (Simulator::Now () << " Server ready waiting for Client request ...");
}

void YoutubeServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  // Called at time specified by Stop
  NS_LOG_DEBUG (Simulator::Now () << " Server: Stop Application");
  m_connected = false;
  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
YoutubeServer::SetIndex (uint32_t index)
{
	m_index = index;
}


void
YoutubeServer::SetServerApplication ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = GetNode ();
  /*for (uint32_t i = 0; i < node->GetNApplications (); ++i)
    {
      if (node->GetApplication (i) == this)
        {
          m_controller->SetServerApplication (node, i);
        }
    }
  */
  m_controller->SetServerApplication (node, m_index);
}

bool
YoutubeServer::ConnectionRequested (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  NS_LOG_DEBUG (Simulator::Now () << " Socket = " << socket << " " << " Server: ConnectionRequested");
  return true;
}

void
YoutubeServer::ConnectionAccepted (Ptr<Socket> socket, const Address& address)
{
  NS_LOG_FUNCTION (this << socket << address);
  /*
   * We have a new socket and need to set the receive callback and respond with the response
   * when the request arrives
   */

  NS_LOG_DEBUG (socket << " " << Simulator::Now () << " Server::Successful socket id : " << socket << " Connection Accepted");

  m_socket = socket;
  m_controller->SetServerSocket (m_socket);
  // Now we've blocked, so register the up-call for later
  m_socket->SetRecvCallback (MakeCallback (&YoutubeServer::ServerReceive, this));
  m_connected = true;
  if (m_firstConnection)
    {
      m_firstConnection = false;
      /// This is the first connection accepted, call the first web session
      //m_controller->StartFirstWebSession ();
      m_controller->CallClientSend();
    }
  else
    {
	  NS_LOG_FUNCTION (this << "!!!! Persistent" << address);
	  //NS_ASSERT (!m_persistent);
      /// After the new connection is established, we start the next client send immediately
      m_controller->CallClientSend();
    }
}
//=====>
void
YoutubeServer::ServerReceive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet = socket->Recv ();
  /// We should not have a null packet
  if (packet == NULL)
    {
      NS_LOG_WARN ("We should not receive this part");
      return;
    }

  YoutubeSite serverSite = m_controller->GetServerSite();
  uint32_t bytesToRecv = packet->GetSize ();


  while (bytesToRecv)
    {
      uint32_t requestSize = serverSite.request.front().size; //FindFirstRequestSize (serverSite);
      NS_LOG_DEBUG ("the request size " << requestSize);

      if (!requestSize)
        {
          //NS_FATAL_ERROR ("This should not happen");
          //return;
    	  break;
        }

    //====================================

      if (requestSize > bytesToRecv)
        {
          // Received less than a whole Request Message. Reduce its size by the number of bytes received.
          requestSize -= bytesToRecv;
    	  serverSite.request.front().size -= bytesToRecv;
          m_controller->SetServerSite(serverSite);
          //UpdateFirstRequestSize (serverSite, requestSize);
          NS_LOG_LOGIC ("RECEIVED " << bytesToRecv << " bytes of an Message. " << serverSite.request.front ().size << " bytes to go. Of: "<< serverSite.request.size() <<" Objets");
          break;
        }
      else
        {
    	  NS_LOG_LOGIC ("All RECEIVED: remove the whole thing and continue receiving: "<< serverSite.request.size());
    	  // Front Request Message size <= bytesToRecv: remove the whole thing and continue receiving
          bytesToRecv -= requestSize;
          // Remove the next request message
         // serverSite.request.front().
          //RemoveFirstRequestSize (serverSite);
          serverSite.request.pop_front();
          serverSite.serverReceiveTime = Simulator::Now ().GetSeconds ();

          m_controller->GetClientSite ().timeSendChunk = Simulator::Now ().GetSeconds ();
          m_controller->ScheduleNextServerSend (socket);
          break;

          if (serverSite.request.size ())
            {
              // After receiving the request we need to schedule the web response
             // m_controller->ScheduleNextServerSend (socket);

            }

          continue;
        }
    }
}

void
YoutubeServer::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);

  if (m_connected)
    {
    // Only send new data if the connection has completed
      Simulator::ScheduleNow (&YoutubeServer::CallController, this);
    }

}

void YoutubeServer::CallController (void)
{
	m_controller->ServerSend(m_socket);
}

} // namespace ns3
