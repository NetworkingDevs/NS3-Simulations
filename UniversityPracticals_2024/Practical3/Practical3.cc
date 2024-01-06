/**
 * -> This is practical-3. Student will be given a group task.
 * -> The lab faculty will assign the topology to each group.
 * -> Each group will have at most 2 students.
 * -> Each group have to write a ".cc" file for simulation of equivalent topology (given by lab faculty) in NS3.
 * -> Then student have to take any two nodes from the topology and configure them as server and client machines.
 * -> Then write a code for communication between them.
 * -> Student can use "UDPEchoClientApplication" and "UDPEchoServerApplication" for communication.
 * -> Observe output in command line using "LogComponents" of "UDPEchoClientApplication" and "UDPEchoServerApplication"!
 * -> Nicely drafted code and understanding of code will leads to full marks.
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

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
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces12.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
