#ifndef __RECEIVER_H__
#define __RECEIVER_H__
#include <netinet/in.h>

#include "packetformat.h"
#include "settings.h"
class Receiver{
	private:
		static int sockIn, sockOut;
		static int filefd;
		static void *mappedPtr;
		static struct sockaddr_in *sendAddr;
                static struct sockaddr_in *recvAddr;
		static PacketMetadata *metadata;
		static char *bitmap;
		static unsigned int bitmapRemainder;
                static int ldpcFlag;
		static void SendMetaNack();
		static void SendAck(unsigned int);
		static void SaveToFile(PacketData*, unsigned int);
		static void InitializeFile();
	public:
		static void Main();
		static void ReceiveMeta();
                static void* LdpcProc(void* ptr);
                static void* SabotageProc(void* ptr);
};
#endif
