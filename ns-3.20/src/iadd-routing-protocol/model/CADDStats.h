/*
 * FuzzyABR.h
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */

#ifndef FUZZYABR_H_
#define FUZZYABR_H_
#include "ns3/data-collector.h"
#include "ns3/omnet-data-output.h"
#include "ns3/basic-data-calculators.h"
#include "ns3/time-data-calculators.h"
#include "constants.h"
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;

namespace ns3 {

class CADDStats : public Object {
public:
	DataCollector datacol;
	Ptr<MinMaxAvgTotalCalculator<double> > hitRatio;
	Ptr<TimeMinMaxAvgTotalCalculator> elapsedTime;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedCache;
	Ptr<CounterCalculator<> > numberofPacketsGeneratedVehicle;

	Ptr<CounterCalculator<> > numberofPacketsReplicated;
	Ptr<CounterCalculator<> > numberofReplicasRecieved;

	Ptr<CounterCalculator<> > numberofPacketArrived;
	Ptr<CounterCalculator<> > numberofPacketsFromVehicles;
	Ptr<CounterCalculator<> > numberofInterstGenerated;
	Ptr<CounterCalculator<> > numberofMisingData;
	Ptr<CounterCalculator<> > numberofMisingInterest;

	Ptr<CounterCalculator<> > numberofBeaconsGenerated;
	Ptr<CounterCalculator<> > numberofBeaconsRecevied;


	Ptr<MinMaxAvgTotalCalculator<uint32_t> > cacheSize;

	uint32_t numberOfVehicles;
	uint32_t protocol;
	uint32_t interestPeriod;

	void setup(uint32_t numberOfVehicles,uint32_t numberOfDest, uint32_t duration, uint32_t interestPeriod,string traceFile, uint32_t protocol);
	void outputResultsToFile(uint32_t pr);
};
} /* namespace ns3 */

#endif /* FUZZYABR_H_ */
