/**
 * -> This is practical-5. Student will be given a group task or may be individual work!
 * -> Topology will be same, given in Practical-3.
 * -> Each group will have at most 2 students. (If it is group task)
 * -> In addition to task of previous practical-4.
 * -> The main task of Practical-5 is to use command line arguments for various parameters. (i.e., number of nodes, server node, client node, net ID, etc.)
 * -> Basically students have to parse command line arguments passed from the terminal in ".cc" file! 
 * -> Student can use class "CommandLine" to achieve above task.
 * -> Student should also do error handling rather than just throwing errors directly to command prompt!
 *     i.e. 1. What if the server and client nodes are same?
 *          2. What if the server and client nodes index are negative?
 *          3. What if no. of nodes are negative or too high.
 *          4. What if the net ID is not properly entered?
 * -> Nicely drafted code and understanding of code will leads to full marks.
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
// Added this module to generate ".xml" file for observation in NetAnim
#include "ns3/netanim-module.h"
// Added this modules to match regex while error handling
#include <bits/stdc++.h>
#include <regex>

// Default Network Topology
//
//      54.0.0.0/8       55.0.0.0/8
// n0 -------------- n1-------------- n2
//    point-to-point    point-to-point
//

using namespace ns3;

// you can make a custom regex pattern according to your need...
std::regex pattern_DataRate("\\b\\d+([KMG]bps)\\b"); // regex pattern for data rate...
std::regex pattern_Delay("\\b\\d+ms\\b"); // regex pattern for delay...

// This is base function for matching any string to any pattern...
// Do not alter this, instead use this function on top of all your functions...
bool validate(std::string str, std::regex pattern) {
    return std::regex_match(str,pattern);
}

// Function that validates DataRate...
// i.e. Create this kind of function
bool validateDateRate(std::string str) {
    return validate(str, pattern_DataRate);
}

bool validateDelay(std::string str) {
    return validate(str, pattern_Delay);
}

int
main(int argc, char* argv[])
{
    // enabling log components for "UdpEchoClientApplication" and "UdpEchoServerApplication"...
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // default variables declared here...
    // which can be taken from command lines...
    std::string vDataRate = "100Mbps";
    std::string vDelay = "2ms";
    int vServerIndex = 2;
    int vClientIndex = 0;
    
    // STEP - 0 : Parsing the command line arguments for practical - 5 =====================
    CommandLine cmd(__FILE__);
    cmd.AddValue("vDataRate", "DataRate attr. of point-to-point link!", vDataRate);
    cmd.AddValue("vDelay", "Delay attr. of point-to-point link!", vDelay);
    cmd.AddValue("vServerIndex", "Index of server node!", vServerIndex);
    cmd.AddValue("vClientIndex", "Index of client node!", vClientIndex);
    cmd.Parse(argc, argv);
    
    // student should do this kind of error handling...
    if (!validateDateRate(vDataRate)) {
        std::cout << "ERROR : Valid data rate expected instead of passed value : " << vDataRate << std::endl;
        return 0;
    }
    
    if (!validateDelay(vDelay)) {
        std::cout << "ERROR : Valid delay expected instead of passed value : " << vDelay << std::endl;
        return 0;
    }
    
    if (vServerIndex < 0 || vServerIndex > 2) {
        std::cout << "ERROR : Valid server node expected (between 0-2) instead of passed value : " << vServerIndex << std::endl;
        return 0;
    }
    
    if (vServerIndex == vClientIndex) {
        std::cout << "WARNING : Server and Client Indices are same! Please consider passing different index for both!" << std::endl;
        return 0;
    }
    
    if (vClientIndex < 0 || vClientIndex > 2) {
        std::cout << "ERROR : Valid client node expected (between 0-2) instead of passed value : " << vClientIndex << std::endl;
        return 0;
    }
    
    // STEP - 1 : Creating nodes ===========================================================
    NodeContainer allNodes, nodes01, nodes12;
    allNodes.Create(3);
    
    nodes01.Add(allNodes.Get(0));
    nodes01.Add(allNodes.Get(1));
    
    nodes12.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(2));
    
    // STEP - 2 : Making a point to point link helper ======================================
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(vDataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(vDelay));

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

    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(vServerIndex));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient(interfaces01.GetAddress(0));
    if (vServerIndex < 2) {
        echoClient.SetAttribute("RemoteAddress", AddressValue(interfaces01.GetAddress(vServerIndex)));
        echoClient.SetAttribute("RemotePort", UintegerValue(9));
    } else {
        echoClient.SetAttribute("RemoteAddress", AddressValue(interfaces12.GetAddress(1)));
        echoClient.SetAttribute("RemotePort", UintegerValue(9));
    }
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(vClientIndex));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    // STEP - 6 : Changes in code for Practical - 4....
    // To generate ".xml" file
    AnimationInterface anim("Practical-5.xml");
    // below three lines will set constant position of nodes in Animation...
    anim.SetConstantPosition(allNodes.Get(0), 20, 20);
    anim.SetConstantPosition(allNodes.Get(1), 30, 20);
    anim.SetConstantPosition(allNodes.Get(2), 40, 20);
    // below line will generate ".pcap" files for each NIC configured with this point to point helper...
    pointToPoint.EnablePcapAll("Pratical5");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
