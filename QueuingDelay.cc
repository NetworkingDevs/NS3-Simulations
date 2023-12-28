#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//(C)   54.0.0.0/8    55.0.0.0/8   (S)
// n0 -------------- n1------------n3
//    
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int queueCounter = 0;
int packetCounterEnQ = 0;
int packetCounterDeQ = 0;

void HenilEnQ(std::string ctx, Ptr<const Packet> pkt)
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

}

void HenilDeQ(std::string ctx, Ptr<const Packet> pkt)
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
}

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    // LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	
    // creating nodes here ==========================================
    NodeContainer nodes,node01,node12;
    nodes.Create(3);
    node01.Add(nodes.Get(0));
    node01.Add(nodes.Get(1));
    node12.Add(nodes.Get(1));
    node12.Add(nodes.Get(2));

    // creating links here ==========================================
    PointToPointHelper p2pHigh,p2pLow;
    // This link, with high data rate and low delay...
    p2pHigh.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
    p2pHigh.SetChannelAttribute("Delay", StringValue("2ms"));
    // This link, with low data rate and high delay...
    p2pLow.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2pLow.SetChannelAttribute("Delay", StringValue("20ms"));
    
    // creating devices here =======================================
    NetDeviceContainer devices01,devices12;
    devices01 = p2pHigh.Install(node01);
    devices12 = p2pLow.Install(node12);
    
    
    // ip assignment here ==========================================
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    
    address.SetBase("55.0.0.", "255.0.0.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    
    // server configuration here =====================================
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(30.0));
    
    // client configuration here ===================================
    UdpEchoClientHelper echoClient(interfaces12.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.001)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(30.0));
    
    // setting a event handler that captures the thrown values by Trace Sources of NetDevices
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Enqueue", MakeCallback(&HenilEnQ));
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Dequeue", MakeCallback(&HenilDeQ));
    
    // configuring utilities and routing...
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("Henil_QueuingDelay.xml");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
