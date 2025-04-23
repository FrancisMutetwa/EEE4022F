#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-helper.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    // ======================
    // 1. Basic Configuration
    // ======================
    Time::SetResolution(Time::NS);
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
    
    // ======================
    // 2. Create Network Nodes
    // ======================
    NodeContainer allNodes;
    allNodes.Create(5);  // 3 anchors + 2 mobile nodes
    
    // ======================
    // 3. Setup WiFi Physical Layer
    // ======================
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    
    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("TxPowerStart", DoubleValue(16.0));
    wifiPhy.Set("TxPowerEnd", DoubleValue(16.0));
    wifiPhy.Set("TxPowerLevels", UintegerValue(1));
    wifiPhy.Set("RxNoiseFigure", DoubleValue(7.0));
    
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());
    
    // ======================
    // 4. Configure MAC Layer
    // ======================
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, allNodes);
    
    // ======================
    // 5. Mobility Configuration
    // ======================
    MobilityHelper mobility;
    
    // Fixed anchor nodes (positions in a triangle formation)
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));    // Anchor 1
    positionAlloc->Add(Vector(100.0, 0.0, 0.0));  // Anchor 2
    positionAlloc->Add(Vector(50.0, 86.6, 0.0));  // Anchor 3
    
    // Mobile nodes (random walk in bounding box)
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds", RectangleValue(Rectangle(-50, 150, -50, 150)));
    
    // Apply mobility: first 3 nodes static, last 2 mobile
    mobility.Install(allNodes.Get(0));
    mobility.Install(allNodes.Get(1));
    mobility.Install(allNodes.Get(2));
    mobility.Install(allNodes.Get(3));
    mobility.Install(allNodes.Get(4));
    
    // ======================
    // 6. Network Stack
    // ======================
    InternetStackHelper stack;
    OlsrHelper olsr;
    stack.SetRoutingHelper(olsr);
    stack.Install(allNodes);
    
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    
    // ======================
    // 7. Traffic Applications
    // ======================
    // UDP Echo Server on Anchor 1
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(20.0));
    
    // UDP Echo Clients on Mobile Nodes
    UdpEchoClientHelper echoClient1(interfaces.GetAddress(0), 9);
    echoClient1.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(0.5)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(512));
    
    ApplicationContainer clientApps1 = echoClient1.Install(allNodes.Get(3));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(18.0));
    
    UdpEchoClientHelper echoClient2(interfaces.GetAddress(0), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(80));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(0.75)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(256));
    
    ApplicationContainer clientApps2 = echoClient2.Install(allNodes.Get(4));
    clientApps2.Start(Seconds(3.0));
    clientApps2.Stop(Seconds(17.0));
    
    // ======================
    // 8. NetAnim Configuration
    // ======================
    AnimationInterface anim("wireless-animation.xml");
    
    // Node colors and labels
    anim.UpdateNodeDescription(allNodes.Get(0), "Anchor 1");
    anim.UpdateNodeDescription(allNodes.Get(1), "Anchor 2");
    anim.UpdateNodeDescription(allNodes.Get(2), "Anchor 3");
    anim.UpdateNodeDescription(allNodes.Get(3), "Mobile 1");
    anim.UpdateNodeDescription(allNodes.Get(4), "Mobile 2");
    
    anim.UpdateNodeColor(allNodes.Get(0), 255, 0, 0);    // Red anchors
    anim.UpdateNodeColor(allNodes.Get(1), 255, 0, 0);
    anim.UpdateNodeColor(allNodes.Get(2), 255, 0, 0);
    anim.UpdateNodeColor(allNodes.Get(3), 0, 0, 255);    // Blue mobiles
    anim.UpdateNodeColor(allNodes.Get(4), 0, 0, 255);
    
    // Set node sizes
    anim.UpdateNodeSize(0, 10, 10);
    anim.UpdateNodeSize(1, 10, 10);
    anim.UpdateNodeSize(2, 10, 10);
    anim.UpdateNodeSize(3, 8, 8);
    anim.UpdateNodeSize(4, 8, 8);
    
    // Background grid (100x100 units)
    anim.SetBackgroundImage("/path/to/grid.png", 0, 0, 0.1, 0.1, 1.0);
    
    // ======================
    // 9. Run Simulation
    // ======================
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}