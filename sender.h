#ifndef __SENDER_H__
#define __SENDER_H__
#include <sys/types.h>
#include <netinet/in.h>
class Sender{
	private:
		struct LinkedList{
			LinkedList *next, *prev;
			unsigned int num;
		};
		static int sockIn, sockOut;
		static struct sockaddr_in* recvAddr;
		static int filefd;
		static void *mappedPtr;
		static char *bitmap;
		static int bitmapRemainder;
		static PacketMetadata metadata;
		static bool Send(char* buf, ssize_t len);
		static bool RecvAck(unsigned int& seqNum);
		static bool SendChunk(unsigned int index);
	public:
		static void Main(sockaddr_in* hostAddr, char* filePath);
		static void Sendmeta();
};
#endif
