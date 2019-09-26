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
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

// Network topology (mine)
//
//        n2 n3 n4              .
//         \ | /                .
//          \|/                 .
//     n1--- n0---n5---n9       .
//          /|\                 .
//         / | \                .
//        n8 n7 n6              .
//


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Star");

int
main (int argc, char *argv[])
{

  // Adding Log Levels for Console Output.
  LogComponentEnable("Star",LOG_LEVEL_INFO);
  LogComponentEnable("PacketSink",LOG_LEVEL_INFO);
  LogComponentEnable("OnOffApplication",LOG_LEVEL_INFO);
  NS_LOG_INFO("Star Topolgy Simulation");

  //
  // Set up some default values for the simulation.
  //
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (137));

  // ??? try and stick 15kb/s into the data rate
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("14kb/s"));

  //
  // Default number of nodes in the star.  Overridable by command line argument.
  //
  uint32_t nSpokes = 8;

  CommandLine cmd;
  cmd.AddValue ("nSpokes", "Number of nodes to place in the star", nSpokes);
  cmd.Parse (argc, argv);

  // Specifying attributes for point tp point links of star topology.
  NS_LOG_INFO("Building Star Topology.");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  PointToPointStarHelper star (nSpokes, pointToPoint);

  // Installing internet stack on all nodes of start topology.
  NS_LOG_INFO("Installing internet stack on nodes of star topology");
  InternetStackHelper internet;
  star.InstallStack (internet);

  // Assigning IP Address to nodes of star topology.
  NS_LOG_INFO ("Assign IP Addresses.");
  star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

  // Creating point to point link for connection between node 5 and node 9.
  NodeContainer p2pNodes;
  p2pNodes.Add(star.GetSpokeNode(4));
  p2pNodes.Create(1);

  // Installing netdevices, internet stack and assigning ip's for this link.
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);

  internet.Install(p2pNodes.Get(1));

  Ipv4AddressHelper address;
  address.SetBase ("10.2.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (p2pDevices);

  // Installing Hub App on Node 9, (making it the packet sink).
  uint16_t port = 50000;
  Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAddress);
  ApplicationContainer hubApp = packetSinkHelper.Install (p2pNodes.Get(1));
  hubApp.Start (Seconds (1.0));
  hubApp.Stop (Seconds (10.0));

  // Installing on off applications.
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer spokeApps;

  for (uint32_t i = 0; i < star.SpokeCount (); ++i)
    {
      if(i == 0 || i == 4) continue;
      AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress(1), port));
      onOffHelper.SetAttribute ("Remote", remoteAddress);
      spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
    }
  spokeApps.Start (Seconds (1.0));
  spokeApps.Stop (Seconds (10.0));

  NS_LOG_INFO ("Enable static global routing.");
  //
  // Turn on global static routing so we can actually be routed across the star.
  //
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Enable pcap tracing.");
  //
  // Do pcap tracing on all point-to-point devices on all nodes.
  //
  pointToPoint.EnablePcapAll ("atif_star");

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
