#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//    10.1.5.0                        10.1.6.0
//    ================  point to     ================
//    |    |    |    |  point        |    |    |    |
//    n4   n5   n6   n0--------------n1   n7   n8   n9
//                    -   10.1.1.0    -                       
//    point to point  -               -  point to point            
//     10.1.2.0       -               -   10.1.4.0                              
//                    -               -
//                    -   10.1.3.0    -
//    n10  n11  n12   n2-------------n3  n13  n14   n15                  
//     |    |    |    |    point to   |   |    |     |
//    =================    point      ================
//      10.1.7.0                           10.1.8.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WiredSimulation");

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


// uint64_t no_of_TCP_flows = 1;

// ApplicationContainer sink_all;                         /* Pointer to the packet sink application */
// uint64_t lastTotalRx[1] = {0};                     /* The value of the last total received bytes */

// AsciiTraceHelper graphascii;
// Ptr<OutputStreamWrapper> throughputstream[3];

// void
// CalculateThroughput ()
// {
// 	uint32_t sink_no = sink_all.GetN();
// 	for(uint32_t i = 0; i < sink_no; i++){
// 		Ptr<PacketSink> sink = StaticCast<PacketSink> (sink_all.Get(i));
// 		Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
// 		double cur = (sink->GetTotalRx () - lastTotalRx[i]) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
// 		*throughputstream[i]->GetStream() << Simulator::Now().GetSeconds() << "		" << cur << std::endl;
// 		//std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
// 		lastTotalRx[i] = sink->GetTotalRx ();
// 	}
  
//   Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
// }

int 
main (int argc, char *argv[])
{
  double simulationTime = 10; 
  uint64_t no_of_TCP_flows = 1;
  uint16_t sinkPort = 8080;
  uint32_t nCsmaNodes1 = 3;
  uint32_t nCsmaNodes2 = 3;
  uint32_t nCsmaNodes3 = 3;
  uint32_t nCsmaNodes4 = 3;
  int packetsize = 1024;
  int totalpacket = 1000;
  std::string dataRate;
  int rateinput = 1;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsmaNodes1", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes1);
  cmd.AddValue ("nCsmaNodes2", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes2);
  cmd.AddValue ("nCsmaNodes3", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes3);
  cmd.AddValue ("nCsmaNodes4", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes4);

  cmd.Parse (argc,argv);

  dataRate = std::to_string(rateinput) + "Mbps";
  

  Ptr<Node> n0 = CreateObject<Node> ();
  Ptr<Node> n1 = CreateObject<Node> ();
  Ptr<Node> n2 = CreateObject<Node> ();
  Ptr<Node> n3 = CreateObject<Node> ();

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10us"));

  NetDeviceContainer ptop01 = pointToPoint.Install(n0,n1);
  NetDeviceContainer ptop02 = pointToPoint.Install(n0,n2);
  NetDeviceContainer ptop23 = pointToPoint.Install(n2,n3);
  NetDeviceContainer ptop13 = pointToPoint.Install(n1,n3);
  
  // 10.1.5.0 
  NodeContainer csmaNodes1;
  csmaNodes1.Add(n0);
  csmaNodes1.Create(nCsmaNodes1);

  CsmaHelper csmahelper1;
  csmahelper1.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmahelper1.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices1;
  csmaDevices1 = csmahelper1.Install (csmaNodes1);

  // 10.1.6.0
  NodeContainer csmaNodes2;
  csmaNodes2.Add(n1);
  csmaNodes2.Create(nCsmaNodes2);

  CsmaHelper csmahelper2;
  csmahelper2.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmahelper2.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices2;
  csmaDevices2 = csmahelper2.Install (csmaNodes2);
  
  //10.1.7.0
  NodeContainer csmaNodes3;
  csmaNodes3.Add(n2);
  csmaNodes3.Create(nCsmaNodes3);

  CsmaHelper csmahelper3;
  csmahelper3.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmahelper3.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices3;
  csmaDevices3 = csmahelper3.Install (csmaNodes3);
  
  //10.1.8.0
  NodeContainer csmaNodes4;
  csmaNodes4.Add(n3);
  csmaNodes4.Create(nCsmaNodes4);

  CsmaHelper csmahelper4;
  csmahelper4.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmahelper4.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices4;
  csmaDevices4 = csmahelper4.Install (csmaNodes4);

  InternetStackHelper stack;
  stack.InstallAll();

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop01_interface = address.Assign (ptop01);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop02_interface = address.Assign (ptop02);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop23_interface = address.Assign (ptop23);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop13_interface = address.Assign (ptop13);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer csma1_interface = address.Assign (csmaDevices1);

  address.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer csma2_interface = address.Assign (csmaDevices2);

  address.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer csma3_interface = address.Assign (csmaDevices3);

  address.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer csma4_interface = address.Assign (csmaDevices4);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // flow
  for(uint64_t i = 0; i < no_of_TCP_flows; i++) {
    Address sinkAddress(InetSocketAddress(csma3_interface.GetAddress(i), sinkPort));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = sinkHelper.Install (csmaNodes3.Get(i));

    //sink_all.Add(StaticCast<PacketSink> (sinkApps.Get (0)));

    sinkApps.Start (Seconds (0.));
    sinkApps.Stop (Seconds (simulationTime));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes1.Get(i), TcpSocketFactory::GetTypeId ());

    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
    csmaNodes1.Get(i)->AddApplication (app);
    app->SetStartTime (Seconds (1.));
    app->SetStopTime (Seconds (simulationTime));
    
    //Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
    Simulator::Stop (Seconds (simulationTime));
    
    //-------- trace ------ //

    // AsciiTraceHelper asciiTraceHelper;
    // Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("customnet_" + std::to_string(i) + ".cwnd");
    // ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));

  }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
