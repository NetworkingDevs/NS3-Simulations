/**
 * -> This is practical-7. Student will be given a group task or may be individual work!
 * -> Each group will have at most 2 students. (If it is group task)
 * -> The main task of Practical-7 is to observe packet drop in Wireless network.
 * -> Nicely drafted code and understanding of code will leads to full marks.
 **/

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
//         
//  *          *       
//  |          |       
//(n1)    (AP)(n0) 
//
// AP = Access Point
using namespace ns3;


int
main(int argc, char* argv[])
{
    
    // default variables...
    int nWifiSta = 2;
    int nWifiAp = 1; // do not changes this...
    std::string fileName = "Practical7_wifi.xml";
    int mDist = 40;
    
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // STEP - 0 : Parsing the command line arguments for practical - 5 =====================
    CommandLine cmd(__FILE__);
    cmd.AddValue("mDist", "Distance of client node on x axis!", mDist);
    cmd.Parse(argc, argv);
    
    // STEP - 1 ====================================================
    // creating nodes...
    NodeContainer allNodes,stationaryNodes,wifiAPNode;
    allNodes.Create(nWifiAp + nWifiSta);
    for(int i=1; i<nWifiAp + nWifiSta; i++) {
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
    
    // setting a constant position for node 0,3,4
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(allNodes);
    
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

    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 54);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    phy.EnablePcapAll("Pratical7");

    AnimationInterface anim(fileName);
    anim.SetConstantPosition(allNodes.Get(0), 20, 20);
    anim.SetConstantPosition(allNodes.Get(1), 30, 20);
    anim.SetConstantPosition(allNodes.Get(2), mDist, 20); // if x(mDist) > 71 ... packet will drop...
    Simulator::Stop(Seconds(10.0));
    
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
