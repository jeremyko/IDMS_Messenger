// iocp_test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

// IocpServer.cpp : Defines the entry point for the console application.
//
 
#include "stdafx.h"
#include "iocp_server.h"
#include "INIManager.h"
#include "thread_pool.h"
#include "log.h"
#include "MemoryPool.h"
#include "ServerGlobal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//! stlport4.6.2 ���� ������ �Ǿ���.
#define ULONG_PTR DWORD
//GLOBAL
SOCKET g_hListenSocket = NULL;
HANDLE g_hAcceptThread = NULL;
HANDLE g_hIocp = NULL;
DWORD g_dwWorkerThreadCnt = 0;
std::vector<HANDLE> g_vWorkerThreadHandle;

LONG g_lCount = 1000000;
BOOL g_bEnded = FALSE;
BOOL g_bStarted = FALSE;

//CRITICAL_SECTION g_cs;
//#pragma comment(lib,"ws2_32.lib")

#define _NONAGLE

long gCurrentConnections = 0;

/*
THREAD_OBJ *gChildThreads=NULL;     // thread objects ����Ʈ 
int  gChildThreadsCount=0;			// Number of child threads created
*/

CIocpServer* g_dlgPtr;

DWORD WINAPI AcceptThreadCallback(LPVOID parameter)
{
	while (TRUE)
	{
		SOCKADDR_IN	peer_addr;
		int         nPeerLen;

		nPeerLen = sizeof(peer_addr);
				
		SOCKET hSocket = WSAAccept(g_hListenSocket, (LPSOCKADDR)&peer_addr, &nPeerLen, NULL, 0);

#ifdef _NONAGLE
		int nValue = 1;
		setsockopt(hSocket, IPPROTO_TCP, TCP_NODELAY, (char*) &nValue, sizeof(int));
#endif

		if (!g_bStarted)
		{
			g_bStarted = TRUE;			
		}
				
		if (hSocket == INVALID_SOCKET)
		{			
			return 0;
		}
		else
		{
			//CIocpNetObj* pTmp = NULL ;
			CIocpNetObj *pObject = new CIocpNetObj();
			pObject->m_hSocket = hSocket;
			strcpy(pObject->m_szIP, ::inet_ntoa(((SOCKADDR_IN*)&peer_addr)->sin_addr));			
			
			CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln[%d] connected ip [%s]\n", __LINE__, pObject->m_szIP );

            									
			CString strSockID ;
			strSockID.Format("%d", hSocket) ;	
			
					
			InterlockedIncrement(&gCurrentConnections);

			g_hIocp = CreateIoCompletionPort((HANDLE) pObject->m_hSocket, g_hIocp, (ULONG_PTR) pObject, 0);
			if (!g_hIocp)
				return 0;

			int   nRet = 0;
			DWORD dwFlag = 0;
			DWORD dwNumOfByteRecved = 0;
			//BOOL  bRet = TRUE;

			WSABUF wsabuf;
			wsabuf.buf = pObject->m_szBuffer;
			wsabuf.len = MAX_BUFF_SIZE;
            
            // fire recv 
			nRet = WSARecv  (   hSocket, 
			                    &wsabuf, 
			                    1, 
			                    &dwNumOfByteRecved, 
				                &dwFlag,                 
                                &pObject->m_prolex->m_ol, 
				                NULL
				            );

			if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{				
				return 0;			
			}
			
			g_dlgPtr->mSockIdSockObjMap.SetAt(strSockID,(CObject*)pObject); 			
		}		
	}

	return 0;
}


DWORD WINAPI WorkerThreadCallback(LPVOID lpParam)
{
	BOOL bSucc = FALSE;
	DWORD dwNumOfByteTransfered = 0;
	ULONG_PTR pCompletionKey = NULL;
	OVERLAPPED *pol = NULL;
	
	while (TRUE)
	{
		bSucc = GetQueuedCompletionStatus
				(
			        g_hIocp,
			        &dwNumOfByteTransfered,
			        &pCompletionKey,
			        &pol,
			        INFINITE	);
		
		if (!pCompletionKey)
		{
			DWORD err = GetLastError ();
            TRACE("IocpServer WorkerThread return [%d]!!! \n", err);
			CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback GetQueuedCompletionStatus [%d]\n", 
									__LINE__ , err );
			return 0;
		}

		if (!pol)
		{			
            DWORD err = GetLastError ();
			TRACE("IocpServer WorkerThread return [%d]!!! \n", err);
			CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback GetQueuedCompletionStatus [%d]\n", 
									__LINE__ , err );
			return 0;
		}

		COverlapped *poOverlapped = (COverlapped*) pol;
		CIocpNetObj *poNetObj = (CIocpNetObj*) poOverlapped->m_pObject;

		// Disconnect
		if ((bSucc && dwNumOfByteTransfered == 0) || !bSucc)
		{						
			if (poOverlapped->m_io == IO_READ)
			{				
				TRACE("IocpServer WorkerThread ���� ���� \n");
				
				g_dlgPtr->InformWhenDisconnected(poNetObj);

				continue;
			}
			else if (poOverlapped->m_io == IO_WRITE)
			{
				TRACE("IocpServer WorkerThread ���� ���� \n");

				continue;
			}
			
			TRACE("IocpServer WorkerThread Disconnect \n");			
		}

		switch (poOverlapped->m_io)
		{
		case IO_READ:
			{
                TRACE ("IocpServer WorkerThread Read \n");

				INT   nRet = 0;
				DWORD dwFlag = 0;
				DWORD dwNumOfByte = 0;
				           
            	if(dwNumOfByteTransfered > 0 )
            	{   
            		char *pTemp = poNetObj->m_pTotalBuff; // ���� ������ copy..
            
            		// ���� ������ + ���� ���� �����͸�ŭ �޸� ���۸� �Ҵ��Ѵ�.		
            		poNetObj->m_pTotalBuff = new char[poNetObj->m_nTotalBufLength + dwNumOfByteTransfered+1];
            		memset(poNetObj->m_pTotalBuff, 0x00, poNetObj->m_nTotalBufLength + dwNumOfByteTransfered +1);
            		
            		if(poNetObj->m_pTotalBuff == NULL)
            		{            
						CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback �޸� �Ҵ� ����\n", 
									__LINE__  );
            			return 0;
            		}
            		
            		if(poNetObj->m_nTotalBufLength != 0)
            		{				
            			// ���� �Ҵ�� ���ۿ� ���ݱ��� ���� ������ �ٽ� ����..			
            			memcpy(poNetObj->m_pTotalBuff, pTemp, poNetObj->m_nTotalBufLength);
            
            			if(pTemp)
            			{
            				// pTemp �� �ٽ� �ʱ�ȭ ���·� ����,,
            				delete[] pTemp;
            				pTemp = NULL;
            			}
            		}
            
            		// ���� ���� ������ ����		
            		memcpy(poNetObj->m_pTotalBuff + poNetObj->m_nTotalBufLength, poNetObj->m_szBuffer, dwNumOfByteTransfered);
            
            		// ���� ������ ������ ���� ���..
            		//TRACE("���� ������ ������ ���� ���poNetObj->m_nTotalBufLength [%d] receive_length[%d]\n", poNetObj->m_nTotalBufLength, receive_length);		
            		poNetObj->m_nTotalBufLength = poNetObj->m_nTotalBufLength + dwNumOfByteTransfered;
            	
            		g_dlgPtr->GetHeaderInfo(poNetObj);
            		
            		if(poNetObj->nTotalOnePacketLen  < 0)
            		{
            			TRACE("CIocpServer::_do_recv() iOnePacketLength < 0 ..Break\n");            			
						
						CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback nTotalOnePacketLen  < 0 => ����\n", 
									__LINE__  );
            			return 0;
            		}
            		// ���� �� Packet�� �����Ͱ� ���� �ʾҴٸ� ���� �����͸� ��ٸ���.		
            		else if(poNetObj->nTotalOnePacketLen  > poNetObj->m_nTotalBufLength)
            		{
            			TRACE("[%d] ���� �� Packet�� �����Ͱ� ���� �ʾҴ� ��ü[%d] ���� [%d]\n", __LINE__ ,poNetObj->nTotalOnePacketLen,poNetObj->m_nTotalBufLength);
            			
            			WSABUF wsabuf;
        				wsabuf.buf = poNetObj->m_szBuffer;
        				wsabuf.len = MAX_BUFF_SIZE;
        
        				nRet = WSARecv(	poNetObj->m_hSocket, &wsabuf, 1, &dwNumOfByte, 
        								&dwFlag,                     
										&poNetObj->m_prolex->m_ol, 
        								NULL	);
        
						if (nRet == SOCKET_ERROR )
						{
							DWORD rslt = WSAGetLastError();
							TRACE("Line %d [%d] \n", __LINE__, rslt);

        					if ( WSA_IO_PENDING != rslt)
        					{
        						closesocket(poNetObj->m_hSocket);					
        						
        						TRACE("IocpServer WorkerThread return 0 \n");

								CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback WSARecv���� [%d]\n", 
									__LINE__ , rslt );

        						return 0;
        					}
						}            			
            		}
            		// �� Packet�� ������ �����Ͱ� �����Ͽ��ٸ� �� ���̸�ŭ�� ó��
            		else
            		{
            			TRACE("��1�� �� Packet�� ������ �����Ͱ� ���� poNetObj->nTotalOnePacketLen[%d]\n",poNetObj->nTotalOnePacketLen);
            
            			// nExcessiveLen => �ϳ��� ���� �����͸� �ʰ��ؼ� �� ���� ���� 
            			// ������ ���� = ���� ���� ������ - �ϳ��� ��Ŷ ������	
            			int nExcessiveLen = poNetObj->m_nTotalBufLength - poNetObj->nTotalOnePacketLen ;
            			
            			g_dlgPtr->ProcessRecvData(poNetObj);
            
            			TRACE("�� 2 ���� �Ϸ� (���ŵ����� ó���Ϸ�) nExcessiveLen [%d]\n",nExcessiveLen);
            
            			//������ ��Ŷ ������ ���� ������ ������ ������ ��� -> ���۸� �ٽ� ���� 
            			//gThreadArray[nIndex].pBuffer �� nExcessiveLen ��ŭ �Ҵ���
            			//�����Ѵ�. �׸��� ó���� �� �� �����ʹ� ����. 
            			if(nExcessiveLen > 0)
            			{
            				TRACE("������ ��Ŷ ������ ���� ������ ������ ������ ��� ���۸� �ٽ� ����\n");				
            				pTemp = poNetObj->m_pTotalBuff;
            								
            				poNetObj->m_pTotalBuff = new char[nExcessiveLen];				
            				
            				if(poNetObj->m_pTotalBuff == NULL)
            				{
								CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback �޸� �Ҵ� ����\n", 
									__LINE__  );

            					return 0;
            				}
            
            				// nExcessiveLen ��ŭ�� ������ ����, pTemp�� ���� ����Ŷ + ������ ������ ����..
            				memcpy(poNetObj->m_pTotalBuff, pTemp + poNetObj->nTotalOnePacketLen , nExcessiveLen);				
            				poNetObj->m_nTotalBufLength = nExcessiveLen;
            
            				if(pTemp)
            				{					
            					delete[] pTemp;
            					pTemp = NULL;
            				}
            
            				// ������ �����Ͱ� �ϳ��� ��Ŷ�� �����ϴ� ���� ��� ó���Ѵ�. 
            				// ��, ������ ���ۿ� ó���Ҽ��ִ� ��Ŷ�� 1�� �̻��� �ɼ��� �ִ�.
            				
            				while(nExcessiveLen > 0)
            				{
            					g_dlgPtr->GetHeaderInfo(poNetObj);
            					
            					if(poNetObj->nTotalOnePacketLen  < 0)
            					{
            						TRACE("CIocpServer::_do_recv() iOnePacketLength < 0 ..Break\n");									
									
									CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback nTotalOnePacketLen  < 0 ����\n", 
									__LINE__  );

            						return 0;
            					}
            					else if(poNetObj->nTotalOnePacketLen  > poNetObj->m_nTotalBufLength)
            					{
            						TRACE("[%d] ���� �� Packet�� �����Ͱ� ���� �ʾҴ�\n", __LINE__ );
            						
            						WSABUF wsabuf;
                    				wsabuf.buf = poNetObj->m_szBuffer;
                    				wsabuf.len = MAX_BUFF_SIZE;
                    
                    				nRet = WSARecv(	poNetObj->m_hSocket, &wsabuf, 1, &dwNumOfByte, 
                    								&dwFlag, &poNetObj->m_prolex->m_ol, 
                    								NULL);
                    
                    				if (nRet == SOCKET_ERROR )
                    				{
										DWORD rslt = WSAGetLastError();

										if(WSA_IO_PENDING != WSA_IO_PENDING )
										{
                    						closesocket(poNetObj->m_hSocket);					
                    						
                    						TRACE("IocpServer WorkerThread return 0 \n");

											CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback WSARecv���� [%d]\n", 
													__LINE__ , rslt );

                    						return 0;
										}
                    				}            						
            					}
            					// �� Packet�� ������ �����Ͱ� �����Ͽ��ٸ� �� ���̸�ŭ�� ó��
            					// ������ �����Ͱ� �ϳ��� ��Ŷ�� �����ϴ� ���� ��� ó���Ѵ�. 
            					else
            					{ 	
            						g_dlgPtr->ProcessRecvData(poNetObj);

            						nExcessiveLen = nExcessiveLen - poNetObj->nTotalOnePacketLen ;
            
            						if(nExcessiveLen == 0)
            						{
            							// ���޾Ұ� ��Ŷ�뷮�� �°� �� ���۵� ��� 
            							if(poNetObj->m_pTotalBuff)
            							{					
            								delete[] poNetObj->m_pTotalBuff;
            								poNetObj->m_pTotalBuff = NULL;
            							}				
            							poNetObj->m_nTotalBufLength = 0;
										poNetObj->nTotalOnePacketLen = 0;
										poNetObj->nRecvedDataLen = 0;
            							
            							//recv fire 
										memset(poNetObj->m_szBuffer, 0x00, sizeof(poNetObj->m_szBuffer) ); 
            							WSABUF wsabuf;
                        				wsabuf.buf = poNetObj->m_szBuffer;
                        				wsabuf.len = MAX_BUFF_SIZE;
                        
                        				nRet = WSARecv(poNetObj->m_hSocket, &wsabuf, 1, &dwNumOfByte, 
                        								&dwFlag,                     
														&poNetObj->m_prolex->m_ol, 
                        								NULL);
                        
                        				if ( nRet == SOCKET_ERROR )
                        				{
											DWORD rslt = WSAGetLastError();

											if(WSA_IO_PENDING != WSA_IO_PENDING )
											{
                        						closesocket(poNetObj->m_hSocket);					
                        						
                        						TRACE("IocpServer WorkerThread return 0 \n");

												CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback WSARecv���� [%d]\n", 
														__LINE__ , rslt );

                        						return 0;
											}
                        				}            							
            						}
            						else if(nExcessiveLen < 0)
            						{
										CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback nExcessiveLen<0 ����\n", 
														__LINE__ );

            							return 0;
            						}
            						else
            						{
            							memcpy(poNetObj->m_pTotalBuff, poNetObj->m_pTotalBuff + poNetObj->nTotalOnePacketLen , nExcessiveLen);
																				
										poNetObj->nRecvedDataLen = 0;
            							
            							//recv fire 
										memset(poNetObj->m_szBuffer, 0x00, sizeof(poNetObj->m_szBuffer) ); 
            							WSABUF wsabuf;
                        				wsabuf.buf = poNetObj->m_szBuffer;
                        				wsabuf.len = MAX_BUFF_SIZE;
                        
                        				nRet = WSARecv(poNetObj->m_hSocket, &wsabuf, 1, &dwNumOfByte, 
                        								&dwFlag,                     
														&poNetObj->m_prolex->m_ol, 
                        								NULL);
                        
                        				if (nRet == SOCKET_ERROR )
                        				{
											DWORD rslt = WSAGetLastError();

											if(WSA_IO_PENDING != WSA_IO_PENDING )
											{
                        						closesocket(poNetObj->m_hSocket);					
                        						
                        						TRACE("IocpServer WorkerThread return 0 \n");

												CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback WSARecv���� [%d]\n", 
														__LINE__ , rslt );

                        						return 0;
											}                        					
                        				}			
            						}
            					}
            				} // while 
            			}
            			else
            			{
            				if(poNetObj->m_pTotalBuff)
            				{
            					delete[] poNetObj->m_pTotalBuff;
            					poNetObj->m_pTotalBuff = NULL;
            				}				
            				poNetObj->m_nTotalBufLength = 0;
							poNetObj->nTotalOnePacketLen = 0;
							poNetObj->nRecvedDataLen = 0;
							            				
            				//recv fire
            				WSABUF wsabuf;
            				wsabuf.buf = poNetObj->m_szBuffer;
            				wsabuf.len = MAX_BUFF_SIZE;
            
            				nRet = WSARecv(poNetObj->m_hSocket, &wsabuf, 1, &dwNumOfByte, 
            							&dwFlag,                     
										&poNetObj->m_prolex->m_ol, 
            							NULL);
            
            				if (nRet == SOCKET_ERROR )
            				{
								DWORD rslt = WSAGetLastError();

								if(WSA_IO_PENDING != WSA_IO_PENDING )
								{
                        			closesocket(poNetObj->m_hSocket);					
                        						
                        			TRACE("IocpServer WorkerThread return 0 \n");

									CLog::Instance().WriteFile(FALSE, "CriticalServerError", "Ln [%d] WorkerThreadCallback WSARecv���� [%d]\n", 
														__LINE__ , rslt );

                        			return 0;
								}            					
            				}				
            			}
            		}		
            	}
            	
            	TRACE("�� 3 ���� ����");			
			}
			break;
			
			
		case IO_WRITE:			
			delete poOverlapped;
			poOverlapped = NULL ; 
            TRACE("IocpServer WorkerThread delete poOverlapped \n");			
			break;			
		}		
	}		

    TRACE("IocpServer WorkerThread end \n");

	return 0;
}


CIocpServer::CIocpServer(HWND p_hWnd_dlg) : m_nMultiChatRoomNo (0l), m_strAllUserList("")
{	
	m_pDlg = (CIDMS_ServerDlg*)CWnd::FromHandle(p_hWnd_dlg);
	g_dlgPtr = this; 
	
	//::InitializeCriticalSection(&m_CS);
	//::InitializeCriticalSectionAndSpinCount( &m_CS, CSPINCOUNT ); // spin lock �� ���

	//gnTest = 0; //TEST

	OStartup(OSTARTUP_MULTITHREADED); 

	memset(m_szFilterIP1, 0x00, sizeof(m_szFilterIP1) );	
	memset(m_szFilterIP2, 0x00, sizeof(m_szFilterIP2) );	
}

CIocpServer::~CIocpServer()
{
	CMemoryPool::AllDeleteMemory();
		
	POSITION pos = mSockIdSockObjMap.GetStartPosition();

	while( pos != NULL )
	{
		CIocpNetObj* p;
		CString string;		
		mSockIdSockObjMap.GetNextAssoc( pos, string, (CObject*&) p );

		if(p)
		{				
			delete p;
			p = NULL;
		}
	}
	
	mSockIdSockObjMap.RemoveAll();		
		
    
    delete m_pObject ;
	OShutdown(); 
	
}

// �õ�.. ������ ����
int	CIocpServer::Start()
{
	WSADATA wsaData;

	int nResult;

	nResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (NO_ERROR != nResult)
	{
		printf("WSAStartup Error : %d\n", WSAGetLastError());
		return 0;
	}
	
	struct sockaddr_in ServerAddress;

	g_hListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == g_hListenSocket) 
	{
		printf("socket Error : %d\n", WSAGetLastError());
		return 0;
	}

	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
		
	CString strPort;
	//CINIManager::Instance().GetValue(INI_SERVER_IP, strIP);	
	CINIManager::Instance().GetValue(INI_SERVER_PORT, strPort);	
	    
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_port = htons( atoi( (LPCSTR) strPort ) );

	if (SOCKET_ERROR == bind(g_hListenSocket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
	{
		closesocket(g_hListenSocket);

		printf("bind socket Error : %d\n", WSAGetLastError());
		return 0;
	}

	if (SOCKET_ERROR == listen(g_hListenSocket, SOMAXCONN))
	{
		closesocket(g_hListenSocket);

		printf("listen socket Error : %d\n", WSAGetLastError());
		return 0;
	}

#ifdef _NONAGLE
	int nValue = 1;
	setsockopt(g_hListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*) &nValue, sizeof(int));
#endif

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!g_hIocp)
		return 0;

	m_pObject = new CIocpNetObj();
	m_pObject->m_hSocket = g_hListenSocket;

	g_hIocp = CreateIoCompletionPort((HANDLE) g_hListenSocket, g_hIocp, (ULONG_PTR) m_pObject, 0);

	if (!g_hIocp)
		return 0;

	DWORD dwAcceptThreadID = 0;
	g_hAcceptThread	= CreateThread(NULL, 0, ::AcceptThreadCallback, NULL, 0, &dwAcceptThreadID);

	if (!g_hAcceptThread)
		return 0;

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	g_dwWorkerThreadCnt = si.dwNumberOfProcessors * 2;

	for (DWORD i=0;i<g_dwWorkerThreadCnt;i++)
	{
		HANDLE hWorkerThread = CreateThread(NULL, 0, ::WorkerThreadCallback, NULL, 0, NULL);
		g_vWorkerThreadHandle.push_back(hWorkerThread);
	}
	
	return 0;
}

int CIocpServer::SendPendingData(char * szUsage, CIocpNetObj *poNetObj)
{
    BUFFER_OBJ *bufobj=NULL;
    BOOL        breakouter;
    int         nleft,  idx,  ret;
	
    // Attempt to dequeue all the buffer objects on the socket	
    ret = 0;	
	bufobj = DequeueBufferObj(poNetObj);
	
    while ( bufobj ) 
	{        
		breakouter = FALSE;
		
		nleft = bufobj->buflen;
		idx = 0;
	
		while (nleft) 
		{
			//rc = send(sock, &bufobj->buf[idx], nleft, 0);
			int   nRet = 0;
			DWORD dwFlag = 0;
			DWORD dwNumOfByte = 0;

			WSABUF sendwsabuf;
			sendwsabuf.buf = &bufobj->buf[idx];
			sendwsabuf.len = nleft ;

			COverlapped *pSendOverlapped = new COverlapped();
			pSendOverlapped->m_io = IO_WRITE;
				
			nRet = WSASend(	poNetObj->m_hSocket, &sendwsabuf,
							1, &dwNumOfByte, dwFlag,                      
							&pSendOverlapped->m_ol,
							NULL	);

			if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				//goto BREAK_OUT;
				closesocket(poNetObj->m_hSocket);	
				break;
			}
			else if (nRet == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING)
			{
				BUFFER_OBJ *newbuf=NULL;
				newbuf = GetBufferObj(nleft);
				memcpy(newbuf->buf, &bufobj->buf[idx], nleft);
				EnqueueBufferObj(poNetObj, newbuf, TRUE); //queue ��(head)�� ����. ������ �̾ ����.					
					
				return WSA_IO_PENDING;
			}

			nleft -= nleft ;
			idx += 0; //��� .. => ����� EnqueueBufferObj �� ������ ������ �������� �ʾҴ�.

			if( m_bLog && m_bChkFilterIP )
			{
				if	( strcmp( m_szFilterIP1 , poNetObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , poNetObj->m_szIP ) == 0 )
				{
					CTime iDT =  CTime::GetCurrentTime();	
					char szPath [200];
					memset(szPath, 0x00, sizeof(szPath));
					sprintf(szPath,"\n�� %s :SendPendingData : sent [%d/%d] [%s]usage [%s]", 
								iDT.Format("%Y%m%d%H%M%S"), 
								nleft, nleft,poNetObj->m_szIP, szUsage ); 
					
					m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
					
					CLog::Instance().WriteFile(FALSE, "whatareyoudoing", 
										"Ln [%d] [%s] SendPendingData : sent [%d/%d] usage [%s]\n",
										__LINE__, poNetObj->m_szIP, nleft, nleft, szUsage ); 
					
				}
			}
			else if( m_bLog && !m_bChkFilterIP)
			{
				// ȭ�鿡 �Ѹ�.
				CTime iDT =  CTime::GetCurrentTime();	
				char szPath [200];
				memset(szPath, 0x00, sizeof(szPath));
				sprintf(szPath,"\n�� %s :SendPendingData : sent [%d/%d] [%s]usage [%s]", 
							iDT.Format("%Y%m%d%H%M%S"), 
							nleft, nleft,poNetObj->m_szIP, szUsage ); 
				m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
				
				CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] [%s] SendPendingData : sent [%d/%d] usage [%s]\n",
					__LINE__, poNetObj->m_szIP, nleft, nleft, szUsage ); 
				
			}
		}			

		FreeBufferObj(bufobj);	
		
		bufobj = DequeueBufferObj(poNetObj);
    }

    return ret;
}



int	CIocpServer::UpdateAllUserList()
{
	// ��ü ����� ����� DB���� ��ȸ�Ͽ� �޸𸮷� �����Ѵ�.
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));

	sprintf(szSql, "SELECT	/*+ ORDERED USE_NL(A B C D ) */	\
A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT(); // POOL ... 	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( UPDATE_ALL_USER_INIT, "", "", "");	// ���� ID
	pSqlWork->setNotiProcThread(SQLNotifyProc , this);  // call back ����
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return 0;
}

int	CIocpServer::makeAllUserList()
{
	// ��ü ����� ����� DB���� ��ȸ�Ͽ� �޸𸮷� �����Ѵ�.
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));

	sprintf(szSql, "SELECT	/*+ ORDERED USE_NL(A B C D ) */	\
A.USER_ID, A.USER_NAME USER_NAME, B.CODE TEAM_GN, B.CODE_NAME TEAM_NAME	\
FROM KM_DOCUSER A, KM_IDMS_CODE B, KM_CODE_REL C, KM_IDMS_CODE D	\
WHERE  A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001'	AND B.GRP_CODE=C.REL_GRP_CODE(+)	\
AND B.CODE=C.REL_CODE(+) AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+) AND A.STATUS= '01' ORDER BY B.CODE,A.USER_NAME") ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);			
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_ALL_USER_INIT, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return 0;
}

int	CIocpServer::makeUserList()
{	
	// �� ������� ��ȭ������� DB���� ��ȸ�Ͽ� �޸𸮷� �����Ѵ�.
	char szSql [100];
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "SELECT A.USER_ID,A.COMPANION_ID FROM BUDDY_LIST A ORDER BY A.USER_ID") ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);		

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_COMP_LIST_INIT, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	
	
	return 0;
}





void CIocpServer::EnqueueBufferObj(CIocpNetObj *sock, BUFFER_OBJ *obj, BOOL AtHead)
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



BUFFER_OBJ * CIocpServer::DequeueBufferObj(CIocpNetObj *sock)
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



/*
 SendDataToClient
	EnqueueBufferObj(FALSE)
		SendPendingData
			DequeueBufferObj
				EnqueueBufferObj
				
	1. ABCDEF, 123456789, LMOPQRST  ���ÿ� ����
	2. ABCDEF 		EnqueueBufferObj(FALSE)	=> buf1
	3. 123456789	EnqueueBufferObj(FALSE)	=> buf2
	4. LMOPQRST		EnqueueBufferObj(FALSE)	=> buf3
	5. SendPendingData ���� DequeueBufferObj(buf1), DequeueBufferObj(buf2), 
		=> send �� buf1 �� ���� buf2 ���� 4���� ������ WSAEWOULDBLOCK : sent1	
		=> context ��ȯ..
    6. �ٸ� �����尡 �̹��ۿ� XXXX ���� :buf4
	    => context ��ȯ..

  �̻��¿��� ���۴� 
	buf3 -> buf4  
	
	7. 56789 �� ���� ���ο� ����, EnqueueBufferObj(TRUE, Queue �տ�)	: buf5

  �̻��¿��� ���۴� 
	buf3(LMOPQRST) -> buf4(XXXX) -> buf5(56789)

  ABCDEF123456789LMOPQRST �������ϴµ� ���߿� context ��ȯ �Ǹ鼭 
  
  ABCDEF1234LMOPQRSTXXXX56789 �̷� ������ �ްԵȴ�. ó���� �ȵǴ� ��� �߻� 
   
	ABCDEF123456789LMOPQRST
	|sent1   |

	ABCDEF1234LMOPQRST
	|sent1   | sent2 |

	ABCDEF1234LMOPQRST56789  
	|sent1   | sent2 |sent3 |
 */




// �̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢�
// Ŭ���̾�Ʈ�� �۾� ��û�� ó��
// �̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢�

BOOL	CIocpServer::ProcessRecvData(CIocpNetObj * pSockObj)
{		
	//typedef BOOL( CIocpServer::* fPServer)( CIocpNetObj * ) ;
	//static fPServer fPAryServer[] = 

	static BOOL( CIocpServer::* fPAryServer[] )( CIocpNetObj * ) = 
	{	// doDummySqlRsltWork �� �ϸ� -> ǥ�ؿ� ���ؼ� VC2008 ���ʹ� Compiler Error C3867 
		&CIocpServer::doWork_STATUS_AWAY,					//  0
		&CIocpServer::doWork_STATUS_ONLINE,               //z	
		&CIocpServer::doWork_Chat,                        //
		&CIocpServer::doWork_GetBuddyGrpList,	            //      GET_BUDDY_GRP   
		&CIocpServer::doWork_GetBuddyList,			    //      GET_BUDDY_LIST
		&CIocpServer::doWork_SearchID,	                //      SEARCH_ID
		&CIocpServer::doRawSqlWork,						//		RAWSQL_JOB_GN
		&CIocpServer::doRawSqlWork,						//		RAWSQL_DAILYJOB_MST
		&CIocpServer::doWork_Dummy ,						//		RAWSQL_LOGIN => ���̻� ���ȵ� , doWork_LOGIN ��ü	
		&CIocpServer::doRawSqlWork,						//  	RAWSQL_GETUSER_INFO
		&CIocpServer::doRawSqlWork,						//	10	RAWSQL_TEAM
		&CIocpServer::doRawSqlWork,						//		RAWSQL_GETCOMPANY_INFO
		&CIocpServer::doRawSqlWork,						//		RAWSQL_GETCLASS_INFO
		&CIocpServer::doRawSqlWork,						//		RAWSQL_GETBUDDY_INFO
		&CIocpServer::doWork_Dummy,						//  	SOMEONE_ADD_ME
		&CIocpServer::doWork_DelBuddyGrp,					//	15	DEL_BUDDY_GRP
		&CIocpServer::doWork_DelBuddy,					//		DEL_BUDDY
		&CIocpServer::doWork_ChgBuddyGrp,					//		CHG_BUDDY_GRP	
		&CIocpServer::doWork_AddBuddyGrp,					//		ADD_BUDDY_GRP
		&CIocpServer::doWork_ChgGrpName,					//  	CHG_GRP_NAME
		&CIocpServer::doWork_AllowBuddy,					//	20	YOUALLOWEDASCOMP
		&CIocpServer::doWork_Dummy,						//		SOMEONE_DEL_ME 
		&CIocpServer::doWork_Dummy,						//		ACK_NICK_CHG
		&CIocpServer::doWork_SomeoneWantsSendFiles,		//		SOMEONE_WANTS_SEND_FILES			
		&CIocpServer::doWork_Dummy,						//		MULTICHAT_SERVER_ROOMNO  
		&CIocpServer::doWork_CANCEL_FILE_TRANSFER,		//	25	CANCEL_FILE_TRANSFER
		&CIocpServer::doWork_AllMsg,						//		ALL_MSG
		&CIocpServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		ALL_MSG_ACK	
		&CIocpServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		FOLDER_SELECTING 
		&CIocpServer::doWork_FSELING_ALLMACK_NOTHANKF,	//		NO_THANKS_FILE_TRANSFER
		&CIocpServer::doRawSqlMultiOneTranWork,			//	30	RAWSQL_REG_NEW_USER
		&CIocpServer::doRawSqlWork,						//		RAWSQL_SAVE_LAST_IP 
		&CIocpServer::doWork_Dummy,						//		ALREADY_LOGIN 
		&CIocpServer::doWork_Dummy,						//		LOGIN_ALLOWED
		&CIocpServer::doRawSqlWork,						//		RAWSQL_INSERT_LOGONTIME
		&CIocpServer::doRawSqlWork,						//	35	RAWSQL_CHK_DUP_ID
		&CIocpServer::doRawSqlWork,						//		RAWSQL_CHK_USERID	
		&CIocpServer::doRawSqlWork,						//		RAWSQL_UPDATE_PASSWD
		&CIocpServer::doWork_Dummy,						//		ADD_BUDDY_LIST_ERR	
		&CIocpServer::doWork_Dummy,						//		ACK_ONLINE	  		
		&CIocpServer::doWork_Dummy,						//	40	ACK_OFFLINE
		&CIocpServer::doWork_Dummy,						//		YOUALLOWEDASCOMP_INFORM	
		&CIocpServer::doRawSqlWork,						//		RAWSQL_UPDATE_LOGOFFTIME 
		&CIocpServer::doWork_AddBuddyList,				//		ADD_BUDDY_LIST		
		&CIocpServer::doWork_CHK_ALREADY_LOGIN,			//		CHK_ALREADY_LOGIN
		&CIocpServer::doWork_ChngNic,						//	45	CHNG_NICK	
		&CIocpServer::doWork_MultiChatGetRoomNo,			//      MULTICHAT_GET_ROOMNO
		&CIocpServer::doWork_MultiChat,	         		//      MULTI_CHAT_MSG
		&CIocpServer::doWork_Dummy,						//		SET_MULTICHAT_SERVER_ROOMNO	
		&CIocpServer::doWork_EXIT_MULTI_CHAT ,            //      EXIT_MULTI_CHAT
		&CIocpServer::doWork_SLIP_MSG        ,            //  50  SLIP_MSG
		&CIocpServer::doWork_GET_SLIP_MSG    ,            //      GET_SLIP_MSG
		&CIocpServer::doRawSqlWork,                       //      RAWSQL_ACK_SLIP_MSG
		&CIocpServer::doWork_ADD_IT_COMP     ,            //      ADD_IT_COMP  
		&CIocpServer::doWork_DEL_IT_COMP     ,            //  
		&CIocpServer::doWork_CHG_IT_COMP     ,            //  55
		&CIocpServer::doWork_Dummy           ,            //  56  GET_COMP_LIST_INIT
		&CIocpServer::doWork_STATUS_BUSY     ,            //  57 
		&CIocpServer::doWork_LOGIN           ,            //  58  LOGIN 
		&CIocpServer::doWork_FOOD_MENU       ,            //  59  LUNCH_MENU
		&CIocpServer::doWork_ALL_USER_ID_TEAM ,           //  60
		&CIocpServer::doWork_Dummy           ,            //  61  GET_ALL_USER_INIT
		&CIocpServer::doWork_ALL_USER_ID_TEAM,            //  62  ��ȭ��� �߰��� ��� 
		&CIocpServer::doWork_Dummy           ,            //  63  UPDATE_ALL_USER_INIT 
		&CIocpServer::doWork_Dummy           ,            //  64  ALERT_PENDING_WORK 
		&CIocpServer::doWork_ALIVE           ,            //  65  IM_ALIVE
	} ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pSockObj->m_pTotalBuff;

	if( m_bLog && m_bChkFilterIP )
	{
		if	( strcmp( m_szFilterIP1 , pSockObj->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSockObj->m_szIP ) == 0 )
		{
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n�� %s : ProcessRecvData [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSockObj->m_szIP, m_szUsageAry[atoi(psHeader->m_szUsage)] ); 
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] �ڡڡ� [%s] ProcessRecvData / usage [%s/%s]\n", __LINE__, pSockObj->m_szIP, psHeader->m_szUsage, m_szUsageAry[atoi(psHeader->m_szUsage)]  ); 
		}
	}
	else if( m_bLog && !m_bChkFilterIP)
	{
		// ȭ�鿡 �Ѹ�.
		CTime iDT =  CTime::GetCurrentTime();	
		char szPath [200];
		memset(szPath, 0x00, sizeof(szPath));
		sprintf(szPath,"\n�� %s : ProcessRecvData [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSockObj->m_szIP, m_szUsageAry[atoi(psHeader->m_szUsage)] );
		m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
		
		CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] �ڡڡ� [%s] ProcessRecvData / usage [%s/%s]\n", __LINE__, pSockObj->m_szIP, psHeader->m_szUsage, m_szUsageAry[atoi(psHeader->m_szUsage)]  ); 
	}

	BOOL bRslt = (this->*fPAryServer[ atoi(psHeader->m_szUsage)]) (pSockObj);   
	
	TRACE("�� ProcessRecvData : Usage [%s][%s] IP [%s] Rslt[%d]\n", psHeader->m_szUsage, 
		m_szUsageAry[atoi(psHeader->m_szUsage)], pSockObj->m_szIP, bRslt  );

	return TRUE;
}

// Multi Chat Room No Get
BOOL CIocpServer::doWork_MultiChatGetRoomNo(CIocpNetObj* pSockObj )	
{
	// User ID �� �ӽ� set�� ������, ������ ����ڷ� �̷���� 
	// ��Ƽä�� Session �� �̹� �����ϴ��� Ȯ���Ѵ�
	// ���ٸ� ���ο� RoomNo key���� ������ map �� ������, RoomNo�� ��û�ڿ��� �˷��ش�.
	// ������ ����ڵ��� set ����ô� �װ��� RoomNo�� �˷��ش�.    
	// RoomNo : Multi Chat Session = 1:1
	
	//CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

	//toDo!
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	int nLen = 	atoi(psHeader->m_szLength);
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	

	
	char szTmpOutID[20+1]; //ID 
	char szTmpOutName[50+1]; // NAME	
	char szData[1024];
	char szMultiChatUser[ ((20+1+1+50+1+1) * 200) +1]; // ID|NAME 200�� Max (20+1+1+50+1 * 50)+null

	memset(szTmpOutID , 0x00, sizeof(szTmpOutID));		
	memset(szTmpOutName , 0x00, sizeof(szTmpOutName));		
	memset(szData , 0x00, sizeof(szData));	
	memset(szMultiChatUser , 0x00, sizeof(szMultiChatUser));	
	
	int nStrLen = nLen - FROMTOID_HEADER_LENGTH;
	strncpy( szData, pData + FROMTOID_HEADER_LENGTH, nStrLen);	
		
	int nPos = 0;	
	// M Chat ��û Client Dlg ID
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
			// SET_MULTICHAT_SERVER_ROOMNO �� ���� ���ڿ� ����
			strcat(szMultiChatUser, szTmpOutID);
			strcat(szMultiChatUser, "|"); // ID
			
			setMultiChatID.insert( szTmpOutID ) ; // ID �� ���� 
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
				TRACE("������ ����ڸ� ���� Session�� ������ �ش� Room No �� �����ش�.\n");	
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
				
		// Multi Chat Map �� ���� ������..
		// Lock!
		{	//local
			CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");
			
			m_nMultiChatRoomNo++;
			sprintf(szServerRoomNo , "%ld", m_nMultiChatRoomNo );
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
	
	// ��Ƽ Chat�� �Ҽӵ� ���� (���� ��û�� ����)�鿡�� ��ȭ��� ����� ������.!
	// SET_MULTICHAT_SERVER_ROOMNO ���� ����!!! (MULTICHAT_SERVER_ROOMNO �ƴ�!!)
	
	SET_STR::iterator itSet;
	
	char szServerRoomNoTmp[5+1+1]; // + DELIM1 
	memset(szServerRoomNoTmp , 0x00, sizeof(szServerRoomNoTmp));	
	sprintf(szServerRoomNoTmp,"%s%c",szServerRoomNo, DELIM1);

	for (itSet= setMultiChatID.begin(); itSet != setMultiChatID.end(); ++itSet) 
	{
		if(itSet != setMultiChatID.end() )  
		{
			if( strcmp(pInfo->m_szFromID,  (*itSet).c_str() ) != 0)
			{				
				//RoomNo + IDs...

				iLength = strlen(szServerRoomNo) + 1+ strlen(szMultiChatUser) + 1;
	
				// ����� ����	
				memcpy(sMSGHeader.m_szUsage, SET_MULTICHAT_SERVER_ROOMNO, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));					
				
				//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
				
				CIocpNetObj *pa = NULL;
				
				{//local
					
					CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");
					
					if(mIDIPMap.Lookup((*itSet).c_str(), ( CObject*& ) pa ))
					{
						CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
						
						if(pSpckObjMap )
						{			
							// ���۵����� ������
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

	// Multi Chat ��û�ڿ��� RoomNo����
	iLength = sizeof(S_SERVER_ROOMNO) + 1;
	
	// ����� ����	
	memcpy(sMSGHeader.m_szUsage, MULTICHAT_SERVER_ROOMNO, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
	memcpy(sServerRoomNo.m_szClientDlgID , szClientDlgID , sizeof(sServerRoomNo.m_szClientDlgID));
	memcpy(sServerRoomNo.m_szServerRoomNo , szServerRoomNo , sizeof(sServerRoomNo.m_szServerRoomNo));	
		
	CIocpNetObj *pa= NULL;		
		
	{ //local scope 
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChatGetRoomNo");

		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap )
			{			
				// ���۵����� ������
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

BOOL  CIocpServer::doWork_EXIT_MULTI_CHAT( CIocpNetObj* pSockObj )
{
	//CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
			
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	MAP_STR2SET::iterator it;
	SET_STR::iterator itSet;

	it= m_mapStr2SetMultiChat.find(pInfo->m_szToID) ; // pInfo->m_szToID => Server Room No
	
	if(it != m_mapStr2SetMultiChat.end())
	{
		// set ���� Multi Chat ���� ����ڸ� �����Ѵ�.
		itSet = (*it).second.find(pInfo->m_szFromID);

		if(itSet != (*it).second.end())
		{
			TRACE("set ���� Multi Chat ���� ����� [%s]�� �����Ѵ�.\n" , pInfo->m_szFromID );

			(*it).second.erase(itSet) ;		
		}

		// ���� set �� �����Ͱ� ������ (��� ����) map ���� �����Ѵ�. 
		if( (*it).second.empty())
		{
			TRACE("set �� �����Ͱ� ������ (��� ����) map ���� ����\n");
			
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_EXIT_MULTI_CHAT");
				m_mapStr2SetMultiChat.erase(it);			
			}
		}
		else
		{
			// Multi chat �Ҽ��ڵ鿡�� ���� 		
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
						TRACE("���� (*itSet).c_str() = [%s]\n",  (*itSet).c_str());

						// ����� ����	
						memcpy(sMSGHeader.m_szUsage, EXIT_MULTI_CHAT, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));					

						memcpy(sFormToInfo.m_szToID, pInfo->m_szToID, sizeof(sFormToInfo.m_szToID)); // RoomNo					
						memcpy(sFormToInfo.m_szFromID, pInfo->m_szFromID, sizeof(sFormToInfo.m_szFromID));					
											
						//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����					
						CIocpNetObj *pa= NULL;		
												
						{
							CAutoLock ScopeLock ( m_ScopeCS, "doWork_EXIT_MULTI_CHAT");
							if(mIDIPMap.Lookup((*itSet).c_str(), ( CObject*& ) pa ))
							{
								CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
								
								if(pSpckObjMap )
								{			
									// ���۵����� ������							
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

// ��ȸ��, ó���� Raw SQL 1���� ó�� 
BOOL  CIocpServer::doRawSqlWork( CIocpNetObj* pSockObj )
{
	//CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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
		//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

		pSqlWork->SetSQL( szData +  sizeof(S_FROMTOID_HEADER) ); 

		TRACE( "[%s]\n", pSockObj->m_szIP );

		//pSqlWork->SetID( psHeader->m_szUsage, "", "", pSockObj->m_szIP );
		
		CString strSockID ;
		strSockID.Format("%d", pSockObj->m_hSocket) ;	
		pSqlWork->SetID( psHeader->m_szUsage, "", "", strSockID );  

		pSqlWork->setNotiProcThread(SQLNotifyProc , this); 		
		
		global::CThreadPool::instance().queue_request( pSqlWork );
	}	
	else
	{
		// USER ID�� �α��� �ϱ��� .. => IP�� ���е�..

		if	(	
				memcmp(psHeader->m_szUsage, RAWSQL_SAVE_LAST_IP, sizeof(psHeader->m_szUsage)) == 0 || 
				memcmp(psHeader->m_szUsage, RAWSQL_JOB_GN, sizeof(psHeader->m_szUsage)) == 0 || 		
				memcmp(psHeader->m_szUsage, RAWSQL_DAILYJOB_MST, sizeof(psHeader->m_szUsage)) == 0 || 		
				memcmp(psHeader->m_szUsage, RAWSQL_INSERT_LOGONTIME, sizeof(psHeader->m_szUsage)) == 0 
			)
		{
			// SQL�۾� ���� �ʰ� ó���Ѵ�.
		}
		else
		{
			S_FROMIP_HEADER sFTInfo ;
			memcpy(&sFTInfo , szData , sizeof(S_FROMIP_HEADER));

			// Memory POOL �̿� ���� ����..
			//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
			
			COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
			//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

			pSqlWork->SetSQL( szData +  sizeof(S_FROMIP_HEADER) ); 

			TRACE( "[%s]\n", pSockObj->m_szIP );

			CString strSockID ;
			strSockID.Format("%d", pSockObj->m_hSocket) ;	

			//pSqlWork->SetID( psHeader->m_szUsage, "", "", pSockObj->m_szIP);
			pSqlWork->SetID( psHeader->m_szUsage, "", "", strSockID );

			pSqlWork->setNotiProcThread(SQLNotifyProc , this); 		
			
			global::CThreadPool::instance().queue_request( pSqlWork );		
		}		
	}	

	return TRUE;
}

BOOL  CIocpServer::doWork_ADD_IT_COMP(CIocpNetObj* pSockObj)	
{	
	char* szRawData= pSockObj->m_pTotalBuff ;	
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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( ADD_IT_COMP, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->SetUsefulInfo(szDescUrl);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;
}



//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL  CIocpServer::doWork_DEL_IT_COMP(CIocpNetObj* pSockObj)	
{
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;

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

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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
	
	// ó�� ��� ����!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, DEL_IT_COMP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	
	CIocpNetObj *pa= NULL;
		
	{	
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_DEL_IT_COMP");
		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap )
			{			
				// ���۵����� ������
				char* pSzBuf = NULL;
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
				
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

BOOL  CIocpServer::doWork_CHG_IT_COMP( CIocpNetObj* pSockObj )	
{
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( CHG_IT_COMP, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->SetUsefulInfo(szDescUrl);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;

}


BOOL  CIocpServer::doWork_SLIP_MSG( CIocpNetObj* pSockObj )
{	
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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

	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( SLIP_MSG, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );

	return TRUE;
}

BOOL  CIocpServer::doWork_GET_SLIP_MSG( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_SLIP_MSG, pInfo->m_szToID, pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	   

	// ������Ʈ �ȵǴ� ����ڵ鿡�� ���� ������.
	/*
	if( strcmp (pInfo->m_szToID, "VERSION_CHK") != 0 )
	{
		m_strAryForUpdate.Add (pInfo->m_szFromID) ;
	}
	else
	{
		//�����
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


BOOL  CIocpServer::doWork_FSELING_ALLMACK_NOTHANKF( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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
	
	CIocpNetObj*  paTmp;
	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_FSELING_ALLMACK_NOTHANKF");
	
		if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) paTmp))
		{			
			if(paTmp )
			{
				// ����� ����		
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
				
				
				// ���۵����� ������
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

BOOL  CIocpServer::doWork_Dummy( CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	// char* szRawData= pSockObj->m_pTotalBuff ;

	return TRUE;
}

BOOL  CIocpServer::doWork_CANCEL_FILE_TRANSFER( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;
	
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
	
	CIocpNetObj*  paTmp;
	
	
	{	
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_FSELING_ALLMACK_NOTHANKF");

		if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) paTmp))
		{			
			if(paTmp )
			{
				// ����� ����		
				memcpy(sMSGHeader.m_szUsage, CANCEL_FILE_TRANSFER, sizeof(sMSGHeader.m_szUsage));		
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, sizeof(S_CANCEL_FILE_TRANSFER) );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));			
				memcpy(sCancelInfo.szFromID, pInfo->m_szFromID, sizeof(sCancelInfo.szFromID));			
				memcpy(sCancelInfo.szFromIP, pSockObj->m_szIP , sizeof(sCancelInfo.szFromIP)); 
				
				// ���۵����� ������
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
// �������� ó���� SQL�� �ϳ��� TRANSACTION ���� ó���Ǿ�� �ϴ� ���
BOOL  CIocpServer::doRawSqlMultiOneTranWork( CIocpNetObj* pSockObj )
{	
	// CIocpNetObj* pSockObj
	char* szRawData= pSockObj->m_pTotalBuff ;

	BOOL bExists = FALSE;
	//S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)szRawData;
	char *szData = szRawData + COMMON_HEADER_LENGTH ;

	// 20060705 ���� �б�� ��� �����Ƿ� ���� (����)
	//if( memcmp(szUsage, RAWSQL_REG_NEW_USER, sizeof(szUsage)) == 0 )	
	//{		
	
	S_FROMIP_HEADER sChkDupInfo ;
	memcpy(&sChkDupInfo , szData , sizeof(S_FROMIP_HEADER));
	
	const string str = szData +  sizeof(S_FROMIP_HEADER) ;
	vector<string> SqlVec;
	const string& delimiters = "!" ;

	// ó���� �����ִ� �����ڵ� Skip
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    
	// ù��° "�����ڰ� �ƴ� ����" ã�´�.
	string::size_type pos  = str.find_first_of(delimiters, lastPos);
	
	while (string::npos != pos || string::npos != lastPos)
	{
		// ��ū�� ã������ vector�� �߰�.
		SqlVec.push_back(str.substr(lastPos, pos - lastPos));
		
		// �����ڸ� SKIP
		lastPos = str.find_first_not_of(delimiters, pos);
		
		// �״��� "�����ڰ� �ƴ� ����"�� ã��
		pos = str.find_first_of(delimiters, lastPos);
	} 	

	int nCnt = SqlVec.size();
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));
	
	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
		
	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!
	
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

		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);
		
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
	
	TRACE("�� nCnt [%d]\n", nCnt );
	
	vector<string>::iterator location ;
	
	for (location=SqlVec.begin(); location != SqlVec.end(); ++location) 
	{
		//TRACE("%s\n", (*location).c_str() );
		bExists = TRUE;
		if (odb.ExecuteSQL( (*location).c_str() ) != OSUCCESS)
		{
			sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					
			
			TRACE("�� ERROR!! [%s]\n", szErrMsg );
			
			osess.Rollback(false); 
			
			odb.Close();
			osess.Close();						
			bRslt = FALSE;
			goto EXIT_HOLE;
		}			
		
		//pSqlWork->SetSQL( (*location).c_str() ); 
		//pSqlWork->SetID( ID_MEANINGLESS, "", "", sChkDupInfo.m_szIP);
		
	}

	bRslt = TRUE;
	osess.Commit(false);		
	odb.Close();
	osess.Close();
	

EXIT_HOLE:
	
	OShutdown();

	if(bExists)
	{
		//pSqlWork->SetID( psHeader->m_szUsage, "", "", sChkDupInfo.m_szIP);

		S_COMMON_HEADER sMSGHeader;
		char szTemp [20];
		memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
		memset(szTemp, 0x00, sizeof(szTemp));
		char* pSzBuf = NULL;
		S_COM_RESPONSE sComRslt;
		memset(&sComRslt, 0x00, sizeof(sComRslt));

		TRACE("����� ��� ������ ��� ó��\n");	
		int iLength = sizeof(S_COM_RESPONSE);				
		
		// �����ͺ� ����
		if(bRslt == TRUE)
		{
			memcpy(sComRslt.szRsltCd, "Y", min(sizeof(sComRslt.szRsltCd), strlen("Y")));			
		}
		else
		{
			memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
		}

		memcpy(sComRslt.szErrMsg, szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(szErrMsg)));
		
		// ����� ����	
		memcpy(sMSGHeader.m_szUsage, RAWSQL_REG_NEW_USER, sizeof(sMSGHeader.m_szUsage));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
		memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
		memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
		//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
		CIocpNetObj *pa= NULL;
		// mIPSockObjMap key: IP 
				
		{		
			CAutoLock ScopeLock ( m_ScopeCS, "doRawSqlMultiOneTranWork");
			//if(mIPSockObjMap.Lookup(sChkDupInfo.m_szIP, ( CObject*& ) pa ))
			
			CString strSockID ;
			strSockID.Format("%d", pSockObj->m_hSocket) ;	
			if(mSockIdSockObjMap.Lookup(strSockID, ( CObject*& ) pa ))
			{
				CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
				
				if(pSpckObjMap  )
				{			
					// ���۵����� ������
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

BOOL CIocpServer::doWork_CHK_ALREADY_LOGIN(CIocpNetObj* pSockObj)
{	
	S_COMMON_HEADER sMSGHeader;
	char szTemp[20];
	char* pSzBuf = NULL;
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	

	// ����� ����		
	memcpy(sMSGHeader.m_szUsage, LOGIN_ALLOWED, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, 0 );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));

	CIocpNetObj*  paTmp2;
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_CHK_ALREADY_LOGIN");
		//if(mIPSockObjMap.Lookup(pSockObj->m_szIP, ( CObject*& ) paTmp2) ) // 20061204 
	
		CString strSockID ;
		strSockID.Format("%d", pSockObj->m_hSocket) ;	

		if(mSockIdSockObjMap.Lookup( strSockID, ( CObject*& ) paTmp2) ) 
		{
			if(paTmp2 )
			{
				// ���۵����� ������
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


// ��ȭ��� GRP ���
BOOL CIocpServer::doWork_GetBuddyGrpList(CIocpNetObj* pSockObj)
{	
	char* pRawData= pSockObj->m_pTotalBuff ;	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;		

	// mIDIPMap �� �߰�!
	CIocpNetObj *pa= NULL;	
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_GetBuddyGrpList");

	CString strSockID ;
	strSockID.Format("%d", pSockObj->m_hSocket) ;	

	//if(mIPSockObjMap.Lookup(pSockObj->m_szIP, ( CObject*& ) pa ))	// 20061204 	
	if(mSockIdSockObjMap.Lookup( strSockID, ( CObject*& ) pa ))	// 20061204 	
	{
		if(pa )
		{
			memset(pa-> m_szUserId, 0x00, sizeof(pa-> m_szUserId));
			strncpy(pa-> m_szUserId , pInfo->m_szFromID, sizeof(pa-> m_szUserId));					
			
			mSockIdSockObjMap.SetAt(strSockID, (CObject*) pa); // ip -> id ������ �˼� �ְ� �Ѵ�. (��������� IP -> ID ����) 20061204 

			mIDIPMap.SetAt(pInfo->m_szFromID, (CObject*) pa); // �� ��ü�� 2���� �ʿ��� �ٶ󺸰� ��..
		}		
	}
	else
	{
		//Log		
		CLog::Instance().WriteFile(FALSE, "ServerLog",  "Ln [%d] [%s] doWork_GetBuddyGrpList mIPSockObjMap ����IP�� ��ã�� ��� FAIL\n", 
			__LINE__, pSockObj->m_szIP );
	}
		
	char szSql [200];
	memset(szSql, 0x00 , sizeof(szSql));
	
	sprintf(szSql, "SELECT /*+ INDEX(A PK_BUDDY_GRP) */ A.COMP_GROUP FROM BUDDY_GRP A	\
WHERE A.USER_ID ='%s' ORDER BY A.COMP_GROUP", pInfo->m_szFromID) ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( GET_BUDDY_GRP, "", pInfo->m_szFromID, "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}

// ��ü ��� ��û�� ó�� 
BOOL CIocpServer::doWork_ALL_USER_ID_TEAM(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	// �̹� ��ȸ�س��� ������ m_strAllUserList �� ������ �¶��� ���θ� �����ؼ� ������.
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
				
				CIocpNetObj *pa = NULL;
				
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
			
	iLength = strSQLRslt.length() ;	// �������� ������ ������ ���� 
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, /*ALL_USER_ID_TEAM*/ pFromHeader->m_szUsage , sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����	=> ID�� �˻�!!	
	CIocpNetObj *pa = NULL;
		
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALL_USER_ID_TEAM3");
		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap )
			{			
				// ���۵����� ������
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
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

BOOL CIocpServer::doWork_FOOD_MENU(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	TRACE("m_szFromID   [%s] [%s]\n", pInfo->m_szFromID, pSockObj->m_szIP);
				
	S_FOOD_MENU_PERIOD sSearchPeriod;
	memcpy(&sSearchPeriod, pData+FROMTOID_HEADER_LENGTH, sizeof(S_FOOD_MENU_PERIOD) );

	/* DB ���� �������� ����� ���...
	char szSql [300];
	memset(szSql, 0x00 , sizeof(szSql));

    sprintf(szSql,"SELECT / *+ INDEX(A PK_FOOD_MENU) * / FOOD_DAY, LD_FLAG, MENU,CAL FROM FOOD_MENU A WHERE FOOD_DAY BETWEEN '%s' AND '%s' ORDER BY FOOD_DAY, LD_FLAG",
		sSearchPeriod.m_szFromDate, sSearchPeriod.m_szToDate);
  			
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);
	pSqlWork->SetID( FOOD_MENU, "", pInfo->m_szFromID, pSockObj->m_szIP);	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		
	*/
	
	// �޸𸮿� �ִ� ���� ����
	S_COMMON_HEADER sMSGHeader;
	S_COM_RESPONSE sComRslt;				

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;
	char szTemp[20];	

	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	memset(&sComRslt, NULL, sizeof(sComRslt));
	memset(szTemp, NULL, sizeof(szTemp));

	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ + MULL
	int nSqlDataLen = m_pDlg->m_strMenu.GetLength(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// �������� ������ ������ ���� 
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, FOOD_MENU, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
	S_SQL_RSLTDATA sSqlRsltData;
	memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));	
	memcpy(sSqlRsltData.szRsltCd, "Y", strlen("Y") );	
	memcpy(sSqlRsltData.szErrMsg, "", strlen("") );				
	
	CIocpNetObj * pa = NULL;
	
	CIocpNetObj * pSpckObjMap = NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_FOOD_MENU");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)m_pDlg->m_strMenu, nSqlDataLen ); 
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
							
			int nRtn = SendDataToClient( (char*) FOOD_MENU,pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_FOOD_MENU SendDataToClient\n",
										__LINE__, pSpckObjMap->m_szIP );	
				
			}			
		}			
	}
		
	return TRUE;
}


BOOL CIocpServer::doWork_LOGIN(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);		
	//pSqlWork->SetID( LOGIN, "", pInfo->m_szFromID, pSockObj->m_szIP );	 
	
	CString strSockID ;
	strSockID.Format("%d", pSockObj->m_hSocket) ;	
	pSqlWork->SetID( LOGIN, "", pInfo->m_szFromID, strSockID );	 
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		
	
	return TRUE;
}

// ��ȭ��� ���
BOOL CIocpServer::doWork_GetBuddyList(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	
	//TRACEX("m_szFromID   [%s] [%s]\n", pInfo->m_szFromID, pSockObj->m_szIP);   
	
	char szSql [1024];
	memset(szSql, 0x00 , sizeof(szSql));

	// ����ģ�� ���� NICK �� �̸�, COMPID �� �ּ�����
	sprintf(szSql, "SELECT X.KIND,X.NAME,X.NICK,X.COMPID,X.COMPGRP, X.IP FROM ( SELECT /*+ INDEX(A PK_BUDDYLIST) INDEX(B TX_DOCUSER_01)*/	\
A.KIND AS KIND,C.USER_NAME AS NAME,DECODE(C.NICK_NAME,'',' ',C.NICK_NAME) AS NICK,A.COMPANION_ID AS COMPID,A.COMP_GROUP AS COMPGRP, C.IP_ADDR AS IP	\
FROM BUDDY_LIST A,KM_DOCUSER B,KM_DOCUSER C WHERE A.USER_ID ='%s' AND B.USER_ID=A.USER_ID	\
AND C.USER_ID=A.COMPANION_ID UNION ALL SELECT /*+ INDEX(D PK_IT_BUDDYLIST) */	\
'9' AS KIND, D.IT_NAME AS NAME,D.IT_NAME AS NICK,D.IT_INFO AS COMPID,'���ͳ� ���ã��' AS COMPGRP , '*' AS IP FROM IT_BUDDY_LIST D WHERE D.USER_ID ='%s'	\
) X	ORDER BY X.NAME", pInfo->m_szFromID, pInfo->m_szFromID) ;

	// '9' AS KIND, '*' AS NAME,D.IT_NAME AS NICK,D.IT_INFO AS COMPID,'���ͳ� ���ã��' AS COMPGRP , '*' AS IP FROM IT_BUDDY_LIST D WHERE D.USER_ID ='%s'	
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);		

	pSqlWork->SetID( GET_BUDDY_LIST, "", pInfo->m_szFromID, pSockObj->m_szIP );	

	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		

	return TRUE;
}


BOOL CIocpServer::doWork_AddBuddyList(CIocpNetObj* pSockObj)
{	
//	KIND ģ�� ���� 
//	* 1 - ���� ����� ���
//	* 2 - ������ ���� �߰��ؼ� ������� ���� ��ȭ��뿡 �߰���
//	* 3 - ��ȭ �ź�
//	* 4 - ��ȭ��밡 ���� ������
//	* 9 - ����ģ��
//
//	BUDDY_LIST �� ����, A �� B �� �߰��Ѱ�� 
//	USER_ID = 'A'                			
//  	COMPANION_ID = 'B'
//  	REG_DATE = ����ð�
//  	COMP_GROUP = ���ñ׷�
//  	KIND ='1'
//  		
//  	USER_ID = 'B'                			
//  	COMPANION_ID = 'A'
//  	REG_DATE = ����ð�
//  	COMP_GROUP = �⺻�׷� 
//  	KIND ='2'
//  		
//  	2 Row �� ����ȴ�..
 
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
		
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	S_ADD_BUDDY * pNewBuddyData = (S_ADD_BUDDY*) pData;
	
	char szSql [300];	
	memset(szSql, 0x00 , sizeof(szSql));
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	int nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::itoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!
	
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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);
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

	// �� ��ȭ��� ��Ͽ� �߰�	
	sprintf(szSql, "INSERT INTO BUDDY_LIST(USER_ID, COMPANION_ID, REG_DATE,COMP_GROUP,KIND)	VALUES('%s',(SELECT /*+ INDEX(A TX_DOCUSER_01)*/ A.USER_ID FROM KM_DOCUSER A WHERE A.USER_ID ='%s'),SYSDATE,'%s','1')", 
		pNewBuddyData->m_szUserID , pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szBuddyGrp) ;

	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("�� ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		odb.Close();
		osess.Close();
	
		bRslt = FALSE;
		goto EXIT_HOLE;
	}
		
	
	//2 ��° DB �۾� -> ������ ��ȭ��Ͽ��� ���� �߰��Ѵ�. �⺻�׷쿡 �߰���
	memset(szSql, 0x00 , sizeof(szSql));	
	sprintf(szSql, "INSERT INTO BUDDY_LIST(USER_ID, COMPANION_ID, REG_DATE,COMP_GROUP,KIND)	\
VALUES((SELECT /*+ INDEX(A TX_DOCUSER_01)*/ A.USER_ID FROM KM_DOCUSER A WHERE A.USER_ID ='%s'),'%s',SYSDATE,'%s','2')", 
		pNewBuddyData->m_szBuddyID , pNewBuddyData->m_szUserID , "�⺻�׷�" /*pNewBuddyData->m_szBuddyGrp*/) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("�� ERROR!! [%s]\n", szErrMsg );

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
		
	// �����ͺ� ����
	if(bRslt == TRUE)
	{
		//20061125 �����ϰ� ONLINEó���� �� exceptionó�� 
		
		// ��ȭ ��� ����� ���� (�޸�)
		MAP_STR2VEC::iterator it;
		
		
		it = m_mapStr2VecCompanion.find( pNewBuddyData->m_szUserID );
		if(it != m_mapStr2VecCompanion.end() )  
		{		
			((*it).second).push_back( pNewBuddyData->m_szBuddyID ) ;
		}
		else
		{
			// vector �� ���� ��� 			
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
			// vector �� ���� ��� 

			VEC_STR vTmp;	
			vTmp.reserve(50);
			vTmp.push_back( pNewBuddyData->m_szUserID ) ;
			m_mapStr2VecCompanion.insert( MAP_STR2VEC::value_type( pNewBuddyData->m_szBuddyID, vTmp ) );
		}
			
		// �߰��� ��ȭ����� ������ ��ȸ�Ѵ�..		
		//A�� B�� �߰��� ��� ���� B������ ���ؼ� A�� �����ϰ� ,
		//A������ ���ؼ� B�� �����Ѵ�.			

		char szSql [300];
		memset(szSql, 0x00 , sizeof(szSql));

		//1. A���� ������ B ������ ���Ѵ�.
		sprintf(szSql, "SELECT /*+  INDEX(A TX_DOCUSER_01)*/ A.USER_ID, A.USER_NAME, A.NICK_NAME, A.IP_ADDR,B.COMP_GROUP, '1'	\
	FROM KM_DOCUSER A, BUDDY_LIST B WHERE A.USER_ID ='%s' AND B.USER_ID = '%s' AND  B.COMPANION_ID = A.USER_ID ", 
		pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID ) ;
									
		COracleWorkerMT * pSqlWork1 = CMemoryPool::New_COracleWorkerMT();	
		//pSqlWork1->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

		pSqlWork1->SetSQL( szSql ); 
		// ó������� ���۵ɶ� SetID �� 3��°�� ������  ID �� ���۵Ǵ°��� �̿��Ѵ�.
		pSqlWork1->SetID( RAWSQL_GETBUDDY_INFO, pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID, pNewBuddyData->m_szUserID);

		pSqlWork1->setNotiProcThread(SQLNotifyProc , this); 
		global::CThreadPool::instance().queue_request( pSqlWork1 );		

		//2. B���� ������  A ������ ���Ѵ�.			
		memset(szSql, 0x00 , sizeof(szSql));
		sprintf(szSql, "SELECT /*+  INDEX(A TX_DOCUSER_01)*/ A.USER_ID, A.USER_NAME, A.NICK_NAME, A.IP_ADDR,B.COMP_GROUP, '2'	\
	FROM KM_DOCUSER A, BUDDY_LIST B WHERE A.USER_ID ='%s' AND B.USER_ID = '%s' AND  B.COMPANION_ID = A.USER_ID ", 
		pNewBuddyData->m_szUserID, pNewBuddyData->m_szBuddyID ) ;			
						
		COracleWorkerMT * pSqlWork2 = CMemoryPool::New_COracleWorkerMT();	
		//pSqlWork2->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd );	
		pSqlWork2->SetSQL( szSql ); 
		// ó������� ���۵ɶ� SetID �� 3��°�� ������  ID �� ���۵Ǵ°��� �̿��Ѵ�. => SOMEONE_ADD_ME !!
		pSqlWork2->SetID( SOMEONE_ADD_ME, pNewBuddyData->m_szUserID, pNewBuddyData->m_szBuddyID, pNewBuddyData->m_szUserID);
		pSqlWork2->setNotiProcThread(SQLNotifyProc , this); 
		global::CThreadPool::instance().queue_request( pSqlWork2 );			
	}
	else
	{
		// �߰� ���� ������ �����Ѵ�.
		S_COM_RESPONSE sComRslt;
		memset(&sComRslt, 0x00, sizeof(sComRslt));
		memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
		memcpy(sComRslt.szErrMsg, szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(szErrMsg)));
		
		// ����� ����		
		memcpy(sMSGHeader.m_szUsage, ADD_BUDDY_LIST_ERR, sizeof(sMSGHeader.m_szUsage));		
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
		memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
		sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
		memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
		
		//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����			
		
		CIocpNetObj *pa = NULL;
				
		{
			CAutoLock ScopeLock ( m_ScopeCS, "doWork_AddBuddyList");

			if(mIDIPMap.Lookup(pNewBuddyData->m_szUserID, ( CObject*& ) pa ))	
			{
				CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
				
				if(pSpckObjMap )
				{			
					// ���۵����� ������
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
BOOL CIocpServer::doWork_AllowBuddy(CIocpNetObj* pSockObj)
{	
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char* pData = pRawData + COMMON_HEADER_LENGTH;
	S_ADD_BUDDY * pNewBuddyData = (S_ADD_BUDDY*) pData;
	
	char szSql [300];	
	
	OStartup(OSTARTUP_MULTITHREADED);
	
	char szSessionNameIndex[10];
	memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

	long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
	::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
	//TRACE("szSessionNameIndex [%s]\n", szSessionNameIndex);

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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
	
	//  KIND : 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '1',COMP_GROUP='%s' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND ='3' ", 
		pNewBuddyData->m_szBuddyGrp, pNewBuddyData->m_szUserID , pNewBuddyData->m_szBuddyID ) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("�� ERROR!! [%s]\n", szErrMsg );

		osess.Rollback(false); 
		
		odb.Close();
		osess.Close();	

		bRslt = FALSE;
		goto EXIT_HOLE;
	}
	//pSqlWork1->SetID( ID_MEANINGLESS, "", "", ""); 	
	
	//----------------------------------------------------------
	// 5 �ΰ��� �ش絥���� ����ø� ó����
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '2' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND = '5' ", 
		pNewBuddyData->m_szBuddyID , pNewBuddyData->m_szUserID ) ;
	
	if (odb.ExecuteSQL(szSql) != OSUCCESS)
	{
		sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

		TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

		TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	CIocpNetObj *pa = NULL;

	// A�� B�� �ٽ� �߰��� ��� A ���� �������� ����
		
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllowBuddy1");

		if(mIDIPMap.Lookup(pNewBuddyData->m_szUserID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap  )
			{
				S_COMMON_HEADER sMSGHeader;
				char szTemp [20];
				memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
				memset(szTemp, 0x00, sizeof(szTemp));
				char* pSzBuf = NULL;

				int iLength = SQL_RSLTDATA_LENGTH + sizeof(pNewBuddyData->m_szBuddyID) + sizeof(S_YOU_ALLOWED) ; //ID GRPNAME
				// ����� ����			
				memcpy(sMSGHeader.m_szUsage, YOUALLOWEDASCOMP, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
				// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
				// ���۵����� ������
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				S_YOU_ALLOWED sDataAllowed;
				memset(&sDataAllowed , NULL, sizeof(S_YOU_ALLOWED));
				memcpy(sDataAllowed.m_szYouAllowedID, pNewBuddyData->m_szBuddyID, sizeof(sDataAllowed.m_szYouAllowedID));
				memcpy(sDataAllowed.m_szGrpName, pNewBuddyData->m_szBuddyGrp, sizeof(sDataAllowed.m_szGrpName));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA
				memcpy(pSzBuf + COMMON_HEADER_LENGTH+SQL_RSLTDATA_LENGTH ,&sDataAllowed, sizeof(S_YOU_ALLOWED)); 
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
				
				int nRtn = SendDataToClient("doWork_AllowBuddy",pNewBuddyData->m_szUserID,pSpckObjMap, pSzBuf, nTotalLen);
				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_AllowBuddy SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}			
		}
	}
			
	// A�� B�� �ٽ� �߰��� ��� B ���� A����ȭ��� ���� ���� ������ ����	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllowBuddy2");
		
		if(mIDIPMap.Lookup(pNewBuddyData->m_szBuddyID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap  )
			{	
				S_COMMON_HEADER sMSGHeader;
				char szTemp [20];
				memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
				memset(szTemp, 0x00, sizeof(szTemp));
				char* pSzBuf = NULL;

				int iLength = sizeof(pNewBuddyData->m_szUserID )  ; // A
				// ����� ����			
				memcpy(sMSGHeader.m_szUsage, YOUALLOWEDASCOMP_INFORM, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));				
				
				// ���۵����� ������
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
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
BOOL CIocpServer::doWork_DelBuddy( CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
//	KIND : 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
//	A�� B�� �����ϴ°��
//	B.nBockMode = 1,2 �ΰ�� => 1. A�� ������ ��û�ؼ� BUDDY_LIST���� A.KIND = '3', B.KINE = '4' UPDATE !
//									2. ������ A�� �޽��������� A Ʈ������ B����, MAP A.nBockMode = 3 ����.
//									3. ������ B�� �޽��������� B Ʈ������ A���ܻ��� ǥ��, MAP A.nBockMode = 4 ����.
//
//	B.nBockMode = 4 �ΰ��	=>	KIND 5 UPDATE!	
//									(B�� A�� �ٽ� �߰��ÿ��� �߰��Ұ� ó���� Client���� �Ѵ�. ��ȭ������� DB�����ü��ְ� ������ �ʴ´�)		
//
//	A�� B�� �ٽ� �߰�		=>	������ ��û�ؼ� BUDDY_LIST���� A.KIND = '1', B.KINE = '2' UPDATE !
	 		
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
	

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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

			//Delete => �̰��� �ش������ �������� �����ɰ���! => ������������. 			
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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();		

				bRslt = FALSE;
				goto EXIT_HOLE;
			}
			
			
			// �������� ����� BUDDY_LIST.KIND = 4 (���濡 ���ؼ� ���ܴ���) ����!			
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '4' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND IN ('1','2') ", szTmpOut , pInfo->m_szFromID ) ;
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

				osess.Rollback(false); 
				odb.Close();
				osess.Close();
			
				bRslt = FALSE;
				goto EXIT_HOLE;
			}
			
			// ����Ͽ��� BUDDY_LIST.KIND = 3 (���� ������) ����!
			memset(szSql, 0x00 , sizeof(szSql));
			sprintf(szSql, "UPDATE BUDDY_LIST SET KIND = '3' WHERE USER_ID ='%s' AND COMPANION_ID = '%s' AND KIND IN ('1','2') ", pInfo->m_szFromID , szTmpOut) ;
			
			if (odb.ExecuteSQL(szSql) != OSUCCESS)
			{
				sprintf(szErrMsg, "[%d] %s", odb.ServerErrorNumber(), odb.GetServerErrorText());					

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

	// 1. A���� B,C ���� ������ �˸�!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, DEL_BUDDY, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����	
	CIocpNetObj *pa = NULL;	
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddy1");

		if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
		{
			CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
			
			if(pSpckObjMap  )
			{			
				// ���۵����� ������
				char* pSzBuf = NULL;
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
				
				int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
				
				int nRtn = SendDataToClient("doWork_DelBuddy",pInfo->m_szFromID, pSpckObjMap, pSzBuf, nTotalLen);

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doWork_DelBuddy SendDataToClient\n",
											__LINE__, pSockObj->m_szIP );
				}
			}			
		}
	}
	
	
	// 2. ���������϶��� ( bRslt == TRUE) B,C �鿡�� A�� ���������� �˸� 
	
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

				// ����� ����			
				memcpy(sMSGHeader.m_szUsage, SOMEONE_DEL_ME, sizeof(sMSGHeader.m_szUsage));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
				memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
				sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
				memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
				// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
				S_SQL_RSLTDATA sSqlRsltData;
				memset(&sSqlRsltData, NULL, sizeof(sSqlRsltData));				
				memcpy(sSqlRsltData.szRsltCd, "Y", min(sizeof(sSqlRsltData.szRsltCd), strlen("Y")));
				memcpy(sSqlRsltData.szErrMsg, szErrMsg, min(sizeof(sSqlRsltData.szErrMsg), strlen(szErrMsg)));
							
				//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����	
				CIocpNetObj *pa= NULL;
								
				{
					CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddy2");
				
					if(mIDIPMap.Lookup(szTmpOut, ( CObject*& ) pa ))
					{
						CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
						
						if(pSpckObjMap )
						{			
							// ���۵����� ������
							char* pSzBuf = NULL;
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
							memset(pSzBuf, NULL, sizeof(pSzBuf));
							
							memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, pInfo->m_szFromID,  strlen( pInfo->m_szFromID ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
							
							int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
							
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


BOOL CIocpServer::doWork_AddBuddyGrp(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_ADD_BUDDY_GRP * pBuddyGrp = (S_ADD_BUDDY_GRP*) pData;

	char szSql [200];	
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "INSERT INTO BUDDY_GRP(USER_ID,COMP_GROUP) VALUES('%s','%s')", pBuddyGrp->m_szUserID , pBuddyGrp->m_szBuddyGrp) ;
	
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	
	pSqlWork->SetID( ADD_BUDDY_GRP, "", pBuddyGrp->m_szUserID, "");	//�߰��� �׷� ������ ��������!!
	pSqlWork->SetUsefulInfo(pBuddyGrp->m_szBuddyGrp);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
BOOL CIocpServer::doWork_ChgGrpName(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
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
	
	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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
	
	// Bef �׷��
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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

	// ó�� ��� ����!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, CHG_GRP_NAME, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	
	CIocpNetObj *pa= NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ChgGrpName");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap )
		{			
			// ���۵����� ������
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
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


// ��ȭ��� �ڸ����
BOOL CIocpServer::doWork_STATUS_AWAY(CIocpNetObj* pSockObj)	
{	
	//return TRUE; //20070118
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	CIocpNetObj * pSpckObjMap = NULL;		
	
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
						
						// ����� ����			
						memcpy(sMSGHeader.m_szUsage, STATUS_AWAY, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// ���۵����� ������
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //���� ��� + sAwayInfo
						
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

/// ��ȭ��� �ڸ���� ����
BOOL CIocpServer::doWork_STATUS_ONLINE(CIocpNetObj* pSockObj)	
{	
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;		
	CIocpNetObj *pSpckObjMap = NULL;		
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
						
						// ����� ����			
						memcpy(sMSGHeader.m_szUsage, STATUS_ONLINE, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// ���۵����� ������
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //���� ��� + sAwayInfo
						
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

BOOL CIocpServer::doWork_ALIVE(CIocpNetObj* pSockObj)	
{	
	// ���� ��� ������ �����Ѵ�.
	char* pRawData= pSockObj->m_pTotalBuff ;	
	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;			
	S_COMMON_HEADER sMSGHeader;

	int iLength = 0;	
	int nTotalLen = 0;	
	char* pSzBuf = NULL;								
	char szTemp[20];	
	
	memset(&sMSGHeader, NULL, sizeof(sMSGHeader));
	
	iLength = 0;	// �׳� ����� ... 
	
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	CIocpNetObj *pSpckObjMap = NULL;	
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ALIVE");

	if(mIDIPMap.Lookup(pInfo->m_szFromID , ( CObject*& ) pSpckObjMap ))
	{
		if(pSpckObjMap  && (DWORD)pSpckObjMap != 0xfeeefeee )
		{					
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			
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



// ��ȭ��� �ٸ��빫��
BOOL CIocpServer::doWork_STATUS_BUSY(CIocpNetObj* pSockObj)	
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

	char* pData = pRawData + COMMON_HEADER_LENGTH;	
	
	S_FROMTOID_HEADER * pInfo = (S_FROMTOID_HEADER*) pData;	
	CIocpNetObj *pSpckObjMap = NULL;		
	
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
						
						// ����� ����			
						memcpy(sMSGHeader.m_szUsage, STATUS_BUSY, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// ���۵����� ������
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sAwayInfo, sizeof(sAwayInfo)); //���� ��� + sAwayInfo
						
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

//�α��� ���̵� ��ȸ
BOOL CIocpServer::doWork_SearchID(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( SEARCH_ID, "", pSearchID->szIDFrom, "");
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	return TRUE;
}


// ��ȭ�� ����
BOOL CIocpServer::doWork_ChngNic(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_CHG_NICK * pChngNic = (S_CHG_NICK*) pData;
	//TRACE("szIDFrom [%s] / BefNick [%s] / AftNick[%s]\n", pChngNic->szIDFrom, pChngNic->szNickBef, pChngNic->szNickAft);   
	
	char szSql [300];
		
	// 1. ���� KM_DOCUSER ���� ����	
	memset(szSql, 0x00 , sizeof(szSql));	
	sprintf(szSql, "UPDATE KM_DOCUSER SET NICK_NAME ='%s' WHERE USER_ID ='%s'", pChngNic->szNickAft, pChngNic->szIDFrom) ;
			
	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();	
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( CHNG_NICK, "", pChngNic->szIDFrom, "");	
	pSqlWork->SetUsefulInfo( pChngNic->szNickAft);
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );		

	return TRUE;
}


//20061105 +++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ��ȭ��� �׷� ����
BOOL CIocpServer::doWork_ChgBuddyGrp(CIocpNetObj* pSockObj)
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

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

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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

	// ������ �׷��
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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

	// ó�� ��� ����!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, CHG_BUDDY_GRP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	
	CIocpNetObj *pa= NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_ChgBuddyGrp");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
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
BOOL CIocpServer::doWork_DelBuddyGrp( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
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

	OSession osess(szSessionNameIndex); // ���ο� Session�� �����ؾ���!!!

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
		oresult nDbRslt = odb.Open(osess, (LPCSTR) CServerGlobal::Instance().m_strDBConn, (LPCSTR)CServerGlobal::Instance().m_strUser, 
						(LPCSTR) CServerGlobal::Instance().m_strPassWd);

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

				TRACE("�� ERROR!! [%s]\n", szErrMsg );

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

	// ó�� ��� ����!!!
	S_COMMON_HEADER sMSGHeader;
	char szTemp [20];
	memset(&sMSGHeader, 0x00, sizeof(sMSGHeader));
	memset(szTemp, 0x00, sizeof(szTemp));

	int nSqlDataLen = strlen( szData ); 
	int iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;

	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, DEL_BUDDY_GRP, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	
	CIocpNetObj *pa= NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_DelBuddyGrp");

	if(mIDIPMap.Lookup(pInfo->m_szFromID, ( CObject*& ) pa ))
	{
		CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			char* pSzBuf = NULL;
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, szData,  strlen( szData ) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			int nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
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



// ���������� ����
BOOL CIocpServer::doWork_SomeoneWantsSendFiles( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;
	
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
	
	// ����� ����	
	memcpy(sMSGHeader.m_szUsage, SOMEONE_WANTS_SEND_FILES, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sFromToIdHeader.m_szToID , pInfo->m_szToID , sizeof(sFromToIdHeader.m_szToID));
	memcpy(sFromToIdHeader.m_szFromID , pInfo->m_szFromID , sizeof(sFromToIdHeader.m_szFromID));	
	memcpy(sFromToIdHeader.m_szIP , pSockObj->m_szIP , sizeof(sFromToIdHeader.m_szIP)); 
	
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	CIocpNetObj *pa= NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_SomeoneWantsSendFiles");

	if(mIDIPMap.Lookup(pInfo->m_szToID, ( CObject*& ) pa ))
	{
		CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap )
		{			
			// ���۵����� ������
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

BUFFER_OBJ *  CIocpServer::GetBufferObj(int buflen)
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
	
	return newobj;

}

BOOL CIocpServer::doWork_AllMsg( CIocpNetObj* pSockObj )
{	
	char* pRawData= pSockObj->m_pTotalBuff ;	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;

	// ToID���� DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 �޽��� DELIM1 ������ �̸�	

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

	// ���� �޽��� ����
	int nLastPos = strData .ReverseFind(DELIM1);		
	strncpy(szAllMsg, pData + S_CHAT_HEADER_LENGTH+ nLastPos + 1, nStrLen - nLastPos -1  );
	
	int nPos = 1;
	char szTmpOut [20 + 1]; 

	memset(szTmpOut, 0x00, sizeof(szTmpOut));
	nPos = GetStrByDelim( (LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
	
	int nCnt = atoi(szTmpOut) ; // ToID ���� 
	
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
			
			// ����� ����	
			memcpy(sMSGHeader.m_szUsage, ALL_MSG, sizeof(sMSGHeader.m_szUsage));
			sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
			memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
			sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
			memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
			
			memcpy(sChatHeader.szIDTo , szTmpOut , sizeof(sChatHeader.szIDTo));
			memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
			
			//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
			CIocpNetObj *pa= NULL;
						
			{
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_AllMsg");

				if(mIDIPMap.Lookup(szTmpOut, ( CObject*& ) pa ))
				{
					CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
					
					if(pSpckObjMap )
					{
						// ���۵����� ������
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

		Sleep(1); // send �� ���������� ������ �ϴ� ��쿡 ���� ����� ���� �ʿ���!!!
	}
	
	return TRUE;
}

// Multi Chat 
BOOL CIocpServer::doWork_MultiChat( CIocpNetObj* pSockObj )
{	
	char* pRawData= pSockObj->m_pTotalBuff ;	
	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);
	char * pData = pRawData + COMMON_HEADER_LENGTH;	
	S_CHAT_HEADER * pInfo = (S_CHAT_HEADER*) pData;			
	int nStrLen = nLen - S_CHAT_HEADER_LENGTH ;	

	// ������ ����ؼ� MAX_CHAT_LEN ��ŭ�� ó������..
	// Client ������ MAX_CHAT_LEN �ִ�� ����..
	char szChatMsg[MAX_CHAT_LEN+1];
	memset(szChatMsg, 0x00, sizeof(szChatMsg));
	strncpy(szChatMsg, pData + S_CHAT_HEADER_LENGTH, nStrLen );

	/* ������ ���̿� ���� ���� ���..
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
	
	//iLength = strlen((LPCSTR)strData) + S_CHAT_HEADER_LENGTH+ 1; // ������ ���̿� ���� ���� ���..
	iLength = strlen(szChatMsg) + S_CHAT_HEADER_LENGTH+ 1;	
	
	// ����� ����	
	memcpy(sMSGHeader.m_szUsage, MULTI_CHAT_MSG, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sChatHeader.szIDTo , pInfo->szIDTo , sizeof(sChatHeader.szIDTo));
	memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
	
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	
	MAP_STR2SET::iterator it;
	SET_STR::iterator itSet;

	it= m_mapStr2SetMultiChat.find(pInfo->szIDTo) ; // pInfo->m_szToID => ServerRoomNo, set�� ã�´�..
	
	if(it != m_mapStr2SetMultiChat.end() )  
	{
		// set�� �ִ� ����鿡�� �����Ѵ�.
		for (itSet= (*it).second.begin(); itSet != (*it).second.end(); ++itSet) 
		{
			CIocpNetObj *pa= NULL;		
						
			{ // ScopeLock �� ������ü�� �����. 
				CAutoLock ScopeLock ( m_ScopeCS, "doWork_MultiChat");

				if(mIDIPMap.Lookup( (*itSet).c_str(), ( CObject*& ) pa ))
				{
					if( strcmp(pInfo->szIDFrom, (*itSet).c_str()) == 0 )
					{
						//LeaveCriticalSection(&m_CS); //20090306
						//continue; // �̷������ξ��� �����ϴ�.... LeaveCriticalSection ȣ��ȵǴ°��߻� 
						//else �� �и��ϴ°� ���� 
					}
					else
					{
						CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
					
						if(pSpckObjMap  )
						{			
							// ���۵����� ������
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
							memset(pSzBuf, NULL, sizeof(pSzBuf));	
							memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
							memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));				
							//memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, (LPCSTR)strData, strlen((LPCSTR)strData));	//������ ���̿� ���� ���� ���..
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

// ä�� �޽���
BOOL CIocpServer::doWork_Chat( CIocpNetObj* pSockObj )
{
	// CIocpNetObj* pSockObj
	char* pRawData= pSockObj->m_pTotalBuff ;

	S_COMMON_HEADER* psHeader = (S_COMMON_HEADER *)pRawData;
	int nLen = atoi(psHeader->m_szLength);
	char * pData = pRawData + COMMON_HEADER_LENGTH;	

	S_CHAT_HEADER * pInfo = (S_CHAT_HEADER*) pData;			
	int nStrLen = nLen - S_CHAT_HEADER_LENGTH ;	

	// ������ ����ؼ� MAX_CHAT_LEN ��ŭ�� ó������..
	// Client ������ MAX_CHAT_LEN �ִ�� ����..
	char szChatMsg[MAX_CHAT_LEN+1];
	memset(szChatMsg, 0x00, sizeof(szChatMsg));
	strncpy(szChatMsg, pData + S_CHAT_HEADER_LENGTH, nStrLen );

	/* ������ ���̿� ���� ���� ���..
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
	
	//iLength = strlen((LPCSTR)strData) + S_CHAT_HEADER_LENGTH+ 1; // ������ ���̿� ���� ���� ���..
	iLength = strlen(szChatMsg) + S_CHAT_HEADER_LENGTH+ 1;
	
	
	// ����� ����	
	memcpy(sMSGHeader.m_szUsage, CHAT_MSG, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	memcpy(sChatHeader.szIDTo , pInfo->szIDTo , sizeof(sChatHeader.szIDTo));
	memcpy(sChatHeader.szIDFrom , pInfo->szIDFrom , sizeof(sChatHeader.szIDFrom));
	
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	CIocpNetObj *pa= NULL;		
		
	CAutoLock ScopeLock ( m_ScopeCS, "doWork_Chat");

	if(mIDIPMap.Lookup(pInfo->szIDTo, ( CObject*& ) pa ))
	{
		CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChatHeader, sizeof(sChatHeader));				
			//memcpy(pSzBuf + COMMON_HEADER_LENGTH+S_CHAT_HEADER_LENGTH, (LPCSTR)strData, strlen((LPCSTR)strData));	//������ ���̿� ���� ���� ���..
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



BOOL CIocpServer::doSqlRsltWork_CHNG_NICK(S_CALLBACKINFO* pSCallBackInfo)
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
				
				CIocpNetObj *pSpckObjMap = NULL;
								
				{
					CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_CHNG_NICK");

					if(mIDIPMap.Lookup((*itV).c_str(), ( CObject*& ) pSpckObjMap ))
					{
						if(pSpckObjMap )
						{
							iLength = sizeof(S_CHG_NICK);	
							
							// ����� ����			
							memcpy(sMSGHeader.m_szUsage, ACK_NICK_CHG, sizeof(sMSGHeader.m_szUsage));
							sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
							memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
							sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // ������ ����!
							memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
							
							// ���۵����� ������
							pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
							memset(pSzBuf, NULL, sizeof(pSzBuf));		
							
							memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
							memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sChngNick, sizeof(sChngNick)); //���� ��� + sChngNick
							
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
									pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
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

BOOL CIocpServer::doSqlRsltWork_2(S_CALLBACKINFO* pSCallBackInfo)
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
	
	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ + MULL
	//CString strTmpOut = "";
	CString strRslt = "";
	
	//int nPos = pSCallBackInfo->m_strData.Find(DELIM1) ;		
	//strTmpOut = pSCallBackInfo->m_strData.Mid(0, nPos); 

	//TEST 20090209
	int nPos = pSCallBackInfo->m_strData.find(DELIM1) ;		
	string strTmpOut = pSCallBackInfo->m_strData.substr(0, nPos); 
	
	TRACE("\n **** strTmpOut [%s]\n" , strTmpOut.c_str() );

	// ���ӿ���
	CIocpNetObj *pa = NULL;		
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_2:1");

		if(mIDIPMap.Lookup(strTmpOut.c_str() , ( CObject*& ) pa ))
		{
			if(pa  )
			{
				// �̹� IP�� ���ؼ� ������ ��������..				
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
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen + 1;	// �������� ������ ������ ���� 		
	
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
	
	
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	CIocpNetObj * pSpckObjMap = NULL;
		
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_2:2");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage ,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_2 SendDataToClient\n",
										__LINE__, pSpckObjMap->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_2 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
			}			
		}
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_ADD_BUDDY_GRP(S_CALLBACKINFO* pSCallBackInfo)
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
				
	strcpy(sGrpInfo.m_szBuddyGrp, pSCallBackInfo->m_szMultiRoleUsefulInfo); // m_szMultiRoleUsefulInfo=>�߰��� �׷��
	
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	
	CIocpNetObj *pa = NULL;	
	CIocpNetObj * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ADD_BUDDY_GRP");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			//!!!!
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, &sGrpInfo, sizeof(S_ADD_BUDDY_CLISVR) ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
			int nRtn = SendDataToClient("doSqlRsltWork_ADD_BUDDY_GRP",pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 
			
			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_ADD_BUDDY_GRP SendDataToClient\n",
										__LINE__, pSpckObjMap->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_ADD_BUDDY_GRP : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);	

	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_4(S_CALLBACKINFO* pSCallBackInfo)
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
	
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	
	CIocpNetObj *pa = NULL;	
	CIocpNetObj * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_4");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, pSCallBackInfo->m_szMultiRoleUsefulInfo, strlen(pSCallBackInfo->m_szMultiRoleUsefulInfo) ); 
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 				
			
			if(nRtn == -1 )
			{			
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_4 SendDataToClient\n",
										__LINE__, pSpckObjMap->m_szIP );
			}
			else
			{
				TRACE("doSqlRsltWork_4 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	
			}
		}			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

//GET_BUDDY_GRP
BOOL CIocpServer::doSqlRsltWork_5(S_CALLBACKINFO* pSCallBackInfo)
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

	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ + MULL
	//int nSqlDataLen = pSCallBackInfo->m_strData.GetLength(); 
	int nSqlDataLen = pSCallBackInfo->m_strData.length(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// �������� ������ ������ ���� 
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����	=> ID�� �˻�!!	
	CIocpNetObj * pa = NULL;
	
	CIocpNetObj * pSpckObjMap = NULL;

	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_5");

	if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
							
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage,pSCallBackInfo->m_szIDFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_5 SendDataToClient!!\n",
					__LINE__, pSpckObjMap->m_szIP);	
				CTime iDT =  CTime::GetCurrentTime();				
			}
			else
			{
				TRACE("doSqlRsltWork_5 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
			}
		}			
	}
	else
	{
		//Log	
		CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] doSqlRsltWork_5 USAGE[%s] SQL����, ID ��ã��!!! FAIL!![%s] [%s]\n", 
			__LINE__, pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIDFrom, pSCallBackInfo->m_szIPFrom );			
	}
		
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_GET_BUDDY_LIST (S_CALLBACKINFO* pSCallBackInfo)
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

	// ID�� ������ ���濡�� �����ϴ� ���.. (mIDIPMap)	
	
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
					// CON_STATUS, AWAY_INFO �� ������ ���� �߰��Ѵ�. (index 5,6)
					CIocpNetObj *pa= NULL;
					
					{										
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_BUDDY_LIST");

						if(mIDIPMap.Lookup(szTmpID, ( CObject*& ) pa ))
						{
							if( pa )
							{
								//IP
								strSQLRslt.append(pa->m_szIP)  ; 
								strSQLRslt.append(1, DELIM1);

								//CON_STATUS
								strSQLRslt.append("Y")  ; //6
								strSQLRslt.append(1, DELIM1); 
								
								//AWAY_INFO //7
								strSQLRslt.append(pa->m_szAwayInfo)  ;
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

	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ + MULL
	int nSqlDataLen = strlen(strSQLRslt.c_str()); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen ;	// �������� ������ ������ ���� 
	
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	CIocpNetObj* pa= NULL;	
	char szTmpIp[20];
	memset(szTmpIp, 0x00, sizeof(szTmpIp) );
	
	{
		CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_BUDDY_LIST2");

		if(mIDIPMap.Lookup(pSCallBackInfo->m_szIDFrom, ( CObject*& ) pa ))
		{		
			if(pa)
			{
				strncpy(szTmpIp, pa->m_szIP , sizeof(szTmpIp));

				// ���۵����� ������
				pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
				memset(pSzBuf, NULL, sizeof(pSzBuf));
				
				memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
				memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, strSQLRslt.c_str(), nSqlDataLen ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
				
				nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
				
				int nRtn = SendDataToClient("doSqlRsltWork_GET_BUDDY_LIST",pSCallBackInfo->m_szIDFrom, pa, pSzBuf, nTotalLen); 

				if(nRtn == -1 )
				{
					CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln [%d] [%s] doSqlRsltWork_GET_BUDDY_LIST SendDataToClient!!\n",
								__LINE__, pa->m_szIP);
				}
				else
				{
					TRACE("doSqlRsltWork_GET_BUDDY_LIST#1 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
							pa->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
							pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
				}
			}			
		}	
	}
		
	// �α����ϴ� ������ �� ����ڸ� ��ȭ���� �ϰ��ִ� ����鿡�� �α��������� �˸�!
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
			CIocpNetObj *pSpckObjMap = NULL ;				
						
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
						
						// ����� ����			
						memcpy(sMSGHeader.m_szUsage, ACK_ONLINE, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); // ������ ����!
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// ���۵����� ������
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sOnlineBuddy, sizeof(sOnlineBuddy)); //���� ��� + sOnlineBuddy
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						//TRACE("�α����ϴ� ������ �α��������� �˸�! [%s]\n", (*itV).c_str() );
						
						int nRtn = SendDataToClient("ACK_ONLINE",(char*)((*itV).c_str()), pSpckObjMap, pSzBuf, nTotalLen); 
						
						if(nRtn == -1 )
						{
							CLog::Instance().WriteFile(FALSE, "ServerError",  "Ln[%d] SendDataToClient [%s]\n",
									__LINE__, pSpckObjMap->m_szIP);
						}
						else
						{
							TRACE("doSqlRsltWork_GET_BUDDY_LIST#2 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
								pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
								pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
						}
					}					
				}
			}			
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
	

	// ȭ�鿡 �Ѹ�.
	char szTmpIP[200];
	memset(szTmpIP, 0x00, sizeof(szTmpIP));
	sprintf(szTmpIP,"\n�� %s [%s] connected..",  pSCallBackInfo->m_szIPFrom, pSCallBackInfo->m_szIDFrom ); //20061106
					
	m_pNotifyProc((LPVOID) m_pDlg,szTmpIP , NULL);

	m_pNotifyProc((LPVOID) m_pDlg, "", &gCurrentConnections); 
	
	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_7(S_CALLBACKINFO* pSCallBackInfo)
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
		
	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ + MULL
	//int nSqlDataLen = pSCallBackInfo->m_strData.GetLength(); // strlen( (LPCSTR)pSCallBackInfo->m_strData); 
	int nSqlDataLen = pSCallBackInfo->m_strData.length(); 
	iLength = SQL_RSLTDATA_LENGTH + nSqlDataLen + 1;	// �������� ������ ������ ���� 		
	
	// ����� ����			
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, pSCallBackInfo->m_nRecordCnt); // ������ ����!
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	// S_SQL_RSLTDATA �����͸� S_COMMON_HEADER �ڿ� ���̰�, + �����͸� ����		
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
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ���� => IP �� �˻�!!!
	
	CIocpNetObj *pa = NULL;
	CIocpNetObj * pSpckObjMap = NULL;
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_7");
	
	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap )
		{
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));		
			
			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sSqlRsltData, SQL_RSLTDATA_LENGTH); //���� ��� + S_SQL_RSLTDATA		
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, (LPCSTR)pSCallBackInfo->m_strData.c_str(), nSqlDataLen ); //���� ��� + S_SQL_RSLTDATA + SQLDATA		
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength; // iLength ���� �̹� SQL_RSLTDATA_LENGTH ����
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError",  "doSqlRsltWork_7 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, 
					pSCallBackInfo->m_szUsage, 
					m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , 
					pSCallBackInfo->m_szIDTo	);	
			}
			else
			{
				TRACE("doSqlRsltWork_7 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);		
				
				//���⼭
				/*
				RAWSQL_SAVE_LAST_IP / doSqlRsltWork_9/ BOOL CChatSession::doWork_RAWSQL_SAVE_LAST_IP( char* data) / LRESULT CLogInDlg::OnSaveLastIPRslt(WPARAM wParam, LPARAM lParam) / 
				RAWSQL_JOB_GN    / doSqlRsltWork_7/ BOOL CChatSession::doWork_RAWSQL_JOB_GN (char* data) / LRESULT CLogInDlg::OnJobGnRslt(WPARAM wParam, LPARAM lParam)
				������ �����ϰų� 
				RAWSQL_DAILYJOB_MST /doSqlRsltWork_7/ BOOL CChatSession::doWork_RAWSQL_DAILYJOB_MST (char* data)   / LRESULT CLogInDlg::OnDailyJobMstRslt(WPARAM wParam, LPARAM lParam)    
				RAWSQL_INSERT_LOGONTIME /doSqlRsltWork_9   / BOOL CChatSession::doWork_RAWSQL_INSERT_LOGONTIME( char* data) / LRESULT CLogInDlg::OnInsertLogOnTmRslt(WPARAM wParam, LPARAM lParam)
				*/
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_8(S_CALLBACKINFO* pSCallBackInfo)
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
	
	// �����ͺ� ����		
	if(pSCallBackInfo->m_nRecordCnt  > 0)
	{
		memcpy(sChkUserId.szExists, "Y", min(sizeof(sChkUserId.szExists), strlen("Y")));
	}
	else
	{
		memcpy(sChkUserId.szExists, "N", min(sizeof(sChkUserId.szExists), strlen("N")));
	}	
	
	memcpy(sChkUserId.szPasswd, (LPCSTR)pSCallBackInfo->m_strData.c_str(), sizeof(sChkUserId.szPasswd) - 1 );			
	
	// ����� ����		
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
				
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����
	
	CIocpNetObj *pa = NULL ;	
	CIocpNetObj * pSpckObjMap = NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_8");

	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	
	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sChkUserId, iLength);	
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_8 SendDataToClient! [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP );					
			}			
			else
			{
				TRACE("doSqlRsltWork_8 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	return TRUE;
}

BOOL CIocpServer::doSqlRsltWork_9(S_CALLBACKINFO* pSCallBackInfo)
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
	
	// �����ͺ� ����
	if(pSCallBackInfo->m_bRslt == TRUE)
	{
		memcpy(sComRslt.szRsltCd, "Y", min(sizeof(sComRslt.szRsltCd), strlen("Y")));
	}
	else
	{
		memcpy(sComRslt.szRsltCd, "N", min(sizeof(sComRslt.szRsltCd), strlen("N")));
	}
	
	memcpy(sComRslt.szErrMsg, pSCallBackInfo->m_szErrMsg, min(sizeof(sComRslt.szErrMsg), strlen(pSCallBackInfo->m_szErrMsg)));
	
	// ����� ����		
	memcpy(sMSGHeader.m_szUsage, pSCallBackInfo->m_szUsage, sizeof(sMSGHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
	
	//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
	// RAWSQL_UPDATE_LOGOFFTIME �� ���� ���� �Ⱥ�����.
	
	if(strcmp( pSCallBackInfo->m_szUsage,RAWSQL_UPDATE_LOGOFFTIME ) ==0 )
	{
		CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
		return TRUE;
	}

	CIocpNetObj * pa = NULL;			
	CIocpNetObj * pSpckObjMap = NULL;
	
	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_9");

	if(mSockIdSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	//if(mIPSockObjMap.Lookup(pSCallBackInfo->m_szIPFrom, ( CObject*& ) pa ))
	{
		pSpckObjMap = static_cast<CIocpNetObj*>(pa);
		
		if(pSpckObjMap  )
		{			
			// ���۵����� ������
			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; //!!!
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sMSGHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sComRslt, iLength);	
			
			nTotalLen = COMMON_HEADER_LENGTH + iLength;
			
			int nRtn = SendDataToClient(pSCallBackInfo->m_szUsage, pSCallBackInfo->m_szIPFrom, pSpckObjMap, pSzBuf, nTotalLen); 			

			if(nRtn == -1 )
			{
				CLog::Instance().WriteFile(FALSE, "ServerError", "Ln [%d] doSqlRsltWork_9 SendDataToClient! [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP );				
			}
			else
			{
				TRACE("doSqlRsltWork_9 : IP[%s] pSCallBackInfo->m_szUsage [%s][%s]  m_szIDFrom[%s] m_szIDTo [%s]\n" , 
					pSpckObjMap->m_szIP, pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], 
					pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);
			}
		}
	}
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
	
	return TRUE;
}



BOOL CIocpServer::doSqlRsltWork_ALERT_PENDING_WORK(S_CALLBACKINFO* pSCallBackInfo)
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

	//int nPos = pSCallBackInfo->m_strData.GetLength();
	int nPos = pSCallBackInfo->m_strData.length();

	//TEST LOG
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\data\\AlertPending%s.log", iDT.Format("%Y%m%d")  );
//	writeLogFile(szTmpPath, "[%s] DATA [%s] \n", 
//							iDT.Format("%Y%m%d%H%M%S"),pSCallBackInfo->m_strData);	

	while( 1 )
	{
		memset(szTmpOut , 0x00, sizeof(szTmpOut));			
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)pSCallBackInfo->m_strData.c_str(), szTmpOut, DELIM1);
		
		if(nPos != -1)
		{
			switch(nIndex % 8)
			{
			case 0:
				{	// 1. REQUIRE_PERSON_ID ��û��
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
				{	// 3. WORK_ID 		������			
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
				{	// 5. REQUIRE_NUM 	��û��ȣ				
					memset(szRequireNum , 0x00, sizeof(szRequireNum));			
					strncpy(szRequireNum , szTmpOut, sizeof(szRequireNum) ); 	
				}					
				break;
			case 5:
				{	// 6. REQUIRE_NAME	������					
					memset(szRequireName , 0x00, sizeof(szRequireName));			
					strncpy(szRequireName , szTmpOut, sizeof(szRequireName) ); 	
				}					
				break;
			case 6:
				{	// 7. REQUIRE_DATE 	��û��				
					memset(szRequireDate , 0x00, sizeof(szRequireDate));			
					strncpy(szRequireDate , szTmpOut, sizeof(szRequireDate) ); 	
				}					
				break;
			case 7:
				{	// 8. END_DUE_DATE	���Ό����		
					memset(szEndDueDate , 0x00, sizeof(szEndDueDate));
					memset(szMsg , 0x00, sizeof(szMsg));
					strncpy(szEndDueDate , szTmpOut, sizeof(szEndDueDate) ); 	
					
					sprintf(szMsg,"\r\n  ���ð��� �������� ������ ���� �˷��帳�ϴ�.\r\n  - ��û��ȣ: %s\r\n  - ����: %s\r\n  - �䱸�� [%s] �Ϸ������[%s]\r\n  �� �Ϸ��Ͽ� �ֽñ� �ٶ��ϴ�.",
						szRequireNum,szRequireName,szRequireDate,szEndDueDate);
					
					//������.
					S_COMMON_HEADER sMSGHeader;
					S_CHAT_HEADER   sChatHeader;
					int iLength = 0;	
					int nTotalLen = 0;
					char* pSzBuf = NULL;
					char szTemp[20];
					
					memset(&sMSGHeader, NULL, sizeof(sMSGHeader));	
					memset(&sChatHeader, NULL, sizeof(sChatHeader));
					
					iLength = strlen( szMsg ) + S_CHAT_HEADER_LENGTH+ 1;
					
					// ����� ����	
					memcpy(sMSGHeader.m_szUsage, ALL_MSG, sizeof(sMSGHeader.m_szUsage));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
					memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
					sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1);
					memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
					
					memcpy(sChatHeader.szIDTo , szWorkID , sizeof(sChatHeader.szIDTo));
					memcpy(sChatHeader.szIDFrom , "SERVER" , sizeof(sChatHeader.szIDFrom));
					
					//����� Ŭ���̾�Ʈ���� ���޴�� �˻��� ����		
					CIocpNetObj *pa= NULL;	
					
															
					// �����ڿ��� ����					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ALERT_PENDING_WORK");

						if(mIDIPMap.Lookup(szWorkID, ( CObject*& ) pa ))
						{
							CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
							
							if(pSpckObjMap )
							{
								// ���۵����� ������
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
										__LINE__ , pSpckObjMap->m_szIP );								
								}							
							}
						}
					}
					
					// KT����ڿ��� ����					
					{
						CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_ALERT_PENDING_WORK2");

						if(mIDIPMap.Lookup(szRequirePersonID, ( CObject*& ) pa ))
						{
							CIocpNetObj * pSpckObjMap = static_cast<CIocpNetObj*>(pa);
							
							if(pSpckObjMap )
							{					
								// ���۵����� ������
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
										__LINE__ , pSpckObjMap->m_szIP );								
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
	return TRUE;
}

// ��ü ����� ��� UPDATE ��ư ���� ���
BOOL CIocpServer::doSqlRsltWork_UPDATE_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo)
{	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_UPDATE_ALL_USER_INIT");
	
	m_strAllUserList = (LPCSTR)pSCallBackInfo->m_strData.c_str();
	
	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	// ��ϸ� ����.
	// m_pDlg->SendMessage(WM_IMADE_ALL_USERLIST,0,0);
			
	TRACE("doSqlRsltWork_UPDATE_ALL_USER_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	

	return TRUE;
}

// ��ü ����� ��� 
BOOL CIocpServer::doSqlRsltWork_GET_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo)
{
	//USER_ID, USER_NAME, TEAM_GN, TEAM_NAME	
	CAutoLock ScopeLock ( m_ScopeCS, "doSqlRsltWork_GET_ALL_USER_INIT");
	
	m_strAllUserList = (LPCSTR)pSCallBackInfo->m_strData.c_str();
	

	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);

	// Main Dlg �� �˸�.
	m_pDlg->SendMessage(WM_IMADE_ALL_USERLIST,0,0);
		
	TRACE("doSqlRsltWork_GET_ALL_USER_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	
	
	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] doSqlRsltWork_GET_ALL_USER_INIT\n", __LINE__  );	
	
	return TRUE;
}

// �ѻ���ڿ� ���� ��ȭ��� ����� �����.
BOOL CIocpServer::doSqlRsltWork_GET_COMP_LIST_INIT(S_CALLBACKINFO* pSCallBackInfo)
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
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] Server �⵿!\n", __LINE__  );	
	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	
	// Main Dlg �� �˸�.
	m_pDlg->SendMessage(WM_IMADEUSERLIST,0,0);
			
	TRACE("doSqlRsltWork_GET_COMP_LIST_INIT : pSCallBackInfo->m_szUsage [%s][%s] m_szIDFrom[%s] m_szIDTo [%s]\n" , 
		pSCallBackInfo->m_szUsage, m_szUsageAry[atoi(pSCallBackInfo->m_szUsage)], pSCallBackInfo->m_szIDFrom , pSCallBackInfo->m_szIDTo);	

	return TRUE;
}


int CIocpServer::SendDataToClient(char * szUsage, char * szFromID, CIocpNetObj * pSpckObjMap, char* pSzBuf, int nTotalLen)
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
			if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_szIP ) == 0 )
			{
				CTime iDT =  CTime::GetCurrentTime();	
				char szPath [200];
				memset(szPath, 0x00, sizeof(szPath));
				sprintf(szPath,"\n�� %s :SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_szIP, szUsage ); 
				m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
				
				CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage ); 
			}
		}
		else if( m_bLog && !m_bChkFilterIP)
		{
			// ȭ�鿡 �Ѹ�.
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n�� %s :SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), 
				pSpckObjMap->m_szIP, szUsage ); //20061106					
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendDataToClient ERROR !! GetBufferObj return NULL ! [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage );			
		}

		return -1;
	}

	memcpy(newbuf->buf, pSzBuf, nTotalLen);
	
	
	{
		//CAutoLock ScopeLock ( m_ScopeCS, "SendDataToClient:EnqueueBufferObj");
		EnqueueBufferObj(pSpckObjMap, newbuf, FALSE); 
	}
					
	if(pSzBuf)
	{
		delete[] pSzBuf; 
		pSzBuf = NULL;
	}
	
	int rc = 0; 
	
	
	{
		//CAutoLock ScopeLock ( m_ScopeCS, "SendDataToClient:SendPendingData");
		rc = SendPendingData(szFromID, pSpckObjMap);
	}
	
				
	if (rc == -1)  	
	{	
				
		if( m_bLog && m_bChkFilterIP )
		{
			if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_szIP ) == 0 )
			{
				CTime iDT =  CTime::GetCurrentTime();	
				char szPath [200];
				memset(szPath, 0x00, sizeof(szPath));
				sprintf(szPath,"\n�� %s :SendPendingData ERROR !! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_szIP, szUsage ); 
				m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
				
				CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData ERROR [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage ); 
			}
		}
		else if( m_bLog && !m_bChkFilterIP)
		{
			// ȭ�鿡 �Ѹ�.
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n�� %s :SendPendingData ERROR !! [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_szIP, szUsage ); //20061106					
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] SendPendingData ERROR [%s] usage [%s]\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage );    		
		}

		return -1;
	}	
	
	// LOGGING //////////////////////////////////////
	if( m_bLog && m_bChkFilterIP )
	{
		if	( strcmp( m_szFilterIP1 , pSpckObjMap->m_szIP ) == 0 || strcmp( m_szFilterIP2 , pSpckObjMap->m_szIP ) == 0 )
		{
			CTime iDT =  CTime::GetCurrentTime();	
			char szPath [200];
			memset(szPath, 0x00, sizeof(szPath));
			sprintf(szPath,"\n�� %s :SendDataToClient return 0(OK) [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_szIP, szUsage ); 
			m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
			
			CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ������ [%s] SendDataToClient return 0(OK)  usage [%s]\n\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage );			
		}
	}
	else if( m_bLog && !m_bChkFilterIP)
	{
		// ȭ�鿡 �Ѹ�.
		CTime iDT =  CTime::GetCurrentTime();	
		char szPath [200];
		memset(szPath, 0x00, sizeof(szPath));
		sprintf(szPath,"\n�� %s :SendDataToClient return 0(OK) [%s] usage [%s]", iDT.Format("%Y%m%d%H%M%S"), pSpckObjMap->m_szIP, szUsage ); //20061106					
		m_pNotifyProc((LPVOID) m_pDlg,szPath , NULL);
		
		CLog::Instance().WriteFile(FALSE, "whatareyoudoing", "Ln [%d] ������ [%s] SendDataToClient return 0(OK)  usage [%s]\n\n", 
						__LINE__ , pSpckObjMap->m_szIP, szUsage );
		
	}
	// LOGGING //////////////////////////////////////

	return 0;
}


// �̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢�
// DB �۾� ��û�� call back.. (1���� ��ȸ/ó���� SQL�� Transaction ó�� ���)
// �������� ����ü�� ����� ���� ������ PArsing�۾��� ���� �ʴ´�.
// (Ŭ���̾�Ʈ�� ���ӵ� ������ �޾Ƽ� ������ ó���Ѵ�) 
// �̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢̢�
void CIocpServer::SQLNotifyProc(	S_CALLBACKINFO* pSCallBackInfo  )										
{	
	CIocpServer* pTh = (CIocpServer*) pSCallBackInfo->m_pThread ;	
		
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
 	
	static BOOL( CIocpServer::* fPArySQLServer[] )(S_CALLBACKINFO* pSCallBackInfo ) = 
	{	// doDummySqlRsltWork �� �ϸ� -> ǥ�ؿ� ���ؼ� VC2008 ���ʹ� Compiler Error C3867 			
		&CIocpServer::doDummySqlRsltWork,					//  0
		&CIocpServer::doDummySqlRsltWork,					//  01  
		&CIocpServer::doDummySqlRsltWork,                 //  02  
		&CIocpServer::doSqlRsltWork_5,	                //  03  GET_BUDDY_GRP
		&CIocpServer::doSqlRsltWork_GET_BUDDY_LIST ,		//  04  GET_BUDDY_LIST 
		&CIocpServer::doSqlRsltWork_5,	                //  05  SEARCH_ID
		&CIocpServer::doSqlRsltWork_7,					//  06  RAWSQL_JOB_GN
		&CIocpServer::doSqlRsltWork_7,					//  07  RAWSQL_DAILYJOB_MST
		&CIocpServer::doDummySqlRsltWork,					//  08  RAWSQL_LOGIN => ���̻� ���ȵ�.  LOGIN ���.	
		&CIocpServer::doSqlRsltWork_7,					//  09  RAWSQL_GETUSER_INFO
		&CIocpServer::doSqlRsltWork_7,					//  10  RAWSQL_TEAM
		&CIocpServer::doSqlRsltWork_7,					//  11  RAWSQL_GETCOMPANY_INFO
		&CIocpServer::doSqlRsltWork_7,					//  12  RAWSQL_GETCLASS_INFO
		&CIocpServer::doSqlRsltWork_2,					//  13  RAWSQL_GETBUDDY_INFO
		&CIocpServer::doSqlRsltWork_2,					//  14  SOMEONE_ADD_ME
		&CIocpServer::doDummySqlRsltWork,					//  15  DEL_BUDDY_GRP
		&CIocpServer::doDummySqlRsltWork,					//  16  DEL_BUDDY
		&CIocpServer::doDummySqlRsltWork,					//  17  CHG_BUDDY_GRP	
		&CIocpServer::doSqlRsltWork_ADD_BUDDY_GRP,		//  18  ADD_BUDDY_GRP
		&CIocpServer::doDummySqlRsltWork,					//  19  CHG_GRP_NAME
		&CIocpServer::doDummySqlRsltWork,					//  20  YOUALLOWEDASCOMP
		&CIocpServer::doDummySqlRsltWork,					//  21  SOMEONE_DEL_ME 
		&CIocpServer::doDummySqlRsltWork,					//  22  ACK_NICK_CHG
		&CIocpServer::doDummySqlRsltWork,	            	//  23  SOMEONE_WANTS_SEND_FILES			
		&CIocpServer::doDummySqlRsltWork,					//  24  MULTICHAT_SERVER_ROOMNO  
		&CIocpServer::doDummySqlRsltWork,					//  25  CANCEL_FILE_TRANSFER
		&CIocpServer::doDummySqlRsltWork,					//  26  ALL_MSG
		&CIocpServer::doDummySqlRsltWork,					//  27  ALL_MSG_ACK	
		&CIocpServer::doDummySqlRsltWork,					//  28  FOLDER_SELECTING 
		&CIocpServer::doDummySqlRsltWork,					//  29  NO_THANKS_FILE_TRANSFER
		&CIocpServer::doDummySqlRsltWork,					//  30  RAWSQL_REG_NEW_USER
		&CIocpServer::doSqlRsltWork_9,					//  31  RAWSQL_SAVE_LAST_IP 
		&CIocpServer::doDummySqlRsltWork,					//  32  ALREADY_LOGIN 
		&CIocpServer::doDummySqlRsltWork,					//  33  LOGIN_ALLOWED
		&CIocpServer::doSqlRsltWork_9,					//  34  RAWSQL_INSERT_LOGONTIME
		&CIocpServer::doSqlRsltWork_9,					//  35  RAWSQL_CHK_DUP_ID
		&CIocpServer::doSqlRsltWork_8,					//  36  RAWSQL_CHK_USERID	
		&CIocpServer::doSqlRsltWork_9,					//  37  RAWSQL_UPDATE_PASSWD
		&CIocpServer::doDummySqlRsltWork,	    			//  38  ADD_BUDDY_LIST_ERR	
		&CIocpServer::doDummySqlRsltWork,					//  39  ACK_ONLINE	  		
		&CIocpServer::doDummySqlRsltWork,					//  40  ACK_OFFLINE
		&CIocpServer::doDummySqlRsltWork,					//  41  YOUALLOWEDASCOMP_INFORM	
		&CIocpServer::doSqlRsltWork_9,					//  42  RAWSQL_UPDATE_LOGOFFTIME 
		&CIocpServer::doDummySqlRsltWork,					//  43  ADD_BUDDY_LIST		
		&CIocpServer::doDummySqlRsltWork,					//  44  CHK_ALREADY_LOGIN
		&CIocpServer::doSqlRsltWork_CHNG_NICK,    		//  45  CHNG_NICK	
		&CIocpServer::doDummySqlRsltWork,					//  46  MULTICHAT_GET_ROOMNO
		&CIocpServer::doDummySqlRsltWork,	         		//  47  MULTI_CHAT_MSG
		&CIocpServer::doDummySqlRsltWork,					//  48  SET_MULTICHAT_SERVER_ROOMNO	
		&CIocpServer::doDummySqlRsltWork ,				//  49  EXIT_MULTI_CHAT
		&CIocpServer::doSqlRsltWork_5       ,				//  50  SLIP_MSG
		&CIocpServer::doSqlRsltWork_5        ,            //  51  GET_SLIP_MSG
		&CIocpServer::doDummySqlRsltWork,                 //  52  RAWSQL_ACK_SLIP_MSG
		&CIocpServer::doSqlRsltWork_4        ,            //  53  ADD_IT_COMP  
		&CIocpServer::doDummySqlRsltWork     ,            //  54  
		&CIocpServer::doSqlRsltWork_4    ,                //  55  
		&CIocpServer::doSqlRsltWork_GET_COMP_LIST_INIT ,  //  56 GET_COMP_LIST_INIT
		&CIocpServer::doDummySqlRsltWork  ,               //  57 STATUS_BUSY
		&CIocpServer::doSqlRsltWork_7		,				//  58 LOGIN 
		&CIocpServer::doSqlRsltWork_5      ,              //  59 FOOD_MENU
		&CIocpServer::doDummySqlRsltWork   ,              //  60 ALL_USER_ID_TEAM
		&CIocpServer::doSqlRsltWork_GET_ALL_USER_INIT,    //  61 GET_ALL_USER_INIT
		&CIocpServer::doDummySqlRsltWork,                 //  62  
		&CIocpServer::doSqlRsltWork_UPDATE_ALL_USER_INIT, //  63 UPDATE_ALL_USER_INIT
		&CIocpServer::doSqlRsltWork_ALERT_PENDING_WORK    //  64
	} ;
	
	(pTh->*fPArySQLServer[ atoi(pSCallBackInfo->m_szUsage)]) (pSCallBackInfo); 		
}




void CIocpServer::InformWhenDisconnected( CIocpNetObj * sock)
{
	// �������� ����ڸ� ��ȭ�����ϴ� ����ڿ��� �˸���.
	MAP_STR2VEC::iterator it;
	VEC_STR::iterator itV;			
	
	it = m_mapStr2VecCompanion.find(sock->m_szUserId);
	
	if(it != m_mapStr2VecCompanion.end() )  
	{	
		S_BUDDY_OFFLINE sOfflineBuddy;	
		memset(&sOfflineBuddy , 0x00, sizeof(sOfflineBuddy));			
		memcpy(sOfflineBuddy.m_szBuddyID , sock->m_szUserId, sizeof(sOfflineBuddy.m_szBuddyID));
		
		for (itV=((*it).second).begin(); itV != ((*it).second).end(); itV++) 
		{					
			CIocpNetObj *pSpckObjMap = NULL;
						
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
						
						// ����� ����			
						memcpy(sMSGHeader.m_szUsage, ACK_OFFLINE, sizeof(sMSGHeader.m_szUsage));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
						memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
						sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
						memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
						
						// ���۵����� ������
						pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
						memset(pSzBuf, NULL, sizeof(pSzBuf));		
						
						memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
						memcpy(pSzBuf + COMMON_HEADER_LENGTH , &sOfflineBuddy, sizeof(sOfflineBuddy)); //���� ��� + sOfflineBuddy
						
						nTotalLen = COMMON_HEADER_LENGTH + iLength; 
						
						BUFFER_OBJ *newbuf = NULL;
						newbuf = GetBufferObj(nTotalLen);
						
						memcpy(newbuf->buf, pSzBuf, nTotalLen);
											
						EnqueueBufferObj(pSpckObjMap, newbuf, FALSE); 					
						
						if(pSzBuf)
						{
							delete[] pSzBuf; 
							pSzBuf = NULL;
						}						
						
						//TRACEX("�������� -> ����ڿ��� �˸��� [%s]\n", (*itV).c_str() );					

						int rc = SendPendingData("FD_CLOSE #1", pSpckObjMap);							
											
						if (rc == -1) 
						{
							// �α׿��� �˸��ٰ� ���г� ��� ó��
							if(pSpckObjMap && pSpckObjMap)
							{
								CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] �� �α׿��� �˸��ٰ� ���г� ��� ID [%s]!\n", __LINE__, 
									pSpckObjMap->m_szUserId );
							}						
						}
					}
				}				
			}				
			
		}//for
	}

	//TEST
	//Sleep(3000);
					
	TRACE("������� [%s]\n", sock->m_szIP);
	CIocpNetObj *pa= NULL;	
	
		
	CAutoLock ScopeLock ( m_ScopeCS, "InformWhenDisconnected 2");

	CString strSockID ;
	strSockID.Format("%d", sock ->m_hSocket ) ;	
	
	// ������ ������ ����� ó��
	//{
	char szTmpIP[200];
	memset(szTmpIP, 0x00, sizeof(szTmpIP));
	sprintf(szTmpIP,"\n�� %s [%s] disconnected..", sock->m_szIP , sock->m_szUserId );
	m_pNotifyProc((LPVOID) m_pDlg,szTmpIP , NULL);						
			
	CLog::Instance().WriteFile(FALSE, "ServerLog", "Ln [%d] �� ������ ������ ����� ó�� Disconnected ip #2 [%s][%s]!\n", 
				__LINE__, sock->m_szIP , sock->m_szUserId );
						
	TRACE("������ ������ ����� ó�� ID [%s]\n", sock->m_szUserId);
			
	InterlockedDecrement(&gCurrentConnections);				
						
	m_pNotifyProc((LPVOID) m_pDlg, "", &gCurrentConnections);	
			
	
	if(mSockIdSockObjMap.Lookup(strSockID, ( CObject*& ) pa )) 
	{ 			
		mSockIdSockObjMap.RemoveKey(strSockID); 
		mIDIPMap.RemoveKey(sock->m_szUserId);

		closesocket( sock->m_hSocket	); 
		CLog::Instance().WriteFile(FALSE, "ServerLog", "�� [%d] ������ ������ ����� ó�� RemoveSocketObj !\n", __LINE__);
	}
}





int		CIocpServer::GetOnePacketLength(char *pData, int iLength)
{
	if(iLength < COMMON_HEADER_LENGTH)
		return -1;
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)pData;
	int nPacketLen = atoi(psHeader->m_szLength) + COMMON_HEADER_LENGTH ;
	//TRACE("��ü PAcket ���� [%d]\n", nPacketLen);
	return nPacketLen;
}
// ��� ������ �����ϴ� ��ü Packet ���� 
// => ���� ����� ���ԵǴ� ��� ��ü ���̿� ����� ���̸� ���ؾ���..

BOOL	CIocpServer::GetHeaderInfo(CIocpNetObj * pSockObj)
{
	if(pSockObj->m_nTotalBufLength < COMMON_HEADER_LENGTH)
		return -1;

	// ��� ������ copy
	memcpy( &(pSockObj->sPacketHeader),  (S_COMMON_HEADER *)pSockObj->m_pTotalBuff, sizeof(S_COMMON_HEADER));
	// ��� ���̸� �����ϴ� ��ü ������ ũ�� 
	pSockObj->nTotalOnePacketLen = atoi(pSockObj->sPacketHeader.m_szLength) + COMMON_HEADER_LENGTH ;		
	TRACE("��ü Packet ���� [%d]\n", pSockObj->nTotalOnePacketLen);		
	return TRUE;
}	



int	CIocpServer::GetStrByDelim(char* szSrc, char* szOut, const char cDeli)
{
	int nPos = 0;
	//char	ch = 0;	
	int	n_size = strlen(szSrc);
	for ( int i=0; i< n_size; i++ ) 
	{
		if ( szSrc[i] == cDeli ) 
		{
			strncpy(szOut , szSrc , nPos) ; // ��¹��ڿ�
			strncpy(szSrc , szSrc + nPos + 1, n_size - nPos) ; // Src�� ����~
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

void	CIocpServer::FreeBufferObj(BUFFER_OBJ *obj)
{
	HeapFree(GetProcessHeap(), 0, obj->buf);
	HeapFree(GetProcessHeap(), 0, obj);
}


/*
//int	CIocpServer::AliveChkWork()
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
//	// ����� ����			
//	memcpy(sMSGHeader.m_szUsage, IM_ALIVE, sizeof(sMSGHeader.m_szUsage));
//	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szLength)-1, iLength );		
//	memcpy(sMSGHeader.m_szLength, szTemp, sizeof(sMSGHeader.m_szLength));
//	sprintf(szTemp, "%0*d", sizeof(sMSGHeader.m_szCount)-1, 1); 
//	memcpy(sMSGHeader.m_szCount, szTemp, sizeof(sMSGHeader.m_szCount));
//	
//	CIocpNetObj *pSpckObjMap = NULL;		
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
//			// ���۵����� ������
//			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength]; 
//			memset(pSzBuf, NULL, sizeof(pSzBuf));		
//			
//			memcpy(pSzBuf , &sMSGHeader, COMMON_HEADER_LENGTH); //���� ���		
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



int	CIocpServer::AlertPendingWork()
{
	// ���� ������ ���� �޽��� ������ 
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
	//pSqlWork->SetLoginInfo ( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	

	//COracleWorkerMT * pSqlWork = new COracleWorkerMT( (LPCSTR)m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);	
		
	pSqlWork->SetSQL(szSql);	
	pSqlWork->SetID( ALERT_PENDING_WORK, "", "", "");	
	pSqlWork->setNotiProcThread(SQLNotifyProc , this); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	
    
	return 0;
}











///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TEST ONLY !!!!!!



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////









