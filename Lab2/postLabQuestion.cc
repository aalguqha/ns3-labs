#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("atifcppprogrammer_post_lab");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Creating Node Container for Our Two Client Nodes and One Server Node.
  NodeContainer allNodes;
  allNodes.Create(3);

  // Node Container for first Client-Server Point to Point Link.
  NodeContainer nodesOne;
  nodesOne = NodeContainer(allNodes.Get(0),allNodes.Get(1));

  // Node Container for second Client-Server Point to Point Link.
  NodeContainer nodesTwo;
  nodesTwo = NodeContainer(allNodes.Get(1),allNodes.Get(2));

  // Specifying PointToPoint Link for nodesOne Container.
  PointToPointHelper pointToPointOne;
  pointToPointOne.SetDeviceAttribute("DataRate",StringValue("8Mbps"));
  pointToPointOne.SetChannelAttribute("Delay",StringValue("8ms"));

  // Specifying PointToPoint Link for nodesTwo Container.
  PointToPointHelper pointToPointTwo;
  pointToPointOne.SetDeviceAttribute("DataRate",StringValue("4Mbps"));
  pointToPointOne.SetChannelAttribute("Delay",StringValue("4ms"));

  // Installing Network-Cards on nodesOne.
  NetDeviceContainer devicesOne;
  devicesOne = pointToPointOne.Install(nodesOne);

  // Installing Network-Cards on nodesTwo.
  NetDeviceContainer devicesTwo;
  devicesTwo = pointToPointTwo.Install(nodesTwo);

  // Installing Protocol Stack on First Client and Server;
  InternetStackHelper stackOne;
  stackOne.Install(nodesOne);

  // Installing Protocol Stack on Second Client.
  InternetStackHelper stackTwo;
  stackTwo.Install(nodesTwo.Get(1));

  // Specifying and Assigning IP-Addresses for First Client and Server Link.
  Ipv4AddressHelper addressOne;
  addressOne.SetBase ("192.168.1.0", "255.255.255.0","0.0.0.1");

  Ipv4InterfaceContainer interfacesOne = addressOne.Assign (devicesOne);

  // Specifying and Assigning IP-Addresses for Server and Second Client Link.
  Ipv4AddressHelper addressTwo;
  addressTwo.SetBase ("192.168.5.0", "255.255.255.0","0.0.0.1");

  Ipv4InterfaceContainer interfacesTwo = addressTwo.Assign (devicesTwo);

  // Installing UdpEchoServer on Central Node.
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodesOne.Get(1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (15.0));

  // Configuring and Installing First Client App.
  UdpEchoClientHelper echoClientOne (interfacesOne.GetAddress (1), 9);
  echoClientOne.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClientOne.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClientOne.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientAppsOne = echoClientOne.Install (nodesOne.Get (0));
  clientAppsOne.Start (Seconds (5.0));
  clientAppsOne.Stop (Seconds (15.0));

  // Configuring and Installing Second Client App.
  UdpEchoClientHelper echoClientTwo (interfacesTwo.GetAddress (0), 9);
  echoClientTwo.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClientTwo.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClientOne.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientAppsTwo = echoClientTwo.Install (nodesTwo.Get (1));
  clientAppsTwo.Start (Seconds (10.0));
  clientAppsTwo.Stop (Seconds (15.0));

  /* 9th-Septemeber-2019 - Programming-Credits - atifcppprogrammer */
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
