/*
 * IADDTypeHeader.cc
 *
 *  Created on: Nov 20, 2013
 *      Author: root
 */

#include "IADDTypeHeader.h"
#include "ns3/network-module.h"

namespace ns3 {

IADDTypeHeader::IADDTypeHeader(MessageType t) {
	type = t;
	valid = true;

}
string IADDTypeHeader::getTypeString() {
	string s;
	switch (type) {
	case IADD_VBEACON:
		s = "Vehicle Beacon";
		break;
	case IADD_RBEACON:
		s = "RSU Beacon";
		break;
	case IADD_DATA:
		s = "Data";
		break;
	case IADD_INTEREST:
		s = "Interest";
		break;
	default:
		s = "UNKNOWN TYPE";
	}
	return s;

}

uint32_t IADDTypeHeader::GetSerializedSize() const {
	return 1;
}

void IADDTypeHeader::Serialize(Buffer::Iterator i) const {
	i.WriteU8((uint8_t) type);
}

uint32_t IADDTypeHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	valid = true;
	uint8_t type2 = i.ReadU8();
	switch (type2) {
	case IADD_VBEACON:
	case IADD_RBEACON:
	case IADD_INTEREST:
	case IADD_DATA: {
		type = (MessageType) type2;
		break;
	}
	default:
		valid = false;

	}
	uint32_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	//if(dist == GetSerializedSize () )
	//valid = true;
	return dist;
}

void IADDTypeHeader::Print(std::ostream& os) const {
	switch (type) {
	case IADD_VBEACON:
		os << "Vehicle Beacon Header";
		break;
	case IADD_RBEACON:
		os << "RSU Beacon Header";
		break;
	case IADD_DATA:
		os << "Data Header";
		break;
	case IADD_INTEREST:
		os << "Interest Header";
		break;
	default:
		os << "UNKNOWN TYPE";
	}

}

uint32_t IADDVBeaconHeader::GetSerializedSize() const {
	return 28;
}

void IADDVBeaconHeader::Serialize(Buffer::Iterator i) const {
	i.WriteHtonU64((uint64_t) (positionX * 1000000));
	i.WriteHtonU64((uint64_t) (positionY * 1000000));
	//i.WriteHtonU64((uint64_t) speed);
	i.WriteHtonU64((uint64_t) (heading* 1000000));
	i.WriteHtonU32(RSUNext.Get());

}

uint32_t IADDVBeaconHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	positionX = ((double) i.ReadNtohU64()) / 1000000.0;
	positionY = ((double) i.ReadNtohU64()) / 1000000.0;
	//speed = (double) i.ReadNtohU64();
	heading = ((double) i.ReadNtohU64())/1000000.0;
	RSUNext.Set((uint32_t) i.ReadNtohU32());

	uint32_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;

}

void IADDVBeaconHeader::Print(std::ostream& os) const {
	os << "Position (" << positionX << "," << positionY << ") , Speed = "
			<< speed << ", heading = " << heading;
}

uint32_t IADDRBeaconHeader::GetSerializedSize() const {
	return 20 + (4 + 8 + 8 + 8 + 4) * MAX_SEGMENTS;
}

void IADDRBeaconHeader::Serialize(Buffer::Iterator i) const {
	i.WriteHtonU64((uint64_t) (positionX * 1000000));
	i.WriteHtonU64((uint64_t) (positionY * 1000000));
	i.WriteHtonU32(centrality);
	for (int j = 0; j < MAX_SEGMENTS; j++) {
		if (rsuNexts[j]->rsuNext.GetReferenceCount() != 0) {
			i.WriteHtonU32(rsuNexts[j]->rsuNext.getAddress().Get());
			i.WriteHtonU64(
					(uint64_t) (rsuNexts[j]->rsuNext.getPosition().x)
							* 1000000);
			i.WriteHtonU64(
					(uint64_t) (rsuNexts[j]->rsuNext.getPosition().y)
							* 1000000);
			i.WriteHtonU64((uint64_t) (rsuNexts[j]->density) * 1000000);
			i.WriteHtonU32(rsuNexts[j]->segmentDirection);
		} else {
			i.WriteHtonU32(0);
			i.WriteHtonU64(0);
			i.WriteHtonU64(0);
			i.WriteHtonU64(0);
			i.WriteHtonU32(0);
		}
	}
}

uint32_t IADDRBeaconHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	positionX = ((double) i.ReadNtohU64()) / 1000000.0;
	positionY = ((double) i.ReadNtohU64()) / 1000000.0;
	centrality = i.ReadNtohU32();
	for (int j = 0; j < MAX_SEGMENTS; j++) {
		rsuNexts[j]->rsuNext.setAddress(i.ReadNtohU32());
		double x = ((double) i.ReadNtohU64()) / 1000000.0;
		double y = ((double) i.ReadNtohU64()) / 1000000.0;
		Vector v(x, y, 0);
		rsuNexts[j]->rsuNext.setPosition(v);
		rsuNexts[j]->density = ((double) i.ReadNtohU64()) / 1000000.0;
		rsuNexts[j]->segmentDirection = i.ReadNtohU32();
	}

	uint32_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

void IADDRBeaconHeader::Print(std::ostream& os) const {
	os << " Position (" << positionX << "," << positionY << ")";
}

double IADDVBeaconHeader::getHeading() const {
	return heading;
}

void IADDVBeaconHeader::setHeading(double heading) {
	this->heading = heading;
}

double IADDVBeaconHeader::getPositionX() const {
	return positionX;
}

void IADDVBeaconHeader::setPositionX(double positionX) {
	this->positionX = positionX;
}

double IADDVBeaconHeader::getPositionY() const {
	return positionY;
}

void IADDVBeaconHeader::setPositionY(double positionY) {
	this->positionY = positionY;
}

double IADDVBeaconHeader::getSpeed() const {
	return speed;
}

void IADDVBeaconHeader::setSpeed(double speed) {
	this->speed = speed;
}

double IADDRBeaconHeader::getPositionX() const {
	return positionX;
}

void IADDRBeaconHeader::setPositionX(double positionX) {
	this->positionX = positionX;
}

double IADDRBeaconHeader::getPositionY() const {
	return positionY;
}

void IADDRBeaconHeader::setPositionY(double positionY) {
	this->positionY = positionY;
}

TypeId IADDTypeHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

TypeId IADDTypeHeader::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::iadd-routing-protocol::IADDTypeHeader").SetParent<
					Header>().AddConstructor<IADDTypeHeader>();
	return tid;
}

TypeId IADDVBeaconHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

TypeId IADDVBeaconHeader::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::iadd-routing-protocol::IADDVBeaconHeader").SetParent<
					Header>().AddConstructor<IADDVBeaconHeader>();
	return tid;
}

TypeId IADDRBeaconHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

TypeId IADDRBeaconHeader::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::iadd-routing-protocol::IADDRBeaconHeader").SetParent<
					Header>().AddConstructor<IADDRBeaconHeader>();
	return tid;
}

MessageType IADDTypeHeader::getType() const {
	return type;
}

void IADDTypeHeader::setType(MessageType type) {
	this->type = type;
}

double IADDInterestHeader::getCenterX() const {
	return centerX;
}

void IADDInterestHeader::setCenterX(double centerX) {
	this->centerX = centerX;
}

double IADDInterestHeader::getCenterY() const {
	return centerY;
}

void IADDInterestHeader::setCenterY(double centerY) {
	this->centerY = centerY;
}

bool IADDInterestHeader::isFwdFlag() const {
	return fwdFlag;
}

void IADDInterestHeader::setFwdFlag(bool fwdFlag) {
	this->fwdFlag = fwdFlag;
}

double IADDInterestHeader::getRange() const {
	return range;
}

void IADDInterestHeader::setRange(double range) {
	this->range = range;
}

TypeId IADDInterestHeader::GetInstanceTypeId() const {
	return GetTypeId();

}

TypeId IADDInterestHeader::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::iadd-routing-protocol::IADDInterestHeader").SetParent<
					Header>().AddConstructor<IADDInterestHeader>();
	return tid;
}

uint32_t IADDInterestHeader::GetSerializedSize() const {
	return 104;
}

void IADDInterestHeader::Serialize(Buffer::Iterator i) const {
	i.WriteHtonU64((uint64_t) (reqX * 1000000));
	i.WriteHtonU64((uint64_t) (reqY * 1000000));
	i.WriteHtonU64((uint64_t) (centerX * 1000000));
	i.WriteHtonU64((uint64_t) (centerY * 1000000));
	i.WriteHtonU64((uint64_t) (range * 1000000));

	i.WriteHtonU64((uint64_t) timeGenerated.GetMilliSeconds());
	i.WriteHtonU64((uint64_t) ttl.GetMilliSeconds());

	i.WriteHtonU32(RCSmax.Get());
	i.WriteHtonU32(RCSmaxCentrality);
	i.WriteHtonU64((uint64_t) (RCSmaxX * 1000000));
	i.WriteHtonU64((uint64_t) (RCSmaxY * 1000000));

	i.WriteHtonU32(RCSmax2.Get());
	i.WriteHtonU32(RCSmax2Centrality);

	i.WriteU8((uint8_t) fwdFlag);
	i.WriteU8((uint8_t) cacheFlag);

	i.WriteHtonU32(lastHopIP.Get());

	i.WriteU8( (uint8_t)isCachedBefore);
	i.WriteU8( (uint8_t)interestType);
	i.WriteHtonU64(interestUID);

}

uint32_t IADDInterestHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	reqX = ((double) i.ReadNtohU64()) / 1000000.0;
	reqY = ((double) i.ReadNtohU64()) / 1000000.0;
	centerX = ((double) i.ReadNtohU64()) / 1000000.0;
	centerY = ((double) i.ReadNtohU64()) / 1000000.0;
	range = ((double) i.ReadNtohU64()) / 1000000.0;

	timeGenerated = MilliSeconds(i.ReadNtohU64());
	ttl = MilliSeconds(i.ReadNtohU64());

	RCSmax.Set(i.ReadNtohU32());
	RCSmaxCentrality = i.ReadNtohU32();
	RCSmaxX = ((double) i.ReadNtohU64()) / 1000000.0;
	RCSmaxY = ((double) i.ReadNtohU64()) / 1000000.0;

	RCSmax2.Set(i.ReadNtohU32());
	RCSmax2Centrality = i.ReadNtohU32();

	fwdFlag = (bool) i.ReadU8();
	cacheFlag = (bool) i.ReadU8();

	lastHopIP.Set(i.ReadNtohU32());
	isCachedBefore = (bool) i.ReadU8();

	interestType = (IADDInterestTypes)i.ReadU8();

	interestUID = i.ReadNtohU64();

	uint32_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetSerializedSize());

	return dist;

}

double IADDInterestHeader::getReqX() const {
	return reqX;
}

void IADDInterestHeader::setReqX(double reqX) {
	this->reqX = reqX;
}

double IADDInterestHeader::getReqY() const {
	return reqY;
}

void IADDInterestHeader::setReqY(double reqY) {
	this->reqY = reqY;
}

void IADDInterestHeader::Print(std::ostream& os) const {
	os << "Center=(" << centerX << "," << centerY << "), Range=" << range
			<< ", FWDFlag = " << fwdFlag;
}

Ptr<Segment> IADDRBeaconHeader::getRsuNexts(uint32_t index) {
	return rsuNexts[index];
}

void IADDRBeaconHeader::setRSUNext(uint32_t index, Ptr<Segment> &s) {
	rsuNexts[index] = CreateObject<Segment>(*s);
}

bool IADDInterestHeader::isMatchtoInterest(IADDInterestHeader interest) {
	/*if(getCenterX()!=r.getCenterX())
	 return false;
	 if(getCenterY()!=r.getCenterY())
	 return false;
	 if(getRange()()!=r.getRange()())
	 return false;
	 return true;*/

	double x = interest.getCenterX();
	double y = interest.getCenterY();
	double r = interest.getRange();
	Vector center(x, y, 0);

	double x2 = getCenterX();
	double y2 = getCenterY();

	Vector point(x2, y2, 0);
	double d = distance(point, center);
	if (d < r && interest.getInterestType()==getInterestType()) {
		return true;
	} else
		return false;
}
double IADDInterestHeader::distance(Vector v1, Vector v2) {
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
}

bool IADDTypeHeader::isValid() {
	return valid;
}
const Time& IADDInterestHeader::getTimeGenerated() const {
	return timeGenerated;
}

void IADDInterestHeader::setTimeGenerated(const Time& timeGenerated) {
	this->timeGenerated = timeGenerated;
}

const Time& IADDInterestHeader::getTtl() const {
	return ttl;
}

bool IADDInterestHeader::isCacheFlag() const {
	return cacheFlag;
}

void IADDInterestHeader::setCacheFlag(bool cacheFlag ) {
	this->cacheFlag = cacheFlag;
}

IADDInterestTypes IADDInterestHeader::getInterestType() const {
	return interestType;
}

const Ipv4Address& IADDInterestHeader::getLastHopIp() const {
	return lastHopIP;
}

void IADDInterestHeader::setLastHopIp(const Ipv4Address& lastHopIp) {
	lastHopIP = lastHopIp;
}

void IADDInterestHeader::setInterestType(IADDInterestTypes interestType) {
	this->interestType = interestType;
}

void IADDInterestHeader::setTtl(const Time& ttl) {
	this->ttl = ttl;
}

const Ipv4Address& IADDInterestHeader::getRcSmax() const {
	return RCSmax;
}

void IADDInterestHeader::setRcSmax(const Ipv4Address& rcSmax) {
	RCSmax = rcSmax;
}

const Ipv4Address& IADDInterestHeader::getRcSmax2() const {
	return RCSmax2;
}

void IADDInterestHeader::setRcSmax2(const Ipv4Address& rcSmax2) {
	RCSmax2 = rcSmax2;
}

uint32_t IADDInterestHeader::getRcSmax2Centrality() const {
	return RCSmax2Centrality;
}

void IADDInterestHeader::setRcSmax2Centrality(uint32_t rcSmax2Centrality) {
	RCSmax2Centrality = rcSmax2Centrality;
}

uint32_t IADDInterestHeader::getRcSmaxCentrality() const {
	return RCSmaxCentrality;
}

void IADDInterestHeader::setRcSmaxCentrality(uint32_t rcSmaxCentrality) {
	RCSmaxCentrality = rcSmaxCentrality;
}

double IADDInterestHeader::getRcSmaxX() const {
	return RCSmaxX;
}

void IADDInterestHeader::setRcSmaxX(double rcSmaxX) {
	RCSmaxX = rcSmaxX;
}

double IADDInterestHeader::getRcSmaxY() const {
	return RCSmaxY;
}

void IADDInterestHeader::setRcSmaxY(double rcSmaxY) {
	RCSmaxY = rcSmaxY;
}
/* namespace ns3 */

uint32_t IADDRBeaconHeader::getCentrality() const {
	return centrality;
}

void IADDRBeaconHeader::setCentrality(uint32_t centrality) {
	this->centrality = centrality;
}




bool IADDInterestHeader::getIsCachedBefore() const {
	return isCachedBefore;
}

void IADDInterestHeader::setIsCachedBefore(bool isCachedBefore) {
	this->isCachedBefore = isCachedBefore;
}


uint64_t IADDInterestHeader::getInterestUid() const {
	return interestUID;
}

void IADDInterestHeader::setInterestUid(uint64_t x) {
	interestUID = x;
}
}
