/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4 ) { std::clog <<getLogPrefix(); }
#define _USE_MATH_DEFINES
#include "IADDTypeHeader.h"
#include "ns3/mobility-module.h"
#include "iadd-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>
#include "ns3/iadd-requester-app.h"
#include <math.h>
#include <iomanip>
// TODODone IADD: Add parameters to typeID + read from file
// TODODone IADD: time in interests and data
// TODODone IADD: create function to fetch type and information
// TODODone IADD: create queue for interests in dest
// TODODone IADD: test switching modes on all directions
// TODODone IADD: send copy to RSU

//TODO IADD: Real Map: 1- change the heading functions ( use segments)
//TODO IADD: Real Map: 2- switch to IntersectionMode
//TODO IADD: Real Map: 3- Get the RSUs positions
//TODO IADD: Real Map: 4- Add segments in packets
//TODO IADD: Real Map: 5- GPS functions
//TODO IADD: Real Map: 6- Interact with OSM
//TODO IADD: Real Map: 7- Update densities

using namespace std;
//NS_LOG_COMPONENT_DEFINE ("IADDRoutingProtocol");

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("IADD");
//const uint32_t IADD_PORT = 777; // control messages port  s

IADD::IADD() :
		protocol(CADD), type(IADD_VEHICLE), mode(SEGMENT_MODE), beaconInterval(
				MilliSeconds(BEACON_INTERVAL)), beaconTimer(
				Timer::CANCEL_ON_DESTROY), neighborList(beaconInterval), anim(
				0), enableAnime(false), interestCount_lastHour(0), interestCount_currentHour(
				0), centralityTimer(Timer::CANCEL_ON_DESTROY) {
	NS_LOG_FUNCTION(this << " Constructor");
	vehicleStopped = false;
	stopCounter = 0;
	tracingEnabled = true;
}
TypeId IADD::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::IADD").SetParent<Ipv4RoutingProtocol>().AddConstructor<
					IADD>().AddAttribute("Protocol",
					"Routing Protocol to be used: IC-IADD, IADD or SCF",
					UintegerValue(CADD), MakeUintegerAccessor(&IADD::protocol),
					MakeUintegerChecker<uint8_t>()).AddAttribute("Type",
					"Whether the node is vehicle, RSU, or destination",
					UintegerValue(IADD_VEHICLE),
					MakeUintegerAccessor(&IADD::type),
					MakeUintegerChecker<uint8_t>());
	return tid;
}
//TODOIADD:Done Now3: create segment list
void IADD::start() {

	neighborList.start();
	beaconTimer.SetFunction(&IADD::beaconIntervalExpired, this);
	beaconTimer.Schedule(MilliSeconds(UniformVariable().GetInteger(0, 100)));

	centralityTimer.SetFunction(&IADD::resetCentrality, this);
	centralityTimer.Schedule(Seconds(IADD_CENTRALITY_TIMEOUT));

	for (uint32_t i = 0; i < TYPES_COUNT; i++) {
		popularity[getInterestType(i)] = 0;
	}
	gps = Create<GPS>();
}
IADD::~IADD() {
	//if (type == IADD_RSU)
	//	hitRatio->Update(datacache.getHitRatio());
}
IADDInterestTypes IADD::getInterestType(uint32_t i) {
	switch (i) {
	case 0:
		return TYPE1;
	case 1:
		return TYPE2;
	case 2:
		return TYPE3;
	case 3:
		return TYPE4;
	case 4:
		return TYPE5;
	}
	return TYPES_COUNT;
}

void IADD::DoDispose() {
	m_ipv4 = 0;
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter =
			m_socketAddresses.begin(); iter != m_socketAddresses.end();
			iter++) {
		iter->first->Close();

	}
	m_socketAddresses.clear();
	Ipv4RoutingProtocol::DoDispose();
}
void IADD::NotifyInterfaceUp(uint32_t i) {
	NS_LOG_FUNCTION(this << m_ipv4->GetAddress (i, 0).GetLocal ());
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
	if (l3->GetNAddresses(i) > 1) {
		NS_LOG_WARN(
				"IADD does not work with more then one address per each interface.");
	}
	Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
	if (iface.GetLocal() == Ipv4Address("127.0.0.1"))
		return;

	// Create a socket to listen only on this interface
	Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
			UdpSocketFactory::GetTypeId());
	NS_ASSERT(socket != 0);
	socket->SetRecvCallback(MakeCallback(&IADD::RecvIadd, this));
	socket->BindToNetDevice(l3->GetNetDevice(i));
	socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), IADD_PORT));
	socket->SetAllowBroadcast(true);
	socket->SetAttribute("IpTtl", UintegerValue(1));
	m_socketAddresses.insert(std::make_pair(socket, iface));
	// Hesham: no routing table
	// Add local broadcast record to the routing table
	//Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
	//RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*know seqno=*/ true, /*seqno=*/ 0, /*iface=*/ iface,
	//                                  /*hops=*/ 1, /*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
	//m_routingTable.AddRoute (rt);

	// Allow neighbor manager use this interface for layer 2 feedback if possible
	//Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
	//if (wifi == 0)
	// return;
	//Ptr<WifiMac> mac = wifi->GetMac ();
	//if (mac == 0)
	// return;

	//mac->TraceConnectWithoutContext ("TxErrHeader", m_nb.GetTxErrorCallback ());
	//m_nb.AddArpCache (l3->GetInterface (i)->GetArpCache ());

}

void IADD::NotifyInterfaceDown(uint32_t i) {
	NS_LOG_FUNCTION(this << m_ipv4->GetAddress (i, 0).GetLocal ());
	/*
	 // Disable layer 2 link state monitoring (if possible)
	 Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
	 Ptr<NetDevice> dev = l3->GetNetDevice (i);
	 Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
	 if (wifi != 0)
	 {
	 Ptr<WifiMac> mac = wifi->GetMac ()->GetObject<AdhocWifiMac> ();
	 if (mac != 0)
	 {
	 mac->TraceDisconnectWithoutContext ("TxErrHeader",
	 m_nb.GetTxErrorCallback ());
	 m_nb.DelArpCache (l3->GetInterface (i)->GetArpCache ());
	 }
	 }
	 */
	// Close socket
	Ptr<Socket> socket = FindSocketWithInterfaceAddress(
			m_ipv4->GetAddress(i, 0));
	NS_ASSERT(socket);
	socket->Close();
	m_socketAddresses.erase(socket);
	if (m_socketAddresses.empty()) {
		NS_LOG_LOGIC("No IADD interfaces");

		//Clear neighbor list

		neighborList.removeAll();

		//m_htimer.Cancel ();
		//m_nb.Clear ();
		// m_routingTable.Clear ();
		return;
	}
	//m_routingTable.DeleteAllRoutesFromInterface (m_ipv4->GetAddress (i, 0));
}

void IADD::NotifyAddAddress(uint32_t i, Ipv4InterfaceAddress address) {
	NS_LOG_FUNCTION(this << " interface " << i << " address " << address);
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
	if (!l3->IsUp(i))
		return;
	if (l3->GetNAddresses(i) == 1) {
		Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
		Ptr<Socket> socket = FindSocketWithInterfaceAddress(iface);
		if (!socket) {
			if (iface.GetLocal() == Ipv4Address("127.0.0.1"))
				return;
			// Create a socket to listen only on this interface
			Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
					UdpSocketFactory::GetTypeId());
			NS_ASSERT(socket != 0);
			socket->SetRecvCallback(MakeCallback(&IADD::RecvIadd, this));
			socket->BindToNetDevice(l3->GetNetDevice(i));
			// Bind to any IP address so that broadcasts can be received
			socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), IADD_PORT));
			socket->SetAllowBroadcast(true);
			m_socketAddresses.insert(std::make_pair(socket, iface));

//TODOIADD:Done Now0: add app socket

			// Add local broadcast record to the routing table
//          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (
			//             m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
			//        RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*know seqno=*/ true,
			//                                         /*seqno=*/ 0, /*iface=*/ iface, /*hops=*/ 1,
			//                                        /*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
			//     m_routingTable.AddRoute (rt);
		}
	} else {
		NS_LOG_LOGIC(
				"IADD does not work with more then one address per each interface. Ignore added address");
	}
}

void IADD::updateDensities(Vector pos, Ipv4Address rsuNext) {
	uint32_t direction = gps->getDirection(myRSU.getPosition(), pos);
	for (list<Ptr<Segment> >::iterator i = segmentsList.begin();
			i != segmentsList.end(); ++i) {
		if (abs((double) (*i)->segmentDirection - direction) < 5
				&& rsuNext == (*i)->rsuNext.getAddress()) {
			(*i)->density = (((*i)->density * IADD_DENSITY_TAW) + 1)
					/ IADD_DENSITY_TAW;
		} else {

		}

	}

	//TODOIADD:Done Now2: update density
}
void IADD::ForwardAllPackets() {
	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
			i != packetsList.packets.end(); ++i) {
		bool packetFwd = false;
		ForwardTotheFurthestNode(i->getPacket(), i->getHeader(), i->getUcb(),
				i->getEcb(), true, packetFwd);
		if (packetFwd) {
			packetsList.packets.erase(i);
			return;
		}
	}
}
bool IADD::ForwardTotheFurthestNode(Ptr<const Packet> p,
		const Ipv4Header & header, UnicastForwardCallback ucb,
		ErrorCallback ecb, bool fromPacketList, bool &packetFwd) {
	Ipv4Address nextHop;
	Ptr<RSU> rsu = neighborList.getFurthestNeighborR(myPosition());
	if (rsu) {
		nextHop = rsu->getAddress();

		NS_LOG_INFO(
				"Terminating Mode, Packet "<<p->GetUid()<<" sent to "<<nextHop);
		Ptr<Packet> pac = p->Copy();
		setPacketFlags(pac, 1, 0, true);

		prepareRouteandSend(pac, header, ucb, nextHop);
		packetFwd = true;
		return true;
	} else {
		Vehicle nextV = neighborList.getFurthestNeighborV(myPosition());
		if (nextV.getAddress() == "0.0.0.0"
				|| nextV.getAddress() == "102.102.102.102"
				|| nextV.getRsuNext() == "192.168.254.254")
			return false;
		nextHop = nextV.getAddress();
		//packetFwd = false;
		NS_LOG_INFO(
				"Terminating Mode, Packet "<<p->GetUid()<<" sent to "<<nextHop);
		Ptr<Packet> pac = p->Copy();
		setPacketFlags(pac, 1, 0, true);
		prepareRouteandSend(p, header, ucb, nextHop);
		packetFwd = true;
		return true;

	}

}
void IADD::ForwardPendingPackets() {
//	int x = packetsList.packets.size();
//	cout << x;
//	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
	//	i != packetsList.packets.end(); ++i) {
	//uint32_t x = (uint32_t) i->getHeader().GetTtl();
	//cout << x;
	//}
	bool again = false;
	if (again)
		cout << again;
	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
			i != packetsList.packets.end(); ++i) {
		bool packetFwd = false;
		Ptr<const Packet> p = i->getPacket();
		Ptr<Packet> packet = p->Copy();
		UdpHeader u;
		packet->RemoveHeader(u);

		IADDTypeHeader typeH(IADD_DATA);
		packet->RemoveHeader(typeH);
		if (type == IADD_VEHICLE) {
			//*** Fixing SCF packets dropping error 3rd Sep 2014
			if (protocol == SCF)
				SCFVehicleForwarding(i->getPacket(), i->getHeader(),
						i->getUcb(), i->getEcb(), true, packetFwd);
			//*** 3rd Sep 2014
			else if (typeH.getType() == IADD_DATA)
				VehicleDataForwarding(i->getPacket(), i->getHeader(),
						i->getUcb(), i->getEcb(), true, packetFwd);
			else if (typeH.getType() == IADD_INTEREST)
				VehicleInterestForwarding(i->getPacket(), i->getHeader(),
						i->getUcb(), i->getEcb(), true, packetFwd);
			if (packetFwd) {
				packetsList.packets.erase(i);
				again = true;
				break;
			}
		} else if (type == IADD_RSU) {
			if (typeH.getType() == IADD_DATA)
				RSUDataForwarding(i->getPacket(), i->getHeader(), i->getUcb(),
						i->getEcb(), true, packetFwd);
			else if (typeH.getType() == IADD_INTEREST)
				RSUInterestForwarding(i->getPacket(), i->getHeader(),
						i->getUcb(), i->getEcb(), true, packetFwd);
			if (packetFwd) {
				packetsList.packets.erase(i);
				again = true;

				break;
			}
		} else if (type == IADD_DEST) {
			if (typeH.getType() == IADD_INTEREST)
				DestInterestForwarding(i->getPacket(), i->getHeader(),
						i->getUcb(), i->getEcb(), true, packetFwd);
			if (packetFwd) {
				packetsList.packets.erase(i);
				again = true;

				break;
			}
		}

	}
	//if (again)
	//ForwardPendingPackets();
}
struct ptr_cmp {
	bool operator()(Ptr<Segment> lhs, Ptr<Segment> rhs) {
		return *lhs < *rhs;
	}
};
list<Ptr<Segment> > IADD::createAndSortSegmentList(Vector dst) {
	list<Ptr<Segment> > r;
	double max = 0;
	double disFromRSU = distance(myRSU.getPosition(), dst);
	double maxDen = 0;
	for (list<Ptr<Segment> >::iterator i = segmentsList.begin();
			i != segmentsList.end(); ++i) {
		if ((*i)->rsuNext.getAddress() == myRSU.getAddress())
			continue;
		double d = distance(dst, (*i)->rsuNext.getPosition());
		if (d > disFromRSU)
			continue;

		if (d > max)
			max = d;
		if ((*i)->density > maxDen)
			maxDen = (*i)->density;
		(*i)->priority = d;
		r.push_back(*i);

	}
	//*** fix of 4-Sep priority and density was missed up
	for (list<Ptr<Segment> >::iterator i = r.begin(); i != r.end(); ++i) {
		double p = (*i)->priority;
		(*i)->priority = p / max;
		double d = (*i)->density;
		(*i)->density = (maxDen - d) / maxDen;
	}
	//***

	/*\\
	 for (list<Ptr<Segment> >::iterator i = r.begin(); i != r.end(); ++i) {
	 (*i)->priority = (*i)->priority / max;
	 double p = (*i)->priority;
	 Ipv4Address a =(*i)->rsuNext.getAddress();
	 cout<<p<<a;
	 }
	 //TODODone IADD: exclude bad segments

	 for (list<Ptr<Segment> >::iterator i = r.begin(); i != r.end(); ++i) {
	 double p = (*i)->priority;
	 Ipv4Address a =(*i)->rsuNext.getAddress();
	 cout<<p<<a;
	 }*/
	r.sort(ptr_cmp());
	return r;

}

void IADD::NotifyRemoveAddress(uint32_t i, Ipv4InterfaceAddress address) {
	NS_LOG_FUNCTION(this);
	Ptr<Socket> socket = FindSocketWithInterfaceAddress(address);
	if (socket) {
		//  m_routingTable.DeleteAllRoutesFromInterface (address);
		m_socketAddresses.erase(socket);
		Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
		if (l3->GetNAddresses(i)) {
			Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
			// Create a socket to listen only on this interface
			Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
					UdpSocketFactory::GetTypeId());
			NS_ASSERT(socket != 0);
			socket->SetRecvCallback(MakeCallback(&IADD::RecvIadd, this));
			// Bind to any IP address so that broadcasts can be received
			socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), IADD_PORT));
			socket->SetAllowBroadcast(true);
			m_socketAddresses.insert(std::make_pair(socket, iface));

			// Add local broadcast record to the routing table
//          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
			//         RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*know seqno=*/ true, /*seqno=*/ 0, /*iface=*/ iface,
			//                                         /*hops=*/ 1, /*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
			//     m_routingTable.AddRoute (rt);
		}
		if (m_socketAddresses.empty()) {
			NS_LOG_LOGIC("No IADD interfaces");
			//   m_htimer.Cancel ();
			//  m_nb.Clear ();
			// m_routingTable.Clear ();
			neighborList.removeAll();
			return;
		}
	} else {
		NS_LOG_LOGIC("Remove address not participating in IADD operation");
	}
}
Ptr<Socket> IADD::FindSocketWithInterfaceAddress(
		Ipv4InterfaceAddress addr) const {
	//NS_LOG_FUNCTION (this);
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		if (iface == addr)
			return socket;
	}
	Ptr<Socket> socket;
	return socket;
}

Ptr<Ipv4Route> IADD::RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
		Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) {
	NS_LOG_FUNCTION(
			this<< p->GetUid ()<<header.GetSource()<<header.GetDestination());
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
	//NS_ASSERT(p->GetSize() < 3000);

	Ipv4InterfaceAddress iface = l3->GetAddress(1, 0);
	if (m_socketAddresses.empty()) {
		sockerr = Socket::ERROR_NOROUTETOHOST;
		NS_LOG_LOGIC("No IADD interfaces");
		Ptr<Ipv4Route> route;
		return route;
	}

	if (header.GetDestination().IsBroadcast()
			|| iface.GetBroadcast() == header.GetDestination()) {
		Ptr<Ipv4Route> route = Create<Ipv4Route>();
		route->SetDestination(iface.GetBroadcast());
		route->SetGateway(iface.GetBroadcast());
		route->SetSource(iface.GetLocal());
		route->SetOutputDevice(oif);
		return route;
	}
	NS_LOG_INFO(
				"RouteOutPut");
	if (type == IADD_RSU || type == IADD_VEHICLE) {
		NS_LOG_LOGIC(
				"RSU/Vehicle sending data, send it to Routeinput using loopback");
		NS_LOG_FUNCTION(this << header << (oif ? oif->GetIfIndex () : 0));
		NS_LOG_INFO(
				"RouteOutput: Sending Packet: "<<p->GetUid()<<" to "<<header.GetDestination());

		return LoopbackRoute(header, oif); // send it to RouteInput
	}
	if (type == IADD_DEST) { // send generated interest
		// TODOIADD:Done Now4: send interest from requester
		return LoopbackRoute(header, oif);			// send it to RouteInput

	}

	return Ptr<Ipv4Route>();
}

bool IADD::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) {

	NS_LOG_FUNCTION(
			this << p->GetUid () << header.GetSource() << header.GetDestination());
	if (m_socketAddresses.empty()) {
		NS_LOG_INFO("No IADD interfaces");
		return false;
	}
	NS_ASSERT(m_ipv4 != 0);
	NS_ASSERT(p != 0);
// Check if input device supports IP
	NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);

	Ptr<Packet> packet = p->Copy();

//Check Node type
	if (type == IADD_VEHICLE) { // I'm a vehicle
		return RouteInputVehicle(packet, header, idev, ucb, mcb, lcb, ecb);
	} else if (type == IADD_RSU) { // I'm an RSU
		return RouteInputRSU(packet, header, idev, ucb, mcb, lcb, ecb);
	} else if (type == IADD_DEST) { // I'm a destination
		return RouteInputDest(packet, header, idev, ucb, mcb, lcb, ecb);
	}
	return false;

}
void IADD::changeLastHopIp(Ptr<Packet> p, Ipv4Address myAdd, bool isUDP) {
	UdpHeader u;
	IADDTypeHeader th;
	IADDInterestHeader intH;
	if (isUDP)
		p->RemoveHeader(u);
	p->RemoveHeader(th);
	p->RemoveHeader(intH);

	intH.setLastHopIp(myAdd);

	p->AddHeader(intH);
	p->AddHeader(th);
	if (isUDP)
		p->AddHeader(u);

}
Ipv4Address IADD::getLastHopIp(Ptr<const Packet> p) {
	IADDInterestHeader th = getInterestHeader(p);
	if (th.getRange() == 0)
		return 0;
	return getInterestHeader(p).getLastHopIp();
}
bool IADD::RouteInputVehicle(Ptr<const Packet> p, const Ipv4Header &header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) {
//Check packet type
	MessageType packetType = getPacketType(p);
if(p->GetUid()==48421 && getNodeId()==93 && Simulator::Now()>Seconds(730))
	cout<<"hhhhh";
	if (packetType == IADD_DATA) { // Data to Vehicle
		Ipv4Address from = getLastHopIp(p);
		if (tracingEnabled)
			tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
					myPosition(), p->GetUid());

		NS_LOG_INFO(
				"Data packet "<<p->GetUid()<<" is received from "<< from <<" RCSmax "<<getInterestHeader(p).getRcSmax()<<" c="<<getInterestHeader(p).getRcSmaxCentrality());
		bool dummy = false;
		return VehicleDataForwarding(p, header, ucb, ecb, false, dummy); // Forward data
	} else if (packetType == IADD_INTEREST) { // Interest to Vehicle
		Ipv4Address from = getLastHopIp(p);
		if (tracingEnabled)
			tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
					myPosition(), p->GetUid());

		NS_LOG_INFO(
				"Interest packet "<<p->GetUid()<<" is received from "<< from<<" RCSmax "<<getInterestHeader(p).getRcSmax()<<" c="<<getInterestHeader(p).getRcSmaxCentrality());
		bool dummy = false;
		return VehicleInterestForwarding(p, header, ucb, ecb, false, dummy); // Forward Interest
	} else if (packetType == IADD_RBEACON || packetType == IADD_VBEACON) {
		if (lcb.IsNull() == false) {
			int32_t iif = m_ipv4->GetInterfaceForDevice(idev);
			lcb(p, header, iif);
			return true;
		} else
			NS_LOG_ERROR(
					"Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << header.GetSource());
	}
	return false;

}

// DI : Insert data to Datacache
bool IADD::RouteInputRSU(Ptr<const Packet> p, const Ipv4Header &header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) {
	NS_LOG_FUNCTION(
			this << p->GetUid () << header.GetSource() << header.GetDestination());

//Check packet type
	MessageType packetType = getPacketType(p);
	if (p->GetUid() == 131256 && getNodeId() == 13)
		cout << "ssss" << endl;
	int32_t iif = m_ipv4->GetInterfaceForDevice(idev);
	Ipv4Address dst = header.GetDestination();
	if (packetType == IADD_DATA) { // Data to RSU
		Ipv4Address from = getLastHopIp(p);
		if (tracingEnabled)
			tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
					myPosition(), p->GetUid());
		if (from != getIpAddress())
			NS_LOG_INFO(
					"Data packet "<<p->GetUid()<<" is received from "<<from);
		/*if (protocol == CADD && (isCacheFlagOn(p)) && from != getIpAddress()) {
		 NS_LOG_INFO("Cache data packet "<<p->GetUid());
		 IADDInterestHeader th = IADD::getInterestHeader(p);
		 bool c = datacache.addPacket(p, getPopularity(th.getInterestType()),
		 popularity);
		 if (!c)
		 NS_LOG_INFO("Cannot cache "<<p->GetUid()<<", cache is full ");

		 numberofReplicasRecieved->Update(1);
		 }*/
		if (protocol == CADD && m_ipv4->IsDestinationAddress(dst, iif)) {
			NS_LOG_INFO(
					"Data packet "<<p->GetUid()<<" is sent to me, cache packet");
			IADDInterestHeader th = IADD::getInterestHeader(p);
			bool c = datacache.addPacket(p, getPopularity(th.getInterestType()),
					popularity);
			if (!c)
				NS_LOG_INFO("Cannot cache "<<p->GetUid()<<", cache is full ");
			numberofReplicasRecieved->Update(1);
			return true;
		}
		if (isFwdFlagOn(p) || protocol == REGIADD) {
			bool dummy = false;
			return RSUDataForwarding(p, header, ucb, ecb, false, dummy);
		} else
			return true; // already forwarded keep a copy only
	} else if (packetType == IADD_INTEREST) { // Interest to RSU
		Ipv4Address from = getLastHopIp(p);
		if (tracingEnabled)
			tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
					myPosition(), p->GetUid());
		NS_LOG_INFO("Interest packet "<<p->GetUid()<<" is received "<<from);
		IADDInterestHeader t = getInterestHeader(p);
		if (isExpiredInterest(t)) {
			NS_LOG_INFO(
					"Interest "<< p->GetUid()<<" has expired. Time generated="<<t.getTimeGenerated().GetSeconds()<<", TTL="<<t.getTtl().GetSeconds());
			return true; // drop expired packet
		}
		interestCount_currentHour++;
		Ptr<Packet> packet = p->Copy();
		if (getRSUCentrality() >= t.getRcSmaxCentrality()) {
			updateRCSmaxinPacket(packet, getRSUCentrality(), getIpAddress(),
					myPosition());
		} else if (getRSUCentrality() > t.getRcSmax2Centrality()) {
			updateRCS2maxinPacket(packet, getRSUCentrality(), getIpAddress());
		}

		bool dummy = false;
		return RSUInterestForwarding(packet, header, ucb, ecb, false, dummy);
	} else if (packetType == IADD_RBEACON || packetType == IADD_VBEACON) {
		lcb(p, header, iif);
		return true;
	}
	return false;

}
void IADD::updateRCSmaxinPacket(Ptr<Packet> p, uint32_t maxCentrality,
		Ipv4Address RCSmaxIP, Vector RCSLoc) {
	IADDInterestHeader interest;
	IADDTypeHeader typeH;
	UdpHeader u;
	p->RemoveHeader(u);
	p->RemoveHeader(typeH);
	p->RemoveHeader(interest);

	interest.setRcSmax2Centrality(interest.getRcSmaxCentrality());
	interest.setRcSmax2(interest.getRcSmax());

	interest.setRcSmaxCentrality(maxCentrality);
	interest.setRcSmax(RCSmaxIP);
	interest.setRcSmaxX(RCSLoc.x);
	interest.setRcSmaxY(RCSLoc.y);

	p->AddHeader(interest);
	p->AddHeader(typeH);
	p->AddHeader(u);
}
void IADD::updateRCS2maxinPacket(Ptr<Packet> p, uint32_t max2Centrality,
		Ipv4Address RCSmax2IP) {
	IADDInterestHeader interest;
	IADDTypeHeader typeH;
	UdpHeader u;
	p->RemoveHeader(u);
	p->RemoveHeader(typeH);
	p->RemoveHeader(interest);

	interest.setRcSmax2Centrality(max2Centrality);
	interest.setRcSmax2(RCSmax2IP);

	p->AddHeader(interest);
	p->AddHeader(typeH);
	p->AddHeader(u);
}
uint32_t IADD::getRSUCentrality() {
	return interestCount_lastHour;
}

MessageType IADD::getPacketType(Ptr<const Packet> packet) {
	Ptr<Packet> p = packet->Copy();
	UdpHeader u;
	p->RemoveHeader(u);
	IADDTypeHeader h(IADD_UNKNOWN);
	p->RemoveHeader(h);

	if (!h.isValid()) {
		// DI :
//		NS_LOG_WARN(
//				"Message received with unknown message type, Message Dropped\n");
//		return h.getType();
		return IADD_DATA;
	}
	return h.getType();
}

bool IADD::RouteInputDest(Ptr<const Packet> p, const Ipv4Header &header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) {
//Check packet type
	MessageType packetType = getPacketType(p);

	int32_t iif = m_ipv4->GetInterfaceForDevice(idev);
	Ipv4Address dst = header.GetDestination();

	if (packetType == IADD_DATA) { // Data to Dest
		if (m_ipv4->IsDestinationAddress(dst, iif)) {
			Ipv4Address from = getLastHopIp(p);

			if (tracingEnabled)
				tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
						myPosition(), p->GetUid());
			lcb(p, header, iif);
			return true;
		}

	} else if (packetType == IADD_INTEREST) { // Interest to Dest
		if (header.GetSource() == getIpAddress()) {
			bool dummy = false;
			return DestInterestForwarding(p, header, ucb, ecb, false, dummy);
		}
		Ipv4Address from = getLastHopIp(p);

		if (tracingEnabled)
			tracer->receiveOperation(getIpAddress(), from, Simulator::Now(),
					myPosition(), p->GetUid());
		return false; // not interested in interest packets :)
	} else if (packetType == IADD_RBEACON || packetType == IADD_VBEACON) {
		lcb(p, header, iif);
		return true;
	}

	return false;

}
Ipv4Address IADD::getIpAddress() {
	return m_ipv4->GetAddress(1, 0).GetLocal();
}

bool IADD::DestInterestForwarding(Ptr<const Packet> p,
		const Ipv4Header & header, UnicastForwardCallback ucb,
		ErrorCallback ecb, bool inPacketList, bool& packetfwd) {

	IADDInterestHeader i = getInterestHeader(p);

	if (isExpiredInterest(i)) {
		NS_LOG_INFO(
				"Interest "<< p->GetUid()<<" has expired. Time generated="<<i.getTimeGenerated().GetSeconds()<<", TTL="<<i.getTtl().GetSeconds());
		packetfwd = true;
		return true; // drop expired packet
	}

	Vector c(i.getCenterX(), i.getCenterY(), 0);
	Ipv4Address nextHop;
	Ptr<RSU> rsu = neighborList.getFurthestNeighborR(myPosition());
	if (rsu) {
		nextHop = rsu->getAddress();
	} else {
		Vehicle nextV = neighborList.getClosestNeighbortoRange(c, myPosition());
		if (nextV.getAddress() == "0.0.0.0") {
			if (!inPacketList) {

				PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
						getLastHopIp(p));
				packetsList.packets.push_back(en);
			}
			return false;
		}
		nextHop = nextV.getAddress();
	}
	if (nextHop != "0.0.0.0" && nextHop != "102.102.102.102") {
		NS_LOG_INFO("Sending Interest "<<getPacetInfo(p)<<" to "<<nextHop);
		prepareRouteandSend(p, header, ucb, nextHop);
		packetfwd = true;
		return true;
	} else {
		if (!inPacketList) {
			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		return true;
	}

	return false;
}

bool IADD::VehicleDataForwarding(Ptr<const Packet> p, const Ipv4Header & header,
		UnicastForwardCallback ucb, ErrorCallback ecb, bool fromPacketList,
		bool &packetFwd) {
	NS_LOG_FUNCTION(this);
//if (fromPacketList)
//	NS_LOG_INFO("Forwarding Data Packet: "<<p->GetUid()<<" From packet list"<<" TTL "<<(uint32_t)header.GetTtl());
//else
//	NS_LOG_INFO("Forwarding just arrived data Packet: "<<p->GetUid()<<" TTL "<<(uint32_t)header.GetTtl());
	Ipv4Address nextHop;

	if (getPacketType(p) != IADD_DATA)
		return false;

	IADDInterestHeader interestH = getInterestHeader(p);
	//if (p->GetUid() == 44978)
	//	cout << "ssss" << endl;
	double x = interestH.getReqX();
	double y = interestH.getReqY();
	Vector dstLoc(x, y, 0);
	//if (header.GetDestination() == "192.168.0.10")
	//cout << "ssss" << endl;

	if (neighborList.isNeighbor(header.GetDestination(),myPosition())) {
//		NS_LOG_INFO(
//				"Data "<<getPacetInfo(p)<<" , "<<header.GetDestination()<<": The dest is my Neighbor, what are you waiting for?");

		prepareRouteandSend(p, header, ucb, header.GetDestination());
		if (fromPacketList) {
			packetFwd = true;
		}
		return true;
	}

	if (protocol == SCF)
		return SCFVehicleForwarding(p, header, ucb, ecb, fromPacketList,
				packetFwd);

	bool foundForwarder = false;
	if (mode == SEGMENT_MODE) { //segment mode
		Vehicle nextV = neighborList.getFurthestNeighborwithRSUNext(RSUnext,
				myPosition());
		if (nextV.getAddress() == "0.0.0.0") {
			if (!fromPacketList) {
				PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
						getLastHopIp(p));
				packetsList.packets.push_back(en);
			}
			//S_LOG_INFO(p->GetUid()<<"("<<typeH.getTypeString()<<"): No good neighbor, the packet stays with me"<< " TTL "<<(uint32_t)header.GetTtl());
			return true; // carry packet
		} else {
			nextHop = nextV.getAddress();
			//NS_LOG_INFO("(Dis. to RSU ->"<<distance(myPosition(),RSUnext.getPosition())<<" ):");NS_LOG_INFO("I found a vehicle better than me: "<<nextHop<<", position:("<<nextV->getPosition()<<" ->"<<distance(nextV->getPosition(),RSUnext.getPosition())<<" ) Send packet ="<<p->GetUid()<< " TTL "<<(uint32_t)header.GetTtl());
			double ds = distance(myPosition(), nextV.getPosition());

			NS_LOG_INFO(
					"Data "<< getPacetInfo(p)<<" is forwarded to closest vehicle to RSU, to " << nextHop<<", distance"<<ds);

			prepareRouteandSend(p, header, ucb, nextHop);

			if (fromPacketList) {
				packetFwd = true;
			}
			return true;
		}
	} else { // intersection mode
		bool cache = false;
		// send copy to RCS MAx
		if (protocol == CADD && !isCachedBefore(p)) {
			//******
			//TODO changes for random caching 29-5
			//double probability = UniformVariable().GetValue(0,1);
			//if(probability < RANDOM_CAHING_THRESHOLD){
			//******
			if (myRSU.getAddress() == interestH.getRcSmax()) {
				Ptr<Packet> packet = createNewPacketFrom(p);
				cache = true;
				setPacketFlags(packet, 0, 1, true);
				setCachedBefore(packet, true);
				setNewDestLoc(packet, myRSU.getPosition().x,
						myRSU.getPosition().y);
				Ipv4Header headerNew(header);
				headerNew.SetDestination(myRSU.getAddress());
				prepareRouteandSend(packet, headerNew, ucb, myRSU.getAddress());
				numberofPacketsReplicated->Update();
				if (tracingEnabled)
					tracer->replicaCreated(getIpAddress(), Simulator::Now(),
							myPosition(), p->GetUid(), packet->GetUid(),
							myRSU.getAddress());
				NS_LOG_INFO(
						"Send copy of Data packet "<<p->GetUid()<<" to MyRSU: "<< myRSU.getAddress()<<", I's the maximum");
			} else if (!gps->isVehicleHeadingtoDst(myVelocity(), myPosition(),
					Vector(interestH.getRcSmaxX(), interestH.getRcSmaxY(), 0))
					&& interestH.getRcSmax() != "102.102.102.102"
					&& interestH.getRcSmax() != "0.0.0.0") {
				Ptr<Packet> packet = createNewPacketFrom(p);
				setPacketFlags(packet, 1, 0, true);
				setNewDestLoc(packet, interestH.getRcSmaxX(),
						interestH.getRcSmaxY());
				Ipv4Header headerNew(header);
				headerNew.SetDestination(interestH.getRcSmax());
				cache = true;
				setCachedBefore(packet, true);
				PendingPacketsEntry en(packet, headerNew, ucb, ecb,
						Simulator::Now(), getLastHopIp(p));
				packetsList.packets.push_back(en);
				numberofPacketsReplicated->Update();
				if (tracingEnabled)
					tracer->replicaCreated(getIpAddress(), Simulator::Now(),
							myPosition(), p->GetUid(), packet->GetUid(),
							interestH.getRcSmax());
				NS_LOG_INFO(
						"Send copy of data packet ("<<p->GetUid()<<" to RSUmax: "<< interestH.getRcSmax());
			}
		}
		Vehicle nextV;
		list<Ptr<Segment> > s = createAndSortSegmentList(dstLoc);
		bool iambetter = false;
		for (list<Ptr<Segment> >::iterator i = s.begin(); i != s.end(); ++i) {
			nextV = neighborList.getFurthestNeighborwithRSUNext((*i)->rsuNext,
					RSUnext, myPosition(), iambetter);
			if (nextV.getAddress() != "0.0.0.0") {
				nextHop = nextV.getAddress();
				foundForwarder = true;
				break;
			}
			if (iambetter)
				break;
			nextV.setAddress("0.0.0.0");
		}
		if (foundForwarder) {
			NS_LOG_INFO("Found Forwarder: "<<nextHop);
			//	bool cached = false;
			Time t;
			Ipv4Address a;
			packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates
			Ptr<Packet> packet = p->Copy();
			//TODO fixing cached to cache 29-5
			if (cache)
				setCachedBefore(packet, true);
			if (Simulator::Now() < t + Seconds(LOOP_TIME_OUT) && a == nextHop) {
				//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << nextHop);
				if (!fromPacketList) {
							PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
									getLastHopIp(p));
							packetsList.packets.push_back(en);
						}
			} else {
				// Send to next vehicle
				prepareRouteandSend(packet, header, ucb, nextHop);

				if (fromPacketList) {
					packetFwd = true;
				}NS_LOG_INFO("Send packet ("<<p->GetUid()<<") to: "<<nextHop);
			}
			return true;

		} else { // could not find a neighbor in intersection

			//	NS_LOG_INFO(p->GetUid()<<"("<<typeH.getTypeString()<<"): No good neighbor");
			if (!(iambetter
					|| gps->isVehicleHeadingtoDst(myVelocity(), myPosition(),
							dstLoc))) //send to RSU
			{
				if (fromPacketList) {
					packetFwd = true;
				}
				Time t;
				Ipv4Address a;
				packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates
				if (Simulator::Now() < t + Seconds(LOOP_TIME_OUT)
						&& a == myRSU.getAddress()) {
					//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << myRSU.getAddress());
					if (!fromPacketList) {
						PendingPacketsEntry en(p, header, ucb, ecb,
								Simulator::Now(), getLastHopIp(p));
						packetsList.packets.push_back(en);
						packetFwd = false; // don't delete from packet list
					}
				} else {
					Ptr<Packet> packet = p->Copy();
					setPacketFlags(packet, 1, 0, true);
					prepareRouteandSend(packet, header, ucb,
							myRSU.getAddress());
					NS_LOG_INFO(
							"Send Data packet "<<p->GetUid()<<" to RSU: "<< myRSU.getAddress());
				}
				return true;
			}

			if (iambetter
					|| gps->isVehicleHeadingtoDst(myVelocity(), myPosition(),
							dstLoc)) {
				//NS_LOG_INFO(p->GetUid()<<"("<<typeH.getTypeString()<<"): I'm going there, the packet stays with me");
				Ptr<Packet> packet = p->Copy();
				if (cache)
					setCachedBefore(packet, true);
				if (!fromPacketList) {
					PendingPacketsEntry en(packet, header, ucb, ecb,
							Simulator::Now(), getLastHopIp(p));
					packetsList.packets.push_back(en);
					packetFwd = false; // don't delete from packet list
				} else if (cache) {
					PendingPacketsEntry en(packet, header, ucb, ecb,
							Simulator::Now(), getLastHopIp(p));
					packetsList.packets.push_back(en);
					packetFwd = true; // delete old packet and new oacket with cachedb4 fag is on
				}
			}
			return true;
		}
	}
	//cout << "Return false" << endl;
	;
	return false;
}
double IADD::distance(Vector v1, Vector v2) {
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
}
IADDInterestHeader IADD::getInterestHeader(Ptr<const Packet> p) {
	Ptr<Packet> packet = p->Copy();
	UdpHeader u;
	packet->RemoveHeader(u);

	IADDTypeHeader typeH(IADD_UNKNOWN);
	packet->RemoveHeader(typeH);
	IADDInterestHeader interestH;

	if (!typeH.getType() == IADD_DATA && !typeH.getType() == IADD_INTEREST)
		return interestH;

	packet->RemoveHeader(interestH);
	return interestH;
}
IADDInterestHeader IADD::getInterestHeaderNoUDP(Ptr<const Packet> p) {
	Ptr<Packet> packet = p->Copy();

	IADDTypeHeader typeH(IADD_DATA);
	packet->RemoveHeader(typeH);
	IADDInterestHeader interestH;

	if (!typeH.getType() == IADD_DATA && !typeH.getType() == IADD_INTEREST)
		return interestH;

	packet->RemoveHeader(interestH);
	return interestH;
}
bool IADD::isFwdFlagOn(Ptr<const Packet> p) {
	IADDInterestHeader h = getInterestHeader(p);
	return h.isFwdFlag();
}
bool IADD::isCacheFlagOn(Ptr<const Packet> p) {
	IADDInterestHeader h = getInterestHeader(p);
	return h.isCacheFlag();
}
void IADD::setPacketFlags(Ptr<Packet> p, bool fwdFlag, bool cacheFlag,
		bool isUDP) {
	IADDInterestHeader interest;
	IADDTypeHeader typeH;
	UdpHeader u;
	if (isUDP)
		p->RemoveHeader(u);
	p->RemoveHeader(typeH);
	p->RemoveHeader(interest);
	interest.setFwdFlag(fwdFlag);
	interest.setCacheFlag(cacheFlag);
	p->AddHeader(interest);
	p->AddHeader(typeH);
	if (isUDP)

		p->AddHeader(u);
}
void IADD::setNewDestLoc(Ptr<Packet> p, double x, double y) {
	IADDInterestHeader interest;
	IADDTypeHeader typeH;
	UdpHeader u;
	p->RemoveHeader(u);
	p->RemoveHeader(typeH);
	p->RemoveHeader(interest);
	interest.setReqX(x);
	interest.setReqY(y);
	p->AddHeader(interest);
	p->AddHeader(typeH);
	p->AddHeader(u);
}

// DI -
bool IADD::VehicleInterestForwarding(Ptr<const Packet> p,
		const Ipv4Header & header, UnicastForwardCallback ucb,
		ErrorCallback ecb, bool fromPacketList, bool &packetFwd) {
	NS_LOG_FUNCTION(this);
//if (fromPacketList)
//	NS_LOG_INFO("Forwarding Interest Packet: "<<p->GetUid()<<" From packet list"<<" TTL "<<(uint32_t)header.GetTtl());
//else
//	NS_LOG_INFO("Forwarding just arrived Interest Packet: "<<p->GetUid()<<" TTL "<<(uint32_t)header.GetTtl());
	packetFwd = false;
	Ipv4Address nextHop;

	if (getPacketType(p) != IADD_INTEREST)
		return false;
	//if (p->GetUid() == 20682)
	//		cout << "stop" << endl;
	IADDInterestHeader interestH = getInterestHeader(p);

	if (isExpiredInterest(interestH)) {
		NS_LOG_INFO(
				"Interest "<< p->GetUid()<<" has expired. Time generated="<<interestH.getTimeGenerated().GetSeconds()<<", TTL="<<interestH.getTtl().GetSeconds());
		packetFwd = true;
		return true; // drop expired packet
	}

	double x = interestH.getCenterX();
	double y = interestH.getCenterY();
	Vector dstLoc(x, y, 0);

	double r = interestH.getRange();

	double d = distance(myPosition(), dstLoc);
	if (d < r) {
		//create data packet and send it to requester
		//TODOIADD:Done Now7: data packet Vehicle
		Ptr<Packet> dataPacket = Create<Packet>();
		IADDInterestHeader dh(interestH.getReqX(), interestH.getReqY(),
				myPosition().x, myPosition().y, r, false, false,
				Simulator::Now(), interestH.getInterestType());
		dh.setTtl(Seconds(IADD_DATA_TTL));
		dh.setRcSmax(interestH.getRcSmax());
		dh.setRcSmax2(interestH.getRcSmax2());
		dh.setRcSmaxCentrality(interestH.getRcSmaxCentrality());
		dh.setRcSmax2Centrality(interestH.getRcSmax2Centrality());
		dh.setRcSmaxX(interestH.getRcSmaxX());
		dh.setRcSmaxY(interestH.getRcSmaxY());

		dh.setInterestUid(p->GetUid());
// TODO rcsmax2

		dataPacket->AddHeader(dh);
		IADDTypeHeader th(IADD_DATA);
		dataPacket->AddHeader(th);
//		setPacketFlags(dataPacket,1,0);
		numberofPacketsGeneratedVehicle->Update();
		NS_LOG_INFO(
				"Data packet "<<dataPacket->GetUid()<<"("<<getInterestHeaderNoUDP(dataPacket).getReqX()<<","<<getInterestHeaderNoUDP(dataPacket).getReqY()<<" "<<getInterestHeaderNoUDP(dataPacket).getCenterX()<<","<<getInterestHeaderNoUDP(dataPacket).getCenterY()<<") is created for interest "<<p->GetUid()<<" and going to "<<header.GetSource());
		SendTo(dataPacket, header.GetSource(), IADD_APP_PORT);// send to requester
		if (fromPacketList) {
			packetFwd = true;
		}
		if (tracingEnabled)
			tracer->dataCreated(getIpAddress(), Simulator::Now(), myPosition(),
					p->GetUid(), dataPacket->GetUid(), header.GetSource());
		return true;
	}
	if (protocol == SCF)
		return SCFVehicleForwarding(p, header, ucb, ecb, fromPacketList,
				packetFwd);
	//bool foundForwarder = false;
	if (mode == SEGMENT_MODE) { //segment mode
		Vehicle nextV = neighborList.getFurthestNeighborwithRSUNext(RSUnext,
				myPosition());

		if (nextV.getAddress() == "0.0.0.0") {
			if (!fromPacketList) {
				PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
						getLastHopIp(p));
				packetsList.packets.push_back(en);
			} //NS_LOG_INFO(p->GetUid()<<"("<<typeH.getTypeString()<<"): No good neighbor, the packet stays with me"<< " TTL "<<(uint32_t)header.GetTtl());
			return true; // carry packet
		} else {
			nextHop = nextV.getAddress();
			//NS_LOG_INFO("(Dis. to RSU ->"<<distance(myPosition(),RSUnext.getPosition())<<" ) I found a vehicle better than me: "<<nextHop<<", position:("<<nextV->getPosition()<<" ->"<<distance(nextV->getPosition(),RSUnext.getPosition())<<" ) Send packet ="<<p->GetUid()<< " TTL "<<(uint32_t)header.GetTtl());
			Time t;
			Ipv4Address a;
			packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates
			if ((Simulator::Now() < t + Seconds(LOOP_TIME_OUT) && a == nextHop)
					|| nextHop == getLastHopIp(p)) {
				//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << nextHop);
				//new change after 25th of Mar
				if (!fromPacketList) {
					PendingPacketsEntry en(p, header, ucb, ecb,
							Simulator::Now(), getLastHopIp(p));
					packetsList.packets.push_back(en);
				}
			} else {
				NS_LOG_INFO(
						"Interest "<< getPacetInfo(p)<<" is forwarded to closest vehicle to RSU, to " << nextHop);

				prepareRouteandSend(p, header, ucb, nextHop);

				if (fromPacketList) {
					packetFwd = true;
				}
			}
			return true;
		}
	} else { // intersection mode

		Time t;
		Ipv4Address a;
		packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates

		if ((myRSU.getAddress() == interestH.getLastHopIp() && !fromPacketList)
				|| (Simulator::Now() < t + Seconds(LOOP_TIME_OUT)
						&& a == myRSU.getAddress())) {
			//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << myRSU.getAddress());
		} else {
			NS_LOG_INFO(
					"Interest "<< getPacetInfo(p)<<" is forwarded to RSU " << myRSU.getAddress());

			packetFwd = true;

			prepareRouteandSend(p, header, ucb, myRSU.getAddress());
		}
		if (!fromPacketList && !packetFwd) {
			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		return true;

	}
	return false;
}

void IADD::prepareRouteandSend(Ptr<const Packet> p, const Ipv4Header & header,
		UnicastForwardCallback ucb, Ipv4Address gateway) {
	Ptr<Packet> packet = p->Copy();
	MessageType th = getPacketType(p);
	if (th == IADD_DATA || th == IADD_INTEREST)
		changeLastHopIp(packet, getIpAddress(), true);

	Ptr<Ipv4Route> route = Create<Ipv4Route>();
	route->SetDestination(header.GetDestination());
	route->SetGateway(gateway);
	route->SetSource(header.GetSource());
	route->SetOutputDevice(m_ipv4->GetNetDevice(1));
	//if (gateway == "102.102.102.102")
	//	cout << "wait";
	if (tracingEnabled)
		tracer->sendOperation(getIpAddress(), gateway, Simulator::Now(),
				myPosition(), p->GetUid(), th, header.GetDestination());
	ucb(route, packet, header);
}

Vector IADD::myPosition() {
	Ptr<MobilityModel> m = m_ipv4->GetObject<MobilityModel>();
	return m->GetPosition();
}
Vector IADD::myVelocity() {
	Ptr<MobilityModel> m = m_ipv4->GetObject<MobilityModel>();
	return m->GetVelocity();
}
bool IADD::RSUDataForwarding(Ptr<const Packet> p, const Ipv4Header & header,
		UnicastForwardCallback ucb, ErrorCallback ecb, bool fromPacketList,
		bool &packetFwd) {
	NS_LOG_FUNCTION(this);

	if (getPacketType(p) != IADD_DATA)
	{   NS_LOG_INFO("--------------1"<<endl);
		return false;
	}
	IADDInterestHeader interestH = getInterestHeader(p);

	double x = interestH.getReqX();
	double y = interestH.getReqY();
	Vector dstLoc(x, y, 0);

	// DI : the use of neighborList
	if (neighborList.isNeighbor(header.GetDestination(),myPosition())) {
//		NS_LOG_INFO(
//				"Data "<<getPacetInfo(p)<<" , "<<header.GetDestination()<<": The dest is my Neighbor, what are you waiting for?");

		prepareRouteandSend(p, header, ucb, header.GetDestination());
		if (fromPacketList) {
			packetFwd = true;
		}
		return true;
	}

	Ipv4Address nextHop;
	Vehicle nextV;
	list<Ptr<Segment> > s = createAndSortSegmentList(dstLoc);

//ostringstream os;
//neighborList.printList(os);
//NS_LOG_INFO(os.str());

	for (list<Ptr<Segment> >::iterator i = s.begin(); i != s.end(); ++i) {

		nextV = neighborList.getFurthestNeighborwithRSUNext((*i)->rsuNext,
				myPosition());

		if (nextV.getAddress() != "0.0.0.0"
				&& distance(myPosition(), dstLoc)
						> distance(nextV.getPosition(), dstLoc)) {
			nextHop = nextV.getAddress();

			break;
		}
		nextV.setAddress("0.0.0.0");
	}
	if (nextV.getAddress() == "0.0.0.0") { // could not find a vehicle
		if (!fromPacketList) {
			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		return true; // carry packet

	}
	NS_LOG_INFO(p->GetUid()<<": Send data packet to "<< nextHop);
	prepareRouteandSend(p, header, ucb, nextHop);

	if (fromPacketList) {
		packetFwd = true;
	}
	return true;
}
bool IADD::isExpiredInterest(IADDInterestHeader i) {
	Time exp = i.getTimeGenerated() + i.getTtl();
	if (exp < Simulator::Now())
		return true;
	return false;
}
bool IADD::RSUInterestForwarding(Ptr<const Packet> p, const Ipv4Header & header,
		UnicastForwardCallback ucb, ErrorCallback ecb, bool fromPacketList,
		bool &packetFwd) {
	NS_LOG_FUNCTION(this);

	if (getPacketType(p) != IADD_INTEREST)
		return false;

	IADDInterestHeader interestH = getInterestHeader(p);

	if (isExpiredInterest(interestH)) {
		NS_LOG_INFO(
				"Interest "<< p->GetUid()<<" has expired. Time generated="<<interestH.getTimeGenerated()<<", TTL="<<interestH.getTtl()<<", Now="<<Simulator::Now());
		packetFwd = true;
		return true; // drop expired packet
	}

	Ptr<Packet> cahedPacket; // = Create<Packet>();

	// DI : changed the code
	cahedPacket = p->Copy();
	if (true) {
	// DI : original code
//	if (datacache.getPacket(interestH, cahedPacket)) {
		NS_LOG_INFO(
				"Cache hit for interest "<<p->GetUid()<<", reply with Data packet "<<cahedPacket->GetUid());
		//hitRatio->Update(1);
		//TODOIADD:Done Nowdata packet RSU

		UdpHeader u;
		IADDTypeHeader th(IADD_DATA);
		IADDInterestHeader t;

		IADDInterestHeader dataHeader;
		cahedPacket->RemoveHeader(u);
		cahedPacket->RemoveHeader(th);
		th.setType(IADD_DATA);
		cahedPacket->RemoveHeader(dataHeader);

		dataHeader.setReqX(interestH.getReqX());
		dataHeader.setReqY(interestH.getReqY());
		dataHeader.setTtl(Seconds(IADD_DATA_TTL));
		dataHeader.setRcSmax(interestH.getRcSmax());
		dataHeader.setRcSmax2(interestH.getRcSmax2());
		dataHeader.setRcSmaxCentrality(interestH.getRcSmaxCentrality());
		dataHeader.setRcSmax2Centrality(interestH.getRcSmax2Centrality());
		dataHeader.setRcSmaxX(interestH.getRcSmaxX());
		dataHeader.setRcSmaxY(interestH.getRcSmaxY());
		dataHeader.setInterestType(interestH.getInterestType());
		dataHeader.setTimeGenerated(Simulator::Now());


		dataHeader.setInterestUid(p->GetUid());


		NS_LOG_INFO("LOOP START");

		for(int i =0 ; i < 1000 ; i++)
		{
			Ptr<Packet> newPacket = Create<Packet>(2000);

			newPacket->AddHeader(dataHeader);
			newPacket->AddHeader(th);


			setPacketFlags(newPacket, 1, 0, false);
			setCachedBefore(newPacket, false);

			//newPacket->AddHeader(u);
			NS_LOG_INFO("SIZE: "<<newPacket->GetSize());
			numberofPacketsGeneratedCache->Update();


			SendTo(newPacket, header.GetSource(), IADD_APP_PORT); // send to requester
		}

		NS_LOG_INFO("LOOP END");

		// original source code
//		Ptr<Packet> newPacket = Create<Packet>(2000);
//
//		newPacket->AddHeader(dataHeader);
//		newPacket->AddHeader(th);
//
//
//		setPacketFlags(newPacket, 1, 0, false);
//		setCachedBefore(newPacket, false);
//
//				//newPacket->AddHeader(u);
//		NS_LOG_INFO("SIZE: "<<newPacket->GetSize());
//		numberofPacketsGeneratedCache->Update();
//
//
//		SendTo(newPacket, header.GetSource(), IADD_APP_PORT); // send to requester

//		if (tracingEnabled)
//			tracer->dataCreated(getIpAddress(), Simulator::Now(), myPosition(),
//					p->GetUid(), newPacket->GetUid(), header.GetSource());
		packetFwd = true;
		return true;
	}
	// increase popularity count for this type;
	if (!fromPacketList)
		popularity[interestH.getInterestType()]++;

	double x = interestH.getCenterX();
	double y = interestH.getCenterY();
	Vector dstLoc(x, y, 0);

	Ipv4Address nextHop;
	Vehicle nextV;
	list<Ptr<Segment> > s = createAndSortSegmentList(dstLoc);

	for (list<Ptr<Segment> >::iterator i = s.begin(); i != s.end(); ++i) {
		nextV = neighborList.getFurthestNeighborwithRSUNext((*i)->rsuNext,
				myPosition());
		//if (nextV != NULL)
		//NS_LOG_INFO("nextV"<<nextV->getAddress()<<" "<<nextV->getPosition()<<" "<<nextV->getRsuNext()<<" "<<(*i)->rsuNext.getAddress());
		//	if (p->GetUid() == 20682)
		//	cout << "stop" << endl;
		if (nextV.getAddress() != "0.0.0.0"
				&& distance(myPosition(), dstLoc)
						> distance(nextV.getPosition(), dstLoc)) { //==22688== Invalid read of size 4
			nextHop = nextV.getAddress();
			break;
		}
		nextV.setAddress("0.0.0.0");
	}

	if (nextV.getAddress() == "0.0.0.0") { // could not find a vehicle
		if (!fromPacketList) {

			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		return true; // carry packet
	}
	Time t;
	Ipv4Address a;
	packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates

	if ((Simulator::Now() < t + Seconds(LOOP_TIME_OUT - 2) && a == nextHop)
			|| (nextHop == getLastHopIp(p) && !fromPacketList)) {
		//	if ((Simulator::Now() < t + Seconds(LOOP_TIME_OUT-2) && a == nextHop)) {

		//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << nextHop);
		if (!fromPacketList) {

			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		return true; // carry packet
	} else {
		// Send to next vehicle
		prepareRouteandSend(p, header, ucb, nextHop);
		if (fromPacketList) {
			packetFwd = true;
		}
	}

	return true;
}
void IADD::beaconIntervalExpired() {
	NS_LOG_FUNCTION(this);
	sendBeacon();
	beaconTimer.Cancel();
	Time jitter = MilliSeconds(UniformVariable().GetInteger(0, MAX_JITTER));
	beaconTimer.Schedule(beaconInterval + jitter);
}

void IADD::sendBeacon() {
	/*
	 if(Simulator::Now() >= Seconds(174) && Simulator::Now() <= Seconds(177))
	 LogComponentEnableAll(LOG_LEVEL_DEBUG);
	 if(Simulator::Now() >= Seconds(177))
	 LogComponentDisableAll(LOG_LEVEL_DEBUG);
	 */

	NS_LOG_FUNCTION("Send Beacon");
	ForwardPendingPackets();
	if (myVelocity().x == 0 && myVelocity().y == 0)
		stopCounter++;
	else
		stopCounter = 0;

	if (stopCounter * BEACON_INTERVAL > STOP_TIMEOUT * 1000)
		vehicleStopped = true;
	else
		vehicleStopped = false;
	if (type == IADD_VEHICLE
			&& stopCounter * BEACON_INTERVAL > TERMINATE_TIMEOUT * 1000) {

		ForwardAllPackets();
	}

	if (vehicleStopped && type == IADD_VEHICLE) {
		//if(getNodeId()== 50 && (Simulator::Now().GetMilliSeconds())%2==0)
		//NS_LOG_INFO("Vehicle Stopped");
		return;
	}
	if (type == IADD_VEHICLE) {

		if (neighborList.rsu.size()
				== 0|| distance(myPosition(),myRSU.getPosition()) > RSU_RANGE) {
			mode = SEGMENT_MODE;
			prevVelocity.x = 0;
			prevVelocity.y = 0;
			prevVelocity.z = 0;
			prevPosition.x = 0;
			prevPosition.y = 0;
			prevPosition.z = 0;
			if (enableAnime)
				anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 0, 255, 0); // green
		}
		if (enableAnime) {
			if (mode == SEGMENT_MODE)
				anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 0, 255, 0);
			else if (mode == INTERSECTION_MODE)
				anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 0, 0, 0);
		}

		stringstream ss;
		ss << RSUnext.getAddress() << "," << myRSU.getAddress();
		if (enableAnime)
			anim->UpdateNodeDescription(m_ipv4->GetObject<Node>(), ss.str());

	}
	if (enableAnime && packetsList.packets.size() > 0) {
		anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 153, 0, 153);
	}
	if (enableAnime && type == IADD_DEST) {
		stringstream ss;
		ss << "I(" << getNumberofIntPackets() << ")";
		anim->UpdateNodeDescription(m_ipv4->GetObject<Node>(), ss.str());
	}
	Vector myPos = myPosition();
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet>();
		if (type == IADD_VEHICLE) {
			IADDVBeaconHeader header(myPos.x, myPos.y, RSUnext.getAddress());
			header.setHeading(calculateAngle(myVelocity()));
			packet->AddHeader(header);
			IADDTypeHeader tHeader(IADD_VBEACON);
			packet->AddHeader(tHeader);

		} else if (type == IADD_DEST) {
			IADDVBeaconHeader header(myPos.x, myPos.y, "192.168.254.254");
			header.setHeading(calculateAngle(myVelocity()));
			packet->AddHeader(header);
			IADDTypeHeader tHeader(IADD_VBEACON);
			packet->AddHeader(tHeader);
		} else if (type == IADD_RSU) {
			IADDRBeaconHeader header(myPos.x, myPos.y, getRSUCentrality());
			uint32_t j = 0;

			for (list<Ptr<Segment> >::iterator i = segmentsList.begin();
					i != segmentsList.end(); ++i) {

				header.setRSUNext(j++, *i);
			}
			/*for (; j < MAX_SEGMENTS; j++) {
			 Ptr<Segment> s = CreateObject<Segment>();
			 header.setRSUNext(j, s);
			 }*/
			packet->AddHeader(header);
			IADDTypeHeader tHeader(IADD_RBEACON);
			packet->AddHeader(tHeader);
		}

		// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
		Ipv4Address destination;
		if (iface.GetMask() == Ipv4Mask::GetOnes()) {
			destination = Ipv4Address("255.255.255.255");
		} else {
			destination = iface.GetBroadcast();
		}
		SendTo(socket, packet, destination);

	}

}

void IADD::SendTo(Ptr<Socket> socket, Ptr<Packet> packet,
		Ipv4Address destination) {
	socket->SendTo(packet, 0, InetSocketAddress(destination, IADD_PORT));

}
void IADD::SendTo(Ptr<Packet> packet, Ipv4Address destination, uint32_t port) {
	NS_LOG_INFO(
			"Sending Packet: "<<packet->GetUid()<<" to "<<destination<<" on "<<port);
	//NS_ASSERT(packet->GetSize() < 3000);
	Ptr<Packet> p = packet->Copy();
	IADDTypeHeader h(IADD_UNKNOWN);
	p->RemoveHeader(h);
	MessageType th = h.getType();
	if (th == IADD_DATA || th == IADD_INTEREST)
		changeLastHopIp(packet, getIpAddress(), false);
	Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
			UdpSocketFactory::GetTypeId());
	NS_ASSERT(socket != 0);
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
	socket->BindToNetDevice(l3->GetNetDevice(1));
	socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), IADD_APP_PORT));
	socket->SetAllowBroadcast(true);
	socket->SetAttribute("IpTtl", UintegerValue(255));

	socket->SendTo(packet, 0, InetSocketAddress(destination, port));
	socket->Close();

}

void IADD::RecvIadd(Ptr<Socket> socket) {

	NS_LOG_FUNCTION(this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(
			sourceAddress);
	Ipv4Address sender = inetSourceAddr.GetIpv4();
	//Ipv4Address receiver = m_socketAddresses[socket].GetLocal();
//NS_LOG_DEBUG ("IADD node " << this << " received a IADD packet from " << sender << " to " << receiver);

	IADDTypeHeader tHeader(IADD_VBEACON);
	packet->RemoveHeader(tHeader);
	if (!tHeader.isValid()) {
		NS_LOG_INFO(
				"IADD message " << packet->GetUid () << " with unknown type received: " << tHeader.getType()<< ". Drop");
		return; // drop
	}
	if (tHeader.getType() == IADD_VBEACON) {

		RecvVehicleBeacon(packet, sender);
	} else if (tHeader.getType() == IADD_RBEACON) {

		RecvRSUBeacon(packet, sender);
		if (mode == INTERSECTION_MODE)
			NS_ASSERT(RSUnext.getAddress() != "0.0.0.0");
	}
//NS_LOG_INFO("New Beacon, Neighbor List size: "<<neighborList.neighbors.size());
	//ForwardPendingPackets();
}
void IADD::RecvVehicleBeacon(Ptr<Packet> p, Ipv4Address src) {

	IADDVBeaconHeader header(0, 0, 0, 0);
	p->RemoveHeader(header);
	double x = header.getPositionX();
	double y = header.getPositionY();
	//if (getNodeId() == 10 && Simulator::Now() > Seconds(350) && src!="192.168.1.39" && src!="192.168.1.21") {
	//NS_LOG_INFO(x<<" "<<y<<" "<<src);
//}
//double speed = header.getSpeed();
//double heading = header.getHeading();

	Ipv4Address r(header.getRsuNext());
	Vector position(x, y, 0);
	Time ttl(beaconInterval.GetInteger() * (uint64_t) ALLOWED_BEACON_LOSS);
	Vehicle v(src, position, r);
	v.setHeading(header.getHeading());
	if (type == IADD_RSU && !neighborList.isNeighbor(src,myPosition())) {
		updateDensities(position, r);
	}
	neighborList.updateNeighbor(v, ttl);
	if (type == IADD_VEHICLE
			&& (neighborList.rsu.size() == 0
					|| distance(myPosition(), myRSU.getPosition()) > RSU_RANGE)) {
		mode = SEGMENT_MODE;
		prevVelocity.x = 0;
		prevVelocity.y = 0;
		prevVelocity.z = 0;
		prevPosition.x = 0;
		prevPosition.y = 0;
		prevPosition.z = 0;
		if (enableAnime)

			anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 0, 255, 0);

//		NS_LOG_INFO("Back to Segment Mode");

	}
	if (Simulator::Now().GetSeconds() / 100 == 0)
		NS_LOG_INFO("Neighbors: "<<neighborList.neighbors.size());

}
//TODOIADD:Done Now4: set RSUNExt
void IADD::RecvRSUBeacon(Ptr<Packet> p, Ipv4Address src) {
	/*if (type == IADD_VEHICLE
	 && (RSUnext.getAddress() != src && RSUnext.getAddress() != "0.0.0.0"))
	 {
	 //cout<<"HOW COME????";
	 uint32_t time = Simulator::Now().GetSeconds();
	 uint32_t id = getNodeId();
	 //cout<<time<<id;
	 }*/
	if (type == IADD_DEST) {
		IADDRBeaconHeader header(0, 0, 0);
		p->RemoveHeader(header);
		double x = header.getPositionX();
		double y = header.getPositionY();

//		if(getNodeId()==65)
//		{
//			NS_LOG_INFO(getNodeId() << " GetBeaconFromRSU"<<" X: "<<x<<" Y: "<<y);
//		}

		header.GetTypeId();
		Vector rsuPosition(x, y, 0);
		Time ttl(beaconInterval.GetInteger() * (int64_t) ALLOWED_BEACON_LOSS);
		RSU rsu(src, rsuPosition);
		neighborList.updateRSU(rsu, ttl);
		return;
	}
	if (type != IADD_VEHICLE)
		return;
	Vector myVel = myVelocity();
	Vector myPos = myPosition();
	IADDRBeaconHeader header(0, 0, 0);
	p->RemoveHeader(header);
	double x = header.getPositionX();
	double y = header.getPositionY();
	Vector rsuPosition(x, y, 0);
	Time ttl(beaconInterval.GetInteger() * (int64_t) ALLOWED_BEACON_LOSS);
	RSU rsu(src, rsuPosition);

	if (type == IADD_VEHICLE) {

		if (myRSU.getAddress() != "0.0.0.0"
				&& myRSU.getAddress() != "102.102.102.102") {
			if (distance(rsuPosition, myPos)
					> distance(myRSU.getPosition(), myPos)
					&& (rsuPosition.x != myRSU.getPosition().x
							&& rsuPosition.y != myRSU.getPosition().y))
				return;
		}

		myRSU.setAddress(src);
		myRSU.setPosition(rsuPosition);
		myRSU.setCentrality(header.getCentrality());
		neighborList.updateRSU(rsu, ttl);
	}
	if (type == IADD_VEHICLE
			&& (RSUnext.getAddress() == src || RSUnext.getAddress() == "0.0.0.0")) {
		if (distance(myPos, myRSU.getPosition()) > RSU_RANGE) {
			return;
		}

		if (prevVelocity.x == 0 && prevVelocity.y == 0
				&& RSUnext.getAddress() != "0.0.0.0") {
			prevVelocity = myVel;
			prevPosition = myPos;
			return;
		} else if ((isCourseChanged(prevVelocity, myVel)
				|| hasPassedThrough(prevPosition, myPos, rsuPosition)
				|| RSUnext.getAddress() == "0.0.0.0")
				&& mode != INTERSECTION_MODE
				&& !(myVel.x == 0 && myVel.y == 0)) {

			segmentsList.clear();
			for (int i = 0; i < MAX_SEGMENTS; i++) {
				RSU ip = header.getRsuNexts(i)->rsuNext;
				if (ip.getAddress() == "0.0.0.0")
					break;
				double d = header.getRsuNexts(i)->density;
				uint32_t sid = header.getRsuNexts(i)->segmentDirection;
				Ptr<Segment> s = CreateObject<Segment>(ip, d, sid);
				segmentsList.push_back(s);

				uint32_t myHeading = (uint32_t) calculateAngle(myVel);

				double diff = (double) myHeading - (double) sid;
				if (diff < 0)
					diff += 360;
				if (diff > 180)
					diff = 360 - diff;
				if (diff < 20 && ip.getAddress() != myRSU.getAddress()) { // direction thresh-hold

					RSUnext = ip;
					mode = INTERSECTION_MODE; // DoneTODO IADD: Now1: switch to intersection mode after changing heading

					//	NS_LOG_INFO("Change to Intersection mode, my heading: " << myHeading);
					/*prevVelocity.x = 0;
					 prevVelocity.y = 0;
					 prevVelocity.z = 0;
					 prevPosition.x = 0;
					 prevPosition.y = 0;
					 prevPosition.z = 0;*/

					if (enableAnime)
						anim->UpdateNodeColor(m_ipv4->GetObject<Node>(), 0, 00,
								00);
				}
			}
		}
	}
}
bool IADD::isCourseChanged(Vector oldV, Vector newV) {
	if (newV.x == 0 && newV.y == 0)
		return false;
	double a1 = calculateAngle(oldV);
	double a2 = calculateAngle(newV);
	double diff = a1 - a2;
	if (diff < 0)
		diff += 360;
	if (diff > 180)
		diff = 360 - diff;
	if (diff > 75)
		return true;
	return false;
}

bool IADD::hasPassedThrough(Vector oldP, Vector newP, Vector rsuP) {

	if ((rsuP.x - oldP.x) * (rsuP.x - newP.x) > 0
			&& (rsuP.y - oldP.y) * (rsuP.y - newP.y) > 0)
		return false;
	if (abs(rsuP.x - newP.x) > 5 || abs(rsuP.y - newP.y) > 5) // 5 meters away from the rsu
		return true;

	return false;

}
double IADD::calculateAngle(Vector velocity) {
	double ar = atan2(velocity.y, velocity.x);
	double a = ar * 180 / M_PI;
	if (a < 0)
		a += 360;

	return a;
}
double IADD::calculateAngle(Vector oldP, Vector newP) {
	return atan2((oldP.y - newP.y), (oldP.x - newP.x));
}
Ptr<Ipv4Route> IADD::LoopbackRoute(const Ipv4Header & hdr,
		Ptr<NetDevice> oif) const {
//NS_LOG_FUNCTION (this << hdr);
	NS_ASSERT(m_ipv4->GetNetDevice(0) != 0);
	Ptr<Ipv4Route> rt = Create<Ipv4Route>();
	rt->SetDestination(hdr.GetDestination());

	std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin();
	if (oif) {
		// Iterate to find an address on the oif device
		for (j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j) {
			Ipv4Address addr = j->second.GetLocal();
			int32_t interface = m_ipv4->GetInterfaceForAddress(addr);
			if (oif == m_ipv4->GetNetDevice(static_cast<uint32_t>(interface))) {
				rt->SetSource(addr);
				break;
			}
		}
	} else {
		rt->SetSource(j->second.GetLocal());
	}
	NS_ASSERT_MSG(rt->GetSource() != Ipv4Address(),
			"Valid IADD source address not found");
//new override the source address ( do not want my address as source)
//rt->SetSource(hdr.GetSource());
	rt->SetGateway(Ipv4Address("127.0.0.1"));
	rt->SetOutputDevice(m_ipv4->GetNetDevice(0));
	return rt;
}
void IADD::SetIpv4(Ptr<Ipv4> ipv4) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT(ipv4 != 0);
	NS_ASSERT(m_ipv4 == 0);

	m_ipv4 = ipv4;
	start();

//Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}
void IADD::PrintRoutingTable(Ptr<OutputStreamWrapper> stream) const {
	*stream->GetStream() << "Node: " << m_ipv4->GetObject<Node>()->GetId()
			<< " Time: " << Simulator::Now().GetSeconds()
			<< "s  Not implemented";
//m_routingTable.Print (stream);
}

string IADD::getNodeType() {
	if (type == IADD_RSU)
		return "RSU";
	if (type == IADD_VEHICLE)
		return "Vehicle";
	if (type == IADD_DEST)
		return "Dest";
	return "Unknown";

}
string IADD::getNodeInfo() {
	Ptr<Node> n = m_ipv4->GetObject<Node>();
	stringstream ss;
	ss << "Node " << n->GetId() << ": (" << getNodeType() << ")";
	return ss.str();

}

void IADD::setSegmentList(RSU r, list<Ptr<Segment> > s) {
	segmentsList.empty();
	for (list<Ptr<Segment> >::iterator i = s.begin(); i != s.end(); ++i) {
		segmentsList.push_back(*i);
	}
	myRSU = r;
	/* check segment list
	 cout<<myPosition()<<endl;
	 for (list<Ptr<Segment> >::iterator i = s.begin(); i != s.end(); ++i) {
	 cout<<(*i)->rsuNext.getPosition()<<" "<<(*i)->segmentDirection<<endl;
	 }
	 cout<<"---------"<<endl;
	 */
//int x = segmentsList.size();
//cout << x;
}
void IADD::setAnime(AnimationInterface* a) {
	anim = a;
	enableAnime = true;

}
void IADD::setCalculators(Ptr<MinMaxAvgTotalCalculator<double> > a,
		Ptr<CounterCalculator<> > b, Ptr<CounterCalculator<> > d,
		Ptr<CounterCalculator<> > c, Ptr<CounterCalculator<> > e) {
	hitRatio = a;
	numberofPacketsGeneratedCache = b;
	numberofPacketsGeneratedVehicle = d;
	numberofPacketsReplicated = c;
	numberofReplicasRecieved = e;

}
uint32_t IADD::getNodeId() {

	return getNode()->GetId();
}
Ptr<Node> IADD::getNode() {
	Ptr<Node> n = m_ipv4->GetObject<Node>();
	return n;

}
string IADD::getMode() {
	if (type != IADD_VEHICLE)
		return "";
	if (mode == SEGMENT_MODE)
		return "Seg";
	if (mode == INTERSECTION_MODE)
		return "Inter";
	return "";
}
uint32_t IADD::getNumberofDataPackets() {
	uint32_t ctr = 0;
	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
			i != packetsList.packets.end(); ++i) {
		Ptr<const Packet> p = i->getPacket();
		Ptr<Packet> packet = p->Copy();
		UdpHeader u;
		packet->RemoveHeader(u);

		IADDTypeHeader typeH(IADD_DATA);
		packet->RemoveHeader(typeH);
		//**** check if the data packet is not duplicate ( i,e, sent to dst node , not RSU)
		if (typeH.getType() == IADD_DATA && (i->getHeader().GetDestination() == "192.168.0.17" ||i->getHeader().GetDestination() == "192.168.0.18"||i->getHeader().GetDestination() == "192.168.0.19"||i->getHeader().GetDestination() == "192.168.0.20"))
			ctr++;
	}
	return ctr;

}
uint32_t IADD::getNumberofIntPackets() {
	uint32_t ctr = 0;
	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
			i != packetsList.packets.end(); ++i) {
		Ptr<const Packet> p = i->getPacket();
		Ptr<Packet> packet = p->Copy();
		UdpHeader u;
		packet->RemoveHeader(u);

		IADDTypeHeader typeH(IADD_DATA);
		packet->RemoveHeader(typeH);
		if (typeH.getType() == IADD_INTEREST)
			ctr++;
	}
	return ctr;
}

uint32_t IADD::getPopularity(IADDInterestTypes t) {
	return popularity.at(t);
}
void IADD::resetCentrality() {
	interestCount_lastHour = interestCount_currentHour;
	interestCount_currentHour = 0;
	if (getNodeId() == 0 || getNodeId() == 3 || getNodeId() == 12
			|| getNodeId() == 15)
		interestCount_lastHour = 0;
	centralityTimer.Cancel();
	centralityTimer.Schedule(Seconds(IADD_CENTRALITY_TIMEOUT));

}
string IADD::getLogPrefix() {
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();

	stringstream ss;
	ss << setiosflags(ios::fixed) << setprecision(1)
			<< Simulator::Now().GetSeconds() << "s [N#" << setw(3)
			<< getNodeId() << "," << getNodeType().c_str()[0] << ",(" << setw(3)
			<< (uint32_t) myPosition().x << "," << setw(3)
			<< (uint32_t) myPosition().y << " - " << myVelocity().x << ","
			<< setw(3) << myVelocity().y << ") I(" << setw(2)
			<< getNumberofIntPackets() << "), D(" << setw(2)
			<< getNumberofDataPackets() << "), ";
	if (type == IADD_VEHICLE)
		ss << getMode() << ", " << myRSU.getAddress() << ", "
				<< RSUnext.getAddress();

	if (type == IADD_RSU)
		ss << "Cent(" << getRSUCentrality() << "), $(" << setw(3)
				<< datacache.getCacheSize() << ") ";
	ss << " ] ";

	return ss.str();
}
bool IADD::SCFVehicleForwarding(Ptr<const Packet> p, const Ipv4Header & header,
		UnicastForwardCallback ucb, ErrorCallback ecb, bool fromPacketList,
		bool &packetFwd) {

	Ipv4Address nextHop;

	IADDInterestHeader interestH = getInterestHeader(p);

	double x =
			(getPacketType(p) == IADD_INTEREST) ?
					interestH.getCenterX() : interestH.getReqX();
	double y =
			(getPacketType(p) == IADD_INTEREST) ?
					interestH.getCenterY() : interestH.getReqY();
	Vector dstLoc(x, y, 0);
	if (myVelocity().x == 0 && myVelocity().y == 0) {
		if (!fromPacketList) {
			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);

		}
		packetFwd = false;

		return true;
	}
	Vehicle nextV = neighborList.getFurthestNeighborHeadingtoDst(dstLoc,
			myPosition(), gps->calculateAngle(myVelocity()), gps);

	if (nextV.getAddress() == "0.0.0.0"
			|| nextV.getAddress() == "102.102.102.102"
			|| nextV.getAddress() == getIpAddress()
			|| nextV.getRsuNext() == "192.168.254.254") {
		if (!fromPacketList) {
			PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
					getLastHopIp(p));
			packetsList.packets.push_back(en);
		}
		packetFwd = false;
		return true; // carry packet
	} else {
		//if (p->GetUid() == 816)
		//	cout << "stop plz";
		nextHop = nextV.getAddress();
		Time t;
		Ipv4Address a;
		packetsList.getInfoForPacket(p, t, a); // to avoid sending duplicates
		if ((Simulator::Now() < t + Seconds(LOOP_TIME_OUT) && a == nextHop)
				|| nextHop == getLastHopIp(p)) {
			//NS_LOG_INFO("Interest "<< p->GetUid()<<" cannot be forwarded again to " << nextHop);
			//new change after 25th of Mar
			if (!fromPacketList) {
				PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
						getLastHopIp(p));
				packetsList.packets.push_back(en);
			}
		} else {
			NS_LOG_INFO(
					"Send packet (type:"<<getPacketType(p)<<" ID: "<<p->GetUid()<<") to "<<nextHop);

			prepareRouteandSend(p, header, ucb, nextHop);

			if (fromPacketList) {
				packetFwd = true;
			}
		}
		return true;
	}

	//if (nextV.getAddress() == "0.0.0.0"
	//		|| nextV.getRsuNext() == "192.168.254.254"
	//	|| nextV.getAddress() == getLastHopIp(p)) {
	//	if (!fromPacketList) {
	//		PendingPacketsEntry en(p, header, ucb, ecb, Simulator::Now(),
	//				getLastHopIp(p));
	//		packetsList.packets.push_back(en);
	//	}
	//	return true; // carry packet

	return false;
}

bool IADD::isCachedBefore(Ptr<const Packet> p) {
	IADDInterestHeader th = getInterestHeader(p);

	return th.getIsCachedBefore();
}
void IADD::setCachedBefore(Ptr<Packet> p, bool isUDP) {
	IADDInterestHeader interest;
	IADDTypeHeader typeH;
	UdpHeader u;
	if (isUDP)
		p->RemoveHeader(u);
	p->RemoveHeader(typeH);
	p->RemoveHeader(interest);
	interest.setIsCachedBefore(1);
	p->AddHeader(interest);
	p->AddHeader(typeH);
	if (isUDP)

		p->AddHeader(u);
}
double IADD::getHitRatio() {
	return datacache.getHitRatio();
}
string IADD::getStats() {
	stringstream ss;

	ss << "Node ID:" << getNodeId() << ", Interest Packets: "
			<< getNumberofIntPackets() << ", Data Packets: "
			<< getNumberofDataPackets() << ", Stop Counter: " << stopCounter;

	return ss.str();
}

Ptr<Packet> IADD::createNewPacketFrom(Ptr<const Packet> p) {
	Ptr<Packet> packet = p->Copy();

	UdpHeader u;
	IADDTypeHeader th;
	IADDInterestHeader ih;

	packet->RemoveHeader(u);
	packet->RemoveHeader(th);
	packet->RemoveHeader(ih);

	Ptr<Packet> newPacket = Create<Packet>();

	newPacket->AddHeader(ih);
	newPacket->AddHeader(th);
	newPacket->AddHeader(u);

	return newPacket;
}
uint32_t IADD::getCacheSize() {
	return datacache.getCacheSize();
}

string IADD::getPacetInfo(Ptr<const Packet> p) {
	IADDInterestHeader th = getInterestHeader(p);
	stringstream ss;
	ss << "(" << p->GetUid() << "," << th.getReqX() << "," << th.getReqY()
			<< "," << th.getCenterX() << "," << th.getCenterY() << ","
			<< th.getRange() << "," << th.getInterestType() << ")";
	return ss.str();
}
//void IADD::SetPacketTrace(vector<uint32_t,uint32_t> p){
//	packetTrace = p;
//}

map<Ipv4Address, uint32_t> IADD::getPendingPacketsDes() {

	map<Ipv4Address, uint32_t> dest;
	for (list<PendingPacketsEntry>::iterator i = packetsList.packets.begin();
			i != packetsList.packets.end(); ++i) {
		dest[i->getHeader().GetDestination()]++;
		if (getPacketType(i->getPacket()) == IADD_INTEREST)
			tracer->packetStatus[i->getPacket()->GetUid()] = "Interest_pending";
		else
			tracer->packetStatus[i->getPacket()->GetUid()] = "Data_pending";

	}
	return dest;
}
void IADD::setTracer(Ptr<IADDPacketTracer> t) {
	tracer = t;
}

//void IADD::WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
//   {
//     while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
//       {
//         uint32_t left = totalTxBytes - currentTxBytes;
//         uint32_t dataOffset = currentTxBytes % writeSize;
//         uint32_t toWrite = writeSize - dataOffset;
//         toWrite = std::min (toWrite, left);
//         toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
//         int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
//         if(amountSent < 0)
//           {
//             // we will be called again when new tx space becomes available.
//             return;
//      }
//  	  	  currentTxBytes += amountSent;
//      }
//  	  localSocket->Close ();
//   }

}

