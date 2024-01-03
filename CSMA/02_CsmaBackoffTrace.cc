#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// Purpose of this file:
// This file is to demonstrate how to trace backoffs in LANs

// Default Network Topology
// 
// n0---             ---n6
//     |             |  
// n1--- (routers)   ---n7
//     |---n4---n5---|  
// n2---             ---n8
//     |             |
// n3---             ---n9
// ( LAN1 )          ( LAN2 )      
//
// LAN1 = 54.0.0.0/8
// LAN2 = 55.0.0.0/8
// LAN3 = 56.0.0.0/8
// Clients = n0,n3
// Server = n8
//    

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaBackoffExample");

void 
HenilBackoffTrace (std::string ctx, Ptr<const Packet> pkt)
{
	std::cout << ctx << std::endl;
	EthernetHeader ethHdr;
	if(pkt->PeekHeader(ethHdr)) 
	{
		std::cout << "\t"
		          << Now()
		          << " Packet from : "
		          << ethHdr.GetSource()
		          << " To : "
		          << ethHdr.GetDestination()
		          << " experiencing backoff"
		          << std::endl; 
	}
}

int
main(int argc, char* argv[])
{
    bool verbose = false;
    
    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    // creating node containers =================================================================
    NodeContainer allNodes,nodesLan1,nodesLan2,nodesRouters;
    allNodes.Create(10);
    for(int i=0; i<=4; i++) { // loop from 0 - to - 4
    	nodesLan1.Add(allNodes.Get(i)); // 0 - to - 4
    	nodesLan2.Add(allNodes.Get(i+5)); // 5 - to - 9
    }
    nodesRouters.Add(allNodes.Get(4));
    nodesRouters.Add(allNodes.Get(5));

    // creating links here ======================================================================
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // creating devices here ====================================================================
    NetDeviceContainer p2pDevices;
    p2pDevices = p2p.Install(nodesRouters);
    
    NetDeviceContainer devicesLan1,devicesLan2;
    devicesLan1 = csma.Install(nodesLan1);
    devicesLan2 = csma.Install(nodesLan2);

    InternetStackHelper stack;
    stack.Install(allNodes);
    
    // ip assignment here =======================================================================
    Ipv4AddressHelper address;
    
    address.SetBase("56.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer p2pInterface = address.Assign(p2pDevices);    

    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaceLan1 = address.Assign(devicesLan1);
    
    address.SetBase("55.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaceLan2 = address.Assign(devicesLan2);

    
    // UDP Echo server configuration here =======================================================
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodesLan2.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // UDP Echo Client configuration here =======================================================
    UdpEchoClientHelper echoClient(interfaceLan2.GetAddress(2), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    
    // making a group of client nodes....
    NodeContainer clientNodes;
    clientNodes.Add(nodesLan1.Get(0));
    clientNodes.Add(nodesLan1.Get(3));
    
    ApplicationContainer clientApps = echoClient.Install(clientNodes);
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    // Attaching Trace Source For Backoff Tarce =================================================
    Config::Connect("NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/MacTxBackoff",MakeCallback(&HenilBackoffTrace));
    
    // Other utilities configuration ============================================================
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    AnimationInterface anim("CSMA_Backoff.xml");
    // this is sample code for setting constant position...
    // anim.SetConstantPosition(csmaNodes.Get(1), 20.0, 20.0, 0);
    for(int i=0; i<4; i++) {
    	anim.SetConstantPosition(nodesLan1.Get(i), 10, i*10 + 10);
    	anim.SetConstantPosition(nodesLan2.Get(i+1), 40, i*10 + 10);
    }
    anim.SetConstantPosition(nodesRouters.Get(0), 20, 25);
    anim.SetConstantPosition(nodesRouters.Get(1), 30, 25);
    

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
