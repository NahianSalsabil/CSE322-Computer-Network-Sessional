#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"
#include <ns3/lr-wpan-error-model.h>
#include "ns3/network-module.h"

using namespace ns3;

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
 
int main (int argc, char** argv)
{
  uint32_t no_of_TCP_flows = 10;
  uint16_t sinkPort = 9;
  double start_time = 0;
  double simulationTime = 10;
  uint32_t nWirelessNodes = 5;
  int packetsize = 1024;
  int totalpacket = 1000;
  std::string dataRate;
  uint64_t packetpersecond = 500;
  double maxRange = 5;
 
  Packet::EnablePrinting ();
 
  CommandLine cmd (__FILE__);
  cmd.AddValue ("no_of_TCP_flows", "Number of TCP Flow", no_of_TCP_flows);
  cmd.AddValue ("nWirelessNodes", "Number of \"extra\" WSN nodes/devices", nWirelessNodes);
  cmd.AddValue ("packetperscond", "packet per second", packetpersecond);

  cmd.Parse (argc,argv);

  dataRate = std::to_string(packetpersecond*packetsize*8/(std::pow(2,20))) + "Mbps";

  std::cout << "cWsnNodes: " << nWirelessNodes << std::endl;
  std::cout << "no of flow: " << no_of_TCP_flows << std::endl;
  std::cout << "pps: " << packetpersecond << std::endl;
  std::cout << "datarate: " << dataRate << "\n";
 
  
  NodeContainer WirelessNodes;
  WirelessNodes.Create (nWirelessNodes);

  std::cout << "wsn nodes: " << WirelessNodes.GetN() << std::endl;

   // creating a channel with range propagation loss model  
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(maxRange));
  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
  Ptr<RangePropagationLossModel> propModel = CreateObject<RangePropagationLossModel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->AddPropagationLossModel (propModel);
  channel->SetPropagationDelayModel (delayModel);
 
  MobilityHelper mobility;
  
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (5.0),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (WirelessNodes);
 
  LrWpanHelper lrWpanHelper;

  lrWpanHelper.SetChannel(channel);

  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install (WirelessNodes);
 
  lrWpanHelper.AssociateToPan (lrwpanDevices, 0);
 
  InternetStackHelper internetv6;
  internetv6.Install (WirelessNodes);
 
  SixLowPanHelper sixLowPanHelper;
  NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install (lrwpanDevices);
 
 
  Ipv6AddressHelper ipv6address;
 
  ipv6address.SetBase (Ipv6Address ("2001:f00d::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer WirelessDeviceInterfaces;
  WirelessDeviceInterfaces = ipv6address.Assign (sixLowPanDevices);
  WirelessDeviceInterfaces.SetForwarding (0, true);
  WirelessDeviceInterfaces.SetDefaultRouteInAllNodes (0);

 
  for (uint32_t i = 0; i < sixLowPanDevices.GetN (); i++)
    {
      Ptr<NetDevice> dev = sixLowPanDevices.Get (i);
      dev->SetAttribute ("UseMeshUnder", BooleanValue (true));
      dev->SetAttribute ("MeshUnderRadius", UintegerValue (10));
    }

  // flow
  uint32_t index=0;
  for(uint64_t i = 0; i < no_of_TCP_flows; i++) {
    Address sinkAddress(Inet6SocketAddress(WirelessDeviceInterfaces.GetAddress(0,1), sinkPort));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = sinkHelper.Install (WirelessNodes.Get(0));

    sinkApps.Start (Seconds (start_time));
    sinkApps.Stop (Seconds (simulationTime));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (WirelessNodes.Get(index+1), TcpSocketFactory::GetTypeId ());

    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (ns3TcpSocket, sinkAddress, packetsize, totalpacket, DataRate (dataRate));
    WirelessNodes.Get(index+1)->AddApplication (app);
    app->SetStartTime (Seconds (start_time + 1));
    app->SetStopTime (Seconds (simulationTime));
   
    Simulator::Stop (Seconds (simulationTime));

    sinkPort++;
    index++;
    if(index == 4){
      index = 0;
    }
  }

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Run ();

  Ptr<Ipv6FlowClassifier> classifier = DynamicCast<Ipv6FlowClassifier> (flowmon.GetClassifier6() );
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

  double total_rxbytes = 0, total_rxpacket = 0, total_txpacket = 0;
  Time total_delaysum;
  double average_throughput, delivery_ratio, drop_ratio;
  Time endtoend_delay;
  uint32_t count = 0;

  for(auto iter = stats.begin(); iter != stats.end(); ++iter){
	  Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
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
  // resultfile1.open(dir + "wireless_nodes.txt", std::ios_base::app); // append instead of overwrite
  // resultfile1 << nWirelessNodes << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  // std::ofstream resultfile2;
  // resultfile2.open(dir + "wireless_flow.txt", std::ios_base::app); // append instead of overwrite
  // resultfile2 << no_of_TCP_flows*2 << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  // std::ofstream resultfile3;
  // resultfile3.open(dir + "wireless_packets.txt", std::ios_base::app); // append instead of overwrite
  // resultfile3 << packetpersecond << " "<< average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  // std::ofstream resultfile3;
  // resultfile3.open(dir + "wireless_coverage.txt", std::ios_base::app); // append instead of overwrite
  // resultfile3 << 5*maxRange << " " << average_throughput << " " << endtoend_delay <<  " " << delivery_ratio << " " << drop_ratio << std::endl; 

  Simulator::Destroy ();
  return 0;
}