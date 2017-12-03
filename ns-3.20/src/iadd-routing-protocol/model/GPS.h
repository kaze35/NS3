/*
 * GPS.h
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#ifndef GPS_H_
#define GPS_H_
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "constants.h"

using namespace std;
namespace ns3 {


class GPS: public SimpleRefCount<GPS>{
public:
	//GPS();
	Vector getCurrentPosition(Ipv4Address id);
	uint32_t getDirection(Vector heading);
	uint32_t getDirection(Vector myPos, Vector theirPos);
	bool isVehicleHeadingtoDst(Vector myHeading, Vector myPos, Vector dstLoc);
	bool isVehicleHeadingtoDst(double heading, Vector myPos, Vector dstLoc);

	double calculateAngle(Vector velocity);
	double calculateAngle(Vector oldP, Vector newP);
	bool isTheSameHeadig(double a , double b);
};
}
#endif /* GPS_H_ */



