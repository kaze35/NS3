/*
 * IADDPacketTracer.cc
 *
 *  Created on: Mar 20, 2014
 *      Author: hesham
 */


#include "ns3/IADDPacketTracer.h"

using namespace std;

namespace ns3 {
void IADDPacketTracer::closeFile() {
	fileTxLog.close();
	fileMatchLog.close();
	filePacketLog.close();
}
void IADDPacketTracer::sendOperation(Ipv4Address from, Ipv4Address to, Time t, Vector p,
		uint64_t packetUID, MessageType packetType,Ipv4Address dst) {
	if(fileTxLog.is_open())
	fileTxLog << from << ",to," << to << "," << t.GetSeconds() << "," << p.x
			<< ":" << p.y << "," << packetUID <<","<<packetType<<","<<dst<< endl;
}
void IADDPacketTracer::receiveOperation(Ipv4Address to, Ipv4Address from, Time t, Vector p,
		uint64_t packetUID) {
	fileTxLog << to << ",from," << from << "," << t.GetSeconds() << ","
			<< p.x << ":" << p.y << "," << packetUID << endl;
}
void IADDPacketTracer::dataCreated(Ipv4Address ip, Time t, Vector p, uint64_t intUID,
		uint64_t dataUID,Ipv4Address to) {
	fileMatchLog <<"Data,"<< intUID << "," << dataUID << "," << ip << ","
			<< t.GetSeconds() << "," << p.x << ":" << p.y << "," <<to<< endl;
	addData(p.x,p.y,dataUID,ip);
	packetStatus[intUID] = "Interest_consumed";
}
void IADDPacketTracer::replicaCreated(Ipv4Address ip, Time t, Vector p, uint64_t dataUID,
		uint64_t repUID,Ipv4Address to) {
	fileMatchLog <<"Replica,"<< dataUID << "," << repUID << "," << ip << ","
			<< t.GetSeconds() << "," << p.x << ":" << p.y << "," <<to<< endl;
}
void IADDPacketTracer::addInterest(double x,double y,double range, uint32_t ty,uint64_t id,Ipv4Address from){
	filePacketLog<<"Interest,"<<id<<","<<from<<","<<x<<":"<<y<<":"<<range<<","<<ty<<","<<Simulator::Now().GetSeconds()<<endl;
	packetStatus.insert(std::make_pair(id,"Interest_created"));
}
	void IADDPacketTracer::addData(double x,double y,uint64_t id,Ipv4Address from){
		filePacketLog<<"Data,"<<id<<","<<from<<","<<x<<":"<<y<<","<<"6"<<","<<Simulator::Now().GetSeconds()<<endl;
		packetStatus.insert(std::make_pair(id,"Data_created"));

	}
}

