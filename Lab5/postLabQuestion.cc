#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Star");

int
main (int argc, char *argv[])
{
  // Enabling log components for console output.
  LogComponentEnable("Star",LOG_LEVEL_INFO);
  LogComponentEnable("PacketSink",LOG_LEVEL_INFO);
  LogComponentEnable("OnOffApplication",LOG_LEVEL_INFO);

  // Check to see if loging is enabled.
  NS_LOG_INFO("atifcppprogrammer's post_lab_question");

  // Setting up default values for the simulation.
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (137));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("14kb/s"));

  // Defining spoke count for star and node count for csma.
  uint32_t nSpokes = 8,nCsma = 4;

  // Case user wishes to ammend above spoke and csma counts at the outset.
  CommandLine cmd;
  cmd.AddValue ("nSpokes", "Number of nodes to place in star", nSpokes);
  cmd.AddValue ("nCsma", "Number of nodes to place in csma", nCsma);
  cmd.Parse (argc, argv);

  // Building our star topology.
  NS_LOG_INFO(" 1. Building our star topology");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay",StringValue("2ms"));
  PointToPointStarHelper star (nSpokes,pointToPoint);

  // Creating nodes for csma channel.
  NS_LOG_INFO(" 2. Creating nodes for csma channel");
  NodeContainer csmaNodes;
  csmaNodes.Create(nCsma);

  // Defining attributes of csma channel.
  NS_LOG_INFO(" 3. Defining attributes of csma channel");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // Installing network cards on csma channel.
  NS_LOG_INFO(" 4. Installing network cards for csma nodes");
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  // Installing internet stack on star and csma nodes.
  NS_LOG_INFO(" 5. Installing protocol stack on star and csma nodes");
  InternetStackHelper stack;
  stack.Install(csmaNodes); star.InstallStack(stack);

  // Creating node container for nodes 5 and 9.
  NS_LOG_INFO(" 6. Create node container for node 5 of star and node 9 of csma");
  NodeContainer p2pNodes = NodeContainer(star.GetSpokeNode(4),csmaNodes.Get(0));

  // Installing Network Cards for p2p nodes.
  NS_LOG_INFO(" 7. Installing additional network cards on p2p nodes");
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);

  // Assigning IP Addresses for star topology.
  NS_LOG_INFO("10. Assigning IP Address for star nodes");
  star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

  // Assigning IP Addresses  for p2p nodes.
  NS_LOG_INFO("11. Assigning IP Address for p2p nodes");
  Ipv4AddressHelper address;
  address.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  // Assigning IP Addresses  for csma nodes.
  address.SetBase ("10.3.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  // Creating Applications.
  NS_LOG_INFO("12. Creating OnOff and Hub Applications");

  // Creating a packet sink on node 12 of csma channel.
  uint16_t port = 50000;
  Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAddress);
  ApplicationContainer hubApp = packetSinkHelper.Install (csmaNodes.Get(nCsma-1));
  hubApp.Start (Seconds (1.0));
  hubApp.Stop (Seconds (10.0));

  // Creating OnOff apps to send tcp packets to hub, on on each spoke node.
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer spokeApps;

  for (uint32_t i = 0; i < star.SpokeCount (); ++i){
    // Skipping installation of apps on node 0 and node 4.
    if (!(i == 0 || i == 4)){
      AddressValue remoteAddress (InetSocketAddress (csmaInterfaces.GetAddress(nCsma-1), port));
      onOffHelper.SetAttribute ("Remote", remoteAddress);
      spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
    }
  }
  spokeApps.Start (Seconds (1.0));
  spokeApps.Stop (Seconds (10.0));

  // Enabling global routing.
  NS_LOG_INFO ("13. Enabling static global routing");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Running program.
  NS_LOG_INFO ("14. Running Simulation");
  Simulator::Run ();
  Simulator::Destroy ();

  // Ok ! we are done now.
  NS_LOG_INFO ("Done !!!");

  return 0;
}
