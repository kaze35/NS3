/*
 * FuzzyABR.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: root
 */

#include "CADDStats.h"
#include <fstream>
using namespace std;
namespace ns3 {

void CADDStats::setup(uint32_t numberOfVehicles, uint32_t numberOfDest,
		uint32_t duration, uint32_t interestPeriod, string traceFile,
		uint32_t protocol) {
	 this->numberOfVehicles=numberOfVehicles;
	 this-> protocol=protocol;
	 this->interestPeriod=interestPeriod;

	stringstream ss;

	time_t now;
	struct tm *current;
	now = time(0);
	current = localtime(&now);
	ss << numberOfVehicles << "-" << duration << "-" << interestPeriod << "-"
			<< setfill('0') << setw(2) << current->tm_hour << ":"
			<< setfill('0') << setw(2) << current->tm_min << ":" << setfill('0')
			<< setw(2) << current->tm_sec;
	stringstream ss2;
	ss2 << current->tm_mday << "-" << current->tm_mon << " " << current->tm_hour
			<< ":" << current->tm_min << ":" << current->tm_sec;

	datacol.AddMetadata("#duration", duration);
	datacol.AddMetadata("#interestPeriod", interestPeriod);
	datacol.AddMetadata("#VehicleNum", numberOfVehicles);
	datacol.AddMetadata("#DestNum", numberOfDest);
	datacol.AddMetadata("#MobilityFile", traceFile);
	datacol.AddMetadata("#timeStarted", ss2.str());
	datacol.AddMetadata("#TransmitionRange", (uint32_t) WIFI_MAX_RANGE);
	datacol.AddMetadata("#CacheSize", (uint32_t) CACHE_SIZE);
	datacol.AddMetadata("#beacon_interval", (uint32_t) BEACON_INTERVAL);

	if (protocol == SCF)
		datacol.DescribeRun("SCF RP", ss.str(), traceFile, ss.str(),
				"Testing Protocol: SCF");
	else
		datacol.DescribeRun("CADD RP", ss.str(), traceFile, ss.str(),
				"Testing Protocol: CADD");

	numberofInterstGenerated = CreateObject<CounterCalculator<> >();
	numberofInterstGenerated->SetKey("Total-number-of-interests");
	datacol.AddDataCalculator(numberofInterstGenerated);

	numberofPacketsGeneratedCache = CreateObject<CounterCalculator<> >();
	numberofPacketsGeneratedCache->SetKey("Packets_generated_cache");
	datacol.AddDataCalculator(numberofPacketsGeneratedCache);

	numberofPacketsGeneratedVehicle = CreateObject<CounterCalculator<> >();
	numberofPacketsGeneratedVehicle->SetKey("Packets_generated_vehicle");
	datacol.AddDataCalculator(numberofPacketsGeneratedVehicle);

	numberofPacketsReplicated = CreateObject<CounterCalculator<> >();
	numberofPacketsReplicated->SetKey("Packets_Replicated");
	datacol.AddDataCalculator(numberofPacketsReplicated);

	numberofReplicasRecieved = CreateObject<CounterCalculator<> >();
	numberofReplicasRecieved->SetKey("Packets_Replicated_received");
	datacol.AddDataCalculator(numberofReplicasRecieved);

	numberofPacketArrived = CreateObject<CounterCalculator<> >();
	numberofPacketArrived->SetKey("Total-data-received");
	datacol.AddDataCalculator(numberofPacketArrived);

	numberofPacketsFromVehicles = CreateObject<CounterCalculator<> >();
	numberofPacketsFromVehicles->SetKey("Total-data-received-vehicle");
	datacol.AddDataCalculator(numberofPacketsFromVehicles);

	numberofMisingData = CreateObject<CounterCalculator<> >();
	numberofMisingData->SetKey("Total-data-missing");
	datacol.AddDataCalculator(numberofMisingData);

	numberofMisingInterest = CreateObject<CounterCalculator<> >();
	numberofMisingInterest->SetKey("Total-interest-missing");
	datacol.AddDataCalculator(numberofMisingInterest);

	elapsedTime = CreateObject<TimeMinMaxAvgTotalCalculator>();
	elapsedTime->SetKey("Elapsed-Time");
	datacol.AddDataCalculator(elapsedTime);

	hitRatio = CreateObject<MinMaxAvgTotalCalculator<double> >();
	hitRatio->SetKey("hit-ratio");
	datacol.AddDataCalculator(hitRatio);

	cacheSize = CreateObject<MinMaxAvgTotalCalculator<uint32_t> >();
	cacheSize->SetKey("cache-Size");
	datacol.AddDataCalculator(cacheSize);

	numberofBeaconsGenerated = CreateObject<CounterCalculator<> >();
	numberofBeaconsGenerated->SetKey("Beacons-Generated");
	datacol.AddDataCalculator(numberofBeaconsGenerated);

	numberofBeaconsRecevied = CreateObject<CounterCalculator<> >();
	numberofBeaconsRecevied->SetKey("Beacons-Received");
	datacol.AddDataCalculator(numberofBeaconsRecevied);

}
void CADDStats::outputResultsToFile(uint32_t protocol) {
	Ptr<DataOutputInterface> output;
	output = CreateObject<OmnetDataOutput>();
	stringstream ss;
	//traceFile.substr(0, traceFile.length() - 4);
	string s;
	if (protocol == CADD)
		s= "CADD";
	else if (protocol == SCF)
		s= "SCF";
	ss<<s<<"-";
	stringstream ss2;
	struct tm *current;
	time_t now = time(0);
	current = localtime(&now);
	ss2 << current->tm_mday << "-" << current->tm_mon << " " << current->tm_hour
			<< ":" << current->tm_min << ":" << current->tm_sec;

	datacol.AddMetadata("#timeFinished", ss2.str());

	output->SetFilePrefix(ss.str());
	output->Output(datacol);

	fstream fout("results.res", fstream::app | fstream::out);

	uint32_t totalGenerated = numberofPacketsGeneratedCache->GetCount()
			+ numberofPacketsGeneratedVehicle->GetCount();
	//uint32_t receivedCache = numberofPacketArrived->GetCount()
	//- numberofPacketsFromVehicles->GetCount();
	uint32_t datadropped = totalGenerated - numberofPacketArrived->GetCount()
			- numberofMisingData->GetCount();

	uint32_t intDropped = numberofInterstGenerated->GetCount() - totalGenerated
			- numberofMisingInterest->GetCount();

	fout << numberOfVehicles << ","
			<< s <<",Aoi,2000,"
			<<interestPeriod<<","
			<<CACHE_SIZE<<","
			<<"getItFROMFIILE,"
			<< numberofInterstGenerated->GetCount() << ","

			<<numberofPacketsGeneratedCache->GetCount()<<","
			<<numberofPacketsGeneratedVehicle->GetCount()<<","
			<< totalGenerated<< ","

			<< numberofPacketArrived->GetCount()- numberofPacketsFromVehicles->GetCount()<<","
			<< numberofPacketsFromVehicles->GetCount()<<","
			<< numberofPacketArrived->GetCount()<<","

			<< numberofMisingData->GetCount()<<","
			<< numberofMisingInterest->GetCount() <<","

			<< numberofPacketsGeneratedCache->GetCount()-numberofPacketArrived->GetCount()+ numberofPacketsFromVehicles->GetCount()<<","
			<< numberofPacketsGeneratedVehicle->GetCount() - numberofPacketsFromVehicles->GetCount()<<","
			<< totalGenerated -  numberofPacketArrived->GetCount()<<","

			<< numberofInterstGenerated->GetCount() - totalGenerated<<","

			<< datadropped <<","
			<< intDropped<<","

			<<numberofPacketsFromVehicles->GetCount()/(double)numberofPacketArrived->GetCount()<<","
			<<numberofPacketsGeneratedVehicle->GetCount()/(double)totalGenerated<< ","


				<< endl;
	/*fout << (uint32_t) BEACON_INTERVAL << ","
			<< numberofBeaconsGenerated->GetCount() << ","
			<< numberofBeaconsRecevied->GetCount() << ",,"

			<< numberofInterstGenerated->GetCount() << "," << totalGenerated
			<< "," << numberofMisingInterest->GetCount() << "," << intDropped
			<< ","

			<< totalGenerated << "," << numberofPacketArrived->GetCount() << ","
			<< numberofMisingData->GetCount() << "," << datadropped << ","
			<< endl;*/

	fout.close();

}
} /* namespace ns3 */
