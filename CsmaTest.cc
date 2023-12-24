#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// Testing : CSMA(10 Nodes) <-> P2P Node <-> CSMA(10 Nodes)

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;
    
    NodeContainer allNodes;
    allNodes.Create(21);
    
    NodeContainer csmaNodes;
    for(int i=0; i<10; i++) { // 0-9 Nodes from allNodes
    	// we add to csmaNodes
    	csmaNodes.Add(allNodes.Get(i));
    }

    NodeContainer p2pNodes,p2pNodes2;
    p2pNodes.Add(csmaNodes.Get(9));
    p2pNodes.Add(allNodes.Get(10)); // 10th Node from allNodes...

    NodeContainer csmaNodes2;
    for(int i=11; i<21; i++) { // 11-19 Nodes from allNodes
    	// we add to csmaNodes2
    	csmaNodes2.Add(allNodes.Get(i));
    }
    
    p2pNodes2.Add(p2pNodes.Get(1));
    p2pNodes2.Add(csmaNodes2.Get(9));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);
    
    NetDeviceContainer p2pDevices2;
    p2pDevices2 = pointToPoint.Install(p2pNodes2);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);
    
    NetDeviceContainer csmaDevices2;
    csmaDevices2 = csma.Install(csmaNodes2);

    InternetStackHelper stack;
    /*stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);
    stack.Install(p2pNodes2.Get(1));*/
    stack.Install(allNodes); // just write this one line...

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);
    
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces2;
    p2pInterfaces2 = address.Assign(p2pDevices2);
    
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces2;
    csmaInterfaces2 = address.Assign(csmaDevices2);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(csmaNodes2.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    pointToPoint.EnablePcapAll("test");
    csma.EnablePcap("test", csmaDevices.Get(1), true);
    
    AnimationInterface anim("HenilTest.xml");
    /*anim.SetConstantPosition(p2pNodes.Get(0), 10.0, 10.0, 0);
    anim.SetConstantPosition(p2pNodes.Get(1), 20.0, 10.0, 0);
    
    anim.SetConstantPosition(csmaNodes.Get(1), 20.0, 20.0, 0);
    anim.SetConstantPosition(csmaNodes.Get(2), 30.0, 20.0, 0);
    anim.SetConstantPosition(csmaNodes.Get(3), 40.0, 20.0, 0);
    
    anim.SetConstantPosition(p2pNodes2.Get(1), 40.0, 10.0, 0);*/
    for(int i=0; i<10; i++) {
    	anim.SetConstantPosition(allNodes.Get(i), i*20, 10, 0);
    }
    
    anim.SetConstantPosition(allNodes.Get(10),200,20,0);
    
    for(int i=0; i<10; i++) {
    	anim.SetConstantPosition(allNodes.Get(i+11), i*20, 30, 0);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
