#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
 
using namespace ns3;
 
static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd
                         << std::endl;
}
 
int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);
 
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
 
    // step-1 = creating group of nodes....
    NodeContainer allNodes,nodes02,nodes12,nodes23,nodes34,nodes35;
allNodes.Create(6);
 
nodes02.Add(allNodes.Get(0));
nodes02.Add(allNodes.Get(2));
nodes12.Add(allNodes.Get(1));
nodes12.Add(allNodes.Get(2));
nodes23.Add(allNodes.Get(2));
nodes23.Add(allNodes.Get(3));
nodes34.Add(allNodes.Get(3));
nodes34.Add(allNodes.Get(4));
nodes35.Add(allNodes.Get(3));
nodes35.Add(allNodes.Get(5));
 
// step-2 = create link
PointToPointHelper p2pp2pLink;
p2pp2pLink.SetDeviceAttribute("DataRate",StringValue("105Mbps"));
p2pp2pLink.SetChannelAttribute("Delay",StringValue("2ms"));
 
 // step-3 = creating devices
 NetDeviceContainer devices02, devices12, devices23, devices34, devices35;
 
 
devices02 = p2pp2pLink.Install(nodes02);
devices12 = p2pp2pLink.Install(nodes12);
devices23 = p2pp2pLink.Install(nodes23);
devices34 = p2pp2pLink.Install(nodes34);
devices35 = p2pp2pLink.Install(nodes35);
// step-4 = Install ip stack
InternetStackHelper stack;
stack.Install(allNodes);
 
// step-5 = Assignment of IP Address
Ipv4AddressHelper address;
 
 
address.SetBase("54.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);
 
 
address.SetBase("55.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
 
 
address.SetBase("56.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces23 = address.Assign(devices23);
 
 
address.SetBase("57.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces34 = address.Assign(devices34);
 
 
address.SetBase("58.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces35 = address.Assign(devices35);
 
 
// step-6 = server configuration
UdpEchoServerHelper echoServer(54);
 
ApplicationContainer serverApps = echoServer.Install(allNodes.Get(4));
serverApps.Start(Seconds(1.0));
serverApps.Stop(Seconds(10.0));

    // TCP...
    // Server on node 5
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces35.GetAddress(1), 8080));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), 8080));
    ApplicationContainer sinkApps = packetSinkHelper.Install(allNodes.Get(5));
    sinkApps.Start(Seconds(1.0));
    sinkApps.Stop(Seconds(10.0));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(allNodes.Get(5), TcpSocketFactory::GetTypeId());

    Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate("1Mbps"));
    allNodes.Get(1)->AddApplication(app);
    app->SetStartTime(Seconds(2.));
    app->SetStopTime(Seconds(10.));
    
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("henil_ns3Wiki.cwnd");
     ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",MakeBoundCallback(&CwndChange, stream));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpCubic"));
 
// step-7 = client configuration
UdpEchoClientHelper echoClient(interfaces34.GetAddress(1),54);
echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
echoClient.SetAttribute("Interval", TimeValue(Seconds(0.000016))); // 0.00015 = 50% 0.000016 = 100%
echoClient.SetAttribute("PacketSize", UintegerValue(1024));
 
ApplicationContainer clientApps = echoClient.Install(allNodes.Get(0));
//clientApps.Start(Seconds(2.0));
//clientApps.Stop(Seconds(10.0));
 
Ipv4GlobalRoutingHelper::PopulateRoutingTables();
AnimationInterface anim("animationCustom.xml");
 
p2pp2pLink.EnablePcapAll("code");
 
 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
