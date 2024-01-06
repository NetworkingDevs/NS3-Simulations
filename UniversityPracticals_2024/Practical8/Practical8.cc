/**
 * -> This is practical-8. Student will be given a group task or individual work!
 * -> Topology is same as Practical-3.
 * -> Each group will have at most 2 students. (If group task)
 * -> Each group have to write a ".cc" file for simulation of equivalent topology (given by lab faculty) in NS3.
 * -> Then student have to take any two nodes from the topology and configure them as server and client machines.
 * -> Then write a code for communication between them.
 * -> Student should use TCP coomunication!
 * -> Find out what are the classes that are required and use those to simulate the same.
 * -> Observe output in command line using "LogComponents" 
 * -> Nicely drafted code and understanding of code will leads to full marks.
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//      54.0.0.0/8       55.0.0.0/8
// n0 -------------- n1-------------- n2
//    point-to-point    point-to-point
//

using namespace ns3;

int
main(int argc, char* argv[])
{
    // enabling log components for "UdpEchoClientApplication" and "UdpEchoServerApplication"...
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // STEP - 1 : Creating nodes ===========================================================
    NodeContainer allNodes, nodes01, nodes12;
    allNodes.Create(3);
    
    nodes01.Add(allNodes.Get(0));
    nodes01.Add(allNodes.Get(1));
    
    nodes12.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(2));
    
    // STEP - 2 : Making a point to point link helper ======================================
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // STEP - 3 : Making of devices ========================================================
    NetDeviceContainer devices01, devices12;
    devices01 = pointToPoint.Install(nodes01);
    devices12 = pointToPoint.Install(nodes12);
    
    // STEP - 4 : IP Assignment and stack installation on nodes ===========================
    InternetStackHelper stack;
    stack.Install(allNodes);

    Ipv4AddressHelper address;
    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    
    address.SetBase("55.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    
    // STEP - 5 : Client an Server Configuration ==========================================
    // Create a TCP sink application on node 2
  Address sinkAddress(InetSocketAddress(interfaces12.GetAddress(1), 8080));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install(allNodes.Get(2));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  // Create a TCP client application on node 0
  OnOffHelper onoff("ns3::TcpSocketFactory", sinkAddress);
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute("PacketSize", UintegerValue(1500));
  onoff.SetAttribute("DataRate", StringValue("5Mbps"));
  
  ApplicationContainer clientApps = onoff.Install(allNodes.Get(0));
  clientApps.Start(Seconds(1.0));
  clientApps.Stop(Seconds(10.0));
    
     // Enable PCAP tracing
  pointToPoint.EnablePcapAll("tcp-example");
  
  AnimationInterface anim("Tcp.xml");
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
