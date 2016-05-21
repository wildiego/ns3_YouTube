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

#ifndef YOUTUBE_SERVER
#define YOUTUBE_SERVER

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include "ns3/nstime.h"
#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/socket.h"


#include "youtube-controller.h"


using namespace std;

namespace ns3 {

class YoutubeController;
//class YoutubeClient;





/**
 * \brief Implements the YoutubeServer applications.
 *
 * This class deals with Youtube server side operations, like receiving Youtube request and
 * sending back Youtube response.
 *
 * \see YoutubeHelper
 */
class YoutubeServer : public Application
{
public:


  static TypeId
  GetTypeId (void);

  /**
   * \brief Constructor.
   */
  YoutubeServer ();
  ~YoutubeServer ();

  void SetServerApplication ();
  /**
   * \brief Notify that the connection has requested
   * \param socket the socket we used for Youtube connection
   * \param address the address
   */
  bool ConnectionRequested (Ptr<Socket> socket, const Address& address);
  /**
   * \brief Notify that the new connection has been created
   * \param socket the socket we used for Youtube connection
   * \param address the address
   */
  void ConnectionAccepted (Ptr<Socket> socket, const Address& address);
  /**
   * \brief The server starts to receive packet from the client
   * \param socket the socket we used for Youtube connection
   */
  void ServerReceive (Ptr<Socket> socket);
  /**
   * \brief Find the first in-line request ADU size
   * \param requestSize The request ADU size
   */
  uint32_t FindFirstRequestSize (YoutubeSite & serverSite);
  /**
   * \brief Update the first in-line request ADU size
   * \param requestSize The request ADU size to be updated to
   */
  void UpdateFirstRequestSize (YoutubeSite & serverSite, uint32_t requestSize);
  /**
   * \brief Remove the first in-line request ADU
   */
  void RemoveFirstRequestSize (YoutubeSite & serverSite);
  /**
   * Set and get the Youtube controller
   */
  void SetController (Ptr<YoutubeController> r);
  Ptr<YoutubeController> GetController () const;
  //void PrintContainer (const AduContainer & container);

  void SetIndex (uint32_t index);

private:
  void CallController (void);

  /**
   * \brief Send more data as soon as some has been transmitted.
   */
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback

  /**
   * \brief Called at time specified by Start to start application
   */
  virtual void StartApplication (void);
  /**
   * \brief Called at time specified by Stop to terminate application
   */
  virtual void StopApplication (void);
  uint32_t 				m_index;

  bool                          m_firstConnection;              ///< The first connection or not
  bool                          m_persistent;                   ///< The persistent connection or not
  Address                       m_local;                        ///< Address where this YoutubeServer application is running.
  Ptr<YoutubeController>           m_controller;                   ///< The pointer to Youtube controller
  Ptr<Socket>                   m_socket;                       ///< The socket to work with
  uint32_t                      m_pageTimeout;                  ///< The page time out value
  uint32_t                      m_run;                          ///< The number of simulation runs
  TypeId                        m_tid;                          ///< The transport protocol id to pass in (default. tcp)
  bool 							m_connected;
};

} // namespace ns3


#endif  /* YOUTUBE_SERVER */
