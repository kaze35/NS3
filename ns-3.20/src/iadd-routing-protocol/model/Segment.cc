/*
 * Segment.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#include "Segment.h"
#include "constants.h"
namespace ns3{


bool Segment::operator<(Segment & b) {

	if( getWeightedPriority() < b.getWeightedPriority())
		return true;

	return false;
}

double Segment::getWeightedPriority(){
	return  priority * ((double)ALPHA)   + density * ((double)BETA) ;


}

Segment::Segment(const Segment& s) {
	density = s.density;
	priority = s.priority;
	rsuNext = s.rsuNext;
	segmentDirection = s.segmentDirection;
}

TypeId Segment::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::iadd-routing-protocol::Segment").SetParent<
					Object>().AddConstructor<Segment>();
	return tid;
}
}
