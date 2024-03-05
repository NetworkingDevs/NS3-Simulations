#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
 
using namespace ns3;
 
class StopAndWaitClient : public Application
{
public:
    StopAndWaitClient()
    {
        m_socket = 0;
    }
 
    virtual ~StopAndWaitClient()
    {
        m_socket = 0;
    }
 
    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t numPackets, Time interPacketInterval)
    {
        m_socket = socket;
        m_peerAddress = address;
        m_packetSize = packetSize;
        m_numPackets = numPackets;
        m_interPacketInterval = interPacketInterval;
 
        m_socket->Bind();
        m_socket->Connect(m_peerAddress);
 
        SendPacket();
    }
 
private:
    virtual void StartApplication()
    {
    }
 
    virtual void StopApplication()
    {
    }
 
    void SendPacket()
    {
        if (m_seq < m_numPackets)
        {
            Ptr<Packet> packet = Create<Packet>(m_packetSize);
            m_socket->Send(packet);
 
            m_seq++;
 
            Simulator::Schedule(m_interPacketInterval, &StopAndWaitClient::SendPacket, this);
        }
    }
 
private:
    Ptr<Socket> m_socket;
    Address m_peerAddress;
    uint32_t m_packetSize;
    uint32_t m_numPackets;
    Time m_interPacketInterval;
    uint32_t m_seq = 0;
};
 
int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);
 
    Time::SetResolution(Time::NS);
 
    NodeContainer nodes;
    nodes.Create(2);
 
    InternetStackHelper stack;
    stack.Install(nodes);
 
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
 
    NetDeviceContainer devices;
    devices = p2p.Install(nodes);
 
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
 
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
 
    uint16_t port = 9;
 
    Address serverAddress(InetSocketAddress(interfaces.GetAddress(1), port));
 
    Ptr<Socket> socket = Socket::CreateSocket(nodes.Get(0), TypeId::LookupByName("ns3::TcpSocketFactory"));
    Ptr<StopAndWaitClient> client = CreateObject<StopAndWaitClient>();
    nodes.Get(0)->AddApplication(client);
    client->Setup(socket, serverAddress, 1024, 100, Time(Seconds(1.0)));
 
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();
 
    return 0;
}
