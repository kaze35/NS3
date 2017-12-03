/*
 * constants.h
 *
 *  Created on: Feb 28, 2014
 *      Author: root
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

namespace ns3 {
#define CACHEENTRY_TTL 3600 // in seconds
#define MAX_JITTER 100 // in millisecnds
#define MAX_SEGMENTS 8 // maximum number of roads in the intersection
#define IADD_DENSITY_TAW 5 // seconds
#define IADD_DATA_TTL 3600 // seconds
#define IADD_INTEREST_TTL 3600 // seconds
#define MAX_SEGMENTS 8 // maximum number of roads in the intersection
#define IADD_PORT  777
#define IADD_APP_PORT  888
#define BEACON_INTERVAL 500 //miliseconds
#define ALLOWED_BEACON_LOSS 2 //Number of beacon messages which may be loss
#define  WIFI_MAX_RANGE 150 //in meters
#define LOOP_TIME_OUT 5 // in seconds
#define IADD_CENTRALITY_TIMEOUT 250 // in seconds
#define RSU_RANGE 100 // in meters
#define STOP_TIMEOUT 10 // in seconds
#define TERMINATE_TIMEOUT 60 // in seconds
#define MAX_TANS_RANGE 120 // in meters
#define HEADING_ANGLE_THRESHOLD 5 // angle
#define ALPHA  0.2 // weighted priority Alpha
#define BETA  0.8 // weighted priority Beta
#define CACHE_SIZE 20
#define RANDOM_CAHING_THRESHOLD 0.5
enum Direction {
	NORTH = 1,
	EAST = 2,
	SOUTH = 3,
	WEST = 4
};

enum MessageType {
	IADD_UNKNOWN = 0,    //unknown packet header
	IADD_VBEACON = 1,   //Vehicle Beacon Packet
	IADD_RBEACON = 2,   //RSU Beacon Packet
	IADD_INTEREST = 3,   // Interest Packet
	IADD_DATA = 4   // Data Packet
};

enum NodeType {
	IADD_VEHICLE = 1, IADD_RSU = 2, IADD_DEST = 3
};

enum VehicleMode {
	SEGMENT_MODE = 1, INTERSECTION_MODE = 2
};
enum IADDProtocol {
	REGIADD = 1, CADD = 2, SCF = 3
};
enum IADDInterestTypes {
	TYPE1 = 0, TYPE2 = 1,TYPE3= 2,TYPE4 = 3, TYPE5 = 4,TYPE6 = 5, TYPES_COUNT = 6
};


}



#endif /* CONSTANTS_H_ */
