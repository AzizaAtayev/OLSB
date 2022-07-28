/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Aziza Atayev
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Aziza Atayev <azizaa@post.bgu.ac.il>
 * Kobi lab reference
 * Ben Gurion University (BGU)
 * Department of Electrical Engineering
 * Beer Sheva, Israel.
 *
 */

#include <fstream>
#include <cmath>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsb-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/trace-helper.h"

using namespace ns3;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("olsb-routing-compare");

/**
 * \ingroup olsb
 * \ingroup olsb-examples
 * \ingroup examples
 *
 * \brief OLSB Manet example
 */
class OlsbRoutingExperiment
{
public:
  OlsbRoutingExperiment ();
  /**
   * Run function
   * \param nWifis The total number of nodes
   * \param nSinks The total number of receivers
   * \param totalTime The total simulation time
   * \param rate The network speed
   * \param phyMode The physical mode
   * \param nodeSpeed The node speed
   * \param periodicUpdateInterval The routing update interval
   * \param settlingTime The routing update settling time
   * \param shortestPathFactor Shortest path factor 
   * \param backpressurFactor Backpressure factor
   * \param dataStart The data transmission start time
   * \param printRoutes print the routes if true
   * \param CSVfileName The CSV file name
   */
  void CaseRun ();
  void setRunParam(uint32_t nWifis, uint32_t nSinks, double totalTime, std::string rate,
            std::string phyMode, uint32_t nodeSpeed, uint32_t periodicUpdateInterval, 
            uint32_t settlingTime, double shortestPathFactor, double backpressurFactor,
            double dataStart, bool printRoutes, std::string CSVfileName);
  void setRunParam(uint32_t nWifis, uint32_t nSinks, double shortestPathFactor, 
            double backpressurFactor);


private:
  uint32_t m_nWifis; ///< total number of nodes
  uint32_t m_nSinks; ///< number of receiver nodes
  double m_totalTime; ///< total simulation time (in seconds)
  std::string m_rate; ///< network bandwidth
  std::string m_phyMode; ///< remote station manager data mode
  uint32_t m_nodeSpeed; ///< mobility speed
  uint32_t m_periodicUpdateInterval; ///< routing update interval
  uint32_t m_settlingTime; ///< routing setting time
  double m_shortestPathFactor; ///< shorest path factor
  double m_backpressurFactor; ///< backpressure factor
  double m_dataStart; ///< time to start data transmissions (seconds)
  uint32_t bytesTotal; ///< total bytes received by all nodes
  uint32_t packetsReceived; ///< total packets received by all nodes
  bool m_printRoutes; ///< print routing table
  std::string m_CSVfileName; ///< CSV file name
  std::string m_protocolName; //!< Protocol name.
  bool m_traceMobility;       //!< Enavle mobility tracing.

  NodeContainer nodes; ///< the collection of nodes
  NetDeviceContainer devices; ///< the collection of devices
  Ipv4InterfaceContainer interfaces; ///< the collection of interfaces

private:
  /// Create and initialize all nodes
  void CreateNodes ();
  /**
   * Create and initialize all devices
   * \param tr_name The trace file name
   */
  void CreateDevices (std::string tr_name);
  /**
   * Create network
   * \param tr_name The trace file name
   */
  void InstallInternetStack (std::string tr_name);
  /// Create data sinks and sources
  void InstallApplications ();
  /// Setup mobility model
  void SetupMobility (std::string tr_name);
  /**
   * Packet receive function
   * \param socket The communication socket
   */
  void ReceivePacket (Ptr <Socket> socket);
  /**
   * Setup packet receivers
   * \param addr the receiving IPv4 address
   * \param node the receiving node
   * \returns the communication socket
   */
  Ptr <Socket> SetupPacketReceive (Ipv4Address addr, Ptr <Node> node );
  /// Check network throughput
  void CheckThroughput ();
  /// Change factors and protocol in testing algorithm
  /// newShortestPathFactor = 0 -> doing backpressur only
  /// newBackpressurFactor = 0 -> doing shortest path only
  void change_protocol(double newShortestPathFactor, double newBackpressurFactor);
};


OlsbRoutingExperiment::OlsbRoutingExperiment ()
{
  bytesTotal = 0;
  packetsReceived = 0;
  m_CSVfileName = "05-olsb-routing-experiment.output.csv";
  m_traceMobility = true;
  m_shortestPathFactor = 0.5;
  m_backpressurFactor = 0.5;
  m_protocolName = "05-olsb";
  m_nWifis = 30;
  m_nSinks = 10;
  m_totalTime = 100;
  m_rate = "8kbps";
  m_phyMode = "DsssRate11Mbps";
  m_nodeSpeed = 10;
  m_periodicUpdateInterval = 15;
  m_settlingTime = 6;
  m_dataStart = 50;
  m_printRoutes = true;
}


static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}


void
OlsbRoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_UNCOND (Simulator::Now ().As (Time::S) << " Received one packet!");
  Ptr<Packet> packet;
  Address senderAddress;
  // while ((packet = socket->Recv ()))
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}


void
OlsbRoutingExperiment::CheckThroughput ()
{
    double kbs = (bytesTotal * 8.0) / 1000;
    bytesTotal = 0;

    std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

    out << (Simulator::Now ()).GetSeconds () << "," 
        << kbs << "," 
        << packetsReceived << "," 
        << m_nSinks << ","
        << m_protocolName << ","
        << std::endl;

    out.close ();
    packetsReceived = 0;
    Simulator::Schedule (Seconds (1.0), &OlsbRoutingExperiment::CheckThroughput, this);
  
}

Ptr<Socket>
OlsbRoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&OlsbRoutingExperiment::ReceivePacket, this));

  return sink;
}

void
OlsbRoutingExperiment::CreateNodes ()
{
  std::cout << "Creating " << (unsigned) m_nWifis << " nodes.\n";
  nodes.Create (m_nWifis);
  NS_ASSERT_MSG (m_nWifis > m_nSinks, "Sinks must be less or equal to the number of nodes in network");
}

void
OlsbRoutingExperiment::SetupMobility (std::string tr_name)
{
  MobilityHelper mobility;
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));

  std::ostringstream speedConstantRandomVariableStream;
  speedConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant="
                                    << m_nodeSpeed
                                    << "]";

  Ptr <PositionAllocator> taPositionAlloc = pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", StringValue (speedConstantRandomVariableStream.str ()),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"), "PositionAllocator", PointerValue (taPositionAlloc));
  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.Install (nodes);
  if (m_traceMobility)
  {
    AsciiTraceHelper ascii;
    MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));
  }
}

void
OlsbRoutingExperiment::CreateDevices (std::string tr_name)
{
  // we can set the transmition power here
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (m_phyMode), "ControlMode",
                                StringValue (m_phyMode));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream (tr_name + ".tr"));
  wifiPhy.EnablePcapAll (tr_name);
}

void
OlsbRoutingExperiment::InstallInternetStack (std::string tr_name)
{
  OlsbHelper olsb;
  olsb.Set ("PeriodicUpdateInterval", TimeValue (Seconds (m_periodicUpdateInterval)));
  olsb.Set ("SettlingTime", TimeValue (Seconds (m_settlingTime)));
  olsb.Set("ShortestPathFactor", DoubleValue(m_shortestPathFactor));
  olsb.Set("BackpressureFactor", DoubleValue(m_backpressurFactor));
  InternetStackHelper stack;
  stack.SetRoutingHelper (olsb); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  if (m_printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ((tr_name + ".routes"), std::ios::out);
      olsb.PrintRoutingTableAllAt (Seconds (m_periodicUpdateInterval), routingStream);
    }
}

void
OlsbRoutingExperiment::InstallApplications ()
{
  for (uint32_t i = 0; i <= m_nSinks - 1; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = SetupPacketReceive (nodeAddress, node);
    }

  for (uint32_t clientNode = 0; clientNode <= m_nWifis - 1; clientNode++ )
    {
      for (uint32_t j = 0; j <= m_nSinks - 1; j++ )
        {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (j), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

          // AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
          // onoff1.SetAttribute ("Remote", remoteAddress);

          if (j != clientNode)
            {
              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (var->GetValue (m_dataStart, m_dataStart + 1)));
              apps1.Stop (Seconds (m_totalTime));
            }
        }
    }
}


void 
OlsbRoutingExperiment::change_protocol(double newShortestPathFactor, double newBackpressurFactor)
{
    m_shortestPathFactor = newShortestPathFactor;
    m_backpressurFactor = newBackpressurFactor;
    m_protocolName = std::to_string(int(newShortestPathFactor*10)) + "-olsb";
    m_CSVfileName = m_protocolName + "-routing-experiment.output.csv";
    
    std::ofstream out (m_CSVfileName.c_str ());
    
    out << "SimulationSecond," <<
    "ReceiveRate," <<
    "PacketsReceived," <<
    "NumberOfSinks," <<
    "RoutingProtocol," <<
    std::endl;
    out.close ();
}
  
void
OlsbRoutingExperiment::setRunParam(uint32_t nWifis, uint32_t nSinks, double totalTime, std::string rate,
            std::string phyMode, uint32_t nodeSpeed, uint32_t periodicUpdateInterval, 
            uint32_t settlingTime, double shortestPathFactor, double backpressurFactor,
            double dataStart, bool printRoutes, std::string CSVfileName)
{
  m_nWifis = nWifis;
  m_nSinks = nSinks;
  m_totalTime = totalTime;
  m_rate = rate;
  m_phyMode = phyMode;
  m_nodeSpeed = nodeSpeed;
  m_periodicUpdateInterval = periodicUpdateInterval;
  m_settlingTime = settlingTime;
  m_shortestPathFactor = shortestPathFactor;
  m_backpressurFactor = backpressurFactor;
  m_dataStart = dataStart;
  m_printRoutes = printRoutes;
  m_CSVfileName = CSVfileName;
}

void
OlsbRoutingExperiment::setRunParam(uint32_t nWifis, uint32_t nSinks, double shortestPathFactor, 
            double backpressurFactor)
{
  m_nWifis = nWifis;
  m_nSinks = nSinks;
  change_protocol(shortestPathFactor, backpressurFactor);
}


void
OlsbRoutingExperiment::CaseRun ()
{
  Packet::EnablePrinting ();
  std::stringstream ss;
  ss << m_nWifis;
  std::string t_nodes = ss.str ();

  std::stringstream ss3;
  ss3 << m_totalTime;
  std::string sTotalTime = ss3.str ();

  std::string tr_name = m_protocolName + t_nodes + "Nodes_" + sTotalTime + "SimTime";
  std::cout << "Trace file generated is " << tr_name << ".tr\n";

  CreateNodes ();
  CreateDevices (tr_name);
  SetupMobility (tr_name);
  InstallInternetStack (tr_name);
  InstallApplications ();

  // Ptr<FlowMonitor> flowmon;
  // FlowMonitorHelper flowmonHelper;
  // flowmon = flowmonHelper.InstallAll ();

  std::cout << "\nStarting simulation for " << m_totalTime << " s ...\n";

  CheckThroughput ();

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();

  // flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);

  Simulator::Destroy ();
}


int
main (int argc, char *argv[])
{
  SeedManager::SetSeed (12345);

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("8kbps"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue ("DsssRate11Mbps"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));


  OlsbRoutingExperiment experiment;

  
  // first trail
  experiment = OlsbRoutingExperiment();
  experiment.CaseRun();

  // second trail
  experiment = OlsbRoutingExperiment();
  experiment.setRunParam(30, 10, 0, 1);
  experiment.CaseRun();

  // thrid trail
  experiment = OlsbRoutingExperiment();
  experiment.setRunParam(30, 10, 1, 0);
  experiment.CaseRun();

  // four trail
  experiment = OlsbRoutingExperiment();
  experiment.setRunParam(30, 10, 0.2, 0.8);
  experiment.CaseRun();

  // fifth trail
  experiment = OlsbRoutingExperiment();
  experiment.setRunParam(30, 10, 0.8, 0.2);
  experiment.CaseRun();

  return 1;
}
