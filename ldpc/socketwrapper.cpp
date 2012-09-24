#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include "socketwrapper.h"
#include "exceptions.h"
#define WAITSEC 3
#define WAITUSEC 0

int SocketWrapper::Socket(int domain, int type, int protocol){
	int sockfd = socket(domain, type, protocol);
	if(sockfd == -1)
		throw EX_SOCK_ERRSOCKET;
	return sockfd;
}

int SocketWrapper::Bind(int sockfd, const struct sockaddr_in* addr, socklen_t addrlen){
	int retVal = bind(sockfd, (const struct sockaddr*) addr, addrlen);
	if(retVal == -1)
		throw EX_SOCK_ERRBIND;
	return retVal;
}

int SocketWrapper::Listen(int sockfd, int backlog){
	int retVal = listen(sockfd, backlog);
	if(retVal == -1)
		throw EX_SOCK_ERRLISTEN;
	return retVal;
}

int SocketWrapper::Accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen){
	int newSockfd = accept(sockfd, (struct sockaddr*) addr, addrlen);
	if(newSockfd == -1)
		throw EX_SOCK_ERRACCEPT;
	return newSockfd;
}

int SocketWrapper::Connect(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen){
	int retVal = connect(sockfd, (const struct sockaddr*) addr, addrlen);
	if(retVal == -1)
		throw EX_SOCK_ERRCONNECT;
	return retVal;
}

ssize_t SocketWrapper::Send(int sockfd, const void* buf, size_t len, int flags){
	ssize_t retVal = 0;
	for(size_t i=0; i<len; i++){
		if(SocketWrapper::CanWriteSocket(sockfd, WAITSEC, WAITUSEC))
		{
			ssize_t sendRet = send(sockfd, (char*)buf+i, 1, flags);
			if(sendRet == 0){
				return retVal;
			}
			else if(sendRet == 1){
				retVal++;
			}
			else{
				throw EX_SOCK_ERRSEND;
			}
		}
		else{
			// Timeout has occurred
			return retVal;
		}
	}
	return retVal;
}

ssize_t SocketWrapper::Recv(int sockfd, void* buf, size_t len, int flags){
	ssize_t retVal = 0;
	for(size_t i=0; i<len; i++){
		if(SocketWrapper::CanReadSocket(sockfd, WAITSEC, WAITUSEC)){
			ssize_t recvRet = recv(sockfd, (char*)buf+i, 1, flags);
			if(recvRet == -1)
				throw EX_SOCK_ERRRECV;
			else if(recvRet == 0)
				return retVal;
			else if(recvRet == 1)
				retVal++;
			else
				throw EX_SOCK_ERRRECV;
		}
		else{
			return retVal;
		}
	}
	return retVal;
}

int SocketWrapper::Select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout){
	int retVal = select(nfds, rfds, wfds, efds, timeout);
	if(retVal == -1)
		throw EX_SOCK_ERRSELECT;
	return retVal;
}

int SocketWrapper::GetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res){
	int retVal = getaddrinfo(node, service, hints, res);
	if(retVal != 0)
		throw EX_SOCK_ERRGETADDRINFO;
	return retVal;
}

bool SocketWrapper::CanReadSocket(int sockFd, long seconds, long micro){
	fd_set readFds;
	FD_ZERO(&readFds);
	FD_SET(sockFd, &readFds);
	
	struct timeval selectTimeout;
	selectTimeout.tv_sec = seconds;
	selectTimeout.tv_usec = micro;
	
	if(select(sockFd+1, &readFds, NULL, NULL, &selectTimeout) <= 0)  // 0 = timeout, and -1 = error
		return false;
	if(FD_ISSET(sockFd, &readFds))
		return true;
	return false;
}

bool SocketWrapper::CanWriteSocket(int sockFd, long seconds, long micro){
	fd_set writeFds;
	FD_ZERO(&writeFds);
	FD_SET(sockFd, &writeFds);
	
	struct timeval selectTimeout;
	selectTimeout.tv_sec = seconds;
	selectTimeout.tv_usec = micro;
	
	if(select(sockFd+1, NULL, &writeFds, NULL, &selectTimeout) <= 0)  // 0 = timeout, and -1 = error
		return false;
	if(FD_ISSET(sockFd, &writeFds))
		return true;
	return false;
}

void SocketWrapper::GetPeerName(int sockfd, struct sockaddr_in* addr, socklen_t *addrlen){
	if(getpeername(sockfd, (struct sockaddr*)addr, addrlen) == -1)
		throw EX_SOCK_ERRGETPEERNAME;
}

void SocketWrapper::Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen){
	if(setsockopt(sockfd, level, optname, optval, optlen) == -1)
		throw EX_SOCK_ERRSETSOCKOPT;
}

void SocketWrapper::SetReuseSock(int sockfd){
	int optVal = 1;
	SocketWrapper::Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

void SocketWrapper::SetNonblock(int sockfd){
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

ssize_t SocketWrapper::Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *destAddr, socklen_t addrlen){
	ssize_t retVal = sendto(sockfd, buf, len, flags, (sockaddr*)destAddr, addrlen);
	if(retVal == -1)
		throw EX_SOCK_ERRSENDTO;
	return retVal;
}

ssize_t SocketWrapper::Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *srcAddr, socklen_t *addrlen){
	ssize_t retVal = recvfrom(sockfd, buf, len, flags, (sockaddr*)srcAddr, addrlen);
	if(retVal == -1)
		throw EX_SOCK_ERRRECVFROM;
	return retVal;
}
