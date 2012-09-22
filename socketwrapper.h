#ifndef __SOCKETWRAPPER_H__
#define __SOCKETWRAPPER_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
class SocketWrapper{
	public:
		static int Socket(int domain, int type, int protocol);
		static int Bind(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
		static int Listen(int sockfd, int backlog);
		static int Accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen);
		static int Connect(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
		
		// The following 2 functions incorporate the "write 1 byte" policy.
		// They are also responsible for calling CanReadSocket() and CanWriteSocket() with appropriate timeout
		static ssize_t Send(int sockfd, const void* buf, size_t len, int flags);
		static ssize_t Recv(int sockfd, void* buf, size_t len, int flags);
		
		static int Select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout);
		static int GetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
		
		// Timeout functionality on sockets, implemented by using select
		static bool CanReadSocket(int sockFd, long seconds, long micro);
		static bool CanWriteSocket(int sockFd, long seconds, long micro);
		
		static void GetPeerName(int sockfd, struct sockaddr_in* addr, socklen_t *addrlen);
		static void Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
		
		static void SetReuseSock(int sockfd);
		static void SetNonblock(int sockfd);
		static ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr_in *dest_addr, socklen_t addrlen);
		static ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen);
};
#endif
