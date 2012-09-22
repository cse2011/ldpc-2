/// Methods for serializing and unserializing data
#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__
#include <sys/types.h>
#include <arpa/inet.h>

#include "packetformat.h"

void serialize_Metadata(Packet_Metadata*, char*);	// Must free() after use
void unserialize_Metadata(char*, Packet_Metadata*);	// Must free() after use

void serialize_Data(Packet_Data*, ssize_t packetSize, char*);	// Must free() after use
void unserialize_Data(char*, ssize_t packetSize, Packet_Data*);	// Must free() after use

char* serialize_CumulativeNack(Packet_CumulativeNack*, ssize_t numSeq);	// Must free() after use
Packet_CumulativeNack* unserialize_CumulativeNack(char*, ssize_t packetSize);	// Must free() after use

void serialize_Ack(Packet_Ack*, char*);  // Must free() after use
void unserialize_Ack(char*, Packet_Ack*);

#endif

