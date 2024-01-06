/**
 * -> This is practical-4. Student will be given a group task.
 * -> Topology will be same, given in Practical-3.
 * -> Each group will have at most 2 students.
 * -> In addition to task of previous practical.
 * -> The main task of Practical-4 is to create and observe ".pcap" file and ".xml" files.
 * -> Basically students have to generate Packet capture files to observe it in "WireShark" and ".xml" files to observe it in "NetAnim".
 * -> Students should be able to derive helpful outcomes from packet capture files and observation of Animation in NetAnim.
 * -> Nicely drafted code and understanding of code will leads to full marks.
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
// Added this module to generate ".xml" file for observation in NetAnim
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
    
    // STEP - 6 : Changes in code for Practical - 4....
    // To generate ".xml" file
    AnimationInterface anim("Practical-4.xml");
    // below three lines will set constant position of nodes in Animation...
    anim.SetConstantPosition(allNodes.Get(0), 20, 20);
    anim.SetConstantPosition(allNodes.Get(1), 30, 20);
    anim.SetConstantPosition(allNodes.Get(2), 40, 20);
    // below line will generate ".pcap" files for each NIC configured with this point to point helper...
    pointToPoint.EnablePcapAll("Pratical4");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
