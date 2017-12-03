/*
 * DataCache.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: root
 */
#define __STDC_LIMIT_MACROS
#include "DataCache.h"
#include "constants.h"
#include "IADDTypeHeader.h"
#include "iadd-routing-protocol.h"
#include <stdint.h>

namespace ns3 {

const list<CacheEntry>& DataCache::getCache() const {
	return cache;
}

void DataCache::setCache(const list<CacheEntry>& cache) {
	this->cache = cache;
}

uint32_t DataCache::getSize() const {
	return Maxsize;
}

bool DataCache::getPacket(IADDInterestHeader interest, Ptr<Packet> &p) {
	totalRequests++;

	for (list<CacheEntry>::iterator i = cache.begin(); i != cache.end(); ++i) {
		Ptr<Packet> pac = i->p->Copy();
		UdpHeader u;
		IADDTypeHeader th;
		IADDInterestHeader t;
		pac->RemoveHeader(u);
		pac->RemoveHeader(th);
		pac->RemoveHeader(t);
		if (t.isMatchtoInterest(interest)) {
			p = i->p->Copy();
			hit++;
			i->t = Simulator::Now() + Seconds(CACHEENTRY_TTL);
			return true;
		}
	}
	miss++;
	return false;
}

CacheEntry::CacheEntry(Ptr<const Packet> p) {
	this->p = p;
	this->t = Simulator::Now() + Seconds(CACHEENTRY_TTL);
}
bool DataCache::replacementPolicy(uint32_t packetPop,
		map<IADDInterestTypes, uint32_t> & popMap) {
	uint32_t min = INT32_MAX;
	for (list<CacheEntry>::iterator i = cache.begin(); i != cache.end(); ++i) {
		if (min < popMap.at(IADD::getInterestHeader(i->p).getInterestType())) {
			min = popMap.at(IADD::getInterestHeader(i->p).getInterestType());
		}
	}
	if (min > packetPop)
		return false;
	int64_t minTime = INT64_MAX;
	list<CacheEntry>::iterator minCE;
	for (list<CacheEntry>::iterator i = cache.begin(); i != cache.end(); ++i) {
		if (min == popMap.at(IADD::getInterestHeader(i->p).getInterestType())) {
			if (minTime < i->t.GetInteger()) {
				minTime = i->t.GetInteger();
				minCE = (i);
			}
		}
	}
	cache.erase(minCE);
	return true;
}
bool DataCache::addPacket(Ptr<const Packet> p, uint32_t packetPop,
		map<IADDInterestTypes, uint32_t> & popMap) {
	if (cache.size() >= Maxsize) {
		if (!replacementPolicy(packetPop, popMap))
			return false;
	}

	Ptr<Packet> packet = p->Copy();

	UdpHeader u2;
	IADDTypeHeader th2;
	IADDInterestHeader t2;
	packet->RemoveHeader(u2);
	packet->RemoveHeader(th2);
	packet->RemoveHeader(t2);


	for (list<CacheEntry>::iterator i = cache.begin(); i != cache.end(); ++i) {
		Ptr<Packet> pac = i->p->Copy();
		UdpHeader u;
		IADDTypeHeader th;
		IADDInterestHeader t;
		pac->RemoveHeader(u);
		pac->RemoveHeader(th);
		pac->RemoveHeader(t);
		if (t.isMatchtoInterest(t2)) {
			cache.erase(i);
			break;
		}
	}


CacheEntry ce(p);
cache.push_back(ce);

return true;

}

void DataCache::setSize(uint32_t size) {
this->Maxsize = size;
}
struct RemoveData //predicate
{
bool operator()(const CacheEntry & c) const {
	return ((c.t < Simulator::Now()));
}

};

void DataCache::removeExpiredData() {
RemoveData pred;

if (!cache.empty()) {
	cache.erase(remove_if(cache.begin(), cache.end(), pred), cache.end());
}

timer.Cancel();
timer.Schedule();
}

void DataCache::start() {
timer.Cancel();
timer = Timer::CANCEL_ON_DESTROY;
timer.SetDelay(delay);
timer.SetFunction(&DataCache::removeExpiredData, this);
}

uint32_t DataCache::getCacheSize() {
return cache.size();
}

bool CacheEntry::operator ==(CacheEntry& b) {
if (p->GetUid() == b.p->GetUid())
	return true;
return false;
}
void DataCache::printCache() {
for (list<CacheEntry>::iterator i = cache.begin(); i != cache.end(); ++i) {
	cout << "(UID=" << i->p->GetUid() << ", Size= " << i->p->GetSize() << ") ";
}
cout << endl;
}

} /* namespace ns3 */
