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

#include "youtube-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"
#include <iostream>

#include "ns3/youtube-client.h"
#include "ns3/youtube-server.h"
#include "ns3/youtube-controller.h"

namespace ns3 {

YoutubeHelper::YoutubeHelper ()
{
  m_factory.SetTypeId (ns3::YoutubeController::GetTypeId ());
  m_controller = m_factory.Create<YoutubeController> ();
}

void
YoutubeHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

Ptr<YoutubeController>
YoutubeHelper::GetController (void)
{
  return m_controller;
}

//------------------------------------------------------------------------------------------------------
YoutubeServerHelper::YoutubeServerHelper ()
{
  m_factory.SetTypeId (YoutubeServer::GetTypeId ());
}

void
YoutubeServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
YoutubeServerHelper::Install (Ptr<Node> node)
{
	m_server = InstallPriv(node)->GetObject<YoutubeServer>();

return ApplicationContainer (m_server);

}

ApplicationContainer
YoutubeServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
	  apps.Add (InstallPriv (*i));
    }
  return apps;
}

Ptr<Application>
YoutubeServerHelper::InstallPriv (Ptr<Node> node) const
{
Ptr<YoutubeServer> server = m_factory.Create<YoutubeServer> ();
uint32_t index = node->AddApplication (server);

server->SetIndex(index);

return server;
}

Ptr<YoutubeServer>
YoutubeServerHelper::GetServer (void)
{
  return m_server;
}


//------------------------------------------------------------------------------------------------------
YoutubeClientHelper::YoutubeClientHelper ()
{
  m_factory.SetTypeId (YoutubeClient::GetTypeId ());
}

void
YoutubeClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
YoutubeClientHelper::Install (Ptr<Node> node)
{

	return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
YoutubeClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
	  apps.Add (InstallPriv (*i));
    }
  return apps;
}


Ptr<Application>
YoutubeClientHelper::InstallPriv (Ptr<Node> node) const
{

	Ptr<YoutubeClient> client = m_factory.Create<YoutubeClient> ();

	uint32_t index = node->AddApplication (client);
	client->SetIndex(index);

	return client;
}

} // namespace ns3
