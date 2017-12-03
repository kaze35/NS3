/*
 * DataCache.h
 *
 *  Created on: Dec 14, 2013
 *      Author: root
 */

#ifndef DATACACHE_H_
#define DATACACHE_H_
using namespace std;
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "IADDTypeHeader.h"
#include "constants.h"

namespace ns3 {

class CacheEntry: public SimpleRefCount<CacheEntry> {
public:
	Ptr<const Packet> p;
	Time t;
	CacheEntry(Ptr<const Packet> p);
	bool operator<(CacheEntry & b);
	bool operator==(CacheEntry & b);
	CacheEntry():t(0){};



};

class DataCache: public SimpleRefCount<CacheEntry> {
private:
	list<CacheEntry> cache;
	uint32_t Maxsize; //in number of packets

	void removeExpiredData();
	void start();
	Time delay;
	Timer timer;

	bool replacementPolicy(uint32_t packetPop,map<IADDInterestTypes, uint32_t> & popMap);
	uint32_t hit;
	uint32_t miss;
	uint32_t totalRequests;

public:
	DataCache(uint32_t s) :
			Maxsize(s),delay(Seconds(1)),hit(0),miss(0),totalRequests(0) {
	}
	;
	DataCache() :
			Maxsize(CACHE_SIZE),delay(Seconds(1)),hit(0),miss(0),totalRequests(0) {
	}
	;
	bool getPacket(IADDInterestHeader interest, Ptr<Packet> &p);
	bool addPacket(Ptr<const Packet> p,uint32_t p2 , map<IADDInterestTypes, uint32_t> & pop);
	const list<CacheEntry>& getCache() const;
	void setCache(const list<CacheEntry>& cache);
	uint32_t getSize() const;
	void setSize(uint32_t size);
	uint32_t getCacheSize();

	double getHitRatio() const {
		return hit/(double)totalRequests;
	}

	double getMissRatio() const {
		return miss/(double)totalRequests;
	}
	void printCache();
};

} /* namespace ns3 */

#endif /* DATACACHE_H_ */
