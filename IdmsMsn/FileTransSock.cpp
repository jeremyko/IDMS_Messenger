// FileTransSock.cpp : implementation file
//

#include "stdafx.h"
//#include "FileTransferDlg.h"
#include "IdmsMsnDlg.h"
#include "FileTransSock.h"
#include "FileSendDlg.h"
#include <process.h>
#include <Windows.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Ws2_32.lib")

//GLOBAL
int
gAddressFamily = AF_UNSPEC,       	// default to unspecified
gSocketType    = SOCK_STREAM,       // default to TCP socket type
gProtocol      = IPPROTO_TCP,       // default to TCP protocol
gBufferSize    = MAX_BUFF_SIZE;

// Statistics counters
LONG	gTotalConnections=0,	//���� ���Ӽ�
		gCurrentConnections=0;	//���� ���Ӽ�

LONG gBytesRead=0,
     gBytesSent=0,
     gStartTime=0,
     gBytesReadLast=0,
     gBytesSentLast=0,
     gStartTimeLast=0;

THREAD_OBJ_FILE *gChildThreads=NULL;        // thread objects ����Ʈ 
int         gChildThreadsCount=0;      // Number of child threads created

CFileTransSock* g_thisCThreadSock;
//GLOBAL

/////////////////////
CSockObjMapInfoFile::CSockObjMapInfoFile()
{	
}

CSockObjMapInfoFile::~CSockObjMapInfoFile()
{
}


CFileTransSock::CFileTransSock()
{
	m_nSockIdIndex = 0;
	//m_iBuffLength = 0;
	g_thisCThreadSock = this; 	

	char		name[255];	
	PHOSTENT	hostinfo;	
    if( gethostname ( name, sizeof(name)) == 0)
	{
		if((hostinfo = gethostbyname(name)) != NULL)
		{
			strcpy(m_szMyIPAddr , inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list) ) ;	
			TRACE("m_szMyIPAddr [%s]\n", m_szMyIPAddr );
		}
	}
	
	::InitializeCriticalSection( &m_CS ); 
}

CFileTransSock::CFileTransSock(HWND hOwnerWnd)
{	
	m_hOwnerWnd = hOwnerWnd;	
	g_thisCThreadSock = this; 
	m_nSockIdIndex = 0;

	::InitializeCriticalSection( &m_CS ); 
}


CFileTransSock::~CFileTransSock(void)
{
	// claear
	CSockObjMapInfoFile * pa1 = NULL ;
	POSITION pos;
	
	TRACE("Cnt [%d]\n", mSockObjMap.GetCount() );

	for( pos = mSockObjMap.GetStartPosition(); pos != NULL; )
	{		
		if(pos)
		{
			CString key;
			mSockObjMap.GetNextAssoc( pos, key, (CObject*&)pa1 );			

			if(pa1)
			{				
				TRACE(" --- CSockObjMapInfo delete !\n");				
				
				delete pa1;
				pa1= NULL;				
			}

			key.Empty();
		}		
	}
	mSockObjMap.RemoveAll();

}

unsigned __stdcall PThreadProc( void* pParam )
{
	CFileTransSock* _thread = static_cast<CFileTransSock*>(pParam);
	unsigned result = 0;
	
	if (_thread)
	{
		try	
		{
			result = _thread->_run();
		}
		catch(...)	
		{
			TRACE(_T("PWorkThreadProc : exception occured\n"));
			result = -1;
		}
	}

	return result;
}



// �õ�.. ������ ����
int	CFileTransSock::Start()
{	
	_thread = (HANDLE)::_beginthreadex(0, 0, PThreadProc, (void*)this, 0, 0);
	
	if (0 == _thread)
	{
		TRACE(_T("CThreadPushWorkAsync::Start() : _beginthreadex failure \n"));
		
		return -1;
	}
	
	return 0;
}

void CFileTransSock::SetParentWnd(HWND hWnd)
{
	m_pHWnd = hWnd;	
}

int	CFileTransSock::Stop(bool wait)
{	
	
	SOCKET_OBJ_FILE *cpyPsockObj = NULL;
	THREAD_OBJ_FILE *cpyPthreadObj = NULL;
	
	cpyPthreadObj = gChildThreads;		
	
	for(int i = 0; i < gChildThreadsCount; i++)
	{
		if(cpyPsockObj && (DWORD)cpyPsockObj != 0xfeeefeee)			
		{
			cpyPsockObj = cpyPthreadObj->SocketList;			
			
			for(int j = 0; j < cpyPthreadObj->SocketCount; j++)
			{
				if(cpyPsockObj && (DWORD)cpyPsockObj != 0xfeeefeee)				
				{
					

					RemoveSocketObj(cpyPthreadObj, cpyPsockObj);
					FreeSocketObj(cpyPsockObj);
					
					
					cpyPsockObj = cpyPsockObj->next;
				}								
			}
			
			FreeThreadObj(cpyPthreadObj);
			cpyPthreadObj = cpyPthreadObj->next;		
		}
	}	

	// claear
	CSockObjMapInfoFile * pa1;
	POSITION pos;
	
	TRACE("Cnt [%d]\n", mSockObjMap.GetCount() );

	for( pos = mSockObjMap.GetStartPosition(); pos != NULL; )
	{		
		if(pos)
		{
			CString key;
			mSockObjMap.GetNextAssoc( pos, key, (CObject*&)pa1 );			

			if(pa1)
			{				
				TRACE(" --- CSockObjMapInfo delete !\n");				
				
				delete pa1;
				pa1= NULL;
			}

			key.Empty();
		}		
	}
	
	mSockObjMap.RemoveAll();
	
	return 1;
}


unsigned	CFileTransSock::_run()
{
	WSADATA          wsd;
    THREAD_OBJ_FILE      *thread=NULL;    
    SOCKET_OBJ_FILE      *sockobj=NULL, *newsock=NULL;                    
    int              index, rc;
    
    // Load Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{		
		TRACE("Unable to load Winsock!");		
		
        return 0;
    }
		
	SOCKADDR_IN sockAddr;

	// TODO: Add your control notification handler code here
	memset( &sockAddr, 0, sizeof( sockAddr ) );
	
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	sockAddr.sin_port = htons( (u_short) C_FILE_SVR_PORT );	
		
    thread = GetThreadObj(); //������ ��ü ����.    
                
    sockobj = GetSocketObj(INVALID_SOCKET, TRUE );
		
	sockobj->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		
	
	if (sockobj->s == INVALID_SOCKET) 
	{			
		TRACE("socket failed: %d\n", WSAGetLastError());		
		
		return 0;
	}

	//�����忡 ���� ��ü ����
	InsertSocketObj(thread, sockobj);
	rc = bind( sockobj->s, (SOCKADDR*)&sockAddr, sizeof( sockAddr ) );
		
	if (rc == SOCKET_ERROR)
	{            		
		TRACE("bind failed: %d\n", WSAGetLastError());
		
		return 0;
	}
	
	rc = listen(sockobj->s, 200);

	if (rc == SOCKET_ERROR)
	{                
		TRACE("listen failed: %d\n", WSAGetLastError());
		
		return 0;
	}

	// socket �̺�Ʈ ���
	rc = WSAEventSelect(sockobj->s, sockobj->event, FD_ACCEPT | FD_CLOSE);
	if (rc == SOCKET_ERROR)
	{                
		TRACE("WSAEventSelect failed: %d\n", WSAGetLastError());
		
		return 0;
	}
	
    while (1) 
	{		
		TRACE("2\n");
    	//event�� ��ٸ�        
		rc = WSAWaitForMultipleEvents(thread->SocketCount + 1, thread->Handles, FALSE, WSA_INFINITE, FALSE);		
		
        if (rc == WAIT_FAILED) 
		{			
			TRACE("WaitForMultipleObjects failed: %d\n", GetLastError());			
            break;
        }
		else if (rc == WAIT_TIMEOUT) 
		{
			TRACE("WAIT_TIMEOUT\n");            
        }
		else
		{
            index = rc - WAIT_OBJECT_0;
            sockobj = FindSocketObj(thread, index-1); // �̺�Ʈ�� �߻��� ���� ��ü ����
			            
			SOCKADDR_STORAGE sa;
			WSANETWORKEVENTS ne;
			SOCKET           sc;
			int              salen;
			
			rc = WSAEnumNetworkEvents(sockobj->s, thread->Handles[index], &ne);
			
			if (rc == SOCKET_ERROR){                    				
				TRACE("WSAEnumNetworkEvents failed: %d\n", WSAGetLastError());
				break;
			}
			
			while (1) {
				sc = INVALID_SOCKET;
				salen = sizeof(sa);                   
				
				// FD_ACCEPT | FD_CLOSE �� �̺�Ʈ�� ����Ͽ���..
				sc = accept( sockobj->s, (SOCKADDR *)&sa, &salen); //accept �Ѵ�
				
				if ((sc == INVALID_SOCKET) && (WSAGetLastError() != WSAEWOULDBLOCK)) 
				{                        
					TRACE("accept failed: %d\n", WSAGetLastError());
					break;
				}
				else if (sc != INVALID_SOCKET)
				{
					newsock = GetSocketObj(INVALID_SOCKET, FALSE);
					
					memcpy(&newsock->addr, &sa, salen);
					newsock->addrlen = salen;
					newsock->s = sc;
					InterlockedIncrement(&gTotalConnections);
					InterlockedIncrement(&gCurrentConnections);
										
					rc = WSAEventSelect( newsock->s, newsock->event, FD_READ | FD_WRITE | FD_CLOSE);
					
					if (rc == SOCKET_ERROR) 
					{						
						TRACE("WSAEventSelect failed: %d\n", WSAGetLastError());
						
						break;
					} 
					
					strcpy(newsock->m_szIP, ::inet_ntoa(((SOCKADDR_IN*)&sa)->sin_addr));
					TRACE("newsock->m_szIP [%s]\n", newsock->m_szIP );
									
					
					CSockObjMapInfoFile *pSpckObjMap = new CSockObjMapInfoFile;
										
					mSockObjMap.SetAt(newsock->m_szSockObjID, pSpckObjMap);

					//�����忡 �Ҵ�	
					pSpckObjMap->m_pSockObj = newsock;								
					
					pSpckObjMap->m_pThreadObj = AssignToFreeThread(newsock);	
					
					TRACE("�� accept : newsock->m_szIP [%s] m_szSockObjID [%s]\n", newsock->m_szIP, newsock->m_szSockObjID);					
				}
				else
				{
					// Failed with WSAEWOULDBLOCK -- just continue
					break;
				}					
			}            
        }		
    }
    WSACleanup();
    
	return 0;
}

CSockObjMapInfoFile *  CFileTransSock::SocketConnect(UINT nSocketPort,  char* lpSocketAddress  )
{	
	struct sockaddr_in addr;			
	struct	hostent *hent;	
	long	laddr;
	LPIN_ADDR paddr;
	IN_ADDR       addrHost;

	//m_iBuffLength = 0;	
			
	paddr = &addrHost;
	laddr = ::inet_addr( lpSocketAddress );
	if( laddr == INADDR_NONE ) 
	{ 
		if ( !(hent = ::gethostbyname(lpSocketAddress)) ) 
		{
			TRACE("Invalid server hostname( %s ), error #%d\n", lpSocketAddress, GetLastError() );
			return NULL;
		}
		memcpy( paddr, hent->h_addr, sizeof(IN_ADDR) );
	} 
	else 
	{
		memcpy( &paddr->s_addr, &laddr, 4);
	}

    if ((m_hSocket = socket(PF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
    {
		TRACE("�� socket ��������\n");
        
        return NULL;
    }
    
    ZeroMemory((char*) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(lpSocketAddress);
    addr.sin_port = htons((u_short)nSocketPort);	
    
    u_long    arg = 1;
    ioctlsocket(m_hSocket, FIONBIO, &arg); // non-blocking mode�� ����	
	struct  timeval     timevalue;	
	fd_set writefds;	

	//TRACE("�� connect �õ� [%s][%d]\n", GetCurrentTimeByString(),m_hSocket);

	connect(m_hSocket, (struct sockaddr*) &addr, sizeof(addr));

	FD_ZERO( &writefds );
	FD_SET( m_hSocket, &writefds );
	timevalue.tv_sec = 0;
	timevalue.tv_usec = 100000; // 0.1 sec 3000000;

	::select( 0, NULL, &writefds, NULL, &timevalue );

	if ( !FD_ISSET( m_hSocket, &writefds ) )
	{		
		TRACE("�� connect ���� [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return NULL;
	}	
	
	TRACE("�� connect ���� [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
	
	THREAD_OBJ_FILE      *thread=NULL;    
    SOCKET_OBJ_FILE      *newsock=NULL;                    
    int             rc;
    		
    thread = GetThreadObj(); //������ ��ü ����. 	
		
	newsock = GetSocketObj(INVALID_SOCKET, FALSE);
	newsock->s = m_hSocket;

	if (newsock->s == INVALID_SOCKET) 
	{			
		TRACE("socket failed: %d\n", WSAGetLastError());		
		
		return NULL;
	}

	//�����忡 ���� ��ü ����
	InsertSocketObj(thread, newsock);

	InterlockedIncrement(&gTotalConnections);
	InterlockedIncrement(&gCurrentConnections);
			
	// ���Ͽ� read, write ,close �̺�Ʈ ���
	rc = WSAEventSelect( newsock->s, newsock->event, FD_READ | FD_WRITE | FD_CLOSE);
	
	if (rc == SOCKET_ERROR) 
	{						
		TRACE("WSAEventSelect failed: %d\n", WSAGetLastError());
		
		return NULL;
	}

	//Map �� ����					
	CSockObjMapInfoFile * pSpckObjMap = new CSockObjMapInfoFile;
	pSpckObjMap->m_pSockObj = newsock;					
	pSpckObjMap->m_pThreadObj = NULL;
			
	mSockObjMap.SetAt(newsock->m_szSockObjID, pSpckObjMap);	

	//�����忡 �Ҵ� �� return!!						
	strcpy(newsock->m_szIP, lpSocketAddress);
	THREAD_OBJ_FILE* pThreadObj = NULL;
	pSpckObjMap->m_pThreadObj = AssignToFreeThread(newsock);	

	//return TRUE;	
	return pSpckObjMap ;
}


CString	CFileTransSock::GetCurrentTimeByString(BOOL bOnlyTime)
{
	CString str;
	COleDateTime dt;

	dt = COleDateTime::GetCurrentTime();

	if(bOnlyTime)
		str = dt.Format("%H:%M:%S");
	else
		str = dt.Format("%Y-%m-%d %H:%M:%S");
		
	return str;
}


BUFFER_OBJ_FILE * CFileTransSock::GetBufferObj(int buflen)
{
    BUFFER_OBJ_FILE *newobj=NULL;

    // Allocate the object
    newobj = (BUFFER_OBJ_FILE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BUFFER_OBJ_FILE));
    
	if (newobj == NULL)
	{        
		TRACE("GetBufferObj: HeapAlloc failed: %d\n", GetLastError());
        ExitProcess(-1);
    }
    // Allocate the buffer
    newobj->buf = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE) *buflen);
    if (newobj->buf == NULL)
	{
		TRACE("GetBufferObj: HeapAlloc failed: %d\n", GetLastError());
        
        ExitProcess(-1);
    }
    
	newobj->buflen = buflen; 
	newobj->bIsFileData = FALSE;
	//newobj->bIsFileOpened = FALSE;	
	newobj->dwReadFilePosition = 0;
	newobj->dwTotalFileLength = 0;	
	newobj->dwReadLength = 0;

    return newobj;
}


//    Free the buffer object.

void CFileTransSock::FreeBufferObj(BUFFER_OBJ_FILE *obj)
{
    HeapFree(GetProcessHeap(), 0, obj->buf);
    HeapFree(GetProcessHeap(), 0, obj);
}


SOCKET_OBJ_FILE * CFileTransSock::GetSocketObj(SOCKET s, int listening)
{
    SOCKET_OBJ_FILE  *sockobj=NULL;
    
	//�޸� �Ҵ�
    sockobj = (SOCKET_OBJ_FILE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SOCKET_OBJ_FILE ));
    
	if (sockobj == NULL) 
	{
		TRACE("GetSocketObj: HeapAlloc failed: %d\n", GetLastError());
        
        ExitProcess(-1);
    }

    // member �ʱ�ȭ
    sockobj->s = s;
    sockobj->listening = listening; //TCP ��� TRUE..
    sockobj->addrlen = sizeof(sockobj->addr);

    sockobj->event = WSACreateEvent(); //�̺�Ʈ ����
    if (sockobj->event == NULL) {
        
		TRACE("GetSocketObj: WSACreateEvent failed: %d\n", WSAGetLastError());
        ExitProcess(-1);
    }
	
	sockobj->nTotalOnePacketLen = 0;		
	sockobj->m_HwndFileRecv = NULL;
	sockobj->m_HwndFileSend = NULL;
	
	m_nSockIdIndex++ ;
	if(m_nSockIdIndex>= 99999)
	{
		m_nSockIdIndex = 0; //reset
	}

	sprintf(sockobj->m_szSockObjID, "%d", m_nSockIdIndex );

	sockobj->m_bIsRealFileByteData = FALSE;
	memset(sockobj->m_szLocalFilePath, 0x00, sizeof(sockobj->m_szLocalFilePath));

	TRACE("sockobj->m_szSockObjID [%s]\n", sockobj->m_szSockObjID );

    return sockobj;
}



//    Frees a socket object along with any queued buffer objects.

void CFileTransSock::FreeSocketObj(SOCKET_OBJ_FILE *obj)
{	
    BUFFER_OBJ_FILE  *ptr=NULL,  *tmp=NULL;

    ptr = obj->pending;
    
	while (ptr)
	{
        tmp = ptr;
        ptr = ptr->next;

        FreeBufferObj(tmp);
    }

    WSACloseEvent(obj->event);

    if (obj->s != INVALID_SOCKET)
	{		
        closesocket(obj->s);
    }
		
	if(obj->m_pBuff)
	{
		delete[] obj->m_pBuff;
		obj->m_pBuff = NULL;
	}
    HeapFree(GetProcessHeap(), 0, obj);
}



//    Allocate a thread object and initializes its members.

THREAD_OBJ_FILE * CFileTransSock::GetThreadObj()
{
    THREAD_OBJ_FILE *thread = NULL;
	
	//������ ��ü ���� �޸� �Ҵ�
    thread = (THREAD_OBJ_FILE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(THREAD_OBJ_FILE));
    
	if (thread == NULL)
	{
		TRACE("GetThreadObj: HeapAlloc failed: %d\n", GetLastError());
        
        ExitProcess(-1);
    }
    
	thread->Event = WSACreateEvent(); //�̺�Ʈ ����
    
	if (thread->Event == NULL)
	{
		TRACE("GetThreadObj: WSACreateEvent failed: %d\n", WSAGetLastError());
        
        ExitProcess(-1);
    }
	//socket event handle�� ù��°�� ���ο� client�� �����忡 �Ҵ�Ǿ����� �˸��� ���� event ����.
    thread->Handles[0] = thread->Event; 

    InitializeCriticalSection(&thread->ThreadCritSec);

    return thread;
}



//    Free a thread object and is member fields.

void CFileTransSock::FreeThreadObj(THREAD_OBJ_FILE *thread)
{
    WSACloseEvent(thread->Event);

    CloseHandle(thread->Thread);

    DeleteCriticalSection(&thread->ThreadCritSec);

    HeapFree(GetProcessHeap(), 0, thread);
}


// 	���� ��ü�� �־��� �������� ���� ��ü ����Ʈ�� �߰��Ѵ�.

int CFileTransSock::InsertSocketObj(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock)
{
    int     ret;
    
	EnterCriticalSection(&thread->ThreadCritSec);
    
	//�ִ� ��� �̺�Ʈ���� ������ ����Ʈ�� �߰����� ����.
    if (thread->SocketCount < MAXIMUM_WAIT_OBJECTS-1)
	{	
        sock->next = sock->prev = NULL;
        
		if (thread->SocketList == NULL) 
		{
            // List�� �����
            thread->SocketList = thread->SocketListEnd = sock;
        }
		else
		{
            // ����Ʈ�� ��ü ����, ����Ʈ ���� �߰�
            sock->prev = thread->SocketListEnd;
            thread->SocketListEnd->next = sock;
            thread->SocketListEnd = sock;
        }
        
        // ������ �̺�Ʈ�� ������ �̺�Ʈ ����Ʈ�� �Ҵ�.
        thread->Handles[thread->SocketCount + 1] = sock->event;
        thread->SocketCount++;

        ret = NO_ERROR;
    }
	else
	{
		//�ִ� ��� �̺�Ʈ���� ������ ����Ʈ�� �߰����� ����.
        ret = SOCKET_ERROR;
    }

    LeaveCriticalSection(&thread->ThreadCritSec);

    return ret;
}


//    Remove a socket object from the list of sockets for the given thread.

void CFileTransSock::RemoveSocketObj(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock)
{
    EnterCriticalSection(&thread->ThreadCritSec);
    
	if (sock->prev)
	{
        sock->prev->next = sock->next;
    }
    
	if (sock->next)
	{
        sock->next->prev = sock->prev;
    }

    if (thread->SocketList == sock)
	{
        thread->SocketList = sock->next;
	}

    if (thread->SocketListEnd == sock)
	{
        thread->SocketListEnd = sock->prev;
	}

    thread->SocketCount--;

    // Signal thread to rebuild array of events
    WSASetEvent(thread->Event);

    InterlockedDecrement(&gCurrentConnections);

    LeaveCriticalSection(&thread->ThreadCritSec);
}


SOCKET_OBJ_FILE * CFileTransSock::FindSocketObj(THREAD_OBJ_FILE *thread, int index)
{
    SOCKET_OBJ_FILE *ptr=NULL;
    int         i;

    EnterCriticalSection(&thread->ThreadCritSec);

    ptr = thread->SocketList;
    
	for(i=0; i < index ;i++)
	{
        ptr = ptr->next;
    }

    LeaveCriticalSection(&thread->ThreadCritSec);

    return ptr;
}



//   Queue up a receive buffer for this connection (socket).
void CFileTransSock::EnqueueBufferObj(SOCKET_OBJ_FILE *sock, BUFFER_OBJ_FILE *obj, BOOL AtHead)
{
    if (sock->pending == NULL)
	{
        // Queue is empty
        sock->pending = sock->pendingtail = obj;
    }
	else if (AtHead == FALSE)
	{
        // Put new object at the end 
        sock->pendingtail->next = obj;
        sock->pendingtail = obj;
    }
	else
	{
        // Put new object at the head
        obj->next = sock->pending;
        sock->pending = obj;
    }
}

 

//    Remove a BUFFER_OBJ from the given connection's queue for sending.
BUFFER_OBJ_FILE * CFileTransSock::DequeueBufferObj(SOCKET_OBJ_FILE *sock)
{
    BUFFER_OBJ_FILE *ret=NULL;

    if (sock->pendingtail != NULL)
	{
        // Queue is non empty
        ret = sock->pending;
        sock->pending = sock->pending->next;
        
		if (sock->pendingtail == ret)
		{
            // Item is the only item in the queue
            sock->pendingtail = NULL;
        }
    }

    return ret;
}


//    Send any data pending on the socket. This routine goes through the 
//    queued buffer objects within the socket object and attempts to
//    send all of them. If the send fails with WSAEWOULDBLOCK, put the
//    remaining buffer back in the queue (at the front) for sending
//    later when select indicates sends can be made. This routine returns
//    -1 to indicate that an error has occured on the socket and the
//    calling routine should remove the socket structure; otherwise, zero
//    is returned.
//
int CFileTransSock::SendPendingData(SOCKET_OBJ_FILE * sock)
{
    BUFFER_OBJ_FILE *bufobj=NULL;
    BOOL        breakouter;
    int         nleft,  idx,  ret,  rc;

    // Attempt to dequeue all the buffer objects on the socket	
    ret = 0;
	
	CFile cfile;
	CFileException ex;
	BYTE* pSzBufFile = NULL;
    while (bufobj = DequeueBufferObj(sock)) 
	{				
		breakouter = FALSE;
		
		// ���ϵ����ʹ� ������ ���� �о�鼭 ������.
		if( bufobj->bIsFileData ) // bufobj->dwTotalFileLength != 0
		{
			//����///////////////////////////////////////////////////////////////////////////////////////////
			if(bufobj->dwTotalFileLength == 0)
			{
				// ���⼭ send dlg �� ���۷� �޽��� ������ 
				if(sock->m_HwndFileSend)
				{
					TRACE("File Size 0 ! \n");
					//::SendMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
					::PostMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
					
					return TRUE;
				}
			}

			pSzBufFile = new BYTE [bufobj->dwReadLength]; 
			memset(pSzBufFile, NULL, bufobj->dwReadLength );	
						
			if (! cfile.Open( sock->m_szLocalFilePath, CFile::modeRead | CFile::shareDenyWrite, &ex))
			{				
				delete [] pSzBufFile;
				
				TCHAR szError[1024];
				ex.GetErrorMessage(szError, 1024);
				cout << "Couldn't open source file: ";
				cout << szError;
				return FALSE;
			}
			
			// ������ ����
			DWORD dwCurrentFilePosition = SetFilePointer( 								
								(HANDLE)cfile.m_hFile,
								bufobj->dwReadFilePosition,				 
								NULL,			
								FILE_BEGIN	);  
		
			TRACE("dwCurrentFilePosition [%ld]\n", dwCurrentFilePosition );
						

			UINT nBytesRead  = cfile.Read(  pSzBufFile, bufobj->dwReadLength  );
			TRACE("nBytesRead [%d]\n", nBytesRead);
		
			nleft = bufobj->dwReadLength;
			idx = 0;
			
			while (nleft) 
			{
				TRACE("1\n");
				rc = send(sock->s, (char*) pSzBufFile, nleft, 0);				
				
				if (rc == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{	
						TRACE("\n �ڡ� WSAEWOULDBLOCK!! �ڡ�\n");
						BUFFER_OBJ_FILE *newbuf=NULL;						
						newbuf = GetBufferObj( sizeof(BUFFER_OBJ_FILE) );

						newbuf->bIsFileData = TRUE;						
						newbuf->buf = NULL;
						newbuf->buflen = 0;
						newbuf->dwReadFilePosition =  bufobj->dwReadLength - nleft ;
						newbuf->dwTotalFileLength = bufobj->dwTotalFileLength ;
						newbuf->dwReadLength = nleft ; 
												
						EnqueueBufferObj(sock, bufobj, TRUE); //queue ��(head)�� ����. ������ �̾ ����.
												
						delete [] pSzBufFile;
						pSzBufFile= NULL;
						
						ret = WSAEWOULDBLOCK;
					}
					else
					{
						// The connection was broken, indicate failure
						ret = -1;
					}
					
					delete [] pSzBufFile;
					pSzBufFile= NULL;
					cfile.Close();

					goto BREAK_OUT;
					break;
				}
				else
				{				
					TRACE("File Sent [%d]\n", rc);
					
					cfile.Close();					
					delete [] pSzBufFile;
					pSzBufFile= NULL;					

					// ���⼭ send dlg �� ���۷� �޽��� ������ 
					if(sock->m_HwndFileSend)
					{
						//::SendMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
						::PostMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
					}

					nleft -= rc;
					idx += 0; //��� ..
				}
			}
			//����///////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			nleft = bufobj->buflen;
			idx = 0;
			
			while (nleft) 
			{
				rc = send(sock->s, (char*)&bufobj->buf[idx], nleft, 0);				
				
				if (rc == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						BUFFER_OBJ_FILE *newbuf=NULL;
						
						newbuf = GetBufferObj(nleft);
						memcpy(newbuf->buf, &bufobj->buf[idx], nleft);
												
						EnqueueBufferObj(sock, newbuf, TRUE); //queue ��(head)�� ����. ������ �̾ ����.
						
						ret = WSAEWOULDBLOCK;
					}
					else
					{
						// The connection was broken, indicate failure
						ret = -1;
					}
					
					goto BREAK_OUT;
					break;
				}
				else
				{				
					TRACE(" Sent [%d]\n", rc);					
					// ���⼭ send dlg �� ���۷� �޽��� ������ 
					//if(sock->m_HwndFileSend)
					//{
					//	::PostMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL); 
					//}

					nleft -= rc;
					idx += 0; //��� ..
				}
			}
		}
	
		//cfile.Close();

		//�޸� clear..
		FreeBufferObj(bufobj);
    }

BREAK_OUT :	
	
    // If no more sends are pending and the socket was marked as closing (the
    // receiver got zero bytes) then close the socket and indicate to the caller
    // to remove the socket structure.
    if ((sock->pending == NULL) && (sock->closing)) 
	{
		WSACloseEvent(sock->event);
        closesocket(sock->s);
        sock->s = INVALID_SOCKET;
        ret = -1;
    }
	
    return ret;
}


// ��� ������ �����ϴ� ��ü PAcket ���� 
BOOL CFileTransSock::GetHeaderInfo(SOCKET_OBJ_FILE * pSockObj)
{
	if(pSockObj->m_iBuffLength < FILE_HEADER_LENGTH)
	{
		TRACE("GetHeaderInfo return\n");
		return FALSE ;
	}
			
	// ��� ������ copy
	memcpy( &(pSockObj->sPacketHeader),  (FILE_HEADER *)pSockObj->m_pBuff, sizeof(FILE_HEADER));

	// ��� ���̸� �����ϴ� ��ü ������ ũ�� 
	pSockObj->nTotalOnePacketLen = atoi(pSockObj->sPacketHeader.szLength) + FILE_HEADER_LENGTH ;
	
	TRACE("��ü Packet ���� [%d]\n", pSockObj->nTotalOnePacketLen);
	
	return TRUE;
}




//Ŭ���̾�Ʈ ��û ó��
int	CFileTransSock::_do_recv(SOCKET_OBJ_FILE * sock)
{
	//TRACE("FD_READ notification\n");
	
	BYTE RecvedBytes [MAX_BUFF_SIZE];
	int receive_length = 0;	
	
	memset(RecvedBytes,0x00,sizeof(RecvedBytes));		

	if( (receive_length = recv(sock->s, (char*) RecvedBytes, MAX_BUFF_SIZE, 0)) > 0 ) 
	{   
		TRACE("receive_length [ %d (%d / %d) ] \n", receive_length, sock->m_iBuffLength, sock->nTotalOnePacketLen);
		
		if( sock->m_bIsRealFileByteData) 
		{
			// ���ϵ����ʹ� �޴´�� ����.
			sock->m_iBuffLength = sock->m_iBuffLength + receive_length;			
									
			ST_RCVED_BYTES_INFO sRecvBytes ; 
			memset(&sRecvBytes, 0x00, sizeof(sRecvBytes));
			sRecvBytes.pBuff = RecvedBytes ;
			sRecvBytes.receive_length = receive_length ;						
			sRecvBytes.sFileHeader = &(sock->sPacketHeader) ;
			sRecvBytes.sock = sock ;
			
			::SendMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) &sRecvBytes , 	(LPARAM) NULL ); 

			//////���ϵ����� ó�� /////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
			{				
				//TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");				
				return 0;
			}			
			else
			{
				// ���� ���� ������ - �ϳ��� ��Ŷ ������ CHK			
				int nExcessiveLen = sock->m_iBuffLength - sock->nTotalOnePacketLen ;
				
				ProcessRecvData(sock);
												
				if(nExcessiveLen > 0)
				{
					TRACE("�ϳ��� ��Ŷ ������ ���� ������ ������ ���� ��� => ���۸� �ٽ� ����[%d]\n", nExcessiveLen);				

					sock->m_pBuff = new BYTE[nExcessiveLen]; 
					
					if(sock->m_pBuff == NULL)
					{				
						return 0;
					}

					memcpy(sock->m_pBuff, RecvedBytes + (receive_length -nExcessiveLen)  , nExcessiveLen);				
					sock->m_iBuffLength = nExcessiveLen;
					
					while(nExcessiveLen)
					{
						if ( GetHeaderInfo(sock) == FALSE ) 
						{
							return 0;
						}

						if(strcmp( sock->sPacketHeader.szUsage, USAGE_MAKE_THIS_FILE ) == 0 )
						{
							sock->m_bIsRealFileByteData = TRUE;
						}
						else
						{
							sock->m_bIsRealFileByteData = FALSE;
						}
						
						if(sock->nTotalOnePacketLen  < 0)
						{
							TRACE("CFileTransSock::_do_recv() iOnePacketLength < 0 ..Break\n");
							
							return 0;
						}
						else if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
						{				
							//TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");				
							return 0;
						}		
						else
						{
							TRACE("�� Packet�� ������! \n");				
							ProcessRecvData(sock);					
						}
						
						nExcessiveLen = nExcessiveLen - sock->nTotalOnePacketLen ;
						
						if(nExcessiveLen == 0)
						{
							// ���޾Ұ� ��Ŷ�뷮�� �°� �� ���۵� ��� 
							if(sock->m_pBuff)
							{					
								delete[] sock->m_pBuff;
								sock->m_pBuff = NULL;
								sock->m_bIsRealFileByteData = FALSE; 
							}				
							sock->m_iBuffLength = 0;
							
							break;
						}
						else if(nExcessiveLen < 0)
						{
							// ���� ����Ŷ�� �����Ͱ� �� ���۵��� �ʾҴ�.
							// => ���޾ƾ��ϹǷ� ���۸� �������� �ʴ´�. SKIP..
							return 0;
						}
						else
						{
							memcpy(sock->m_pBuff, RecvedBytes + (receive_length -nExcessiveLen) , nExcessiveLen);
						}
					}				
				}
				else
				{				
					if(sock->m_pBuff)
					{
						TRACE("CFileTransSock::_do_recv() delete m_pBuff \n");							
						
						delete[] sock->m_pBuff;
						sock->m_pBuff = NULL;
						sock->m_bIsRealFileByteData = FALSE;
					}
					
					sock->m_iBuffLength = 0;
				}
			}
			//////���ϵ�����ó������///////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			// �޽��� �����͸� ���۸� ����
			
			BYTE* pTemp = sock->m_pBuff; // ���� ������ copy..
			
			// ���� ������ + ���� ���� �����͸�ŭ �޸� ���۸� �Ҵ��Ѵ�.		
			sock->m_pBuff = new BYTE [sock->m_iBuffLength + receive_length + 1];
			memset(sock->m_pBuff, 0x00, sock->m_iBuffLength + receive_length +1);
			
			if(sock->m_pBuff == NULL)
			{
				return 0;
			}
			
			if(sock->m_iBuffLength != 0)
			{				
				// ���� �Ҵ�� ���ۿ� ���ݱ��� ���� ������ �ٽ� ����..			
				
				memcpy(sock->m_pBuff, pTemp, sock->m_iBuffLength);
				
				if(pTemp)
				{
					// pTemp �� �ٽ� �ʱ�ȭ ���·� ����,,
					delete[] pTemp;
					pTemp = NULL;
				}
			}		
			
			// ���� ���� ������ ����		
			memcpy(sock->m_pBuff + sock->m_iBuffLength, RecvedBytes, receive_length);
			
			//TRACE("���� ������ ������ ���� ���sock->m_iBuffLength [%d] receive_length[%d]\n", sock->m_iBuffLength, receive_length);
			
			int nOldLen  = sock->m_iBuffLength ;

			sock->m_iBuffLength = sock->m_iBuffLength + receive_length;
			
			if ( GetHeaderInfo(sock) == FALSE ) 
			{
				return 0;
			}
			
			// ���� �������̸� ������ write �Ѵ�. => ���۸� �����ָ鼭 �޴´�
						
			if(strcmp( sock->sPacketHeader.szUsage, USAGE_MAKE_THIS_FILE ) == 0 )
			{
				sock->m_bIsRealFileByteData = TRUE;

				// recv dlg�� �˸� 
				if(sock->m_HwndFileRecv)
				{
					if(nOldLen == 0 ) // ó��
					{												
						TRACE("nOldLen <= FILE_HEADER_LENGTH\n");
						
						ST_RCVED_BYTES_INFO sRecvBytes ; 
						memset(&sRecvBytes, 0x00, sizeof(sRecvBytes));
						sRecvBytes.pBuff = sock->m_pBuff + FILE_HEADER_LENGTH ;
						sRecvBytes.receive_length = sock->m_iBuffLength - FILE_HEADER_LENGTH ;
						sRecvBytes.sFileHeader = &(sock->sPacketHeader) ;
						sRecvBytes.sock = sock ;
						
						::SendMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) &sRecvBytes , 	(LPARAM) NULL ); 
					}
					/*
					else
					{
						TRACE("nOldLen > FILE_HEADER_LENGTH\n");
						
						ST_RCVED_BYTES_INFO sRecvBytes ; 
						memset(&sRecvBytes, 0x00, sizeof(sRecvBytes));
						sRecvBytes.pBuff = sock->m_pBuff + nOldLen ;
						sRecvBytes.receive_length = receive_length ;						
						sRecvBytes.sFileHeader = &(sock->sPacketHeader) ;
						sRecvBytes.sock = sock ;
						
						::SendMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) &sRecvBytes , 	(LPARAM) NULL ); 
					}
					*/
				}	
			}	
			else
			{
				sock->m_bIsRealFileByteData = FALSE;
			}

			if(sock->nTotalOnePacketLen  < 0)
			{
				TRACE("CFileTransSock::_do_recv() iOnePacketLength < 0 ..Break\n");
				
				return 0;
			}
			else if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
			{				
				//TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");				
				return 0;
			}			
			else
			{				
				//TRACE("\n������ �� Packet�� ������ �����Ͱ� ���� (���ŵ����� ó������) sock->nTotalOnePacketLen[%d]\n",sock->nTotalOnePacketLen);
				
				// ���� ���� ������ - �ϳ��� ��Ŷ ������ CHK			
				int nExcessiveLen = sock->m_iBuffLength - sock->nTotalOnePacketLen ;
				
				ProcessRecvData(sock);
																
				if(nExcessiveLen > 0)
				{
					TRACE("�ϳ��� ��Ŷ ������ ���� ������ ������ ���� ��� => ���۸� �ٽ� ����[%d]\n", nExcessiveLen);				
					
					pTemp = sock->m_pBuff; // �������� ���� ������, ������ ����					
					
					sock->m_pBuff = new BYTE[nExcessiveLen]; 
					
					if(sock->m_pBuff == NULL)
					{				
						return 0;
					}
					
					// ���� ������ ����, pTemp�� ���� ����Ŷ + ������ ������ ����..				
					memcpy(sock->m_pBuff, pTemp + sock->nTotalOnePacketLen , nExcessiveLen);				
					sock->m_iBuffLength = nExcessiveLen;
					
					if(pTemp)
					{					
						delete[] pTemp;
						pTemp = NULL;
					}
					
					// ������ �����͸� ó���Ѵ�. 
					// ���۰� 0 or 0���� ����(���� �� ������) �� �ɶ�����..
					while(nExcessiveLen)
					{
						//GetHeaderInfo(sock);		
						
						//////////////////////////////////////////////////////////////////////////
						if ( GetHeaderInfo(sock) == FALSE ) 
						{
							return 0;
						}

						if(strcmp( sock->sPacketHeader.szUsage, USAGE_MAKE_THIS_FILE ) == 0 )
						{
							sock->m_bIsRealFileByteData = TRUE;
						}
						else
						{
							sock->m_bIsRealFileByteData = FALSE;
						}
						//////////////////////////////////////////////////////////////////////////
						
						if(sock->nTotalOnePacketLen  < 0)
						{
							TRACE("CFileTransSock::_do_recv() iOnePacketLength < 0 ..Break\n");
							
							return 0;
						}
						else if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
						{				
							//TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");				
							return 0;
						}		
						else
						{
							TRACE("�� Packet�� ������! \n");				
							ProcessRecvData(sock);					
						}
						
						nExcessiveLen = nExcessiveLen - sock->nTotalOnePacketLen ;
						
						if(nExcessiveLen == 0)
						{
							// ���޾Ұ� ��Ŷ�뷮�� �°� �� ���۵� ��� 
							if(sock->m_pBuff)
							{					
								delete[] sock->m_pBuff;
								sock->m_pBuff = NULL;
								sock->m_bIsRealFileByteData = FALSE; 
							}				
							sock->m_iBuffLength = 0;
							
							break;
						}
						else if(nExcessiveLen < 0)
						{
							// ���� ����Ŷ�� �����Ͱ� �� ���۵��� �ʾҴ�.
							// => ���޾ƾ��ϹǷ� ���۸� �������� �ʴ´�. SKIP..
							return 0;
						}
						else
						{
							memcpy(sock->m_pBuff, sock->m_pBuff + sock->nTotalOnePacketLen , nExcessiveLen);
						}
					}				
				}
				else
				{				
					if(sock->m_pBuff)
					{
						TRACE("CFileTransSock::_do_recv() delete m_pBuff \n");							
						
						delete[] sock->m_pBuff;
						sock->m_pBuff = NULL;
						sock->m_bIsRealFileByteData = FALSE;
					}
					
					sock->m_iBuffLength = 0;
				}
			}
		}		
	}
	else if( receive_length == SOCKET_ERROR )
	{
		int nRet = GetLastError();
		
		if(nRet == WSAEWOULDBLOCK)
		{	
			TRACE("\n������ ���� WSAEWOULDBLOCK ����\n");
			return 1;
		}		
		else
		{
			TRACE("\n������ ERROR : ���� ���� [%d] \n", nRet);
			//break;			
			return -1;
		}
	}
	else if( receive_length == 0 ) 
	{			
		TRACE("\n������ receive_length == 0  \n");
		int nRet = GetLastError();
		//break;
		return -1; // Socket Close..
	}		
	
	TRACE("������ ���� ����\n");
	
	return 1;
	
}





/*
						ST_RCVED_BYTES_INFO* pRecvBytes =  new ST_RCVED_BYTES_INFO ; 
						pRecvBytes->pBuff = sock->m_pBuff + FILE_HEADER_LENGTH ;
						pRecvBytes->receive_length = sock->m_iBuffLength - FILE_HEADER_LENGTH ;
						pRecvBytes->sFileHeader = &sFileHeader ;
						pRecvBytes->sock = sock ;
					
						::PostMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) pRecvBytes , 	(LPARAM) NULL ); 
						*/

						/*
						ST_RCVED_BYTES_INFO* pRecvBytes =  new ST_RCVED_BYTES_INFO ; 
						pRecvBytes->pBuff = sock->m_pBuff + nOldLen ;
						pRecvBytes->receive_length = receive_length ;
						pRecvBytes->sFileHeader = &sFileHeader ;
						pRecvBytes->sock = sock ;
					
						::PostMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) pRecvBytes , 	(LPARAM) NULL ); 					
						*/


int  CFileTransSock::HandleIo(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock)
{
    WSANETWORKEVENTS nevents;
    int              rc;

    // Enumerate the events
    rc = WSAEnumNetworkEvents(sock->s, sock->event, &nevents);
    
	if (rc == SOCKET_ERROR)
	{

		TRACE("HandleIo: WSAEnumNetworkEvents failed: %d\n", WSAGetLastError());
        return SOCKET_ERROR;
    }

    if (nevents.lNetworkEvents & FD_READ) 
	{
        // Check for read error
        if (nevents.iErrorCode[FD_READ_BIT] == 0) 
		{
			//recv 						
			rc = _do_recv(sock); 

			if (rc == -1)
			{
				EnterCriticalSection(&m_CS); 
				TRACE("_do_recv() return -1\n");
                RemoveSocketObj(thread, sock);
                FreeSocketObj(sock);
				LeaveCriticalSection(&m_CS); 
                return SOCKET_ERROR;
            }	
			
        }
		else
		{
            //ERROR..
			EnterCriticalSection(&m_CS); 
			TRACE("HandleIo: FD_READ error %d\n", nevents.iErrorCode[FD_READ_BIT]);
            RemoveSocketObj(thread, sock);
            FreeSocketObj(sock);
			LeaveCriticalSection(&m_CS); 

            return SOCKET_ERROR;
        }
    }
    
	if (nevents.lNetworkEvents & FD_WRITE) 
	{        
		// Check for write error
        if (nevents.iErrorCode[FD_WRITE_BIT] == 0) 
		{			
			EnterCriticalSection(&m_CS); 
            rc = SendPendingData(sock);
			LeaveCriticalSection(&m_CS); 
            
			if (rc == -1)
			{
				EnterCriticalSection(&m_CS); 
                RemoveSocketObj(thread, sock);
                FreeSocketObj(sock);
				LeaveCriticalSection(&m_CS); 

                return SOCKET_ERROR;
            }
			
        }else{
            
			TRACE("HandleIo: FD_WRITE error %d\n", nevents.iErrorCode[FD_WRITE_BIT]);
            return SOCKET_ERROR;
        }
    }
    
	if (nevents.lNetworkEvents & FD_CLOSE)
	{
        // Check for close error
        if (nevents.iErrorCode[FD_CLOSE_BIT] == 0)
		{
            // Socket has been indicated as closing so make sure all the data has been read									
			CSockObjMapInfoFile *pa2 = NULL ;			
			if(mSockObjMap.Lookup(sock->m_szSockObjID, ( CObject*& ) pa2 )) 
			{
				CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
				
				if(pSpckObjMap)
				{
					mSockObjMap.RemoveKey(sock->m_szSockObjID);
					delete pSpckObjMap;					
					pSpckObjMap = NULL;
				}
			}						
		
			ProcessWhenConnClosed(sock);

			// sock ��ü clear.
			EnterCriticalSection(&m_CS); 
			RemoveSocketObj(thread, sock);
            FreeSocketObj(sock);
			RenumberThreadArray(thread);
			LeaveCriticalSection(&m_CS); 
        }
		else
		{

			TRACE("HandleIo: FD_CLOSE error %d\n",nevents.iErrorCode[FD_CLOSE_BIT]);
			
			ProcessWhenConnClosed(sock);

			CSockObjMapInfoFile *pa2= NULL;

			if(mSockObjMap.Lookup(sock->m_szSockObjID, ( CObject*& ) pa2 )) 
			{
				CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
				
				if(pSpckObjMap)
				{
					mSockObjMap.RemoveKey(sock->m_szSockObjID);
					delete pSpckObjMap;					
					pSpckObjMap = NULL;
				}
			}				

			EnterCriticalSection(&m_CS); 
            RemoveSocketObj(thread, sock);
            FreeSocketObj(sock);
			LeaveCriticalSection(&m_CS); 
			
            return SOCKET_ERROR;
        }
    }
    return NO_ERROR;
}


// �������� socket event �迭�� ������ ���� ��ü�� �̺�Ʈ�� ��� �ٽ� �����Ѵ�(rebuild)
void CFileTransSock::RenumberThreadArray(THREAD_OBJ_FILE *thread)
{
    SOCKET_OBJ_FILE *sptr=NULL;
    int         i;

    EnterCriticalSection(&thread->ThreadCritSec);
    i = 0;
    sptr = thread->SocketList;

	while (sptr && (DWORD)sptr != 0xfeeefeee ) 
	{
		thread->Handles[i+1] = sptr->event;
		i++;
		sptr = sptr->next;
	}	
    
    LeaveCriticalSection(&thread->ThreadCritSec);
}

// Ŭ���̾�Ʈ�� ��û�� ó���� ������(�̹� Accept��)
//DWORD WINAPI ChildThread(LPVOID lpParam)
unsigned __stdcall ChildThread( void* lpParam )
{
    THREAD_OBJ_FILE *thread=NULL;
    SOCKET_OBJ_FILE *sptr=NULL,   *sockobj=NULL;
    int         index,  rc,   i;

	thread = static_cast<THREAD_OBJ_FILE *>(lpParam);

    while (1) 
	{
        rc = WaitForMultipleObjects(thread->SocketCount + 1, thread->Handles, FALSE, INFINITE);
    
		if (rc == WAIT_FAILED || rc == WAIT_TIMEOUT) 
		{
			rc = GetLastError();
            if(rc == ERROR_INVALID_HANDLE)
				TRACE("ChildThread: WaitForMultipleObjects failed: ERROR_INVALID_HANDLE\n");
			else
				TRACE("ChildThread: WaitForMultipleObjects failed: %d\n", rc);
            break;
        }
		else
		{
            // ���ÿ� �������� �̺�Ʈ���� set�� ��� ���, ��� �˻��Ѵ�.            
            for(i=0; i < thread->SocketCount + 1 ;i++)
			{
                rc = WaitForSingleObject(thread->Handles[i], 0);
                if (rc == WAIT_FAILED)
				{
					TRACE("ChildThread: WaitForSingleObject failed: %d\n", GetLastError());
                    ExitThread(-1);
                }
				else 
				if (rc == WAIT_TIMEOUT)
				{                    
                    continue;
                }
                
				index = i;
				
                if (index == 0) 
				{                    
                    WSAResetEvent(thread->Handles[index]);
                    g_thisCThreadSock->RenumberThreadArray(thread);                     
                    i = 1;
                }
				else 
				{                    
                    sockobj = g_thisCThreadSock->FindSocketObj(thread, index-1);
                    if (sockobj != NULL)
					{                    	
                        if (g_thisCThreadSock->HandleIo(thread, sockobj) == SOCKET_ERROR) 
						{
                            g_thisCThreadSock->RenumberThreadArray(thread);
                        }
                    }
					else
					{                        
						TRACE("Unable to find socket object!");
                    }
                }
            }
        }
    }
    ExitThread(0);
    return 0;
}


THREAD_OBJ_FILE * CFileTransSock::AssignToFreeThread(SOCKET_OBJ_FILE *sock)
{
    THREAD_OBJ_FILE *thread=NULL;

    thread = gChildThreads;
    
	// thread ���� ����Ʈ�� ���鼭 ���� ��ü�� ����
    while (thread) 
	{
        // SOCKET_ERROR �� ��ȯ���� �ʴ´ٸ� child thread�� �Ҵ��� �����Ѱ�.        
        if (InsertSocketObj(thread, sock) != SOCKET_ERROR)
            break;
        thread = thread->next;
    }

    if (thread == NULL) 
	{        
		TRACE("Creating new thread object\n");		
		
        thread = GetThreadObj(); //�޸� �Ҵ���,

		//pOut = thread;
		
		TRACE("�� AssignToFreeThread: CreateThread \n");
        //thread->Thread = ::CreateThread(NULL, 0, ChildThread, (LPVOID)thread, 0, NULL); 
		unsigned threadID = 0;
		thread->Thread = (HANDLE) _beginthreadex(NULL, 0, &ChildThread, (LPVOID)thread, 0, &threadID ) ;				
        
		if (thread->Thread == NULL)
		{
			TRACE("AssignToFreeThread: CreateThread failed: %d\n", GetLastError());
            ExitProcess(-1);
        }
        
		InsertSocketObj(thread, sock); //������ �����尴ü�� ���� ��ü �߰�
        
		// ������ ������� ������ ����Ʈ�� �߰�.
        if (gChildThreads == NULL)
		{
            gChildThreads = thread;
        }
		else
		{
            thread->next = gChildThreads;
            gChildThreads = thread;
        }
        
		gChildThreadsCount++;
    } 

    // child thread�� event list�� �籸���ϰ� �Ѵ�.
    WSASetEvent(thread->Event);	

	return thread ;
}


int CFileTransSock::Init(HWND p_hWnd_dlg)
{	
	g_thisCThreadSock = this; 
	
	return 0;
}

BOOL CFileTransSock::CancelSendFile	(CSockObjMapInfoFile * pSpckObjMap)
{
	

	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];
	memset(szTemp, 0x00, sizeof(szTemp));
	TRACE("DenyTransferFile\n");

	memset(&sFileHeader, NULL, sizeof(sFileHeader));
					
	// ����� ����	
	memcpy(sFileHeader.szUsage, (char*)USAGE_CANCEL_FILE_TRANSFER, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSpckObjMap->m_pSockObj->m_szIP, sizeof(pSpckObjMap->m_pSockObj->m_szIP) ); 
			
	nTotalLen = FILE_HEADER_LENGTH  ;					
		
	// ���۵����� ������
	pSzBuf = new BYTE [FILE_HEADER_LENGTH ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);		

	BUFFER_OBJ_FILE *newbuf = NULL;
	newbuf = GetBufferObj(nTotalLen);					
	memcpy(newbuf->buf, pSzBuf, nTotalLen);
	
	::EnterCriticalSection(&m_CS); 
	EnqueueBufferObj(pSpckObjMap->m_pSockObj, newbuf, FALSE); 
	::LeaveCriticalSection(&m_CS); 
	
	if(pSzBuf)
	{
		delete[] pSzBuf; 
		pSzBuf = NULL;
	}
	
	::EnterCriticalSection(&m_CS); 
	int rc = SendPendingData(pSpckObjMap->m_pSockObj);
	
	if (rc == -1) 
	{
		RemoveSocketObj(pSpckObjMap->m_pThreadObj, pSpckObjMap->m_pSockObj);
		FreeSocketObj(pSpckObjMap->m_pSockObj);					
	}	
	::LeaveCriticalSection(&m_CS); 

	return TRUE;
}



BOOL CFileTransSock::InformFileSend( CSockObjMapInfoFile * pSpckObjMap, char* szToId,CString& strToIP, char* szFilePath, int nFileCnt)
{	
	// �������� ������ ������ ���� 	
	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];	
	memset(szTemp, 0x00, sizeof(szTemp));
	CString strMsg;
	
	TRACE("InformFileSend [%s]\n", szFilePath);

	memset(&sFileHeader, NULL, sizeof(sFileHeader));
	
	iLength = strlen(szFilePath) + 1; // +1 NULL	

	memset(&sFileHeader, NULL, sizeof(sFileHeader));		
	// ����� ����	
	memcpy(sFileHeader.szUsage, (char*)USAGE_TAKE_FILE, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, CChatSession::Instance().m_szMyUserID /*m_szMyIPAddr*/,
			min(sizeof(sFileHeader.szFromID), strlen(CChatSession::Instance().m_szMyUserID))); 

	memcpy(sFileHeader.szFromName, (LPCSTR) CChatSession::Instance().m_LoginName , //20080710
			min(sizeof(sFileHeader.szFromName), strlen( (LPCSTR) CChatSession::Instance().m_LoginName))); 

	memcpy(sFileHeader.szToIP, (LPCSTR)strToIP, strToIP.GetLength() ); 

	// ���۵����� ������
	pSzBuf = new BYTE [FILE_HEADER_LENGTH + iLength]; //!!!
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);
	memcpy(pSzBuf + FILE_HEADER_LENGTH, szFilePath, iLength);
	nTotalLen = FILE_HEADER_LENGTH + iLength;
			
	BUFFER_OBJ_FILE *newbuf = NULL;

	// �����Ͱ� ū ��� ���ۿ� ���� �����Ѵ�.
	int nTimes = nTotalLen / MAX_BUFF_SIZE ;

	if(nTotalLen >= MAX_BUFF_SIZE )
	{				
		int nOffSet = 0;				
				
		for ( int i=0; i < nTotalLen / MAX_BUFF_SIZE ; i++ )
		{			
			newbuf = NULL;
			newbuf = CFileTransSock::Instance().GetBufferObj( MAX_BUFF_SIZE );
		
			memcpy(newbuf->buf, pSzBuf + nOffSet , MAX_BUFF_SIZE);
			
			::EnterCriticalSection(& m_CS); 
			CFileTransSock::Instance().EnqueueBufferObj( pSpckObjMap->m_pSockObj, newbuf , FALSE); 
			::LeaveCriticalSection(& m_CS); 			
			
			nOffSet+= MAX_BUFF_SIZE ;
		}
		
		if( (nTotalLen % MAX_BUFF_SIZE) > 0 )
		{
			newbuf = CFileTransSock::Instance().GetBufferObj( nTotalLen % MAX_BUFF_SIZE );
			memcpy(newbuf->buf, pSzBuf + nOffSet , nTotalLen % MAX_BUFF_SIZE);
			
			::EnterCriticalSection(& m_CS); 
			CFileTransSock::Instance().EnqueueBufferObj( pSpckObjMap->m_pSockObj, newbuf  , FALSE); 
			::LeaveCriticalSection(& m_CS); 			
			
			nTimes++;
		}
	}
	else
	{		
		newbuf = GetBufferObj(nTotalLen);					
		memcpy(newbuf->buf, pSzBuf, nTotalLen);

		::EnterCriticalSection(&m_CS); 
		EnqueueBufferObj(pSpckObjMap->m_pSockObj, newbuf, FALSE); 
		::LeaveCriticalSection(&m_CS); 
	}
	
	
	if(pSzBuf){
		delete[] pSzBuf; 
		pSzBuf = NULL;
	}
	
	::EnterCriticalSection(&m_CS); 
	int rc = SendPendingData(pSpckObjMap->m_pSockObj);	
	::LeaveCriticalSection(&m_CS); 

	if (rc == -1) 
	{
		::EnterCriticalSection(&m_CS); 
		RemoveSocketObj(pSpckObjMap->m_pThreadObj, pSpckObjMap->m_pSockObj);
		FreeSocketObj(pSpckObjMap->m_pSockObj);					
		::LeaveCriticalSection(&m_CS); 
	}
	else
	{				
		CFileSendDlg* pDlg = new CFileSendDlg(nFileCnt,  pSpckObjMap ,szToId, CString(szFilePath) ); 
		
		pDlg->Create(MAKEINTRESOURCE(IDD_FILESEND_DLG) , CWnd::FromHandle( GetDesktopWindow() )); 
		pDlg->ShowWindow(SW_SHOW);							
		::PostMessage(pDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);
		pDlg->SetForegroundWindow();
		pDlg->SetActiveWindow();	
		
		pSpckObjMap->m_pSockObj->m_HwndFileSend = pDlg->GetSafeHwnd();
		
		CFileTransSock::Instance().mStrFileSendDlgMap.SetAt( pSpckObjMap->m_pSockObj->m_szSockObjID, pDlg);			
	}
	
	return TRUE;
}


BOOL CFileTransSock::ProcessRecvData(SOCKET_OBJ_FILE * pSockObj)
{
	int nPos = 0;
		
	FILE_HEADER *psHeader = (FILE_HEADER *)pSockObj->m_pBuff;
			
	if( strcmp(psHeader->szUsage, (char*)USAGE_TAKE_FILE) == 0)
	{	
		TRACE("USAGE_TAKE_FILE\n");
		
		char* szData = (char*) pSockObj->m_pBuff + FILE_HEADER_LENGTH;	
		ST_RCV_SOMEONE_WANT_SEND_FILE_S* pFile  = new ST_RCV_SOMEONE_WANT_SEND_FILE_S();		
		strcpy(pFile->szFromIP, pSockObj->m_szIP);
		strcpy(pFile->szFromID, psHeader->szFromID);
		pFile->strFile = string(szData);
	
		::PostMessage(m_hOwnerWnd, WM_SOMEONE_WANTS_SEND_FILES, (WPARAM)pFile, (LPARAM) pSockObj ); 
	}
	else if( strcmp(psHeader->szUsage, (char*)USAGE_FOLDER_SELECTING) == 0)
	{	
		TRACE("USAGE_FOLDER_SELECTING\n");
				
		::PostMessage(m_hOwnerWnd, WM_FOLDER_SELECTING, (WPARAM)psHeader, (LPARAM)pSockObj);
	}
	else if( strcmp(psHeader->szUsage, (char*)USAGE_CANCEL_FILE_TRANSFER) == 0)
	{	
		TRACE("USAGE_CANCEL_FILE_TRANSFER\n");
		
		if(pSockObj->m_HwndFileRecv)
		{
			::PostMessage( pSockObj->m_HwndFileRecv, WM_CANCEL_FILE_TRANSFER, (WPARAM)NULL, (LPARAM) NULL ); 
		}
	}	
	else if( strcmp(psHeader->szUsage, (char*)USAGE_NO_THANKS_FILE_TRANSFER) == 0)
	{	
		TRACE("USAGE_NO_THANKS_FILE_TRANSFER\n");
		
		if(pSockObj->m_HwndFileSend)
		{
			::PostMessage( pSockObj->m_HwndFileSend, WM_NO_THANKS_FILE_TRANSFER, (WPARAM)NULL, (LPARAM) NULL ); 
		}
		
	}
	else if( strcmp(psHeader->szUsage, (char*)USAGE_SEND_THIS_INFORMED_FILE) == 0)
	{	
		TRACE("USAGE_SEND_THIS_INFORMED_FILE\n");
		char* szData = (char*) pSockObj->m_pBuff + FILE_HEADER_LENGTH;	
		int nLen =  strlen(szData) + 1 ;
		char* szFile  = new char[ nLen];	
		memset(szFile, 0x00, nLen );		
		memcpy(szFile, szData, nLen);

		::PostMessage(m_hOwnerWnd, WM_SEND_THIS_INFORMED_FILE, (WPARAM)szFile, (LPARAM) pSockObj ); 
	}
	else if( strcmp(psHeader->szUsage, (char*)USAGE_MAKE_THIS_FILE) == 0)
	{	
		TRACE("USAGE_MAKE_THIS_FILE\n");
		BYTE* pFileData = pSockObj->m_pBuff + FILE_HEADER_LENGTH;	
		
		ST_RCVED_FILE_INFO* pFileInfo  = new ST_RCVED_FILE_INFO();
	
		__int64 nLen = _atoi64(psHeader->szFileLength);	
				
		pFileInfo->szFilePath = new char [ sizeof(psHeader->szFilePath) ];
		memset(pFileInfo->szFilePath, 0x00, sizeof(psHeader->szFilePath) );
		memcpy(pFileInfo->szFilePath, psHeader->szFilePath, sizeof(psHeader->szFilePath) );		
		
		pFileInfo->nFileLen = nLen ;

		if(pSockObj->m_HwndFileRecv)
		{
			::PostMessage( pSockObj->m_HwndFileRecv, WM_MAKE_THIS_FILE, (WPARAM)NULL, (LPARAM) pFileInfo ); 
		}
	}
	
	

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFileTransSock message handlers


void CFileTransSock::ProcessWhenConnClosed(SOCKET_OBJ_FILE * pSockObj)
{
	TRACE("CChatSession::ProcessWhenConnClosed() ������ �����!!\n");	
		
	::SendMessage( m_hOwnerWnd, WM_DISCONNECTED, (WPARAM)NULL, (LPARAM) pSockObj ); 
	
}


//! ���� ������ �˷��ְ� �����ϰ� �Ѵ�
BOOL CFileTransSock::GetInformedFile(SOCKET_OBJ_FILE * pSockObj, char* szFilePath)
{		
	// �������� ������ ������ ���� 	
	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];	
	CString strMsg;
	
	memset(szTemp, 0x00, sizeof(szTemp));
	TRACE("GetInformedFile [%s]\n", szFilePath);

	memset(&sFileHeader, NULL, sizeof(sFileHeader));
	
	iLength = strlen(szFilePath) + 1; // +1 NULL	

	memset(&sFileHeader, NULL, sizeof(sFileHeader));		
	// ����� ����	
	memcpy(sFileHeader.szUsage, (char*)USAGE_SEND_THIS_INFORMED_FILE, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 
		
	CSockObjMapInfoFile *pa2= NULL;

	if(mSockObjMap.Lookup( (LPCSTR)pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// ���۵����� ������
		pSzBuf = new BYTE [FILE_HEADER_LENGTH + iLength]; //!!!
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);
		memcpy(pSzBuf + FILE_HEADER_LENGTH, szFilePath, iLength);
		nTotalLen = FILE_HEADER_LENGTH + iLength;

		CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
		
		if(pSpckObjMap)
		{			
			BUFFER_OBJ_FILE *newbuf = NULL;
			newbuf = GetBufferObj(nTotalLen);					
			memcpy(newbuf->buf, pSzBuf, nTotalLen);
			
			::EnterCriticalSection(&m_CS); 
			EnqueueBufferObj(pSockObj, newbuf, FALSE); 
			::LeaveCriticalSection(&m_CS); 

			if(pSzBuf){
				delete[] pSzBuf; 
				pSzBuf = NULL;
			}

			::EnterCriticalSection(&m_CS); 
			int rc = SendPendingData(pSockObj);
												
			if (rc == -1) 
			{
				RemoveSocketObj(pSpckObjMap->m_pThreadObj,pSockObj );
				FreeSocketObj( pSockObj );					
			}			
			::LeaveCriticalSection(&m_CS); 
		}
	}

	return TRUE;
}

//! �������������� �˸�
BOOL CFileTransSock::InformFolderSelect(SOCKET_OBJ_FILE * pSockObj)
{
	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];
	
	TRACE("InformFolderSelect\n");
	memset(szTemp, 0x00, sizeof(szTemp));	
	memset(&sFileHeader, NULL, sizeof(sFileHeader));		
	
	// ����� ����	
	memcpy(sFileHeader.szUsage, (char*)USAGE_FOLDER_SELECTING, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 

	nTotalLen = FILE_HEADER_LENGTH  ;					

	CSockObjMapInfoFile *pa2= NULL;	
	
	
	if(mSockObjMap.Lookup( pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// ���۵����� ������
		pSzBuf = new BYTE [FILE_HEADER_LENGTH ]; 
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);		

		CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
		
		if(pSpckObjMap)
		{			
			BUFFER_OBJ_FILE *newbuf = NULL;
			newbuf = GetBufferObj(nTotalLen);					
			memcpy(newbuf->buf, pSzBuf, nTotalLen);
			
			::EnterCriticalSection(&m_CS); 
			EnqueueBufferObj(pSockObj, newbuf, FALSE); 
			::LeaveCriticalSection(&m_CS); 

			if(pSzBuf)
			{
				delete[] pSzBuf; 
				pSzBuf = NULL;
			}

			::EnterCriticalSection(&m_CS); 
			int rc = SendPendingData(pSockObj);
			
			if (rc == -1) 
			{
				RemoveSocketObj(pSpckObjMap->m_pThreadObj, pSockObj );
				FreeSocketObj(pSockObj);					
			}
			::LeaveCriticalSection(&m_CS); 
		}
	}

	return TRUE;
}

BOOL CFileTransSock::DenyTransferFile( SOCKET_OBJ_FILE* pSockObj)
{
	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];
	memset(szTemp, 0x00, sizeof(szTemp));
	TRACE("DenyTransferFile\n");

	memset(&sFileHeader, NULL, sizeof(sFileHeader));
					
	// ����� ����	
	memcpy(sFileHeader.szUsage, (char*)USAGE_NO_THANKS_FILE_TRANSFER, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 

			
	nTotalLen = FILE_HEADER_LENGTH  ;					

	CSockObjMapInfoFile *pa2= NULL;
	
	if(mSockObjMap.Lookup(pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// ���۵����� ������
		pSzBuf = new BYTE [FILE_HEADER_LENGTH ]; 
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);		

		CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
		
		if(pSpckObjMap)
		{			
			BUFFER_OBJ_FILE *newbuf = NULL;
			newbuf = GetBufferObj(nTotalLen);					
			memcpy(newbuf->buf, pSzBuf, nTotalLen);
			
			::EnterCriticalSection(&m_CS); 
			EnqueueBufferObj(pSockObj, newbuf, FALSE); 
			::LeaveCriticalSection(&m_CS); 
			
			if(pSzBuf){
				delete[] pSzBuf; 
				pSzBuf = NULL;
			}
			
			::EnterCriticalSection(&m_CS); 
			int rc = SendPendingData(pSockObj);
			
			if (rc == -1) 
			{   // �̰��� �ǹ̾�������. socket�̺�Ʈ�� ���ؼ� ���� ������ ������ ó����
				// ���۽��и� �Ʒ��κ� ȣ���ϰ� �ֱ� ����..
				RemoveSocketObj(pSpckObjMap->m_pThreadObj, pSockObj);
				FreeSocketObj(pSockObj);					
			}
			::LeaveCriticalSection(&m_CS); 
		}
	}

	
	return TRUE;
}