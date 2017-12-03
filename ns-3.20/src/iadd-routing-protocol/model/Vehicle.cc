/*
 * Vehicle.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: root
 */

#include "Vehicle.h"
namespace ns3{


const Ipv4Address& Vehicle::getAddress() const {
	return address;
}

void Vehicle::setAddress(const Ipv4Address& address) {
	this->address = address;
}

const Time& Vehicle::getExpiryTime() const {
	return expiryTime;
}

void Vehicle::setExpiryTime(const Time& expiryTime) {
	this->expiryTime = expiryTime;
}

double Vehicle::getHeading() const {
	return heading;
}

void Vehicle::setHeading(double heading) {
	this->heading = heading;
}

const Mac48Address& Vehicle::getHwAddress() const {
	return hwAddress;
}

void Vehicle::setHwAddress(const Mac48Address& hwAddress) {
	this->hwAddress = hwAddress;
}

const Vector& Vehicle::getPosition() const {
	return position;
}

void Vehicle::setPosition(const Vector& position) {
	this->position = position;
}

double Vehicle::getSpeed() const {
	return speed;
}

void Vehicle::setSpeed(double speed) {
	this->speed = speed;
}


const Ipv4Address& RSU::getAddress() const {
	return address;
}

void RSU::setAddress(const Ipv4Address& address) {
	this->address = address;
}

const Time& RSU::getExpiryTime() const {
	return expiryTime;
}

void RSU::setExpiryTime(const Time& expiryTime) {
	this->expiryTime = expiryTime;
}

const Mac48Address& RSU::getHwAddress() const {
	return hwAddress;
}

void RSU::setHwAddress(const Mac48Address& hwAddress) {
	this->hwAddress = hwAddress;
}

const Vector& RSU::getPosition() const {
	return position;
}

void RSU::setPosition(const Vector& position) {
	this->position = position;
}

uint32_t RSU::getRsuId() const {
	return RSU_ID;
}

void RSU::setAddress(uint32_t address) {
	this->address.Set(address);
}

RSU::RSU() {

	address.Set((uint32_t)0);
RSU_ID = -1;
centrality =0;
}

uint32_t RSU::getCentrality() const {
	return centrality;
}

void RSU::setCentrality(uint32_t centrality) {
	this->centrality = centrality;
}

void RSU::setRsuId(uint32_t rsuId) {
	RSU_ID = rsuId;
}

const Ipv4Address& Vehicle::getRsuNext() const {
	return rsuNext;
}

void Vehicle::setRsuNext(const Ipv4Address& rsuNext) {
	this->rsuNext = rsuNext;
}



}
