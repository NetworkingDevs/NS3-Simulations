// Trace Sources Test...
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include <string.h>

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

int enqCount1;
int enqCount2;

int deqCount1;
int deqCount2;

int queCount1;
int queCount2;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

// This is my function that will handle the event fired by the UdpEchoClient...
void HenilReceiveTrace(std::string context, Ptr<const Packet> pkt, const Address& a, const Address& b) 
{
	std::cout << context << std::endl;
	std::cout << "\tRxTrace Size : " << pkt->GetSize()
		  << " From : " << InetSocketAddress::ConvertFrom(a).GetIpv4()
		  << " Local Address : " << InetSocketAddress::ConvertFrom(b).GetIpv4() << std::endl;
}

void HenilTransmitTrace(std::string ctx, Ptr<const Packet> pkt, const Address& a, const Address& b) 
{
	std::cout << ctx << std::endl;
	std::cout << "\tRxTrace Size : " << pkt->GetSize()
		  << " Local Address : " << InetSocketAddress::ConvertFrom(a).GetIpv4()
		  << " Target : " << InetSocketAddress::ConvertFrom(b).GetIpv4() << std::endl;
}

void HenilEnQTrace(std::string ctx, Ptr<const Packet> pkt)
{
	char nodeNumber = ctx[10];
	int internalPktCounter = 0;
	int internalQCounter = 0;
	
	if(nodeNumber == '0')
	{
		internalPktCounter = ++enqCount1;
		internalQCounter = ++queCount1;
	}
	else
	{
		internalPktCounter = ++enqCount2;
		internalQCounter = ++queCount2;
	}
	std::cout 
		<< ctx[10]
		<< " , "
		<< internalPktCounter
		<< " , "  
		<< Time(Simulator::Now()).GetNanoSeconds() 
		<< " , " 
		<< internalQCounter 
		<< " , " 
		<< "enq" 
		<< std::endl;

//	std::cout << ctx
//		  << "\tTime : " << Simulator::Now()
//		  << " Packet of size : " << pkt->GetSize()
//		  << " enqueued!" << std::endl;
}

void HenilDeQTrace(std::string ctx, Ptr<const Packet> pkt)
{
	char nodeNumber = ctx[10];
	int internalPktCounter = 0;
	int internalQCounter = 0;
	
	if(nodeNumber == '0')
	{
		internalPktCounter = ++deqCount1;
		internalQCounter = --queCount1;
	}
	else
	{
		internalPktCounter = ++deqCount2;
		internalQCounter = --queCount2;
	}
	std::cout 
		<< ctx[10]
		<< " , "
		<< internalPktCounter
		<< " , "  
		<< Time(Simulator::Now()).GetNanoSeconds()
		<< " , " 
		<< internalQCounter 
		<< " , " 
		<< "deq" 
		<< std::endl;

//	std::cout << ctx
//		  << "\tTime : " << Simulator::Now()
//		  << " Packet of size : " << pkt->GetSize()
//		  << " dequeued!" << std::endl;
}

int
main(int argc, char* argv[])
{

    deqCount1 = 0;
    enqCount1 = 0;
    queCount1 = 0;
    deqCount2 = 0;
    enqCount2 = 0;
    queCount2 = 0;
    
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    // These are the log componets...
    // LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.00001)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    
    // This configuration lines should always be after application container is assigned to node...

    // setting a max packets to be 5...
    // this line is equivalent to "echoClient.SetAttribute("MaxPackets", UintegerValue(5));" for all client..
    Config::Set("/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/MaxPackets", UintegerValue(5));
    
    // setting a event handler that captures the thrown values by the UdpEchoClient's TraceSource
    // Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/RxWithAddresses", MakeCallback(&HenilReceiveTrace));
    // Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/TxWithAddresses", MakeCallback(&HenilTransmitTrace));
    
    // setting a event handler that captures the thrown values by Trace Sources of NetDevices
    Config::Connect("/NodeList/*/DeviceList/*/TxQueue/Enqueue", MakeCallback(&HenilEnQTrace));
    Config::Connect("/NodeList/*/DeviceList/*/TxQueue/Dequeue", MakeCallback(&HenilDeQTrace));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
