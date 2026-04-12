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

    NodeContainer nodes0, nodes1, router;
    router.Create(1);
    nodes0.Create(3);
    nodes0.Add(router.Get(0));
    nodes1.Add(router.Get(0));
    nodes1.Create(3);

    CsmaHelper csmaChannel;
    csmaChannel.SetChannelAttribute("DataRate", StringValue("5Mbps"));
    csmaChannel.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices0, devices1;
    devices0 = csmaChannel.Install(nodes0);
    devices1 = csmaChannel.Install(nodes1);

    InternetStackHelper stack;
    stack.Install(nodes0);
    stack.Install(nodes1);

    Ipv4AddressHelper address;
    address.SetBase("192.168.10.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces0 = address.Assign(devices0);

    address.SetBase("192.168.20.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces1 = address.Assign(devices1);

    PingHelper pingHelper(Ipv4Address("192.168.20.2"));
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    pingHelper.SetAttribute("Size", UintegerValue(1024));
    pingHelper.SetAttribute("Count", UintegerValue(4));

    ApplicationContainer app = pingHelper.Install(nodes0.Get(0));
    app.Start(Seconds(2.0));
    app.Stop(Seconds(10.0));

    AnimationInterface anim("first-arp-simulation.xml");
    anim.SetConstantPosition(nodes0.Get(0), 10, 20);
    anim.SetConstantPosition(nodes0.Get(1), 20, 20);
    anim.SetConstantPosition(nodes0.Get(2), 30, 20);

    anim.SetConstantPosition(nodes0.Get(3), 35, 10);

    anim.SetConstantPosition(nodes1.Get(0), 40, 20);
    anim.SetConstantPosition(nodes1.Get(1), 50, 20);
    anim.SetConstantPosition(nodes1.Get(2), 60, 20);

    csmaChannel.EnablePcapAll("arp_simulation");

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(30.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
