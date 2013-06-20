#ifndef _ALL_DEFINED_SERVER_H_
#define _ALL_DEFINED_SERVER_H_

#include <winsock2.h> 
#include <ws2tcpip.h>
#include "..\\common\\tpipv6.h"
#include <afxtempl.h>

#include "..\\common\\AllCommonDefines.h"

typedef struct _BUFFER_OBJ {
	char*   buf;                // Data buffer for data
	int		buflen;             // Length of buffer or number of bytes contained in buffer	
	struct _BUFFER_OBJ* next;   // Used to maintain a linked list of buffers
} BUFFER_OBJ;

/*
typedef struct _SOCKET_OBJ {
	char*				m_pBuff;
	int					m_iBuffLength;					///< 모아놓은(m_pBuff) 데이터의 크기	
	char                m_szIP[15+1];
	char                m_szAwayInfo[1+1];
	char                m_szUserId[20+1];
	S_COMMON_HEADER		sPacketHeader;
	int					nTotalOnePacketLen;  // 전체 데이터 길이
	int					nRecvedDataLen; // 수신된 데이터 길이
	SOCKET				s;              // Socket handle
	HANDLE				event;          // Event handle
	int					listening;      // Socket is a listening socket (TCP)
	int					closing;        // Indicates whether the connection is closing
	SOCKADDR_STORAGE	addr;			// Used for client's remote address	
	int					addrlen;		// Length of the address	
	//전송할 데이터 큐.
	BUFFER_OBJ			*pending,       // List of pending buffers to be sent
						*pendingtail;   // Last entry in buffer list
	
	struct _SOCKET_OBJ	*next,			// Used to link socket objects together
	                	*prev;
} SOCKET_OBJ;
*/

#endif
