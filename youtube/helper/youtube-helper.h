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

#ifndef YOUTUBE_HELPER_H
#define YOUTUBE_HELPER_H

#include <stdint.h>
#include <vector>
#include <string>

#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"
//#include "ns3/Youtube-client.h"
//#include "ns3/Youtube-server.h"
//#include "ns3/Youtube-controller.h"


namespace ns3 {

class YoutubeController;
class YoutubeServer;
class YoutubeClient;
/**
 * \brief Create a server application which waits for input Youtube packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class YoutubeHelper
{
public:
  /**
   * Create YoutubeServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  YoutubeHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  Ptr<YoutubeController> GetController (void);
private:
  ObjectFactory m_factory;
  Ptr<YoutubeController> m_controller;
};

/**
 * \brief Create a server application which waits for input Youtube packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class YoutubeServerHelper
{
public:
  /**
   * Create YoutubeServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  YoutubeServerHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  /**
   * Create one Youtube server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param node The node on which to create the Applications.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (Ptr<Node> node);
  /**
   * Create one udp server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);
  /**
   * Link to Server
   * YoutubeServer ptr->
   */
   Ptr<YoutubeServer> GetServer (void);
private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;
  YoutubeHelper youtubeHelper;
  Ptr<YoutubeServer> m_server;
};

/**
 * \brief Create a client application which sends udp packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class YoutubeClientHelper
{

public:
  /**
   * Create YoutubeClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  YoutubeClientHelper ();
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  /**
   * Create one Youtube client application on each of the Nodes in the
   * NodeContainer.
   *
   * \param node The node on which to create the Applications.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (Ptr<Node> node);
  /**
     * \param c the nodes
     *
     * Create one udp client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);


private:

  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
  //Ptr<YoutubeClient> m_client;
  YoutubeHelper youtubeHelper;
};

} // namespace ns3

#endif /* YOUTUBE_HELPER_H */
