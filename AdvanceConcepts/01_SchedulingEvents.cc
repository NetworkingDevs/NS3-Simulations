/*
 * This code is made for demonstration of Scheduling an event.
 * You can review this code to set-up event scheduling
 * in your nodes.
 *
 * Major LOC: 47-to-52
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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

void PrintInfo()
{
	std::cout << "Another 1 Seconds Passed!" << std::endl;
	Simulator::Schedule(Seconds(1), &PrintInfo);
}

void ChangeVelo()
{
	std::cout << "5 Seconds Passed!" << std::endl;
}

int
main(int argc, char* argv[])
{
      
    Simulator::Schedule(Seconds(1), &PrintInfo);
    Simulator::Schedule(Seconds(5), &ChangeVelo);
    Simulator::Stop(Seconds(60));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
