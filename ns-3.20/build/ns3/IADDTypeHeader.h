/*
 * IADDTypeHeader.h
 *
 *  Created on: Nov 20, 2013
 *      Author: root
 */

#ifndef IADDTYPEHEADER_H_
#define IADDTYPEHEADER_H_
using namespace std;
#include "ns3/header.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "Segment.h"
#include "constants.h"

#include <vector>
namespace ns3 {




class IADDTypeHeader: public Header {
public:
	IADDTypeHeader(MessageType t);
	IADDTypeHeader() :
			type(IADD_INTEREST), valid(false) {
	}
	;
	uint32_t GetSerializedSize() const;
	void Serialize(Buffer::Iterator i) const;
	uint32_t Deserialize(Buffer::Iterator i);
	void Print(std::ostream &os) const;
	TypeId GetInstanceTypeId() const;
	static TypeId GetTypeId();
	bool isValid();
	MessageType getType() const;
	void setType(MessageType type);
	string getTypeString();

private:
	MessageType type;
	bool valid;
};

class IADDVBeaconHeader: public Header {
public:
	IADDVBeaconHeader() :
			positionX(0), positionY(0), speed(0), heading(0) {
	}
	;
	IADDVBeaconHeader(double x, double y, double s, double h) :
			positionX(x), positionY(y), speed(s), heading(h) {
	}
	;
	IADDVBeaconHeader(double x, double y, Ipv4Address rsu) :
			positionX(x), positionY(y), RSUNext(rsu), speed(0), heading(0) {
	}
	;

	uint32_t GetSerializedSize() const;
	void Serialize(Buffer::Iterator i) const;
	uint32_t Deserialize(Buffer::Iterator i);
	void Print(std::ostream &os) const;
	double getHeading() const;
	void setHeading(double heading);
	double getPositionX() const;
	void setPositionX(double positionX);
	double getPositionY() const;
	void setPositionY(double positionY);
	double getSpeed() const;
	void setSpeed(double speed);
	TypeId GetInstanceTypeId() const;
	static TypeId GetTypeId();

	const Ipv4Address& getRsuNext() const {
		return RSUNext;
	}

	void setRsuNext(const Ipv4Address& rsuNext) {
		RSUNext = rsuNext;
	}

private:
	double positionX;
	double positionY;
	Ipv4Address RSUNext;
	double speed;
	double heading;

};
class IADDRBeaconHeader: public Header {
public:
	IADDRBeaconHeader() :
			positionX(0), positionY(0),centrality(0)  {
		for (uint32_t i; i < MAX_SEGMENTS; i++) {
			Ptr<Segment> s = CreateObject<Segment>();
			rsuNexts.push_back(s);
		}

	}
	;
	IADDRBeaconHeader(double x, double y,uint32_t c) :
			positionX(x), positionY(y),centrality(c) {
		for (uint32_t i = 0; i < MAX_SEGMENTS; i++) {
			Ptr<Segment> s = CreateObject<Segment>();
			rsuNexts.push_back(s);
		}
	}
	;
	uint32_t GetSerializedSize() const;
	void Serialize(Buffer::Iterator i) const;
	uint32_t Deserialize(Buffer::Iterator i);
	void Print(std::ostream &os) const;
	double getPositionX() const;
	void setPositionX(double positionX);
	double getPositionY() const;
	void setPositionY(double positionY);
	TypeId GetInstanceTypeId() const;
	static TypeId GetTypeId();
	Ptr<Segment> getRsuNexts(uint32_t index);
	void setRSUNext(uint32_t index, Ptr<Segment> &s);
	uint32_t getCentrality() const;
	void setCentrality(uint32_t centrality);

private:
	double positionX;
	double positionY;
	uint32_t centrality;
	vector<Ptr<Segment> > rsuNexts;
};
// TODODone Now6: add time to the header
class IADDInterestHeader: public Header {
private:
	double reqX;
	double reqY ;
	double centerX ;
	double centerY ;
	double range ;
	bool fwdFlag ;
	bool cacheFlag ;
	Time timeGenerated;
	Time ttl;
	IADDInterestTypes interestType;
	Ipv4Address RCSmax;
	uint32_t RCSmaxCentrality ;
	double RCSmaxX ;
	double RCSmaxY ;
	Ipv4Address RCSmax2;
	uint32_t RCSmax2Centrality ;
	Ipv4Address lastHopIP;
	bool isCachedBefore;
	uint64_t interestUID;

public:

	 IADDInterestHeader(double rX, double rY, double x, double y, double r,
	 bool f, bool c, Time t, IADDInterestTypes it) :
	 reqX(rX), reqY(rY), centerX(x), centerY(y), range(r), fwdFlag(f), cacheFlag(
	 c), timeGenerated(t), interestType(it),RCSmaxCentrality(0),RCSmaxX(0),RCSmaxY(0),RCSmax2Centrality(0){
		 isCachedBefore = false;
		 interestUID=0;}
	IADDInterestHeader() :
			reqX(0), reqY(0), centerX(0), centerY(0), range(0), fwdFlag(0), cacheFlag(
					0), timeGenerated(0), interestType(TYPES_COUNT),RCSmaxCentrality(0),RCSmaxX(0),RCSmaxY(0),RCSmax2Centrality(0) {
		 isCachedBefore = false;interestUID=0;}
	TypeId GetInstanceTypeId() const;
	static TypeId GetTypeId();
	uint32_t GetSerializedSize() const;
	void Serialize(Buffer::Iterator i) const;
	uint32_t Deserialize(Buffer::Iterator i);
	void Print(std::ostream &os) const;
	bool isMatchtoInterest(IADDInterestHeader r);
	double distance(Vector v1, Vector v2);

	double getCenterX() const;
	void setCenterX(double centerX);
	double getCenterY() const;
	void setCenterY(double centerY);
	bool isFwdFlag() const;
	void setFwdFlag(bool fwdFlag);
	double getRange() const;
	void setRange(double range);
	double getReqX() const;
	void setReqX(double reqX);
	double getReqY() const;
	void setReqY(double reqY);
	const Time& getTimeGenerated() const;
	void setTimeGenerated(const Time& timeGenerated);
	const Time& getTtl() const;
	void setTtl(const Time& ttl);
	const Ipv4Address& getRcSmax() const;
	void setRcSmax(const Ipv4Address& rcSmax);
	const Ipv4Address& getRcSmax2() const;
	void setRcSmax2(const Ipv4Address& rcSmax2);
	uint32_t getRcSmax2Centrality() const;
	void setRcSmax2Centrality(uint32_t rcSmax2Centrality);
	uint32_t getRcSmaxCentrality() const;
	void setRcSmaxCentrality(uint32_t rcSmaxCentrality);
	double getRcSmaxX() const;
	void setRcSmaxX(double rcSmaxX);
	double getRcSmaxY() const;
	void setRcSmaxY(double rcSmaxY);
	bool isCacheFlag() const;
	void setCacheFlag(bool cacheFlag);
	IADDInterestTypes getInterestType() const;
	void setInterestType(IADDInterestTypes interestType);
	const Ipv4Address& getLastHopIp() const;
	void setLastHopIp(const Ipv4Address& lastHopIp);
	bool getIsCachedBefore() const;
	void setIsCachedBefore(bool isCachedBefore);
	uint64_t getInterestUid() const;
	void setInterestUid(uint64_t interestUid);
};

} /* namespace ns3 */

#endif /* IADDTYPEHEADER_H_ */
