#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("atifcppprogrammer_lab_question");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Creating Three Nodes Required by Topology.
  NodeContainer allNodes;
  allNodes.Create(3);

  // Node Container for Client and Router.
  NodeContainer clientRouterNodes = NodeContainer(allNodes.Get(0),
    allNodes.Get(1));

  // Node Container for Server and Router.
  NodeContainer routerServerNodes = NodeContainer(allNodes.Get(1),
    allNodes.Get(2));

  // Specifying Point to Point Link Between Client and Router.
  PointToPointHelper pointToPointOne;
  pointToPointOne.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
  pointToPointOne.SetChannelAttribute("Delay",StringValue("2ms"));

  // Specifying Point to Point Link Between Server and Router.
  PointToPointHelper pointToPointTwo;
  pointToPointTwo.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
  pointToPointTwo.SetChannelAttribute("Delay",StringValue("2ms"));

  // Installing Net-Devices for Client Rotuer Link.
  NetDeviceContainer devicesOne;
  devicesOne = pointToPointOne.Install(clientRouterNodes);

  // Installing Net-Devices for Server Router Link.
  NetDeviceContainer devicesTwo;
  devicesTwo = pointToPointTwo.Install(routerServerNodes);

  // Installing Protocol Stack on Client and Router.
  InternetStackHelper stackOne;
  stackOne.Install(clientRouterNodes);

  // Installing Protocol Stack on Server.
  InternetStackHelper stackTwo;
  stackTwo.Install(routerServerNodes.Get(1));

  // Setting Address for Client Router Link.
  Ipv4AddressHelper addressOne;
  addressOne.SetBase ("192.168.40.0", "255.255.255.0","0.0.0.1");

  Ipv4InterfaceContainer interfacesOne = addressOne.Assign (devicesOne);

  // Setting Address for Server Router Link.
  Ipv4AddressHelper addressTwo;
  addressTwo.SetBase ("192.168.41.0", "255.255.255.0","0.0.0.1");

  Ipv4InterfaceContainer interfacesTwo = addressTwo.Assign (devicesTwo);

  // Installing Server-Apps.
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (routerServerNodes.Get(1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // Installing Client-Apps.
  UdpEchoClientHelper echoClient (interfacesTwo.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (clientRouterNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // To Enable Routing.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
