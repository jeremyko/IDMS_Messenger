// DaimsThreadSockAsync.cpp: implementation of the CThreadSockAsync class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadSockAsync.h"

extern "C"
{
    #include <direct.h>
}
#include <process.h>
#include "..\\common\\utility.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static int gx_nID = 0;

#pragma comment(lib, "Ws2_32.lib")

IMPLEMENT_DYNCREATE(CThreadSockAsync, CObject)

//unsigned __stdcall PDaimsThreadProc( void* pParam )
UINT fThreadProc( LPVOID pParam )
{
	CThreadSockAsync* pOwner = static_cast<CThreadSockAsync*>(pParam);
	unsigned result = 0;	

	if (pOwner)
	{
		TRACE("pOwner\n");

		try
		{
			TRACE("pOwner->_run()\n");
			result = pOwner->_run(); 
			TRACE("pOwner->_run() return..\n");
		}
		catch(...)
		{			
			TRACE("EXCEPTION 발생!!!! CThreadSockAsync---->PWorkThreadProc \n");					
			
			result = -1;
			pOwner = NULL;

			exit(-1);
		}
	}

	
	if(pOwner)
	{
		pOwner->m_bDeadThreadFlag = TRUE;		
	}
	
	return result;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
CThreadSockAsync::CThreadSockAsync(void)
:	m_hSocket(INVALID_SOCKET),
	m_work_event(INVALID_HANDLE_VALUE),
	m_kill_event(INVALID_HANDLE_VALUE)
{
	if(gx_nID >= INT_MAX)
		gx_nID = 0;

	m_nClassID	   = ++gx_nID;			//현재 CThreadSockAsync클래스 생성갯수 --> 스레드 디버깅시 필요

	m_bThreadStart			  = FALSE;	//스레드 시작 여부 
	
	::InitializeCriticalSection(&m_cs);	
	m_pThread = NULL;
	m_pBuff = NULL; /* 20071203 */
	m_iBuffLength = 0; /* 20071203 */
	
}


//! 소멸자
CThreadSockAsync::~CThreadSockAsync(void)
{
//	m_bufs.clear();

	SBUFDATA data ;
	data.nLen = 0;
	data.pData = NULL;
	
	::EnterCriticalSection(&m_cs);
	while (0 != m_bufs.size())
	{
		data = (SBUFDATA)m_bufs.front();
		m_bufs.pop_front();
		delete data.pData;
	}
	::LeaveCriticalSection(&m_cs);

	_shutdown();
	
}

CString	CThreadSockAsync::GetCurrentTimeByString(BOOL bOnlyTime)
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

// 실질적인 Create부분, 소켓을 생성하여 Connect해 준다.
BOOL CThreadSockAsync::SocketConnect(UINT nSocketPort, LPCTSTR lpSocketAddress, BOOL bStartThread)  
{		
	struct sockaddr_in addr;			
	struct	hostent *hent;	
	long	laddr;
	LPIN_ADDR paddr;
	IN_ADDR       addrHost;

	m_iBuffLength = 0;
		
	_init();	
		
	paddr = &addrHost;
	laddr = ::inet_addr( lpSocketAddress );
	if( laddr == INADDR_NONE ) 
	{ 
		if ( !(hent = ::gethostbyname(lpSocketAddress)) ) 
		{
			TRACE("Invalid server hostname( %s ), error [%d]\n", lpSocketAddress, GetLastError() );
			//Log
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "Invalid server hostname [%s] [%s] %d\n" , iDT.Format("%Y%m%d%H%M%S") , lpSocketAddress, GetLastError()  );
			
			return FALSE;
		}
		memcpy( paddr, hent->h_addr, sizeof(IN_ADDR) );
	} 
	else 
	{
		memcpy( &paddr->s_addr, &laddr, 4);
	}

	if(m_hSocket)
	{
		closesocket( m_hSocket );
		m_hSocket = NULL;
	}
		
    if ((m_hSocket = socket(PF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
    {
		TRACE("★ socket 생성에러 [%d]\n", GetLastError() );
        //Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "socket 생성에러 [%s] %d\n" , iDT.Format("%Y%m%d%H%M%S") , GetLastError()  );
		
        return FALSE;
    }
    
    ZeroMemory((char*) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(lpSocketAddress);
    addr.sin_port = htons((u_short)nSocketPort);	

	//20080926 nagel
	//int nValue = 1; 
	//DWORD optval = SO_SEC_SSL;
	//setsockopt( m_hSocket,IPPROTO_TCP,TCP_NODELAY, &nValue, sizeof(int)); 

    //20080926 linger 
	struct linger opt_ling = { 1, 0 }; // l_onoff, l_linger;
	setsockopt(m_hSocket, SOL_SOCKET, SO_LINGER, (const char*)&opt_ling, sizeof(opt_ling));

    u_long    arg = 1;
    ioctlsocket(m_hSocket, FIONBIO, &arg); // non-blocking mode로 설정	
	struct  timeval     timevalue;	
	fd_set writefds;	

	
	int nRtn = connect(m_hSocket, (struct sockaddr*) &addr, sizeof(addr));
	
	if(	nRtn == SOCKET_ERROR && WSAEWOULDBLOCK != WSAGetLastError() )
	{
		if(	WSAEALREADY == WSAGetLastError() )
		{
			// 이미 연결 시도중 재시도 	
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "connect 실패 error 이미 연결 시도중 재시도 [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );
			
			closesocket( m_hSocket );		
			m_hSocket = NULL;
			return FALSE;
		}
		else
		{
			//Log
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "connect 실패 error [%s] [%d]\n" , iDT.Format("%Y%m%d%H%M%S") ,WSAGetLastError()  );
			
			closesocket( m_hSocket );		
			m_hSocket = NULL;
			return FALSE;
		}		
	}	

	FD_ZERO( &writefds );
	FD_SET( m_hSocket, &writefds );
	timevalue.tv_sec = 0;
	timevalue.tv_usec = 500000; // 0.5 sec 

	nRtn = ::select( 0, NULL, &writefds, NULL, &timevalue );

	if(SOCKET_ERROR == nRtn)
	{
		//Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "connect select error [%s] [%d]\n" , iDT.Format("%Y%m%d%H%M%S") ,WSAGetLastError()  );
		
		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return FALSE;
		
	}

	if ( !FD_ISSET( m_hSocket, &writefds ) )
	{		
		//Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "connect 실패 [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );

		TRACE("★ connect 실패 [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);

		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return FALSE;
	}	
	
	TRACE("★ connect 성공 [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
	
	//Log	
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "connect 성공 [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );
	
	if(bStartThread)
	{
		TRACE("CThreadSock / 연결에 성공되었을때만 스레드 생성\n");
		Start();
	}

	return TRUE;
}




int CThreadSockAsync::Send(char* lpBuf,int iBufLen, const char* szUsage) 
{	
	if(!mbConnected) // 서버연결이 끊긴 경우는 send못함.
	{
		//LogReconn
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "서버연결이 끊긴 경우는 send못함 [%s] [%s]\n" , iDT.Format("%Y%m%d%H%M%S") ,szUsage );
		
		TRACE("CChatSession::Send : 서버연결이 끊긴 경우는 send못함!\n");	

		delete lpBuf;
		return  -1 ;
	}
	
	_put_buffer(lpBuf, iBufLen);			//전송할 데이터 큐에 넣고		
	::SetEvent(m_work_event);		
	
	TRACE("CChatSession::Send !\n");	

	return 0;
}



void CThreadSockAsync::Close()
{		
	//스레드가 종료될때까지 기다린다!
	Stop(TRUE);
}


void CThreadSockAsync::CloseForTerminate()
{
	m_bRunExitFlag = TRUE;
	::shutdown(m_hSocket, SD_BOTH) ; //CloseForTerminate()
	::closesocket(m_hSocket); m_hSocket = INVALID_SOCKET;	//CloseForTerminate()	

	//스레드가 종료될때까지 기다린다!
	Stop(TRUE);
}




//BOOL CThreadSockAsync::IsConnected()
//{
//	return m_bConnect;
//}


//! 기동
int	CThreadSockAsync::Start()
{
	TRACE("CThreadSockAsync::Start()\n");
	if (m_hSocket == INVALID_SOCKET)
	{
		TRACE(_T("CThreadPushWorkAsync::Start() : m_hSocket == INVALID_SOCKET failure \n"));
		return -1;
	}
	
	//m_thread = (HANDLE)::_beginthreadex(0, 0, PDaimsThreadProc, (void*)this, 0, 0);
	m_pThread = AfxBeginThread(fThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if(m_pThread == NULL)
	{
		TRACE(_T("CThreadPushWorkAsync::Start() : _beginthreadex failure \n"));
		
		return -1;
	}

	// 스레드 가동
	m_pThread->ResumeThread();
	
	m_bThreadStart = TRUE;

	TRACE("서버 수신 스레드 시작 [ID : %d]\n", m_nClassID);
	return 0;
}



//! 쓰레드를 종료시킨다.
//! 제대로 쓰레드가 종료할때까지 기다린다.

int	CThreadSockAsync::Stop(BOOL wait)
{
	TRACE("CThreadSock::Stop(%d) 소켓 번호[%d]\n", wait, m_hSocket );	
	
	std :: deque<SBUFDATA> ::iterator it;
	for (it= m_bufs.begin(); it != m_bufs.end(); ++it)
	{
		SBUFDATA data ;
		data.nLen = 0;
		data.pData = NULL;		
		if (0 != m_bufs.size())
		{
			data = (SBUFDATA)m_bufs.front();
			if(data.pData)
			{
				delete[] data.pData;
				data.pData = NULL;
			}
			m_bufs.pop_front();
		}
	}
	
	_clear_buffer();

	::SetEvent(m_kill_event);
	
	TRACE("CThreadSock::Stop 소켓 Close[%d]\n", m_hSocket );	
	::shutdown(m_hSocket, SD_BOTH) ; // Stop
	::closesocket(m_hSocket); m_hSocket = INVALID_SOCKET;	// Stop

	
	if (m_bThreadStart)	
	{
		if (wait)
		{
			if(	m_bDeadThreadFlag || m_pThread == NULL || m_pThread->m_hThread == NULL || 
				(DWORD)m_pThread->m_hThread == 0xfeeefeee || 
				m_pThread->m_hThread == INVALID_HANDLE_VALUE )
			{				
				TRACE("서버 수신 스레드 [자동] 종료 완료 \n" );				
				
				return 0;
			}

			// 스레드가 자고있다면 스레드를 깨운다!!
			DWORD dwResult;
			int nCount = 0;
			while (dwResult = m_pThread->ResumeThread() > 1)
			{
				if (dwResult == 0xFFFFFFFF)
					break;
				else
				{
					if(m_pThread->m_hThread == NULL || (DWORD)m_pThread->m_hThread == 0xfeeefeee)
						break;
				}

				// 무한 루프에 빠지지않기위해
				if(nCount > 1000)
					break;
				nCount++;
			}
			
			DWORD result = ::WaitForSingleObject(m_pThread->m_hThread, MAX_WAIT_TIME);			
			
			m_bThreadStart = FALSE;
			if(dwResult == WAIT_OBJECT_0 || (DWORD)m_pThread == 0xfeeefeee)
			{
				TRACE("수신 스레드 [안전하게] 종료 완료  [ID : %d]\n", m_nClassID );
				return 0;
			}
			else if(dwResult == WAIT_TIMEOUT)
			{
				if(m_hSocket != INVALID_SOCKET)
				{
					::shutdown(m_hSocket, SD_BOTH) ; //Stop
					::closesocket(m_hSocket); m_hSocket = INVALID_SOCKET; //Stop
				}

				DWORD dwExitCode;
				::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);  
				::TerminateThread(m_pThread->m_hThread, dwExitCode);
				TRACE("수신 스레드 [강제로] 종료 완료  [ID : %d]\n", m_nClassID );

				if(m_pThread != NULL || (DWORD)m_pThread != 0xfeeefeee)
				{
					delete m_pThread;
					m_pThread = NULL;
				}
				return -1;
			}
		}		
		m_bThreadStart = FALSE;
	}
	return 0;	
}




/*!
    크리티걸 섹션 초기화 및 이벤트 객체 초기화 
	초기화.
*/
int	CThreadSockAsync::_init()
{
	TRACE("CThreadSockAsync::_init()\n");
	_shutdown();
	
	m_work_event = ::CreateEvent(0, TRUE, FALSE, 0);
	m_kill_event = ::CreateEvent(0, TRUE, FALSE, 0);
	return 0;
}


//! 쓰레드 종료시 작업
int	CThreadSockAsync::_shutdown()
{
	if(m_work_event != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_work_event); 
		m_work_event = INVALID_HANDLE_VALUE;
	}

	if(m_kill_event != INVALID_HANDLE_VALUE)
	{
		if(m_kill_event)
		{
			::CloseHandle(m_kill_event); 
			m_kill_event = INVALID_HANDLE_VALUE;
		}		
	}
	return 0;
}



unsigned	CThreadSockAsync::_run()
{
	::ResetEvent(m_kill_event);
	::ResetEvent(m_work_event);
	
	HANDLE sock_event = ::WSACreateEvent();

	//::ResetEvent(sock_event); //TEST
	
	// WSAEventSelect function automatically sets socket s to nonblocking mode...
	DWORD result = ::WSAEventSelect(m_hSocket, sock_event, FD_READ|FD_WRITE|FD_CLOSE);

	if(result == SOCKET_ERROR)
	{
		if( WSAEWOULDBLOCK != WSAGetLastError())
		{
			TRACE("\n▶▶▶ ERROR : WSAEventSelect return SOCKET_ERROR[%d] [클래스 ID : %d]\n", WSAGetLastError(), m_nClassID);	
			return -1;		
		}		
	}

	HANDLE h[] = { m_kill_event, m_work_event, sock_event };

	m_bRunExitFlag = FALSE;
	BOOL bDisconnected = FALSE;

	//! 배열 h의 객체들이 signaled 되기를 기다린다.
	while (!m_bRunExitFlag)
	{		
		result = ::WaitForMultipleObjects(3, h, FALSE, INFINITE);
		switch (result-WAIT_OBJECT_0)
		{
		case 0:	//! kill this thread
			{
				::shutdown(m_hSocket, SD_BOTH) ;
				::closesocket(m_hSocket); m_hSocket = INVALID_SOCKET;				
				m_bRunExitFlag = TRUE;
				bDisconnected = TRUE;
			}
			break;
						
		case 1:	//! send
			{
				_do_send();				
				TRACEX("★★ _do_send() #1소켓 번호[%d]\n", m_hSocket );				
			}
			break;
			
		case 2:	//! socket event		
			{
				WSANETWORKEVENTS events;
				result = ::WSAEnumNetworkEvents(m_hSocket, sock_event, &events);
				if (SOCKET_ERROR == result)
				{
					int nErr = WSAGetLastError();

					//Log 20070528
//					CTime iDT =  CTime::GetCurrentTime();					
//					char szTmpPath[100];
//					memset(szTmpPath, 0x00, sizeof(szTmpPath));
//					sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//					writeLogFile(szTmpPath, "[%s] WSAEnumNetworkEvents Error [%d] \n" , iDT.Format("%Y%m%d%H%M%S") , nErr ); 


					TRACE(" ★★ CThreadSock WAIT_OBJECT_0 + 2 , SOCKET_ERROR == result \n");
										
					if(WSANOTINITIALISED == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSANOTINITIALISED\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSANOTINITIALISED\n");	
					}
					if(WSAENETDOWN == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSAENETDOWN\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSAENETDOWN\n");	
					}
					if(WSAEINVAL == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSAEINVAL\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSAEINVAL\n");	
					}
					if(WSAEINPROGRESS == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSAEINPROGRESS\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSAEINPROGRESS\n");	
					}
					if(WSAENOTSOCK == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSAENOTSOCK\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSAENOTSOCK\n");	
					}
					if(WSAEFAULT == nErr)
					{
						//writeLogFile(szTmpPath, "[%s] WSAEFAULT\n" , iDT.Format("%Y%m%d%H%M%S")); 
						TRACE("ERROR : WSAEFAULT\n");	
					}
					

					m_bRunExitFlag = TRUE;
					bDisconnected = TRUE;
				}
				else
				{
					if (events.lNetworkEvents & FD_READ)
					{
						//TRACE("CThreadSock::_do_recv 소켓 번호[%d]\n", m_hSocket);
						
						_do_recv();
					}					
					else if (events.lNetworkEvents & FD_WRITE)
					{
						TRACEX("★★ FD_WRITE _do_send()\n");				
						_do_send();						
					}					
					else if (events.lNetworkEvents & FD_CLOSE)
					{
						TRACE("CThreadSock::소켓 연결 종료 이벤트 소켓 번호[%d]\n", m_hSocket );
						
						//::shutdown(m_hSocket, SD_BOTH) ;	// FD_CLOSE					
						::closesocket(m_hSocket);  // FD_CLOSE
						m_hSocket = INVALID_SOCKET;		
						m_bRunExitFlag = TRUE;
						bDisconnected = TRUE;
						
						break;
					}
					else
					{
						TRACE("▶▶▶ (CThreadSock) ERROR 알려지지않은 소켓 이벤트 발생\n");	
					}
				}
			}
			break;		
		default:
			{
				TRACE("CThreadSock::소켓 연결 종료 이벤트 소켓 번호[%d]\n", m_hSocket );
				m_bRunExitFlag = TRUE;
				bDisconnected = TRUE;
			}
			break;
		}//switch
	}//while
	
	TRACEX("★ CThreadSock::thread loop Exit! 소켓 번호[%d]\n", m_hSocket );
	
	
	if(bDisconnected)
	{
		TRACEX("★ ProcessWhenConnClosed 호출!\n");
		ProcessWhenConnClosed();
		TRACEX("★ ProcessWhenConnClosed 호출후\n");
	}
	
	
	return 0;
}

// 헤더 정보를 포함하는 전체 PAcket 길이 
int CThreadSockAsync::GetOnePacketLength(char *pData, int iLength)
{
	if(iLength < COMMON_HEADER_LENGTH)
		return -1;

	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)pData;

	int nPacketLen = atoi(psHeader->m_szLength) + COMMON_HEADER_LENGTH ; 
	TRACE("전체 PAcket 길이 [%d]\n", nPacketLen);
	return nPacketLen;
}

int	CThreadSockAsync::_do_recv()
{	
	char szTemp[MAX_BUFF_SIZE];
	int receive_length;	
	
	memset(szTemp,0x00,sizeof(szTemp));

	if((receive_length = recv(m_hSocket, (char *)szTemp, MAX_BUFF_SIZE, 0)) > 0) // Receive  
	{   
		//TRACE("★ CThreadSockAsync::recv [%d] bytes\n", receive_length );

		char *pTemp = m_pBuff; // 이전 데이터 copy..

		// 이전 데이터 + 지금 받은 데이터만큼 메모리 버퍼를 할당한다.

		m_pBuff = new char[m_iBuffLength + receive_length+1]; // 1 -> null
		memset(m_pBuff, 0x00, m_iBuffLength + receive_length +1);
		
		if(m_pBuff == NULL)
		{
			TRACE("if(m_pBuff == NULL)  [%d][%s]\n", m_nClassID );
			//break;
			return 0;
		}

		if(m_iBuffLength != 0)
		{				
			// 새로 할당된 버퍼에 지금까지 받은 데이터 다시 저장..
			memcpy(m_pBuff, pTemp, m_iBuffLength);

			if(pTemp)
			{
				// pTemp 는 다시 초기화 상태로 설정,,
				delete[] pTemp;
				pTemp = NULL;
			}
		}

		// 새로 받은 데이터 복사
		memcpy(m_pBuff + m_iBuffLength, szTemp, receive_length);

		// 받은 데이터 사이즈 증가 기록..
		m_iBuffLength = m_iBuffLength + receive_length;

		//while(1)
		//{
		// 패킷의 전체 데이터를 구한다..
		int nOnePacketLength = GetOnePacketLength(m_pBuff, m_iBuffLength);

		if(nOnePacketLength < 0)
		{
			TRACE("CThreadSock::_do_recv() iOnePacketLength < 0 ..Break\n");
			//break;
			return 0;
		}
		// 아직 한 Packet의 데이터가 오질 않았다면 다음 데이터를 기다린다.
		else if(nOnePacketLength > m_iBuffLength)
		{
			TRACE("아직 한 Packet의 데이터가 오질 않았다\n");
			//break;
			return 0;
		}
		// 한 Packet을 구성할 데이터가 도착하였다면 그 길이만큼을 처리
		else
		{
			//TRACE("\n▷▷▷ 한 Packet을 구성할 데이터가 도착 (수신데이터 처리시작)\n" );

			// 현재 받은 데이터 - 하나의 패킷 데이터 CHK
			int nExcessiveLen = m_iBuffLength - nOnePacketLength;

			// 이함수에서 데이터 처리후 delete 시키면 안된다.. 
			ProcessRecvData(m_pBuff, nOnePacketLength);

			// 정해진 패킷 데이터 보다 더많은 데이터 수신의 경우 -> 버퍼를 다시 구성
			if(nExcessiveLen > 0)
			{				
				TRACEX("정해진 패킷 데이터 보다 더많은 데이터 수신의 경우 버퍼를 다시 구성\n");				
				pTemp = m_pBuff;
				TRACE("CThreadSock::_do_recv() m_pBuff = new char[%d]\n", nExcessiveLen);
				
				m_pBuff = new char[nExcessiveLen];				
				
				if(m_pBuff == NULL)
				{				
					return 0;
				}

				// 남은 데이터 복사, pTemp는 아직 한패킷 + 여분의 데이터 상태..				
				memcpy(m_pBuff, pTemp + nOnePacketLength , nExcessiveLen);				
				m_iBuffLength = nExcessiveLen;
								
				if(pTemp)
				{					
					delete[] pTemp;
					pTemp = NULL;
				}

				// 더받은 데이터를 처리한다. 
				// 버퍼가 0 or 0보다작음(아직 다 못받음) 이 될때까지..
				while(nExcessiveLen) 
				{															
					nOnePacketLength = GetOnePacketLength(m_pBuff, m_iBuffLength);					

					TRACEX("[%d] [%d]\n", nExcessiveLen, nOnePacketLength);				
					
					if(nOnePacketLength < 0)
					{
						TRACE("CThreadSock::_do_recv() iOnePacketLength < 0 ..Break\n");						
						return 0;
					}					
					else if(nOnePacketLength > m_iBuffLength)
					{
						TRACE("아직 한 Packet의 데이터가 오질 않았다\n");						
						return 0;
					} 
					else
					{	
						ProcessRecvData(m_pBuff, nOnePacketLength);
					}
					 
					nExcessiveLen = nExcessiveLen - nOnePacketLength ;

					if(nExcessiveLen == 0)
					{
						// 더받았고 패킷용량에 맞게 다 전송된 경우 
						if(m_pBuff)
						{					
							delete[] m_pBuff;
							m_pBuff = NULL;
						}				
						m_iBuffLength = 0;			

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
						memcpy(m_pBuff, m_pBuff + nOnePacketLength , nExcessiveLen);
					}
				}
			}
			else
			{
				if(m_pBuff)
				{
					//TRACE("CThreadSock::_do_recv() delete m_pBuff \n" );
					delete[] m_pBuff;
					m_pBuff = NULL;
				}

				m_iBuffLength = 0;
			}
		}
	}
	else if( receive_length == SOCKET_ERROR )
	{
		int nRet = GetLastError();
		
		if(nRet == WSAEWOULDBLOCK)
		{				
			TRACE("CThreadSock SOCKET_ERROR, WSAEWOULDBLOCK\n");
			// No Error				
			if(m_pBuff)
			{
				//TRACE("CThreadSock SOCKET_ERROR, WSAEWOULDBLOCK ..just continue\n");
				//Sleep(1);
				//continue;
			}
			else
			{				
				return 0;
			}
		}
		else
		{
			TRACE("\n▶▶▶ ERROR : 수신 에러 [%d] --> 소켓 번호 : %d [클래스 ID : %d]\n \n", nRet, m_hSocket, m_nClassID );
			//break;			
			return 0;
		}
	}
	else if( receive_length == 0 ) 
	{			
		TRACE("\n▶▶▶ receive_length == 0  /break 소켓 번호 : %d [클래스 ID : %d]\n", m_hSocket, m_nClassID );
		int nRet = GetLastError();
		//break;
		return 0;
	}
	
	
	return 1;
}



int	CThreadSockAsync::_do_send()
{
	char buffer[MAX_BUFF_SIZE];
	memset(buffer,0,sizeof(buffer));	
	
	SBUFDATA data = (SBUFDATA) _get_buffer();	
	TRACEX("data [%d]\n", data.nLen);
	
	if(data.nLen == 0) //20070803 풀었다
	{

//		CTime iDT =  CTime::GetCurrentTime();
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "[%s] data.nLen == 0]\n" , iDT.Format("%Y%m%d%H%M%S"));

		return 0;
	}

	int result = 0;
	
	//Log 20070528
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "[%s] _do_send [%d] \n" , iDT.Format("%Y%m%d%H%M%S") , data.nLen ); 

	// send 호출!!
	//plugware::CAutoLock LockCriticlaSection(m_criSecLocalScopeSend,"_do_send"); //20070618 , 20070803 막음 

	result = ::send(m_hSocket, data.pData, data.nLen, 0);		

	if (SOCKET_ERROR == result)	
	{
		if (WSAEWOULDBLOCK != WSAGetLastError())
		{			
			TRACE("CThreadSock::_do_send : send error[%d]",WSAGetLastError());				
			
			CTime iDT =  CTime::GetCurrentTime();
			char szTmpPath[100];
			memset(szTmpPath, 0x00, sizeof(szTmpPath));
			sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
			writeLogFile(szTmpPath, "[%s] send Error [%d] \n" , iDT.Format("%Y%m%d%H%M%S") , WSAGetLastError() ); 			

			result = -1;
		}
		else
		{
			//전송할 데이터 다시 제자리에...
			TRACE("☆ WSAEWOULDBLOCK 에러시 전송할 데이터 다시 제자리에...\n");
			
			CTime iDT =  CTime::GetCurrentTime();
			char szTmpPath[100];
			memset(szTmpPath, 0x00, sizeof(szTmpPath));
			sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
			writeLogFile(szTmpPath, "WSAEWOULDBLOCK 에러시 전송할 데이터 다시 제자리에 [%s]\n" , iDT.Format("%Y%m%d%H%M%S"));

			_unget_buffer(data);
		}
	}
	else
	{
		if (result != data.nLen)
		{
			//만약 모두다 전송하지 못했다면 
			//전송할 데이터 다시 제자리에...
			_unget_buffer(data);
				
			TRACEX("모두 send 하지못했다, 전송 재시도.\n");
			::SetEvent(m_work_event);
			
			CTime iDT =  CTime::GetCurrentTime();					
			char szTmpPath[100];
			memset(szTmpPath, 0x00, sizeof(szTmpPath));
			sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
			writeLogFile(szTmpPath, "모두 send 하지못했다, 전송 재시도 [%s]\n" , iDT.Format("%Y%m%d%H%M%S"));
		}

		TRACEX("모두 send \n");
	}
	
	delete[] data.pData;
	data.pData = NULL;
	::ResetEvent(m_work_event);

	return 0;
}



//////////////////////////////////////////////////////////////////////////////////
//							전송할 데이터 관리 --> 큐							
//////////////////////////////////////////////////////////////////////////////////

void CThreadSockAsync::_put_buffer(char* data, int nLen)
{
	::EnterCriticalSection(&m_cs);
	SBUFDATA sbuf;
	sbuf.nLen = nLen;
	sbuf.pData = data;

	//m_bufs.push_back(data);	//! 큐 마지막에 추가,
	m_bufs.push_back(sbuf);
	
	::LeaveCriticalSection(&m_cs);	
}


void CThreadSockAsync::_clear_buffer()
{
	m_bufs.clear();
}


//! send 에 실패한 경우에는 재전송을 위해 다시 큐에 넣음(큐 제일 앞쪽).
void CThreadSockAsync::_unget_buffer(SBUFDATA data)
{
	::EnterCriticalSection(&m_cs);
	m_bufs.push_front(data);
	::LeaveCriticalSection(&m_cs);
}


//! 큐에서 제일 앞쪽의 데이터를 얻음
SBUFDATA CThreadSockAsync::_get_buffer()
{
	SBUFDATA data ;
	data.nLen = 0;
	data.pData = NULL;
	::EnterCriticalSection(&m_cs);
	if (0 != m_bufs.size())
	{
		data = (SBUFDATA)m_bufs.front();
		m_bufs.pop_front();
	}
	::LeaveCriticalSection(&m_cs);
	return data;
}