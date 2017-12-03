/*
 * GPS.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#include "GPS.h"
#include "math.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "constants.h"


namespace ns3 {

Vector GPS::getCurrentPosition(Ipv4Address address) {
	uint32_t n = NodeList().GetNNodes();
	uint32_t i;
	Ptr<Node> node;

	//NS_LOG_UNCOND("Position of " << adr);

	for (i = 0; i < n; i++) {
		node = NodeList().GetNode(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();

		//NS_LOG_UNCOND("Have " << ipv4->GetAddress (1, 0).GetLocal ());
		if (ipv4->GetAddress(1, 0).GetLocal() == address) {
			return (*node->GetObject<MobilityModel>()). GetPosition();
		}
	}
	Vector v;
	return v;
}

uint32_t GPS::getDirection(Vector heading) {
	uint32_t result;
	double angle = atan2(heading.y,heading.x)*180/3.14159265;
	if ((angle<=45)&&(angle>-45))
		result = EAST;
	else if ((angle<=135)&&(angle>45))
		result = NORTH;
	else if ((angle<=-45)&&(angle>-135))
		result = SOUTH;
	else
		result = WEST;

	return result;
}

uint32_t GPS::getDirection(Vector myPos, Vector theirPos) {
	Vector delta(theirPos.x-myPos.x, theirPos.y-myPos.y,0);
	return getDirection(delta);
}
bool GPS::isVehicleHeadingtoDst(Vector velocity, Vector myPos, Vector dstLoc) {

	double a1 = calculateAngle(velocity);
	double a2 = calculateAngle(myPos,dstLoc);

	double diff = a1-a2;
	if (diff < 0)
		diff += 360;
	if (diff > 180)
		diff = 360 - diff;

	if(diff<=90)
		return true;
	return false;
	/*if(getDirection(myHeading)==getDirection(myPos,dstLoc))
		return true;
	else
		return false;
		*/
}
bool GPS::isVehicleHeadingtoDst(double heading, Vector myPos, Vector dstLoc) {
	double a1 = heading;
	double a2 = calculateAngle(myPos,dstLoc);

	double diff = a1-a2;
	if (diff < 0)
		diff += 360;
	if (diff > 180)
		diff = 360 - diff;

	if(diff<=90)
		return true;
	return false;
	/*if(getDirection(myHeading)==getDirection(myPos,dstLoc))
		return true;
	else
		return false;
		*/
}
double GPS::calculateAngle(Vector velocity) {
	double ar = atan2(velocity.y, velocity.x);
	double a = ar * 180 / M_PI;
	if (a < 0)
		a += 360;

	return a;
}
double GPS::calculateAngle(Vector oldP, Vector newP) {
	double ar = atan2((newP.y - oldP.y), (newP.x - oldP.x));
	double a = ar * 180 / M_PI;
	if (a < 0)
		a += 360;

	return a;
}
bool GPS::isTheSameHeadig(double a , double b){

	double diff = a-b;
		if (diff < 0)
			diff += 360;
		if (diff > 180)
			diff = 360 - diff;

		if(diff<=HEADING_ANGLE_THRESHOLD)
			return true;
		return false;
}
}
