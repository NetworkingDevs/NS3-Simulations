/**
 * This file is simple simulation of DHCP. Where you have only
 * one DHCP Server and only one DHCP Client in same local net-
 * work.
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
  * n0<---------------  CSMA Channel  ------------------->n1, n2
  * 
  * Things to note:
  * 1) We are not using Wireless Channel to simplyfy things.
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

int
main(int argc, char* argv[]) {

    /**
     * Step-0: Command Line Argument(s) configuration and
     *         Log / Verbose configuration.
     */
    bool verbose = false;
    bool tracing = true;
    bool simulation = false;
    uint32_t nNodes = 2;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of stationary devices (Default: 2)", nNodes);
    cmd.AddValue("verbose", "Tell applications and other modules to log if true (Default: false)", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing for all node if true (Default: true)", tracing);
    cmd.AddValue("simulation", "Record simulation for NetAnim if true (Default: false)", simulation);

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
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    csma.SetDeviceAttribute("Mtu", UintegerValue(1500));


    /**
     * Step-3 Creating Device(s)
     */
    NetDeviceContainer allDevices = csma.Install(allNodes);


    /**
     * Step-4: Install Network Stack and Essential IP
     *          Assignment (Mostly to AP Only)
     */
    InternetStackHelper stack;
    stack.Install(allNodes);

    DhcpHelper dhcpHelper;

    // DHCP Server
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(
        allDevices.Get(0), Ipv4Address("192.168.10.1"),
        Ipv4Address("192.168.10.0"), Ipv4Mask("/24"),
        Ipv4Address("192.168.10.2"), Ipv4Address("192.168.10.20"),
        Ipv4Address("192.168.10.1")
    );
    dhcpServerApp.Start(Seconds(0));
    dhcpServerApp.Stop(Seconds(30));

    // DHCP Client
    NetDeviceContainer staDevices;
    for (uint32_t i=0; i<nNodes; i++) {
        staDevices.Add(allDevices.Get(1+i));
    }

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

    if (tracing) {
        csma.EnablePcapAll("dhcp-simulation-node");
    }

    if (simulation) {
        AnimationInterface anim("dhcp-simulation-basic.xml");
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}