/*
 * IADDPacketTracer.h
 *
 *  Created on: Mar 20, 2014
 *      Author: hesham
 */

#ifndef IADDPACKETTRACER_H_
#define IADDPACKETTRACER_H_
#include <fstream>
#include <ctime>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "constants.h"

using namespace std;

namespace ns3 {

struct TxLog {
	TxLog(Ipv4Address i1, Ipv4Address i2, Time tt, Vector v, uint64_t id) :
			ip1(i1), ip2(i2), t(tt), positionOf1(v), packetUid(id) {
	}
	;
	Ipv4Address ip1;
	Ipv4Address ip2;
	Time t;
	Vector positionOf1;
	uint64_t packetUid;
};
struct MatchLog {
	MatchLog(Ipv4Address i1, Time tt, Vector v, uint64_t id, uint64_t id2) :
			ip(i1), t(tt), position(v), packetUid(id), newpacketUid(id2) {
	}
	;
	Ipv4Address ip;
	Time t;
	Vector position;
	uint64_t packetUid;
	uint64_t newpacketUid;
};

class IADDPacketTracer: public Object {
public:
	fstream fileTxLog;
	fstream fileMatchLog;
	fstream filePacketLog;

	map<uint64_t, string> packetStatus;

	IADDPacketTracer(string fname, string fname2, string fname3) {
		fileTxLog.open(fname.c_str(), fstream::out | fstream::trunc);
		fileMatchLog.open(fname2.c_str(), fstream::out | fstream::trunc);
		filePacketLog.open(fname3.c_str(), fstream::out | fstream::trunc);

	}
	~IADDPacketTracer() {
		closeFile();
	}
	void closeFile();
	void sendOperation(Ipv4Address from, Ipv4Address to, Time t, Vector p,
			uint64_t packetUID, MessageType packetType, Ipv4Address dst);
	void receiveOperation(Ipv4Address to, Ipv4Address from, Time t, Vector p,
			uint64_t packetUID);
	void dataCreated(Ipv4Address ip, Time t, Vector p, uint64_t intUID,
			uint64_t dataUID, Ipv4Address to);
	void replicaCreated(Ipv4Address ip, Time t, Vector p, uint64_t dataUID,
			uint64_t repUID, Ipv4Address to);
	void addInterest(double x, double y, double range, uint32_t ty, uint64_t id,
			Ipv4Address from);
	void addData(double x, double y, uint64_t id, Ipv4Address from);

};

}

#endif /* IADDPACKETTRACER_H_ */
