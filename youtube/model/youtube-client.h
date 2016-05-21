

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

#ifndef YOUTUBE_CLIENT
#define YOUTUBE_CLIENT

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

#include "ns3/nstime.h"
#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/socket.h"

#include "youtube-controller.h"
#include "youtube-server.h"

#include "ns3/ipv4-interface.h"
#include "ns3/ipv4.h"

using namespace std;

namespace ns3 {

class YoutubeController;
// YoutubeServer;
class Socket;
class RandomVariableStream;



/**
 * \brief Implements the YoutubeClient client and server applications.
 *
 * It is difficult and probably unnecessary to use this class
 * directly; see TmixHelper or TmixTopology instead.
 *
 * \see YoutubeHelper
 */
class YoutubeClient : public Application
{
public:
	/**
	* \brief Get the type ID.
	* \return the object TypeId
	*/
  static TypeId
  GetTypeId (void);

  /**
   * \brief Constructor.
   */
  YoutubeClient ();
  /**
   * \brief Destructor.
   */
  virtual ~YoutubeClient ();

  /**
   * \brief Return a pointer to associated socket.
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;




  /**
   * \brief Set and get the Youtube controller
   */
  void SetController (Ptr<YoutubeController> r);
  Ptr<YoutubeController> GetController () const;
  /**
   * \brief Set and get the Youtube server
   */
  void SetServer (Ptr<YoutubeServer> s);
  Ptr<YoutubeServer> GetServer () const;
  /**
   * \brief Generate the Traffic (objets, timeGap, etc)
   * it generates Youtube traffic
   */
    void TrafficGeneration ();


  /**
   * \brief Schedule an adu container for immediate execution.
   *
   * You must specify the TCP port it will connect to listen on; make sure it won't be in use.
   * \param requestAdu request ADU for HTTP request
   * \param responseAdu response AUD for HTTP response
   * \param requestPageGap the gap time among web pages
   * \param container ADU container to execute immediately.
   */
  void StartVideo (YoutubeSite site);
  // Event handlers
  /**
   * \brief Start web nav
   */
  void StartSending ();
  /**
   * \brief Send more data as soon as some has been transmitted.
   */

 //==============================
  void  Stadistics ();

  void StartNewSocket ();



  void StartFirstWebSession ();

  void SetClientApplication ();
  /**
   * \brief Notify the completion of connection
   * \param socket the socket that get connected
   */
  void ConnectionComplete (Ptr<Socket> socket);
  /**
   * \brief Notify the failure of connection
   * \param socket the socket that failed the connected
   */
  void ConnectionFailed (Ptr<Socket> socket);
  /**
   * \brief Schedule a adu container for immediate execution.
   *
   * \param container The ADU container to start sending
   */
  //void StartWebNavigation  (const AduContainer & container);
  //void StartAduContainer (const AduContainer & container);
  /**
   * \brief Record the web page gap time.
   * \param pageGapTime the gap time among web pages
   */
  void RecordPageGapTime (double pageGapTime);
  /**
   * \brief Send out the first object for one web page
   * \param socket The socket for this connection
   * \param container The ADU container
   */
  // void FirstObject (Ptr<Socket> socket, const AduContainer & container, bool firstPage);
  /**
   * \brief Call the time out function and time out the ongoing web page and record the results
   * \param socket The socket for the connection
   */
  void CallTimeOut (Ptr<Socket> socket);
//  void TestAduGeneration ();

  /**
   * \brief Schedule an adu container for immediate execution.
   *
   * You must specify the TCP port it will connect to listen on; make sure it won't be in use.
   * \param requestAdu request ADU for HTTP request
   * \param responseAdu response AUD for HTTP response
   * \param requestPageGap the gap time among web pages
   * \param container ADU container to execute immediately.
   */
  // void ConstructAndStartAdu (ADU requestAdu, ADU responseADU, double requestPageGap, AduContainer & container);

  /**
   * \brief Set the file name for recording object deliver ratio and page delay
   * \param filename the file name for output
   */
  void SetFileName (string filename);
  /**
   * \brief Record the parameter for the Internet mode traffic
   * \param requestAdu The web request ADU
   * \param responseAdu The web response ADU
   */
  //void RecordParameter (ADU requestAdu, ADU responseAdu);
  /**
   * \brief Record the results after finishing each web page
   * \param delay the delay to load each individual web page, one page may include multiple web objects
   * \param odr the object delivery ratio for the web page
   */
  void RecordResults (double delay, double odr);
  /**
   * \brief Record the results after finishing each web page
   * \param socket the socket that sending the web page
   */
  void FinishOnePage (Ptr<Socket> socket);
  /**
   * \brief It deals with web client receiving web pages
   * \param socket the socket that sending the web page
   */
  void ClientReceive (Ptr<Socket> socket);
  /**
   * \brief Notify the completion of web sessions
   */
  void SessionCompleted ();

  void SetIndex (uint32_t index);

  /**
   * Print the container adu sizes
   * \param container the adu container used in the simulation
   */
  // void PrintContainer (const AduContainer & container);

//==================================
private:


  void CallController (void);
  /**
   * \brief Send more data as soon as some has been transmitted.
   */
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
  /**
   * Call at time when starting application, inherited from application
   */
  virtual void StartApplication (void);
  /**
   * Called at time specified by Stop to terminate application, inherited from application
   */
  virtual void StopApplication (void);

  /// Traced Callback: transmitted packets.
  //TracedCallback<Ptr<const Packet> > m_txTrace;
  Address 			m_addClient;

  bool                  m_firstVideo;              			///< Whether this is the first transport connection or not
  Ptr<YoutubeController>   m_controller;                   ///< the pointer for Youtube controller
  Ptr<YoutubeServer> 	m_server;
  Ptr<Socket>           m_socket;                       ///< Associated socket
  bool  	  	  	  	m_connected;
  TypeId                m_tid;                          ///< The transport protocol id to pass in (default. tcp)
  EventId         		m_startStopEvent;     			//!< Event id for next start or stop event
  EventId         		m_sendEvent;   					 //!< Event id of pending "send packet" event
  uint32_t 				m_index;
  bool                  m_firstChunk;              ///< Whether this is the first transport connection or not
  double 				m_timeBuffer;
  double				m_timeVideo;
  double				m_timeStartVideo;
  double				m_videoSize;
  double				m_chunkNumber;
  double				m_pauseTime;
  double				m_pauseStart;
  double				m_pauseTimes;
  bool 					m_tagPause;
  uint32_t				m_userID;
  bool                  m_realFirstObject;
  double				m_lastNow;
  double				m_pauseTotal;
  bool  				m_firstWrite;
  double  				m_endDl; 				///< END video download time
  double  				m_timeClick; 			///< Record time click video
  uint32_t 				m_itag1; 				///< Record itag 1 number for each video
  uint32_t 				m_itag2; 				///< Record itag 2 number for each video
  uint32_t 				m_itag3; 				///< Record itag 3 number for each video
  uint32_t 				m_itag4; 				///< Record itag 4 number for each video
  double 				m_endVideo; 			///< END video reproduction time
  uint32_t 				m_chunkSize; 				///< Record itag 1 number for each video
  ///< Whether this is the first transport connection or not
  Address               m_local;                        ///< Address where this HttpClient application is running.
  Address               m_peer;                         ///< Address where the corresponding HttpClient application is running.
  vector<uint32_t>      m_objectsPerPage;               ///< The objects per page
  vector<vector<int> >  m_topSites;               ///< The objects per page
  int					m_siteID;
  vector<uint32_t>      m_mainObjectsPerPage;           ///< The Main objects per page
  bool                  m_lastItem;                     ///< The last page in one container
  uint32_t              m_sessionNumber;                ///< Record the session number for this open socket
  uint32_t              m_maxSessions;                  ///< The maximum session number to run for the simulation
  bool                  m_pipelining;                   ///< The boolean value for pipelining or not
  bool                  m_persistent;                   ///< The boolean value for persistent connection or not
  EventId               m_timeoutEvent;                 ///< Event id to schedule timeout
  uint32_t              m_Timeout;                  ///< The page time out value
  ostringstream         ClientFile1;                    ///< Record object delivery ratio
  ostringstream         ClientFile2;                    ///< Record the delay
  ostringstream         ClientFile3;                    ///< Record the distribution generated parameters
  ostringstream         ClientFile4;                    ///< Record the web page request gap
  /**
   * The model uses the request/response size for all the objects when using the transaction mode
   */
  uint32_t              m_userRequestSize;              ///< The user defined request size
  double                m_userServerDelay;              ///< The user defined server delay
  list<double>        	m_PageRequestGap;				/// Page Gap to compute delay

};

} // namespace ns3


#endif
