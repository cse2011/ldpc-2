#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include "packetformat.h"

#define RECVER_PORT 2112
#define SENDER_PORT 2113
#define PACKET_SIZE 1470
#define ACK_RETRANSMIT 5
#define ACK_TERMINATION_NUMBER 30
#define META_RETRANSMIT 10
#define LDPC_DATA_BUFFER_SIZE (100000*PACKET_SIZE)

#define TYPE_ACK  0
#define TYPE_NACK 1

#define RECVR_METADATA_WAIT 20   //Milliseconds
#define RECVR_FILE "data"

/// Related to LDPC
#define LDPC_VERBOSITY 1
#define LDPC_MULT 0.292
#define LDPC_SEED 2003
#define LDPC_SESSION_TYPE TypeSTAIRS
#define LDPC_LEFT_DEGREE 3

typedef struct Packet_Metadata_ PacketMetadata;
typedef struct Packet_Data_ PacketData;
typedef struct Packet_CumulativeNack_ PacketCumulativeNack;
typedef struct Packet_Ack_ PacketAck;
#endif
