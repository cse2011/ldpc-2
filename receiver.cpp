#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "exceptions.h"
#include "settings.h"
#include "serializer.h"
#include "receiver.h"
#include "socketwrapper.h"

#include "ldpcuser.h"

using namespace std;
int Receiver::sockIn;
int Receiver::sockOut;
struct sockaddr_in *Receiver::sendAddr;
PacketMetadata *Receiver::metadata;
char *Receiver::bitmap;
int Receiver::filefd;
void *Receiver::mappedPtr;
unsigned int Receiver::bitmapRemainder;
int Receiver::ldpcFlag = 0;
char* ldpcDataBuffer = NULL;
int lBufCount = 0;
int mutexFlag = 0;
pthread_mutex_t decodeLock;

void* Receiver::LdpcProc(void* ptr) {
    int i = 0;
    // Initialize the LDPC
    LdpcUser::InitLdpcUser(metadata->numPackets, metadata->numFecPackets, metadata->sizePacket - sizeof(uint32_t));
    LdpcUser::InitDecoder();
    LdpcUser::MapMemory(mappedPtr, metadata->fileSize);
    
    ldpcFlag = 1; //Other thread can now decode
    
    for(i = 0; i < lBufCount; i++) {
        pthread_mutex_lock(&decodeLock);
        PacketData* packet = (PacketData*)(ldpcDataBuffer + (i * metadata->sizePacket));
        LdpcUser::AddDecodePacket(packet->data, packet->seqNum);
        pthread_mutex_unlock(&decodeLock);
    }
    cerr<<"Done processing old packets"<<endl;
    mutexFlag = 1;
    return NULL;
    
}

int main(int argc, char* argv[]){
	// No arguments, just start the receiver
	try{
		Receiver::Main();
	}
	catch(int ex){
		cerr<<"Exception: "<<hex<<ex<<endl;
		if(ex >= 0x0200 && ex < 0x0300){
			perror("exp:");
		}
	}
}

void Receiver::Main(){
        pthread_t ldpcThread;
        pthread_mutex_init(&decodeLock,NULL);
        //Allocate memory for ldpcDataBuffer
        ldpcDataBuffer = (char*) malloc(LDPC_DATA_BUFFER_SIZE);
        
	// UDP socket connection time
	sendAddr = new sockaddr_in;
	
	sockIn = SocketWrapper::Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockOut = SocketWrapper::Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SocketWrapper::SetReuseSock(sockIn);
	SocketWrapper::SetReuseSock(sockOut);
	
	// Bind the sockIn to the UDP port
	struct sockaddr_in selfAddr;
	selfAddr.sin_family = AF_INET;
	selfAddr.sin_addr.s_addr = INADDR_ANY;
	selfAddr.sin_port = htons(RECVER_PORT);
	memset(selfAddr.sin_zero, 0, 8);
	SocketWrapper::Bind(sockIn, &selfAddr, sizeof(sockaddr_in));
	
	// Receive the metadata
	Receiver::ReceiveMeta();
	cerr<<"Received the metadata"<<endl;
        
        //Create the LDPC Thread here
        pthread_create(&ldpcThread,NULL,&Receiver::LdpcProc,NULL);
	
	
	// The loop
	struct sockaddr_in senderAddr;
	socklen_t senderAddrLen = sizeof(sockaddr_in);
	unsigned int totalNumPackets = metadata->numPackets + metadata->numFecPackets;
	
	char buf[PACKET_SIZE + 50];
	int count = 0;
	int skipCount = totalNumPackets / 1000;
	if(skipCount == 0) skipCount = 1;
	char packetBuffer[PACKET_SIZE];
	PacketData *packet = (PacketData*)packetBuffer;
	while(bitmapRemainder>0){
		unsigned int rcvdLen = SocketWrapper::Recvfrom(sockIn, buf, sizeof(buf), 0, &senderAddr, &senderAddrLen);
		if(rcvdLen < metadata->sizePacket){
			memset(packet, 0, PACKET_SIZE);
			unserialize_Data(buf, rcvdLen, packet);
		}
		else
			unserialize_Data(buf, rcvdLen, packet);
		if(packet->seqNum == 0xFFFFFFFF){
			continue;   // Duplicate metadata packet
		}
		else if(packet->seqNum < totalNumPackets){
			// Send ack
			SendAck(packet->seqNum);
			if(bitmap[packet->seqNum] != 1){
				// SaveToFile(packet, rcvdLen);
				bitmapRemainder--;
				count++;
				
				if(count%skipCount == 0)		//Display purposes
					cerr<<".";
					
                                if(ldpcFlag == 1) {
                                
                                    // Add to the LDPC
                                    if(mutexFlag) pthread_mutex_lock(&decodeLock);
                                    LdpcUser::AddDecodePacket(packet->data, packet->seqNum);
                                    if(mutexFlag) pthread_mutex_unlock(&decodeLock);
                                    // LDPC terminating condition
                                    if(bitmapRemainder < metadata->numFecPackets){
                                        if(mutexFlag) pthread_mutex_lock(&decodeLock);
                                        if(LdpcUser::IsDecodingComplete()){
                                                    cerr<<"\nLDPC Terminating condition, saving "<<bitmapRemainder<<endl;
                                                    if(mutexFlag) pthread_mutex_unlock(&decodeLock);
                                                    break;
                                        }
                                        if(mutexFlag) pthread_mutex_unlock(&decodeLock);
                                    }
                                }
                                else {
                                    memcpy(ldpcDataBuffer+(lBufCount*metadata->sizePacket),packet,metadata->sizePacket);
                                    lBufCount++;
                                }
                                    				
			}
			bitmap[packet->seqNum] = 1;
		}
	}
	// Send the termination command
	for(int i=0;i<ACK_TERMINATION_NUMBER; i++)
		SendAck(totalNumPackets);
	msync(mappedPtr, metadata->fileSize, MS_ASYNC);
	munmap(mappedPtr, metadata->fileSize);
	close(filefd);
	cerr<<endl;
}

void Receiver::ReceiveMeta(){
	char buf[PACKET_SIZE + 50];   // Static allocation on the stack
	bool isMetaReceived = false;
	PacketMetadata *metadata = new PacketMetadata;
	while(!isMetaReceived){
		socklen_t sendAddrlen = sizeof(struct sockaddr_in);
		ssize_t recvLen;
		recvLen = SocketWrapper::Recvfrom(sockIn, buf, PACKET_SIZE, 0, sendAddr, &sendAddrlen);
		sendAddr->sin_port = htons(SENDER_PORT);
		if(recvLen == sizeof(PacketMetadata)){
			unserialize_Metadata(buf, metadata);
			if(metadata->marker == 0xFFFFFFFF){
				// Valid metadata
				Receiver::metadata = metadata;
				Receiver::bitmap = new char[metadata->numPackets + metadata->numFecPackets];
				bitmapRemainder = metadata->numPackets + metadata->numFecPackets;
				memset(Receiver::bitmap, 0, metadata->numPackets + metadata->numFecPackets);
				InitializeFile();

				isMetaReceived = true;
			}
			else{
				Receiver::SendMetaNack();
			}
		}
		else
			Receiver::SendMetaNack();
	}
}

void Receiver::SendMetaNack(){
	if(sendAddr != NULL){
		PacketAck packet;
		packet.type = TYPE_NACK;
		packet.seqNum = ~0;
		char buf[sizeof(PacketAck)];
		serialize_Ack(&packet, buf);
		while(1){
			try{
				SocketWrapper::Sendto(sockOut, buf, sizeof(PacketAck), 0, sendAddr, sizeof(sockaddr_in));
			}
			catch(int ex){
				if(ex == EX_SOCK_ERRSENDTO && errno == ENOBUFS){
					continue;
				}
				else {
					throw ex;
				}
			}
			return;
		}
	}
}

void Receiver::InitializeFile(){
	filefd = open(RECVR_FILE, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	if(filefd == -1)
		throw EX_FILE_NOOPEN;
	if(lseek(filefd, metadata->fileSize-1, SEEK_SET)== -1)
		throw EX_FILE_NOSEEK;
	if(write(filefd, "", 1) == -1)
		throw EX_FILE_NOWRITE;
#ifdef MAP_NOSYNC
	mappedPtr = mmap(0, metadata->fileSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NOSYNC, filefd, 0);
#else
	mappedPtr = mmap(0, metadata->fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
#endif
	if(mappedPtr == MAP_FAILED)
		throw EX_FILE_NOMAP;
}

void Receiver::SendAck(unsigned int seqNum){
	for(int i=0;i<ACK_RETRANSMIT;i++){
		PacketAck packet;
		packet.type = TYPE_ACK;
		packet.seqNum = seqNum;
		char buf[sizeof(PacketAck)];
		serialize_Ack(&packet, buf);
		while(1){
			try{
			SocketWrapper::Sendto(sockOut, buf, sizeof(PacketAck), 0, sendAddr, sizeof(sockaddr_in));
			}
			catch(int ex){
				if(ex == EX_SOCK_ERRSENDTO && errno == ENOBUFS)
					continue;
				else throw ex;
			}
			return;
		}
	}
}

void Receiver::SaveToFile(PacketData* packet, unsigned int packLen){
	int dataLen = packLen - sizeof(uint32_t);
	int chunkSize = metadata->sizePacket - sizeof(uint32_t);
	if(packet->seqNum < metadata->numPackets){
		unsigned int location = packet->seqNum * chunkSize;
		memcpy((char*)mappedPtr+location, packet->data, dataLen);
	}
	else{
		unsigned int location = (packet->seqNum - metadata->numPackets) * chunkSize;
		memcpy(LdpcUser::fecMem+location, packet->data, dataLen);
	}
}
