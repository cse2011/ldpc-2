#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

/// Exceptions related to the program as a whole
#define EX_ABORT                      0x0001
#define EX_ABORT_SIGFAIL              0x0002

/// Exceptions related to command processing go here


/// Exceptions related to socket operations
#define EX_SOCK_ERRSOCKET             0x0201
#define EX_SOCK_ERRBIND               0x0202
#define EX_SOCK_ERRLISTEN             0x0203
#define EX_SOCK_ERRACCEPT             0x0204
#define EX_SOCK_ERRCONNECT            0x0205
#define EX_SOCK_ERRSEND               0x0206
#define EX_SOCK_ERRRECV               0x0207
#define EX_SOCK_ERRSELECT             0x0208
#define EX_SOCK_ERRGETADDRINFO        0x0209
#define EX_SOCK_ERRGETPEERNAME        0x020a
#define EX_SOCK_BADPEERPROTO          0x020b
#define EX_SOCK_ERRSETSOCKOPT         0x020c
#define EX_SOCK_ERRSENDTO             0x020d
#define EX_SOCK_ERRRECVFROM           0x020e

/// Exceptions related to file handling
#define EX_FILE_NOOPEN                0x0301
#define EX_FILE_NOGOOD                0x0302
#define EX_FILE_NOMAP                 0x0303
#define EX_FILE_NOSEEK                0x0304
#define EX_FILE_NOWRITE               0x0305

/// Exceptions related to error in packets

/// Exceptions related to LDPC
#define EX_LDPC_DECODEINIT            0x0501
#define EX_LDPC_ENCODEINIT            0x0502
#define EX_LDPC_NOMAP                 0x0503
#define EX_LDPC_BADMEMSIZE            0x0504
#define EX_LDPC_BADMEMSEQ             0x0505
#define EX_LDPC_DECODERR              0x0506

#endif
