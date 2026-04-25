/**
 * This file is simple simulation of DHCP.
 * 
 * Reference: /src/internet-apps/examples/dhcp-example.cc
 * 
 * Author: Henil Mistry <henilmistry74496@gmail.com>
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

void printIpAddress(NodeContainer nodes) {
    for (uint32_t i= 0; i<nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();

        std::cout << "Node " << i << " IP Configuration: \n";

        for (uint32_t j=0; j<ipv4->GetNInterfaces(); ++j) {
            for (uint32_t k=0; k<ipv4->GetNAddresses(j); ++k) {
                Ipv4InterfaceAddress iaddr = ipv4->GetAddress(j, k);
                std::cout << " Interface " << j
                          << " IP: " << iaddr.GetLocal()
                          << " Mask: " << iaddr.GetMask()
                          << std::endl;
            }
        }

        std::cout << "----------------------------------------------\n";
    }
}

int
main(int argc, char* argv[]) {

    /**
     * Step-0: Command Line Argument(s) configuration and
     *         Log / Verbose configuration.
     */
    bool verbose = false;
    bool tracing = true;
    uint32_t nNodes = 2;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of stationary devices (Default: 2)", nNodes);
    cmd.AddValue("verbose", "Tell applications and other modules to log if true (Default: false)", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing for all node if true (Default: true)", tracing);

    cmd.Parse(argc, argv);

    if ( verbose ) {
        LogComponentEnable("DhcpServer", LOG_LEVEL_ALL);
        LogComponentEnable("DhcpClient", LOG_LEVEL_ALL);
    }

    /**
     * Step-1: Creating Node(s) and NodeContainer(s)
     */
    NodeContainer apNode, staNodes, server;
    // first node: WiFi Access Point Node
    apNode.Create(1);
    // other nodes: WiFi Stationary Nodes
    staNodes.Create(nNodes);
    // server node: in remote network
    server.Create(1);

    // grouping all nodes together
    NodeContainer allNodes(apNode, staNodes, server);
    NodeContainer localNodes(apNode, staNodes);
    NodeContainer remoteNodes(apNode, server);


    /**
     * Step-2: Creating Channel(s)
     */
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    csma.SetDeviceAttribute("Mtu", UintegerValue(1500));

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));


    /**
     * Step-3 Creating Device(s)
     */
    NetDeviceContainer localDevices = csma.Install(localNodes);
    NetDeviceContainer remoteDevices = p2p.Install(remoteNodes);

    MobilityHelper m;
    m.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    m.Install(allNodes);

    Ptr<ConstantPositionMobilityModel> mobApNode = apNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
    mobApNode->SetPosition(Vector(0, 5, 0));

    Ptr<ConstantPositionMobilityModel> mobServer = server.Get(0)->GetObject<ConstantPositionMobilityModel>();
    mobServer->SetPosition(Vector(10, 5, 0));

    for (uint32_t i=0; i<nNodes; i++) {
        Ptr<ConstantPositionMobilityModel> mob = staNodes.Get(i)->GetObject<ConstantPositionMobilityModel>();
        mob->SetPosition(Vector(5*i, 20+i, 0));
    }


    /**
     * Step-4: Install Network Stack and Essential IP
     *          Assignment (Mostly to AP Only)
     */
    InternetStackHelper stack;
    stack.Install(allNodes);

    // Ipv4AddressHelper address;
    // address.SetBase("192.168.20.0", "255.255.255.0");
    // Ipv4InterfaceContainer remoteInterfaces = address.Assign(remoteDevices);

    // Ipv4StaticRoutingHelper routingHelper;
    // Ptr<Ipv4> ipv4Ptr = remoteNodes.Get(1)->GetObject<Ipv4>();
    // Ptr<Ipv4StaticRouting> staticRoutingA = routingHelper.GetStaticRouting(ipv4Ptr);
    // staticRoutingA->AddNetworkRouteTo(
    //     Ipv4Address("192.168.10.0"), Ipv4Mask("/24"),
    //     Ipv4Address("192.168.20.1"), 1
    // );

    DhcpHelper dhcpHelper;

    // DHCP Server
    /**
     * Node Pointer, DHCP Server's Address,
     * Pool's Network ID, Pool's Subnet Mask,
     * Pool's Start IP (Min.IP), Pool's End IP (Max.IP), // scope of DHCP Server
     * Pool's G/W Address
     */
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(
        localDevices.Get(0), Ipv4Address("192.168.10.1"),
        Ipv4Address("192.168.10.0"), Ipv4Mask("/24"),
        Ipv4Address("192.168.10.2"), Ipv4Address("192.168.10.3"),
        Ipv4Address("192.168.10.1")
    );
    ApplicationContainer dhcpServerAppRemote = dhcpHelper.InstallDhcpServer(
        remoteDevices.Get(0), Ipv4Address("192.168.20.1"),
        Ipv4Address("192.168.20.0"), Ipv4Mask("/24"),
        Ipv4Address("192.168.20.2"), Ipv4Address("192.168.20.100"),
        Ipv4Address("192.168.20.1")
    );
    dhcpServerApp.Start(Seconds(0));
    dhcpServerApp.Stop(Seconds(30));
    dhcpServerAppRemote.Start(Seconds(0));
    dhcpServerAppRemote.Stop(Seconds(30));

    // DHCP Client
    NetDeviceContainer staDevices;
    for (uint32_t i=0; i<nNodes; i++) {
        staDevices.Add(localDevices.Get(1+i));
    }
    staDevices.Add(remoteDevices.Get(1));

    ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient(staDevices);
    dhcpClients.Start(Seconds(1));
    dhcpClients.Stop(Seconds(30.0));


    /**
     * Step-5: Add manual IP route to All intermediate routers
     *         because Dynamic Routing for DHCP simulation is
     *         an overkill.
     */
    // Skipping this part since this simulation has only one subnet.


    /**
     * Step-6: Use Pinghelper to determine connectivity
     */
    PingHelper pingHelper(Ipv4Address("192.168.20.2"));
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1)));
    pingHelper.SetAttribute("Size", UintegerValue(1024));
    pingHelper.SetAttribute("Count", UintegerValue(4));

    ApplicationContainer app = pingHelper.Install(staNodes.Get(0));
    app.Start(Seconds(15.0));
    app.Stop(Seconds(30.0));


    /**
     * Step-7: Simulation Configuration(s)
     */
    Simulator::Stop(Seconds(60));
    Simulator::Schedule(Seconds(30), &printIpAddress, allNodes);

    if (tracing) {
        csma.EnablePcapAll("dhcp-simulation-node");
    }

    AnimationInterface anim("dhcp-simulation-basic.xml");
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}