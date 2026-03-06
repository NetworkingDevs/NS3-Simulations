/**
 * This code is mostly the `first.cc` file from the `scratch`
 * directory but it is modified a bit to simulate topology-1.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(3);

    CsmaHelper csmaChannel;
    csmaChannel.SetChannelAttribute("DataRate", StringValue("5Mbps"));
    csmaChannel.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = csmaChannel.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.10.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    PingHelper pingHelper(Ipv4Address("192.168.10.2"));
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    pingHelper.SetAttribute("Size", UintegerValue(1024));
    pingHelper.SetAttribute("Count", UintegerValue(4));

    ApplicationContainer app = pingHelper.Install(nodes.Get(0));
    app.Start(Seconds(2.0));
    app.Stop(Seconds(10.0));

    AnimationInterface anim("first-arp-simulation.xml");
    anim.SetConstantPosition(nodes.Get(0), 10, 10);
    anim.SetConstantPosition(nodes.Get(1), 20, 10);
    anim.SetConstantPosition(nodes.Get(2), 30, 10);

    csmaChannel.EnablePcapAll("basic_arp");

    Simulator::Stop(Seconds(30.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
