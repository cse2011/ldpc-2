#include <sys/mman.h>
#include "ldpcuser.h"
#include "settings.h"
#include "exceptions.h"

#include <iostream>

using namespace std;

/// Initializing the static members
LDPCFecSession LdpcUser::ldpcSession;
int LdpcUser::numPackets;
int LdpcUser::numFec;
int LdpcUser::dataSize;
bool LdpcUser::isEncoder;
char** LdpcUser::packetMap;
char* LdpcUser::fileMem;
char* LdpcUser::fecMem;
unsigned int LdpcUser::fileSize;

/// Constructor
void LdpcUser::InitLdpcUser(int numPackets, int numFec, int dataSize){
	LdpcUser::numPackets = numPackets;
	LdpcUser::numFec = numFec;
	LdpcUser::dataSize = dataSize;
	LdpcUser::ldpcSession.SetVerbosity(LDPC_VERBOSITY);
	LdpcUser::ldpcSession.m_decodedSymbol_callback = LdpcUser::MemoryMapper;
}

/// Initialize the encoder
void LdpcUser::InitEncoder(){
	if(ldpcSession.InitSession(LdpcUser::numPackets, LdpcUser::numFec, LdpcUser::dataSize, FLAG_CODER, LDPC_SEED, LDPC_SESSION_TYPE, LDPC_LEFT_DEGREE) == LDPC_ERROR)
		throw EX_LDPC_ENCODEINIT;
	LdpcUser::isEncoder = true;
}

void LdpcUser::InitDecoder(){
	if(ldpcSession.InitSession(LdpcUser::numPackets, LdpcUser::numFec, LdpcUser::dataSize, FLAG_DECODER, LDPC_SEED, LDPC_SESSION_TYPE, LDPC_LEFT_DEGREE) == LDPC_ERROR)
		throw EX_LDPC_DECODEINIT;
	LdpcUser::isEncoder = false;
}

void LdpcUser::MapMemory(void* fileMem, unsigned int fileSize){
	LdpcUser::packetMap = new char*[LdpcUser::numPackets + LdpcUser::numFec];
	LdpcUser::fileMem = (char*)fileMem;
	// Create the fec memory for encoding
	#ifdef MAP_NOSYNC
		LdpcUser::fecMem = (char*)mmap(0, (LdpcUser::numFec * LdpcUser::dataSize), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_NOSYNC | MAP_ANONYMOUS, -1, 0);
	#else
		LdpcUser::fecMem = (char*)mmap(0, (LdpcUser::numFec * LdpcUser::dataSize), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	#endif
	if(LdpcUser::fecMem == MAP_FAILED)
		throw EX_LDPC_NOMAP;	
	if(LdpcUser::isEncoder){
		for(int i=0; i < (LdpcUser::numPackets-1); i++){
			LdpcUser::packetMap[i] = (char*)fileMem + (i * LdpcUser::dataSize);
		}
		// For the last packet, we need to manually add padding, and hence handle it specially
		LdpcUser::packetMap[LdpcUser::numPackets-1] = new char[LdpcUser::dataSize];
		if(fileSize % LdpcUser::dataSize)
			memcpy(LdpcUser::packetMap[LdpcUser::numPackets-1], (char*)fileMem + ((LdpcUser::numPackets-1) * LdpcUser::dataSize), fileSize % LdpcUser::dataSize);
		else
			LdpcUser::packetMap[LdpcUser::numPackets-1] = (char*)fileMem + ((LdpcUser::numPackets-1) * LdpcUser::dataSize);
		for(int i=0; i < LdpcUser::numFec; i++){
			LdpcUser::packetMap[LdpcUser::numPackets+i] = LdpcUser::fecMem + (i * LdpcUser::dataSize);
		}
	}
	else{
		for(int i=0; i < (LdpcUser::numPackets+LdpcUser::numFec); i++){
			LdpcUser::packetMap[i] = NULL;
		}
	}
	LdpcUser::fileSize = fileSize;
}

void LdpcUser::GenerateFec(){
	for(int i=0; i < LdpcUser::numFec; i++){
		LdpcUser::ldpcSession.BuildParitySymbol((void**)packetMap, i, packetMap[LdpcUser::numPackets+i]);
	}
}

bool LdpcUser::IsDecodingComplete(){
	if(LdpcUser::ldpcSession.IsDecodingComplete((void**)packetMap)){
		// Do the last copy back
		if(fileSize % LdpcUser::dataSize){
			unsigned int lastPos = LdpcUser::dataSize * (LdpcUser::numPackets - 1);
			memcpy(LdpcUser::fileMem + lastPos, packetMap[(LdpcUser::numPackets - 1)], LdpcUser::fileSize % LdpcUser::dataSize);
		}
		return true;
	}
	return false;
}

void LdpcUser::AddDecodePacket(char* packetData, int seqNum){
	if(LdpcUser::ldpcSession.DecodingStepWithSymbol((void**)packetMap, packetData, seqNum, true) == LDPC_ERROR)
		throw EX_LDPC_DECODERR;
}

void* LdpcUser::MemoryMapper(void* context, int size, int seqNum){
// MOGRE: Remove below if conditions don't ever occur after tests, to improve efficiency
	if(size > LdpcUser::dataSize)
		throw EX_LDPC_BADMEMSIZE;
	if((seqNum < 0) || (seqNum >= (LdpcUser::numPackets + LdpcUser::numFec)))
		throw EX_LDPC_BADMEMSEQ;
// MOGRE: End of optimization
	if(seqNum < (LdpcUser::numPackets-1))
		return LdpcUser::fileMem + (seqNum*LdpcUser::dataSize);
	else
		return new char[size];
}



