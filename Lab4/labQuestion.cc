#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lab_question_script");

int
main (int argc, char *argv[])
{
  // For verbose output and number of csma nodes.
  bool verbose = true;
  uint32_t nCsma = 3;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  // Creating Node Container to House Nodes-0,1.
  NodeContainer p2pNodes;
  p2pNodes.Create(2);

  // Creating Node Container to House Nodes-1,2,3,4.
  NodeContainer csmaNodesOne;
  csmaNodesOne.Add(p2pNodes.Get(1));
  csmaNodesOne.Create(nCsma);

  // Creating Node Container to House Nodes-5,6,7,0.
  NodeContainer csmaNodesTwo;
  csmaNodesTwo.Create(nCsma);
  csmaNodesTwo.Add(p2pNodes.Get(0));

  // Specifying Point to Point Link Attributes.
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Installing Net Devices.
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Specifying csma channel Attributes.
  CsmaHelper csmaOne;
  csmaOne.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmaOne.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  CsmaHelper csmaTwo;
  csmaTwo.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmaTwo.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // Installing NetDevices.
  NetDeviceContainer csmaDevicesOne,csmaDevicesTwo;
  csmaDevicesOne = csmaOne.Install(csmaNodesOne);
  csmaDevicesTwo = csmaTwo.Install(csmaNodesTwo);

  // Installing Protocol Stack on p2pNodes and csma nodes.
  InternetStackHelper stack;

  stack.Install(csmaNodesOne);
  stack.Install(csmaNodesTwo);

  // Specifying IP Assignments for point to point nodes.
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  // Specifying IP Assignments for csma nodes.
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfacesOne;
  csmaInterfacesOne = address.Assign (csmaDevicesOne);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfacesTwo;
  csmaInterfacesTwo = address.Assign (csmaDevicesTwo);

  // Creating Server on Node-3 with port 93 and Installing Server Apps.
  UdpEchoServerHelper echoServer (93);

  ApplicationContainer serverApps = echoServer.Install(csmaNodesOne.Get(nCsma-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // Creating Client on Node-2
  UdpEchoClientHelper echoClient (csmaInterfacesOne.GetAddress (nCsma-1), 93);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (csmaNodesTwo.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // To Enable Routing.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // For tcp-dump traces of client and server.
  csmaOne.EnablePcap ("client", csmaDevicesOne.Get (1), true);
  csmaTwo.EnablePcap ("server", csmaDevicesTwo.Get (2), true);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
