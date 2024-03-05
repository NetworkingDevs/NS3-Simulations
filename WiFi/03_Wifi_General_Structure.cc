// comparing with second.cc
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h" // new include
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h" // new import
#include "ns3/yans-wifi-helper.h" // new import
#include "ns3/wifi-standards.h"

// my includes...
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("General_Wifi_Structure");

int
main(int argc, char* argv[])
{
    
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    NodeContainer allNodes,wifiStaNodes, wifiApNodes, wifiStaNodes0, wifiApNodes0;
    allNodes.Create(7);
    wifiStaNodes.Add(allNodes.Get(0));
    wifiStaNodes.Add(allNodes.Get(1));
    
    wifiApNodes.Add(allNodes.Get(2));
    
    wifiStaNodes0.Add(allNodes.Get(3));
    wifiStaNodes0.Add(allNodes.Get(4));
    
    wifiApNodes0.Add(allNodes.Get(5));
    
    NodeContainer allAp_Sta, allAp_Ap;
    allAp_Sta.Add(allNodes.Get(2));
    allAp_Sta.Add(allNodes.Get(5));
    
    allAp_Ap.Add(allNodes.Get(6));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default();
    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;       
    WifiHelper wifi;
    
    Ssid test = Ssid("ns-3-ssid");
    Ssid henu = Ssid("henil");
    Ssid master = Ssid("master");
    
    WifiMacHelper mac;
        
    phy.SetChannel(channel.Create());        
    wifi.SetStandard(ns3::WIFI_STANDARD_80211n);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    
    mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(test));
    NetDeviceContainer staDev = wifi.Install(phy,mac,wifiStaNodes);
    
    mac.SetType("ns3::ApWifiMac","Ssid",SsidValue(test));
    NetDeviceContainer apDev = wifi.Install(phy,mac,wifiApNodes);
    
    phy.SetChannel(channel1.Create());
        
    mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(henu));
    NetDeviceContainer staDev0 = wifi.Install(phy,mac,wifiStaNodes0);
    
    mac.SetType("ns3::ApWifiMac","Ssid",SsidValue(henu));
    NetDeviceContainer apDev0 = wifi.Install(phy,mac,wifiApNodes0);
    
    phy.SetChannel(channel2.Create());
    
    mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(master));
    NetDeviceContainer staDev_aps = wifi.Install(phy,mac,allAp_Sta);
    
    mac.SetType("ns3::ApWifiMac","Ssid",SsidValue(master));
    NetDeviceContainer apDev_aps = wifi.Install(phy,mac,allAp_Ap);
    
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(allNodes);

    InternetStackHelper stack;
    stack.Install(allNodes);

    Ipv4AddressHelper address;
    address.SetBase("55.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfacesSta = address.Assign(staDev);
    Ipv4InterfaceContainer interfacesAp = address.Assign(apDev);
    
    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfacesSta0 = address.Assign(staDev0);
    Ipv4InterfaceContainer interfacesAp0 = address.Assign(apDev0);
    
    address.SetBase("56.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces_aps = address.Assign(staDev_aps);
    Ipv4InterfaceContainer interfacesAps = address.Assign(apDev_aps);

    NodeContainer serverNodes;
    serverNodes.Add(allNodes.Get(0));
    serverNodes.Add(allNodes.Get(3));    
    
    UdpEchoServerHelper echoServer(90);

    ApplicationContainer serverApps = echoServer.Install(serverNodes);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient0(interfacesSta0.GetAddress(0), 90);
    echoClient0.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient0.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient0.SetAttribute("PacketSize", UintegerValue(1024));
    
    // std::cout << "Address : " << interfacesSta0.GetAddress(0);
    UdpEchoClientHelper echoClient3(interfacesSta.GetAddress(0), 90);
    echoClient3.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient3.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient3.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient0.Install(allNodes.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    ApplicationContainer clientApps3 = echoClient3.Install(allNodes.Get(4));
    clientApps3.Start(Seconds(3.0));
    clientApps3.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("HenilWiFiGeneral.xml");
    Simulator::Stop(Seconds(10.0));

    // channel.EnablePcapAll("wifi",true,true);
    phy.EnablePcap("wifi_general", staDev);
    phy.EnablePcap("wifi_general", apDev);
    
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
