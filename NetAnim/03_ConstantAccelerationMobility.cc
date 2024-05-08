/*
 * This code is made for demonstration of 'Constant Acceleration Mobility Model'
 * You can review this code to set-up ConstantAccelerationMobilityModel
 * in your nodes. Also, this code covers some advance things like
 * object aggeragation and dynamic casting concepts and scheduling events.
 *
 * Major LOC: 68-to-84,34-to-44
 *
 **/

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/constant-acceleration-mobility-model.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

void PrintInfo()
{
	Ptr<Node> n0 = ns3::NodeList::GetNode(0);
	Ptr<Node> n1 = ns3::NodeList::GetNode(1);
	
	Ptr<MobilityModel> m0 = n0->GetObject<MobilityModel>();
	Ptr<MobilityModel> m1 = n1->GetObject<MobilityModel>();
	
	std::cout << "n0 Velocity: " << m0->GetVelocity() << "\t\tn1 Velocity: " << m1->GetVelocity() << std::endl;
	Simulator::Schedule(Seconds(1), &PrintInfo);
}

void ChangeVelo()
{
	Ptr<Node> n1 = ns3::NodeList::GetNode(0);
	
	Ptr<ConstantAccelerationMobilityModel> m1 = DynamicCast<ConstantAccelerationMobilityModel>(n1->GetObject<MobilityModel>());
	
	m1->SetVelocityAndAcceleration(Vector(3,0,0), Vector(-10,0,0));
}

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);
    
    MobilityHelper m;
    m.SetMobilityModel("ns3::ConstantAccelerationMobilityModel");
    m.Install(nodes);
    
    Ptr<ConstantAccelerationMobilityModel> m0 = DynamicCast<ConstantAccelerationMobilityModel>(nodes.Get(0)->GetObject<MobilityModel>());
    Ptr<ConstantAccelerationMobilityModel> m1 = DynamicCast<ConstantAccelerationMobilityModel>(nodes.Get(1)->GetObject<MobilityModel>());
    
    // init. positions
    m0->SetPosition(Vector(50,10,0));
    m1->SetPosition(Vector(75,20,0));
    
    m0->SetVelocityAndAcceleration(Vector(3,0,0), Vector(3,0,0));
    m1->SetVelocityAndAcceleration(Vector(10,0,0), Vector(-3,0,0));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));

    /*NetDeviceContainer devices;
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
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));*/
    
    AnimationInterface anim("acc.xml");
    
    Simulator::Schedule(Seconds(1), &PrintInfo);
    Simulator::Schedule(Seconds(5), &ChangeVelo);
    Simulator::Stop(Seconds(60));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
