// Purpose of this file:
// This file is to demonstrate that how we can measure Change in Congestion Window...
#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

#include <fstream>
std::ofstream enq,deq,mainF;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Tcp_CongestionWindowChange");

/**
 * Congestion window change callback
 *
 * \param stream The output stream file.
 * \param oldCwnd Old congestion window.
 * \param newCwnd New congestion window.
 */
static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd
                         << std::endl;
}

int queueCounter = 0;
int packetCounterEnQ = 0;
int packetCounterDeQ = 0;

void HenilEnQ(std::string ctx, Ptr<const Packet> pkt)
{
	enq << ++packetCounterEnQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << ++queueCounter
	    << std::endl;
	    
	
	mainF << packetCounterEnQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << ","
	    << "enQ"
	    << std::endl;
}

void HenilDeQ(std::string ctx, Ptr<const Packet> pkt)
{
	deq << ++packetCounterDeQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << --queueCounter
	    << std::endl;		    	
	
	mainF << packetCounterDeQ
	    << ","
	    << Time(Simulator::Now()).GetNanoSeconds()
	    << ","
	    << queueCounter
	    << ","
	    << "deQ"
	    << std::endl;
}

/**
 * Rx drop callback
 *
 * \param file The output PCAP file.
 * \param p The dropped packet.
 */
static void
RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    // commneted this line
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
    file->Write(Simulator::Now(), p);
}

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);
    
    enq.open("Henil_EnQ_TCP.txt");
    deq.open("Henil_DeQ_TCP.txt");
    mainF.open("Henil_Main_TCP.txt");

    NodeContainer allNodes,nodes01,nodes12;
    allNodes.Create(3);
    
    nodes01.Add(allNodes.Get(0));
    nodes01.Add(allNodes.Get(1));
    
    nodes12.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(2));

    PointToPointHelper high,low;
    high.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
    high.SetChannelAttribute("Delay", StringValue("2ms"));
    
    low.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    low.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer devices01,devices12;
    devices01 = high.Install(nodes01);
    devices12 = high.Install(nodes12);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices12.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(allNodes);

    Ipv4AddressHelper address;
    address.SetBase("54.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    
    address.SetBase("55.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);

    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces12.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install(allNodes.Get(2));
    sinkApps.Start(Seconds(0.));
    sinkApps.Stop(Seconds(20.));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(allNodes.Get(0), TcpSocketFactory::GetTypeId());

    Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate("1Mbps"));
    allNodes.Get(0)->AddApplication(app);
    app->SetStartTime(Seconds(1.));
    app->SetStopTime(Seconds(20.));

    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("Henil_CwndChange.txt");
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",
                                             MakeBoundCallback(&CwndChange, stream));

    PcapHelper pcapHelper;
    Ptr<PcapFileWrapper> file =
        pcapHelper.CreateFile("Henil_TcpCwnd.pcap", std::ios::out, PcapHelper::DLT_PPP);
    devices01.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file));
    
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    // setting a event handler that captures the thrown values by Trace Sources of NetDevices
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Enqueue", MakeCallback(&HenilEnQ));
    Config::Connect("/NodeList/1/DeviceList/*/TxQueue/Dequeue", MakeCallback(&HenilDeQ));
    
    // AnimationInterface anim("Henil_TcpCwnd.xml");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
