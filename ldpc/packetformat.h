/// Packet format
#ifndef __PACKETFORMAT_H__
#define __PACKETFORMAT_H__
#include <stdint.h>

struct Packet_Metadata_{
	uint32_t marker;
	uint32_t fileSize;
	uint32_t numPackets;
	uint32_t sizePacket;
	uint32_t numFecPackets;
}__attribute__((__packed__));

struct Packet_Data_{
	uint32_t seqNum;
	char data[1];   	// Extensible attribute
}__attribute__((__packed__));

struct Packet_CumulativeNack_{
	uint32_t seqNum[1]; 		//Extensible attribute
}__attribute__((__packed__));

struct Packet_Ack_{
	uint8_t type;
	uint32_t seqNum;
}__attribute__((__packed__));

typedef struct Packet_Metadata_ Packet_Metadata;
typedef struct Packet_Data_ Packet_Data;
typedef struct Packet_CumulativeNack_ Packet_CumulativeNack;
typedef struct Packet_Ack_ Packet_Ack;
#endif
