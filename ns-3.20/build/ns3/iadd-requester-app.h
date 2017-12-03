/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IADD_REQUESTER_APP_H
#define IADD_REQUESTER_APP_H

#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"
#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/iadd-routing-protocol.h"
#include "ns3/IADDTypeHeader.h"
#include "ns3/mobility-module.h"
#include "ns3/constants.h"
#include "ns3/time-data-calculators.h"
#include "ns3/IADDPacketTracer.h"

using namespace std;
namespace ns3 {

struct Interest {
	Interest(double x, double y, double r,	IADDInterestTypes ty, uint64_t id) :
			centerX(x), centerY(y), range(r),type(ty),uid(id) {

	}
	;
	double centerX;
	double centerY;
	double range;
	IADDInterestTypes type;
	uint64_t uid;
	inline bool operator==(Interest a) {
		if (a.centerX != centerX)
			return false;
		if (a.centerY != centerY)
			return false;
		if (a.range != range)
			return false;
		if(a.type != type)
			return false;
		if(a.uid != uid)
					return false;
		return true;
	}
};
struct Data {
	Data(double x, double y, Time t,	IADDInterestTypes ty,uint64_t id) :
			generatorX(x), generatorY(y), time(t),type(ty),uid(id) {
	}
	;
	double generatorX;
	double generatorY;
	Time time;
	IADDInterestTypes type;
	uint64_t uid;

};

class InterestTableEntry {
public:
	InterestTableEntry(Interest i) :
			interest(i), interestAdded(Simulator::Now()), dataGenerated(0), dataReceived(
					0) {
	}
	Interest interest;
	Time interestAdded;
	Time dataGenerated;
	Time dataReceived;
};

class InterestTable {
public:
	list<InterestTableEntry> table;

	void addInterest(Interest i);
	uint64_t dataReceived(Data d, Ptr<TimeMinMaxAvgTotalCalculator> e);
	double distance(Vector v1, Vector v2);

};

class IADDApplication: public Application {
public:
	static TypeId GetTypeId(void);

	IADDApplication();
	~IADDApplication();

	uint32_t getNumberofInterestsSent();
	uint32_t getNumberofDataReceived();
	double getAvgTime();
	Time getMaxTime();
	Time getMinTime();

	void setCalculators(Ptr<TimeMinMaxAvgTotalCalculator> a , Ptr<CounterCalculator<> > b , Ptr<CounterCalculator<> > c, Ptr<CounterCalculator<> > d);

	bool isVehicle(Ipv4Address a);
	void setTracer(Ptr<IADDPacketTracer> tracer);

private:
	IADDInterestTypes  getRandomType();

	virtual void StartApplication(void);
	virtual void StopApplication(void);

	void SendInterest();
	void RecvPacket(Ptr<Socket> socket);
Vector getRandomPointOnGrid();
double getRandomRange();
	void CancelTimers();
	InterestTable table;
	Ptr<TimeMinMaxAvgTotalCalculator> elapsedTime;
	Ptr<CounterCalculator<> > packetsArrived;
	Ptr<CounterCalculator<> > packetsFromVehicles;
	Ptr<CounterCalculator<> > numberofInterstGenerated;
	Ptr<IADDPacketTracer> tracer;
private:

	Time m_interval;
	Timer m_transmitTimer;

	//uint32_t m_packetNum;
uint32_t interestRate;
Time interestGenerationStart;
Time interestGenerationEnd;
	Ipv4Address m_myAddress;
	Ptr<Socket> m_socket;
	//vector<uint32_t,uint32_t> packetTrace;

	double gridMin;
	double gridMax;

	//void SetPacketTrace(vector<uint32_t,uint32_t>p);
};
/* ... */

}

#endif /* IADD_REQUESTER_APP_H */

