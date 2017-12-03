/*
 * Segment.h
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#ifndef SEGMENT_H_
#define SEGMENT_H_
#include "ns3/core-module.h"
#include "Vehicle.h"
#include "constants.h"

namespace ns3 {



class Segment :public Object{
public:
	Segment(RSU r,double d,uint32_t s):rsuNext(r),density(d),segmentDirection(s),priority(0){};
	Segment():density(0),segmentDirection(0),priority(0){};
	Segment(const Segment& s);
	RSU rsuNext;
	double density;
	uint32_t segmentDirection;
	double priority;

	bool operator< ( Segment & b);
	double getWeightedPriority();

	static TypeId GetTypeId();



};
}
#endif /* SEGMENT_H_ */
