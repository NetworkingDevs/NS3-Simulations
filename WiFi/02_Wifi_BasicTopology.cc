// The purpose of this file :
// This is base file for learning WiFi nodes configuration in NS3
// I have just used WiFi Nodes here...

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h" 
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h" 
#include "ns3/yans-wifi-helper.h" 
#include "ns3/netanim-module.h"

// Default Network Topology
//         * 
//         |
//        (n1)
//  *          *       *
//  |          |       |
//(n2)    (AP)(n0)    (n4)
//
//           *        
//           |
//          (n5)
// AP = Access Point
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // STEP - 1 ====================================================
    // creating nodes...
    NodeContainer allNodes,stationaryNodes,wifiAPNode;
    allNodes.Create(5);
    for(int i=1; i<5; i++) {
    	stationaryNodes.Add(allNodes.Get(i));
    }
    wifiAPNode.Add(allNodes.Get(0));
    
    // STEP - 2 ====================================================
    // creating a channel for WiFi Communication....
    // WiFi Communication uses one channel for multiple devices...
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy; // created phy layer for WiFi...
    phy.SetChannel(channel.Create());
    
    WifiMacHelper mac; // created mac layer helper bcoxz AP and Stationary nodes have diff. imple.
    Ssid ssid = Ssid("henil-ssid");
    
    // STEP - 3 ====================================================
    // Now, creating the devices....
    WifiHelper wifi; // helper in device creation...
    NetDeviceContainer stationaryDevices,wifiAPDevice;
    
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    stationaryDevices = wifi.Install(phy, mac, stationaryNodes);
    
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    wifiAPDevice = wifi.Install(phy, mac, wifiAPNode);
    
    // STEP - 4 ====================================================
    // Now, assigning a mobility...
    // This is very important for signals propagation and such readings...
    MobilityHelper mobility;
    
    NodeContainer node12, node034;
    node12.Add(allNodes.Get(1));
    node12.Add(allNodes.Get(2));
    
    node034.Add(allNodes.Get(0));
    node034.Add(allNodes.Get(3));
    node034.Add(allNodes.Get(4));
    // setting random movement mobility for node 1,2
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(node12);
    // setting a constant position for node 0,3,4
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(node034);
    
    // STEP - 5 ====================================================
    // Assignment of IP....
    InternetStackHelper stack;
    stack.Install(allNodes);
    
    Ipv4AddressHelper address;

    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces = address.Assign(stationaryDevices);
    Ipv4InterfaceContainer APinterface = address.Assign(wifiAPDevice);
    
    // STEP - 6s ====================================================
    // Server and Client Configuration....
    UdpEchoServerHelper echoServer(54);

    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 54);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(3));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("Henil_OnlyWiFi.xml");
    Simulator::Stop(Seconds(10.0));
    
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
