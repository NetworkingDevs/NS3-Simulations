/**
 * -> Simple Client-Server communication (NS-3 Warmup)
 * -> Ref. link : https://www.nsnam.org/wiki/Lab_Assignments_using_ns-3
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

// Default Network Topology
//
//       54.0.0.0/8
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

int
main(int argc, char* argv[])
{
    std::string vDataRate = "500Mbps";
    std::string vDelay = "2ms";
    
    CommandLine cmd(__FILE__);
    cmd.AddValue("vDataRate", "DataRate of p2p link!", vDataRate);
    cmd.AddValue("vDelay", "Delay or Latency of p2p link!", vDelay);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(vDataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(vDelay));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("54.0.0.0", "255.0.0.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9), anotherServer(54);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Add(anotherServer.Install(nodes.Get(1)));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9), anotherClient(interfaces.GetAddress(1), 54);
    echoClient.SetAttribute("MaxPackets", UintegerValue(20));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    
    anotherClient.SetAttribute("MaxPackets", UintegerValue(20));
    anotherClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
    anotherClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Add(anotherClient.Install(nodes.Get(0)));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    pointToPoint.EnablePcapAll("simple_client_server");
 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
