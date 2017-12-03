/*
 * NeighborList.h
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#ifndef NEIGHBORLIST_H_
#define NEIGHBORLIST_H_
using namespace std;
#include "Vehicle.h"
#include "constants.h"
#include "GPS.h"

namespace ns3 {
class NeighborList {
public:
	NeighborList();
	NeighborList(Time d);
	void updateNeighbor(Vehicle v,Time expire);
	void updateRSU(RSU r, Time expire);
	void removeExpiredNeighbors();
	list<Vehicle> getVehiclesInSegment(uint32_t segmentId, Vector myPo);
	Ptr<Vehicle> getFurthestNeighborinSegment(uint32_t segmentId, Vector myPo);
	list<Vehicle> getVehiclesWithRSUNext(RSU rsuNext, Vector myPo);
	Vehicle getFurthestNeighborwithRSUNext(RSU rsuNext, Vector myPos);
	Vehicle getFurthestNeighborwithRSUNext(RSU rsuNext,
			RSU myRSU, Vector myPos, bool& x);
	Vehicle getClosestNeighbortoRange(Vector pos, Vector myPos);
	Vehicle getFurthestNeighborHeadingtoDst(Vector dst, Vector myPos,double myHeading, Ptr<GPS> gps);
	Vehicle getFurthestNeighborV(Vector mypos);
	Ptr<RSU> getFurthestNeighborR(Vector mypos);

	bool isNeighbor(Ipv4Address address,Vector pos);
	void removeAll();
	Ptr<RSU> getClosestRSU(Vector myPos);
	 double distance(Vector v1, Vector v2);
	 void start();
	 void printList(std::ostringstream &os);

	 Vehicle getTheBestVehicleSCF(Vector dst);


	list<Vehicle> neighbors;
	Time delay;
	Timer timer;
	list<RSU> rsu;

};
}
#endif /* NEIGHBORLIST_H_ */
