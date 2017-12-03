/*
 * PendingPackets.h
 *
 *  Created on: Nov 21, 2013
 *      Author: root
 */

#ifndef PENDINGPACKETS_H_
#define PENDINGPACKETS_H_
#include "ns3/internet-module.h"
#include "constants.h"

using namespace std;
namespace ns3 {

class PendingPacketsEntry {
public:
	typedef Ipv4RoutingProtocol::UnicastForwardCallback UnicastForwardCallback;
	typedef Ipv4RoutingProtocol::ErrorCallback ErrorCallback;

	PendingPacketsEntry(Ptr<const Packet> pa = 0, Ipv4Header const & h =
			Ipv4Header(), UnicastForwardCallback u = UnicastForwardCallback(),
			ErrorCallback e = ErrorCallback(), Time t = Simulator::Now(),
			Ipv4Address a = 0) :
			packet(pa), header(h), ucb(u), ecb(e), timeAdded(t), recivedFrom(a) {
	}

	const ErrorCallback& getEcb() const {
		return ecb;
	}

	void setEcb(const ErrorCallback& ecb) {
		this->ecb = ecb;
	}

	const Ipv4Header getHeader() const {
		return header;
	}

	const Ptr<const Packet>& getPacket() const {
		return packet;
	}

	void setPacket(const Ptr<Packet>& packet) {
		this->packet = packet;
	}

	const UnicastForwardCallback& getUcb() const {
		return ucb;
	}

	void setUcb(const UnicastForwardCallback& ucb) {
		this->ucb = ucb;
	}
	bool operator==(PendingPacketsEntry & b) {

		if (b.packet == packet)
			return true;

		return false;
	}

	const Ipv4Address& getRecivedFrom() const {
		return recivedFrom;
	}

	const Time& getTimeAdded() const {
		return timeAdded;
	}

private:
	Ptr<const Packet> packet;
	const Ipv4Header header;
	UnicastForwardCallback ucb;
	ErrorCallback ecb;
	Time timeAdded;
	Ipv4Address recivedFrom;

};

class PendingPackets {
public:

	list<PendingPacketsEntry> packets;

	void removePacket(Ptr<Packet> p) {
		for (list<PendingPacketsEntry>::iterator i = packets.begin();
				i != packets.end(); ++i) {
			if (i->getPacket() == p)
				packets.erase(i);
		}
	}
	void getInfoForPacket(Ptr<const Packet> p, Time &t, Ipv4Address &a) {
		/*if (p->GetUid() == 13314)
		{
			for (list<PendingPacketsEntry>::iterator i = packets.begin();
							i != packets.end(); ++i) {
				std::cout << "Packet "<<i->getPacket()->GetUid()<<" from "<< i->getRecivedFrom()<<"     ";
			}
			std::cout <<std::endl;
		}
		*/
		for (list<PendingPacketsEntry>::iterator i = packets.begin();
				i != packets.end(); ++i) {
			if (i->getPacket()->GetUid() == p->GetUid()) {
				t = i->getTimeAdded();
				a = i->getRecivedFrom();
				break;
			}
		}
	}

};

} /* namespace ns3 */

#endif /* PENDINGPACKETS_H_ */
