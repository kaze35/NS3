/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IADD_ROUTING_PROTOCOL_H
#define IADD_ROUTING_PROTOCOL_H

using namespace std;

#include "ns3/ipv4-routing-protocol.h"
#include "NeighborList.h"
#include "GPS.h"
#include "Segment.h"
#include "PendingPackets.h"
#include "DataCache.h"
#include "ns3/netanim-module.h"
#include "constants.h"
#include "ns3/basic-data-calculators.h"
#include "IADDPacketTracer.h"

namespace ns3 {

/* ... */

class IADD: public Ipv4RoutingProtocol {
private:
	IADDProtocol protocol;
	NodeType type; //1 mobile , 2 RSU, 3 Dest
	VehicleMode mode; //1 segment , 2 intersection
	RSU myRSU; // RSU
	RSU RSUnext; // next RSU the node is heading to
	Ptr<GPS> gps;
	list<Ptr<Segment> > segmentsList; // segment list for RSU
	Ptr<Ipv4> m_ipv4; 	/// IP protocol
	/// Raw socket per each IP interface, map socket -> iface address (IP + mask)
	map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
	Time beaconInterval; // how often do we send beacon msg
	Timer beaconTimer; // to schedule the beacons msgs
	//uint32_t allowedVeaconLoss;   //Number of beacon messages which may be loss
	PendingPackets packetsList; //list of packets carried
	DataCache datacache; // cache to hold data in RSU
	NeighborList neighborList;

	map<IADDInterestTypes, uint32_t> popularity;

	Ptr<Socket> IDSocket; // Socket to send data and interest packets

	AnimationInterface * anim;
	bool enableAnime;

	Vector prevVelocity; // used to determine the junction crossing ( turn right/left)
	Vector prevPosition; // used to determine junction crossing ( pass through)

	uint32_t interestCount_lastHour;
	uint32_t interestCount_currentHour;

	Timer centralityTimer;

	Ptr<MinMaxAvgTotalCalculator<double> > hitRatio;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedCache;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedVehicle;

	Ptr<CounterCalculator<> > numberofPacketsReplicated;
	Ptr<CounterCalculator<> > numberofReplicasRecieved;
	uint32_t stopCounter;

	bool vehicleStopped;

	Vector myPosition();
//	// DI : ------------------------------------------
//	// The number of bytes to send in this simulation.
//	static const uint32_t totalTxBytes = 2000000;
//	static uint32_t currentTxBytes = 0;
//   	// Perform series of 1040 byte writes (this is a multiple of 26 since
//	// we want to detect data splicing in the output stream)
//	static const uint32_t writeSize = 1040;
//	uint8_t data[writeSize];
//
//	void WriteUntilBufferFull (Ptr<Socket>, uint32_t);
//	//------------------------------------------

	void resetCentrality();
	bool isCourseChanged(Vector oldV, Vector newV);
	bool hasPassedThrough(Vector oldP, Vector newP, Vector rsuP);
	void prepareRouteandSend(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, Ipv4Address gateway);

	double calculateAngle(Vector oldP, Vector newP);
	double calculateAngle(Vector velocity);
	void beaconIntervalExpired();
	void sendBeacon();
	void SendTo(Ptr<Socket> socket, Ptr<Packet> packet,
			Ipv4Address destination);
	void SendTo(Ptr<Packet> packet, Ipv4Address destination, uint32_t port);
	void RecvIadd(Ptr<Socket> socket);
	Ptr<Socket> FindSocketWithInterfaceAddress(Ipv4InterfaceAddress) const;
	Ptr<Ipv4Route> LoopbackRoute(const Ipv4Header & hdr,
			Ptr<NetDevice> oif) const;
	bool RSUDataForwarding(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, ErrorCallback ecb, bool inPacketList,
			bool&pcaketfwd);
	bool VehicleDataForwarding(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, ErrorCallback ecb, bool inPacketList,
			bool& packetfwd);
	bool SCFVehicleForwarding(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, ErrorCallback ecb, bool inPacketList,
			bool& packetfwd);
	bool RSUInterestForwarding(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, ErrorCallback ecb, bool inPacketList,
			bool&pcaketfwd);
	bool VehicleInterestForwarding(Ptr<const Packet> p,
			const Ipv4Header & header, UnicastForwardCallback ucb,
			ErrorCallback ecb, bool inPacketList, bool& packetfwd);
	bool DestInterestForwarding(Ptr<const Packet> p, const Ipv4Header & header,
			UnicastForwardCallback ucb, ErrorCallback ecb, bool inPacketList,
			bool& packetfwd);
	void RecvVehicleBeacon(Ptr<Packet> p, Ipv4Address src);
	void RecvRSUBeacon(Ptr<Packet> p, Ipv4Address src);
	void updateDensities(Vector pos, Ipv4Address rsuNext);
	void ForwardPendingPackets();
	void ForwardAllPackets();
	bool ForwardTotheFurthestNode(Ptr<const Packet> p,
			const Ipv4Header & header, UnicastForwardCallback ucb,
			ErrorCallback ecb, bool fromPacketList, bool &packetFwd);
	list<Ptr<Segment> > createAndSortSegmentList(Vector dst);
	bool RouteInputVehicle(Ptr<const Packet> p, const Ipv4Header &header,
			Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
			MulticastForwardCallback mcb, LocalDeliverCallback lcb,
			ErrorCallback ecb);
	bool RouteInputRSU(Ptr<const Packet> p, const Ipv4Header &header,
			Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
			MulticastForwardCallback mcb, LocalDeliverCallback lcb,
			ErrorCallback ecb);
	bool RouteInputDest(Ptr<const Packet> p, const Ipv4Header &header,
			Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
			MulticastForwardCallback mcb, LocalDeliverCallback lcb,
			ErrorCallback ecb);
	MessageType getPacketType(Ptr<const Packet> p);
	bool isFwdFlagOn(Ptr<const Packet> p);
	bool isCacheFlagOn(Ptr<const Packet> p);

	double distance(Vector v1, Vector v2);
	Vector myVelocity();
	void setPacketFlags(Ptr<Packet> p, bool fwdFlag, bool cacheFlag,
			bool isUDP);
	void setNewDestLoc(Ptr<Packet> p, double, double);
	Ipv4Address getIpAddress();
	IADDInterestTypes getInterestType(uint32_t i);
	uint32_t getRSUCentrality();
	bool isExpiredInterest(IADDInterestHeader i);
	void updateRCSmaxinPacket(Ptr<Packet> p, uint32_t maxCentrality,
			Ipv4Address RCSmaxIP, Vector RCSLoc);
	void updateRCS2maxinPacket(Ptr<Packet> p, uint32_t max2Centrality,
			Ipv4Address RCSmax2IP);
	uint32_t getPopularity(IADDInterestTypes t);

	void changeLastHopIp(Ptr<Packet> p, Ipv4Address myAdd, bool isUDP);
	Ipv4Address getLastHopIp(Ptr<const Packet> p);
	bool isCachedBefore(Ptr<const Packet> p);
	void setCachedBefore(Ptr<Packet> p, bool isUDP);

	string getLogPrefix();
	IADDInterestHeader getInterestHeaderNoUDP(Ptr<const Packet> p);

	Ptr<Packet> createNewPacketFrom(Ptr<const Packet> p);
	string getPacetInfo(Ptr<const Packet> p);

public:
	IADD();
	virtual ~IADD();
	virtual void DoDispose();
	static TypeId GetTypeId(void);
	void start();

	Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
			Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

	bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
			Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
			MulticastForwardCallback mcb, LocalDeliverCallback lcb,
			ErrorCallback ecb);

	virtual void NotifyInterfaceUp(uint32_t interface);
	virtual void NotifyInterfaceDown(uint32_t interface);

	virtual void NotifyAddAddress(uint32_t interface,
			Ipv4InterfaceAddress address);

	virtual void NotifyRemoveAddress(uint32_t interface,
			Ipv4InterfaceAddress address);

	virtual void SetIpv4(Ptr<Ipv4> ipv4);

	virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream) const;

	string getNodeType();
	string getNodeInfo();
	uint32_t getNodeId();
	Ptr<Node> getNode();
	string getMode();
	void setSegmentList(RSU r, list<Ptr<Segment> > segments);

	void setAnime(AnimationInterface* anim);
	void setCalculators(Ptr<MinMaxAvgTotalCalculator<double> > a,
			Ptr<CounterCalculator<> > b, Ptr<CounterCalculator<> > d,
			Ptr<CounterCalculator<> > c,Ptr<CounterCalculator<> > e);

	uint32_t getNumberofDataPackets();
	uint32_t getNumberofIntPackets();
	bool ptrsorter(Ptr<Segment> a, Ptr<Segment> b);
	static IADDInterestHeader getInterestHeader(Ptr<const Packet> p);
	double getHitRatio();
	string getStats();

	uint32_t getCacheSize();
	//vector<uint32_t,uint32_t>packetTrace;
	//void SetPacketTrace(vector<uint32_t,uint32_t> p);
	map<Ipv4Address, uint32_t> getPendingPacketsDes();

	Ptr<IADDPacketTracer> tracer;
	void setTracer(Ptr<IADDPacketTracer> tracer);
	bool tracingEnabled;



};

}

#endif /* IADD_ROUTING_PROTOCOL_H */

