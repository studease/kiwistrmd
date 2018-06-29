/*
 * stu_socket.h
 *
 *  Created on: 2018��3��30��
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_CORE_WIN32_STU_SOCKET_H_
#define STUDEASE_CN_CORE_WIN32_STU_SOCKET_H_

#include "../../stu_config.h"
#include "../stu_core.h"

#define STU_WRITE_SHUTDOWN      SD_SEND
#define STU_SOCKET_INVALID      INVALID_SOCKET

typedef SOCKET  stu_socket_t;
typedef int     socklen_t;


#define stu_socket(af, type, proto)                                          \
		WSASocketW(af, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED)

#define stu_socket_n           "WSASocketW()"

int stu_nonblocking(stu_socket_t s);
int stu_blocking(stu_socket_t s);

#define stu_nonblocking_n      "ioctlsocket(FIONBIO)"
#define stu_blocking_n         "ioctlsocket(!FIONBIO)"

#define stu_socket_shutdown     shutdown
#define stu_socket_shutdown_n  "shutdown()"

#define stu_socket_close        closesocket
#define stu_socket_close_n     "closesocket()"

int stu_tcp_push(stu_socket_t s);
#define stu_tcp_push_n         "tcp_push()"


#ifndef WSAID_ACCEPTEX

typedef BOOL (PASCAL FAR * LPFN_ACCEPTEX)(
	IN SOCKET sListenSocket,
	IN SOCKET sAcceptSocket,
	IN PVOID lpOutputBuffer,
	IN DWORD dwReceiveDataLength,
	IN DWORD dwLocalAddressLength,
	IN DWORD dwRemoteAddressLength,
	OUT LPDWORD lpdwBytesReceived,
	IN LPOVERLAPPED lpOverlapped
);

#define WSAID_ACCEPTEX                                                       \
		{0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#endif


#ifndef WSAID_GETACCEPTEXSOCKADDRS

typedef VOID (PASCAL FAR * LPFN_GETACCEPTEXSOCKADDRS)(
	IN PVOID lpOutputBuffer,
	IN DWORD dwReceiveDataLength,
	IN DWORD dwLocalAddressLength,
	IN DWORD dwRemoteAddressLength,
	OUT struct sockaddr **LocalSockaddr,
	OUT LPINT LocalSockaddrLength,
	OUT struct sockaddr **RemoteSockaddr,
	OUT LPINT RemoteSockaddrLength
);

#define WSAID_GETACCEPTEXSOCKADDRS                                           \
		{0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#endif


#ifndef WSAID_TRANSMITFILE

#ifndef TF_DISCONNECT

#define TF_DISCONNECT           1
#define TF_REUSE_SOCKET         2
#define TF_WRITE_BEHIND         4
#define TF_USE_DEFAULT_WORKER   0
#define TF_USE_SYSTEM_THREAD    16
#define TF_USE_KERNEL_APC       32

typedef struct _TRANSMIT_FILE_BUFFERS {
    LPVOID Head;
    DWORD HeadLength;
    LPVOID Tail;
    DWORD TailLength;
} TRANSMIT_FILE_BUFFERS, *PTRANSMIT_FILE_BUFFERS, FAR *LPTRANSMIT_FILE_BUFFERS;

#endif

typedef BOOL (PASCAL FAR * LPFN_TRANSMITFILE)(
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

#define WSAID_TRANSMITFILE                                                   \
    {0xb5367df0,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#endif


#ifndef WSAID_TRANSMITPACKETS

/* OpenWatcom has a swapped TP_ELEMENT_FILE and TP_ELEMENT_MEMORY definition */

#ifndef TP_ELEMENT_FILE

#ifdef _MSC_VER
#pragma warning(disable:4201) /* Nonstandard extension, nameless struct/union */
#endif

typedef struct _TRANSMIT_PACKETS_ELEMENT {
	ULONG dwElFlags;
#define TP_ELEMENT_MEMORY   1
#define TP_ELEMENT_FILE     2
#define TP_ELEMENT_EOP      4
	ULONG cLength;
	union {
		struct {
			LARGE_INTEGER nFileOffset;
			HANDLE        hFile;
		};
		PVOID             pBuffer;
	};
} TRANSMIT_PACKETS_ELEMENT, *PTRANSMIT_PACKETS_ELEMENT, FAR *LPTRANSMIT_PACKETS_ELEMENT;

#ifdef _MSC_VER
#pragma warning(default:4201)
#endif

#endif

typedef BOOL (PASCAL FAR * LPFN_TRANSMITPACKETS) (
	SOCKET hSocket,
	TRANSMIT_PACKETS_ELEMENT *lpPacketArray,
	DWORD nElementCount,
	DWORD nSendSize,
	LPOVERLAPPED lpOverlapped,
	DWORD dwFlags
);

#define WSAID_TRANSMITPACKETS                                                \
		{0xd9689da0,0x1f90,0x11d3,{0x99,0x71,0x00,0xc0,0x4f,0x68,0xc8,0x76}}

#endif


#ifndef WSAID_CONNECTEX

typedef BOOL (PASCAL FAR * LPFN_CONNECTEX) (
	IN SOCKET s,
	IN const struct sockaddr FAR *name,
	IN int namelen,
	IN PVOID lpSendBuffer OPTIONAL,
	IN DWORD dwSendDataLength,
	OUT LPDWORD lpdwBytesSent,
	IN LPOVERLAPPED lpOverlapped
);

#define WSAID_CONNECTEX \
		{0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}

#endif


#ifndef WSAID_DISCONNECTEX

typedef BOOL (PASCAL FAR * LPFN_DISCONNECTEX) (
	IN SOCKET s,
	IN LPOVERLAPPED lpOverlapped,
	IN DWORD  dwFlags,
	IN DWORD  dwReserved
);

#define WSAID_DISCONNECTEX                                                   \
		{0x7fda2e11,0x8630,0x436f,{0xa0,0x31,0xf5,0x36,0xa6,0xee,0xc1,0x57}}

#endif


extern LPFN_ACCEPTEX              stu_acceptex;
extern LPFN_GETACCEPTEXSOCKADDRS  stu_getacceptexsockaddrs;
extern LPFN_TRANSMITFILE          stu_transmitfile;
extern LPFN_TRANSMITPACKETS       stu_transmitpackets;
extern LPFN_CONNECTEX             stu_connectex;
extern LPFN_DISCONNECTEX          stu_disconnectex;

#endif /* STUDEASE_CN_CORE_WIN32_STU_SOCKET_H_ */
