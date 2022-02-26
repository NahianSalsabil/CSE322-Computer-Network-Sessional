#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"

//    Network Topology
//
//    10.1.5.0                        10.1.6.0
//    ================  point to     =================
//    |    |    |    |  point        |    |    |    |
//    n4   n5   n6   n0--------------n1   n7   n8   n9
//                     -   10.1.1.0   -                       
//         point to point  -           -  point to point            
//                10.1.2.0    -       -   10.1.4.0                              
//                                -   -
//                        10.1.3.0    -
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

int 
main (int argc, char *argv[])
{
  std::string tcpCongestionAlgo = std::string("ns3::") + "TcpCubic";
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName (tcpCongestionAlgo)));

  double simulationTime = 10; 
  uint64_t no_of_TCP_flows = 8;
  DoubleValue errorRate = 0.0001;
  uint32_t nCsmaNodes1 = 5;
  uint32_t nCsmaNodes2 = 5;
  uint32_t nCsmaNodes3 = 5;
  uint32_t nCsmaNodes4 = 5;
  int packetsize = 1024;
  int totalpacket = 5000;
  std::string dataRate;
  uint64_t packetpersecond = 500;
  uint16_t sinkPort = 8080;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("no_of_TCP_flows", "Number of TCP Flow", no_of_TCP_flows);
  cmd.AddValue ("nCsmaNodes1", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes1);
  cmd.AddValue ("nCsmaNodes2", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes2);
  cmd.AddValue ("nCsmaNodes3", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes3);
  cmd.AddValue ("nCsmaNodes4", "Number of \"extra\" CSMA nodes/devices", nCsmaNodes4);
  cmd.AddValue ("packetperscond", "packet per second", packetpersecond);

  cmd.Parse (argc,argv);

  dataRate = std::to_string(packetpersecond*packetsize*8/(std::pow(2,20))) + "Mbps";
  std::cout << "csmanodes1: " << nCsmaNodes1 << std::endl;
  std::cout << "csmanodes2: " << nCsmaNodes2 << std::endl;
  std::cout << "csmanodes3: " << nCsmaNodes3 << std::endl;
  std::cout << "csmanodes4: " << nCsmaNodes4 << std::endl;
  std::cout << "no of flow: " << no_of_TCP_flows << std::endl;
  std::cout << "pps: " << packetpersecond << std::endl;
  std::cout << "datarate: " << dataRate << "\n";
  
  Ptr<Node> n0 = CreateObject<Node> ();
  Ptr<Node> n1 = CreateObject<Node> ();
  Ptr<Node> n2 = CreateObject<Node> ();
  Ptr<Node> n3 = CreateObject<Node> ();

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10us"));

  NetDeviceContainer ptop01 = pointToPoint.Install(n0,n1);
  NetDeviceContainer ptop23 = pointToPoint.Install(n2,n3);
  NetDeviceContainer ptop13 = pointToPoint.Install(n1,n3);
  NetDeviceContainer ptop03 = pointToPoint.Install(n0,n3);

  // 10.1.5.0 
  NodeContainer csmaNodes1;
  csmaNodes1.Add(n0);
  csmaNodes1.Create(nCsmaNodes1);

  CsmaHelper csmahelper1;
  csmahelper1.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csmahelper1.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices1;
  csmaDevices1 = csmahelper1.Install (csmaNodes1);

  // 10.1.6.0
  NodeContainer csmaNodes2;
  csmaNodes2.Add(n1);
  csmaNodes2.Create(nCsmaNodes2);

  CsmaHelper csmahelper2;
  csmahelper2.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csmahelper2.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices2;
  csmaDevices2 = csmahelper2.Install (csmaNodes2);
  
  //10.1.7.0
  NodeContainer csmaNodes3;
  csmaNodes3.Add(n2);
  csmaNodes3.Create(nCsmaNodes3);

  CsmaHelper csmahelper3;
  csmahelper3.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csmahelper3.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices3;
  csmaDevices3 = csmahelper3.Install (csmaNodes3);
  
  //10.1.8.0
  NodeContainer csmaNodes4;
  csmaNodes4.Add(n3);
  csmaNodes4.Create(nCsmaNodes4);

  CsmaHelper csmahelper4;
  csmahelper4.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csmahelper4.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices4;
  csmaDevices4 = csmahelper4.Install (csmaNodes4);

  //Error Model
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (errorRate));

  for(uint32_t i = 0; i < 2; i++){
    ptop01.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    ptop03.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    ptop23.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    ptop13.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  }

  for(uint32_t i = 0; i < nCsmaNodes1; i++){
    csmaDevices1.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  }
  for(uint32_t i = 0; i < nCsmaNodes2; i++){
    csmaDevices2.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  }
  for(uint32_t i = 0; i < nCsmaNodes3; i++){
    csmaDevices3.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  }
  for(uint32_t i = 0; i < nCsmaNodes4; i++){
    csmaDevices4.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  }

  InternetStackHelper stack;
  stack.InstallAll();

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop01_interface = address.Assign (ptop01);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ptop03_interface = address.Assign (ptop03);

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
  uint32_t index = 0;
  uint32_t phase = 1;
  for(uint64_t i = 0; i < no_of_TCP_flows/2; i++) {
    if(phase == 1){
      Address sinkAddress(InetSocketAddress(csma4_interface.GetAddress(index), sinkPort));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = sinkHelper.Install (csmaNodes4.Get(index));

      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes1.Get(index), TcpSocketFactory::GetTypeId ());

      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
      csmaNodes1.Get(index)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTime));
      
      Simulator::Stop (Seconds (simulationTime));
    }

    if(phase == 2){
      Address sinkAddress(InetSocketAddress(csma3_interface.GetAddress(index), sinkPort));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = sinkHelper.Install (csmaNodes3.Get(index));

      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes2.Get(index), TcpSocketFactory::GetTypeId ());

      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
      csmaNodes1.Get(index)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTime));
      
      Simulator::Stop (Seconds (simulationTime));
    }
    
    index++;
    if(index == no_of_TCP_flows/4){
      index = 0;
      phase = 2;
    } 
  }

  index = 0;
  phase = 1;
  for(uint64_t i = 0; i < no_of_TCP_flows/2; i++) {
    if(phase == 1){
      Address sinkAddress(InetSocketAddress(csma2_interface.GetAddress(index), sinkPort));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = sinkHelper.Install (csmaNodes2.Get(index));
      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes3.Get(index), TcpSocketFactory::GetTypeId ());

      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
      csmaNodes1.Get(index)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTime));
      
      Simulator::Stop (Seconds (simulationTime));
    }
    if(phase == 2){
      Address sinkAddress(InetSocketAddress(csma1_interface.GetAddress(index), sinkPort));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = sinkHelper.Install (csmaNodes1.Get(index));

      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes4.Get(index), TcpSocketFactory::GetTypeId ());

      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
      csmaNodes1.Get(index)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTime));
      
      Simulator::Stop (Seconds (simulationTime));
    }

    index++;
    if(index == no_of_TCP_flows/4) {
      index = 0;
      phase = 2;
    }
  }

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier() );
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

  double total_rxbytes = 0, total_rxpacket = 0, total_txpacket = 0;
  Time total_delaysum;
  double average_throughput, delivery_ratio, drop_ratio;
  Time endtoend_delay;
  uint32_t count = 0;

  for(auto iter = stats.begin(); iter != stats.end(); ++iter){
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
	  NS_LOG_UNCOND("====Flow ID: " << iter->first << "====");
	  NS_LOG_UNCOND("src addr: " << t.sourceAddress << "-- dest addr: " << t.destinationAddress);
	  NS_LOG_UNCOND("Sent Packets =" <<iter->second. txPackets);
	  NS_LOG_UNCOND("Received Patkets =" <<iter->second.rxPackets);
    NS_LOG_UNCOND("Dropped Packet = " << iter->second. txPackets - iter->second.rxPackets);
    NS_LOG_UNCOND("Metrics Calculated:");
    NS_LOG_UNCOND("\tNetwork Throughput =" <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) << "bps");
    if(iter->second.rxBytes != 0)
      NS_LOG_UNCOND("\tEnd to End Delay = " << iter->second.delaySum/iter->second.rxPackets);
    else
      NS_LOG_UNCOND("\tEnd to End Delay = NA");
    NS_LOG_UNCOND("\tPacket delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
	  NS_LOG_UNCOND("\tPacket loss ratio =" << (iter->second.txPackets-iter->second.rxPackets)*100.0/iter->second.txPackets << "%");

    // collect all flow info
    total_rxbytes += iter->second.rxBytes;
    total_delaysum += iter->second.delaySum;
    total_rxpacket += iter->second.rxPackets;
    total_txpacket += iter->second.txPackets;
    count++;
  }

  // Calculate Metrics
  average_throughput = (total_rxbytes * 8)/ simulationTime;
  endtoend_delay = total_delaysum/ total_rxpacket;
  delivery_ratio = total_rxpacket * 100 / total_txpacket;
  drop_ratio = (total_txpacket - total_rxpacket) * 100 / total_txpacket;

  std::cout << "Count: " << count << std::endl;
  NS_LOG_UNCOND("Average Throughput: " << average_throughput/1000 << "kbps");
  NS_LOG_UNCOND("End to End Delay: " << endtoend_delay);
  NS_LOG_UNCOND("Packet Delivery Ratio: " << delivery_ratio << "%");
  NS_LOG_UNCOND("Packet Drop Ratio: " << drop_ratio << "%");

  std::string dir = "Output_Files/";
  std::string dirToSave = "mkdir -p " + dir;
    if (system (dirToSave.c_str ()) == -1)
    {
      exit (1);
    }

  // std::ofstream resultfile1;
  // resultfile1.open(dir + "nodes.txt", std::ios_base::app); // append instead of overwrite
  // resultfile1 << nCsmaNodes1+nCsmaNodes2+nCsmaNodes3+nCsmaNodes4 << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  // std::ofstream resultfile2;
  // resultfile2.open(dir + "flow.txt", std::ios_base::app); // append instead of overwrite
  // resultfile2 << no_of_TCP_flows*2 << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  std::ofstream resultfile3;
  resultfile3.open(dir + "packets.txt", std::ios_base::app); // append instead of overwrite
  resultfile3 << packetpersecond << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  Simulator::Destroy ();
  return 0;
}
