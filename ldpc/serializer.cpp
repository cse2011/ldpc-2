#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "packetformat.h"
#include "serializer.h"

void serialize_Metadata(Packet_Metadata* data, char* retVal){
	Packet_Metadata dataCopy;	// Copy for htonl
	memcpy(&dataCopy, data, sizeof(Packet_Metadata));
	dataCopy.fileSize = htonl(dataCopy.fileSize);
	dataCopy.numPackets = htonl(dataCopy.numPackets);
	dataCopy.sizePacket = htonl(dataCopy.sizePacket);
	dataCopy.numFecPackets = htonl(dataCopy.numFecPackets);
	dataCopy.marker = htonl(~0);
	
	memcpy(retVal, &dataCopy, sizeof(Packet_Metadata));
}

// Must ensure that data is of correct length before calling this function
void unserialize_Metadata(char* data, Packet_Metadata* retVal){
	memcpy(retVal, data, sizeof(Packet_Metadata));
	retVal->marker = ntohl(retVal->marker);
	retVal->fileSize = ntohl(retVal->fileSize);
	retVal->numPackets = ntohl(retVal->numPackets);
	retVal->sizePacket = ntohl(retVal->sizePacket);
	retVal->numFecPackets = ntohl(retVal->numFecPackets);
}

// Warning: Modifies the data
void serialize_Data(Packet_Data* data, ssize_t packetSize, char* retVal){
	data->seqNum = htonl(data->seqNum);
	memcpy(retVal, data, packetSize);
}

void unserialize_Data(char* data, ssize_t packetSize, Packet_Data* retVal){
	memcpy(retVal, data, packetSize);
	retVal->seqNum = ntohl(retVal->seqNum);
}

char* serialize_CumulativeNack(Packet_CumulativeNack* data, ssize_t numSeq){
	ssize_t packetSize = numSeq * sizeof(uint32_t);
	char* retVal = new char[packetSize];
	memset(retVal, 0, packetSize);
	int i;
	for(i=0; i<numSeq; i++){
		uint32_t seqNum = htonl(data->seqNum[i]);
		memcpy(retVal + (i * sizeof(uint32_t)), &seqNum, sizeof(uint32_t));
	}
	return retVal;
}

Packet_CumulativeNack* unserialize_CumulativeNack(char* data, ssize_t packetSize){
	Packet_CumulativeNack* retVal = (Packet_CumulativeNack*)new char[packetSize];
	memcpy(retVal, data, packetSize);
	// To convert the numbers
	unsigned int i;
	for(i=0; i < (packetSize / sizeof(uint32_t)); i++){
		retVal->seqNum[i] = ntohl(retVal->seqNum[i]);
	}
	return retVal;
}

void serialize_Ack(Packet_Ack* data, char *retVal){
	Packet_Ack dataCopy;
	dataCopy.type = data->type;
	dataCopy.seqNum = htonl(data->seqNum);
	
	memcpy(retVal, &dataCopy, sizeof(Packet_Ack));
}

void unserialize_Ack(char* data, Packet_Ack *retVal){
	memcpy(retVal, data, sizeof(Packet_Ack));
	retVal->seqNum = ntohl(retVal->seqNum);
}
