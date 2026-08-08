#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE ("LatestBasicTcpSimulation");
 
int main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);
 
    // 1. Initialize two standard simulator nodes
    NodeContainer nodes;
    nodes.Create (2);
 
    // 2. Set up a physical Point-to-Point link channel
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1GB/s"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));
 
    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);
 
    // 3. Install the unified TCP/IP Internet Stack on both nodes
    InternetStackHelper stack;
    stack.Install (nodes);
 
    // 4. Set up the IPv4 address base and assign to interfaces
    Ipv4AddressHelper address;
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);
 
    // 5. Setup the modern TCP Server (Packet Sink Application) on Node 1
    uint16_t port = 8080; 
    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
 
    // Explicit TypeId notation compliant with modern ns-3 coding standards
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
 
    ApplicationContainer serverApps = packetSinkHelper.Install (nodes.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (8.0));
 
    // 6. Setup the TCP Client (Bulk Send Application) on Node 0
    AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (1), port));
 
    // Configured to send continuous stream data as fast as possible 
    BulkSendHelper bulkSendHelper ("ns3::TcpSocketFactory", Address ());
    bulkSendHelper.SetAttribute ("Remote", remoteAddress);
    
    // stream continuos data for entire duration of simulation
    bulkSendHelper.SetAttribute ("MaxBytes", UintegerValue (0)); // 0 = unlimited streaming

    // to change the data budget
    // bulkSendHelper.SetAttribute ("MaxBytes", UintegerValue (50 * 1024 * 1024)); // Stream Fixed File Size = 50MB

    // default payload size = 536 bytes
    // bulkSendHelper.SetAttribute ("SendSize", UintegerValue (1460)); // Set Payload Size Per write operation to MSS

    ApplicationContainer clientApps = bulkSendHelper.Install (nodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (8.0));
 
    // 7. Generate trace files to record raw simulation packet data
    pointToPoint.EnablePcapAll ("latest-tcp-simulation");

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName("ns3::TcpCubic")));
 
    // 8. Execute and safely tear down the discrete event loop
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
