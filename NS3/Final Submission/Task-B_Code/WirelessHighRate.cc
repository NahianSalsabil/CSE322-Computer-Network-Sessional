#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/flow-monitor-module.h"

// Default Network Topology
//
//   Wifi 10.1.2.0
//                   AP				   AP
//  *    *    *  *   *
//  |    |    |  |   |    10.1.1.0	   
// n2   n3   n4  n5  n0 -------------- n1   n6   n7   n8  n9
//                   point-to-point    |    |    |    |   |
//                                     *    *    *    *   *
//                                       Wifi 10.1.3.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WirelessHighRateSimulation");

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

// static void
// CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
// {
//   *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
// }

// uint64_t no_of_TCP_flows = 50;

// ApplicationContainer sink_all;                         /* Pointer to the packet sink application */
// uint64_t lastTotalRx[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                     /* The value of the last total received bytes */

// AsciiTraceHelper graphascii;
// Ptr<OutputStreamWrapper> throughputstream[50];

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
  std::string tcpCongestionAlgo1 = std::string("ns3::") + "TcpCubicFit";
  std::string tcpCongestionAlgo2 = std::string("ns3::") + "TcpCubic";
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName (tcpCongestionAlgo1)));

  // for(uint32_t i = 0; i < no_of_TCP_flows; i++){
	//   std::string temp = std::to_string(i);
	//   throughputstream[i] = graphascii.CreateFileStream("Output_Files/cubicfit_throughput_time" + temp);
  // }

  double simulationTime = 50; 
  uint64_t no_of_TCP_flows = 50;
  uint32_t extraWifiNodes1 = 100;
  uint32_t extraWifiNodes2 = 100;
  uint32_t PointToPointNodes = 2;
  DoubleValue maxRange;
  uint32_t range = 10;
  DoubleValue errorRate = 0.001;
  int packetsize = 1024;
  int totalpacket = 1000;
  std::string dataRate;
  uint64_t packetpersecond = 12800;
  uint16_t sinkPort = 8080;
  uint64_t delay = 100;
   

  CommandLine cmd (__FILE__);
  cmd.AddValue ("no_of_TCP_flows", "Number of TCP Flow", no_of_TCP_flows);
  cmd.AddValue ("extraWifiNodes1", "Number of wifi STA devices1", extraWifiNodes1);
  cmd.AddValue ("extraWifiNodes2", "Number of wifi STA devices2", extraWifiNodes2);
  cmd.AddValue ("packetpersecond", "Packet Per Second", packetpersecond);
  cmd.AddValue ("range", "Maximum Covergae Area", range);
  cmd.AddValue ("delay", "Delay", delay);
  cmd.Parse (argc,argv);

  dataRate = std::to_string(packetpersecond*packetsize*8/(std::pow(2,20))) + "Mbps";
  std::cout << "datarate: " << dataRate << "\n";
  maxRange = DoubleValue(range);

  std::string delayms = std::to_string(delay) + "ms";

  NodeContainer p2pNodes;
  p2pNodes.Create (PointToPointNodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (delayms));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer wifiStaNodes1;
  wifiStaNodes1.Create (extraWifiNodes1);
  NodeContainer wifiApNode1 = p2pNodes.Get (0);

  NodeContainer wifiStaNodes2;
  wifiStaNodes2.Create (extraWifiNodes2);
  NodeContainer wifiApNode2 = p2pNodes.Get (1);

  YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default ();
  channel1.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(maxRange));
  YansWifiPhyHelper phy1;
  phy1.SetChannel (channel1.Create ());

  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  channel2.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(maxRange));
  YansWifiPhyHelper phy2;
  phy2.SetErrorRateModel("ns3::NistErrorRateModel");
  phy2.SetChannel (channel2.Create ());

  WifiHelper wifi1;
  wifi1.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiHelper wifi2;
  wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac1;
  Ssid ssid1 = Ssid ("ns-3-ssid");
  mac1.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid1),
               "ActiveProbing", BooleanValue (false));

  WifiMacHelper mac2;
  Ssid ssid2 = Ssid ("ns-3-ssid");
  mac2.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid2),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices1;
  staDevices1 = wifi1.Install (phy1, mac1, wifiStaNodes1);

  NetDeviceContainer staDevices2;
  staDevices2 = wifi2.Install (phy2, mac2, wifiStaNodes2);

  mac1.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid1));

  mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid2));

  NetDeviceContainer apDevices1;
  apDevices1 = wifi1.Install (phy1, mac1, wifiApNode1);

  NetDeviceContainer apDevices2;
  apDevices2 = wifi2.Install (phy2, mac2, wifiApNode2);

  // error model
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (errorRate));

  //after wifi netdevices are created
  Config::Set("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));

  for(uint32_t i = extraWifiNodes1+2; i < extraWifiNodes1+extraWifiNodes2+PointToPointNodes; i++){
    Config::Set("/NodeList/"+std::__cxx11::to_string(i)+"/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
  }
  
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (1.0),
                                 "DeltaY", DoubleValue (0.25),
                                 "GridWidth", UintegerValue (10),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiStaNodes1);
  mobility.Install (wifiStaNodes2);

  
  mobility.Install (wifiApNode1);
  mobility.Install (wifiApNode2);

  InternetStackHelper stack;
  stack.Install (wifiApNode1);
  stack.Install (wifiApNode2);
  stack.Install (wifiStaNodes1);
  stack.Install (wifiStaNodes2);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfaces1 = address.Assign (staDevices1);
  Ipv4InterfaceContainer apInterfaces1 = address.Assign (apDevices1);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfaces2 = address.Assign (staDevices2);
  Ipv4InterfaceContainer apInterfaces2 = address.Assign (apDevices2);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // // for fairness
  // Ipv4Address return_src_add_cubic[no_of_TCP_flows/2+1];
  // Ipv4Address return_src_add_fit[no_of_TCP_flows/2+1];

  // uint32_t index1 = 0;
  // uint32_t index2 = 0;

  // flow
  uint32_t index = 0;
  for(uint64_t i = 0; i < no_of_TCP_flows; i++) {
    // // for fairness
    // TypeId tid;
    // if(i%2 == 0)
    //   tid = TypeId::LookupByName (tcpCongestionAlgo1);
    // if(i%2 != 0)
    //   tid = TypeId::LookupByName (tcpCongestionAlgo2);

    // std::stringstream nodeId;
    // nodeId << wifiStaNodes1.Get (i)->GetId ();
    // std::string specificNode = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketType";
    // Config::Set (specificNode, TypeIdValue (tid));


    Address sinkAddress(InetSocketAddress(staInterfaces2.GetAddress(index), sinkPort));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = sinkHelper.Install (wifiStaNodes2.Get(index));

    // //for fairness - verification print

    // if(i%2 == 0){
    //   return_src_add_fit[index1] = staInterfaces2.GetAddress(i);
    //   index1++;
    // }
    // if(i%2 != 0){
    //   return_src_add_cubic[index2] = staInterfaces2.GetAddress(i);
    //   index2++;
    // }

    //sink_all.Add(StaticCast<PacketSink> (sinkApps.Get (0)));

    sinkApps.Start (Seconds (0.));
    sinkApps.Stop (Seconds (simulationTime));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodes1.Get(index), TcpSocketFactory::GetTypeId ());

    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
    wifiStaNodes1.Get(index)->AddApplication (app);
    app->SetStartTime (Seconds (1.));
    app->SetStopTime (Seconds (simulationTime));
   
    Simulator::Stop (Seconds (simulationTime));

   // Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
    index++;
    if(index == extraWifiNodes1)
      index = 0;
    
    //-------- trace ------ //

    // AsciiTraceHelper asciiTraceHelper;
    // Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("Output_Files/cubic_" + std::to_string(i) + ".cwnd");
    // ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));

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

  // //for jain's index
  // uint32_t count = 0;
  // uint32_t count_fit_flow = 0;
  // uint32_t count_cubic_flow = 0;
  // double throughput_sum_cubic = 0, throughput_sum_fit = 0;
  // double throughput_square_sum_cubic = 0, throughput_square_sum_fit = 0;


  for(auto iter = stats.begin(); iter != stats.end(); ++iter){
	  //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
	  // NS_LOG_UNCOND("====Flow ID: " << iter->first << "====");
	  // NS_LOG_UNCOND("src addr: " << t.sourceAddress << "-- dest addr: " << t.destinationAddress);
	  // NS_LOG_UNCOND("Sent Packets =" <<iter->second. txPackets);
	  // NS_LOG_UNCOND("Received Patkets =" <<iter->second.rxPackets);
    // NS_LOG_UNCOND("Dropped Packet = " << iter->second.txPackets - iter->second.rxPackets);
    // NS_LOG_UNCOND("Metrics Calculated:");
    // double throughput = iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds());
    // NS_LOG_UNCOND("\tNetwork Throughput =" <<throughput << "bps");
    // if(iter->second.rxBytes != 0)
    //   NS_LOG_UNCOND("\tEnd to End Delay = " << iter->second.delaySum/iter->second.rxPackets);
    // else
    //   NS_LOG_UNCOND("\tEnd to End Delay = NA");
    // NS_LOG_UNCOND("\tPacket delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
	  // NS_LOG_UNCOND("\tPacket loss ratio =" << (iter->second.txPackets-iter->second.rxPackets)*100.0/iter->second.txPackets << "%");
  
    // collect all flow info
    total_rxbytes += iter->second.rxBytes;
    total_delaysum += iter->second.delaySum;
    total_rxpacket += iter->second.rxPackets;
    total_txpacket += iter->second.txPackets;

    // // for jain's index
    // if(count < no_of_TCP_flows && count != 7 && count % 2 == 0){          // cubic fit
    //   throughput_sum_fit += throughput;
    //   throughput_square_sum_fit += throughput*throughput;
    //   count_fit_flow++;
    // }
    // if(count < no_of_TCP_flows && count != 7 && count % 2 != 0){          // cubic
    //   throughput_sum_cubic += throughput;
    //   throughput_square_sum_cubic += throughput*throughput;
    //   count_cubic_flow++;
    // }
    // if(count >= no_of_TCP_flows){
    //   for(uint64_t i = 0; i < no_of_TCP_flows/2+1; i++){
    //     if(t.sourceAddress == return_src_add_fit[i]){    // cubic fit 
    //       throughput_sum_fit += throughput;
    //       throughput_square_sum_fit += throughput*throughput;
    //       count_fit_flow++;
    //       break;
    //     } 
    //     else if(t.sourceAddress == return_src_add_cubic[i]){    // cubic
    //       throughput_sum_cubic += throughput;
    //       throughput_square_sum_cubic += throughput*throughput;
    //       count_cubic_flow++;
    //       break;
    //     }
    //   }
    // }

    // count++;
  }
  std::string dir = "Output_Files/";
  std::string dirToSave = "mkdir -p " + dir;
  if (system (dirToSave.c_str ()) == -1)
    exit(1);


  // //Calculate Jain's Index
  // double jains_for_fit = (throughput_sum_fit * throughput_sum_fit) / (count_fit_flow*throughput_square_sum_fit);
  // double jains_for_cubic = (throughput_sum_cubic * throughput_sum_cubic) / (count_cubic_flow*throughput_square_sum_cubic);

  // std::cout << "=== Jain's Index ===\n";
  // std::cout << "jain for fit: " << jains_for_fit << std::endl;
  // std::cout << "jain for cubic: " << jains_for_cubic << std::endl;
  // std::cout << "=========\n\n";

  // std::ofstream janefile;
  // janefile.open(dir + "jain's_index.txt", std::ios_base::app); // append instead of overwrite
  // janefile << delay << " "<< jains_for_fit << " " << jains_for_cubic <<  std::endl;

  // Calculate Metrics
  average_throughput = (total_rxbytes * 8)/ simulationTime;
  endtoend_delay = total_delaysum/ total_rxpacket;
  delivery_ratio = total_rxpacket * 100 / total_txpacket;
  drop_ratio = (total_txpacket - total_rxpacket) * 100 / total_txpacket;

  NS_LOG_UNCOND("Average Throughput: " << average_throughput/1000 << "kbps");
  NS_LOG_UNCOND("End to End Delay: " << endtoend_delay);
  NS_LOG_UNCOND("Packet Delivery Ratio: " << delivery_ratio << "%");
  NS_LOG_UNCOND("Packet Drop Ratio: " << drop_ratio << "%");


  // std::ofstream resultfile1;
  // resultfile1.open(dir + "wirelesshigh_fit_nodes.txt", std::ios_base::app); // append instead of overwrite
  // resultfile1 << extraWifiNodes1+extraWifiNodes2 << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  // std::ofstream resultfile1;
  // resultfile1.open(dir + "wirelesshigh_cubic_nodes.txt", std::ios_base::app); // append instead of overwrite
  // resultfile1 << extraWifiNodes1+extraWifiNodes2 << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  Simulator::Destroy ();
  
  return 0;
}
