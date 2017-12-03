/*


 * IADDTest.cc
 *
 *  Created on: Dec 20, 2013
 *      Author: root
 */
#include <ctime>
#include <iomanip>
#include <sstream>
#include <map>
#include "ns3/wifi-module.h"
#include "ns3/wimax-module.h"
#include "ns3/iadd-routing-protocol-module.h"
#include "ns3/iadd-routing-protocol-helper.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/aodv-helper.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"
#include "ns3/iadd-requester-app-helper.h"
#include "ns3/constants.h"
#include "ns3/data-collector.h"
#include "ns3/omnet-data-output.h"
#include "ns3/basic-data-calculators.h"
#include "ns3/time-data-calculators.h"
#include "ns3/IADDPacketTracer.h"
//WAVE
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
using namespace ns3;
using namespace std;
struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct rgb colors[] = { { 255, 0, 0 }, // Red // RSU
		{ 0, 0, 255 }, // blue // dest
		{ 0, 255, 0 },  // green // Vehicle in segment
		{ 0, 0, 0 } // Black // vehicle in intersection
};
class IADDTest {
public:
	IADDTest();
	/// Configure script parameters, \return true on successful configuration
	bool Configure(int argc, char **argv);
	/// Run simulation
	void Run();

private:

	uint32_t numberOfVehicles;
	uint32_t numberOfDest;
	uint32_t numberOfRSU;
	uint32_t gridSize;
	uint32_t duration;
	string traceFile;
	string logFile;
	uint32_t interestPeriod;

	NodeContainer vehicleNodes;
	NodeContainer rsuNodes;
	NodeContainer destNodes;

	NetDeviceContainer vehicleDevices;
	NetDeviceContainer rsuDevices;
	NetDeviceContainer destDevices;

	Ipv4InterfaceContainer interfaces;

	AnimationInterface * anim;

	map<uint32_t, vector<Segment> > RSUneighbors;

	bool pcap;

	uint32_t protocol;

	DataCollector datacol;
	Ptr<MinMaxAvgTotalCalculator<double> > hitRatio;
	Ptr<TimeMinMaxAvgTotalCalculator> elapsedTime;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedCache;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedVehicle;

	Ptr<CounterCalculator<> > numberofPacketsReplicated;
	Ptr<CounterCalculator<> > numberofReplicasRecieved;

	Ptr<CounterCalculator<> > numberofPacketArrived;
	Ptr<CounterCalculator<> > numberofPacketsFromVehicles;
	Ptr<CounterCalculator<> > numberofInterstGenerated;
	Ptr<CounterCalculator<> > numberofMisingData;
	Ptr<CounterCalculator<> > numberofMisingInterest;

	Ptr<MinMaxAvgTotalCalculator<uint32_t> > cacheSize;

	Ptr<IADDPacketTracer> tracer;

	Timer hearbeat;
private:
	void CreateNodes();
	void CreateDevices();
	void InstallInternetStack();
	void InstallApplications();
	void SetMobility();
	void FindRSUNeighbors();
	void CourseChange(std::ostream *os, std::string foo,
			Ptr<const MobilityModel> mobility);
	void printInfo();

	void printResult();
	void ConfigureLogs();

	void setAniminIADD();
	Vector getPosition(Ptr<Node> node);
	Ipv4Address getIpAddress(Ptr<Node> node);

	void setupDataCollector();

	void setupDroppedPacket();

	void getHitRatios();
	uint32_t countTxDropped;
	uint32_t countTxPackets;
	uint32_t countRxDropped;
	uint32_t countRxPackets;
	void CountTxDropped(std::string path, Ptr<const Packet> p);
	void CountTxPackets(std::string path, Ptr<const Packet> p);
	void CountRxDropped(std::string path, Ptr<const Packet> p);
	void CountRxPackets(std::string path, Ptr<const Packet> p);
	void HeartBeat();
	void outputResultsToFile();
	time_t begin;
	time_t end;
};

int main(int argc, char **argv) {
	{
		IADD a1;
		a1.GetTypeId();
	}
	//AodvHelper sodv;
	IADDTest s1;
	//cout << "Simulation Started at: " << begin << endl;

	if (!s1.Configure(argc, argv))
		NS_FATAL_ERROR("Configuration failed. Aborted.");

	s1.Run();

	return 0;
}
IADDTest::IADDTest() :
		numberOfVehicles(0), numberOfDest(0), numberOfRSU(0), gridSize(0), duration(
				0), anim(0), pcap(false), hearbeat(Timer::CANCEL_ON_DESTROY) {
	countTxDropped = 0;
	countRxDropped = 0;
	countTxPackets = 0;
	countRxPackets = 0;
	protocol = CADD;
	interestPeriod = 0;
}

bool IADDTest::Configure(int argc, char** argv) {
	// Parse command line attribute
	CommandLine cmd;
	cmd.AddValue("protocol", "RoutingProtocol", protocol);
	cmd.AddValue("traceFile", "Ns2 movement trace file", traceFile);
	cmd.AddValue("nodeNum", "Number of nodes", numberOfVehicles);
	cmd.AddValue("destinationNum", "Number of destinations", numberOfDest);
	cmd.AddValue("duration", "Duration of Simulation", duration);
	cmd.AddValue("interestPeriod", "Interest generation interval",
			interestPeriod);
	cmd.Parse(argc, argv);

	ns3::RngSeedManager::SetSeed(4); // change the seed from 1 to 3
	ns3::RngSeedManager::SetRun(1);
	// Check command line arguments
	if (traceFile.empty() || numberOfVehicles <= 0 || numberOfDest <= 0
			|| duration <= 0 || interestPeriod < 0 || protocol < 1
			|| protocol > 3) {
		std::cout << "Usage of " << argv[0]
				<< " :\n\n"
						"./waf --run \"ns2-mobility-trace"
						" --traceFile=src/mobility/examples/default.ns_movements"
						" --nodeNum=2 --destinationNum=1 --duration=100.0 ...\" \n\n"
						"NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
						"      included in the same directory of this example file.\n\n"
						"NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
						"        be a positive number. Note that you must know it before to be able to load it.\n\n"
						"NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

		return false;
	}

	return true;
}
void IADDTest::ConfigureLogs() {
	// Enable logging from the ns2 helper
	//LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_ALL);
	LogComponentEnable("IADD", LOG_LEVEL_INFO);
	LogComponentEnable("IADDApplication", LOG_LEVEL_INFO);
//LogComponentEnable("MapScheduler",LOG_LEVEL_INFO);
	//LogComponentEnable ("ArpL3Protocol",LOG_LEVEL_LOGIC);	// LogComponentEnableAll(LOG_LEVEL_DEBUG);
	pcap = false;

	stringstream ss2;
	struct tm *current;
	time_t now = time(0);
	current = localtime(&now);
	ss2 << setfill('0') << setw(2) << current->tm_hour << setfill('0')
			<< setw(2) << current->tm_min << setfill('0') << setw(2)
			<< current->tm_sec << ".log";

	tracer = CreateObject<IADDPacketTracer>(string("LTx") + ss2.str(),
			string("LMa") + ss2.str(), string("LPa") + ss2.str());
	//Packet::EnablePrinting();
	//Packet::EnableChecking();

}
void IADDTest::Run() {
	begin = time(0);

	ConfigureLogs();

	// open log file for output
	//std::ofstream os;
	//os.open(logFile.c_str());
	// Configure callback for logging
	//Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
	//	MakeBoundCallback(&IADDTest::CourseChange, &os));
	//os.close(); // close log file

	CreateNodes();

	SetMobility();
	CreateDevices();
	InstallInternetStack();
	InstallApplications();

	//setAniminIADD();
	if (protocol != SCF)
		FindRSUNeighbors();
	printInfo();
	setupDataCollector();
	setupDroppedPacket();

	std::cout << "Starting simulation for " << duration << " s ...\n";
	Simulator::Stop(Seconds(duration));

	//hearbeat.SetFunction(&IADDTest::HeartBeat, this);
	//hearbeat.Schedule(Seconds(duration/10));

	Simulator::Run();

	end = time(0);
	getHitRatios();
	printResult();

	cout << "Destroying simulation ... \n";
	Simulator::Destroy();
	cout << "Simulation is done ... \n";

	tracer->closeFile();

	outputResultsToFile();
	;

	cout << "Simulation duration: " << end - begin << " seconds";
	delete anim;
	cout << "Bye";
}
// Prints actual position and velocity when a course change event occurs
void IADDTest::CourseChange(std::ostream *os, std::string foo,
		Ptr<const MobilityModel> mobility) {
	Vector pos = mobility->GetPosition(); // Get position
	Vector vel = mobility->GetVelocity(); // Get velocity

	// Prints position and velocities
	*os << Simulator::Now() << " POS: x=" << pos.x << ", y=" << pos.y << ", z="
			<< pos.z << "; VEL:" << vel.x << ", y=" << vel.y << ", z=" << vel.z
			<< std::endl;
}

void IADDTest::CreateNodes() {
	// create an array of empty nodes for testing purposes RSU
	if (protocol != SCF) {
		rsuNodes.Create(16);
		anim->SetNodeColor(rsuNodes, colors[0].r, colors[0].g, colors[0].b);
	}
	//create all destinations

	destNodes.Create(numberOfDest);
	anim->SetNodeColor(destNodes, colors[1].r, colors[1].g, colors[1].b);

	//create Vehicles
	vehicleNodes.Create(numberOfVehicles);
	anim->SetNodeColor(vehicleNodes, colors[2].r, colors[2].g, colors[2].b);

}

void IADDTest::CreateDevices() {
	//NodeContainer nodes(rsuNodes, destNodes, vehicleNodes);
	/*
	 NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	 wifiMac.SetType("ns3::AdhocWifiMac");
	 YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();

	 YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	 wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel"); //, "MaxRange",
	 //DoubleValue(WIFI_MAX_RANGE));
	 wifiPhy.SetChannel(wifiChannel.Create());
	 WifiHelper wifi = WifiHelper::Default();
	 wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
	 StringValue("OfdmRate6Mbps"));//, "RtsCtsThreshold", UintegerValue(0));
	 rsuDevices = wifi.Install(wifiPhy, wifiMac, rsuNodes);
	 destDevices = wifi.Install(wifiPhy, wifiMac, destNodes);
	 vehicleDevices = wifi.Install(wifiPhy, wifiMac, vehicleNodes);


	 */
	//WAVE
	// The below set of helpers will help us to put together the wifi NICs we want
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange",
			DoubleValue(WIFI_MAX_RANGE));
	Ptr<YansWifiChannel> channel = wifiChannel.Create();
	wifiPhy.SetChannel(channel);
	// ns-3 supports generate a pcap trace
	//  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default();
	Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default();

	wifi80211p.SetRemoteStationManager("ns3::ConstantRateWifiManager",
			"DataMode", StringValue("OfdmRate6MbpsBW10MHz"), "ControlMode",
			StringValue("OfdmRate6MbpsBW10MHz"), "RtsCtsThreshold",
			UintegerValue(0));
	if (protocol != SCF)
		rsuDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, rsuNodes);
	destDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, destNodes);
	vehicleDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, vehicleNodes);

	// wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging

	if (pcap) {
		wifiPhy.EnablePcapAll(std::string("iadd"));
	}
}

void IADDTest::InstallInternetStack() {

	AodvHelper aodv;
	IaddRoutingProtocolHelper iaddV;
	// you can configure IADD attributes here using iadd.Set(name, value)
	iaddV.Set("Type", UintegerValue(IADD_VEHICLE));
	iaddV.Set("Protocol", UintegerValue(protocol));
	InternetStackHelper stack;
	stack.SetRoutingHelper(iaddV); // has effect on the next Install ()
	stack.Install(vehicleNodes);

	IaddRoutingProtocolHelper iaddD;
	// you can configure IADD attributes here using iadd.Set(name, value)
	iaddD.Set("Type", UintegerValue(IADD_DEST));
	iaddD.Set("Protocol", UintegerValue(protocol));
	InternetStackHelper stackD;
	stackD.SetRoutingHelper(iaddD); // has effect on the next Install ()
	stackD.Install(destNodes);

	if (protocol != SCF) {
		IaddRoutingProtocolHelper iaddR;
		// you can configure IADD attributes here using iadd.Set(name, value)
		iaddR.Set("Type", UintegerValue(IADD_RSU));
		iaddR.Set("Protocol", UintegerValue(CADD));
		InternetStackHelper stackR;
		stackR.SetRoutingHelper(iaddR); // has effect on the next Install ()
		stackR.Install(rsuNodes);
	}
	Ipv4AddressHelper address;
	address.SetBase("192.168.0.0", "255.255.0.0");
	if (protocol != SCF) {
		NetDeviceContainer devicesT(rsuDevices, destDevices);
		NetDeviceContainer devices(devicesT, vehicleDevices);
		interfaces = address.Assign(devices);
	} else {
		NetDeviceContainer devices(destDevices, vehicleDevices);
		interfaces = address.Assign(devices);
	}
	Config::Set("/NodeList/*/$ns3::ArpL3Protocol/CacheList/*/PendingQueueSize",
			UintegerValue(20));
	Config::Set("/NodeList/*/$ns3::ArpL3Protocol/CacheList/*/AliveTimeout",
			TimeValue(Seconds(500)));
	/*

	 Ipv4AddressHelper address2;
	 address2.SetBase("192.168.1.0", "255.255.255.0");
	 interfaces = address2.Assign(rsuDevices);

	 Ipv4AddressHelper address3;
	 address3.SetBase("192.168.1.0", "255.255.255.0");
	 interfaces = address3.Assign(destDevices);*/
}

void IADDTest::InstallApplications() {
	IADDApplicationHelper helper;
	helper.SetAttribute("Interval", TimeValue(Seconds(interestPeriod)));
	helper.SetAttribute("interestGenerationStart", TimeValue(Seconds(2)));
	helper.SetAttribute("interestGenerationEnd",
			TimeValue(Seconds(duration - 500)));

	ApplicationContainer nodeApplication = helper.Install(destNodes);

	nodeApplication.Start(Seconds(0));
	nodeApplication.Stop(Seconds(duration - 0.0001));
}

void IADDTest::SetMobility() {
	if (protocol != SCF) {
		//********** RSU Mobility **********
		MobilityHelper rsuMobility;
		// setup the grid itself: objects are layed out
		// started from (-100,-100) with 20 objects per row,
		// the x interval between each object is 5 meters
		// and the y interval between each object is 20 meters
		rsuMobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX",
				DoubleValue(0.0), "MinY", DoubleValue(0.0), "DeltaX",
				DoubleValue(400.0), "DeltaY", DoubleValue(400.0), "GridWidth",
				UintegerValue(4), "LayoutType", StringValue("RowFirst"));

		// each object will be attached a static position.
		// i.e., once set by the "position allocator", the
		// position will never change.
		rsuMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

		// finalize the setup by attaching to each object
		// in the input array a position and initializing
		// this position with the calculated coordinates.
		rsuMobility.Install(rsuNodes);
	}
	//********** Destination Mobility **********
	MobilityHelper destinationMobility;

	destinationMobility.SetPositionAllocator(
			"ns3::RandomRectanglePositionAllocator", "X",
			StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"), "Y",
			StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"));

	destinationMobility.SetPositionAllocator("ns3::GridPositionAllocator",
			"MinX", DoubleValue(0.0), "MinY", DoubleValue(0.0), "DeltaX",
			DoubleValue(900.0), "DeltaY", DoubleValue(900.0), "GridWidth",
			UintegerValue(2), "LayoutType", StringValue("RowFirst"));

	destinationMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	destinationMobility.Install(destNodes);

	//********** Vehicles Mobility **********

	// Create Ns2MobilityHelper with the specified trace log file as parameter
	Ns2MobilityHelper vehicleMobility = Ns2MobilityHelper(traceFile);
	vehicleMobility.Install(vehicleNodes.Begin(), vehicleNodes.End()); // configure movements for each node, while reading trace file
}

void IADDTest::printInfo() {
	cout << "****************************" << endl;

	cout << "Vehicles:" << endl;
	for (uint32_t i = 0; i < vehicleNodes.GetN(); i++) {
		cout << "Node ID:" << vehicleNodes.Get(i)->GetId() << ", IP: "
				<< vehicleNodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()
				<< ", MAC: " << vehicleNodes.Get(i)->GetDevice(0)->GetAddress()
				<< ", Position: (" << getPosition(vehicleNodes.Get(i)).x << " "
				<< getPosition(vehicleNodes.Get(i)).y << ")" << endl;

	}
	if (protocol != SCF) {

		cout << "****************************" << endl;
		cout << "RSU:" << endl;
		for (uint32_t i = 0; i < rsuNodes.GetN(); i++) {
			cout << "Node ID:" << rsuNodes.Get(i)->GetId() << ", IP: "
					<< rsuNodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()
					<< ", MAC: " << rsuNodes.Get(i)->GetDevice(0)->GetAddress()
					<< ", Position: (" << getPosition(rsuNodes.Get(i)).x << " "
					<< getPosition(rsuNodes.Get(i)).y << ")" << endl;
		}
		cout << "****************************" << endl;
	}
	cout << "Destination nodes:" << endl;
	for (uint32_t i = 0; i < destNodes.GetN(); i++) {
		cout << "Node ID:" << destNodes.Get(i)->GetId() << ", IP: "
				<< destNodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()
				<< ", MAC: " << destNodes.Get(i)->GetDevice(0)->GetAddress()
				<< ", Position: (" << getPosition(destNodes.Get(i)).x << " "
				<< getPosition(destNodes.Get(i)).y << ")" << endl;
	}

	cout << "****************************" << endl;



}

void IADDTest::printResult() {
	cout << "Total Packets tx " << countTxPackets << endl;
	cout << "Total Packets dropped tx " << countTxDropped << endl;
	cout << "Total Packets Rx " << countRxPackets << endl;
	cout << "Total Packets dropped Rx " << countRxDropped << endl;
	NodeContainer nodes;

	if (protocol != SCF) {
		nodes.Add(rsuNodes);
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);

	} else {
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);
	}

	for (uint32_t i = 0; i < nodes.GetN(); i++) {
		Ptr<Node> n = nodes.Get(i);
		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		cout << iadd->getStats() << endl;
	}
	for (uint32_t i = 0; i < nodes.GetN(); i++) {
		Ptr<Node> n = nodes.Get(i);
		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		//cout << iadd->getStats() << endl;
		if (iadd->getNumberofDataPackets() > 0
				|| iadd->getNumberofIntPackets() > 0) {
			cout << "Node " << i << endl;
			map<Ipv4Address, uint32_t> dest = iadd->getPendingPacketsDes();
			for (map<Ipv4Address, uint32_t>::iterator j = dest.begin();
					j != dest.end(); ++j) {
				cout << j->first << " --> " << j->second << endl;
			}
			cout << "******************" << endl;

		}

	}
	/*
	 numberofInterstGenerated->
	 uint32_t totalGenerated = numberofPacketsGeneratedCache->m_count
	 + numberofPacketsGeneratedVehicle->m_count;
	 uint32_t receivedCache = numberofPacketArrived->m_count
	 - numberofPacketsFromVehicles->m_count;
	 uint32_t datadropped = numberofPacketsGeneratedVehicle->m_count
	 - numberofPacketsFromVehicles->m_count
	 - numberofMisingData->m_count;
	 uint32_t intDropped = totalGenerated - numberofPacketArrived->m_count
	 - numberofMisingInterest->m_count;

	 cout << elapsedTime->m_total / elapsedTime->m_count << ","
	 << numberofInterstGenerated->m_count << ","
	 << numberofPacketsGeneratedCache->m_count << ","
	 << numberofPacketsGeneratedVehicle->m_count << "," << totalGenerated
	 << "," << receivedCache << ","
	 << numberofPacketsFromVehicles->m_count << ","
	 << numberofPacketArrived->m_count << ","
	 << numberofMisingData->m_count << ","
	 << numberofMisingInterest->m_count << ","
	 << numberofPacketsGeneratedCache->m_count - (receivedCache) << ","
	 << numberofPacketsGeneratedVehicle->m_count
	 - numberofPacketsFromVehicles->m_count << ","
	 << totalGenerated - numberofPacketArrived->m_count << ","
	 << datadropped << "," << intDropped << ","
	 << numberofPacketsFromVehicles->m_count
	 / (double) numberofPacketArrived->m_count << ","
	 << numberofPacketsGeneratedVehicle->m_count
	 / (double) totalGenerated << begin - end << endl;
	 */
	/*for (uint32_t i = 0; i < destNodes.GetN(); i++) {

	 cout
	 << destNodes.Get(i)->GetApplication(0)->GetObject<
	 IADDApplication>()->getAvgTime();
	 }
	 */
	stringstream ss2;
	struct tm *current;
	time_t now = time(0);
	current = localtime(&now);
	ss2 << string("Packet_status")<<setfill('0') << setw(2) << current->tm_hour << setfill('0')
			<< setw(2) << current->tm_min << setfill('0') << setw(2)
			<< current->tm_sec << ".log";
	fstream fout2;
	fout2.open(ss2.str().c_str(), fstream::out);
	for (map<uint64_t, string>::iterator i = tracer->packetStatus.begin();
			i != tracer->packetStatus.end(); i++) {
		fout2 << i->first << "," << i->second << endl;
	}
	fout2.close();
}

void IADDTest::FindRSUNeighbors() {

	for (uint32_t i = 0; i < rsuNodes.GetN(); i++) {
		list<Ptr<Segment> > s;
		Ptr<Node> n = rsuNodes.Get(i);
		uint32_t id = n->GetId();
		Vector myPos = getPosition(n);
		Ipv4Address myIp = getIpAddress(n);

		RSU myRSU(id, myIp, myPos);

		if ((id + 4) < 16) { // add to north
			//cout << "1: "<<((id + 4)) << endl;
			Ptr<Node> nextNode = rsuNodes.Get(id + 4);
			uint32_t nid = n->GetId();
			Vector nPos = getPosition(nextNode);
			Ipv4Address nIp = getIpAddress(nextNode);
			RSU r(nid, nIp, nPos);
			Ptr<Segment> s1 = CreateObject<Segment>(r, 0.0, 90);
			Ptr<Segment> s2 = CreateObject<Segment>(myRSU, 0, 90);
			s.push_back(s1);
			s.push_back(s2);
		}
		if ((id + 1) % 4 != 0) { //add to east
			//cout << "2: " << ((id + 1) % 4) << endl;
			Ptr<Node> nextNode = rsuNodes.Get(id + 1);
			uint32_t nid = n->GetId();
			Vector nPos = getPosition(nextNode);
			Ipv4Address nIp = getIpAddress(nextNode);
			RSU r(nid, nIp, nPos);
			Ptr<Segment> s1 = CreateObject<Segment>(r, 0, 0);
			Ptr<Segment> s2 = CreateObject<Segment>(myRSU, 0, 0);
			s.push_back(s1);
			s.push_back(s2);
		}
		if ((int) (id - 4) >= 0) { //add to south
			Ptr<Node> nextNode = rsuNodes.Get(id - 4);
			uint32_t nid = n->GetId();
			Vector nPos = getPosition(nextNode);
			Ipv4Address nIp = getIpAddress(nextNode);
			RSU r(nid, nIp, nPos);
			Ptr<Segment> s1 = CreateObject<Segment>(r, 0, 270);
			Ptr<Segment> s2 = CreateObject<Segment>(myRSU, 0, 270);
			s.push_back(s1);
			s.push_back(s2);
		}
		if ((id % 4 != 0)) { //add to west�
			Ptr<Node> nextNode = rsuNodes.Get(id - 1);
			uint32_t nid = n->GetId();
			Vector nPos = getPosition(nextNode);
			Ipv4Address nIp = getIpAddress(nextNode);
			RSU r(nid, nIp, nPos);
			Ptr<Segment> s1 = CreateObject<Segment>(r, 0, 180);
			Ptr<Segment> s2 = CreateObject<Segment>(myRSU, 0, 180);
			s.push_back(s1);
			s.push_back(s2);
		}

		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		iadd->setSegmentList(myRSU, s);

	}
	/*
	 for (map<uint32_t, vector<Segment> >::iterator iter = RSUneighbors.begin();
	 iter != RSUneighbors.end(); iter++) {
	 cout << iter->first << endl;
	 for (uint32_t i = 0; i < iter->second.size(); i++) {
	 cout << iter->second[i].rsuNext.getNode()->GetId() << " - ";
	 }
	 cout << endl;
	 }*/

}
Vector IADDTest::getPosition(Ptr<Node> node) {
	Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
	return mobility->GetPosition();
}

Ipv4Address IADDTest::getIpAddress(Ptr<Node> node) {
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
	Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
	return iaddr.GetLocal();
}

void IADDTest::setAniminIADD() {
	anim = new AnimationInterface("animation.xml");
	anim->EnablePacketMetadata(false);
	if (protocol != SCF) {
		for (uint32_t i = 0; i < rsuNodes.GetN(); i++) {
			Ptr<Node> n = rsuNodes.Get(i);

			Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
			Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
			Ptr<IADD> iadd = DynamicCast<IADD>(proto);
			iadd->setAnime(anim);
		}
	}
	for (uint32_t i = 0; i < vehicleNodes.GetN(); i++) {
		Ptr<Node> n = vehicleNodes.Get(i);

		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		iadd->setAnime(anim);
	}
	for (uint32_t i = 0; i < destNodes.GetN(); i++) {
		Ptr<Node> n = destNodes.Get(i);

		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		iadd->setAnime(anim);
	}
}

void IADDTest::setupDataCollector() {
	stringstream ss;

	time_t now;
	struct tm *current;
	now = time(0);
	current = localtime(&now);
	ss << numberOfVehicles << "-" << duration << "-" << interestPeriod << "-"
			<< setfill('0') << setw(2) << current->tm_hour << ":"
			<< setfill('0') << setw(2) << current->tm_min << ":" << setfill('0')
			<< setw(2) << current->tm_sec;
	stringstream ss2;
	ss2 << current->tm_mday << "-" << current->tm_mon << " " << current->tm_hour
			<< ":" << current->tm_min << ":" << current->tm_sec;

	datacol.AddMetadata("duration", duration);
	datacol.AddMetadata("interestPeriod", interestPeriod);
	datacol.AddMetadata("VehicleNum", numberOfVehicles);
	datacol.AddMetadata("DestNum", numberOfDest);
	datacol.AddMetadata("MobilityFile", traceFile);
	datacol.AddMetadata("timeStarted", ss2.str());
	datacol.AddMetadata("TransmitionRange", (uint32_t) WIFI_MAX_RANGE);
	datacol.AddMetadata("CacheSize", (uint32_t) CACHE_SIZE);

	if (protocol == SCF)
		datacol.DescribeRun("SCF RP", ss.str(), traceFile, ss.str(),
				"Testing Protocol: SCF");
	else
		datacol.DescribeRun("CADD RP", ss.str(), traceFile, ss.str(),
				"Testing Protocol: CADD");

	numberofInterstGenerated = CreateObject<CounterCalculator<> >();
	numberofInterstGenerated->SetKey("Total-number-of-interests");
	datacol.AddDataCalculator(numberofInterstGenerated);

	numberofPacketsGeneratedCache = CreateObject<CounterCalculator<> >();
	numberofPacketsGeneratedCache->SetKey("#Packets_generated_cache");
	datacol.AddDataCalculator(numberofPacketsGeneratedCache);

	numberofPacketsGeneratedVehicle = CreateObject<CounterCalculator<> >();
	numberofPacketsGeneratedVehicle->SetKey("#Packets_generated_vehicle");
	datacol.AddDataCalculator(numberofPacketsGeneratedVehicle);

	numberofPacketsReplicated = CreateObject<CounterCalculator<> >();
	numberofPacketsReplicated->SetKey("#Packets_Replicated");
	datacol.AddDataCalculator(numberofPacketsReplicated);

	numberofReplicasRecieved = CreateObject<CounterCalculator<> >();
	numberofReplicasRecieved->SetKey("#Packets_Replicated_received");
	datacol.AddDataCalculator(numberofReplicasRecieved);

	numberofPacketArrived = CreateObject<CounterCalculator<> >();
	numberofPacketArrived->SetKey("Total-data-received");
	datacol.AddDataCalculator(numberofPacketArrived);

	numberofPacketsFromVehicles = CreateObject<CounterCalculator<> >();
	numberofPacketsFromVehicles->SetKey("Total-data-received-vehicle");
	datacol.AddDataCalculator(numberofPacketsFromVehicles);

	numberofMisingData = CreateObject<CounterCalculator<> >();
	numberofMisingData->SetKey("Total-data-missing");
	datacol.AddDataCalculator(numberofMisingData);

	numberofMisingInterest = CreateObject<CounterCalculator<> >();
	numberofMisingInterest->SetKey("Total-interest-missing");
	datacol.AddDataCalculator(numberofMisingInterest);

	NodeContainer nodes;

	if (protocol != SCF) {
		nodes.Add(rsuNodes);
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);

	} else {
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);
	}

	for (uint32_t i = 0; i < nodes.GetN(); i++) {
		Ptr<Node> n = nodes.Get(i);
		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		iadd->setCalculators(hitRatio, numberofPacketsGeneratedCache,
				numberofPacketsGeneratedVehicle, numberofPacketsReplicated,
				numberofReplicasRecieved);
		iadd->setTracer(tracer);
	}

	elapsedTime = CreateObject<TimeMinMaxAvgTotalCalculator>();
	elapsedTime->SetKey("Elapsed-Time");
	datacol.AddDataCalculator(elapsedTime);

	hitRatio = CreateObject<MinMaxAvgTotalCalculator<double> >();
	hitRatio->SetKey("hit-ratio");
	datacol.AddDataCalculator(hitRatio);

	cacheSize = CreateObject<MinMaxAvgTotalCalculator<uint32_t> >();
	cacheSize->SetKey("cache-Size");
	datacol.AddDataCalculator(cacheSize);

	for (uint32_t i = 0; i < destNodes.GetN(); i++) {
		Ptr<Node> n = destNodes.Get(i);
		Ptr<IADDApplication> app = DynamicCast<IADDApplication>(
				n->GetApplication(0));
		app->setCalculators(elapsedTime, numberofPacketArrived,
				numberofPacketsFromVehicles, numberofInterstGenerated);
		app->setTracer(tracer);

	}

}

void IADDTest::setupDroppedPacket() {
	Config::Connect(
			"/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop",
			MakeCallback(&IADDTest::CountTxDropped, this));
	Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
			MakeCallback(&IADDTest::CountTxPackets, this));
	Config::Connect(
			"/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop",
			MakeCallback(&IADDTest::CountRxDropped, this));
	Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
			MakeCallback(&IADDTest::CountRxPackets, this));

}
void IADDTest::CountTxDropped(std::string path, Ptr<const Packet> p) {
	//if (p->GetUid() == 20682)
	//	cout<<"ttttttt";
	countTxDropped++;
}
void IADDTest::CountTxPackets(std::string path, Ptr<const Packet> p) {
	countTxPackets++;
}
void IADDTest::CountRxDropped(std::string path, Ptr<const Packet> p) {
//	if (p->GetUid() == 20682 && Simulator::Now().GetSeconds() >350)
	//cout<<"ttttttt";
	countRxDropped++;
}
void IADDTest::CountRxPackets(std::string path, Ptr<const Packet> p) {
	countRxPackets++;
}

void IADDTest::getHitRatios() {
	if (protocol != SCF)
		for (uint32_t i = 0; i < rsuNodes.GetN(); i++) {
			Ptr<Node> n = rsuNodes.Get(i);
			Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
			Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
			Ptr<IADD> iadd = DynamicCast<IADD>(proto);
			double h = iadd->getHitRatio();
			uint32_t cs = iadd->getCacheSize();
			hitRatio->Update(h);
			cacheSize->Update(cs);
		}
	NodeContainer nodes;

	if (protocol != SCF) {
		nodes.Add(rsuNodes);
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);

	} else {
		nodes.Add(destNodes);
		nodes.Add(vehicleNodes);
	}

	for (uint32_t i = 0; i < nodes.GetN(); i++) {
		Ptr<Node> n = nodes.Get(i);
		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
		Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
		Ptr<IADD> iadd = DynamicCast<IADD>(proto);
		numberofMisingData->Update(iadd->getNumberofDataPackets());
		numberofMisingInterest->Update(iadd->getNumberofIntPackets());

	}
}

void IADDTest::HeartBeat() {
	cout << "I'm alive at " << Simulator::Now().GetSeconds() << endl;
	hearbeat.Cancel();

	if (!hearbeat.IsRunning())
		hearbeat.Schedule(Seconds(duration / 10) + Simulator::Now());

}
void IADDTest::outputResultsToFile() {
	Ptr<DataOutputInterface> output;
	output = CreateObject<OmnetDataOutput>();
	stringstream ss;
	//traceFile.substr(0, traceFile.length() - 4);
	if (protocol == CADD)
		ss << "CADD-";
	else if (protocol == SCF)
		ss << "SCF-";

	stringstream ss2;
	struct tm *current;
	time_t now = time(0);
	current = localtime(&now);
	ss2 << current->tm_mday << "-" << current->tm_mon << " " << current->tm_hour
			<< ":" << current->tm_min << ":" << current->tm_sec;

	datacol.AddMetadata("timeFinished", ss2.str());

	output->SetFilePrefix(ss.str());
	output->Output(datacol);

}
