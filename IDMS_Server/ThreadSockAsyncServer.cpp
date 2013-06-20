
// 2009-03-16 local Scope Lock , CAutoLock 으로 대체. 

// ThreadPushWorkAsync.cpp: implementation of the CThreadPushWorkAsync class.

// ToDo 
// 1) doWork_ALL_USER_ID_TEAM에서 Ip도 넘겨줘서 대화상대 추가후 
//      바로 파일 전송시에 가능하게해야함.

#include "stdafx.h"
#include "ThreadSockAsyncServer.h"
#include "INIManager.h"
#include "thread_pool.h"
#include "log.h"
//#include "catchexcept.h"

#include "MemoryPool.h"

 
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//! stlport4.6.2 에서 컴파일 되었음.

//GLOBAL
int
gAddressFamily = AF_UNSPEC,       	// default to unspecified
gSocketType    = SOCK_STREAM,       // default to TCP socket type
gProtocol      = IPPROTO_TCP;       // default to TCP protocol

THREAD_OBJ *gChildThreads=NULL;     // thread objects 리스트 
int  gChildThreadsCount=0;			// Number of child threads created
long gCurrentConnections = 0;

CThreadSockAsyncServer* g_dlgPtr;
int gnTest = 0;
//GLOBAL

/////////////////////
CIpSockObjMapInfo::CIpSockObjMapInfo()
{
}

CIpSockObjMapInfo::~CIpSockObjMapInfo()
{
	
}

CThreadSockAsyncServer::CThreadSockAsyncServer(HWND p_hWnd_dlg) : m_nMultiChatRoomNo (0l), m_strAllUserList("")
{	
	m_pDlg = (CIDMS_ServerDlg*)CWnd::FromHandle(p_hWnd_dlg);
	g_dlgPtr = this; 
		
	gnTest = 0; //TEST

	OStartup(OSTARTUP_MULTITHREADED); 

	memset(m_szFilterIP1, 0x00, sizeof(m_szFilterIP1) );	
	memset(m_szFilterIP2, 0x00, sizeof(m_szFilterIP2) );	
}

CThreadSockAsyncServer::~CThreadSockAsyncServer()
{
	CMemoryPool::AllDeleteMemory();		
		
	POSITION pos = mSockIdSockObjMap.GetStartPosition();

	while( pos != NULL )
	{
		CIpSockObjMapInfo* p;
		CString string;		
		mSockIdSockObjMap.GetNextAssoc( pos, string, (CObject*&) p );

		if(p && (DWORD)p!= 0xfeeefeee && (DWORD)p->m_pSockObj!= 0xfeeefeee)
		{				
			delete p;
			p = NULL;
		}		
	}
	
	mSockIdSockObjMap.RemoveAll();
}

			
unsigned __stdcall PThreadProc( void* pParam )
{
	CThreadSockAsyncServer* _thread = static_cast<CThreadSockAsyncServer*>(pParam);
	unsigned result = 0;
	if (_thread)
	{
		try	
		{
			result = _thread->_run();			
		}
		catch(...)	
		{							
			CLog::Instance().WriteFile(FALSE, "ErrorTerminate", "Ln [%d] ▷  PThreadProc Exit! \n", __LINE__ );			
			exit(-1);
			
			result = -1; 										
		}		    		
	}	

	return result;
}




int	CThreadSockAsyncServer::testError()
{
	/*
	if(gnTest == 0)	
	{
		AfxMessageBox("TEST!");
		gnTest = 1;
	}
	else
	{
		AfxMessageBox("NOT TEST!");
		gnTest = 0;
	}
	*/

	return 0;
}

int	CThreadSockAsyncServer::UpdateAllUserList()
{
	// 전체 사용자 목록을 DB에서 조회하여 메모리로 관리한다.
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));
	
	_snprintf(szSql ,sizeof(szSql)-1, "%s", "SELECT	/*+ ORDERED USE_NL(A B C D ) */	\
A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;

	
//	sprintf(szSql, "SELECT	/*+ ORDERED USE_NL(A B C D ) */	\
//A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
//FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
//WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
//AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT(); // POOL ... 	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( UPDATE_ALL_USER_INIT, "", "", "");	// 고유 ID
	pSqlWork->setNotiProcThread(SQLNotifyProc , this);  // call back 지정
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return 0;
}



int	CThreadSockAsyncServer::makeAllUserList()
{		
	// 전체 사용자 목록을 DB에서 조회하여 메모리로 관리한다.
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));
		
	_snprintf(szSql ,sizeof(szSql)-1, "%s" , "SELECT /*+ ORDERED USE_NL(A B C D ) */	\
A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;

//	sprintf(szSql, "SELECT	/*+ ORDERED USE_NL(A B C D ) */	\
//A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
//FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
//WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
//AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);			
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_ALL_USER_INIT, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return 0;
}

int	CThreadSockAsyncServer::makeUserList()
{	
	// 각 사용자의 대화상대목록을 DB에서 조회하여 메모리로 관리한다.
	char szSql [100];
	memset(szSql, 0x00 , sizeof(szSql));
	//sprintf(szSql, "SELECT A.USER_ID,A.COMPANION_ID FROM BUDDY_LIST A ORDER BY A.USER_ID") ;

	_snprintf(szSql ,sizeof(szSql)-1, "%s" , "SELECT A.USER_ID,A.COMPANION_ID FROM BUDDY_LIST A ORDER BY A.USER_ID" );
		
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_COMP_LIST_INIT, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	
	
	return 0;
}

// 시동.. 쓰레드 생성
int	CThreadSockAsyncServer::Start()
{	
	WSADATA          wsd;
    
    // Load Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{		
		TRACE("ERROR Unable to load Winsock!");		
		AfxMessageBox("Unable to load Winsock!");
        return 0;
    }

	CLog::Instance().WriteFile(FALSE, "ServerLog",  "▷ CThreadSockAsyncServer::Start() ! \n" );
	
	_thread = (HANDLE)::_beginthreadex(0, 0, PThreadProc, (void*)this, 0, 0);
	if (0 == _thread)
	{
		TRACE(_T("CThreadPushWorkAsync::Start() : _beginthreadex ERROR \n"));
		
		return -1;
	}
	TRACE(_T("_beginthreadex success!\n"));
	
	/*
	_threadUpdateAlert = (HANDLE)::_beginthreadex(0, 0, PThreadProcUpdateAlert, (void*)this, 0, 0);
	if (0 == _threadUpdateAlert)
	{
		TRACE(_T("CThreadPushWorkAsync::Start() : _beginthreadex ERROR \n"));
		
		return -1;
	}
	TRACE(_T("_beginthreadex success!\n"));
    */	
		
	
	return 0;
}


int	CThreadSockAsyncServer::TestClose()
{
	
// 	CIpSockObjMapInfo* pa = NULL ;
// 	if(mIPSockObjMap.Lookup( "10.225.230.135", ( CObject*& ) pa ))
// 	{		
// 		CIpSockObjMapInfo * pSpckObjMapTmp2 = static_cast<CIpSockObjMapInfo*>(pa);		
// 				
// 		if(pSpckObjMapTmp2)
// 		{
// 			closesocket(pSpckObjMapTmp2->m_pSockObj->s); 
// 		}
// 	}
	

	return 0;
}

int	CThreadSockAsyncServer::Stop(bool wait)
{
	
	SOCKET_OBJ *cpyPsockObj;
	THREAD_OBJ *cpyPthreadObj;
	
	cpyPthreadObj = gChildThreads;		
	
	for(int i = 0; i < gChildThreadsCount; i++)
	{//thread
		
		cpyPsockObj = cpyPthreadObj->SocketList;			
		
		for(int j = 0; j < cpyPthreadObj->SocketCount; j++)
		{
			if((DWORD)cpyPsockObj != 0xfeeefeee)
			{
				RemoveSocketObj(cpyPthreadObj, cpyPsockObj);
				FreeSocketObj(cpyPsockObj);
				
				cpyPsockObj = cpyPsockObj->next;
			}								
		}
		
		FreeThreadObj(cpyPthreadObj); 
		cpyPthreadObj = cpyPthreadObj->next;		
	}	
	
	return 1;
}

unsigned	CThreadSockAsyncServer::_run()
{
	//if (!CMiniDump::Begin()) //20080303
	//	return 0;
	
    WSADATA          wsd;
    THREAD_OBJ      *thread=NULL;    
    SOCKET_OBJ      *sockobj=NULL, *newsock=NULL;                    
    int              index, rc;
    struct addrinfo *res=NULL, *ptr=NULL;

    // Load Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{		
		TRACE("ERROR Unable to load Winsock!");		
		AfxMessageBox("Unable to load Winsock!");
        return 0;
    }
	
	
	char szIP[15+1], szPort[10];
	memset(szIP,0x00,sizeof(szIP));
	memset(szPort,0x00,sizeof(szPort));	
	
	CString strIP, strPort;
	CINIManager::Instance().GetValue(INI_SERVER_IP, strIP);	
	CINIManager::Instance().GetValue(INI_SERVER_PORT, strPort);	
	
	res = ResolveAddress( (LPCSTR)strIP, (LPCSTR)strPort , AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
    if (res == NULL) 
	{        
		//m_dlg->m_ListSendMsg.AddString("ResolveAddress failed to return any addresses");
		TRACE("ERROR ResolveAddress failed to return any addresses!");
		AfxMessageBox("서버 구동 실패! IP 설정을 확인하세요 ");
        return 0;
    }	

    thread = GetThreadObj(  __LINE__ ); //쓰레드 객체 생성.    
     
    ptr = res; //copy
	
    while (ptr) 
	{
    	// 호스트 로컬 어드레스가 여러개인 경우 각각의 local address에 대한 
		// listening/receiving socket 생성.
        
        sockobj = GetSocketObj(INVALID_SOCKET, TRUE );
                        
		sockobj->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		
		 
        if (sockobj->s == INVALID_SOCKET) 
		{			
			TRACE("ERROR socket failed: %d\n", WSAGetLastError());		
		
            return 0;
        }
		
		//20080926 linger 
		struct linger opt_ling = { 1, 0 }; // l_onoff, l_linger;
		setsockopt(sockobj->s, SOL_SOCKET, SO_LINGER, (const char*)&opt_ling, sizeof(opt_ling));

        //쓰레드에 소켓 객체 저장        
        
		{   // local obj 
			CAutoLock ScopeLock ( m_ScopeCS, "run:InsertSocketObj");
			InsertSocketObj(thread, sockobj);
		}
		                        
        rc = bind(sockobj->s, ptr->ai_addr, ptr->ai_addrlen);

        if (rc == SOCKET_ERROR)
		{
			TRACE("ERROR bind failed: %d\n", WSAGetLastError());
			AfxMessageBox("서버 구동 실패 서버 ! IP 설정을 확인하세요 ");
            return 0;
        }
        
		
		//20061106
        rc = listen(sockobj->s, 200);
        if (rc == SOCKET_ERROR)
		{ 				
			TRACE("ERROR listen failed: %d\n", WSAGetLastError());
			
			return 0;
		}
		
		// socket 이벤트 등록
		rc = WSAEventSelect(sockobj->s, sockobj->event, FD_ACCEPT | FD_CLOSE);
		if (rc == SOCKET_ERROR)
		{
			TRACE("ERROR WSAEventSelect failed: %d\n", WSAGetLastError());
			
			return 0;
		}         
		
        ptr = ptr->ai_next;
    }
    
    // free the addrinfo structure for the 'bind' address
    freeaddrinfo(res);

    while (1) 
	{    	
		rc = WSAWaitForMultipleEvents(thread->SocketCount + 1, thread->Handles, FALSE, WSA_INFINITE, FALSE);		
		
        if (rc == WAIT_FAILED) 
		{			
			TRACE("ERROR WaitForMultipleObjects failed: %d\n", GetLastError());			
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
			 
			if (sockobj == NULL)
			{
				//Log	
				CLog::Instance().WriteFile(FALSE, "ServerError",  "FindSocketObj return NULL\n");	
								
				continue;
			}
			
			//SOCKADDR_STORAGE sa;
			SOCKADDR sa;
			WSANETWORKEVENTS ne;
			SOCKET           sc;
			int              salen;
			
			//발생한 소켓 이벤트 종류를 얻음
			rc = WSAEnumNetworkEvents(sockobj->s, thread->Handles[index], &ne);
			
			if (rc == SOCKET_ERROR)
			{
				TRACE("ERROR WSAEnumNetworkEvents failed: %d\n", WSAGetLastError());
				break;
			}
			
			while (1) 
			{
				sc = INVALID_SOCKET;
				salen = sizeof(sa);
				
				// FD_ACCEPT | FD_CLOSE 로 이벤트를 등록하였다..
				sc = accept( sockobj->s, (SOCKADDR *)&sa, &salen); //accept 한다
								
				if ( sc == INVALID_SOCKET && WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAENOBUFS ) 
				{                        
					TRACE("ERROR accept failed: %d\n", WSAGetLastError());
								
					char szTmpIP[100];
					memset(szTmpIP, 0x00, sizeof(szTmpIP));
					
					_snprintf(szTmpIP ,sizeof(szTmpIP)-1, "accept failed: %d\n", WSAGetLastError() );

					m_pNotifyProc((LPVOID) m_pDlg,szTmpIP , NULL);						
				
					break;
				}
				else if (sc != INVALID_SOCKET)
				{
					newsock = GetSocketObj(INVALID_SOCKET, FALSE);
					
					memcpy(&newsock->addr, &sa, salen);
					newsock->addrlen = salen;
					newsock->s = sc;
										
					// accept된 소켓(newsock->s)에 read, write ,close 이벤트 등록
					rc = WSAEventSelect( newsock->s, newsock->event, FD_READ | FD_WRITE | FD_CLOSE);
					
					if (rc == SOCKET_ERROR) 
					{
						
						TRACE("ERROR WSAEventSelect failed: %d\n", WSAGetLastError());
						
						break;
					}					

					strcpy(newsock->m_szIP, ::inet_ntoa(((SOCKADDR_IN*)&sa)->sin_addr));

					//!!!!!!!! 쓰레드에 할당 전에 map에 저장해야함 !!!!!!!!!!
					//Map 에 관리 : key (IP) -> Sock OBJ					
					
					//CIpSockObjMapInfo *pa = NULL;
					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "run:mSockIdSockObjMap.SetAt");
									
						CString strSockID ;
						strSockID.Format("%d", newsock->s) ;	

						//if(mSockIdSockObjMap.Lookup( strSockID, ( CObject*& ) pa ))
						//if(mIPSockObjMap.Lookup(newsock->m_szIP, ( CObject*& ) pa ))
						//{	
							//20090309 동일 IP에서 여러 건 접속시에도 허용한다. 							
						//}
																		
						CIpSockObjMapInfo *pSpckObjMap = new CIpSockObjMapInfo; 
						pSpckObjMap->m_pSockObj = newsock; 							
						pSpckObjMap->m_pThreadObj = NULL; //먼저 NULL로 set 

						//mIPSockObjMap.SetAt(newsock->m_szIP,pSpckObjMap); 
						mSockIdSockObjMap.SetAt(strSockID,pSpckObjMap); 
							
						//쓰레드에 할당
						pSpckObjMap->m_pThreadObj = AssignToFreeThread( newsock );					

						TRACE("SocketCount [%d]\n", pSpckObjMap->m_pThreadObj->SocketCount );

						InterlockedIncrement(&gCurrentConnections);

						CLog::Instance().WriteFile(FALSE, "ServerLog", "★ Ln [%d] connected IP [%s] sock [%s] Connection [%d]\n", 
							__LINE__, newsock->m_szIP, strSockID, gCurrentConnections);
					}
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

void	CThreadSockAsyncServer::SetLoginInfo(const char* strConn, const char* strUser, const char* strPasswd)
{
	memset(m_strDBConn, 0x00, sizeof(m_strDBConn));
	memset(m_strUser, 0x00, sizeof(m_strUser));
	memset(m_strPassWd, 0x00, sizeof(m_strPassWd));

	strncpy( m_strDBConn ,  strConn, sizeof(m_strDBConn) ) ; 
	strncpy( m_strUser ,  strUser, sizeof(m_strUser) ) ; 
	strncpy( m_strPassWd ,  strPasswd, sizeof(m_strPassWd) ) ; 
}

struct addrinfo* CThreadSockAsyncServer::ResolveAddress(const char *addr, const char *port, int af, int type, int proto)
{
	TRACE("ResolveAddress addr [%s] port [%s] \n", addr, port);
    struct addrinfo hints, *res = NULL;
    int             rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    rc = getaddrinfo(addr, port, &hints, &res );
    if (rc != 0) 
	{
        TRACE("ERROR Invalid address %s, getaddrinfo failed: %d\n", addr, rc);
        return NULL;
    }
    return res;
}


SOCKET_OBJ * CThreadSockAsyncServer::GetSocketObj(SOCKET s, int listening)
{
    SOCKET_OBJ  *sockobj=NULL;
    //메모리 할당
    sockobj = (SOCKET_OBJ *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SOCKET_OBJ));
    
	if (sockobj == NULL) 
	{		        
		CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] GetSocketObj: HeapAlloc failed!! [%d]\n",
					__LINE__ , GetLastError() );	
		
        ExitProcess(-1);
    }

    // member 초기화
    sockobj->s = s;
    sockobj->listening = listening; //TCP 경우 TRUE..
    sockobj->addrlen = sizeof(sockobj->addr);

    sockobj->event = WSACreateEvent(); //이벤트 생성
    if (sockobj->event == NULL) 
	{        
		TRACE("ERROR GetSocketObj: WSACreateEvent failed: %d\n", WSAGetLastError());

		CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] GetSocketObj: WSACreateEvent failed!! [%d]\n",
					__LINE__ , GetLastError() );
		
        ExitProcess(-1);
    }

	// 
	sockobj->nTotalOnePacketLen = 0;
	sockobj->nRecvedDataLen = 0;
    return sockobj;
}


// Function: FreeSocketObj
//
// Description:
//    Frees a socket object along with any queued buffer objects.

void CThreadSockAsyncServer::FreeSocketObj(SOCKET_OBJ *obj)
{		
	if(obj == NULL)
		return;

	if((DWORD)obj == 0xfeeefeee)
		return;

    BUFFER_OBJ  *ptr=NULL,  *tmp=NULL;

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
		obj->s = INVALID_SOCKET ;
    }	
	
    HeapFree(GetProcessHeap(), 0, obj);
	obj = NULL;
	
}


THREAD_OBJ * CThreadSockAsyncServer::GetThreadObj(int nLine)
{
    THREAD_OBJ *thread = NULL;
	
	//쓰레드 객체 위한 메모리 할당
    thread = (THREAD_OBJ *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(THREAD_OBJ));
    if (thread == NULL)
	{
		TRACE("ERROR GetThreadObj: HeapAlloc failed\n");
     
		CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] GetThreadObj: HeapAlloc failed failed!! [%d]\n",
					__LINE__ , GetLastError() );
		
        ExitProcess(-1);
    }
    thread->Event = WSACreateEvent(); //이벤트 생성
    if (thread->Event == NULL)
	{
		TRACE("ERROR GetThreadObj: WSACreateEvent failed\n");
        
		CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] GetThreadObj: WSACreateEvent failed failed!! [%d]\n",
					__LINE__ , GetLastError() );
		
        ExitProcess(-1);
    }
	
    thread->Handles[0] = thread->Event; // 0 번째는 compaction 용도.. 

    InitializeCriticalSection(&thread->ThreadCritSec);

	CLog::Instance().WriteFile(FALSE, "ServerLog", "▷ Ln [%d] GetThreadObj Called!!\n",	nLine );
	
    return thread;
}


int CThreadSockAsyncServer::InsertSocketObj(THREAD_OBJ * thread, SOCKET_OBJ *sock)
{
    int     ret;
   
    EnterCriticalSection(&thread->ThreadCritSec);
	//최대 대기 이벤트수를 넘으면 리스트에 추가하지 않음.
    if (thread->SocketCount < MAXIMUM_WAIT_OBJECTS-1)
	//if (thread->SocketCount < MAXIMUM_WAIT_OBJECTS_TEST-1) //TEST ~~
		
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
		// thread->Handles[0]은 thread 

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


void CThreadSockAsyncServer::RemoveSocketObj(THREAD_OBJ *thread, SOCKET_OBJ *sock)
{    
	EnterCriticalSection(&thread->ThreadCritSec);
	
    if (sock->prev && (DWORD)sock->prev != 0xfeeefeee )
	{
        sock->prev->next = sock->next;
    }
    if (sock->next && (DWORD)sock->next != 0xfeeefeee)
	{
        sock->next->prev = sock->prev;
    }

    if (thread->SocketList == sock)
        thread->SocketList = sock->next;

    if (thread->SocketListEnd == sock)
        thread->SocketListEnd = sock->prev;
		
	thread->SocketCount--;
    // Signal thread to rebuild array of events
    WSASetEvent(thread->Event); // => RenumberThreadArray     
    
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] RemoveSocketObj OK!! thread->SocketCount [%d]\n", __LINE__, thread->SocketCount );

	LeaveCriticalSection(&thread->ThreadCritSec);	
}


// 	쓰레드의 소켓객체 리스트로부터 인덱스를 이용, 이벤트가 발생한 소켓 객체 구함
//  (쓰레드의 이벤트 리스트에서의 이벤트 객체의 인덱스와 쓰레드의 
//   소켓 리스트의 소켓객체의 인덱스가 동일)

SOCKET_OBJ * CThreadSockAsyncServer::FindSocketObj(THREAD_OBJ *thread, int index)
{
	EnterCriticalSection(&thread->ThreadCritSec);
	
    SOCKET_OBJ *ptr=NULL;
    int         i;
    
    ptr = thread->SocketList;
    for(i=0; i < index ;i++){
        ptr = ptr->next;
    }

    LeaveCriticalSection(&thread->ThreadCritSec);

    return ptr;
}


void CThreadSockAsyncServer::EnqueueBufferObj(SOCKET_OBJ *sock, BUFFER_OBJ *obj, BOOL AtHead)
{		
	if( sock && (DWORD)sock != 0xfeeefeee )
	{
		if (sock->pending == NULL ) 
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
}



BUFFER_OBJ * CThreadSockAsyncServer::DequeueBufferObj(SOCKET_OBJ *sock)
{
    BUFFER_OBJ *ret = NULL; 	
			
	if ( sock && (DWORD)sock != 0xfeeefeee && sock->pendingtail != NULL)
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

int CThreadSockAsyncServer::SendPendingData(char * szUsage, SOCKET_OBJ *sock)
{
    BUFFER_OBJ *bufobj=NULL;
    BOOL        breakouter;
    int         nleft,  idx,  ret,  rc;

    // Attempt to dequeue all the buffer objects on the socket	
    ret = 0;	
	bufobj = DequeueBufferObj(sock);
	
    while ( bufobj ) 
	{        
		breakouter = FALSE;
		
		nleft = bufobj->buflen;
		idx = 0;
	
		while (nleft) 
		{
			rc = send(sock->s, &bufobj->buf[idx], nleft, 0);				
			
			if (rc == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK )
				{
					TRACE("★ CThreadSockAsyncServer::SendPendingData : WSAEWOULDBLOCK!!\n");
					
					//Log
					CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] SendPendingData : WSAEWOULDBLOCK! queue 앞(head)에 넣음 [%s][%s][%d]!\n", 
						__LINE__, szUsage ,sock->m_szIP, nleft );

					BUFFER_OBJ *newbuf=NULL;
					
					newbuf = GetBufferObj(nleft);
					memcpy(newbuf->buf, &bufobj->buf[idx], nleft);
					EnqueueBufferObj(sock, newbuf, TRUE); //queue 앞(head)에 넣음. 다음에 이어서 전송.
					
					/* 200703221645 return 한다!!! */
					return WSAEWOULDBLOCK;
				}
				if ( WSAGetLastError() == WSAENOBUFS)
				{
					TRACE("★ CThreadSockAsyncServer::SendPendingData : WSAENOBUFS!!\n");
					
					CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] SendPendingData : WSAENOBUFS! queue 앞(head)에 넣음 [%s][%s][%d]\n", 
						__LINE__, szUsage ,sock->m_szIP, nleft );
					
					BUFFER_OBJ *newbuf=NULL;
					
					newbuf = GetBufferObj(nleft);
					memcpy(newbuf->buf, &bufobj->buf[idx], nleft);
					EnqueueBufferObj(sock, newbuf, TRUE); //queue 앞(head)에 넣음. 다음에 이어서 전송.
					
					/* 200703221645 return 한다!!! */
					return WSAENOBUFS;
				}
				else
				{
					// The connection was broken, indicate failure
					ret = -1;

					CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] [%s] SendPendingData : connection was broken ! ToIP [%s][%d] \n", 
						__LINE__, szUsage, sock->m_szIP, WSAGetLastError()  );	
				}
				
				goto BREAK_OUT;
				break;
			}
			else
			{								
				nleft -= rc;
				idx += 0; //모두 ..

				if( m_bLog && m_bChkFilterIP )
				{
					if	( strcmp( m_szFilterIP1 , sock->m_szIP ) == 0 || strcmp( m_szFilterIP2 , sock->m_szIP ) == 0 )
					{
						CTime iDT =  CTime::GetCurrentTime();	
						char szPath [200];
						memset(szPath, 0x00, sizeof(szPath));
						_snprintf(szPath ,sizeof(szPath)-1, "\n★ %s :SendPendingData : sent [%d/%d] [%s]usage [%s]", iDT.Format("%Y%m%d%H%M%S"), 
							rc, nleft,sock->m_szIP, szUsage );
						
						m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
						
						CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] [%s] SendPendingData : sent [%d/%d] usage [%s]\n", 
							__LINE__, sock->m_szIP, rc, nleft, szUsage ); 
						
					}
				}
				else if( m_bLog && !m_bChkFilterIP)
				{
					// 화면에 뿌림.
					CTime iDT =  CTime::GetCurrentTime();	
					char szPath [200];
					
					memset(szPath, 0x00, sizeof(szPath));					
					_snprintf(szPath, sizeof(szPath)-1, "\n★ %s :SendPendingData : sent [%d/%d] [%s]usage [%s]", 
						iDT.Format("%Y%m%d%H%M%S"), rc, nleft,sock->m_szIP, szUsage ); 
					
					m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
					
					CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] [%s] SendPendingData : sent [%d/%d] usage [%s]\n", 
						__LINE__, sock->m_szIP, rc, nleft, szUsage ); 

				}
			}
		}

		FreeBufferObj(bufobj);	
		
		bufobj = DequeueBufferObj(sock);
    }
BREAK_OUT :	
    
	
	// LOGGING //////////////////////////////////////
	if( m_bLog && m_bChkFilterIP )
	{
		if	( strcmp( m_szFilterIP1 , sock->m_szIP ) == 0 || strcmp( m_szFilterIP2 , sock->m_szIP ) == 0 )
		{
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			
			memset(szPath, 0x00, sizeof(szPath));
			_snprintf(szPath,sizeof(szPath)-1, 
				"\n★ %s :SendPendingData : return [%d] [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), ret, sock->m_szIP, szUsage ); 
			
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData : return [%d] [%s] usage [%s]\n", 
				__LINE__, ret, sock->m_szIP, szUsage ); 
			
		}
	}
	else if( m_bLog && !m_bChkFilterIP)
	{
		// 화면에 뿌림.
		CTime iDT =  CTime::GetCurrentTime();	
		char szPath [200];
		
		memset(szPath, 0x00, sizeof(szPath));
		_snprintf(szPath,sizeof(szPath)-1,
			"\n★ %s :SendPendingData : return [%d] [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), ret, sock->m_szIP, szUsage ); 
		
		m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
		
		CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData : return [%d] [%s] usage [%s]\n", 
			__LINE__, ret, sock->m_szIP, szUsage ); 
	}
	// LOGGING //////////////////////////////////////

    return ret;
}


// ▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩
// 클라이언트의 작업 요청별 처리
// ▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩

BOOL	CThreadSockAsyncServer::ProcessRecvData(SOCKET_OBJ * pSockObj)
{
	static BOOL( CThreadSockAsyncServer::* fPAryServer[] )( SOCKET_OBJ * ) = 
	{	// doDummySqlRsltWork 만 하면 -> 표준에 의해서 VC2008 부터는 Compiler Error C3867 
		&CThreadSockAsyncServer::doWork_STATUS_AWAY,				//  0
		&CThreadSockAsyncServer::doWork_STATUS_ONLINE,              //z	
		&CThreadSockAsyncServer::doWork_Chat,                       //
		&CThreadSockAsyncServer::doWork_GetBuddyGrpList,	        //      GET_BUDDY_GRP   
		&CThreadSockAsyncServer::doWork_GetBuddyList,			    //      GET_BUDDY_LIST
		&CThreadSockAsyncServer::doWork_SearchID,	                //      SEARCH_ID
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_JOB_GN
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_DAILYJOB_MST
		&CThreadSockAsyncServer::doWork_Dummy ,						//		RAWSQL_LOGIN => 더이상 사용안됨 , doWork_LOGIN 대체	
		&CThreadSockAsyncServer::doRawSqlWork,						//  	RAWSQL_GETUSER_INFO
		&CThreadSockAsyncServer::doRawSqlWork,						//	10	RAWSQL_TEAM
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_GETCOMPANY_INFO
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_GETCLASS_INFO
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_GETBUDDY_INFO
		&CThreadSockAsyncServer::doWork_Dummy,						//  	SOMEONE_ADD_ME
		&CThreadSockAsyncServer::doWork_DelBuddyGrp,				//	15	DEL_BUDDY_GRP
		&CThreadSockAsyncServer::doWork_DelBuddy,					//		DEL_BUDDY
		&CThreadSockAsyncServer::doWork_ChgBuddyGrp,				//		CHG_BUDDY_GRP	
		&CThreadSockAsyncServer::doWork_AddBuddyGrp,				//		ADD_BUDDY_GRP
		&CThreadSockAsyncServer::doWork_ChgGrpName,					//  	CHG_GRP_NAME
		&CThreadSockAsyncServer::doWork_AllowBuddy,					//	20	YOUALLOWEDASCOMP
		&CThreadSockAsyncServer::doWork_Dummy,						//		SOMEONE_DEL_ME 
		&CThreadSockAsyncServer::doWork_Dummy,						//		ACK_NICK_CHG
		&CThreadSockAsyncServer::doWork_SomeoneWantsSendFiles,		//		SOMEONE_WANTS_SEND_FILES			
		&CThreadSockAsyncServer::doWork_Dummy,						//		MULTICHAT_SERVER_ROOMNO  
		&CThreadSockAsyncServer::doWork_CANCEL_FILE_TRANSFER,		//	25	CANCEL_FILE_TRANSFER
		&CThreadSockAsyncServer::doWork_AllMsg,						//		ALL_MSG
		&CThreadSockAsyncServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		ALL_MSG_ACK	
		&CThreadSockAsyncServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		FOLDER_SELECTING 
		&CThreadSockAsyncServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		NO_THANKS_FILE_TRANSFER
		&CThreadSockAsyncServer::doRawSqlMultiOneTranWork,			//	30	RAWSQL_REG_NEW_USER
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_SAVE_LAST_IP 
		&CThreadSockAsyncServer::doWork_Dummy,						//		ALREADY_LOGIN 
		&CThreadSockAsyncServer::doWork_Dummy,						//		LOGIN_ALLOWED
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_INSERT_LOGONTIME
		&CThreadSockAsyncServer::doRawSqlWork,						//	35	RAWSQL_CHK_DUP_ID
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_CHK_USERID	
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_UPDATE_PASSWD
		&CThreadSockAsyncServer::doWork_Dummy,						//		ADD_BUDDY_LIST_ERR	
		&CThreadSockAsyncServer::doWork_Dummy,						//		ACK_ONLINE	  		
		&CThreadSockAsyncServer::doWork_Dummy,						//	40	ACK_OFFLINE
		&CThreadSockAsyncServer::doWork_Dummy,						//		YOUALLOWEDASCOMP_INFORM	
		&CThreadSockAsyncServer::doRawSqlWork,						//		RAWSQL_UPDATE_LOGOFFTIME 
		&CThreadSockAsyncServer::doWork_AddBuddyList,				//		ADD_BUDDY_LIST		
		&CThreadSockAsyncServer::doWork_CHK_ALREADY_LOGIN,			//		CHK_ALREADY_LOGIN
		&CThreadSockAsyncServer::doWork_ChngNic,					//	45	CHNG_NICK	
		&CThreadSockAsyncServer::doWork_MultiChatGetRoomNo,			//      MULTICHAT_GET_ROOMNO
		&CThreadSockAsyncServer::doWork_MultiChat,	         		//      MULTI_CHAT_MSG
		&CThreadSockAsyncServer::doWork_Dummy,						//		SET_MULTICHAT_SERVER_ROOMNO	
		&CThreadSockAsyncServer::doWork_EXIT_MULTI_CHAT ,           //      EXIT_MULTI_CHAT
		&CThreadSockAsyncServer::doWork_SLIP_MSG        ,            //  50  SLIP_MSG
		&CThreadSockAsyncServer::doWork_GET_SLIP_MSG    ,            //      GET_SLIP_MSG
		&CThreadSockAsyncServer::doRawSqlWork,                       //      RAWSQL_ACK_SLIP_MSG
		&CThreadSockAsyncServer::doWork_ADD_IT_COMP     ,            //      ADD_IT_COMP  
		&CThreadSockAsyncServer::doWork_DEL_IT_COMP     ,            //  
		&CThreadSockAsyncServer::doWork_CHG_IT_COMP     ,            //  55
		&CThreadSockAsyncServer::doWork_Dummy           ,            //  56  GET_COMP_LIST_INIT
		&CThreadSockAsyncServer::doWork_STATUS_BUSY     ,            //  57 
		&CThreadSockAsyncServer::doWork_LOGIN           ,            //  58  LOGIN 
		&CThreadSockAsyncServer::doWork_FOOD_MENU       ,            //  59  LUNCH_MENU
		&CThreadSockAsyncServer::doWork_ALL_USER_ID_TEAM ,           //  60
		&CThreadSockAsyncServer::doWork_Dummy           ,            //  61  GET_ALL_USER_INIT
		&CThreadSockAsyncServer::doWork_ALL_USER_ID_TEAM,            //  62  대화상대 추가시 사용 
		&CThreadSockAsyncServer::doWork_Dummy           ,            //  63  UPDATE_ALL_USER_INIT 
		&CThreadSockAsyncServer::doWork_Dummy           ,            //  64  ALERT_PENDING_WORK 
		&CThreadSockAsyncServer::doWork_ALIVE           ,            //  65  IM_ALIVE
	} ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pSockObj->m_pBuff;

	if( m_bLog && m_bChkFilterIP )
	{
		if	( strcmp( m_szFilterIP1 , pSockObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSockObj->m_szIP ) == 0 )
		{
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			
			memset(szPath, 0x00, sizeof(szPath));
			_snprintf(szPath,sizeof(szPath)-1,
				"\n★ %s : ProcessRecvData [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSockObj->m_szIP, m_szUsageAry[atoi(psHeader->m_szUsage)] ); 
			
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ★★★ [%s] ProcessRecvData / usage [%s/%s]\n", 
				__LINE__, pSockObj->m_szIP, psHeader->m_szUsage, m_szUsageAry[atoi(psHeader->m_szUsage)]  ); 
		}
	}
	else if( m_bLog && !m_bChkFilterIP)
	{
		// 화면에 뿌림.
		CTime iDT =  CTime::GetCurrentTime();	
		char szPath [200];
		
		memset(szPath, 0x00, sizeof(szPath));
		_snprintf(szPath, sizeof(szPath)-1,
			"\n★ %s : ProcessRecvData [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), 
			pSockObj->m_szIP, m_szUsageAry[atoi(psHeader->m_szUsage)] );
		
		m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
		
		CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ★★★ [%s] ProcessRecvData / usage [%s/%s]\n", 
			__LINE__, pSockObj->m_szIP, psHeader->m_szUsage, m_szUsageAry[atoi(psHeader->m_szUsage)]  ); 
	}

	BOOL bRslt = (this->*fPAryServer[ atoi(psHeader->m_szUsage)]) (pSockObj);   
	
	TRACE("▼ ProcessRecvData : Usage [%s][%s] IP [%s] Rslt[%d]\n", psHeader->m_szUsage, 
		m_szUsageAry[atoi(psHeader->m_szUsage)], pSockObj->m_szIP, bRslt  );
	
	return TRUE;
}

// Multi Chat Room No Get
BOOL CThreadSockAsyncServer::doWork_MultiChatGetRoomNo(SOCKET_OBJ* pSockObj )	
{
	// User ID 로 임시 set을 구성후, 동일한 사용자로 이루어진 
	// 멀티채팅 Session 이 이미 존재하는지 확인한다
	// 없다면 새로운 RoomNo key값을 생성후 map 에 저장후, RoomNo를 요청자에게 알려준다.
	// 동일한 사용자들의 set 존재시는 그것의 RoomNo를 알려준다.    
	// RoomNo : Multi Chat Session = 1:1
	
	//SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	//toDo!
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	int nLen = 	atoi(psHeader->m_szLength);
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	

	
	char szTmpOutID[20+1]; //ID 
	char szTmpOutName[50+1]; // NAME	
	char szData[1024];
	char szMultiChatUser[ ((20+1+1+50+1+1) * 200) +1]; // ID|NAME 200명 Max (20+1+1+50+1 * 50)+null

	memset(szTmpOutID , 0x00, sizeof(szTmpOutID));		
	memset(szTmpOutName , 0x00, sizeof(szTmpOutName));		
	memset(szData , 0x00, sizeof(szData));	
	memset(szMultiChatUser , 0x00, sizeof(szMultiChatUser));	
	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
		
	int nPos = 0;	
	// M Chat 요청 Client Dlg ID
	char szClientDlgID [15+1];
	memset(szClientDlgID , 0x00, sizeof(szClientDlgID));	

	nPos = GetStrByDelim( szData, szClientDlgID, DELIM1);
	SET_STR setMultiChatID; 

	while(1)
	{
		memset(szTmpOutID, 0x00, sizeof(szTmpOutID));		
		
		nPos = GetStrByDelim( szData, szTmpOutID, '|' );
		
		if(nPos < 0)
		{
			break;
		}

		//ID|NAME|ID|NAME|...
		if(nPos != -1 && strlen(szTmpOutID) > 0 )		
		{
			// SET_MULTICHAT_SERVER_ROOMNO 로 보낼 문자열 구성
			strcat(szMultiChatUser, szTmpOutID);
			strcat(szMultiChatUser, "|"); // ID
			
			setMultiChatID.insert( szTmpOutID ) ; // ID 만 저장 
			//TRACEX ("szTmpOutID [%s]\n", szTmpOutID );

			memset(szTmpOutName, 0x00, sizeof(szTmpOutName));		
			nPos = GetStrByDelim( szData, szTmpOutName, '|' ); //NAME
			strcat(szMultiChatUser, szTmpOutName);
			strcat(szMultiChatUser, "|"); // NAME
		}
	}
	
	char szServerRoomNo [5+1];
	memset(szServerRoomNo , 0x00, sizeof(szServerRoomNo));
	
	MAP_STR2SET::iterator it;
	//SET_STR::iterator itSet;

	for (it= m_mapStr2SetMultiChat.begin(); it != m_mapStr2SetMultiChat.end(); ++it) 
	{
		if(it != m_mapStr2SetMultiChat.end() )  
		{
			if( setMultiChatID == (*it).second )
			{
				TRACE("동일한 사용자를 가진 Session이 존재함 해당 Room No 를 돌려준다.\n");	
				strncpy(szServerRoomNo ,  (char*) ((*it).first).c_str() , sizeof(szServerRoomNo) );
				// (*it).first ...								
			}
		}
	}

	if(strlen(szServerRoomNo) == 0)
	{
		if(m_nMultiChatRoomNo == 99999l)
		{
			m_nMultiChatRoomNo = 0l; //ReSet
		}
				
		// Multi Chat Map 에 새로 저장함..
		// Lock!
		{	//local
			CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");
			
			m_nMultiChatRoomNo++;

			memset(szServerRoomNo, 0x00, sizeof(szServerRoomNo) );
			_snprintf(szServerRoomNo, sizeof(szServerRoomNo)-1, "%ld", m_nMultiChatRoomNo );			

			m_mapStr2SetMultiChat.insert( MAP_STR2SET::value_type( szServerRoomNo , setMultiChatID ) );
			
		}
	}
		
	S_COMMON_HEADER sMSGHeader;
	S_SERVER_ROOMNO   sServerRoomNo;
	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(&sServerRoomNo, NULL, sizeof(sServerRoomNo));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// 멀티 Chat에 소속된 상대방 (최초 요청자 제외)들에게 대화상대 목록을 보낸다.!
	// SET_MULTICHAT_SERVER_ROOMNO 으로 보냄!!! (MULTICHAT_SERVER_ROOMNO 아님!!)
	
	SET_STR::iterator itSet;
	
	char szServerRoomNoTmp[5+1+1]; // + DELIM1 
	memset(szServerRoomNoTmp , 0x00, sizeof(szServerRoomNoTmp));	
	_snprintf(szServerRoomNoTmp, sizeof(szServerRoomNoTmp)-1, "%s%c",szServerRoomNo, DELIM1);

	for (itSet= setMultiChatID.begin(); itSet != setMultiChatID.end(); ++itSet) 
	{
		if(itSet != setMultiChatID.end() )  
		{
			if( strcmp(pInfo->m_szFromID,  (*itSet).c_str() ) != 0)
			{				
				//RoomNo + IDs...

				iLength = strlen(szServerRoomNo) + 1+ strlen(szMultiChatUser) + 1;
	
				// 헤더부 설정	
				memcpy(sMSGHeader.m_szUsage, SET_MULTICHAT_SERVER_ROOMNO, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));					
				
				//연결된 클라이언트에서 전달대상 검색후 전송
				
				CIpSockObjMapInfo *pa = NULL;
				
				{//local
					
					CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");
					
					if(mIDIPMap.Lookup((*itSet).c_str(), ( CObject*& ) pa ))
					{
						CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
						
						if(pSpckObjMap )
						{			
							// 전송데이터 모으기
							// COMMON_HEADER + RoomNo+DELIM1+ ID1|ID2|ID3...
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
							memset(pSzBuf, NULL, sizeof(pSzBuf));	
							memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
							memcpy(pSzBuf + COMMON_HEADER_LENGTH, szServerRoomNoTmp, strlen(szServerRoomNoTmp));						
							memcpy(pSzBuf + COMMON_HEADER_LENGTH+strlen(szServerRoomNoTmp), szMultiChatUser, strlen(szMultiChatUser));						
																
							nTotalLen = COMMON_HEADER_LENGTH + iLength;

							int nRtn = SendDataToClient("doWork_MultiChatGetRoomNo", pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen) ;
							
							if(nRtn == -1 )
							{
								CLog::Instance().WriteFile(FALSE, "ServerError", "Ln[%d] [%s]doWork_MultiChatGetRoomNo SendDataToClient!!\n",
									__LINE__, pInfo->m_szFromID );	
								
							}
						}
					}					
				}				
			}
		}
	}

	// Multi Chat 요청자에게 RoomNo전송
	iLength = sizeof(S_SERVER_ROOMNO) + 1;
	
	// 헤더부 설정	
	memcpy(sMSGHeader.m_szUsage, MULTICHAT_SERVER_ROOMNO, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
	memcpy(sServerRoomNo.m_szClientDlgID , szClientDlgID , sizeof(sServerRoomNo.m_szClientDlgID));
	memcpy(sServerRoomNo.m_szServerRoomNo , szServerRoomNo , sizeof(sServerRoomNo.m_szServerRoomNo));	
		
	CIpSockObjMapInfo *pa= NULL;		
		
	{ //local scope 
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");

		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap )
			{			
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
				memset(pSzBuf, NULL, sizeof(pSzBuf));	
				memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
				memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sServerRoomNo, sizeof(sServerRoomNo));				
							
				nTotalLen = COMMON_HEADER_LENGTH + iLength;
				
				int nRtn = SendDataToClient("doWork_MultiChatGetRoomNo",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen) ;
				
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError", "Ln[%d] [%s] doWork_MultiChatGetRoomNo SendDataToClient!!\n",
									__LINE__, pInfo->m_szFromID);					
				}
			}
		}
	}	
	
	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_EXIT_MULTI_CHAT( SOCKET_OBJ* pSockObj )
{
	//SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
			
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	MAP_STR2SET::iterator it;
	SET_STR::iterator itSet;

	it= m_mapStr2SetMultiChat.find(pInfo->m_szToID) ; // pInfo->m_szToID => Server Room No
	
	if(it != m_mapStr2SetMultiChat.end())
	{
		// set 에서 Multi Chat 나간 사용자를 제거한다.
		itSet = (*it).second.find(pInfo->m_szFromID);

		if(itSet != (*it).second.end())
		{
			TRACE("set 에서 Multi Chat 나간 사용자 [%s]를 제거한다.\n" , pInfo->m_szFromID );

			(*it).second.erase(itSet) ;		
		}

		// 만약 set 의 데이터가 없으면 (모두 나감) map 에서 제거한다. 
		if( (*it).second.empty())
		{
			TRACE("set 의 데이터가 없으면 (모두 나감) map 에서 제거\n");
			
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_EXIT_MULTI_CHAT");
				m_mapStr2SetMultiChat.erase(it);			
			}
		}
		else
		{
			// Multi chat 소속자들에게 전송 		
			S_COMMON_HEADER sMSGHeader;
			S_FROMTOID_HEADER sFormToInfo;
			memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
			memset(&sFormToInfo, 0x00, sizeof(sFormToInfo));

			int iLength = FROMTOID_HEADER_LENGTH;
			int nTotalLen = 0;
			
			char* pSzBuf = NULL;			
			//int  nSendRtn = SOCKET_ERROR;
			char szTemp[20];		
					
			SET_STR::iterator itSet;		
			
			for (itSet= (*it).second.begin(); itSet != (*it).second.end(); ++itSet) 
			{
				if(itSet != (*it).second.end() )  
				{				
					if( strcmp(pInfo->m_szFromID,  (*itSet).c_str() ) != 0)
					{		
						TRACE("전송 (*itSet).c_str() = [%s]\n",  (*itSet).c_str());

						// 헤더부 설정	
						memcpy(sMSGHeader.m_szUsage, EXIT_MULTI_CHAT, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));					

						memcpy(sFormToInfo.m_szToID, pInfo->m_szToID, sizeof(sFormToInfo.m_szToID)); // RoomNo					
						memcpy(sFormToInfo.m_szFromID, pInfo->m_szFromID, sizeof(sFormToInfo.m_szFromID));					
											
						//연결된 클라이언트에서 전달대상 검색후 전송					
						CIpSockObjMapInfo *pa= NULL;		
												
						{ //local scope 로...
							CAutoLock ScopeLock ( m_ScopeCS, "doWork_EXIT_MULTI_CHAT");
							if(mIDIPMap.Lookup((*itSet).c_str(), ( CObject*& ) pa ))
							{
								CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
								
								if(pSpckObjMap )
								{			
									// 전송데이터 모으기							
									pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
									memset(pSzBuf, NULL, sizeof(pSzBuf));	
									memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);							
									memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sFormToInfo, FROMTOID_HEADER_LENGTH );						
																		
									nTotalLen = COMMON_HEADER_LENGTH + iLength;
									
									int nRtn  = SendDataToClient("doWork_EXIT_MULTI_CHAT",pInfo->m_szFromID,pSpckObjMap, pSzBuf, nTotalLen) ;

									if(nRtn == -1 )
									{
										CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_EXIT_MULTI_CHAT SendDataToClient\n",
											__LINE__, pInfo->m_szFromID );	
																			
									}
								}
							}
						}
					}
				}
			}
		}		
	}	

	return TRUE;
}

// 조회성, 처리성 Raw SQL 1개씩 처리 
BOOL  CThreadSockAsyncServer::doRawSqlWork( SOCKET_OBJ* pSockObj )
{
	//SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char *szData = szRawData + COMMON_HEADER_LENGTH ;	

	if	(	
			memcmp(psHeader->m_szUsage, RAWSQL_UPDATE_LOGOFFTIME, sizeof(psHeader->m_szUsage)) == 0 || 
			memcmp(psHeader->m_szUsage, RAWSQL_ACK_SLIP_MSG, sizeof(psHeader->m_szUsage)) == 0 		
		)
	{
		S_FROMTOID_HEADER sFTInfo ;
		memcpy(&sFTInfo , szData , sizeof(S_FROMTOID_HEADER));		
				
		COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
		pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

		pSqlWork->SetSQL( szData +  sizeof(S_FROMTOID_HEADER) ); 

		TRACE( "[%s]\n", pSockObj->m_szIP );

		//pSqlWork->SetID( psHeader->m_szUsage, "", "", pSockObj->m_szIP );
		
		CString strSockID ;
		strSockID.Format("%d", pSockObj->s) ;	
		pSqlWork->SetID( psHeader->m_szUsage, "", "", strSockID );  

		pSqlWork->setNotiProcThread(SQLNotifyProc , this); 		
		
		global::CThreadPool::instance().queue_request( pSqlWork );
	}	
	else
	{
		// USER ID로 로그인 하기전 .. => IP로 구분됨..

		if	(	
				memcmp(psHeader->m_szUsage, RAWSQL_SAVE_LAST_IP, sizeof(psHeader->m_szUsage)) == 0 || 
				memcmp(psHeader->m_szUsage, RAWSQL_JOB_GN, sizeof(psHeader->m_szUsage)) == 0 || 		
				memcmp(psHeader->m_szUsage, RAWSQL_DAILYJOB_MST, sizeof(psHeader->m_szUsage)) == 0 || 		
				memcmp(psHeader->m_szUsage, RAWSQL_INSERT_LOGONTIME, sizeof(psHeader->m_szUsage)) == 0 
			)
		{
			// SQL작업 하지 않고 처리한다.
		}
		else
		{
			S_FROMIP_HEADER sFTInfo ;
			memcpy(&sFTInfo , szData , sizeof(S_FROMIP_HEADER));

			// Memory POOL 이용 수정 예정..
			//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
			
			COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
			pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

			pSqlWork->SetSQL( szData +  sizeof(S_FROMIP_HEADER) ); 

			TRACE( "[%s]\n", pSockObj->m_szIP );

			CString strSockID ;
			strSockID.Format("%d", pSockObj->s) ;	

			//pSqlWork->SetID( psHeader->m_szUsage, "", "", pSockObj->m_szIP);
			pSqlWork->SetID( psHeader->m_szUsage, "", "", strSockID );

			pSqlWork->setNotiProcThread(SQLNotifyProc , this); 		
			
			global::CThreadPool::instance().queue_request( pSqlWork );		
		}		
	}	

	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_ADD_IT_COMP(SOCKET_OBJ* pSockObj)	
{	
	char* szRawData= pSockObj->m_pBuff ;	
	char * pData = szRawData + COMMON_HEADER_LENGTH;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;			
	S_ADD_IT_COMP sAddITComp;
	memcpy(&sAddITComp, pData+FROMTOID_HEADER_LENGTH, sizeof(S_ADD_IT_COMP) );

	char szDescUrl[500];		
	char szSql [1024];
	
	memset(szDescUrl, 0x00 , sizeof(szDescUrl));	
	memset(szSql, 0x00 , sizeof(szSql));	
		
	sprintf(szDescUrl, "%s%c%s%c", sAddITComp.m_szDesc, DELIM1, sAddITComp.m_szUrl, DELIM1 );

	sprintf(szSql, "INSERT INTO IT_BUDDY_LIST(USER_ID,IT_NAME,IT_INFO,REG_DATE) VALUES('%s','%s','%s',SYSDATE)", 
		pInfo->m_szFromID, sAddITComp.m_szDesc , sAddITComp.m_szUrl); 

	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( ADD_IT_COMP, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->SetUsefulInfo(szDescUrl);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;
}



//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL  CThreadSockAsyncServer::doWork_DEL_IT_COMP(SOCKET_OBJ* pSockObj)	
{
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	int nLen = atoi(psHeader->m_szLength);
	char* pData = szRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	char szTmpOut[100+1];
	char szData[2048];
	memset(szData , 0x00, sizeof(szData));	
	CString strSrc;	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	strSrc = szData;	
	
	int nPos = -1;	
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	//TRACEX("szSessionNameIndex [%s]\n", szSessionNameIndex);

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	oresult nOsessRslt = osess.Open();

	ODatabase odb;
	char szErrMsg [100];
	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		osess.Close();
		odb.Close();
		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	//osess.Open();

	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		osess.Close();
		odb.Close();
		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{								
		osess.Close();		
		odb.Close();
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}	
	
	if ( ! odb.IsOpen() )
	{ 
		osess.Close();
		odb.Close();		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		goto EXIT_HOLE;
	}
		
	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szTmpOut, DELIM1);		
				
		if(nPos != -1 && strlen(szTmpOut) > 0 )		
		{			
			char szSql [400];			
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "DELETE FROM IT_BUDDY_LIST WHERE USER_ID ='%s' AND IT_NAME ='%s'", pInfo->m_szFromID,  szTmpOut); 
									
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();				

				bRslt = FALSE;
				goto EXIT_HOLE;
			}			
		}
		
		if(nPos < 0)
		{
			//pSqlWork1->SetID( DEL_IT_COMP, pInfo->m_szToID, pInfo->m_szFromID,  ""); 						
			break;
		}
	}
	
	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();
	

EXIT_HOLE:

	OShutdown();
	
	// 처리 결과 전송!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, DEL_IT_COMP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));			
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송
	
	CIpSockObjMapInfo *pa= NULL;
		
	{	
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_DEL_IT_COMP");
		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap )
			{			
				// 전송데이터 모으기
				char* pSzBuf = NULL;
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
				
				int nRtn = SendDataToClient("doWork_DEL_IT_COMP", pInfo->m_szFromID,pSpckObjMap, pSzBuf, nTotalLen);

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_DEL_IT_COMP SendDataToClient\n",
											__LINE__, pInfo->m_szFromID );
				}
			}			
		}
	}
	
	
	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_CHG_IT_COMP( SOCKET_OBJ* pSockObj )	
{
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;
	
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char * pData = szRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	//TRACEX("doWork_ADD_IT_COMP m_szFromID [%s]\n", pInfo->m_szFromID); 	
	
	S_CHG_IT_COMP sChgITComp;
	memcpy(&sChgITComp, pData+FROMTOID_HEADER_LENGTH, sizeof(S_CHG_IT_COMP) );

	char szDescUrl[600];		
	char szSql [1024];
	
	memset(szDescUrl, 0x00 , sizeof(szDescUrl));	
	memset(szSql, 0x00 , sizeof(szSql));	
	
	sprintf(szDescUrl, "%s%c%s%c%s%c", sChgITComp.m_szDescOld, DELIM1, sChgITComp.m_szDesc, DELIM1, sChgITComp.m_szUrl, DELIM1 );

	sprintf(szSql, "UPDATE IT_BUDDY_LIST SET IT_NAME='%s',IT_INFO='%s' WHERE USER_ID='%s' AND IT_NAME='%s'", 
		sChgITComp.m_szDesc, sChgITComp.m_szUrl, pInfo->m_szFromID, sChgITComp.m_szDescOld); 

	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( CHG_IT_COMP, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->SetUsefulInfo(szDescUrl);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;

}


BOOL  CThreadSockAsyncServer::doWork_SLIP_MSG( SOCKET_OBJ* pSockObj )
{	
	char* szRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char * pData = szRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;		
	
	char szMsg [1024+1];
	char szSql [1200];
	memset(szMsg, 0x00 , sizeof(szMsg));
	memset(szSql, 0x00 , sizeof(szSql));
	
	strncpy(szMsg , pData+FROMTOID_HEADER_LENGTH, atoi(psHeader->m_szLength) - FROMTOID_HEADER_LENGTH - 1);

	sprintf(szSql, "INSERT INTO BUDDY_MSG(FROM_ID,TO_ID,MSG) VALUES('%s','%s','%s')", pInfo->m_szFromID, pInfo->m_szToID, szMsg ) ;

	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	

	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( SLIP_MSG, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_GET_SLIP_MSG( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;
	
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char * pData = szRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	//TRACEX("doWork_GET_SLIP_MSG m_szFromID [%s] \n", pInfo->m_szFromID); 	
		
	char szSql [300];	
	memset(szSql, 0x00 , sizeof(szSql));
 
	sprintf(szSql, "SELECT /*+ ORDERED USE_NL(A B)*/ A.FROM_ID, B.USER_NAME,A.MSG,TO_CHAR(A.REG_DATE,'YYYYMMDDHH24MISS') FROM BUDDY_MSG A, KM_DOCUSER B	\
WHERE A.TO_ID='%s' AND A.CHKED='N' AND B.USER_ID = A.FROM_ID", pInfo->m_szFromID) ;

    //COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_SLIP_MSG, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	   

	// 업데이트 안되는 사용자들에게 공지 날린다.
	/*
	if( strcmp (pInfo->m_szToID, "VERSION_CHK") != 0 )
	{
		m_strAryForUpdate.Add (pInfo->m_szFromID) ;
	}
	else
	{
		//지운다
		for(int k=0; k< m_strAryForUpdate.GetSize(); k++)
		{
			if( ( m_strAryForUpdate.ElementAt(k) ).Compare( pInfo->m_szFromID) == 0  )
			{
				m_strAryForUpdate.RemoveAt(k);
			}
		}		
	}
	*/

	return TRUE;
}


BOOL  CThreadSockAsyncServer::doWork_FSELING_ALLMACK_NOTHANKF( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;

	char* szData = szRawData + COMMON_HEADER_LENGTH ;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) szData;
	S_COMMON_HEADER sMSGHeader;
	S_FROMTOID_HEADER sFormToInfo;
	memset(&sFormToInfo, 0x00, sizeof(sFormToInfo));

#ifdef _DEBUG
		if(	memcmp(psHeader->m_szUsage, FOLDER_SELECTING, sizeof(psHeader->m_szUsage)) == 0 )
		{
		//	TRACEX("FOLDER_SELECTING\n");
		}
		else 
		if(	memcmp(psHeader->m_szUsage, NO_THANKS_FILE_TRANSFER, sizeof(psHeader->m_szUsage)) == 0 )
		{
		//	TRACEX("NO_THANKS_FILE_TRANSFER\n");
		}
#endif
		
	char szTemp[20];
	char* pSzBuf = NULL;
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(szTemp, NULL, sizeof(szTemp));
	
	CIpSockObjMapInfo*  paTmp;
	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_FSELING_ALLMACK_NOTHANKF");
	
		if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) paTmp))
		{			
			if(paTmp )
			{
				// 헤더부 설정		
				int iLength = FROMTOID_HEADER_LENGTH;		
				
				memcpy(sMSGHeader.m_szUsage, psHeader->m_szUsage, sizeof(sMSGHeader.m_szUsage));		
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));				
				
				
				memcpy(sFormToInfo.m_szFromID, pInfo->m_szFromID, sizeof(sFormToInfo.m_szFromID));
				memcpy(sFormToInfo.m_szToID, pInfo->m_szToID, sizeof(sFormToInfo.m_szToID));
				//memcpy(sFormToInfo.m_szIP, pInfo->m_szIP , sizeof(sFormToInfo.m_szIP));	
				memcpy(sFormToInfo.m_szIP, pSockObj->m_szIP , sizeof(sFormToInfo.m_szIP));	
				
				
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH ]; //!!!
				memset(pSzBuf, NULL, sizeof(pSzBuf));	
				
				memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);					
				memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFormToInfo, sizeof(sFormToInfo));	
				int nTotalLen = COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH;
				
				int nRtn = SendDataToClient("doWork_FSELING_ALLMACK_NOTHANKF",pInfo->m_szToID, paTmp, pSzBuf, nTotalLen) ;

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_FSELING_ALLMACK_NOTHANKF SendDataToClient\n",
											__LINE__, pInfo->m_szFromID );
				}			
			}
		}
	}
	
		
	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_Dummy( SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	// char* szRawData= pSockObj->m_pBuff ;

	return TRUE;
}

BOOL  CThreadSockAsyncServer::doWork_CANCEL_FILE_TRANSFER( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;
	
	char* szData = szRawData + COMMON_HEADER_LENGTH ;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) szData;	
	
	//////////////////////		
	S_COMMON_HEADER sMSGHeader;
	S_CANCEL_FILE_TRANSFER sCancelInfo;
	memset(&sCancelInfo, 0x00, sizeof(sCancelInfo));
	
	char szTemp[20];
	char* pSzBuf = NULL;
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	CIpSockObjMapInfo*  paTmp;
	
	
	{	
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_FSELING_ALLMACK_NOTHANKF");

		if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) paTmp))
		{			
			if(paTmp )
			{
				// 헤더부 설정		
				memcpy(sMSGHeader.m_szUsage, CANCEL_FILE_TRANSFER, sizeof(sMSGHeader.m_szUsage));		
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, sizeof(S_CANCEL_FILE_TRANSFER) );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));			
				memcpy(sCancelInfo.szFromID, pInfo->m_szFromID, sizeof(sCancelInfo.szFromID));			
				memcpy(sCancelInfo.szFromIP, pSockObj->m_szIP , sizeof(sCancelInfo.szFromIP)); 
				
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + sizeof(S_CANCEL_FILE_TRANSFER) ]; //!!!
				memset(pSzBuf, NULL, sizeof(pSzBuf));	
				
				memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);					
				memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sCancelInfo, sizeof(S_CANCEL_FILE_TRANSFER));					
				
				int nTotalLen = COMMON_HEADER_LENGTH + sizeof(S_CANCEL_FILE_TRANSFER);
				
				int nRtn = SendDataToClient("doWork_CANCEL_FILE_TRANSFER", pInfo->m_szToID, paTmp, pSzBuf, nTotalLen) ;
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_CANCEL_FILE_TRANSFER SendDataToClient\n",
											__LINE__, pInfo->m_szFromID );
				}			
			}
		}
	}
	
	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 여러개의 처리성 SQL이 하나의 TRANSACTION 으로 처리되어야 하는 경우
BOOL  CThreadSockAsyncServer::doRawSqlMultiOneTranWork( SOCKET_OBJ* pSockObj )
{	
	// SOCKET_OBJ* pSockObj
	char* szRawData= pSockObj->m_pBuff ;

	BOOL bExists = FALSE;
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char *szData = szRawData + COMMON_HEADER_LENGTH ;

	// 20060705 현재 분기될 경우 없으므로 막음 (성능)
	//if( memcmp(szUsage, RAWSQL_REG_NEW_USER, sizeof(szUsage)) == 0 )	
	//{		
	
	S_FROMIP_HEADER sChkDupInfo ;
	memcpy(&sChkDupInfo , szData , sizeof(S_FROMIP_HEADER));
	
	const string str = szData +  sizeof(S_FROMIP_HEADER) ;
	vector<string> SqlVec;
	const string& delimiters = "!" ;

	// 처음에 나와있는 구분자들 Skip
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    
	// 첫번째 "구분자가 아닌 문자" 찾는다.
	string::size_type pos  = str.find_first_of(delimiters, lastPos);
	
	while (string::npos != pos || string::npos != lastPos)
	{
		// 토큰을 찾았으면 vector에 추가.
		SqlVec.push_back(str.substr(lastPos, pos - lastPos));
		
		// 구분자를 SKIP
		lastPos = str.find_first_not_of(delimiters, pos);
		
		// 그다음 "구분자가 아닌 문자"를 찾음
		pos = str.find_first_of(delimiters, lastPos);
	} 	

	int nCnt = SqlVec.size();
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));
	
	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
		
	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!
	
	oresult nOsessRslt = osess.Open();

	BOOL bRslt = FALSE;
	ODatabase odb;
	char szErrMsg [100];

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );		
		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		
		goto EXIT_HOLE;
	}		
	
	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	if ( ! osess.IsOpen() )
	{			
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
			
		goto EXIT_HOLE;
	}
	
	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}	
	
	TRACE("★ nCnt [%d]\n", nCnt );
	
	vector<string>::iterator location ;
	
	for (location=SqlVec.begin(); location != SqlVec.end(); ++location) 
	{
		//TRACE("%s\n", (*location).c_str() );
		bExists = TRUE;
		if (odb.ExecuteSQL( (*location).c_str() ) != OSUCCESS)
		{
			sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					
			
			TRACE("★ ERROR!! [%s]\n", szErrMsg );
			
			osess.Rollback(false); 
			
			odb.Close();
			osess.Close();						
			bRslt = FALSE;
			goto EXIT_HOLE;
		}
	}

	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();
	

EXIT_HOLE:
	
	OShutdown();

	if(bExists)
	{
		S_COMMON_HEADER sMSGHeader;
		char szTemp [20];
		memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
		memset(szTemp, 0x00, sizeof(szTemp));
		char* pSzBuf = NULL;
		S_COM_RESPONSE sComRslt;
		memset(&sComRslt, 0x00, sizeof(sComRslt));

		TRACE("사용자 등록 데이터 결과 처리\n");	
		int iLength = sizeof(S_COM_RESPONSE);				
		
		// 데이터부 설정
		if(bRslt == TRUE)
		{
			memcpy(sComRslt.szRsltCd, "Y", min(sizeof(sComRslt.szRsltCd), strlen("Y")));			
		}
		else
		{
			memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
		}

		memcpy(sComRslt.szErrMsg, szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(szErrMsg)));
		
		// 헤더부 설정	
		memcpy(sMSGHeader.m_szUsage, RAWSQL_REG_NEW_USER, sizeof(sMSGHeader.m_szUsage));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
		memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
		memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
		//연결된 클라이언트에서 전달대상 검색후 전송		
		CIpSockObjMapInfo *pa= NULL;
		// mIPSockObjMap key: IP 
				
		{		
			CAutoLock ScopeLock ( m_ScopeCS, "doRawSqlMultiOneTranWork");
			//if(mIPSockObjMap.Lookup(sChkDupInfo.m_szIP, ( CObject*& ) pa ))
			
			CString strSockID ;
			strSockID.Format("%d", pSockObj->s) ;	
			if(mSockIdSockObjMap.Lookup(strSockID, ( CObject*& ) pa ))
			{
				CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
				
				if(pSpckObjMap  )
				{			
					// 전송데이터 모으기
					pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
					memset(pSzBuf, NULL, sizeof(pSzBuf));	
					memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
					memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sComRslt, iLength);	

					int nTotalLen = COMMON_HEADER_LENGTH + iLength;

					int nRtn = SendDataToClient("doRawSqlMultiOneTranWork", sChkDupInfo.m_szIP, pSpckObjMap, pSzBuf, nTotalLen);
					if(nRtn == -1 )
					{
						CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doRawSqlMultiOneTranWork SendDataToClient\n",
											__LINE__, sChkDupInfo.m_szIP );
					}
				}
			}	
		}		
	}

	return TRUE;
}

BOOL CThreadSockAsyncServer::doWork_CHK_ALREADY_LOGIN(SOCKET_OBJ* pSockObj)
{	
	S_COMMON_HEADER sMSGHeader;
	char szTemp[20];
	char* pSzBuf = NULL;
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	

	// 헤더부 설정		
	memcpy(sMSGHeader.m_szUsage, LOGIN_ALLOWED, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, 0 );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));

	CIpSockObjMapInfo*  paTmp2;
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_CHK_ALREADY_LOGIN");
		//if(mIPSockObjMap.Lookup(pSockObj->m_szIP, ( CObject*& ) paTmp2) ) // 20061204 
	
		CString strSockID ;
		strSockID.Format("%d", pSockObj->s) ;	

		if(mSockIdSockObjMap.Lookup( strSockID, ( CObject*& ) paTmp2) ) 
		{
			if(paTmp2 )
			{
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + 0]; //!!!
				memset(pSzBuf, NULL, sizeof(pSzBuf));	
				memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);					
				
				int nTotalLen = COMMON_HEADER_LENGTH ;
				
				int nRtn = SendDataToClient("doWork_CHK_ALREADY_LOGIN", pSockObj->m_szIP, paTmp2, pSzBuf, nTotalLen) ;
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_CHK_ALREADY_LOGIN SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}
		}
	}
	

	return TRUE;
}


// 대화상대 GRP 목록
BOOL CThreadSockAsyncServer::doWork_GetBuddyGrpList(SOCKET_OBJ* pSockObj)
{	
	char* pRawData= pSockObj->m_pBuff ;	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;		

	// mIDIPMap 에 추가!
	CIpSockObjMapInfo *pa= NULL;	
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_GetBuddyGrpList");

	CString strSockID ;
	strSockID.Format("%d", pSockObj->s) ;	

	//if(mIPSockObjMap.Lookup(pSockObj->m_szIP, ( CObject*& ) pa ))	// 20061204 	
	if(mSockIdSockObjMap.Lookup( strSockID, ( CObject*& ) pa ))	// 20061204 	
	{
		if(pa )
		{
			// ID 를 설정한다..
			memset(pa->m_pSockObj->m_szUserId, 0x00, sizeof(pa->m_pSockObj->m_szUserId));
			strncpy(pa->m_pSockObj->m_szUserId , pInfo->m_szFromID, sizeof(pa->m_pSockObj->m_szUserId));
						
			//mIPSockObjMap.SetAt(pSockObj->m_szIP, pa); // ip -> id 정보를 알수 있게 한다. (연결종료시 IP -> ID 구함) 20061204 
			
			mSockIdSockObjMap.SetAt(strSockID, pa); // ip -> id 정보를 알수 있게 한다. (연결종료시 IP -> ID 구함) 20061204 
			
			// ID로 사용자를 찾을수있게끔 MAP에 저장한다.
			// 한 객체를 2개의 맵에서 바라보게 됨..
			mIDIPMap.SetAt(pInfo->m_szFromID, pa); 
		}		
	}
	else
	{
		//Log		
		CLog::Instance().WriteFile(FALSE, "ServerLog",  "Ln [%d] [%s] doWork_GetBuddyGrpList mIPSockObjMap 에서IP로 못찾은 경우 FAIL\n", 
			__LINE__, pSockObj->m_szIP );
	}
		
	char szSql [200];
	memset(szSql, 0x00 , sizeof(szSql));
	
	sprintf(szSql, "SELECT /*+ INDEX(A PK_BUDDY_GRP) */ A.COMP_GROUP FROM BUDDY_GRP A	\
WHERE A.USER_ID ='%s' ORDER BY A.COMP_GROUP", pInfo->m_szFromID) ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_BUDDY_GRP, "", pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}

// 전체 목록 요청시 처리 
BOOL CThreadSockAsyncServer::doWork_ALL_USER_ID_TEAM(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	// 이미 조회해놓은 데이터 m_strAllUserList 를 가지고 온라인 여부를 조사해서 돌려줌.
	int nPos, nIndex = 0;
	char szTmpOut [256];
	char szTmpID [20+1];		
	string strSQLRslt="";	
	CString strCpy = ""; 
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALL_USER_ID_TEAM1");		
		
		strCpy = m_strAllUserList.c_str() ;		
	}

	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));			
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strCpy, szTmpOut, DELIM1);
		
		if(nPos != -1 && strlen(szTmpOut) > 0 )		
		{
			// USER_ID, USER_NAME, TEAM_GN, TEAM_NAME + CON_STATUS
			switch(nIndex % 4)
			{
			case 0:	// USER_ID
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1);
				memset(szTmpID, 0x00, sizeof(szTmpID));
				strncpy(szTmpID, szTmpOut, sizeof(szTmpID));
				break;
			case 1:	// USER_NAME
			case 2:	// TEAM_GN
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1);
				break;
			case 3:	// TEAM_NAME 
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1); 
				
				CIpSockObjMapInfo *pa = NULL;
				
				{				
					CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALL_USER_ID_TEAM2");
					if(mIDIPMap.Lookup(szTmpID, ( CObject*& ) pa ))
					{
						if(pa  )
						{
							//CON_STATUS
							strSQLRslt.append("Y")  ; 
							strSQLRslt.append(1, DELIM1);
						}
						else
						{
							strSQLRslt.append("N")  ;
							strSQLRslt.append(1, DELIM1); 
						}
					}
					else
					{												
						//CON_STATUS
						strSQLRslt.append("N")  ;
						strSQLRslt.append(1, DELIM1); 
					}

					//ToDo : Ip도 넘겨줘서 대화상대 추가후 바로 파일 전송시에 가능하게해야함
					//pa->m_pSockObj->m_szIP
				}
				
				break;
			}
			
			nIndex++;
		}
		
		if(nPos < 0)
		{
			break;
		}
	}	
	
	///////////////////////////////////////////////////////////

	S_COMMON_HEADER* pFromHeader = (S_COMMON_HEADER*)pRawData  ;

	char* pData = pRawData + COMMON_HEADER_LENGTH ;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	S_COMMON_HEADER sMSGHeader;
	
	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	
	
	memset(szTemp, 0x00, sizeof(szTemp));
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
			
	iLength = strSQLRslt.length() ;	// 동적으로 데이터 사이즈 변동 
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, /*ALL_USER_ID_TEAM*/ pFromHeader->m_szUsage , sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
	//연결된 클라이언트에서 전달대상 검색후 전송	=> ID로 검색!!	
	CIpSockObjMapInfo *pa = NULL;
		
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALL_USER_ID_TEAM3");
		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap )
			{			
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , strSQLRslt.c_str(), strSQLRslt.length()); 						
				nTotalLen = COMMON_HEADER_LENGTH + iLength; 
				
				int nRtn = SendDataToClient("doWork_ALL_USER_ID_TEAM",pInfo->m_szFromID,pSpckObjMap, pSzBuf, nTotalLen); 
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_ALL_USER_ID_TEAM SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
					
				}
			}			
		}
	}
		
	return TRUE;
}

BOOL CThreadSockAsyncServer::doWork_FOOD_MENU(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	TRACE("m_szFromID   [%s] [%s]\n", pInfo->m_szFromID, pSockObj->m_szIP);
				
	S_FOOD_MENU_PERIOD sSearchPeriod;
	memcpy(&sSearchPeriod, pData+FROMTOID_HEADER_LENGTH, sizeof(S_FOOD_MENU_PERIOD) );

	/* DB 에서 가져오는 방식인 경우...
	char szSql [300];
	memset(szSql, 0x00 , sizeof(szSql));

    sprintf(szSql,"SELECT / *+ INDEX(A PK_FOOD_MENU) * / FOOD_DAY, LD_FLAG, MENU,CAL FROM FOOD_MENU A WHERE FOOD_DAY BETWEEN '%s' AND '%s' ORDER BY FOOD_DAY, LD_FLAG",
		sSearchPeriod.m_szFromDate, sSearchPeriod.m_szToDate);
  			
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);
	pSqlWork->SetID( FOOD_MENU, "", pInfo->m_szFromID, pSockObj->m_szIP);	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		
	*/
	
	// 메모리에 있는 정보 전송
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));

	// S_COMMON_HEADER 이후 데이터 : S_SQL_RSLTDATA + 데이터 + MULL
	int nSqlDataLen = m_pDlg->m_strMenu.GetLength(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// 동적으로 데이터 사이즈 변동 
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, FOOD_MENU, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));	
	memcpy(sSqlRsltData.szRsltCd, "Y", strlen("Y") );	
	memcpy(sSqlRsltData.szErrMsg, "", strlen("") );				
	
	CIpSockObjMapInfo * pa = NULL;
	
	CIpSockObjMapInfo * pSpckObjMap = NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_FOOD_MENU");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)m_pDlg->m_strMenu, nSqlDataLen ); 
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
							
			int nRtn = SendDataToClient( (char*) FOOD_MENU,pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_FOOD_MENU SendDataToClient\n",
										__LINE__, pSpckObjMap->m_pSockObj->m_szIP );	
				
			}			
		}			
	}
		
	return TRUE;
}


BOOL CThreadSockAsyncServer::doWork_LOGIN(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	TRACE("m_szFromID   [%s] [%s]\n", pInfo->m_szFromID, pSockObj->m_szIP);   
	
	char szSql [1300];
	memset(szSql, 0x00 , sizeof(szSql));

	sprintf(szSql,"SELECT	DECODE(A.COMPANY_POS,'02','KT','NoKT') KT_GN,A.USER_NAME USER_NAME,A.PASSWD PASSWD,B.CODE TEAM_GN,B.CODE_NAME TEAM_NAME	\
,A.USER_GRADE USER_GRADE,D.CODE TOP_TEAM_GN,D.CODE_NAME TOP_TEAM_NAME,A.STATUS STATUS,A.NICK_NAME ,A.IAMS_AUTHORITY IAMS_AUTH, A.PROG_SELECT PROG_SELECT	\
FROM KM_DOCUSER A,KM_IDMS_CODE B,KM_CODE_REL C ,KM_IDMS_CODE D WHERE A.USER_ID='%s' AND A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	\
AND B.GRP_CODE=C.REL_GRP_CODE(+) AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+)", pInfo->m_szFromID);	
		
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);		
	//pSqlWork->SetID( LOGIN, "", pInfo->m_szFromID, pSockObj->m_szIP );	 
	
	CString strSockID ;
	strSockID.Format("%d", pSockObj->s) ;	
	pSqlWork->SetID( LOGIN, "", pInfo->m_szFromID, strSockID );	 
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		
	
	return TRUE;
}

// 대화상대 목록
BOOL CThreadSockAsyncServer::doWork_GetBuddyList(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	//TRACEX("m_szFromID   [%s] [%s]\n", pInfo->m_szFromID, pSockObj->m_szIP);   
	
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));

	// 정보친구 포함 NICK 에 이름, COMPID 에 주소정보
	sprintf(szSql, "SELECT X.KIND,X.NAME,X.NICK,X.COMPID,X.COMPGRP, X.IP FROM ( SELECT /*+ INDEX(A PK_BUDDYLIST) INDEX(B TX_DOCUSER_01)*/	\
A.KIND AS KIND,C.USER_NAME AS NAME,DECODE(C.NICK_NAME,'',' ',C.NICK_NAME) AS NICK,A.COMPANION_ID AS COMPID,A.COMP_GROUP AS COMPGRP, C.IP_ADDR AS IP	\
FROM BUDDY_LIST A,KM_DOCUSER B,KM_DOCUSER C WHERE A.USER_ID ='%s' AND B.USER_ID=A.USER_ID	\
AND C.USER_ID=A.COMPANION_ID UNION ALL SELECT /*+ INDEX(D PK_IT_BUDDYLIST) */	\
'9' AS KIND, D.IT_NAME AS NAME,D.IT_NAME AS NICK,D.IT_INFO AS COMPID,'인터넷 즐겨찾기' AS COMPGRP , '*' AS IP FROM IT_BUDDY_LIST D WHERE D.USER_ID ='%s'	\
) X	ORDER BY X.NAME", pInfo->m_szFromID, pInfo->m_szFromID) ;

	// '9' AS KIND, '*' AS NAME,D.IT_NAME AS NICK,D.IT_INFO AS COMPID,'인터넷 즐겨찾기' AS COMPGRP , '*' AS IP FROM IT_BUDDY_LIST D WHERE D.USER_ID ='%s'	
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);		

	pSqlWork->SetID( GET_BUDDY_LIST, "", pInfo->m_szFromID, pSockObj->m_szIP );	

	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		

	return TRUE;
}


BOOL CThreadSockAsyncServer::doWork_AddBuddyList(SOCKET_OBJ* pSockObj)
{	
//	KIND 친구 종류 
//	* 1 - 내가 등록한 상대
//	* 2 - 상대방이 나를 추가해서 만들어진 나의 대화상대에 추가됨
//	* 3 - 대화 거부
//	* 4 - 대화상대가 나를 삭제함
//	* 9 - 정보친구
//
//	BUDDY_LIST 에 저장, A 가 B 를 추가한경우 
//	USER_ID = 'A'                			
//  	COMPANION_ID = 'B'
//  	REG_DATE = 현재시간
//  	COMP_GROUP = 선택그룹
//  	KIND ='1'
//  		
//  	USER_ID = 'B'                			
//  	COMPANION_ID = 'A'
//  	REG_DATE = 현재시간
//  	COMP_GROUP = 기본그룹 
//  	KIND ='2'
//  		
//  	2 Row 가 저장된다..
 
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
		
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	S_ADD_BUDDY * pNewBuddyData = (S_ADD_BUDDY*) pData;
	
	char szSql [300];	
	memset(szSql, 0x00 , sizeof(szSql));
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	int nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::itoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!
	
	oresult nOsessRslt = osess.Open();

	ODatabase odb;
	char szErrMsg [100];
	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		goto EXIT_HOLE;
	}
	
	memset(szErrMsg, 0x00, sizeof(szErrMsg));

	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!");
		osess.Close();
		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{								
		osess.Close();	
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 
		odb.Close();	
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	// 내 대화상대 목록에 추가	
	sprintf(szSql, "INSERT INTO BUDDY_LIST(USER_ID, COMPANION_ID, REG_DATE,COMP_GROUP,KIND)	VALUES('%s',(SELECT /*+ INDEX(A TX_DOCUSER_01)*/ A.USER_ID FROM KM_DOCUSER A WHERE A.USER_ID ='%s'),SYSDATE,'%s','1')", 
		pNewBuddyData->m_szUserID , pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szBuddyGrp) ;

	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("★ ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		odb.Close();
		osess.Close();
	
		bRslt = FALSE;
		goto EXIT_HOLE;
	}
		
	
	//2 번째 DB 작업 -> 상대방의 대화목록에도 나를 추가한다. 기본그룹에 추가됨
	memset(szSql, 0x00 , sizeof(szSql));	
	sprintf(szSql, "INSERT INTO BUDDY_LIST(USER_ID, COMPANION_ID, REG_DATE,COMP_GROUP,KIND)	\
VALUES((SELECT /*+ INDEX(A TX_DOCUSER_01)*/ A.USER_ID FROM KM_DOCUSER A WHERE A.USER_ID ='%s'),'%s',SYSDATE,'%s','2')", 
		pNewBuddyData->m_szBuddyID , pNewBuddyData->m_szUserID , "기본그룹" /*pNewBuddyData->m_szBuddyGrp*/) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("★ ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		odb.Close();
		osess.Close();	

		bRslt = FALSE;
		goto EXIT_HOLE;
	}
	
	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();
	
EXIT_HOLE:

	OShutdown();

	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int iLength = sizeof(S_COM_RESPONSE);		
		
	// 데이터부 설정
	if(bRslt == TRUE)
	{
		//20061125 저장하고 ONLINE처리시 는 exception처리 
		
		// 대화 상대 목록을 갱신 (메모리)
		MAP_STR2VEC::iterator it;
		
		
		it = m_mapStr2VecCompanion.find( pNewBuddyData->m_szUserID );
		if(it != m_mapStr2VecCompanion.end() )  
		{		
			((*it).second).push_back( pNewBuddyData->m_szBuddyID ) ;
		}
		else
		{
			// vector 가 없는 경우 			
			VEC_STR vTmp;							
			vTmp.reserve(50);
			vTmp.push_back( pNewBuddyData->m_szBuddyID ) ;
			m_mapStr2VecCompanion.insert( MAP_STR2VEC::value_type( pNewBuddyData->m_szUserID, vTmp ) );			

		}

		it = m_mapStr2VecCompanion.find(pNewBuddyData->m_szBuddyID );
		if(it != m_mapStr2VecCompanion.end() )  
		{		
			((*it).second).push_back( pNewBuddyData->m_szUserID ) ;	
		}
		else
		{
			// vector 가 없는 경우 

			VEC_STR vTmp;	
			vTmp.reserve(50);
			vTmp.push_back( pNewBuddyData->m_szUserID ) ;
			m_mapStr2VecCompanion.insert( MAP_STR2VEC::value_type( pNewBuddyData->m_szBuddyID, vTmp ) );
		}
			
		// 추가한 대화상대의 정보를 조회한다..		
		//A가 B를 추가한 경우 먼저 B정보를 구해서 A로 전송하고 ,
		//A정보를 구해서 B로 전송한다.			

		char szSql [300];
		memset(szSql, 0x00 , sizeof(szSql));

		//1. A에게 전송할 B 정보를 구한다.
		sprintf(szSql, "SELECT /*+  INDEX(A TX_DOCUSER_01)*/ A.USER_ID, A.USER_NAME, A.NICK_NAME, A.IP_ADDR,B.COMP_GROUP, '1'	\
	FROM KM_DOCUSER A, BUDDY_LIST B WHERE A.USER_ID ='%s' AND B.USER_ID = '%s' AND  B.COMPANION_ID = A.USER_ID ", 
		pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID ) ;

		COracleWorkerMT * pSqlWork1 = CMemoryPool::New_COracleWorkerMT();	
		pSqlWork1->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

		pSqlWork1->SetSQL( szSql ); 
		// 처리결과가 전송될때 SetID 의 3번째에 지정한  ID 로 전송되는것을 이용한다.
		pSqlWork1->SetID( RAWSQL_GETBUDDY_INFO, pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID, pNewBuddyData->m_szUserID);

		pSqlWork1->setNotiProcThread(SQLNotifyProc , this); 
		global::CThreadPool::instance().queue_request( pSqlWork1 );		

		//2. B에게 전송할  A 정보를 구한다.			
		memset(szSql, 0x00 , sizeof(szSql));
		sprintf(szSql, "SELECT /*+  INDEX(A TX_DOCUSER_01)*/ A.USER_ID, A.USER_NAME, A.NICK_NAME, A.IP_ADDR,B.COMP_GROUP, '2'	\
	FROM KM_DOCUSER A, BUDDY_LIST B WHERE A.USER_ID ='%s' AND B.USER_ID = '%s' AND  B.COMPANION_ID = A.USER_ID ", 
		pNewBuddyData->m_szUserID, pNewBuddyData->m_szBuddyID ) ;			
						
		COracleWorkerMT * pSqlWork2 = CMemoryPool::New_COracleWorkerMT();	
		pSqlWork2->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd );	
		pSqlWork2->SetSQL( szSql ); 
		// 처리결과가 전송될때 SetID 의 3번째에 지정한  ID 로 전송되는것을 이용한다. => SOMEONE_ADD_ME !!
		pSqlWork2->SetID( SOMEONE_ADD_ME, pNewBuddyData->m_szUserID, pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID);
		pSqlWork2->setNotiProcThread(SQLNotifyProc , this); 
		global::CThreadPool::instance().queue_request( pSqlWork2 );			
	}
	else
	{
		// 추가 실패 사유를 전송한다.
		S_COM_RESPONSE sComRslt;
		memset(&sComRslt, 0x00, sizeof(sComRslt));
		memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
		memcpy(sComRslt.szErrMsg, szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(szErrMsg)));
		
		// 헤더부 설정		
		memcpy(sMSGHeader.m_szUsage, ADD_BUDDY_LIST_ERR, sizeof(sMSGHeader.m_szUsage));		
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
		memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
		memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
		//연결된 클라이언트에서 전달대상 검색후 전송			
		
		CIpSockObjMapInfo *pa = NULL;
				
		{
			CAutoLock ScopeLock ( m_ScopeCS, "doWork_AddBuddyList");

			if(mIDIPMap.Lookup(pNewBuddyData->m_szUserID, ( CObject*& ) pa ))	
			{
				CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
				
				if(pSpckObjMap )
				{			
					// 전송데이터 모으기
					char* pSzBuf = NULL;
					pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
					memset(pSzBuf, NULL, sizeof(pSzBuf));	
					memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
					memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sComRslt, iLength);	
					
					int nTotalLen = COMMON_HEADER_LENGTH + iLength;
					
					int nRtn= SendDataToClient("doWork_AddBuddyList", pNewBuddyData->m_szUserID,pSpckObjMap, pSzBuf, nTotalLen);
					if(nRtn == -1 )
					{
						CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_AddBuddyList SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
					}
				}
			}
		}		
	}	
	
	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL CThreadSockAsyncServer::doWork_AllowBuddy(SOCKET_OBJ* pSockObj)
{	
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	S_ADD_BUDDY * pNewBuddyData = (S_ADD_BUDDY*) pData;
	
	char szSql [300];	
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	//TRACE("szSessionNameIndex [%s]\n", szSessionNameIndex);

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	oresult nOsessRslt = osess.Open();
	
	ODatabase odb;
	char szErrMsg [100];
	memset(szErrMsg, 0x00, sizeof(szErrMsg));

	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{								
		osess.Close();	
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	//  KIND : 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함)
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '1',COMP_GROUP='%s' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND ='3' ", 
		pNewBuddyData->m_szBuddyGrp, pNewBuddyData->m_szUserID , pNewBuddyData->m_szBuddyID ) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("★ ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		
		odb.Close();
		osess.Close();	

		bRslt = FALSE;
		goto EXIT_HOLE;
	}
	//pSqlWork1->SetID( ID_MEANINGLESS, "", "", ""); 	
	
	//----------------------------------------------------------
	// 5 인경우는 해당데이터 존재시만 처리됨
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '2' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND = '5' ", 
		pNewBuddyData->m_szBuddyID , pNewBuddyData->m_szUserID ) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("★ ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		odb.Close();
		osess.Close();	

		bRslt = FALSE;
		goto EXIT_HOLE;
	}

	
	//----------------------------------------------------------
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '2' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND = '4' ", 
		pNewBuddyData->m_szBuddyID ,  pNewBuddyData->m_szUserID ) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("★ ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		odb.Close();
		osess.Close();	

		bRslt = FALSE;
		goto EXIT_HOLE;
	}
	
	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();

EXIT_HOLE:

	OShutdown();

	//연결된 클라이언트에서 전달대상 검색후 전송		
	CIpSockObjMapInfo *pa = NULL;

	// A가 B를 다시 추가한 경우 A 에게 성공여부 전송
		
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllowBuddy1");

		if(mIDIPMap.Lookup(pNewBuddyData->m_szUserID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap  )
			{
				S_COMMON_HEADER sMSGHeader;
				char szTemp [20];
				memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
				memset(szTemp, 0x00, sizeof(szTemp));
				char* pSzBuf = NULL;

				int iLength = SQL_RSLTDATA_LENGTH + sizeof(pNewBuddyData->m_szBuddyID) + sizeof(S_YOU_ALLOWED) ; //ID GRPNAME
				// 헤더부 설정			
				memcpy(sMSGHeader.m_szUsage, YOUALLOWEDASCOMP, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
				// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
				S_SQL_RSLTDATA sSqlRsltData;
				memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
				
				if(bRslt == TRUE)
				{
					memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
				}
				else
				{
					memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
				}
				memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				S_YOU_ALLOWED sDataAllowed;
				memset(&sDataAllowed , NULL, sizeof(S_YOU_ALLOWED));
				memcpy(sDataAllowed.m_szYouAllowedID, pNewBuddyData->m_szBuddyID, sizeof(sDataAllowed.m_szYouAllowedID));
				memcpy(sDataAllowed.m_szGrpName, pNewBuddyData->m_szBuddyGrp, sizeof(sDataAllowed.m_szGrpName));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA
				memcpy(pSzBuf + COMMON_HEADER_LENGTH+SQL_RSLTDATA_LENGTH ,&sDataAllowed, sizeof(S_YOU_ALLOWED)); 
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
				
				int nRtn = SendDataToClient("doWork_AllowBuddy",pNewBuddyData->m_szUserID,pSpckObjMap, pSzBuf, nTotalLen);
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_AllowBuddy SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}			
		}
	}
			
	// A가 B를 다시 추가한 경우 B 에게 A가대화상대 삭제 해제 했음을 전송	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllowBuddy2");
		
		if(mIDIPMap.Lookup(pNewBuddyData->m_szBuddyID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap  )
			{	
				S_COMMON_HEADER sMSGHeader;
				char szTemp [20];
				memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
				memset(szTemp, 0x00, sizeof(szTemp));
				char* pSzBuf = NULL;

				int iLength = sizeof(pNewBuddyData->m_szUserID )  ; // A
				// 헤더부 설정			
				memcpy(sMSGHeader.m_szUsage, YOUALLOWEDASCOMP_INFORM, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));				
				
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , pNewBuddyData->m_szUserID, iLength); 
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; 
				
				int nRtn = SendDataToClient("doWork_AllowBuddy",pNewBuddyData->m_szBuddyID,pSpckObjMap, pSzBuf, nTotalLen);
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_AllowBuddy SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}			
		}
	}
	

	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL CThreadSockAsyncServer::doWork_DelBuddy( SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
//	KIND : 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함)
//	A가 B를 삭제하는경우
//	B.nBockMode = 1,2 인경우 => 1. A가 서버로 요청해서 BUDDY_LIST에서 A.KIND = '3', B.KINE = '4' UPDATE !
//									2. 서버는 A로 메시지보내서 A 트리에서 B삭제, MAP A.nBockMode = 3 변경.
//									3. 서버는 B로 메시지보내서 B 트리에서 A차단상태 표시, MAP A.nBockMode = 4 변경.
//
//	B.nBockMode = 4 인경우	=>	KIND 5 UPDATE!	
//									(B가 A를 다시 추가시에는 추가불가 처리를 Client에서 한다. 대화상대정보 DB가져올수있게 지우지 않는다)		
//
//	A가 B를 다시 추가		=>	서버로 요청해서 BUDDY_LIST에서 A.KIND = '1', B.KINE = '2' UPDATE !
	 		
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);
	char* pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	char szTmpOut[256];
	char szData[1024];
	memset(szTmpOut , 0x00, sizeof(szTmpOut));
	memset(szData , 0x00, sizeof(szData));

	//CString strSrc, strData;	
	CString strSrc;	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	strSrc = szData;	
	
	int nPos =  0;
		
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	oresult nOsessRslt = osess.Open();

	ODatabase odb;
	char szErrMsg [100];
	memset(szErrMsg, 0x00, sizeof(szErrMsg));
	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
			
	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szTmpOut, DELIM1);
		
		if(nPos != -1 && strlen(szTmpOut) > 0 )		
		{
			//Chain!
			char szSql [200];

			//Delete => 이것은 해당사항이 있을때만 삭제될것임! => 삭제하지않음. 			
//			memset(szSql, 0x00 , sizeof(szSql));
//			sprintf(szSql, "DELETE FROM BUDDY_LIST WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND ='4' ", (LPCSTR)strTmpOut , pInfo->m_szFromID ) ;
//			pSqlWork1 = new COracleWorkerMTTran( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
//			pSqlWork1->SetSQL( szSql ); 
//			pSqlWork1->SetID( ID_MEANINGLESS, "", (LPCSTR)strTmpOut,  ""); 
//			pSqlWork1->SetUsefulInfo( pInfo->m_szFromID ); 
//			pSqlWork1->setNotiProcThread(SQLNotifyProcTran , this); 
//
//			chain.SetWork(pSqlWork1);
			
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '5' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND ='4' ", pInfo->m_szFromID, szTmpOut ) ;
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();		

				bRslt = FALSE;
				goto EXIT_HOLE;
			}
			
			
			// 삭제당한 사람의 BUDDY_LIST.KIND = 4 (상대방에 의해서 차단당함) 변경!			
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '4' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND IN ('1','2') ", szTmpOut , pInfo->m_szFromID ) ;
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();
			
				bRslt = FALSE;
				goto EXIT_HOLE;
			}
			
			// 내목록에서 BUDDY_LIST.KIND = 3 (내가 삭제함) 변경!
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '3' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND IN ('1','2') ", pInfo->m_szFromID , szTmpOut) ;
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();
			
				bRslt = FALSE;
				goto EXIT_HOLE;
			}
		}
		
		if(nPos < 0)
		{			
			break;
		}
	}

	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();

EXIT_HOLE:

	OShutdown();

	// 1. A에게 B,C 삭제 됬음을 알림!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, DEL_BUDDY, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));			
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송	
	CIpSockObjMapInfo *pa = NULL;	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddy1");

		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
			
			if(pSpckObjMap  )
			{			
				// 전송데이터 모으기
				char* pSzBuf = NULL;
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
				
				int nRtn = SendDataToClient("doWork_DelBuddy",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen);

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_DelBuddy SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}			
		}
	}
	
	
	// 2. 삭제성공일때만 ( bRslt == TRUE) B,C 들에게 A가 삭제했음을 알림 
	
	if(bRslt == TRUE)
	{		
		nStrLen = nLen - FROMTOID_HEADER_LENGTH;
		strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
		strSrc = szData;	
		
		while( 1 )
		{
			memset(szTmpOut, 0x00, sizeof(szTmpOut));
			nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szTmpOut, DELIM1);
			
			if(nPos != -1 && strlen(szTmpOut) > 0 )		
			{
				S_COMMON_HEADER sMSGHeader;
				char szTemp [20];
				memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
				memset(szTemp, 0x00, sizeof(szTemp));

				int nSqlDataLen = strlen( pInfo->m_szFromID ); 
				int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

				// 헤더부 설정			
				memcpy(sMSGHeader.m_szUsage, SOMEONE_DEL_ME, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
				// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
				S_SQL_RSLTDATA sSqlRsltData;
				memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));				
				memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
				memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
							
				//연결된 클라이언트에서 전달대상 검색후 전송	
				CIpSockObjMapInfo *pa= NULL;
								
				{
					CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddy2");
				
					if(mIDIPMap.Lookup(szTmpOut, ( CObject*& ) pa ))
					{
						CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
						
						if(pSpckObjMap )
						{			
							// 전송데이터 모으기
							char* pSzBuf = NULL;
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
							memset(pSzBuf, NULL, sizeof(pSzBuf));
							
							memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, pInfo->m_szFromID,  strlen( pInfo->m_szFromID ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
							
							int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
							
							int nRtn = SendDataToClient("doWork_DelBuddy",szTmpOut, pSpckObjMap, pSzBuf, nTotalLen);
							if(nRtn == -1 )
							{
								CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_DelBuddy SendDataToClient\n", 
									__LINE__, pSockObj->m_szIP );
							}
						}			
					}
				}
			}

			if(nPos < 0)
			{			
				break;
			}
		}
	}	

	return TRUE;
}


BOOL CThreadSockAsyncServer::doWork_AddBuddyGrp(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_ADD_BUDDY_GRP * pBuddyGrp = (S_ADD_BUDDY_GRP*) pData;

	char szSql [200];	
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "INSERT INTO BUDDY_GRP(USER_ID,COMP_GROUP) VALUES('%s','%s')", pBuddyGrp->m_szUserID , pBuddyGrp->m_szBuddyGrp) ;
	
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	
	pSqlWork->SetID( ADD_BUDDY_GRP, "", pBuddyGrp->m_szUserID, "");	//추가된 그룹 정보를 보내야함!!
	pSqlWork->SetUsefulInfo(pBuddyGrp->m_szBuddyGrp);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL CThreadSockAsyncServer::doWork_ChgGrpName(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = 	atoi(psHeader->m_szLength);

	char* pData = pRawData + COMMON_HEADER_LENGTH;
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	char szData[1024];
	memset(szData , 0x00, sizeof(szData));	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	CString strSrc (szData);	

	char szBefGrpName [C_GRP_NAME_LEN+1];
	char szAftGrpName [C_GRP_NAME_LEN+1];
	int nPos = 0;

	OStartup(OSTARTUP_MULTITHREADED);	
	char szSessionNameIndex[10];

	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));
	memset(szBefGrpName, NULL, sizeof(szBefGrpName));
	memset(szAftGrpName, NULL, sizeof(szAftGrpName));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	
	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	oresult nOsessRslt = osess.Open();

	ODatabase odb;
	char szErrMsg [100];
	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	// Bef 그룹명
	memset(szBefGrpName, 0x00, sizeof(szBefGrpName));
	nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szBefGrpName, DELIM1);

	while( 1 )
	{
		memset(szAftGrpName, 0x00, sizeof(szAftGrpName));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szAftGrpName, DELIM1);
		
		if(nPos != -1 && strlen(szAftGrpName) > 0 )
		{
			//Chain!
			char szSql [200];	
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET COMP_GROUP = '%s' WHERE USER_ID ='%s' AND COMP_GROUP = '%s'", 
				szAftGrpName, pInfo->m_szFromID , szBefGrpName) ;
			
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();			

				bRslt = FALSE;
				goto EXIT_HOLE;
			}

			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_GRP SET COMP_GROUP = '%s' WHERE USER_ID ='%s' AND COMP_GROUP = '%s'", 
				szAftGrpName, pInfo->m_szFromID , szBefGrpName) ;

			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();			

				bRslt = FALSE;
				goto EXIT_HOLE;
			}
		}
		
		if(nPos < 0)
		{
			break;
		}		
	}	

	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();

EXIT_HOLE:

	OShutdown();

	// 처리 결과 전송!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, CHG_GRP_NAME, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));			
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송
	
	CIpSockObjMapInfo *pa= NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ChgGrpName");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap )
		{			
			// 전송데이터 모으기
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient("doWork_ChgGrpName",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen);
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_ChgGrpName SendDataToClient\n",
										__LINE__, pSockObj->m_szIP );
			}
		}			
	}
		
	return TRUE;
}


// 대화상대 자리비움
BOOL CThreadSockAsyncServer::doWork_STATUS_AWAY(SOCKET_OBJ* pSockObj)	
{	
	char* pRawData= pSockObj->m_pBuff ;	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	CIpSockObjMapInfo * pSpckObjMap = NULL;		
	
	MAP_STR2VEC::iterator it;
	VEC_STR::iterator itV;
	
	it = m_mapStr2VecCompanion.find(pInfo->m_szFromID);
	
	if(it != m_mapStr2VecCompanion.end() )  
	{		
		strcpy(pSockObj->m_szAwayInfo,"1");
		
		char szTmp[20+1];
		S_AWAY_INFO sAwayInfo;	
		
		for (itV=((*it).second).begin(); itV != ((*it).second).end(); ++itV) //20061121
		{		
			//TRACE("%s\n", (*itV).c_str() );	
			pSpckObjMap = NULL;		
			
			memset(szTmp, 0x00, sizeof(szTmp));
			strncpy(szTmp, (*itV).c_str(), sizeof(szTmp));	
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_STATUS_AWAY");

				if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
				{
					if(pSpckObjMap  && (DWORD)pSpckObjMap != 0xfeeefeee )
					{					
						S_COMMON_HEADER sMSGHeader;
						
						int iLength = 0;	
						int nTotalLen = 0;	
						char* pSzBuf = NULL;								
						char szTemp[20];	
						
						memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
						memset(&sAwayInfo , 0x00, sizeof(sAwayInfo));				
						memcpy(sAwayInfo.szIDFrom , pInfo->m_szFromID, sizeof(sAwayInfo.szIDFrom));
						
						iLength = sizeof(S_AWAY_INFO);	
						
						// 헤더부 설정			
						memcpy(sMSGHeader.m_szUsage, STATUS_AWAY, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// 전송데이터 모으기
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //공통 헤더 + sAwayInfo
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						int nRtn = 0;

						nRtn = SendDataToClient("doWork_STATUS_AWAY", (char*)((*itV).c_str()), pSpckObjMap, pSzBuf, nTotalLen) ;
						
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] => [%s] doWork_STATUS_AWAY SendDataToClient\n",
											__LINE__, pInfo->m_szFromID, (*itV).c_str() );
							//break;
						}
					}
				}
			}
		}
	}
    

	return TRUE;
}

/// 대화상대 자리비움 해제
BOOL CThreadSockAsyncServer::doWork_STATUS_ONLINE(SOCKET_OBJ* pSockObj)	
{	
	char* pRawData= pSockObj->m_pBuff ;
	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;		
	CIpSockObjMapInfo *pSpckObjMap = NULL;		
	MAP_STR2VEC::iterator it;
	VEC_STR::iterator itV;
	
	char szTmp[20+1];
		
    it = m_mapStr2VecCompanion.find(pInfo->m_szFromID);
	if(it != m_mapStr2VecCompanion.end() )  
	{		
		strcpy( pSockObj->m_szAwayInfo,"0");						
		
		S_AWAY_INFO sAwayInfo;	
		
		for (itV=((*it).second).begin(); itV != ((*it).second).end(); ++itV) //20061121
		{		
			//TRACE("%s\n", (*itV).c_str() );	
			memset(szTmp, 0x00, sizeof(szTmp));
			strncpy(szTmp, (*itV).c_str(), sizeof(szTmp));				
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_STATUS_ONLINE");
			
				if(mIDIPMap.Lookup( szTmp /*(*itV).c_str() */ , ( CObject*& ) pSpckObjMap ))
				{
					if(pSpckObjMap  && (DWORD)pSpckObjMap != 0xfeeefeee )
					{						
						S_COMMON_HEADER sMSGHeader;
						int iLength = 0;	
						int nTotalLen = 0;	
						char* pSzBuf = NULL;								
						char szTemp[20];	
						
						memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
						memset(&sAwayInfo , 0x00, sizeof(sAwayInfo));				
						memcpy(sAwayInfo.szIDFrom , pInfo->m_szFromID, sizeof(sAwayInfo.szIDFrom));
						
						iLength = sizeof(S_AWAY_INFO);	
						
						// 헤더부 설정			
						memcpy(sMSGHeader.m_szUsage, STATUS_ONLINE, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// 전송데이터 모으기
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //공통 헤더 + sAwayInfo
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						int nRtn = 0;
						nRtn = SendDataToClient("doWork_STATUS_ONLINE",szTmp,pSpckObjMap, pSzBuf, nTotalLen) ;
						
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s]=>[%s] doWork_STATUS_ONLINE SendDataToClient\n",
								__LINE__, pInfo->m_szFromID, (*itV).c_str() );
													
							//break;
						}
					}
				}
			}			
		}
	}
		
	return TRUE;
}

BOOL CThreadSockAsyncServer::doWork_ALIVE(SOCKET_OBJ* pSockObj)	
{	
	// 받은 즉시 응답을 전송한다.
	char* pRawData= pSockObj->m_pBuff ;	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;			
	S_COMMON_HEADER sMSGHeader;

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;								
	char szTemp[20];	
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	
	iLength = 0;	// 그냥 헤더만 ... 
	
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	CIpSockObjMapInfo *pSpckObjMap = NULL;	
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALIVE");

	if(mIDIPMap.Lookup(pInfo->m_szFromID , ( CObject*& ) pSpckObjMap ))
	{
		if(pSpckObjMap  && (DWORD)pSpckObjMap != 0xfeeefeee )
		{					
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; 
			
			int nRtn = 0;
			
			nRtn = SendDataToClient("doWork_ALIVE",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen) ;
			
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_ALIVE SendDataToClient\n",
										__LINE__, pInfo->m_szFromID );
				
				//return FALSE;
			}	
		}
	}	
		
	return TRUE;
}



// 대화상대 다른용무중
BOOL CThreadSockAsyncServer::doWork_STATUS_BUSY(SOCKET_OBJ* pSockObj)	
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	CIpSockObjMapInfo *pSpckObjMap = NULL;		
	
	MAP_STR2VEC::iterator it;
	VEC_STR::iterator itV;
	
	it = m_mapStr2VecCompanion.find(pInfo->m_szFromID);
	
	if(it != m_mapStr2VecCompanion.end() )  
	{				
		strcpy(pSockObj->m_szAwayInfo,"2");			
		
		char szTmp[20+1];
		S_AWAY_INFO sAwayInfo;	
		
		for (itV=((*it).second).begin(); itV != ((*it).second).end(); ++itV)  //20061121
		{		
			//TRACE("%s\n", (*itV).c_str() );	
			
			memset(szTmp, 0x00, sizeof(szTmp));
			strncpy(szTmp, (*itV).c_str(), sizeof(szTmp));	
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_STATUS_BUSY");

				if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
				{
					if(pSpckObjMap  && (DWORD)pSpckObjMap != 0xfeeefeee )
					{					
						S_COMMON_HEADER sMSGHeader;
						int iLength = 0;	
						int nTotalLen = 0;	
						char* pSzBuf = NULL;								
						char szTemp[20];	
						
						memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
						memset(&sAwayInfo , 0x00, sizeof(sAwayInfo));				
						memcpy(sAwayInfo.szIDFrom , pInfo->m_szFromID, sizeof(sAwayInfo.szIDFrom));
						
						iLength = sizeof(S_AWAY_INFO);	
						
						// 헤더부 설정			
						memcpy(sMSGHeader.m_szUsage, STATUS_BUSY, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// 전송데이터 모으기
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //공통 헤더 + sAwayInfo
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						int nRtn = 0;
						nRtn = SendDataToClient("doWork_STATUS_BUSY",(char*)((*itV).c_str()), pSpckObjMap, pSzBuf, nTotalLen) ;
						
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] => [%s] doWork_STATUS_BUSY SendDataToClient\n",
											__LINE__, pInfo->m_szFromID, (*itV).c_str() );
							//break;
						}
					}
				}
			}
		}
	}
	
	return TRUE;
}

//로그인 아이디 조회
BOOL CThreadSockAsyncServer::doWork_SearchID(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	char *pData = pRawData + COMMON_HEADER_LENGTH ;
	S_SEARCH_ID * pSearchID = (S_SEARCH_ID*) pData;
		
	char szSql [1300];
	memset(szSql, 0x00 , sizeof(szSql));

	sprintf(szSql, "SELECT /*+ ORDERED USE_INDEX( A TX_DOCUSER_01 ) */ A.USER_NAME USER_NAME,	\
A.USER_ID USER_ID,F.CODE_NAME COMPANY,D.CODE_NAME TOP_TEAM_NAME,B.CODE_NAME TEAM_NAME,	\
DECODE(HP_TEL_NO,'','',SUBSTR(A.HP_TEL_NO ,1,4) || '-' || SUBSTR(A.HP_TEL_NO ,5,4) || '-' ||	\
SUBSTR(A.HP_TEL_NO ,9,4)) TEL1,DECODE(OFF_TEL_NO,'','',SUBSTR(A.OFF_TEL_NO,1,4) || '-' ||	\
SUBSTR(A.OFF_TEL_NO,5,4) || '-' ||SUBSTR(A.OFF_TEL_NO,9,4) ||DECODE(A.OFF_IN_NO,'','',	\
'('||A.OFF_IN_NO||')')) TEL2,E.CODE_NAME JOB_CLASS_NAME,F.CODE COMPANYU_CODE,D.CODE TOP_TEAM_CODE,	\
B.CODE TEAM_CODE,E.CODE JOB_CLASS_CODE FROM KM_DOCUSER A,KM_IDMS_CODE B,KM_CODE_REL C,	\
KM_IDMS_CODE D,KM_IDMS_CODE E,KM_IDMS_CODE F WHERE A.USER_NAME LIKE '%s%%' AND A.USER_ID LIKE '%s%%'	\
AND B.GRP_CODE(+)='1001' AND B.CODE(+)=A.TEAM_GN AND C.GRP_CODE(+)='6008' AND C.REL_GRP_CODE(+)='1001'	\
AND C.REL_CODE(+)=A.TEAM_GN AND D.GRP_CODE(+)='6008' AND D.CODE(+)=C.CODE AND E.GRP_CODE(+)='6004'	\
AND E.CODE(+)=A.JOB_CLASS AND F.GRP_CODE(+)='6013' AND F.CODE(+)=A.COMPANY_POS AND A.STATUS!='02' "	\
,pSearchID->szName, pSearchID->szID );	
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( SEARCH_ID, "", pSearchID->szIDFrom, "");
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}


// 대화명 변경
BOOL CThreadSockAsyncServer::doWork_ChngNic(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_CHG_NICK * pChngNic = (S_CHG_NICK*) pData;
	//TRACE("szIDFrom [%s] / BefNick [%s] / AftNick[%s]\n", pChngNic->szIDFrom, pChngNic->szNickBef, pChngNic->szNickAft);   
	
	char szSql [300];
		
	// 1. 먼저 KM_DOCUSER 에서 변경	
	memset(szSql, 0x00 , sizeof(szSql));	
	sprintf(szSql, "UPDATE KM_DOCUSER SET NICK_NAME ='%s' WHERE USER_ID ='%s'", pChngNic->szNickAft, pChngNic->szIDFrom) ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( CHNG_NICK, "", pChngNic->szIDFrom, "");	
	pSqlWork->SetUsefulInfo( pChngNic->szNickAft);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		

	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 대화상대 그룹 변경
BOOL CThreadSockAsyncServer::doWork_ChgBuddyGrp(SOCKET_OBJ* pSockObj)
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);

	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;

	int nPos;
	char szTmpOut [256];	
	char szData[1024];
	memset(szData , 0x00, sizeof(szData));	
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	CString strSrc (szData);

	char szChgGrpName [C_GRP_NAME_LEN+1];
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	//TRACEX("szSessionNameIndex [%s]\n", szSessionNameIndex);

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	ODatabase odb;
	char szErrMsg [100];
	BOOL bRslt = FALSE;

	oresult nOsessRslt = osess.Open();

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}	

	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction Error!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	// 변경할 그룹명
	memset(szChgGrpName, 0x00, sizeof(szChgGrpName));
	nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szChgGrpName, DELIM1);

	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szTmpOut, DELIM1);
		
		if(nPos != -1 )
		{
			//Chain!
			char szSql [200];	
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET COMP_GROUP = '%s' WHERE USER_ID ='%s' AND COMPANION_ID = '%s'", 
				szChgGrpName, pInfo->m_szFromID , szTmpOut) ;
									
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();
			
				bRslt = FALSE;
				goto EXIT_HOLE;
			}			
		}
		
		if(nPos < 0)
		{		
			break;
		}
	}
				
	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();

EXIT_HOLE:

	OShutdown();

	// 처리 결과 전송!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, CHG_BUDDY_GRP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));			
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송
	
	CIpSockObjMapInfo *pa= NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ChgBuddyGrp");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn =SendDataToClient("doWork_ChgBuddyGrp",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen);
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_ChgBuddyGrp SendDataToClient\n",
										__LINE__, pSockObj->m_szIP );
			}
		}			
	}
	
	return TRUE;
}



//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL CThreadSockAsyncServer::doWork_DelBuddyGrp( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	int nLen = atoi(psHeader->m_szLength);
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	char szTmpOut [256];
	char szData[1024];
	memset(szData , 0x00, sizeof(szData));	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	CString strSrc (szData);	
	
	int nPos;
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	//TRACEX("szSessionNameIndex [%s]\n", szSessionNameIndex);

	OSession osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!

	oresult nOsessRslt = osess.Open();

	ODatabase odb;
	char szErrMsg [100];
	BOOL bRslt = FALSE;

	if( OSUCCESS != nOsessRslt)
	{
		CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] osess.Open() Error!! ErrNum [%d] \n", __LINE__, osess.ErrorNumber() );
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}
	
	if( OFAILURE  == osess.BeginTransaction() )
	{
		TRACE("BeginTransaction ERROR!!\n");
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	if ( ! osess.IsOpen() )
	{		
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		

		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	{
		oresult nDbRslt = odb.Open(osess, (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);

		if( OSUCCESS != nDbRslt)
		{
			CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] odb.Open() Error!! ErrNum [%d] \n", __LINE__, odb.ErrorNumber() );
			bRslt = FALSE;
			osess.Close();
			odb.Close();		
			goto EXIT_HOLE;
		}
	}
	
	if ( ! odb.IsOpen() )
	{ 	
		sprintf(szErrMsg , "[%d] %s", osess.ServerErrorNumber(), osess.GetServerErrorText());		
		bRslt = FALSE;
		osess.Close();
		odb.Close();		
		goto EXIT_HOLE;
	}

	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strSrc, szTmpOut, DELIM1);
		
		if(nPos != -1 )
		{	
			char szSql [100];	
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "DELETE FROM BUDDY_GRP WHERE USER_ID ='%s' AND COMP_GROUP = '%s'", pInfo->m_szFromID , szTmpOut) ;

			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("★ ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();
			
				bRslt = FALSE;
				goto EXIT_HOLE;
			}
		}
		
		if(nPos < 0)
		{
			break;
		}
	}
				
	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();

EXIT_HOLE:

	OShutdown();

	// 처리 결과 전송!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, DEL_BUDDY_GRP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));			
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송
	
	CIpSockObjMapInfo *pa= NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddyGrp");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient("doWork_DelBuddyGrp",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen);
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_DelBuddyGrp SendDataToClient\n",
										__LINE__, pSockObj->m_szIP );
			}
		}			
	}
	

	return TRUE;
}



// 파일전송을 원함
BOOL CThreadSockAsyncServer::doWork_SomeoneWantsSendFiles( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;
	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;	

	int nLen = atoi(psHeader->m_szLength);

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;			
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH ;	

	CString strData ;
	strncpy( strData.GetBuffer(nStrLen) , pData + FROMTOID_HEADER_LENGTH, nStrLen);	
	strData.ReleaseBuffer();

	S_COMMON_HEADER sMSGHeader;
	S_FROMTOID_HEADER   sFromToIdHeader;
	int iLength = 0;	
	int nTotalLen = 0;
	
	char* pSzBuf = NULL;
	char szTemp[20];
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(&sFromToIdHeader, NULL, sizeof(sFromToIdHeader));
	memset(szTemp, NULL, sizeof(szTemp));

	iLength = strlen((LPCSTR)strData) + FROMTOID_HEADER_LENGTH+ 1;
	
	// 헤더부 설정	
	memcpy(sMSGHeader.m_szUsage, SOMEONE_WANTS_SEND_FILES, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sFromToIdHeader.m_szToID , pInfo->m_szToID , sizeof(sFromToIdHeader.m_szToID));
	memcpy(sFromToIdHeader.m_szFromID , pInfo->m_szFromID , sizeof(sFromToIdHeader.m_szFromID));	
	memcpy(sFromToIdHeader.m_szIP , pSockObj->m_szIP , sizeof(sFromToIdHeader.m_szIP)); 
	
	//연결된 클라이언트에서 전달대상 검색후 전송		
	CIpSockObjMapInfo *pa= NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_SomeoneWantsSendFiles");

	if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) pa ))
	{
		CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sFromToIdHeader, sizeof(sFromToIdHeader));				
			memcpy(pSzBuf + COMMON_HEADER_LENGTH+FROMTOID_HEADER_LENGTH, (LPCSTR)strData, strlen((LPCSTR)strData));	
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient("doWork_SomeoneWantsSendFiles",pInfo->m_szToID,pSpckObjMap, pSzBuf, nTotalLen) ;
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_SomeoneWantsSendFiles SendDataToClient\n",
										__LINE__, pSockObj->m_szIP );
			}			
		}
	}
		
	return TRUE;
}

BUFFER_OBJ *  CThreadSockAsyncServer::GetBufferObj(int buflen)
{
	BUFFER_OBJ *newobj=NULL;
	newobj = (BUFFER_OBJ *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BUFFER_OBJ)); 
	
	if (newobj == NULL)
	{ 
		CLog::Instance().WriteFile(FALSE, "Error",  "Ln [%d] [%d] GetBufferObj: HeapAlloc 1 failed \n", 
								__LINE__, GetLastError() );
		
		ExitProcess(-1);
	}
	
	newobj->buf = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE) *buflen);    
	if (newobj->buf == NULL)
	{
		CLog::Instance().WriteFile(FALSE, "Error",  "Ln [%d] [%d] GetBufferObj: HeapAlloc 2 failed\n",
										__LINE__ , GetLastError() );		
		ExitProcess(-1);
	}
	newobj->buflen = buflen;    
	newobj->addrlen = sizeof(newobj->addr);	
	return newobj;
}

BOOL CThreadSockAsyncServer::doWork_AllMsg( SOCKET_OBJ* pSockObj )
{	
	char* pRawData= pSockObj->m_pBuff ;	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;

	// ToID갯수 DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 메시지 DELIM1 전송자 이름	

	int nLen = atoi(psHeader->m_szLength) ;		
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	S_CHAT_HEADER * pInfo = (S_CHAT_HEADER*) pData;			
	int nStrLen = nLen - S_CHAT_HEADER_LENGTH ;		
	char szAllMsg[MAX_CHAT_LEN+1];
	memset(szAllMsg, 0x00, sizeof(szAllMsg));
	CString strData ;
	strData.Empty();
	strncpy( strData.GetBuffer(nStrLen) , pData + S_CHAT_HEADER_LENGTH, nStrLen);	
	strData.ReleaseBuffer();

	// 전달 메시지 구함
	int nLastPos = strData .ReverseFind(DELIM1);		
	strncpy(szAllMsg, pData + S_CHAT_HEADER_LENGTH+ nLastPos + 1, nStrLen - nLastPos -1  );
	
	int nPos = 1;
	char szTmpOut [20 + 1]; 

	memset(szTmpOut, 0x00, sizeof(szTmpOut));
	nPos = GetStrByDelim( (LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
	
	int nCnt = atoi(szTmpOut) ; // ToID 갯수 
	
	S_COMMON_HEADER sMSGHeader;
	S_CHAT_HEADER   sChatHeader;
	int iLength = 0;	
	int nTotalLen = 0;			
	char* pSzBuf = NULL;
	char szTemp[20];

	for(int i =0 ; i < nCnt; i++)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);

		if(nPos != -1 )
		{			
			iLength = 0;	
			nTotalLen = 0;			
			pSzBuf = NULL;
			memset(szTemp, NULL, sizeof(szTemp));	
			memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
			memset(&sChatHeader, NULL, sizeof(sChatHeader));
			
			iLength = strlen( szAllMsg ) + S_CHAT_HEADER_LENGTH+ 1;
			
			// 헤더부 설정	
			memcpy(sMSGHeader.m_szUsage, ALL_MSG, sizeof(sMSGHeader.m_szUsage));
			sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
			memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
			sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
			memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
			
			memcpy(sChatHeader.szIDTo , szTmpOut , sizeof(sChatHeader.szIDTo));
			memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
			
			//연결된 클라이언트에서 전달대상 검색후 전송		
			CIpSockObjMapInfo *pa= NULL;
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllMsg");

				if(mIDIPMap.Lookup(szTmpOut, ( CObject*& ) pa ))
				{
					CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
					
					if(pSpckObjMap )
					{
						// 전송데이터 모으기
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
						memset(pSzBuf, NULL, COMMON_HEADER_LENGTH + iLength );	
						memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
						memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));
						memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH,szAllMsg, strlen(szAllMsg));	
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength;
						
						int nRtn = SendDataToClient("doWork_AllMsg",szTmpOut, pSpckObjMap, pSzBuf, nTotalLen) ;
					
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_AllMsg SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
						}				
					}
				}
			}			
		}

		Sleep(1); // send 를 연속적으로 여러번 하는 경우에 대한 고려가 추후 필요함!!!
	}
	
	return TRUE;
}

// Multi Chat 
BOOL CThreadSockAsyncServer::doWork_MultiChat( SOCKET_OBJ* pSockObj )
{	
	char* pRawData= pSockObj->m_pBuff ;	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_CHAT_HEADER * pInfo = (S_CHAT_HEADER*) pData;			
	int nStrLen = nLen - S_CHAT_HEADER_LENGTH ;	

	// 성능을 고려해서 MAX_CHAT_LEN 만큼만 처리하자..
	// Client 에서도 MAX_CHAT_LEN 최대로 보냄..
	char szChatMsg[MAX_CHAT_LEN+1];
	memset(szChatMsg, 0x00, sizeof(szChatMsg));
	strncpy(szChatMsg, pData + S_CHAT_HEADER_LENGTH, nStrLen );

	/* 데이터 길이에 제한 없는 경우..
//	CString strData ;
//	strncpy( strData.GetBuffer(nStrLen) , pData + FROMTOID_HEADER_LENGTH, nStrLen);	
//	strData.ReleaseBuffer();
	*/

	S_COMMON_HEADER sMSGHeader;
	S_CHAT_HEADER   sChatHeader;
	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(&sChatHeader, NULL, sizeof(sChatHeader));
	memset(szTemp, NULL, sizeof(szTemp));
	
	//iLength = strlen((LPCSTR)strData) + S_CHAT_HEADER_LENGTH+ 1; // 데이터 길이에 제한 없는 경우..
	iLength = strlen(szChatMsg) + S_CHAT_HEADER_LENGTH+ 1;	
	
	// 헤더부 설정	
	memcpy(sMSGHeader.m_szUsage, MULTI_CHAT_MSG, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sChatHeader.szIDTo , pInfo->szIDTo , sizeof(sChatHeader.szIDTo));
	memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
	
	//연결된 클라이언트에서 전달대상 검색후 전송		
	
	MAP_STR2SET::iterator it;
	SET_STR::iterator itSet;

	it= m_mapStr2SetMultiChat.find(pInfo->szIDTo) ; // pInfo->m_szToID => ServerRoomNo, set을 찾는다..
	
	if(it != m_mapStr2SetMultiChat.end() )  
	{
		// set에 있는 상대방들에게 전송한다.
		for (itSet= (*it).second.begin(); itSet != (*it).second.end(); ++itSet) 
		{
			CIpSockObjMapInfo *pa= NULL;		
						
			{ // ScopeLock 를 지역객체로 만든다. 
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChat");

				if(mIDIPMap.Lookup( (*itSet).c_str(), ( CObject*& ) pa ))
				{
					if( strcmp(pInfo->szIDFrom, (*itSet).c_str()) == 0 )
					{
						//LeaveCriticalSection(&m_CS); //20090306
						//continue; // 이런식으로쓰면 위험하다.... 
						//else 로 분리하는게 안전 
					}
					else
					{
						CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
					
						if(pSpckObjMap  )
						{			
							// 전송데이터 모으기
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
							memset(pSzBuf, NULL, sizeof(pSzBuf));	
							memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
							memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));				
							//memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, (LPCSTR)strData, strlen((LPCSTR)strData));	//데이터 길이에 제한 없는 경우..
							memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, szChatMsg, strlen(szChatMsg));						
							
							nTotalLen = COMMON_HEADER_LENGTH + iLength;
							
							int nRtn = SendDataToClient("doWork_MultiChat",(char*)((*itSet).c_str()), pSpckObjMap, pSzBuf, nTotalLen) ;
							if(nRtn == -1 )
							{
								CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_MultiChat SendDataToClient\n",
												__LINE__, pSockObj->m_szIP );
							}									
						}
					}				
				}
			}			
		}		
	}
	
	return TRUE;
}

// 채팅 메시지
BOOL CThreadSockAsyncServer::doWork_Chat( SOCKET_OBJ* pSockObj )
{
	// SOCKET_OBJ* pSockObj
	char* pRawData= pSockObj->m_pBuff ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_CHAT_HEADER * pInfo = (S_CHAT_HEADER*) pData;			
	int nStrLen = nLen - S_CHAT_HEADER_LENGTH ;	

	// 성능을 고려해서 MAX_CHAT_LEN 만큼만 처리하자..
	// Client 에서도 MAX_CHAT_LEN 최대로 보냄..
	char szChatMsg[MAX_CHAT_LEN+1];
	memset(szChatMsg, 0x00, sizeof(szChatMsg));
	strncpy(szChatMsg, pData + S_CHAT_HEADER_LENGTH, nStrLen );

	/* 데이터 길이에 제한 없는 경우..
//	CString strData ;
//	strncpy( strData.GetBuffer(nStrLen) , pData + S_CHAT_HEADER_LENGTH, nStrLen);	
//	strData.ReleaseBuffer();
	*/

	S_COMMON_HEADER sMSGHeader;
	S_CHAT_HEADER   sChatHeader;
	int iLength = 0;	
	int nTotalLen = 0;
	
	char* pSzBuf = NULL;			
	//int  nSendRtn = SOCKET_ERROR;
	char szTemp[20];
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(&sChatHeader, NULL, sizeof(sChatHeader));
	
	//iLength = strlen((LPCSTR)strData) + S_CHAT_HEADER_LENGTH+ 1; // 데이터 길이에 제한 없는 경우..
	iLength = strlen(szChatMsg) + S_CHAT_HEADER_LENGTH+ 1;
	
	
	// 헤더부 설정	
	memcpy(sMSGHeader.m_szUsage, CHAT_MSG, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sChatHeader.szIDTo , pInfo->szIDTo , sizeof(sChatHeader.szIDTo));
	memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
	
	//연결된 클라이언트에서 전달대상 검색후 전송		
	CIpSockObjMapInfo *pa= NULL;		
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_Chat");

	if(mIDIPMap.Lookup(pInfo->szIDTo, ( CObject*& ) pa ))
	{
		CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));				
			//memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, (LPCSTR)strData, strlen((LPCSTR)strData));	//데이터 길이에 제한 없는 경우..
			memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, szChatMsg, strlen(szChatMsg));	
						
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient("doWork_Chat",pInfo->szIDTo, pSpckObjMap, pSzBuf, nTotalLen) ;
			
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_Chat SendDataToClient\n",
										__LINE__, pSockObj->m_szIP );
			}
		}
	}
		
	return TRUE;
}		


void PrintVal(string& str)
{
    TRACE("%s\n", str.c_str() );	
}



BOOL CThreadSockAsyncServer::doSqlRsltWork_CHNG_NICK(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
			
	if(pSCallBackInfo->m_bRslt)
	{			
		MAP_STR2VEC::iterator it;
		VEC_STR::iterator itV;
		it = m_mapStr2VecCompanion.find(pSCallBackInfo->m_szIDFrom); 
		
		if(it != m_mapStr2VecCompanion.end() )  
		{	
			S_CHG_NICK sChngNick;							
			memset(&sChngNick , 0x00, sizeof(sChngNick));				
			memcpy(sChngNick.szIDFrom , pSCallBackInfo->m_szIDFrom, sizeof(sChngNick.szIDFrom));
			memcpy(sChngNick.szNickAft, pSCallBackInfo->m_szMultiRoleUsefulInfo, sizeof(sChngNick.szNickAft));				
			
			for (itV=((*it).second).begin(); itV != ((*it).second).end(); itV++) 
			{		
				//TRACE("%s\n", (*itV).c_str() );	
				
				CIpSockObjMapInfo *pSpckObjMap = NULL;
								
				{
					CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_CHNG_NICK");

					if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
					{
						if(pSpckObjMap )
						{
							iLength = sizeof(S_CHG_NICK);	
							
							// 헤더부 설정			
							memcpy(sMSGHeader.m_szUsage, ACK_NICK_CHG, sizeof(sMSGHeader.m_szUsage));
							sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
							memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
							sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // 데이터 갯수!
							memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
							
							// 전송데이터 모으기
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
							memset(pSzBuf, NULL, sizeof(pSzBuf));		
							
							memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sChngNick, sizeof(sChngNick)); //공통 헤더 + sChngNick
							
							nTotalLen = COMMON_HEADER_LENGTH + iLength; 
							
							int nRtn = SendDataToClient("doSqlRsltWork_CHNG_NICK",(char*)((*itV).c_str()), pSpckObjMap, pSzBuf, nTotalLen); 
				
							if(nRtn == -1 )
							{
								CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] => [%s] doSqlRsltWork_CHNG_NICK SendDataToClient\n",
											__LINE__, pSCallBackInfo->m_szIDFrom, (*itV).c_str() );
							}
							else
							{
								TRACE("doSqlRsltWork_CHNG_NICK : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
									pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
									pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
							}
						}					
					}
				}				
			}				
		}			
	}
	else
	{
		TRACE("ERROR doSqlRsltWork_CHNG_NICK : pSCallBackInfo->m_szUsage [%s] m_szIDFrom[%s] m_szIDTo [%s] [%s]\n" , 
			pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo, pSCallBackInfo->m_szErrMsg);
	}	
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);	

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_2(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));		
	
	// S_COMMON_HEADER 이후 데이터 : S_SQL_RSLTDATA + 데이터 + MULL
	
	CString strRslt = "";
		
	int nPos = pSCallBackInfo->m_strData.find(DELIM1) ;		
	string strTmpOut = pSCallBackInfo->m_strData.substr(0, nPos); 
	
	TRACE("\n **** strTmpOut [%s]\n" , strTmpOut.c_str() );

	// 접속여부
	CIpSockObjMapInfo *pa = NULL;		
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_2:1");

		if(mIDIPMap.Lookup(strTmpOut.c_str() , ( CObject*& ) pa ))
		{
			if(pa  )
			{
				// 이미 IP는 구해서 전달한 상태지만..				
				pSCallBackInfo->m_strData+= string ("Y") ; 
				pSCallBackInfo->m_strData+= DELIM1 ; 
			}					
		}
		else
		{			
			pSCallBackInfo->m_strData+= string ("N") ; 
			pSCallBackInfo->m_strData+= DELIM1 ; 
		}	
	}
	
	TRACE("\n **** pSCallBackInfo->m_strData [%s]\n" , pSCallBackInfo->m_strData.c_str() );
	
	//int nSqlDataLen = pSCallBackInfo->m_strData.GetLength() ; 
	int nSqlDataLen = pSCallBackInfo->m_strData.length() ; 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen + 1;	// 동적으로 데이터 사이즈 변동 		
	
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
	
	
	//연결된 클라이언트에서 전달대상 검색후 전송
	CIpSockObjMapInfo * pSpckObjMap = NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_2:2");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage ,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_2 SendDataToClient\n",
										__LINE__, pSpckObjMap->m_pSockObj->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_2 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
			}			
		}
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_ADD_BUDDY_GRP(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;			
	//int  nSendRtn = SOCKET_ERROR;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));
	
	S_ADD_BUDDY_CLISVR sGrpInfo;
	memset(&sGrpInfo, NULL, sizeof(sGrpInfo));
	
	iLength = SQL_RSLTDATA_LENGTH + sizeof(S_ADD_BUDDY_CLISVR);	
				
	strcpy(sGrpInfo.m_szBuddyGrp, pSCallBackInfo->m_szMultiRoleUsefulInfo); // m_szMultiRoleUsefulInfo=>추가한 그룹명
	
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송		
	
	CIpSockObjMapInfo *pa = NULL;	
	CIpSockObjMapInfo * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ADD_BUDDY_GRP");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			//!!!!
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, &sGrpInfo, sizeof(S_ADD_BUDDY_CLISVR) ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient("doSqlRsltWork_ADD_BUDDY_GRP",pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 
			
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_ADD_BUDDY_GRP SendDataToClient\n",
										__LINE__, pSpckObjMap->m_pSockObj->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_ADD_BUDDY_GRP : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);	

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_4(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));	
	memset(szTemp, NULL, sizeof(szTemp));
	
	iLength = SQL_RSLTDATA_LENGTH + strlen(pSCallBackInfo->m_szMultiRoleUsefulInfo);	
	
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송		
	
	CIpSockObjMapInfo *pa = NULL;	
	CIpSockObjMapInfo * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_4");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, pSCallBackInfo->m_szMultiRoleUsefulInfo, strlen(pSCallBackInfo->m_szMultiRoleUsefulInfo) ); 
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 				
			
			if(nRtn == -1 )
			{			
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_4 SendDataToClient\n",
										__LINE__, pSpckObjMap->m_pSockObj->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_4 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	
			}
		}			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

//GET_BUDDY_GRP
BOOL CThreadSockAsyncServer::doSqlRsltWork_5(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));

	// S_COMMON_HEADER 이후 데이터 : S_SQL_RSLTDATA + 데이터 + MULL
	//int nSqlDataLen = pSCallBackInfo->m_strData.GetLength(); 
	int nSqlDataLen = pSCallBackInfo->m_strData.length(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// 동적으로 데이터 사이즈 변동 
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송	=> ID로 검색!!	
	CIpSockObjMapInfo * pa = NULL;
	
	CIpSockObjMapInfo * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_5");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
							
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_5 SendDataToClient!!\n",
					__LINE__, pSpckObjMap->m_pSockObj->m_szIP);	
				CTime iDT =  CTime::GetCurrentTime();				
			}
			else
			{
				TRACE("doSqlRsltWork_5 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
			}
		}			
	}
	else
	{
		//Log	
		CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] doSqlRsltWork_5 USAGE[%s] SQL응답, ID 못찾음!!! FAIL!![%s] [%s]\n", 
			__LINE__, pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIDFrom, pSCallBackInfo->m_szIPFrom );			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_GET_BUDDY_LIST (S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));

	// ID를 가지고 상대방에게 전달하는 경우.. (mIDIPMap)	
	
	// KIND, NAME, NICK, COMPID, COMPGRP, IP ... (0 ~ 5)		
		
	int nIndex = 0;
	int nPos;
	char szTmpOut [256];
	char szTmpID [20+1];		
	string strSQLRslt;
	int nTotalCnt = pSCallBackInfo->m_nRecordCnt;
						
	while( 1 )
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));			
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)pSCallBackInfo->m_strData.c_str(), szTmpOut, DELIM1);
		
		if(nPos != -1 )
		//if(nTotalCnt > 0 )
		{
			// KIND, NAME, NICK, COMPID, COMPGRP, IP, CON_STATUS, AWAY_INFO
			switch(nIndex % 6)
			{
			case 0:	// KIND
			case 1:	// NAME
			case 2:	// NICK
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1); 					
				break;
			case 3:	// COMPID 
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1); 									
				memset(szTmpID, 0x00, sizeof(szTmpID));
				strncpy(szTmpID, szTmpOut, sizeof(szTmpID));
				break;
			case 4:	// COMPGRP				
				strSQLRslt.append(szTmpOut)  ;
				strSQLRslt.append(1, DELIM1); 					
				break;
			case 5: //IP
				{										
					// CON_STATUS, AWAY_INFO 를 데이터 끝에 추가한다. (index 5,6)
					CIpSockObjMapInfo *pa= NULL;
					
					{										
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_BUDDY_LIST");

						if(mIDIPMap.Lookup(szTmpID, ( CObject*& ) pa ))
						{
							if( pa )
							{
								//IP
								strSQLRslt.append(pa->m_pSockObj->m_szIP)  ; 
								strSQLRslt.append(1, DELIM1);

								//CON_STATUS
								strSQLRslt.append("Y")  ; //6
								strSQLRslt.append(1, DELIM1); 
								
								//AWAY_INFO //7
								strSQLRslt.append(pa->m_pSockObj->m_szAwayInfo)  ;
								strSQLRslt.append(1, DELIM1); 								
							}					
							else
							{
								//IP
								strSQLRslt.append(szTmpOut)  ;
								strSQLRslt.append(1, DELIM1);

								//CON_STATUS
								strSQLRslt.append("N")  ;
								strSQLRslt.append(1, DELIM1); 										
								
								//AWAY_INFO
								strSQLRslt.append("0")  ;
								strSQLRslt.append(1, DELIM1); 
							}	
						}
						else
						{
							//IP
							strSQLRslt.append(szTmpOut)  ;
							strSQLRslt.append(1, DELIM1);

							//CON_STATUS
							strSQLRslt.append("N")  ;
							strSQLRslt.append(1, DELIM1); 										
							
							//AWAY_INFO
							strSQLRslt.append("0")  ;
							strSQLRslt.append(1, DELIM1); 
						}
					}					
					
					nTotalCnt--;
				}
				
				break;
			}

			nIndex++;				
		}
		
		if(nPos < 0)
		//if(nTotalCnt == 0 )
		{
			break;
		}
	}
			
	//////////////////////////////////////////////////////////////////////////////

	// S_COMMON_HEADER 이후 데이터 : S_SQL_RSLTDATA + 데이터 + MULL
	int nSqlDataLen = strlen(strSQLRslt.c_str()); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// 동적으로 데이터 사이즈 변동 
	
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송		
	CIpSockObjMapInfo* pa= NULL;	
	char szTmpIp[20];
	memset(szTmpIp, 0x00, sizeof(szTmpIp) );
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_BUDDY_LIST2");

		if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
		{		
			if(pa)
			{
				strncpy(szTmpIp, pa->m_pSockObj->m_szIP , sizeof(szTmpIp));

				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, strSQLRslt.c_str(), nSqlDataLen ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
				
				nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
				
				int nRtn = SendDataToClient("doSqlRsltWork_GET_BUDDY_LIST",pSCallBackInfo->m_szIDFrom, pa, pSzBuf, nTotalLen); 

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_GET_BUDDY_LIST SendDataToClient!!\n",
								__LINE__, pa->m_pSockObj->m_szIP);
				}
				else
				{
					TRACE("doSqlRsltWork_GET_BUDDY_LIST#1 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
							pa->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
							pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
				}
			}			
		}	
	}
		
	// 로그인하는 시점에 이 사용자를 대화상대로 하고있는 사람들에게 로그인했음을 알림!
	MAP_STR2VEC::iterator it;
	VEC_STR::iterator itV;	
		
	it = m_mapStr2VecCompanion.find(pSCallBackInfo->m_szIDFrom);
	if(it != m_mapStr2VecCompanion.end() )  
	{	
		S_BUDDY_ONLINE sOnlineBuddy;			
		memset(&sOnlineBuddy , 0x00, sizeof(sOnlineBuddy));			
		memcpy(sOnlineBuddy.m_szBuddyID , pSCallBackInfo->m_szIDFrom, sizeof(sOnlineBuddy.m_szBuddyID));
		memcpy(sOnlineBuddy.m_szBuddyIP , szTmpIp, sizeof(sOnlineBuddy.m_szBuddyIP));
		
		for (itV=((*it).second).begin(); itV != ((*it).second).end(); itV++) 
		{
			CIpSockObjMapInfo *pSpckObjMap = NULL ;				
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_BUDDY_LIST3");

				if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
				{				
					if(pSpckObjMap  )
					{
						S_COMMON_HEADER sMSGHeader;
						int iLength = 0;	
						int nTotalLen = 0;	
						char* pSzBuf = NULL;								
						char szTemp[20];	
						
						memset(&sMSGHeader, NULL, sizeof(sMSGHeader));					
						
						iLength = sizeof(S_BUDDY_ONLINE);	
						
						// 헤더부 설정			
						memcpy(sMSGHeader.m_szUsage, ACK_ONLINE, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // 데이터 갯수!
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// 전송데이터 모으기
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sOnlineBuddy, sizeof(sOnlineBuddy)); //공통 헤더 + sOnlineBuddy
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						//TRACE("로그인하는 시점에 로그인했음을 알림! [%s]\n", (*itV).c_str() );
						
						int nRtn = SendDataToClient("ACK_ONLINE",(char*)((*itV).c_str()), pSpckObjMap, pSzBuf, nTotalLen); 
						
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln[%d] SendDataToClient [%s]\n",
									__LINE__, pSpckObjMap->m_pSockObj->m_szIP);
						}
						else
						{
							TRACE("doSqlRsltWork_GET_BUDDY_LIST#2 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
								pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
								pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
						}
					}					
				}
			}			
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
	

	// 화면에 뿌림.
	char szTmpIP[200];
	memset(szTmpIP, 0x00, sizeof(szTmpIP));
	sprintf(szTmpIP,"\n★ %s [%s] connected..",  pSCallBackInfo->m_szIPFrom, pSCallBackInfo->m_szIDFrom ); //20061106
					
	m_pNotifyProc((LPVOID) m_pDlg,szTmpIP , NULL);

	m_pNotifyProc((LPVOID) m_pDlg, "", &gCurrentConnections); 
	
	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_7(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));
		
	// S_COMMON_HEADER 이후 데이터 : S_SQL_RSLTDATA + 데이터 + MULL
	//int nSqlDataLen = pSCallBackInfo->m_strData.GetLength(); // strlen( (LPCSTR)pSCallBackInfo->m_strData); 
	int nSqlDataLen = pSCallBackInfo->m_strData.length(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen + 1;	// 동적으로 데이터 사이즈 변동 		
	
	// 헤더부 설정			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // 데이터 갯수!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA 데이터를 S_COMMON_HEADER 뒤에 붙이고, + 데이터를 붙임		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));
	
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sSqlRsltData.szRsltCd, "N", min(sizeof(sSqlRsltData.szRsltCd), strlen("N")));
	}
	memcpy(sSqlRsltData.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
				
	//연결된 클라이언트에서 전달대상 검색후 전송 => IP 로 검색!!!
	
	CIpSockObjMapInfo *pa = NULL;
	CIpSockObjMapInfo * pSpckObjMap = NULL;
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_7");
	
	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap )
		{
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //공통 헤더 + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //공통 헤더 + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength 에는 이미 SQL_RSLTDATA_LENGTH 포함
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "doSqlRsltWork_7 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, 
					pSCallBackInfo->m_szUsage, 
					m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , 
					pSCallBackInfo->m_szIDTo	);	
			}
			else
			{
				TRACE("doSqlRsltWork_7 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
				
				//여기서
				/*
				RAWSQL_SAVE_LAST_IP / doSqlRsltWork_9/ BOOL CChatSession::doWork_RAWSQL_SAVE_LAST_IP( char* data) / LRESULT CLogInDlg::OnSaveLastIPRslt(WPARAM wParam, LPARAM lParam) / 
				RAWSQL_JOB_GN    / doSqlRsltWork_7/ BOOL CChatSession::doWork_RAWSQL_JOB_GN (char* data) / LRESULT CLogInDlg::OnJobGnRslt(WPARAM wParam, LPARAM lParam)
				위에서 종료하거나 
				RAWSQL_DAILYJOB_MST /doSqlRsltWork_7/ BOOL CChatSession::doWork_RAWSQL_DAILYJOB_MST (char* data)   / LRESULT CLogInDlg::OnDailyJobMstRslt(WPARAM wParam, LPARAM lParam)    
				RAWSQL_INSERT_LOGONTIME /doSqlRsltWork_9   / BOOL CChatSession::doWork_RAWSQL_INSERT_LOGONTIME( char* data) / LRESULT CLogInDlg::OnInsertLogOnTmRslt(WPARAM wParam, LPARAM lParam)
				*/
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_8(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;				
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));
		
	S_CHK_USERID sChkUserId;
	memset(&sChkUserId, NULL, sizeof(sChkUserId));
	
	iLength = sizeof(S_CHK_USERID);	
	
	// 데이터부 설정		
	if(pSCallBackInfo->m_nRecordCnt  > 0)
	{
		memcpy(sChkUserId.szExists, "Y", min(sizeof(sChkUserId.szExists), strlen("Y")));
	}
	else
	{
		memcpy(sChkUserId.szExists, "N", min(sizeof(sChkUserId.szExists), strlen("N")));
	}	
	
	memcpy(sChkUserId.szPasswd, (LPCSTR)pSCallBackInfo->m_strData.c_str(), sizeof(sChkUserId.szPasswd) - 1 );			
	
	// 헤더부 설정		
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
	//연결된 클라이언트에서 전달대상 검색후 전송
	
	CIpSockObjMapInfo *pa = NULL ;	
	CIpSockObjMapInfo * pSpckObjMap = NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_8");

	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	
	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChkUserId, iLength);	
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_8 SendDataToClient! [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP );					
			}			
			else
			{
				TRACE("doSqlRsltWork_8 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CThreadSockAsyncServer::doSqlRsltWork_9(S_CALLBACKINFO* pSCallBackInfo)
{
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));

	iLength = sizeof(S_COM_RESPONSE);		
	
	// 데이터부 설정
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sComRslt.szRsltCd, "Y", min(sizeof(sComRslt.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
	}
	
	memcpy(sComRslt.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
	
	// 헤더부 설정		
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	//연결된 클라이언트에서 전달대상 검색후 전송		
	// RAWSQL_UPDATE_LOGOFFTIME 인 경우는 응답 안보낸다.
	
	if(strcmp( pSCallBackInfo->m_szUsage,RAWSQL_UPDATE_LOGOFFTIME ) ==0 )
	{
		CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
		return TRUE;
	}

	CIpSockObjMapInfo * pa = NULL;			
	CIpSockObjMapInfo * pSpckObjMap = NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_9");

	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
		
		if(pSpckObjMap  )
		{			
			// 전송데이터 모으기
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sComRslt, iLength);	
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 			

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_9 SendDataToClient! [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP );				
			}
			else
			{
				TRACE("doSqlRsltWork_9 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_pSockObj->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
	
	return TRUE;
}



BOOL CThreadSockAsyncServer::doSqlRsltWork_ALERT_PENDING_WORK(S_CALLBACKINFO* pSCallBackInfo)
{	
	// 1. REQUIRE_PERSON_ID 10
	// 2. REQUIRE_PERSON    50 
	// 3. WORK_ID           20
	// 4. WORK_NAME         50
	// 5. REQUIRE_NUM       8
	// 6. REQUIRE_NAME	    100
	// 7. REQUIRE_DATE      10
	// 8. END_DUE_DATE	    10
	
	int nIndex = 0;		
	char szRequirePersonID [20+1];
	char szRequirePerson   [50+1];
	char szWorkID          [20+1];
	char szWorkName        [50+1];
	char szRequireNum      [ 8+1];
	char szRequireName     [100+1];
	char szRequireDate     [10+1];
	char szEndDueDate      [10+1];

	char szTmpOut [100+1];
	char szMsg [1024+1];
	
	int nPos = pSCallBackInfo->m_strData.length();
	
	while( 1 )
	{
		memset(szTmpOut , 0x00, sizeof(szTmpOut));			
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)pSCallBackInfo->m_strData.c_str(), szTmpOut, DELIM1);
		
		if(nPos != -1)
		{
			switch(nIndex % 8)
			{
			case 0:
				{	// 1. REQUIRE_PERSON_ID 요청자
					memset(szRequirePersonID , 0x00, sizeof(szRequirePersonID));			
					strncpy(szRequirePersonID , szTmpOut, sizeof(szRequirePersonID) ); 			
				}
				break;			
			case 1:
				{	// 2. REQUIRE_PERSON 					
					memset(szRequirePerson , 0x00, sizeof(szRequirePerson));			
					strncpy(szRequirePerson , szTmpOut, sizeof(szRequirePerson) ); 			
				}					
				break;
			case 2:
				{	// 3. WORK_ID 		개발자			
					memset(szWorkID , 0x00, sizeof(szWorkID));			
					strncpy(szWorkID , szTmpOut, sizeof(szWorkID) ); 			
				}					
				break;
			case 3:
				{	// 4. WORK_NAME 					
					memset(szWorkName , 0x00, sizeof(szWorkName));			
					strncpy(szWorkName , szTmpOut, sizeof(szWorkName) ); 			
				}					
				break;
			case 4:
				{	// 5. REQUIRE_NUM 	요청번호				
					memset(szRequireNum , 0x00, sizeof(szRequireNum));			
					strncpy(szRequireNum , szTmpOut, sizeof(szRequireNum) ); 	
				}					
				break;
			case 5:
				{	// 6. REQUIRE_NAME	과제명					
					memset(szRequireName , 0x00, sizeof(szRequireName));			
					strncpy(szRequireName , szTmpOut, sizeof(szRequireName) ); 	
				}					
				break;
			case 6:
				{	// 7. REQUIRE_DATE 	요청일				
					memset(szRequireDate , 0x00, sizeof(szRequireDate));			
					strncpy(szRequireDate , szTmpOut, sizeof(szRequireDate) ); 	
				}					
				break;
			case 7:
				{	// 8. END_DUE_DATE	만료예정일		
					memset(szEndDueDate , 0x00, sizeof(szEndDueDate));
					memset(szMsg , 0x00, sizeof(szMsg));
					strncpy(szEndDueDate , szTmpOut, sizeof(szEndDueDate) ); 	
					
					sprintf(szMsg,"\r\n  수시과제 적기수행률 관리를 위해 알려드립니다.\r\n  - 요청번호: %s\r\n  - 내용: %s\r\n  - 요구일 [%s] 완료희망일[%s]\r\n  을 완료하여 주시기 바랍니다.",
						szRequireNum,szRequireName,szRequireDate,szEndDueDate);
					
					//보낸다.
					S_COMMON_HEADER sMSGHeader;
					S_CHAT_HEADER   sChatHeader;
					int iLength = 0;	
					int nTotalLen = 0;
					char* pSzBuf = NULL;
					char szTemp[20];
					
					memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
					memset(&sChatHeader, NULL, sizeof(sChatHeader));
					
					iLength = strlen( szMsg ) + S_CHAT_HEADER_LENGTH+ 1;
					
					// 헤더부 설정	
					memcpy(sMSGHeader.m_szUsage, ALL_MSG, sizeof(sMSGHeader.m_szUsage));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
					memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
					memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
					
					memcpy(sChatHeader.szIDTo , szWorkID , sizeof(sChatHeader.szIDTo));
					memcpy(sChatHeader.szIDFrom , "SERVER" , sizeof(sChatHeader.szIDFrom));
					
					//연결된 클라이언트에서 전달대상 검색후 전송		
					CIpSockObjMapInfo *pa= NULL;	
					
															
					// 개발자에게 전송					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ALERT_PENDING_WORK");

						if(mIDIPMap.Lookup(szWorkID, ( CObject*& ) pa ))
						{
							CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
							
							if(pSpckObjMap )
							{
								// 전송데이터 모으기
								pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
								memset(pSzBuf, NULL, COMMON_HEADER_LENGTH + iLength );	
								memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
								memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));
								memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH,szMsg, strlen(szMsg));	
								
								nTotalLen = COMMON_HEADER_LENGTH + iLength;
								
								int nRtn = SendDataToClient("doWork_AllMsg",szWorkID, pSpckObjMap, pSzBuf, nTotalLen) ;

								if(nRtn == -1 )
								{
									CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_ALERT_PENDING_WORK SendDataToClient! [%s]\n", 
										__LINE__ , pSpckObjMap->m_pSockObj->m_szIP );								
								}							
							}
						}
					}
					
					// KT담당자에게 전송					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ALERT_PENDING_WORK2");

						if(mIDIPMap.Lookup(szRequirePersonID, ( CObject*& ) pa ))
						{
							CIpSockObjMapInfo * pSpckObjMap = static_cast<CIpSockObjMapInfo*>(pa);
							
							if(pSpckObjMap )
							{					
								// 전송데이터 모으기
								pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
								memset(pSzBuf, NULL, COMMON_HEADER_LENGTH + iLength );	
								memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
								memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));
								memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH,szMsg, strlen(szMsg));	
								
								nTotalLen = COMMON_HEADER_LENGTH + iLength;
								
								int nRtn = SendDataToClient("doWork_AllMsg",szRequirePersonID, pSpckObjMap, pSzBuf, nTotalLen) ;

								if(nRtn == -1 )
								{
									CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_ALERT_PENDING_WORK SendDataToClient! [%s]\n", 
										__LINE__ , pSpckObjMap->m_pSockObj->m_szIP );								
								}			
								else
								{
									CLog::Instance().WriteFile(FALSE, "AlertPending", "[%s]\n%s\n",  szRequirePersonID, szMsg );
								}
							}
						}
					}					
				}					
				break;
			}
		}
		
		if(nPos < 0)
		{			
			
			break;
		}
		
		nIndex++;
		
		Sleep(1); 
	}

	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT); //20090610 

	return TRUE;
}

// 전체 사용자 목록 UPDATE 버튼 누른 경우
BOOL CThreadSockAsyncServer::doSqlRsltWork_UPDATE_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo)
{	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_UPDATE_ALL_USER_INIT");
	
	m_strAllUserList = (LPCSTR)pSCallBackInfo->m_strData.c_str();
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	// 목록만 변경.
	// m_pDlg->SendMessage(WM_IMADE_ALL_USERLIST,0,0);
			
	TRACE("doSqlRsltWork_UPDATE_ALL_USER_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	

	return TRUE;
}

// 전체 사용자 목록 
BOOL CThreadSockAsyncServer::doSqlRsltWork_GET_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo)
{
	//USER_ID, USER_NAME, TEAM_GN, TEAM_NAME	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_ALL_USER_INIT");
	
	m_strAllUserList = (LPCSTR)pSCallBackInfo->m_strData.c_str();
	

	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	// Main Dlg 로 알림.
	m_pDlg->SendMessage(WM_IMADE_ALL_USERLIST,0,0);
		
	TRACE("doSqlRsltWork_GET_ALL_USER_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	
	
	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] doSqlRsltWork_GET_ALL_USER_INIT\n", __LINE__  );	
	
	return TRUE;
}

// 한사용자에 대한 대화상대 목록을 만든다.
BOOL CThreadSockAsyncServer::doSqlRsltWork_GET_COMP_LIST_INIT(S_CALLBACKINFO* pSCallBackInfo)
{
	//DWORD n_Start = GetTickCount();
	
	//USER_ID DELIM1 COMPANION_ID DELIM1 USER_ID DELIM1 COMPANION_ID ...		
	
	BOOL bFirstFlag = TRUE;
	int nIndex = 0;		
	char szTmpOut [20+1];
	//int nPos = pSCallBackInfo->m_strData.GetLength();
	int nPos = pSCallBackInfo->m_strData.length();
	char szKeyOld [20+1];
	char szKeyCur [20+1];	
	VEC_STR v1;
	
	memset(szKeyOld , 0x00, sizeof(szKeyOld));			
	memset(szKeyCur , 0x00, sizeof(szKeyCur));	
		
	while( 1 )
	{
		memset(szTmpOut , 0x00, sizeof(szTmpOut));			
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)pSCallBackInfo->m_strData.c_str(), szTmpOut, DELIM1);
		
		if(nPos != -1)
		{
			switch(nIndex % 2)
			{
			case 0:
				{	
					if(bFirstFlag)
					{
						strncpy(szKeyOld , szTmpOut, sizeof(szKeyOld) ); 						
						strncpy(szKeyCur , szTmpOut, sizeof(szKeyCur) ); 		
						
						bFirstFlag = FALSE;
					}
					else
					{
						strncpy(szKeyCur , szTmpOut, sizeof(szKeyCur) );
					}
				}
				break;			
			case 1:
				{						
					if(strcmp(szKeyOld , szKeyCur) != 0)
					{
						VEC_STR vTmp;							
						VEC_STR::iterator itV;
						
						for (itV=v1.begin(); itV != v1.end(); ++itV) 
						{								
							vTmp.push_back((*itV)) ;
						}
						
						v1.clear();	
						m_mapStr2VecCompanion.insert( MAP_STR2VEC::value_type( szKeyOld, vTmp ) );
						strncpy(szKeyOld , szKeyCur, sizeof(szKeyCur) ); // Old Set						
					}
					
					v1.push_back( szTmpOut );
				}					
				break;
			}
		}
		
		if(nPos < 0)
		{				
			break;
		}
		
		nIndex++;
	}	
	
	m_mapStr2VecCompanion.insert( MAP_STR2VEC::value_type( szKeyOld, v1 ) );
	v1.clear();	
		
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
	
	
	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] Server 기동!\n", __LINE__  );	
	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	
	// Main Dlg 로 알림.
	m_pDlg->SendMessage(WM_IMADEUSERLIST,0,0);
			
	TRACE("doSqlRsltWork_GET_COMP_LIST_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	

	return TRUE;
}


int CThreadSockAsyncServer::SendDataToClient(	char * szUsage, char * szFromID, 
												CIpSockObjMapInfo * pSpckObjMap, char* pSzBuf, int nTotalLen)
{
	BUFFER_OBJ *newbuf = NULL;
	newbuf = GetBufferObj(nTotalLen);
	
	if(newbuf == NULL)
	{
		TRACE("ERROR !! GetBufferObj return NULL !!!!\n\n");
		
		if(pSzBuf)
		{
			delete[] pSzBuf; 
			pSzBuf = NULL;
		}

		if( m_bLog && m_bChkFilterIP )
		{
			if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 )
			{
				CTime iDT =  CTime::GetCurrentTime();	
				char szPath [200];
				memset(szPath, 0x00, sizeof(szPath));
				sprintf(szPath,"\n★ %s :SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); 
				m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
				
				CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage ); 
			}
		}
		else if( m_bLog && !m_bChkFilterIP)
		{
			// 화면에 뿌림.
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n★ %s :SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); //20061106					
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage );			
		}

		return -1;
	}

	memcpy(newbuf->buf, pSzBuf, nTotalLen);
			
	EnqueueBufferObj(pSpckObjMap->m_pSockObj, newbuf, FALSE); 
						
	if(pSzBuf)
	{
		delete[] pSzBuf; 
		pSzBuf = NULL;
	}
	
	int rc = SendPendingData(szUsage, pSpckObjMap->m_pSockObj);
						
	if (rc == -1)  	
	{					
		if( m_bLog && m_bChkFilterIP )
		{
			if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 )
			{
				CTime iDT =  CTime::GetCurrentTime();	
				char szPath [200];
				memset(szPath, 0x00, sizeof(szPath));
				sprintf(szPath,"\n★ %s :SendPendingData ERROR !! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); 
				m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
				
				CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData ERROR [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage ); 
			}
		}
		else if( m_bLog && !m_bChkFilterIP)
		{
			// 화면에 뿌림.
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n★ %s :SendPendingData ERROR !! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); //20061106					
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData ERROR [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage );    		
		}

		return -1;
	}	
	
	// LOGGING //////////////////////////////////////
	if( m_bLog && m_bChkFilterIP )
	{
		if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_pSockObj->m_szIP ) == 0 )
		{
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n★ %s :SendDataToClient return 0(OK) [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); 
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ♥♥♥ [%s] SendDataToClient return 0(OK)  usage [%s]\n\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage );			
		}
	}
	else if( m_bLog && !m_bChkFilterIP)
	{
		// 화면에 뿌림.
		CTime iDT =  CTime::GetCurrentTime();	
		char szPath [200];
		memset(szPath, 0x00, sizeof(szPath));
		sprintf(szPath,"\n★ %s :SendDataToClient return 0(OK) [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_pSockObj->m_szIP, szUsage ); //20061106					
		m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
		
		CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ♥♥♥ [%s] SendDataToClient return 0(OK)  usage [%s]\n\n", 
						__LINE__ , pSpckObjMap->m_pSockObj->m_szIP, szUsage );
		
	}
	// LOGGING //////////////////////////////////////

	return 0;
}


// ▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩
// DB 작업 요청시 call back.. (1개의 조회/처리성 SQL이 Transaction 처리 경우)
// 서버에서 구조체를 만들기 위해 구분자 PArsing작업은 하지 않는다.
// (클라이언트가 연속된 데이터 받아서 구분자 처리한다) 
// ▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩▩
void CThreadSockAsyncServer::SQLNotifyProc(	S_CALLBACKINFO* pSCallBackInfo  )										
{	
	CThreadSockAsyncServer* pTh = (CThreadSockAsyncServer*) pSCallBackInfo->m_pThread ;	
		
 // 045                                 doSqlRsltWork_CHNG_NICK	
 // 013, 014                            doSqlRsltWork_2
 // 018									doSqlRsltWork_ADD_BUDDY_GRP	
 // 053, 055							doSqlRsltWork_4
 // 003, 005, 050, 051, 59				doSqlRsltWork_5
 // 004									doSqlRsltWork_GET_BUDDY_LIST 
 // 009, 010, 011, 012, 008, 006, 007   doSqlRsltWork_7
 // 036                                 doSqlRsltWork_8
 // 035, 037, 031, 034, 042             doSqlRsltWork_9
 // 056                                 doSqlRsltWork_GET_COMP_LIST_INIT
 	
	static BOOL( CThreadSockAsyncServer::* fPArySQLServer[] )(S_CALLBACKINFO* pSCallBackInfo ) = 
	{	// doDummySqlRsltWork 만 하면 -> 표준에 의해서 VC2008 부터는 Compiler Error C3867 			
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  0
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  01  
		&CThreadSockAsyncServer::doDummySqlRsltWork,                //  02  
		&CThreadSockAsyncServer::doSqlRsltWork_5,	                //  03  GET_BUDDY_GRP
		&CThreadSockAsyncServer::doSqlRsltWork_GET_BUDDY_LIST,		//  04  GET_BUDDY_LIST 
		&CThreadSockAsyncServer::doSqlRsltWork_5,	                //  05  SEARCH_ID
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  06  RAWSQL_JOB_GN
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  07  RAWSQL_DAILYJOB_MST
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  08  RAWSQL_LOGIN => 더이상 사용안됨.  LOGIN 사용.	
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  09  RAWSQL_GETUSER_INFO
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  10  RAWSQL_TEAM
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  11  RAWSQL_GETCOMPANY_INFO
		&CThreadSockAsyncServer::doSqlRsltWork_7,					//  12  RAWSQL_GETCLASS_INFO
		&CThreadSockAsyncServer::doSqlRsltWork_2,					//  13  RAWSQL_GETBUDDY_INFO
		&CThreadSockAsyncServer::doSqlRsltWork_2,					//  14  SOMEONE_ADD_ME
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  15  DEL_BUDDY_GRP
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  16  DEL_BUDDY
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  17  CHG_BUDDY_GRP	
		&CThreadSockAsyncServer::doSqlRsltWork_ADD_BUDDY_GRP,		//  18  ADD_BUDDY_GRP
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  19  CHG_GRP_NAME
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  20  YOUALLOWEDASCOMP
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  21  SOMEONE_DEL_ME 
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  22  ACK_NICK_CHG
		&CThreadSockAsyncServer::doDummySqlRsltWork,	            //  23  SOMEONE_WANTS_SEND_FILES			
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  24  MULTICHAT_SERVER_ROOMNO  
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  25  CANCEL_FILE_TRANSFER
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  26  ALL_MSG
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  27  ALL_MSG_ACK	
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  28  FOLDER_SELECTING 
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  29  NO_THANKS_FILE_TRANSFER
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  30  RAWSQL_REG_NEW_USER
		&CThreadSockAsyncServer::doSqlRsltWork_9,					//  31  RAWSQL_SAVE_LAST_IP 
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  32  ALREADY_LOGIN 
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  33  LOGIN_ALLOWED
		&CThreadSockAsyncServer::doSqlRsltWork_9,					//  34  RAWSQL_INSERT_LOGONTIME
		&CThreadSockAsyncServer::doSqlRsltWork_9,					//  35  RAWSQL_CHK_DUP_ID
		&CThreadSockAsyncServer::doSqlRsltWork_8,					//  36  RAWSQL_CHK_USERID	
		&CThreadSockAsyncServer::doSqlRsltWork_9,					//  37  RAWSQL_UPDATE_PASSWD
		&CThreadSockAsyncServer::doDummySqlRsltWork,	    		//  38  ADD_BUDDY_LIST_ERR	
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  39  ACK_ONLINE	  		
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  40  ACK_OFFLINE
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  41  YOUALLOWEDASCOMP_INFORM	
		&CThreadSockAsyncServer::doSqlRsltWork_9,					//  42  RAWSQL_UPDATE_LOGOFFTIME 
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  43  ADD_BUDDY_LIST		
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  44  CHK_ALREADY_LOGIN
		&CThreadSockAsyncServer::doSqlRsltWork_CHNG_NICK,    		//  45  CHNG_NICK	
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  46  MULTICHAT_GET_ROOMNO
		&CThreadSockAsyncServer::doDummySqlRsltWork,	         	//  47  MULTI_CHAT_MSG
		&CThreadSockAsyncServer::doDummySqlRsltWork,				//  48  SET_MULTICHAT_SERVER_ROOMNO	
		&CThreadSockAsyncServer::doDummySqlRsltWork ,				//  49  EXIT_MULTI_CHAT
		&CThreadSockAsyncServer::doSqlRsltWork_5       ,			//  50  SLIP_MSG
		&CThreadSockAsyncServer::doSqlRsltWork_5        ,           //  51  GET_SLIP_MSG
		&CThreadSockAsyncServer::doDummySqlRsltWork,                //  52  RAWSQL_ACK_SLIP_MSG
		&CThreadSockAsyncServer::doSqlRsltWork_4        ,           //  53  ADD_IT_COMP  
		&CThreadSockAsyncServer::doDummySqlRsltWork     ,           //  54  
		&CThreadSockAsyncServer::doSqlRsltWork_4    ,               //  55  
		&CThreadSockAsyncServer::doSqlRsltWork_GET_COMP_LIST_INIT , //  56 GET_COMP_LIST_INIT
		&CThreadSockAsyncServer::doDummySqlRsltWork  ,              //  57 STATUS_BUSY
		&CThreadSockAsyncServer::doSqlRsltWork_7		,			//  58 LOGIN 
		&CThreadSockAsyncServer::doSqlRsltWork_5      ,             //  59 FOOD_MENU
		&CThreadSockAsyncServer::doDummySqlRsltWork   ,             //  60 ALL_USER_ID_TEAM
		&CThreadSockAsyncServer::doSqlRsltWork_GET_ALL_USER_INIT,   //  61 GET_ALL_USER_INIT
		&CThreadSockAsyncServer::doDummySqlRsltWork,                //  62  
		&CThreadSockAsyncServer::doSqlRsltWork_UPDATE_ALL_USER_INIT,//  63 UPDATE_ALL_USER_INIT
		&CThreadSockAsyncServer::doSqlRsltWork_ALERT_PENDING_WORK   //  64
	} ;
	
	(pTh->*fPArySQLServer[ atoi(pSCallBackInfo->m_szUsage)]) (pSCallBackInfo); 		
}



//클라이언트 요청 처리
int	CThreadSockAsyncServer::_do_recv(SOCKET_OBJ * sock)
{	
	char szTemp[MAX_BUFF_SIZE];
	int receive_length;	
	
	memset(szTemp,0x00,sizeof(szTemp));		

	if((receive_length = recv(sock->s, (char *)szTemp, MAX_BUFF_SIZE, 0)) > 0) // Receive  	
	{   
		char *pTemp = sock->m_pBuff; // 이전 데이터 copy..

		// 이전 데이터 + 지금 받은 데이터만큼 메모리 버퍼를 할당한다.		
		sock->m_pBuff = new char[sock->m_iBuffLength + receive_length+1];
		memset(sock->m_pBuff, 0x00, sock->m_iBuffLength + receive_length +1);
		
		if(sock->m_pBuff == NULL)
		{	
			//break;
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
		memcpy(sock->m_pBuff + sock->m_iBuffLength, szTemp, receive_length);

		// 받은 데이터 사이즈 증가 기록..
		//TRACE("받은 데이터 사이즈 증가 기록sock->m_iBuffLength [%d] receive_length[%d]\n", sock->m_iBuffLength, receive_length);		
		sock->m_iBuffLength = sock->m_iBuffLength + receive_length;
	
		GetHeaderInfo(sock);
		
		if(sock->nTotalOnePacketLen  < 0)
		{
			TRACE("CThreadSockAsyncServer::_do_recv() iOnePacketLength < 0 ..Break\n");
			//break;
			return 0;
		}
		// 아직 한 Packet의 데이터가 오질 않았다면 다음 데이터를 기다린다.		
		else if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
		{
			TRACE("아직 한 Packet의 데이터가 오질 않았다\n");
			//break;
			return 0;
		}
		// 한 Packet을 구성할 데이터가 도착하였다면 그 길이만큼을 처리
		else
		{
			TRACE("▷1▷ 한 Packet을 구성할 데이터가 도착 sock->nTotalOnePacketLen[%d]\n",sock->nTotalOnePacketLen);

			// nExcessiveLen => 하나의 전송 데이터를 초과해서 더 받은 길이 
			// 더받은 길이 = 현재 받은 데이터 - 하나의 패킷 데이터	
			int nExcessiveLen = sock->m_iBuffLength - sock->nTotalOnePacketLen ;
			
			ProcessRecvData(sock);

			TRACE("△ 2 수신 완료 (수신데이터 처리완료) nExcessiveLen [%d]\n",nExcessiveLen);

			//정해진 패킷 데이터 보다 더많은 데이터 수신의 경우 -> 버퍼를 다시 구성 
			//gThreadArray[nIndex].pBuffer 에 nExcessiveLen 만큼 할당후
			//복사한다. 그리고 처리가 다 된 데이터는 삭제. 
			if(nExcessiveLen > 0)
			{
				TRACE("정해진 패킷 데이터 보다 더많은 데이터 수신의 경우 버퍼를 다시 구성\n");				
				pTemp = sock->m_pBuff;
								
				sock->m_pBuff = new char[nExcessiveLen];				
				
				if(sock->m_pBuff == NULL)
				{
					return 0;
				}

				// nExcessiveLen 만큼의 데이터 복사, pTemp는 아직 한패킷 + 여분의 데이터 상태..
				memcpy(sock->m_pBuff, pTemp + sock->nTotalOnePacketLen , nExcessiveLen);				
				sock->m_iBuffLength = nExcessiveLen;

				if(pTemp)
				{					
					delete[] pTemp;
					pTemp = NULL;
				}

				// 더받은 데이터가 하나의 패킷을 구성하는 경우는 모두 처리한다. 
				// 즉, 더받은 버퍼에 처리할수있는 패킷이 1개 이상이 될수도 있다.
				
				while(nExcessiveLen > 0)
				{
					GetHeaderInfo(sock);
					
					if(sock->nTotalOnePacketLen  < 0)
					{
						TRACE("CThreadSockAsyncServer::_do_recv() iOnePacketLength < 0 ..Break\n");						
						return 0;
					}
					else if(sock->nTotalOnePacketLen  > sock->m_iBuffLength)
					{
						TRACE("아직 한 Packet의 데이터가 오질 않았다\n");						
						return 0;
					}
					// 한 Packet을 구성할 데이터가 도착하였다면 그 길이만큼을 처리
					// 더받은 데이터가 하나의 패킷을 구성하는 경우는 모두 처리한다. 
					else
					{ 	
						ProcessRecvData(sock);
						nExcessiveLen = nExcessiveLen - sock->nTotalOnePacketLen ;

						if(nExcessiveLen == 0)
						{
							// 더받았고 패킷용량에 맞게 다 전송된 경우 
							if(sock->m_pBuff)
							{					
								delete[] sock->m_pBuff;
								sock->m_pBuff = NULL;
							}				
							sock->m_iBuffLength = 0;
							return 0;
						}
						else if(nExcessiveLen < 0)
						{
							return 0;
						}
						else
						{
							memcpy(sock->m_pBuff, sock->m_pBuff + sock->nTotalOnePacketLen , nExcessiveLen);
						}
					}
				} // while 
			}
			else
			{
				if(sock->m_pBuff)
				{
					delete[] sock->m_pBuff;
					sock->m_pBuff = NULL;
				}				
				sock->m_iBuffLength = 0;
			}
		}		
	}
	else if( receive_length == SOCKET_ERROR )
	{
		int nRet = GetLastError();
		
		if(nRet == WSAEWOULDBLOCK)
		{	
			TRACE("▶ 수신 WSAEWOULDBLOCK ERROR \n");
			return 1;
		}
		if(nRet == WSAENOBUFS)
		{	
			TRACE("▶ 수신 WSAENOBUFS ERROR \n");
			return 1;
		}		
		else
		{
			TRACE("▶ ERROR : 수신 에러 [%d] \n", nRet);
			//break;			
			
			//shutdown(sock->s, SD_BOTH ); //20070110
			
			//FreeBufferObj(buffobj);
			/* 20070323 */
			if(sock->m_pBuff)
			{
				//TRACE("CThreadSockAsyncServer::_do_recv() delete m_pBuff \n");							
					
				delete[] sock->m_pBuff;
				sock->m_pBuff = NULL;
			}				
			sock->m_iBuffLength = 0;
		
					
			//closesocket(sock->s); 
			//sock->s = INVALID_SOCKET; 

			return -1;
		}
	}
	else if( receive_length == 0 ) 
	{			
		TRACE("\n▶▶▶ receive_length == 0  \n");
		//int nRet = GetLastError();
		//break;
		
		/* 20070323 */
		if(sock->m_pBuff)
		{
			//TRACE("CThreadSockAsyncServer::_do_recv() delete m_pBuff \n");							
					
			delete[] sock->m_pBuff;
			sock->m_pBuff = NULL;
		}				
		sock->m_iBuffLength = 0;
		/* 20070323 */		
		
		// Set the socket object to closing
        sock->closing = TRUE;

        if (sock->pending == NULL)
        {
            // If no sends are pending, close the socket for good
			
            //closesocket(sock->s);
            //sock->s = INVALID_SOCKET;

            return -1; // Socket Close..
        }
        else
        {
            // Sends are pending, just return
            return 1;
        }
	}		
	
	TRACE("△ 3 수신 종료\n");
	
	return 1;
	
}


//    소켓에 처리해야할 이벤트가 발생할때마다 실행된다.
int  CThreadSockAsyncServer::HandleIo(THREAD_OBJ * thread, SOCKET_OBJ *sock)
{
    WSANETWORKEVENTS nevents;
    int              rc;

    // Enumerate the events
    rc = WSAEnumNetworkEvents(sock->s, sock->event, &nevents);
    
	if (rc == SOCKET_ERROR)
	{
		//TRACE("HandleIo: WSAEnumNetworkEvents ERROR: %d\n", WSAGetLastError());

		CLog::Instance().WriteFile(FALSE, "ServerLog", "★ERR★ Ln [%d] [%s] HandleIo: WSAEnumNetworkEvents SOCKET_ERROR [%d]\n", 
						__LINE__ ,sock->m_szIP, WSAGetLastError() );

        return SOCKET_ERROR;
    }

    if (nevents.lNetworkEvents & FD_READ) 
	{
        // Check for read error
        if (nevents.iErrorCode[FD_READ_BIT] == 0) 
		{        							
			rc = _do_recv(sock); //kojh

			if (rc == -1)
			{
				TRACE("_do_recv() return -1\n");
				
				CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] [%s] HandleIo: nevents.lNetworkEvents & FD_READ -1 \n", 
						__LINE__ ,sock->m_szIP  );
				
				{
					CAutoLock ScopeLock ( m_ScopeCS, "lNetworkEvents & FD_READ");					
				
					//mIPSockObjMap.RemoveKey(sock->m_szIP);
					CString strSockID ;
					strSockID.Format("%d", sock->s) ;	
					mSockIdSockObjMap.RemoveKey(strSockID); 

					mIDIPMap.RemoveKey(sock->m_szUserId);  

					CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] FD_READ RemoveSocketObj !\n", __LINE__);
					RemoveSocketObj(thread, sock);
					FreeSocketObj(sock);
					sock = NULL;
					
				}
				
                return SOCKET_ERROR;
            }
        }
		else
		{
            //ERROR..

			TRACE("HandleIo: FD_READ ERROR %d\n", nevents.iErrorCode[FD_READ_BIT]);
            
			CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] [%s] HandleIo: FD_READ ERROR \n", 
						__LINE__ ,sock->m_szIP  );
			
			{
				CAutoLock ScopeLock ( m_ScopeCS, "HandleIo: FD_READ ERROR");
							
				//mIPSockObjMap.RemoveKey(sock->m_szIP); 
				
				CString strSockID ;
				strSockID.Format("%d", sock->s) ;	 
				mSockIdSockObjMap.RemoveKey(strSockID); 
				mIDIPMap.RemoveKey(sock->m_szUserId);

				CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] FD_READ RemoveSocketObj !\n", __LINE__);
				RemoveSocketObj(thread, sock);
				FreeSocketObj(sock);
				sock = NULL;				
			}

            return SOCKET_ERROR;
        }
    } //! nevents.lNetworkEvents & FD_READ
    
	if (nevents.lNetworkEvents & FD_WRITE) 
	{
        // Check for write error
        if (nevents.iErrorCode[FD_WRITE_BIT] == 0) 
		{	
			{
				CAutoLock ScopeLock ( m_ScopeCS, "HandleIo: FD_WRITE");			
				
				rc = SendPendingData("FD_WRITE",sock);				
			}

			if (rc == -1)
			{
				CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] [%s] nevents.lNetworkEvents & FD_WRITE -1 \n", 
						__LINE__ ,sock->m_szIP  );
				
				{
					CAutoLock ScopeLock ( m_ScopeCS, "HandleIo: FD_WRITE -1");
									
					//mIPSockObjMap.RemoveKey(sock->m_szIP); 

					CString strSockID ;
					strSockID.Format("%d", sock->s) ;	
					mSockIdSockObjMap.RemoveKey(strSockID); 
					mIDIPMap.RemoveKey(sock->m_szUserId);  

					CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] FD_WRITE RemoveSocketObj !\n", __LINE__);
					RemoveSocketObj(thread, sock);
					FreeSocketObj(sock);
					sock = NULL;					
				}
				
				return SOCKET_ERROR;
			}			
        }
		else
		{            
			TRACE("HandleIo: FD_WRITE ERROR %d\n", nevents.iErrorCode[FD_WRITE_BIT]);
            return SOCKET_ERROR;
        }
    } //! nevents.lNetworkEvents & FD_WRITE
    
	if (nevents.lNetworkEvents & FD_CLOSE)
	{		
		TRACE("*** FD_CLOSE Set!!\n");
		
		if (nevents.iErrorCode[FD_CLOSE_BIT] == 0)
		{	
			CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] FD_CLOSE 정상 [%s] \n", 	__LINE__, sock->m_szIP );	
		}
		else
		{
			CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] FD_CLOSE ERROR [%s] \n", 	__LINE__, sock->m_szIP );	
		}		
		        				
		InformWhenDisconnected(thread, sock);             
		
		// 20090312 !!!!
		// SOCKET_ERROR 을 돌려줘서 RenumberThreadArray 가 바로 호출되도록 

		return SOCKET_ERROR; 

    } //! nevents.lNetworkEvents & FD_CLOSE

    return NO_ERROR;
}

void CThreadSockAsyncServer::InformWhenDisconnected(THREAD_OBJ *thread, SOCKET_OBJ * sock)
{	
	// 연결종료 사용자를 대화상대로하는 사용자에게 알린다.
	
	// 20090525 chk alive 로 인한 연결끊김 처리는 다른 사용자들에게 알리면 안됨
	// => mIDIPMap 에서 ID로 찾아서 그 sock id 가 입력인자와 틀리면
	// (즉 , 연재 연결된 sock과 틀린 sock에서 발생한 close event..)
	// chk alive로 인한 것이라고 판단, 사용자들에게 알리지 않는다.
	CIpSockObjMapInfo* pObj = NULL;
	if ( mIDIPMap.Lookup( sock->m_szUserId, ( CObject*& ) pObj )) 
	{
		if(pObj && pObj->m_pSockObj->s == sock->s )
		{
			MAP_STR2VEC::iterator it;
			VEC_STR::iterator itV;
			
			CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] InformWhenDisconnected Start [%s]!\n", __LINE__, sock->m_szIP );
			
			it = m_mapStr2VecCompanion.find(sock->m_szUserId);
			
			if(it != m_mapStr2VecCompanion.end() )  
			{	
				S_BUDDY_OFFLINE sOfflineBuddy;	
				memset(&sOfflineBuddy , 0x00, sizeof(sOfflineBuddy));			
				memcpy(sOfflineBuddy.m_szBuddyID , sock->m_szUserId, sizeof(sOfflineBuddy.m_szBuddyID));
				
				for (itV=((*it).second).begin(); itV != ((*it).second).end(); itV++) 
				{					
					CIpSockObjMapInfo *pSpckObjMap = NULL;
					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "InformWhenDisconnected");
						
						if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
						{
							if(pSpckObjMap  )
							{			
								S_COMMON_HEADER sMSGHeader;
								int iLength = 0;	
								int nTotalLen = 0;	
								char* pSzBuf = NULL;								
								char szTemp[20];	
								
								memset(&sMSGHeader, NULL, sizeof(sMSGHeader));					
								
								iLength = sizeof(S_BUDDY_OFFLINE);	
								
								// 헤더부 설정			
								memcpy(sMSGHeader.m_szUsage, ACK_OFFLINE, sizeof(sMSGHeader.m_szUsage));
								sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
								memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
								sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
								memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
								
								// 전송데이터 모으기
								pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
								memset(pSzBuf, NULL, sizeof(pSzBuf));		
								
								memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
								memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sOfflineBuddy, sizeof(sOfflineBuddy)); //공통 헤더 + sOfflineBuddy
								
								nTotalLen = COMMON_HEADER_LENGTH + iLength; 
								
								BUFFER_OBJ *newbuf = NULL;
								newbuf = GetBufferObj(nTotalLen);
								
								memcpy(newbuf->buf, pSzBuf, nTotalLen);
								
								EnqueueBufferObj(pSpckObjMap->m_pSockObj, newbuf, FALSE); 					
								
								if(pSzBuf)
								{
									delete[] pSzBuf; 
									pSzBuf = NULL;
								}						
								
								//TRACEX("연결종료 -> 사용자에게 알린다 [%s]\n", (*itV).c_str() );					
								
								int rc = SendPendingData("FD_CLOSE #1", pSpckObjMap->m_pSockObj);							
								
								if (rc == -1) 
								{
									// 로그오프 알리다가 실패난 경우 처리
									if(pSpckObjMap && pSpckObjMap->m_pSockObj)
									{
										CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] ☆ 로그오프 알리다가 실패난 경우 ID [%s]!\n", __LINE__, pSpckObjMap->m_pSockObj->m_szUserId );
									}						
								}
							}
						}				
					}
				}//for
			}			
		}
	}
	
	//TEST
	//Sleep(3000);
					
	TRACE("연결끊김 [%s]\n", sock->m_szIP);
	CIpSockObjMapInfo *pa= NULL;	
	
		
	CAutoLock ScopeLock ( m_ScopeCS, "InformWhenDisconnected 2");

	CString strSockID ;
	strSockID.Format("%d", sock->s) ;	
	
	// 연결이 끊어진 사용자 처리
	
	char szTmpIP[200];
	memset(szTmpIP, 0x00, sizeof(szTmpIP));
	sprintf(szTmpIP,"\n☆ %s [%s] disconnected..", sock->m_szIP , sock->m_szUserId );
	m_pNotifyProc((LPVOID) m_pDlg,szTmpIP , NULL);						
			
	CLog::Instance().WriteFile(FALSE, "ServerLog", "● Ln [%d] 연결이 끊어진 사용자처리 ip #2 [%s] sock [%s] ID [%s]!\n", 
				__LINE__, sock->m_szIP , strSockID, sock->m_szUserId );
						
	TRACE("연결이 끊어진 사용자 처리 ID [%s]\n", sock->m_szUserId);
			
	InterlockedDecrement(&gCurrentConnections);				
						
	m_pNotifyProc((LPVOID) m_pDlg, "", &gCurrentConnections);	
			
	if(mSockIdSockObjMap.Lookup(strSockID, ( CObject*& ) pa )) 
	{ 		
		mSockIdSockObjMap.RemoveKey(strSockID); 

		CIpSockObjMapInfo* pObjTmp = NULL;
		if ( mIDIPMap.Lookup( sock->m_szUserId, ( CObject*& ) pObjTmp )) // 20090525 chk alive 로 인한 연결끊김 처리
		{
			// 20090525 chk alive 로 인한 연결끊김 처리는 sockid가 도일한 경우만.
			// 그냥 mIDIPMap 에서 삭제하면 , 비정상 소켓처리시에도 삭제되는 현상있다.
			// 다른 사용자가 다시 로그인 하면 이 사용자는 로그오프로 보이게 된다.(mIDIPMap 에 없으므로)
			if(pObjTmp && pObjTmp->m_pSockObj->s == sock->s )
			{
				mIDIPMap.RemoveKey(sock->m_szUserId);
			}
		}

		CLog::Instance().WriteFile(FALSE, "ServerLog", "● Ln [%d] 연결이 끊어진 사용자 처리 sock [%s] RemoveSocketObj !\n", __LINE__, strSockID);
				
		RemoveSocketObj(thread, sock);
		
		FreeSocketObj(sock);  
		sock = NULL;			

		// sock 객체 clear.			
		delete pa;
		pa = NULL;
		TRACE("sock 객체 clear");		
		CLog::Instance().WriteFile(FALSE, "ServerLog", "[%d] sock 객체 clear!\n", __LINE__);
	}
	
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] InformWhenDisconnected End !\n" );
}

// 쓰레드의 socket event 배열에 쓰레드 소켓 객체의 이벤트를 모두 다시 저장한다(Compaction)
void CThreadSockAsyncServer::RenumberThreadArray(THREAD_OBJ *thread)
{
	EnterCriticalSection(&thread->ThreadCritSec);

    SOCKET_OBJ *sptr=NULL;
    int         i;

	if(!thread)
	{
		CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] ★ RenumberThreadArray thread NULL=> return!\n", __LINE__ );
	
		LeaveCriticalSection(&thread->ThreadCritSec); 
		return ;
	}
    
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] RenumberThreadArray 시작\n", __LINE__ );
	
    i = 0;
    sptr = thread->SocketList;

	// Compaction !
    while (sptr && (DWORD)sptr != 0xfeeefeee ) 
	{
        thread->Handles[i+1] = sptr->event;
        i++;
        sptr = sptr->next;
    }

    LeaveCriticalSection(&thread->ThreadCritSec); 

	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] RenumberThreadArray 종료\n", __LINE__ );
}

// 클라이언트의 요청을 처리할 쓰레드(이미 Accept된)
unsigned __stdcall ChildThread( void* lpParam )
{	
    THREAD_OBJ *thread=NULL;    
	SOCKET_OBJ* sockobj=NULL;
    int	index=0;
	int rc = 0;
	int i = 0;

	thread = static_cast<THREAD_OBJ *>(lpParam);
		
	while (1) 
	{
		rc = WaitForMultipleObjects(thread->SocketCount + 1, thread->Handles, FALSE, INFINITE);
		
		if (rc == WAIT_FAILED || rc == WAIT_TIMEOUT) 
		{
			rc = GetLastError();
			
			if(rc == ERROR_INVALID_HANDLE)
			{
				CLog::Instance().WriteFile(FALSE, "ErrorTerminate", 
					"Ln [%d] ★ ERROR_INVALID_HANDLE : WaitForMultipleObjects failed (ERROR_INVALID_HANDLE) thread->SocketCount [%d]\n", __LINE__ ,thread->SocketCount );
				
				//TRACE("ChildThread: WaitForMultipleObjects failed: ERROR_INVALID_HANDLE\n");
				//break; //프로그램 종료 !!						
			}
			else
			{
				CLog::Instance().WriteFile(FALSE, "ErrorTerminate", 
					"Ln [%d] ★ Thread죽었다 : WaitForMultipleObjects failed: [%d]!! thread->SocketCount [%d]\n", __LINE__, rc , thread->SocketCount );
				
				//TRACE("ChildThread: WaitForMultipleObjects failed: %d\n", rc);
				//break;
			}				
		}
		else
		{
			// 동시에 여러개의 이벤트들이 set된 경우 대비, 모두 검사한다.            
			for(i=0; i < thread->SocketCount + 1 ;i++)
			{
				rc = WaitForSingleObject(thread->Handles[i], 0);
				
				if (rc == WAIT_FAILED)
				{					
					rc = GetLastError();
					
					if(rc == ERROR_INVALID_HANDLE)
					{
						CLog::Instance().WriteFile(FALSE, "ErrorTerminate", 
							"Ln [%d] ★ ERROR_INVALID_HANDLE WaitForSingleObject failed [ERROR_INVALID_HANDLE]!! thread->SocketCount[%d]\n", 
							__LINE__ ,thread->SocketCount );
						
						//TRACE("ChildThread: WaitForSingleObject failed: ERROR_INVALID_HANDLE\n");
						//break;
					}
					else
					{
						CLog::Instance().WriteFile(FALSE, "ErrorTerminate", 
							"Ln [%d] ★ Thread죽었다 WaitForSingleObject failed: [%d]!! thread->SocketCount[%d]\n", 
							__LINE__, rc , thread->SocketCount);
						
						//break;
						
					}
				}
				else if (rc == WAIT_TIMEOUT)
				{
					// event가 signaled되지 않음, 다음으로 진행.
					continue;
				}
				
				index = i;
				
				//AssignToFreeThread(..)에서 쓰레드 생성후 이벤트를 set해 준다.				
				if (index == 0) 
				{
					// If index 0 is signaled then rebuild the array of event handles to wait on
					// 쓰레드의 socket event 배열에 쓰레드 소켓 객체의 이벤트를 모두 다시 저장한다(rebuild)
					if(!g_dlgPtr) // 20061120
					{
						CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] ★ ChildThread g_dlgPtr NULL #2 !!\n", __LINE__ );
					}
					

					WSAResetEvent(thread->Handles[index]);
					
					CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] ChildThread 이벤트에 의한 index=0 Call RenumberThreadArray\n", __LINE__ );
					
					g_dlgPtr->RenumberThreadArray(thread);  // 이벤트에 의한 (RemoveSocketObj, AssignToFreeThread ) 
					
					TRACE("i = 1 Set\n");
					i = 1;					
				}
				else 
				{
					// Otherwise, its an event associated with a socket that was signaled. Handle the IO on that socket.
					
					if(!g_dlgPtr) // 20061120
					{
						CLog::Instance().WriteFile(FALSE, "Error", "Ln [%d] ★ ChildThread g_dlgPtr NULL!!\n", __LINE__ );
					}
					
					// HANDLE	Handles[MAXIMUM_WAIT_OBJECTS]; // Array of socket's event handles
					// 이벤트 set된 것으로 소켓 객체를 구할때, thread->SocketList (소켓객체 리스트)에서  
					// 배열의 인덱스값으로 찾아온다. 
					
					sockobj = g_dlgPtr->FindSocketObj(thread, index-1);					
					
					//TRACE("★ [%d] ip [%s] index[%d] !!\n", __LINE__ , sockobj->m_szIP , index-1 );
					
					if (sockobj != NULL && 0xfeeefeee != (DWORD)sockobj->event && sockobj->s != INVALID_SOCKET)   //&& sockobj->s != INVALID_SOCKET 20070129
					{
						// HandleIo(..) 호출하여 I/O를 처리한다.
						if (g_dlgPtr->HandleIo(thread, sockobj) == SOCKET_ERROR) 
						{
							CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] ChildThread HandleIo 이후 Call RenumberThreadArray\n", __LINE__ );
														
							g_dlgPtr->RenumberThreadArray(thread); // 쓰레드 자체에서 직접 호출하는 경우.

							// HandleIo , RenumberThreadArray 
							// 동일한 쓰레드에서 호출하므로 동기화객체로 감싸지 않음..
						}
					}								
				}
			} // for 
		}
	}	

	CLog::Instance().WriteFile(FALSE, "ErrorTerminate", "Ln [%d] ★ ChildThread#1 Thread죽었다!\n", __LINE__ );
	exit( 1 ); //프로그램 종료 !!		

    ExitThread(0);
    return 0;
}



//    연결된 소켓을 child thread에 할당하여 I/O 처리하게 한다.
//    만약 쓰레드 최대 처리 이벤트를 초과하는등의 이유로 할당할 쓰레드가 없으면
//    새로운 쓰레드가 생성된다.
THREAD_OBJ * CThreadSockAsyncServer::AssignToFreeThread(SOCKET_OBJ *sock)
{

	THREAD_OBJ *thread=NULL;

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
		// client socket을 할당할 쓰레드를 찾지 못함. 
		// InsertSocketObj(thread, sock) SOCKET_ERROR 반환
		// 기존 쓰레드에 대기이벤트가 초과하여 추가할수 없다 -> 쓰레드 새로 생성.        
		TRACE("Creating new thread object\n");		
		
		CLog::Instance().WriteFile(FALSE, "Serverlog", "Creating new thread object : Cur Connection [%d]\n", gCurrentConnections  );		
		
		thread = GetThreadObj( __LINE__ ); //메모리 할당후,
		
		unsigned threadID;
		
		//쓰레드 생성			
		thread->Thread = (HANDLE) _beginthreadex(NULL, 0, &ChildThread, (LPVOID)thread, 0, &threadID ) ;				
        
		if (thread->Thread == NULL)
		{			
			TRACE("AssignToFreeThread: CreateThread ERROR: %d\n", GetLastError());
			
			CLog::Instance().WriteFile(FALSE, "ServerError", "[%d] AssignToFreeThread: CreateThread failed! \n", __LINE__ );		
			
			ExitProcess(-1);
		}
		
		InsertSocketObj(thread, sock); //생성된 쓰레드객체에 소켓 객체 추가
		// 생성된 쓰레드는 쓰레드 리스트에 추가.
        
		if (gChildThreads == NULL)
		{
			gChildThreads = thread;
		}else{
			thread->next = gChildThreads;
			gChildThreads = thread;
		}
		gChildThreadsCount++;
	} 
	
	// child thread가 event list를 재구성하게 한다.
	WSASetEvent(thread->Event); // thread->Handles[0] 를 SetEvent한다.
	
	return thread;   
}


int		CThreadSockAsyncServer::GetOnePacketLength(char *pData, int iLength)
{
	if(iLength < COMMON_HEADER_LENGTH)
		return -1;
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)pData;
	int nPacketLen = atoi(psHeader->m_szLength) + COMMON_HEADER_LENGTH ;
	//TRACE("전체 PAcket 길이 [%d]\n", nPacketLen);
	return nPacketLen;
}
// 헤더 정보를 포함하는 전체 Packet 길이 
// => 세부 헤더가 포함되는 경우 전체 길이와 헤더의 길이를 구해야함..

BOOL	CThreadSockAsyncServer::GetHeaderInfo(SOCKET_OBJ * pSockObj)
{
	if(pSockObj->m_iBuffLength < COMMON_HEADER_LENGTH)
		return -1;

	// 헤더 정보를 copy
	memcpy( &(pSockObj->sPacketHeader),  (S_COMMON_HEADER *)pSockObj->m_pBuff, sizeof(S_COMMON_HEADER));
	// 헤더 길이를 포함하는 전체 데이터 크기 
	pSockObj->nTotalOnePacketLen = atoi(pSockObj->sPacketHeader.m_szLength) + COMMON_HEADER_LENGTH ;		
	TRACE("전체 Packet 길이 [%d]\n", pSockObj->nTotalOnePacketLen);		
	return TRUE;
}	

void	CThreadSockAsyncServer::FreeThreadObj(THREAD_OBJ *thread)
{	
	WSACloseEvent(thread->Event);
	CloseHandle(thread->Thread);
	DeleteCriticalSection(&thread->ThreadCritSec);
	HeapFree(GetProcessHeap(), 0, thread);

	thread = NULL;	
}

int	CThreadSockAsyncServer::GetStrByDelim(char* szSrc, char* szOut, const char cDeli)
{
	int nPos = 0;
	//char	ch = 0;	
	int	n_size = strlen(szSrc);
	for ( int i=0; i< n_size; i++ ) 
	{
		if ( szSrc[i] == cDeli ) 
		{
			strncpy(szOut , szSrc , nPos) ; // 출력문자열
			strncpy(szSrc , szSrc + nPos + 1, n_size - nPos) ; // Src를 변경~
			return nPos;
		}
		else
		{
			nPos++;
		}		
	}
	strcpy(szOut , "NotFound" ) ; 
	return  -1;	
}

void	CThreadSockAsyncServer::FreeBufferObj(BUFFER_OBJ *obj)
{
	HeapFree(GetProcessHeap(), 0, obj->buf);
	HeapFree(GetProcessHeap(), 0, obj);
}


/*
//int	CThreadSockAsyncServer::AliveChkWork()
//{		
//	S_COMMON_HEADER sMSGHeader;
//	int iLength = 0;	
//	int nTotalLen = 0;	
//	char* pSzBuf = NULL;								
//	char szTemp[20];	
//	
//	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
//	
//	iLength = 0;	
//	
//	// 헤더부 설정			
//	memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
//	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
//	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
//	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
//	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
//	
//	CIpSockObjMapInfo *pSpckObjMap = NULL;		
//	
//	POSITION pos = mIPSockObjMap.GetStartPosition();
//	CString string;		
//
//	while( pos != NULL )
//	{ 
//		mIDIPMap.GetNextAssoc( pos, string, (CObject*&) pSpckObjMap );
//
//		if(pSpckObjMap && (DWORD)pSpckObjMap != 0xfeeefeee )
//		{				
//			// 전송데이터 모으기
//			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
//			memset(pSzBuf, NULL, sizeof(pSzBuf));		
//			
//			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
//			
//			nTotalLen = COMMON_HEADER_LENGTH + iLength; 
//			
//			int nRtn = 0;
//			
//			nRtn = SendDataToClient("doWork_ALIVE", "server", pSpckObjMap, pSzBuf, nTotalLen) ;
//			
//			if(nRtn == -1 )
//			{
//				CTime iDT =  CTime::GetCurrentTime();
//				char szTmpPath[100];
//				memset(szTmpPath, 0x00, sizeof(szTmpPath));
//				sprintf(szTmpPath, "c:\\data\\ServerError%s.log", iDT.Format("%Y%m%d")  );
//				writeLogFile(szTmpPath, "[%d] [%s] CHK ALIVE SendDataToClient !!\n",
//					__LINE__, iDT.Format("%Y%m%d%H%M%S"));	
//				
//				return FALSE;
//			}
//		}
//		
//		Sleep(1);
//	}
//
//	return 0;
//}
*/


// Chk socket alive => 새벽에 작업되게 한다.
// 비정상 연결 소켓 대해서 전송해보고, 만약 연결이 끊긴 소켓이면 
// 정리되게 한다.

void	CThreadSockAsyncServer::ChkSockAlive()
{	
	CAutoLock ScopeLock ( m_ScopeCS, "ChkSockAlive");

	/*
	CString key;		
	POSITION pos = NULL; 
	CIpSockObjMapInfo* p = NULL;

	for( pos = mSockIdSockObjMap.GetStartPosition(); pos != NULL; )
	{
		mSockIdSockObjMap.GetNextAssoc( pos, key, (CObject*&) p );
	}
    */
	
	POSITION pos = mSockIdSockObjMap.GetStartPosition();
	CString string;		
	
	while( pos != NULL )
	{		
		CIpSockObjMapInfo* p = NULL;
		
		mSockIdSockObjMap.GetNextAssoc( pos, string, (CObject*&) p );
		
		if(p)
		{				
			CIpSockObjMapInfo* pObj = NULL;
					    
			if ( mIDIPMap.Lookup( p->m_pSockObj->m_szUserId, ( CObject*& ) pObj )) 
			{
				// 사용자가 접속중  

				if( pObj && pObj->m_pSockObj->s != p->m_pSockObj->s )
				{
					/* 접속된 사용자의 소켓과 틀린경우 */

					// 비정상 종료 소켓						
					S_COMMON_HEADER sMSGHeader;
					
					int iLength = 0;	
					int nTotalLen = 0;	
					char* pSzBuf = NULL;								
					char szTemp[20];	
					
					memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
					
					iLength = 0;	// 그냥 헤더만 ... 
					
					// 헤더부 설정			
					memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
					memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
					memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
					
					// 전송데이터 모으기
					pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
					memset(pSzBuf, NULL, sizeof(pSzBuf));		
					
					memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
					
					nTotalLen = COMMON_HEADER_LENGTH + iLength; 
					
					CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "Ln [%d] 비정상 socket 확인 [%d] [%s] [%s]\n",
						__LINE__  , p->m_pSockObj->s , p->m_pSockObj->m_szIP ,p->m_pSockObj->m_szUserId );
					
					int nRtn = SendDataToClient("doWork_ALIVE", "", p, pSzBuf, nTotalLen) ;
					
					if(nRtn == -1 )
					{
						CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] doWork_ALIVE SendDataToClient\n",
							__LINE__  );
						
						//return FALSE;
					}
				}
			}
			else
			{
				//이미 사용자가 접속끊은 경우 && mSockIdSockObjMap 에 존재시 비정상 소켓
				
				// 비정상 종료 소켓	
				S_COMMON_HEADER sMSGHeader;
					
				int iLength = 0;	
				int nTotalLen = 0;	
				char* pSzBuf = NULL;								
				char szTemp[20];	
				
				memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
				
				iLength = 0;	// 그냥 헤더만 ... 
				
				// 헤더부 설정			
				memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
				// 전송데이터 모으기
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));		
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //공통 헤더		
				
				nTotalLen = COMMON_HEADER_LENGTH + iLength; 
				
				CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "Ln [%d] 비정상 socket 확인 [%d] [%s] [%s]\n",
					__LINE__  , p->m_pSockObj->s , p->m_pSockObj->m_szIP ,p->m_pSockObj->m_szUserId );
				
				int nRtn = SendDataToClient("doWork_ALIVE", "", p, pSzBuf, nTotalLen) ;
				
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] doWork_ALIVE SendDataToClient\n",
						__LINE__  );
					
					//return FALSE;
				}				
			}
		}		
	}		
}

int	CThreadSockAsyncServer::AlertPendingWork()
{
	// 적기 수행율 관련 메시지 보내기 
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));
	
	// 1. REQUIRE_PERSON_ID 
	// 2. REQUIRE_PERSON 
	// 3. WORK_ID 
	// 4. WORK_NAME 
	// 5. REQUIRE_NUM 
	// 6. REQUIRE_NAME	
	// 7. REQUIRE_DATE 
	// 8. END_DUE_DATE
	
	CTime ctDaily = CTime::GetCurrentTime();

	sprintf(szSql, "SELECT DISTINCT REQUIRE_PERSON_ID, REQUIRE_PERSON, WORK_ID, WORK_NAME, REQUIRE_NUM, REQUIRE_NAME,	\
REQUIRE_DATE, END_DUE_DATE	FROM VIEW_ICIS_TASK WHERE END_DUE_DATE BETWEEN  '%s'  AND '%s'	\
AND COMPLETED_DATE IS NULL AND  WORK_ID IS NOT NULL AND REQ_STATUS_CD NOT IN ('05','07','08') ORDER BY REQUIRE_NUM", ctDaily.Format("%Y-01-01"), ctDaily.Format("%Y-%m-%d")) ;
	
	TRACE("[%s]\n",szSql);
	
	//CLog::Instance().WriteFile(FALSE, "AlertPending", "SQL [%s]\n", szSql  );		
		
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
		
	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( ALERT_PENDING_WORK, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	
    
	return 0;
}











