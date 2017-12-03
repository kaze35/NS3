/*
 * Vehicle.h
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#ifndef VEHICLE_H_
#define VEHICLE_H_
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "constants.h"

namespace ns3 {

class Vehicle: public SimpleRefCount<Vehicle> {
//class Vehicle: public Node {

public:
	/*Vehicle(Ipv4Address ip, Mac48Address mac, Time t, Vector p, double h,
	 double s) :
	 address(ip), hwAddress(mac),  position(p), heading(h), speed(
	 s),expiryTime(t) {
	 }*/
	Vehicle(Ipv4Address ip, Vector p, double h, double s) :
			address(ip), position(p), heading(h), speed(s) {
	}
	Vehicle(Ipv4Address ip, Vector p, Ipv4Address rsu) :
			address(ip), position(p), rsuNext(rsu), heading(0), speed(0) {
	}
	Vehicle() :
		address("0.0.0.0"), position(0,0,0),heading(0), speed(0) {
	}

	const Ipv4Address& getAddress() const;
	void setAddress(const Ipv4Address& address);
	const Time& getExpiryTime() const;
	void setExpiryTime(const Time& expiryTime);
	double getHeading() const;
	void setHeading(double heading);
	const Mac48Address& getHwAddress() const;
	void setHwAddress(const Mac48Address& hwAddress);
	const Vector& getPosition() const;
	void setPosition(const Vector& position);
	double getSpeed() const;
	void setSpeed(double speed);
	const Ipv4Address& getRsuNext() const;
	void setRsuNext(const Ipv4Address& rsuNext);
	Vehicle& operator=(const Vehicle& v) {
		address = v.address;
		position = v.position;
		rsuNext = v.rsuNext;
		heading = v.heading; // angle
		speed = v.speed; //m/h
		hwAddress = v.hwAddress;
		expiryTime = v.expiryTime;
		return *this;
	}
private:
	Ipv4Address address;
	Vector position;
	Ipv4Address rsuNext;
	double heading; // angle
	double speed; //m/h
	Mac48Address hwAddress;
	Time expiryTime;
};

class RSU: public Object {
//class RSU: public Node {

private:
	uint32_t RSU_ID;
	Vector position;
	Ipv4Address address;
	Mac48Address hwAddress;
	Time expiryTime;
	uint32_t centrality;
	//Ptr<Node> node;
public:
	RSU(uint32_t id, Ipv4Address add, Mac48Address hw, Vector pos, Time t,
			uint32_t c) :
			RSU_ID(id), position(pos), address(add), hwAddress(hw), expiryTime(
					t), centrality(c) {
	}
	RSU(const RSU &r) :
			RSU_ID(r.RSU_ID), position(r.position), address(r.address), hwAddress(
					r.hwAddress), expiryTime(r.expiryTime), centrality(
					r.centrality) {
	}
	RSU(Ipv4Address add, Vector pos) :
			RSU_ID(-1), position(pos), address(add), centrality(0) {
	}
	RSU(uint32_t ID, Ipv4Address add, Vector pos) :
			RSU_ID(ID), position(pos), address(add), centrality(0) {
	}
	RSU();

	const Ipv4Address& getAddress() const;
	void setAddress(const Ipv4Address& address);
	void setAddress(uint32_t address);

	const Time& getExpiryTime() const;
	void setExpiryTime(const Time& expiryTime);
	const Mac48Address& getHwAddress() const;
	void setHwAddress(const Mac48Address& hwAddress);
	const Vector& getPosition() const;
	void setPosition(const Vector& position);
	uint32_t getRsuId() const;
	void setRsuId(uint32_t rsuId);

	RSU& operator=(const RSU& r) {
		RSU_ID = (r.RSU_ID);
		position = (r.position);
		address = (r.address);
		hwAddress = (r.hwAddress);
		expiryTime = (r.expiryTime);
		return *this;
	}

	uint32_t getCentrality() const;
	void setCentrality(uint32_t centrality = 0);
};
}
#endif /* VEHICLE_H_ */

