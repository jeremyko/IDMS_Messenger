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
LONG	gTotalConnections=0,	//누적 접속수
		gCurrentConnections=0;	//현재 접속수

LONG gBytesRead=0,
     gBytesSent=0,
     gStartTime=0,
     gBytesReadLast=0,
     gBytesSentLast=0,
     gStartTimeLast=0;

THREAD_OBJ_FILE *gChildThreads=NULL;        // thread objects 리스트 
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



// 시동.. 쓰레드 생성
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
		
    thread = GetThreadObj(); //쓰레드 객체 생성.    
                
    sockobj = GetSocketObj(INVALID_SOCKET, TRUE );
		
	sockobj->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		
	
	if (sockobj->s == INVALID_SOCKET) 
	{			
		TRACE("socket failed: %d\n", WSAGetLastError());		
		
		return 0;
	}

	//쓰레드에 소켓 객체 저장
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

	// socket 이벤트 등록
	rc = WSAEventSelect(sockobj->s, sockobj->event, FD_ACCEPT | FD_CLOSE);
	if (rc == SOCKET_ERROR)
	{                
		TRACE("WSAEventSelect failed: %d\n", WSAGetLastError());
		
		return 0;
	}
	
    while (1) 
	{		
		TRACE("2\n");
    	//event를 기다림        
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
            sockobj = FindSocketObj(thread, index-1); // 이벤트가 발생한 소켓 객체 구함
			            
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
				
				// FD_ACCEPT | FD_CLOSE 로 이벤트를 등록하였다..
				sc = accept( sockobj->s, (SOCKADDR *)&sa, &salen); //accept 한다
				
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

					//쓰레드에 할당	
					pSpckObjMap->m_pSockObj = newsock;								
					
					pSpckObjMap->m_pThreadObj = AssignToFreeThread(newsock);	
					
					TRACE("★ accept : newsock->m_szIP [%s] m_szSockObjID [%s]\n", newsock->m_szIP, newsock->m_szSockObjID);					
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
		TRACE("★ socket 생성에러\n");
        
        return NULL;
    }
    
    ZeroMemory((char*) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(lpSocketAddress);
    addr.sin_port = htons((u_short)nSocketPort);	
    
    u_long    arg = 1;
    ioctlsocket(m_hSocket, FIONBIO, &arg); // non-blocking mode로 설정	
	struct  timeval     timevalue;	
	fd_set writefds;	

	//TRACE("★ connect 시도 [%s][%d]\n", GetCurrentTimeByString(),m_hSocket);

	connect(m_hSocket, (struct sockaddr*) &addr, sizeof(addr));

	FD_ZERO( &writefds );
	FD_SET( m_hSocket, &writefds );
	timevalue.tv_sec = 0;
	timevalue.tv_usec = 100000; // 0.1 sec 3000000;

	::select( 0, NULL, &writefds, NULL, &timevalue );

	if ( !FD_ISSET( m_hSocket, &writefds ) )
	{		
		TRACE("★ connect 실패 [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return NULL;
	}	
	
	TRACE("★ connect 성공 [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
	
	THREAD_OBJ_FILE      *thread=NULL;    
    SOCKET_OBJ_FILE      *newsock=NULL;                    
    int             rc;
    		
    thread = GetThreadObj(); //쓰레드 객체 생성. 	
		
	newsock = GetSocketObj(INVALID_SOCKET, FALSE);
	newsock->s = m_hSocket;

	if (newsock->s == INVALID_SOCKET) 
	{			
		TRACE("socket failed: %d\n", WSAGetLastError());		
		
		return NULL;
	}

	//쓰레드에 소켓 객체 저장
	InsertSocketObj(thread, newsock);

	InterlockedIncrement(&gTotalConnections);
	InterlockedIncrement(&gCurrentConnections);
			
	// 소켓에 read, write ,close 이벤트 등록
	rc = WSAEventSelect( newsock->s, newsock->event, FD_READ | FD_WRITE | FD_CLOSE);
	
	if (rc == SOCKET_ERROR) 
	{						
		TRACE("WSAEventSelect failed: %d\n", WSAGetLastError());
		
		return NULL;
	}

	//Map 에 관리					
	CSockObjMapInfoFile * pSpckObjMap = new CSockObjMapInfoFile;
	pSpckObjMap->m_pSockObj = newsock;					
	pSpckObjMap->m_pThreadObj = NULL;
			
	mSockObjMap.SetAt(newsock->m_szSockObjID, pSpckObjMap);	

	//쓰레드에 할당 후 return!!						
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
    
	//메모리 할당
    sockobj = (SOCKET_OBJ_FILE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SOCKET_OBJ_FILE ));
    
	if (sockobj == NULL) 
	{
		TRACE("GetSocketObj: HeapAlloc failed: %d\n", GetLastError());
        
        ExitProcess(-1);
    }

    // member 초기화
    sockobj->s = s;
    sockobj->listening = listening; //TCP 경우 TRUE..
    sockobj->addrlen = sizeof(sockobj->addr);

    sockobj->event = WSACreateEvent(); //이벤트 생성
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
	
	//쓰레드 객체 위한 메모리 할당
    thread = (THREAD_OBJ_FILE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(THREAD_OBJ_FILE));
    
	if (thread == NULL)
	{
		TRACE("GetThreadObj: HeapAlloc failed: %d\n", GetLastError());
        
        ExitProcess(-1);
    }
    
	thread->Event = WSACreateEvent(); //이벤트 생성
    
	if (thread->Event == NULL)
	{
		TRACE("GetThreadObj: WSACreateEvent failed: %d\n", WSAGetLastError());
        
        ExitProcess(-1);
    }
	//socket event handle의 첫번째에 새로운 client가 쓰레드에 할당되었음을 알리기 위한 event 저장.
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


// 	소켓 객체를 주어진 쓰레드의 소켓 객체 리스트에 추가한다.

int CFileTransSock::InsertSocketObj(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock)
{
    int     ret;
    
	EnterCriticalSection(&thread->ThreadCritSec);
    
	//최대 대기 이벤트수를 넘으면 리스트에 추가하지 않음.
    if (thread->SocketCount < MAXIMUM_WAIT_OBJECTS-1)
	{	
        sock->next = sock->prev = NULL;
        
		if (thread->SocketList == NULL) 
		{
            // List가 비었다
            thread->SocketList = thread->SocketListEnd = sock;
        }
		else
		{
            // 리스트에 객체 있음, 리스트 끝에 추가
            sock->prev = thread->SocketListEnd;
            thread->SocketListEnd->next = sock;
            thread->SocketListEnd = sock;
        }
        
        // 소켓의 이벤트를 쓰레드 이벤트 리스트에 할당.
        thread->Handles[thread->SocketCount + 1] = sock->event;
        thread->SocketCount++;

        ret = NO_ERROR;
    }
	else
	{
		//최대 대기 이벤트수를 넘으면 리스트에 추가하지 않음.
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
		
		// 파일데이터는 파일을 직접 읽어가면서 보낸다.
		if( bufobj->bIsFileData ) // bufobj->dwTotalFileLength != 0
		{
			//파일///////////////////////////////////////////////////////////////////////////////////////////
			if(bufobj->dwTotalFileLength == 0)
			{
				// 여기서 send dlg 에 전송량 메시지 날린다 
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
			
			// 파일을 쓴다
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
						TRACE("\n ★★ WSAEWOULDBLOCK!! ★★\n");
						BUFFER_OBJ_FILE *newbuf=NULL;						
						newbuf = GetBufferObj( sizeof(BUFFER_OBJ_FILE) );

						newbuf->bIsFileData = TRUE;						
						newbuf->buf = NULL;
						newbuf->buflen = 0;
						newbuf->dwReadFilePosition =  bufobj->dwReadLength - nleft ;
						newbuf->dwTotalFileLength = bufobj->dwTotalFileLength ;
						newbuf->dwReadLength = nleft ; 
												
						EnqueueBufferObj(sock, bufobj, TRUE); //queue 앞(head)에 넣음. 다음에 이어서 전송.
												
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

					// 여기서 send dlg 에 전송량 메시지 날린다 
					if(sock->m_HwndFileSend)
					{
						//::SendMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
						::PostMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL ); 
					}

					nleft -= rc;
					idx += 0; //모두 ..
				}
			}
			//파일///////////////////////////////////////////////////////////////////////////////////////////
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
												
						EnqueueBufferObj(sock, newbuf, TRUE); //queue 앞(head)에 넣음. 다음에 이어서 전송.
						
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
					// 여기서 send dlg 에 전송량 메시지 날린다 
					//if(sock->m_HwndFileSend)
					//{
					//	::PostMessage( sock->m_HwndFileSend, WM_SEND_BYTES, (WPARAM)rc, (LPARAM) NULL); 
					//}

					nleft -= rc;
					idx += 0; //모두 ..
				}
			}
		}
	
		//cfile.Close();

		//메모리 clear..
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


// 헤더 정보를 포함하는 전체 PAcket 길이 
BOOL CFileTransSock::GetHeaderInfo(SOCKET_OBJ_FILE * pSockObj)
{
	if(pSockObj->m_iBuffLength < FILE_HEADER_LENGTH)
	{
		TRACE("GetHeaderInfo return\n");
		return FALSE ;
	}
			
	// 헤더 정보를 copy
	memcpy( &(pSockObj->sPacketHeader),  (FILE_HEADER *)pSockObj->m_pBuff, sizeof(FILE_HEADER));

	// 헤더 길이를 포함하는 전체 데이터 크기 
	pSockObj->nTotalOnePacketLen = atoi(pSockObj->sPacketHeader.szLength) + FILE_HEADER_LENGTH ;
	
	TRACE("전체 Packet 길이 [%d]\n", pSockObj->nTotalOnePacketLen);
	
	return TRUE;
}




//클라이언트 요청 처리
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
			// 파일데이터는 받는대로 쓴다.
			sock->m_iBuffLength = sock->m_iBuffLength + receive_length;			
									
			ST_RCVED_BYTES_INFO sRecvBytes ; 
			memset(&sRecvBytes, 0x00, sizeof(sRecvBytes));
			sRecvBytes.pBuff = RecvedBytes ;
			sRecvBytes.receive_length = receive_length ;						
			sRecvBytes.sFileHeader = &(sock->sPacketHeader) ;
			sRecvBytes.sock = sock ;
			
			::SendMessage( sock->m_HwndFileRecv, WM_RECV_BYTES, (WPARAM) &sRecvBytes , 	(LPARAM) NULL ); 

			//////파일데이터 처리 /////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
			{				
				//TRACE("아직 한 Packet의 데이터가 오질 않았다\n");				
				return 0;
			}			
			else
			{
				// 현재 받은 데이터 - 하나의 패킷 데이터 CHK			
				int nExcessiveLen = sock->m_iBuffLength - sock->nTotalOnePacketLen ;
				
				ProcessRecvData(sock);
												
				if(nExcessiveLen > 0)
				{
					TRACE("하나의 패킷 데이터 보다 더많은 데이터 수신 경우 => 버퍼를 다시 구성[%d]\n", nExcessiveLen);				

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
							//TRACE("아직 한 Packet의 데이터가 오질 않았다\n");				
							return 0;
						}		
						else
						{
							TRACE("한 Packet의 데이터! \n");				
							ProcessRecvData(sock);					
						}
						
						nExcessiveLen = nExcessiveLen - sock->nTotalOnePacketLen ;
						
						if(nExcessiveLen == 0)
						{
							// 더받았고 패킷용량에 맞게 다 전송된 경우 
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
							// 아직 한패킷의 데이터가 다 전송되지 않았다.
							// => 더받아야하므로 버퍼를 삭제하지 않는다. SKIP..
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
			//////파일데이터처리종료///////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			// 메시지 데이터만 버퍼를 관리
			
			BYTE* pTemp = sock->m_pBuff; // 이전 데이터 copy..
			
			// 이전 데이터 + 지금 받은 데이터만큼 메모리 버퍼를 할당한다.		
			sock->m_pBuff = new BYTE [sock->m_iBuffLength + receive_length + 1];
			memset(sock->m_pBuff, 0x00, sock->m_iBuffLength + receive_length +1);
			
			if(sock->m_pBuff == NULL)
			{
				return 0;
			}
			
			if(sock->m_iBuffLength != 0)
			{				
				// 새로 할당된 버퍼에 지금까지 받은 데이터 다시 저장..			
				
				memcpy(sock->m_pBuff, pTemp, sock->m_iBuffLength);
				
				if(pTemp)
				{
					// pTemp 는 다시 초기화 상태로 설정,,
					delete[] pTemp;
					pTemp = NULL;
				}
			}		
			
			// 새로 받은 데이터 복사		
			memcpy(sock->m_pBuff + sock->m_iBuffLength, RecvedBytes, receive_length);
			
			//TRACE("받은 데이터 사이즈 증가 기록sock->m_iBuffLength [%d] receive_length[%d]\n", sock->m_iBuffLength, receive_length);
			
			int nOldLen  = sock->m_iBuffLength ;

			sock->m_iBuffLength = sock->m_iBuffLength + receive_length;
			
			if ( GetHeaderInfo(sock) == FALSE ) 
			{
				return 0;
			}
			
			// 파일 데이터이면 파일을 write 한다. => 버퍼를 지워주면서 받는다
						
			if(strcmp( sock->sPacketHeader.szUsage, USAGE_MAKE_THIS_FILE ) == 0 )
			{
				sock->m_bIsRealFileByteData = TRUE;

				// recv dlg로 알림 
				if(sock->m_HwndFileRecv)
				{
					if(nOldLen == 0 ) // 처음
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
				//TRACE("아직 한 Packet의 데이터가 오질 않았다\n");				
				return 0;
			}			
			else
			{				
				//TRACE("\n▷▷▷ 한 Packet을 구성할 데이터가 도착 (수신데이터 처리시작) sock->nTotalOnePacketLen[%d]\n",sock->nTotalOnePacketLen);
				
				// 현재 받은 데이터 - 하나의 패킷 데이터 CHK			
				int nExcessiveLen = sock->m_iBuffLength - sock->nTotalOnePacketLen ;
				
				ProcessRecvData(sock);
																
				if(nExcessiveLen > 0)
				{
					TRACE("하나의 패킷 데이터 보다 더많은 데이터 수신 경우 => 버퍼를 다시 구성[%d]\n", nExcessiveLen);				
					
					pTemp = sock->m_pBuff; // 이제까지 받은 데이터, 포인터 저장					
					
					sock->m_pBuff = new BYTE[nExcessiveLen]; 
					
					if(sock->m_pBuff == NULL)
					{				
						return 0;
					}
					
					// 남은 데이터 복사, pTemp는 아직 한패킷 + 여분의 데이터 상태..				
					memcpy(sock->m_pBuff, pTemp + sock->nTotalOnePacketLen , nExcessiveLen);				
					sock->m_iBuffLength = nExcessiveLen;
					
					if(pTemp)
					{					
						delete[] pTemp;
						pTemp = NULL;
					}
					
					// 더받은 데이터를 처리한다. 
					// 버퍼가 0 or 0보다 작음(아직 다 못받음) 이 될때까지..
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
							//TRACE("아직 한 Packet의 데이터가 오질 않았다\n");				
							return 0;
						}		
						else
						{
							TRACE("한 Packet의 데이터! \n");				
							ProcessRecvData(sock);					
						}
						
						nExcessiveLen = nExcessiveLen - sock->nTotalOnePacketLen ;
						
						if(nExcessiveLen == 0)
						{
							// 더받았고 패킷용량에 맞게 다 전송된 경우 
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
							// 아직 한패킷의 데이터가 다 전송되지 않았다.
							// => 더받아야하므로 버퍼를 삭제하지 않는다. SKIP..
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
			TRACE("\n▶▶▶ 수신 WSAEWOULDBLOCK 에러\n");
			return 1;
		}		
		else
		{
			TRACE("\n▶▶▶ ERROR : 수신 에러 [%d] \n", nRet);
			//break;			
			return -1;
		}
	}
	else if( receive_length == 0 ) 
	{			
		TRACE("\n▶▶▶ receive_length == 0  \n");
		int nRet = GetLastError();
		//break;
		return -1; // Socket Close..
	}		
	
	TRACE("▷▷▷ 수신 종료\n");
	
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

			// sock 객체 clear.
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


// 쓰레드의 socket event 배열에 쓰레드 소켓 객체의 이벤트를 모두 다시 저장한다(rebuild)
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

// 클라이언트의 요청을 처리할 쓰레드(이미 Accept된)
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
            // 동시에 여러개의 이벤트들이 set된 경우 대비, 모두 검사한다.            
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
    
	// thread 연결 리스트를 돌면서 소켓 개체를 삽입
    while (thread) 
	{
        // SOCKET_ERROR 를 반환하지 않는다면 child thread에 할당이 성공한것.        
        if (InsertSocketObj(thread, sock) != SOCKET_ERROR)
            break;
        thread = thread->next;
    }

    if (thread == NULL) 
	{        
		TRACE("Creating new thread object\n");		
		
        thread = GetThreadObj(); //메모리 할당후,

		//pOut = thread;
		
		TRACE("★ AssignToFreeThread: CreateThread \n");
        //thread->Thread = ::CreateThread(NULL, 0, ChildThread, (LPVOID)thread, 0, NULL); 
		unsigned threadID = 0;
		thread->Thread = (HANDLE) _beginthreadex(NULL, 0, &ChildThread, (LPVOID)thread, 0, &threadID ) ;				
        
		if (thread->Thread == NULL)
		{
			TRACE("AssignToFreeThread: CreateThread failed: %d\n", GetLastError());
            ExitProcess(-1);
        }
        
		InsertSocketObj(thread, sock); //생성된 쓰레드객체에 소켓 객체 추가
        
		// 생성된 쓰레드는 쓰레드 리스트에 추가.
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

    // child thread가 event list를 재구성하게 한다.
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
					
	// 헤더부 설정	
	memcpy(sFileHeader.szUsage, (char*)USAGE_CANCEL_FILE_TRANSFER, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSpckObjMap->m_pSockObj->m_szIP, sizeof(pSpckObjMap->m_pSockObj->m_szIP) ); 
			
	nTotalLen = FILE_HEADER_LENGTH  ;					
		
	// 전송데이터 모으기
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
	// 동적으로 데이터 사이즈 변동 	
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
	// 헤더부 설정	
	memcpy(sFileHeader.szUsage, (char*)USAGE_TAKE_FILE, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, CChatSession::Instance().m_szMyUserID /*m_szMyIPAddr*/,
			min(sizeof(sFileHeader.szFromID), strlen(CChatSession::Instance().m_szMyUserID))); 

	memcpy(sFileHeader.szFromName, (LPCSTR) CChatSession::Instance().m_LoginName , //20080710
			min(sizeof(sFileHeader.szFromName), strlen( (LPCSTR) CChatSession::Instance().m_LoginName))); 

	memcpy(sFileHeader.szToIP, (LPCSTR)strToIP, strToIP.GetLength() ); 

	// 전송데이터 모으기
	pSzBuf = new BYTE [FILE_HEADER_LENGTH + iLength]; //!!!
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sFileHeader, FILE_HEADER_LENGTH);
	memcpy(pSzBuf + FILE_HEADER_LENGTH, szFilePath, iLength);
	nTotalLen = FILE_HEADER_LENGTH + iLength;
			
	BUFFER_OBJ_FILE *newbuf = NULL;

	// 데이터가 큰 경우 버퍼에 나눠 저장한다.
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
	TRACE("CChatSession::ProcessWhenConnClosed() 연결이 끊겼다!!\n");	
		
	::SendMessage( m_hOwnerWnd, WM_DISCONNECTED, (WPARAM)NULL, (LPARAM) pSockObj ); 
	
}


//! 받은 파일을 알려주고 전송하게 한다
BOOL CFileTransSock::GetInformedFile(SOCKET_OBJ_FILE * pSockObj, char* szFilePath)
{		
	// 동적으로 데이터 사이즈 변동 	
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
	// 헤더부 설정	
	memcpy(sFileHeader.szUsage, (char*)USAGE_SEND_THIS_INFORMED_FILE, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 
		
	CSockObjMapInfoFile *pa2= NULL;

	if(mSockObjMap.Lookup( (LPCSTR)pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// 전송데이터 모으기
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

//! 폴더선택중임을 알림
BOOL CFileTransSock::InformFolderSelect(SOCKET_OBJ_FILE * pSockObj)
{
	BYTE* pSzBuf = NULL;
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];
	
	TRACE("InformFolderSelect\n");
	memset(szTemp, 0x00, sizeof(szTemp));	
	memset(&sFileHeader, NULL, sizeof(sFileHeader));		
	
	// 헤더부 설정	
	memcpy(sFileHeader.szUsage, (char*)USAGE_FOLDER_SELECTING, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 

	nTotalLen = FILE_HEADER_LENGTH  ;					

	CSockObjMapInfoFile *pa2= NULL;	
	
	
	if(mSockObjMap.Lookup( pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// 전송데이터 모으기
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
					
	// 헤더부 설정	
	memcpy(sFileHeader.szUsage, (char*)USAGE_NO_THANKS_FILE_TRANSFER, sizeof(sFileHeader.szUsage)); 
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, iLength );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	memcpy(sFileHeader.szFromID, m_szMyIPAddr, min(sizeof(sFileHeader.szFromID), strlen(m_szMyIPAddr))); 
	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 

			
	nTotalLen = FILE_HEADER_LENGTH  ;					

	CSockObjMapInfoFile *pa2= NULL;
	
	if(mSockObjMap.Lookup(pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// 전송데이터 모으기
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
			{   // 이것은 의미없을것임. socket이벤트에 의해서 전송 가능한 시점에 처리시
				// 전송실패면 아래부분 호출하고 있기 때문..
				RemoveSocketObj(pSpckObjMap->m_pThreadObj, pSockObj);
				FreeSocketObj(pSockObj);					
			}
			::LeaveCriticalSection(&m_CS); 
		}
	}

	
	return TRUE;
}