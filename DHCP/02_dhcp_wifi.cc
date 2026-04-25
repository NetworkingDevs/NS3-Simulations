/**
 * This file is simple simulation of DHCP. Where we'll be
 * simulating a purely wireless network.
 * 
 * Reference: /src/internet-apps/examples/dhcp-example.cc
 * 
 * Author: Henil Mistry <henilmistry74496@gmail.com>
 */

 /**
  * Network Layout:
  * 
  * (192.168.10.1)                                      
  *  DHCP Server                                  DHCP Clients
  * (Home Router)                           (Your Mobile Devices)
  * n0<---------------  Wireless Channel  --------------->n1, n2
  * 
  * Things to note:
  * 1) We are using Wireless Channel For Simulation.
  * 2) We will use `PingHelper` b/w n2 and n1. After DHCP Process is
  *     completed. This will verify if IP Assignment is successful.
  * 3) You can add multiple nodes using command line arguments.
  * 4) Default G/W is Home router only which is also a DHCP Server.
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
    NodeContainer apNode, staNodes;
    // first node: WiFi Access Point Node
    apNode.Create(1);
    // other nodes: WiFi Stationary Nodes
    staNodes.Create(nNodes);

    // grouping all nodes together
    NodeContainer allNodes(apNode, staNodes);


    /**
     * Step-2: Creating Channel(s)
     */
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(wifiChannel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("henils-home");

    WifiHelper wifi;
    // wifi.SetStandard(WIFI_STANDARD_80211ax);


    /**
     * Step-3 Creating Device(s)
     */
    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, staNodes);

    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, apNode);

    MobilityHelper mobility;
    // You can change the mobility from here
    // mobility.SetMobilityModel(
    //     "ns3::ConstantAccelerationMobilityModel");
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(staNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNode);


    /**
     * Step-4: Install Network Stack and Essential IP
     *          Assignment (Mostly to AP Only)
     */
    InternetStackHelper stack;
    stack.Install(allNodes);

    DhcpHelper dhcpHelper;

    // DHCP Server
    /**
     * Node Pointer, DHCP Server's Address,
     * Pool's Network ID, Pool's Subnet Mask,
     * Pool's Start IP (Min.IP), Pool's End IP (Max.IP), // scope of DHCP Server
     * Pool's G/W Address
     */
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(
        apDevices.Get(0), Ipv4Address("192.168.10.1"),
        Ipv4Address("192.168.10.0"), Ipv4Mask("/24"),
        Ipv4Address("192.168.10.2"), Ipv4Address("192.168.10.3"),
        Ipv4Address("192.168.10.1")
    );
    dhcpServerApp.Start(Seconds(0));
    dhcpServerApp.Stop(Seconds(30));

    // DHCP Client
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
    PingHelper pingHelper(Ipv4Address("192.168.10.3"));
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

    Ptr<ConstantPositionMobilityModel> apPosition = apNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
    apPosition->SetPosition(Vector(10, 10, 0));

    // Change the modibility from here for Node 1

    // Ptr<ConstantAccelerationMobilityModel> staNode0 = staNodes.Get(0)->GetObject<ConstantAccelerationMobilityModel>();
    // staNode0->SetPosition(Vector(0, 5, 0));
    // staNode0->SetVelocityAndAcceleration(
    //     Vector(0, 0, 0),
    //     Vector(0, 0, 0)
    // );

    // Change the mobility from here for all the Nodes in staNodes 

    // for (uint32_t i=0; i<nNodes-1; i++) {
    //     double value = 2*(i+1), xVal, yVal;
    //     if (i%2==0) {
    //         xVal = value;
    //         yVal = 0;
    //     } else {
    //         xVal = 0;
    //         yVal = value;
    //     }
    //     Ptr<ConstantAccelerationMobilityModel> staNode = staNodes.Get(i+1)->GetObject<ConstantAccelerationMobilityModel>();
    //     staNode->SetPosition(Vector(xVal, yVal, 0));
    //     staNode->SetVelocityAndAcceleration(
    //         Vector(0, 0, 0),
    //         Vector(0, 0, 0)
    //     );
    // }

    if (tracing) {
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.EnablePcapAll("dhcp-wifi");
    }

    AnimationInterface anim("dhcp-simulation-wifi.xml");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}