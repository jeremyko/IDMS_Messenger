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
			TRACE("EXCEPTION �߻�!!!! CThreadSockAsync---->PWorkThreadProc \n");					
			
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

	m_nClassID	   = ++gx_nID;			//���� CThreadSockAsyncŬ���� �������� --> ������ ������ �ʿ�

	m_bThreadStart			  = FALSE;	//������ ���� ���� 
	
	::InitializeCriticalSection(&m_cs);	
	m_pThread = NULL;
	m_pBuff = NULL; /* 20071203 */
	m_iBuffLength = 0; /* 20071203 */
	
}


//! �Ҹ���
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

// �������� Create�κ�, ������ �����Ͽ� Connect�� �ش�.
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
		TRACE("�� socket �������� [%d]\n", GetLastError() );
        //Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "socket �������� [%s] %d\n" , iDT.Format("%Y%m%d%H%M%S") , GetLastError()  );
		
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
    ioctlsocket(m_hSocket, FIONBIO, &arg); // non-blocking mode�� ����	
	struct  timeval     timevalue;	
	fd_set writefds;	

	
	int nRtn = connect(m_hSocket, (struct sockaddr*) &addr, sizeof(addr));
	
	if(	nRtn == SOCKET_ERROR && WSAEWOULDBLOCK != WSAGetLastError() )
	{
		if(	WSAEALREADY == WSAGetLastError() )
		{
			// �̹� ���� �õ��� ��õ� 	
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "connect ���� error �̹� ���� �õ��� ��õ� [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );
			
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
//			writeLogFile(szTmpPath, "connect ���� error [%s] [%d]\n" , iDT.Format("%Y%m%d%H%M%S") ,WSAGetLastError()  );
			
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
//		writeLogFile(szTmpPath, "connect ���� [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );

		TRACE("�� connect ���� [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);

		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return FALSE;
	}	
	
	TRACE("�� connect ���� [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
	
	//Log	
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "connect ���� [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );
	
	if(bStartThread)
	{
		TRACE("CThreadSock / ���ῡ �����Ǿ������� ������ ����\n");
		Start();
	}

	return TRUE;
}




int CThreadSockAsync::Send(char* lpBuf,int iBufLen, const char* szUsage) 
{	
	if(!mbConnected) // ���������� ���� ���� send����.
	{
		//LogReconn
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "���������� ���� ���� send���� [%s] [%s]\n" , iDT.Format("%Y%m%d%H%M%S") ,szUsage );
		
		TRACE("CChatSession::Send : ���������� ���� ���� send����!\n");	

		delete lpBuf;
		return  -1 ;
	}
	
	_put_buffer(lpBuf, iBufLen);			//������ ������ ť�� �ְ�		
	::SetEvent(m_work_event);		
	
	TRACE("CChatSession::Send !\n");	

	return 0;
}



void CThreadSockAsync::Close()
{		
	//�����尡 ����ɶ����� ��ٸ���!
	Stop(TRUE);
}


void CThreadSockAsync::CloseForTerminate()
{
	m_bRunExitFlag = TRUE;
	::shutdown(m_hSocket, SD_BOTH) ; //CloseForTerminate()
	::closesocket(m_hSocket); m_hSocket = INVALID_SOCKET;	//CloseForTerminate()	

	//�����尡 ����ɶ����� ��ٸ���!
	Stop(TRUE);
}




//BOOL CThreadSockAsync::IsConnected()
//{
//	return m_bConnect;
//}


//! �⵿
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

	// ������ ����
	m_pThread->ResumeThread();
	
	m_bThreadStart = TRUE;

	TRACE("���� ���� ������ ���� [ID : %d]\n", m_nClassID);
	return 0;
}



//! �����带 �����Ų��.
//! ����� �����尡 �����Ҷ����� ��ٸ���.

int	CThreadSockAsync::Stop(BOOL wait)
{
	TRACE("CThreadSock::Stop(%d) ���� ��ȣ[%d]\n", wait, m_hSocket );	
	
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
	
	TRACE("CThreadSock::Stop ���� Close[%d]\n", m_hSocket );	
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
				TRACE("���� ���� ������ [�ڵ�] ���� �Ϸ� \n" );				
				
				return 0;
			}

			// �����尡 �ڰ��ִٸ� �����带 �����!!
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

				// ���� ������ �������ʱ�����
				if(nCount > 1000)
					break;
				nCount++;
			}
			
			DWORD result = ::WaitForSingleObject(m_pThread->m_hThread, MAX_WAIT_TIME);			
			
			m_bThreadStart = FALSE;
			if(dwResult == WAIT_OBJECT_0 || (DWORD)m_pThread == 0xfeeefeee)
			{
				TRACE("���� ������ [�����ϰ�] ���� �Ϸ�  [ID : %d]\n", m_nClassID );
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
				TRACE("���� ������ [������] ���� �Ϸ�  [ID : %d]\n", m_nClassID );

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
    ũ��Ƽ�� ���� �ʱ�ȭ �� �̺�Ʈ ��ü �ʱ�ȭ 
	�ʱ�ȭ.
*/
int	CThreadSockAsync::_init()
{
	TRACE("CThreadSockAsync::_init()\n");
	_shutdown();
	
	m_work_event = ::CreateEvent(0, TRUE, FALSE, 0);
	m_kill_event = ::CreateEvent(0, TRUE, FALSE, 0);
	return 0;
}


//! ������ ����� �۾�
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
			TRACE("\n������ ERROR : WSAEventSelect return SOCKET_ERROR[%d] [Ŭ���� ID : %d]\n", WSAGetLastError(), m_nClassID);	
			return -1;		
		}		
	}

	HANDLE h[] = { m_kill_event, m_work_event, sock_event };

	m_bRunExitFlag = FALSE;
	BOOL bDisconnected = FALSE;

	//! �迭 h�� ��ü���� signaled �Ǳ⸦ ��ٸ���.
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
				TRACEX("�ڡ� _do_send() #1���� ��ȣ[%d]\n", m_hSocket );				
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


					TRACE(" �ڡ� CThreadSock WAIT_OBJECT_0 + 2 , SOCKET_ERROR == result \n");
										
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
						//TRACE("CThreadSock::_do_recv ���� ��ȣ[%d]\n", m_hSocket);
						
						_do_recv();
					}					
					else if (events.lNetworkEvents & FD_WRITE)
					{
						TRACEX("�ڡ� FD_WRITE _do_send()\n");				
						_do_send();						
					}					
					else if (events.lNetworkEvents & FD_CLOSE)
					{
						TRACE("CThreadSock::���� ���� ���� �̺�Ʈ ���� ��ȣ[%d]\n", m_hSocket );
						
						//::shutdown(m_hSocket, SD_BOTH) ;	// FD_CLOSE					
						::closesocket(m_hSocket);  // FD_CLOSE
						m_hSocket = INVALID_SOCKET;		
						m_bRunExitFlag = TRUE;
						bDisconnected = TRUE;
						
						break;
					}
					else
					{
						TRACE("������ (CThreadSock) ERROR �˷��������� ���� �̺�Ʈ �߻�\n");	
					}
				}
			}
			break;		
		default:
			{
				TRACE("CThreadSock::���� ���� ���� �̺�Ʈ ���� ��ȣ[%d]\n", m_hSocket );
				m_bRunExitFlag = TRUE;
				bDisconnected = TRUE;
			}
			break;
		}//switch
	}//while
	
	TRACEX("�� CThreadSock::thread loop Exit! ���� ��ȣ[%d]\n", m_hSocket );
	
	
	if(bDisconnected)
	{
		TRACEX("�� ProcessWhenConnClosed ȣ��!\n");
		ProcessWhenConnClosed();
		TRACEX("�� ProcessWhenConnClosed ȣ����\n");
	}
	
	
	return 0;
}

// ��� ������ �����ϴ� ��ü PAcket ���� 
int CThreadSockAsync::GetOnePacketLength(char *pData, int iLength)
{
	if(iLength < COMMON_HEADER_LENGTH)
		return -1;

	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)pData;

	int nPacketLen = atoi(psHeader->m_szLength) + COMMON_HEADER_LENGTH ; 
	TRACE("��ü PAcket ���� [%d]\n", nPacketLen);
	return nPacketLen;
}

int	CThreadSockAsync::_do_recv()
{	
	char szTemp[MAX_BUFF_SIZE];
	int receive_length;	
	
	memset(szTemp,0x00,sizeof(szTemp));

	if((receive_length = recv(m_hSocket, (char *)szTemp, MAX_BUFF_SIZE, 0)) > 0) // Receive  
	{   
		//TRACE("�� CThreadSockAsync::recv [%d] bytes\n", receive_length );

		char *pTemp = m_pBuff; // ���� ������ copy..

		// ���� ������ + ���� ���� �����͸�ŭ �޸� ���۸� �Ҵ��Ѵ�.

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
			// ���� �Ҵ�� ���ۿ� ���ݱ��� ���� ������ �ٽ� ����..
			memcpy(m_pBuff, pTemp, m_iBuffLength);

			if(pTemp)
			{
				// pTemp �� �ٽ� �ʱ�ȭ ���·� ����,,
				delete[] pTemp;
				pTemp = NULL;
			}
		}

		// ���� ���� ������ ����
		memcpy(m_pBuff + m_iBuffLength, szTemp, receive_length);

		// ���� ������ ������ ���� ���..
		m_iBuffLength = m_iBuffLength + receive_length;

		//while(1)
		//{
		// ��Ŷ�� ��ü �����͸� ���Ѵ�..
		int nOnePacketLength = GetOnePacketLength(m_pBuff, m_iBuffLength);

		if(nOnePacketLength < 0)
		{
			TRACE("CThreadSock::_do_recv() iOnePacketLength < 0 ..Break\n");
			//break;
			return 0;
		}
		// ���� �� Packet�� �����Ͱ� ���� �ʾҴٸ� ���� �����͸� ��ٸ���.
		else if(nOnePacketLength > m_iBuffLength)
		{
			TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");
			//break;
			return 0;
		}
		// �� Packet�� ������ �����Ͱ� �����Ͽ��ٸ� �� ���̸�ŭ�� ó��
		else
		{
			//TRACE("\n������ �� Packet�� ������ �����Ͱ� ���� (���ŵ����� ó������)\n" );

			// ���� ���� ������ - �ϳ��� ��Ŷ ������ CHK
			int nExcessiveLen = m_iBuffLength - nOnePacketLength;

			// ���Լ����� ������ ó���� delete ��Ű�� �ȵȴ�.. 
			ProcessRecvData(m_pBuff, nOnePacketLength);

			// ������ ��Ŷ ������ ���� ������ ������ ������ ��� -> ���۸� �ٽ� ����
			if(nExcessiveLen > 0)
			{				
				TRACEX("������ ��Ŷ ������ ���� ������ ������ ������ ��� ���۸� �ٽ� ����\n");				
				pTemp = m_pBuff;
				TRACE("CThreadSock::_do_recv() m_pBuff = new char[%d]\n", nExcessiveLen);
				
				m_pBuff = new char[nExcessiveLen];				
				
				if(m_pBuff == NULL)
				{				
					return 0;
				}

				// ���� ������ ����, pTemp�� ���� ����Ŷ + ������ ������ ����..				
				memcpy(m_pBuff, pTemp + nOnePacketLength , nExcessiveLen);				
				m_iBuffLength = nExcessiveLen;
								
				if(pTemp)
				{					
					delete[] pTemp;
					pTemp = NULL;
				}

				// ������ �����͸� ó���Ѵ�. 
				// ���۰� 0 or 0��������(���� �� ������) �� �ɶ�����..
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
						TRACE("���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n");						
						return 0;
					} 
					else
					{	
						ProcessRecvData(m_pBuff, nOnePacketLength);
					}
					 
					nExcessiveLen = nExcessiveLen - nOnePacketLength ;

					if(nExcessiveLen == 0)
					{
						// ���޾Ұ� ��Ŷ�뷮�� �°� �� ���۵� ��� 
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
						// ���� ����Ŷ�� �����Ͱ� �� ���۵��� �ʾҴ�.
						// => ���޾ƾ��ϹǷ� ���۸� �������� �ʴ´�. SKIP..
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
			TRACE("\n������ ERROR : ���� ���� [%d] --> ���� ��ȣ : %d [Ŭ���� ID : %d]\n \n", nRet, m_hSocket, m_nClassID );
			//break;			
			return 0;
		}
	}
	else if( receive_length == 0 ) 
	{			
		TRACE("\n������ receive_length == 0  /break ���� ��ȣ : %d [Ŭ���� ID : %d]\n", m_hSocket, m_nClassID );
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
	
	if(data.nLen == 0) //20070803 Ǯ����
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

	// send ȣ��!!
	//plugware::CAutoLock LockCriticlaSection(m_criSecLocalScopeSend,"_do_send"); //20070618 , 20070803 ���� 

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
			//������ ������ �ٽ� ���ڸ���...
			TRACE("�� WSAEWOULDBLOCK ������ ������ ������ �ٽ� ���ڸ���...\n");
			
			CTime iDT =  CTime::GetCurrentTime();
			char szTmpPath[100];
			memset(szTmpPath, 0x00, sizeof(szTmpPath));
			sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
			writeLogFile(szTmpPath, "WSAEWOULDBLOCK ������ ������ ������ �ٽ� ���ڸ��� [%s]\n" , iDT.Format("%Y%m%d%H%M%S"));

			_unget_buffer(data);
		}
	}
	else
	{
		if (result != data.nLen)
		{
			//���� ��δ� �������� ���ߴٸ� 
			//������ ������ �ٽ� ���ڸ���...
			_unget_buffer(data);
				
			TRACEX("��� send �������ߴ�, ���� ��õ�.\n");
			::SetEvent(m_work_event);
			
			CTime iDT =  CTime::GetCurrentTime();					
			char szTmpPath[100];
			memset(szTmpPath, 0x00, sizeof(szTmpPath));
			sprintf(szTmpPath, "c:\\Send%s.log", iDT.Format("%Y%m%d")  );	
			writeLogFile(szTmpPath, "��� send �������ߴ�, ���� ��õ� [%s]\n" , iDT.Format("%Y%m%d%H%M%S"));
		}

		TRACEX("��� send \n");
	}
	
	delete[] data.pData;
	data.pData = NULL;
	::ResetEvent(m_work_event);

	return 0;
}



//////////////////////////////////////////////////////////////////////////////////
//							������ ������ ���� --> ť							
//////////////////////////////////////////////////////////////////////////////////

void CThreadSockAsync::_put_buffer(char* data, int nLen)
{
	::EnterCriticalSection(&m_cs);
	SBUFDATA sbuf;
	sbuf.nLen = nLen;
	sbuf.pData = data;

	//m_bufs.push_back(data);	//! ť �������� �߰�,
	m_bufs.push_back(sbuf);
	
	::LeaveCriticalSection(&m_cs);	
}


void CThreadSockAsync::_clear_buffer()
{
	m_bufs.clear();
}


//! send �� ������ ��쿡�� �������� ���� �ٽ� ť�� ����(ť ���� ����).
void CThreadSockAsync::_unget_buffer(SBUFDATA data)
{
	::EnterCriticalSection(&m_cs);
	m_bufs.push_front(data);
	::LeaveCriticalSection(&m_cs);
}


//! ť���� ���� ������ �����͸� ����
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