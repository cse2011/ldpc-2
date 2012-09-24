#ifndef __LDPCUSER_H__
#define __LDPCUSER_H__
#include "ldpclib/src/ldpc_fec.h"


class LdpcUser{
	private:
		static LDPCFecSession ldpcSession;
		static int numPackets;
		static int numFec;
		static int dataSize;
		static bool isEncoder;
		static char** packetMap;
		static char* fileMem;
		static unsigned int fileSize;
	protected:
		static void* MemoryMapper(void* context, int size, int seqNum);
	public:
		static char* fecMem;
		
		static void InitLdpcUser(int numPackets, int numFec, int dataSize);
		static void InitDecoder();
		static void InitEncoder();
		static void MapMemory(void* fileMem, unsigned int fileSize);
		static void GenerateFec();
		static bool IsDecodingComplete();
		static void AddDecodePacket(char* packetData, int seqNum);
};

#endif
