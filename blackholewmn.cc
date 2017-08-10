/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 *
 * Author: Md. Rukunujjaman Miaji <rukucse11@gmail.com>
 * Chittagong University of Engineering & Technology
 * Department of Computer Science & Engineering
 * Student Id : 1104028
 *
 */

#include "ns3/aodv-module.h"
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
#include "myapp.h"
#include "ns3/node.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include <iostream>
#include <sstream>
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("Blackhole Mode Data Transmission in Wireless Mesh Network");

using namespace ns3;
    int       m_xSize=3;
    int       m_ySize=3;
    int       m_sta=1;
    int       m_ap=1;
    uint16_t  m_packetSize=1024;
    int       m_NumOfPacket=50;

int packetCount,received_bits=0,transmitted_bits=0;
float throughput,localThrou,pdf;
float first_transmittedpacket,last_transmittedpacket,sum_of_ete_delay;

FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor;

uint64_t simulationTime = 30; //seconds
//long long  m_defaultAddress;
void ReceivePacket(Ptr<const Packet> p, const Address & addr)
{
    packetCount++;
    std::cout << packetCount << "\t" << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() <<"\n";
    if(packetCount==1)
    {
        first_transmittedpacket=Simulator::Now ().GetSeconds ();
    }

    transmitted_bits+=p->GetSize()*8;
    received_bits+=p->GetSize()*8;
    std::map<FlowId, FlowMonitor::FlowStats> flowStats1 = monitor->GetFlowStats();

    //InetSocketAddress transport = InetSocketAddress::ConvertFrom (addr);

    if(packetCount==m_NumOfPacket)
    {
        last_transmittedpacket=Simulator::Now ().GetSeconds ();
        sum_of_ete_delay=last_transmittedpacket - first_transmittedpacket;

        std::cout<<"*************************************"<<std::endl;
        std::cout<<"Total Sent Packet="<<m_NumOfPacket<<std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout<<"Total Received Packet="<<packetCount<<std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout << "Duration    : " <<Simulator::Now ().GetSeconds ()<< "Seconds" << std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout << "transmitted bits : " <<transmitted_bits<< "bits" << std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout << "received bits : " <<received_bits<< "bits" << std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout << "Throughput    : " << (received_bits)/(Simulator::Now ().GetSeconds () - first_transmittedpacket)/1024/1024 << " Mbps" <<
                  std::endl;

        pdf = (double)packetCount/packetCount;
        std::cout<<"*************************************"<<std::endl;
        std::cout<<"Avereage End to End Delay = "<<sum_of_ete_delay*1000/(packetCount)<<"ms"<<std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout<<"Average Packet Delivery Fraction = "<<pdf<<""<<std::endl;
        std::cout<<"*************************************"<<std::endl;
        std::cout<<"Percentage OF Packet loss = "<<(1-pdf)*100<<"%"<<std::endl;
        std::cout<<"*************************************"<<std::endl;
        packetCount=0;
        transmitted_bits=0;
        received_bits=0;
        sum_of_ete_delay=0;
    }

}

int main (int argc, char *argv[])
{
    bool enableFlowMonitor = false;
    std::string phyMode ("OfdmRate6MbpsBW10MHz");
    ApplicationContainer clientApps;

    CommandLine cmd;
    cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
    cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
    cmd.Parse (argc, argv);

    NS_LOG_INFO ("Create nodes.");
    NodeContainer nc_meshNodes,nc_sta1, nc_sta2,nc_ap11, nc_ap12,nc_ap13, nc_ap14,nc_ap11Mbb1, nc_ap12Mbb1,nc_ap13Mbb1,
                  nc_ap14Mbb1,not_malicious,malicious; // ALL Nodes

    NetDeviceContainer meshDevices,staApDevices1,staApDevices2,staDevices1,staDevices2, apDevices1,apDevices2,ap11Mbb1Devices, ap12Mbb1Devices, ap13Mbb1Devices,ap14Mbb1Devices;
    Ipv4InterfaceContainer interfaces,staAp_interfaces1,staAp_interfaces2,staAp_interfaces3,staAp_interfaces4,csmaInterfaces1,
                           csmaInterfaces2,csmaInterfaces3, csmaInterfaces4;
    CsmaHelper csma1, csma2, csma3, csma4;
//***********************************station device creation********************************************
    nc_sta1.Create(m_sta);
    nc_sta2.Create(m_sta);

//***********************************mesh device creation********************************************
    nc_meshNodes.Create(m_xSize * m_ySize);

    not_malicious.Add(nc_meshNodes.Get(1));
    not_malicious.Add(nc_meshNodes.Get(2));
    not_malicious.Add(nc_meshNodes.Get(3));
    not_malicious.Add(nc_meshNodes.Get(4));
    not_malicious.Add(nc_meshNodes.Get(5));
    not_malicious.Add(nc_meshNodes.Get(6));
    not_malicious.Add(nc_meshNodes.Get(7));
    not_malicious.Add(nc_meshNodes.Get(8));
    malicious.Add(nc_meshNodes.Get(0));

    WifiHelper wifi;

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

    YansWifiChannelHelper wifiChannel ;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
                                    "SystemLoss", DoubleValue(1),
                                    "HeightAboveZ", DoubleValue(1.5));

    wifiPhy.Set ("TxPowerStart", DoubleValue(33));
    wifiPhy.Set ("TxPowerEnd", DoubleValue(33));
    wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
    wifiPhy.Set ("TxGain", DoubleValue(0));
    wifiPhy.Set ("RxGain", DoubleValue(0));
    wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));
    wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-64.8));

    wifiPhy.SetChannel (wifiChannel.Create ());

    // Add a non-QoS upper mac
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();

    NetDeviceContainer devices;
    meshDevices = wifi.Install (wifiPhy, wifiMac, nc_meshNodes);
// *******************************Setup WiFi for network 1**************************************
    WifiHelper wifi1 = WifiHelper::Default();
    wifi1.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi1.SetRemoteStationManager("ns3::AarfWifiManager");

    NqosWifiMacHelper mac1 = NqosWifiMacHelper::Default();

    Ssid ssid1 = Ssid("network-1");
    mac1.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid1),
                 "ActiveProbing", BooleanValue(true));

    staDevices1 = wifi1.Install(wifiPhy, mac1, nc_sta1.Get(0));

    // Setup AP for network 1
    mac1.SetType("ns3::ApWifiMac",
                 "Ssid", SsidValue(ssid1));

    apDevices1.Add(wifi1.Install(wifiPhy, mac1, nc_meshNodes.Get(6)));

    staApDevices1.Add(staDevices1);
    staApDevices1.Add(apDevices1);
    //********************************************End wifi 1*********************************************
    // ***************************************Setup WiFi for network 2***********************************
    WifiHelper wifi2 = WifiHelper::Default();
    wifi2.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi2.SetRemoteStationManager("ns3::AarfWifiManager");

    NqosWifiMacHelper mac2 = NqosWifiMacHelper::Default();

    Ssid ssid2 = Ssid("network-3");
    mac2.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid2),
                 "ActiveProbing", BooleanValue(true));

    staDevices2 = wifi2.Install(wifiPhy, mac2, nc_sta2);

    // Setup AP for network 1
    mac2.SetType("ns3::ApWifiMac",
                 "Ssid", SsidValue(ssid2));

    apDevices2.Add(wifi2.Install(wifiPhy, mac2, nc_meshNodes.Get(3)));

    staApDevices2.Add(staDevices2);
    staApDevices2.Add(apDevices2);

//********************************************End wifi 2********************************************
//***********************************mobility for mesh********************************************

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
    positionAlloc ->Add(Vector(150, 0, 0)); // node0
    positionAlloc ->Add(Vector(200, 0, 0)); // node1
    positionAlloc ->Add(Vector(200, 300, 0)); // node2
    positionAlloc ->Add(Vector(150,300, 0)); // node3
    positionAlloc ->Add(Vector(150, 150, 0)); // node4
    positionAlloc ->Add(Vector(200, 150, 0)); // node5
    positionAlloc ->Add(Vector(250, 0, 0)); // node6
    positionAlloc ->Add(Vector(250,150, 0)); // node7
    positionAlloc ->Add(Vector(250, 300, 0)); // node8
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nc_meshNodes);

//***********************************station node 1 mobility********************************************
    MobilityHelper mobility5;
    Ptr<ListPositionAllocator> positionAlloc5 = CreateObject <ListPositionAllocator>();
    positionAlloc5 ->Add(Vector(250, -100, 0)); // sta 1
    mobility5.SetPositionAllocator(positionAlloc5);
    mobility5.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    //mobility5.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
    //                          "Bounds", RectangleValue (Rectangle (-100, 300, -100, 300)),
    //                          "Speed", StringValue ("ns3::UniformRandomVariable[Min=20.0|Max=50.0]"),
    //                          "Direction", StringValue ("ns3::UniformRandomVariable[Min=10.0|Max=26.283184]"));
    mobility5.Install(nc_sta1);

//***********************************station node 2 mobility********************************************
    MobilityHelper mobility6;
    Ptr<ListPositionAllocator> positionAlloc6 = CreateObject <ListPositionAllocator>();
    positionAlloc6 ->Add(Vector(150, 400, 0)); // sta 2
    mobility6.SetPositionAllocator(positionAlloc6);
    //mobility6.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  //mobility6.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                            "Bounds", RectangleValue (Rectangle (-200, 400, -200, 400)),
  //                            "Speed", StringValue ("ns3::UniformRandomVariable[Min=20.0|Max=50.0]"),
  //                            "Direction", StringValue ("ns3::UniformRandomVariable[Min=10.0|Max=26.283184]"));
    mobility6.Install(nc_sta2);

//  Enable AODV
    AodvHelper aodv;
    AodvHelper malicious_aodv;
    // Set up internet stack
    InternetStackHelper internetStack;
    internetStack.SetRoutingHelper (aodv);
    internetStack.Install (not_malicious);
    internetStack.Install (nc_sta1);
    internetStack.Install (nc_sta2);

    bool IsMalicious=true;
    malicious_aodv.Set("IsMalicious",BooleanValue(IsMalicious));
    internetStack.SetRoutingHelper (malicious_aodv);
    internetStack.Install (malicious);

//***********************************Setup IP Address********************************************
    Ipv4AddressHelper address;
    NS_LOG_INFO ("Assign IP Addresses.");
    address.SetBase ("10.1.2.0", "255.255.255.0");
    interfaces = address.Assign (meshDevices);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    staAp_interfaces1 = address.Assign (staApDevices1);

    address.SetBase ("20.1.3.0", "255.255.255.0");
    staAp_interfaces2 = address.Assign (staApDevices2);

    NS_LOG_INFO ("Create Applications.");

//*******************************************client to nc_ap11*******************************************
    uint16_t sinkPort3 = 7;
    Address sinkAddress3 (InetSocketAddress (staAp_interfaces1.GetAddress (1), sinkPort3)); // interface of n3
    PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort3));
    ApplicationContainer sinkApps3 = packetSinkHelper3.Install (nc_meshNodes.Get(6)); //n3 as sink
    sinkApps3.Start (Seconds (0.));
    sinkApps3.Stop (Seconds (10.));

    Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (nc_sta1.Get (0), UdpSocketFactory::GetTypeId ()); //source at n1

    Ptr<MyApp> app3 = CreateObject<MyApp> ();
    app3->Setup (ns3UdpSocket3, sinkAddress3, 1024, m_NumOfPacket, DataRate ("250Kbps"));
    nc_sta1.Get (0)->AddApplication (app3);
    app3->SetStartTime (Seconds (5.));
    app3->SetStopTime (Seconds (10.));


    uint16_t sinkPort1 = 5;
    Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (3), sinkPort1)); // interface of n3
    PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort1));
    ApplicationContainer sinkApps1 = packetSinkHelper1.Install (nc_meshNodes.Get (3)); //n3 as sink
    sinkApps1.Start (Seconds (10.));
    sinkApps1.Stop (Seconds (20.));

    Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (nc_meshNodes.Get (6), UdpSocketFactory::GetTypeId ()); //source at n1

    Ptr<MyApp> app1 = CreateObject<MyApp> ();
    app1->Setup (ns3UdpSocket1, sinkAddress1, m_packetSize, m_NumOfPacket, DataRate ("500Kbps"));
    nc_meshNodes.Get (6)->AddApplication (app1);
    app1->SetStartTime (Seconds (15.));
    app1->SetStopTime (Seconds (20.));

if(IsMalicious==false)
{
    uint16_t sinkPort4 = 2;
    Address sinkAddress4 (InetSocketAddress (staAp_interfaces2.GetAddress (1), sinkPort4)); // interface of n3
    PacketSinkHelper packetSinkHelper4("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort4));
    ApplicationContainer sinkApps4 = packetSinkHelper4.Install (nc_meshNodes.Get (3)); //n3 as sink
    sinkApps4.Start (Seconds (20));
    sinkApps4.Stop (Seconds (30.));

    Ptr<Socket> ns3UdpSocket4 = Socket::CreateSocket (nc_sta2.Get (0), UdpSocketFactory::GetTypeId ()); //source at n1

    Ptr<MyApp> app4 = CreateObject<MyApp> ();
    app4->Setup (ns3UdpSocket4, sinkAddress4, m_packetSize, m_NumOfPacket, DataRate ("250Kbps"));
    nc_sta2.Get (0)->AddApplication (app4);
    app4->SetStartTime (Seconds (25.));
    app4->SetStopTime (Seconds (30.));
}

//*******************************************flow monitor*******************************************
    monitor = flowmon.InstallAll();

  AnimationInterface anim("blackholeWMN.xml"); // Mandatory
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (0), 150, 0);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (1), 200, 0);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (2), 200, 300);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (3), 150, 300); 
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (4), 150, 150);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (5), 200, 150);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (6), 250, 0);
  AnimationInterface::SetConstantPosition (nc_meshNodes.Get (7), 250, 150);
   AnimationInterface::SetConstantPosition (nc_meshNodes.Get (8), 250, 300);

   AnimationInterface::SetConstantPosition (nc_sta1.Get (0), 250, -100);
   AnimationInterface::SetConstantPosition (nc_sta2.Get (0), 150, 400);

  anim.UpdateNodeDescription(nc_meshNodes.Get (0),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (0),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (1),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (1),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (2),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (2),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (3),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (3),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (4),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (4),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (5),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (5),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (6),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (6),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (7),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (7),0,255,0);
  anim.UpdateNodeDescription(nc_meshNodes.Get (8),"Node");
  anim.UpdateNodeColor(nc_meshNodes.Get (8),0,255,0);

  anim.UpdateNodeDescription(nc_sta1.Get (0),"Sender");
  anim.UpdateNodeDescription(nc_sta2.Get (0),"Receiver");


  anim.EnablePacketMetadata(true);

      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("blackholeWMN.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (60), routingStream);

//*******************************************Trace Received Packet*******************************************
    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&ReceivePacket));

    Simulator::Stop (Seconds(simulationTime+1));
    Simulator::Run ();

    Simulator::Destroy ();

    return 0;
}

