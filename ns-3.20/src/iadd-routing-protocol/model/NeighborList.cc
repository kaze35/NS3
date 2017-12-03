#include "NeighborList.h"
#include "cmath"
#include <float.h>
#include <string.h>
using namespace std;
namespace ns3 {
NeighborList::NeighborList() {
	NeighborList(MilliSeconds(200));
}

NeighborList::NeighborList(Time d) {
	delay = d;

}

Vehicle NeighborList::getClosestNeighbortoRange(Vector center,
		Vector myPos) {

	double min = DBL_MAX;
	Vehicle minV;
	for (list<Vehicle>::iterator v = neighbors.begin(); v != neighbors.end();
			++v) {
		if (distance(myPos, v->getPosition()) > MAX_TANS_RANGE)
			continue;
		double d = distance(v->getPosition(), center);
		if (d < min) {
			min = d;
			minV = (*v);
		}
	}

	return minV;
}

void NeighborList::start() {
	timer.Cancel();
	timer = Timer::CANCEL_ON_DESTROY;
	timer.SetDelay(delay);
	timer.SetFunction(&NeighborList::removeExpiredNeighbors, this);
}
void NeighborList::printList(std::ostringstream &os) {

	for (list<Vehicle>::iterator i = neighbors.begin(); i != neighbors.end();
			++i) {
		i->getAddress().Print(os);
		os << "(" << i->getRsuNext() << ") , ";
	}
}

void NeighborList::updateNeighbor(Vehicle v, Time expire) {
	for (list<Vehicle>::iterator i = neighbors.begin(); i != neighbors.end();
			++i) {
		if (v.getAddress() == i->getAddress()) {
			i->setPosition(v.getPosition());
			i->setSpeed(v.getSpeed());
			i->setHeading(v.getHeading());
			i->setRsuNext(v.getRsuNext());
			i->setExpiryTime(
					std::max(expire + Simulator::Now(), i->getExpiryTime()));
			return;
		}
	}
	v.setExpiryTime(expire + Simulator::Now());
	neighbors.push_back(v);
	removeExpiredNeighbors();

}
void NeighborList::updateRSU(RSU r, Time expire) {
	for (list<RSU>::iterator i = rsu.begin(); i != rsu.end(); ++i) {
		if (r.getAddress() == i->getAddress()) {
			i->setPosition(r.getPosition());

			i->setExpiryTime(
					std::max(expire + Simulator::Now(), i->getExpiryTime()));
			return;
		}
	}
	r.setExpiryTime(expire + Simulator::Now());
	rsu.push_back(r);
	removeExpiredNeighbors();

}
struct RemoveNeighbor //predicate
{
	bool operator()(const Vehicle & v) const {
		return ((v.getExpiryTime() < Simulator::Now()));
	}
	bool operator()(const RSU & r) const {
		return ((r.getExpiryTime() < Simulator::Now()));
	}
};
void NeighborList::removeExpiredNeighbors() {

	RemoveNeighbor pred;
	//int c = neighbors.size();
	//int c2 = rsu.size();
	//if(c2<1)
	//cout<<c<<c2;
	if (!neighbors.empty()) {
		neighbors.erase(remove_if(neighbors.begin(), neighbors.end(), pred),
				neighbors.end());
	}/*
	 cout<<rsu.size()<<endl;
	 for (list<RSU>::iterator i = rsu.begin(); i != rsu.end(); ++i) {
	 Time x = i->getExpiryTime();
	 Time y = Simulator::Now();
	 cout<<x.GetSeconds()<<"    "<<y.GetSeconds()<<endl;
	 }*/
	if (!rsu.empty()) {
		rsu.erase(remove_if(rsu.begin(), rsu.end(), pred), rsu.end());
	}
	/*cout<<rsu.size()<<endl;

	 for (list<RSU>::iterator i = rsu.begin(); i != rsu.end(); ++i) {
	 Time x = i->getExpiryTime();
	 Time y = Simulator::Now();
	 cout<<x.GetSeconds()<<"    "<<y.GetSeconds()<<endl;
	 }
	 cout<<"-------"<<endl;*/
	timer.Cancel();
	timer.Schedule();

}
//*** new Change 5th Sep add node position to avoid sending packets near the boundry
bool NeighborList::isNeighbor(Ipv4Address address,Vector pos) {
	for (list<Vehicle>::iterator i = neighbors.begin(); i != neighbors.end();
			++i) {
		if (address == i->getAddress() && distance(pos,i->getPosition())<MAX_TANS_RANGE)
			return true;
	}
	for (list<RSU>::iterator i = rsu.begin(); i != rsu.end();
			++i) {
		if (address == i->getAddress() && distance(pos,i->getPosition())<MAX_TANS_RANGE)
			return true;
	}
	return false;
}

void NeighborList::removeAll() {
	neighbors.clear();
	rsu.clear();
}

list<Vehicle> NeighborList::getVehiclesInSegment(uint32_t segmentId,
		Vector myPos) {
	list<Vehicle> vs;
	return vs;
}

//  getFurthestNeighborinSegment
Ptr<Vehicle> NeighborList::getFurthestNeighborinSegment(uint32_t segmentId,
		Vector myPos) {
	Ptr<Vehicle> v;
	return v;
}

list<Vehicle> NeighborList::getVehiclesWithRSUNext(RSU rsuNext, Vector myPos) {
	list<Vehicle> vs;

	for (list<Vehicle>::iterator v = neighbors.begin(); v != neighbors.end();
			++v) {

		//NS_ASSERT(rsuNext.getAddress() !="0.0.0.0");
		if (distance(myPos, v->getPosition()) > MAX_TANS_RANGE)
			continue;
		if (v->getRsuNext() == rsuNext.getAddress()
				&& rsuNext.getAddress() != "0.0.0.0")
			vs.push_back(*v);
	}
	return vs;
}
Vehicle NeighborList::getFurthestNeighborHeadingtoDst(Vector dst,
		Vector myPos, double myHeading, Ptr<GPS> gps) {

	double min = DBL_MAX;
	Vehicle minV;

	double myD = distance(myPos, dst);
	for (list<Vehicle>::iterator v = neighbors.begin(); v != neighbors.end();
			++v) {
		if (distance(myPos, v->getPosition()) > MAX_TANS_RANGE)
			continue;
		//if (gps->isVehicleHeadingtoDst(v->getHeading(), v->getPosition(),
		//		dst)) {
	//	if(gps->isTheSameHeadig(v->getHeading(),myHeading)){
			double d = distance(v->getPosition(), dst);
			if (d < min) {
				min = d;
				minV = (*v);
			}
		//}
	}
	Vehicle n;

	if (myD < min /* && gps->isVehicleHeadingtoDst(myHeading, myPos, dst)*/)
		return n;
	return minV;

}
Vehicle NeighborList::getFurthestNeighborwithRSUNext(RSU rsuNext,
		Vector myPos) {
	/*// test max range
	 for (list<RSU>::iterator i = rsu.begin(); i != rsu.end();
	 ++i) {
	 if ( distance(i->getPosition(),myPos) > 10){
	 double x = distance(i->getPosition(),myPos);
	 cout<<"WHAAAAAAT!!!!"<<x;
	 }
	 }*/
	list<Vehicle> vs = getVehiclesWithRSUNext(rsuNext, myPos);

	double min = DBL_MAX;
	Vehicle minV;

	double myD = distance(myPos, rsuNext.getPosition());
	for (list<Vehicle>::iterator v = vs.begin(); v != vs.end(); ++v) {
		double d = distance(v->getPosition(), rsuNext.getPosition());
		if (d < min) {
			min = d;
			minV = (*v);
		}
	}

	if (min < myD - 3)
		return minV;
	Vehicle n;
	return n;

}
Vehicle NeighborList::getFurthestNeighborwithRSUNext(RSU rsuNext,
		RSU myRSUNext, Vector myPos, bool &myVehicle) {
	list<Vehicle> vs = getVehiclesWithRSUNext(rsuNext, myPos);

	double min = DBL_MAX;
	Vehicle minV;
	Vehicle n;
	myVehicle = false;

	double myD = distance(myPos, rsuNext.getPosition());
	for (list<Vehicle>::iterator v = vs.begin(); v != vs.end(); ++v) {
		double d = distance(v->getPosition(), rsuNext.getPosition());
		if (d < min) {
			min = d;
			minV = (*v);
		}
	}
	if (rsuNext.getAddress() == myRSUNext.getAddress() && myD < min) {
		myVehicle = true;
		return n;
	}

	return minV;

}
/*
 Ptr<Vehicle> NeighborList::getFurthestNeighborwithRSUNext(RSU rsuNext, Vector myPos) {
 list<Vehicle> vs = getVehiclesWithRSUNext(rsuNext,myPos);
 double min = DBL_MAX;
 Ptr<Vehicle> minV;
 Ptr<Vehicle> n;
 for (list<Vehicle>::iterator v = vs.begin(); v != vs.end(); ++v) {
 double d = distance(v->getPosition(), rsuNext.getPosition());
 if (d < min) {
 min = d;
 minV = (&(*v));
 }
 }

 return minV;

 }*/
double NeighborList::distance(Vector v1, Vector v2) {
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
}

Ptr<RSU> NeighborList::getClosestRSU(Vector myPos) {
	double minD = 0;
	double x = myPos.x;
	double y = myPos.y;
	Ptr<RSU> minRSU;
	for (list<RSU>::iterator i = rsu.begin(); i != rsu.end(); ++i) {
		if (distance(myPos, i->getPosition()) > MAX_TANS_RANGE)
			continue;
		double c = 0;
		double r_x = i->getPosition().x;
		double r_y = i->getPosition().y;

		c = sqrt(pow(r_y - y, 2) + pow(r_x - x, 2));
		if (c < minD) {
			minD = c;
			minRSU = (&(*i));
		}

	}
	return minRSU;

}
Vehicle NeighborList::getTheBestVehicleSCF(Vector dst) {
//double min = DBL_MAX;
	Vehicle minV;

	return minV;

}
Vehicle NeighborList::getFurthestNeighborV(Vector pos) {
	Vehicle maxV;
	double max = 0;

	for (list<Vehicle>::iterator v = neighbors.begin(); v != neighbors.end();
			++v) {
		if (distance(pos, v->getPosition()) > MAX_TANS_RANGE)
			continue;
		double d = distance(v->getPosition(), pos);
		if (d > max) {
			max = d;
			maxV = ((*v));
		}
	}

	return maxV;
}
Ptr<RSU> NeighborList::getFurthestNeighborR(Vector pos) {
	Ptr<RSU> maxR;
	double max = 0;

	for (list<RSU>::iterator v = rsu.begin(); v != rsu.end(); ++v) {
		if (distance(pos, v->getPosition()) > MAX_TANS_RANGE)
			continue;
		double d = distance(v->getPosition(), pos);
		if (d >= max) {
			max = d;
			maxR = (&(*v));
		}
	}
	return maxR;
}
}
;
