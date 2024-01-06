/**
 * -> This is practical-6. Student will be given a group task or may be individual work!
 * -> Topology will be same, given in Practical-3.
 * -> Each group will have at most 2 students. (If it is group task)
 * -> In addition to task of previous practical-5.
 * -> The main task of Practical-6 is to observe Queuing Delay!
 * -> Basically students have to make a graph or some sort of analysis kind of thing for actual measurement of Queuing Delay!
 * -> Student can use Hint given in the practicl definition or they can use any other online help (only from NS3's websites!)
 * -> Student should be able to explian the line of codes with it's significance.
 * -> Nicely drafted code and understanding of code will leads to full marks.
 * -> Helpful Link : https://www.nsnam.org/docs/tutorial/singlehtml/#available-queueing-models-in-ns3
 **/
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
// Added this module to generate ".xml" file for observation in NetAnim
#include "ns3/netanim-module.h"
// Added this for file handling
#include <iostream>
#include <fstream>

// Default Network Topology
//
//      54.0.0.0/8       55.0.0.0/8
// n0 -------------- n1-------------- n2
//    point-to-point    point-to-point
//
using namespace ns3;

int queueCounter = 0;
int packetCounterEnQ = 0;
int packetCounterDeQ = 0;

std::ofstream enq,deq,mainF;

void EnQ(std::string ctx, Ptr<const Packet> pkt)
{
	std::cout 
		<< 1
		<< " , "
		<< ++packetCounterEnQ
		<< " , "  
		<< Time(Simulator::Now()).GetNanoSeconds() 
		<< " , " 
		<< ++queueCounter 
		<< " , " 
		<< "enq" 
		<< std::endl;

	enq << packetCounterEnQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << std::endl;
	    
	
	mainF << packetCounterEnQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << ","
	    << "AenQ"
	    << std::endl;
}

void DeQ(std::string ctx, Ptr<const Packet> pkt)
{
	std::cout 
		<< 1
		<< " , "
		<< ++packetCounterDeQ
		<< " , "  
		<< Time(Simulator::Now()).GetNanoSeconds()
		<< " , " 
		<< --queueCounter 
		<< " , " 
		<< "deq" 
		<< std::endl;
	
	deq << packetCounterDeQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << std::endl;		    	
	
	mainF << packetCounterDeQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << ","
	    << "BdeQ"
	    << std::endl;
}

int
main(int argc, char* argv[])
{
    enq.open("Test_EnQ.txt");
    deq.open("Test_DeQ.txt");
    mainF.open("Test_Main.txt");
    
    Time::SetResolution(Time::NS);
    // enabling log components for "UdpEchoClientApplication" and "UdpEchoServerApplication"...
    // LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // ================ Do not change default declaration here ============================
    // default variables declared here...
    // which can be taken from command lines...
    std::string vDataRate = "100Mbps";
    std::string vDelay = "2ms";
    int vServerIndex = 2;
    int vClientIndex = 0;
    
    // Removed command line parsing...
    
    // STEP - 1 : Creating nodes ===========================================================
    NodeContainer allNodes, nodes01, nodes12;
    allNodes.Create(3);
    
    nodes01.Add(allNodes.Get(0));
    nodes01.Add(allNodes.Get(1));
    
    nodes12.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(2));
    
    // STEP - 2 : Making a point to point link helper ======================================
    PointToPointHelper pointToPoint;
    // changing the default queue type to DropTail with max. packets capacity to 25 packets...
    pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("25p"));
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
    echoClient.SetAttribute("MaxPackets", UintegerValue(1000)); // change this to capture variety of readings...(keep it more)
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.0001))); // change this to capture variety of readings...(keep it less)
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(vClientIndex));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    // STEP - 6 : Changes in code for Practical - 4....
    // To generate ".xml" file
    AnimationInterface anim("Practical-6.xml");
    // below three lines will set constant position of nodes in Animation...
    anim.SetConstantPosition(allNodes.Get(0), 20, 20);
    anim.SetConstantPosition(allNodes.Get(1), 40, 20);
    anim.SetConstantPosition(allNodes.Get(2), 60, 20);
    // below line will generate ".pcap" files for each NIC configured with this point to point helper...
    pointToPoint.EnablePcapAll("Pratical6");
    
    // STEP - 7 : Setting up the Trace Sources...
    // setting a event handler that captures the thrown values by Trace Sources of NetDevices
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Enqueue", MakeCallback(&EnQ));
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Dequeue", MakeCallback(&DeQ));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
