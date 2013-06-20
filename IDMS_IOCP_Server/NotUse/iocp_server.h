
#if !defined(AFX_IOCP_SERVER_H_)
#define AFX_IOCP_SERVER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "utility.h"
#include "IDMS_ServerDlg.h"
#include"OraMultithrd.h"

typedef void (CALLBACK* NOTIFYPROC)(LPVOID, char*, long* nCode);
typedef vector <string> VEC_STR;
typedef map< string, vector <string>, less<string> > MAP_STR2VEC; 
typedef set<string, less<string> > SET_STR; 
typedef map< string, set <string>, less<string> > MAP_STR2SET; 

enum IO_TYPE
{
	IO_NONE,
	IO_READ,
	IO_WRITE
};

//class COverlapped : public CMemoryPool<COverlapped>
class COverlapped  
{
public:
	COverlapped()
	{
		ZeroMemory(&m_ol, sizeof(m_ol));
		m_io = IO_NONE;
		m_pObject = NULL;
	}
	
    OVERLAPPED m_ol;
    IO_TYPE m_io;	
	VOID* m_pObject;
};

//class CIocpNetObj : public CMemoryPool<CIocpNetObj>
class CIocpNetObj 
{
public:
	CIocpNetObj()
	{
		m_hSocket = NULL;
		ZeroMemory(m_szBuffer, sizeof(m_szBuffer));

		ZeroMemory(m_szAwayInfo, sizeof(m_szAwayInfo));
		ZeroMemory(m_szUserId, sizeof(m_szUserId));
		ZeroMemory(&sPacketHeader, sizeof(sPacketHeader));
		ZeroMemory(m_szIP, sizeof(m_szIP));

		m_pTotalBuff = NULL; 
		m_prolex = new COverlapped();
		m_prolex->m_pObject = this;
		m_prolex->m_io = IO_READ;
		m_nTotalBufLength = 0;
		nRecvedDataLen = 0;
		pending = NULL;
		pendingtail = NULL;
		ZeroMemory(&m_prolex->m_ol, sizeof(OVERLAPPED));
		memset( &sPacketHeader, 0x00, sizeof(sPacketHeader) ); 
	}
	
	~CIocpNetObj()
	{
		delete m_prolex;
	}
		
	COverlapped *m_prolex;	
	char                m_szBuffer [MAX_BUFF_SIZE] ; // 한번에 받을수있는 최대버퍼
	char*				m_pTotalBuff;       // 지금까지 누적해서 받은 전체 데이터를 가르킴.
	int					m_nTotalBufLength;  // 모아놓은(m_pTotalBuff) 데이터의 크기	
	char                m_szIP[15+1];
	char                m_szAwayInfo[1+1];
	char                m_szUserId[20+1];
	S_COMMON_HEADER		sPacketHeader;
	int					nTotalOnePacketLen; // 전달되는 데이터의 전체 길이
	int					nRecvedDataLen;     // 수신된 데이터 길이
	SOCKET m_hSocket;
	
	//전송할 데이터 큐.
	BUFFER_OBJ			*pending,       // List of pending buffers to be sent
						*pendingtail;   // Last entry in buffer list
	
};



class CIocpServer:public CObject
{
private:
	MAP_STR2VEC m_mapStr2VecCompanion;
	MAP_STR2SET m_mapStr2SetMultiChat;
	string		m_strAllUserList; //! 전체 사용자 목록 (공지사항에서 사용)
	
public:	
	
	CIocpServer(HWND p_hWnd_dlg);
	virtual ~CIocpServer();	
		
	//CString		m_strDBConn, m_strUser, m_strPassWd ;			
	HANDLE		_thread, _threadUpdateAlert;	
	HANDLE		_threadChkAlive;	
	volatile long		m_nMultiChatRoomNo;
	CIDMS_ServerDlg*	m_pDlg;	
	NOTIFYPROC			m_pNotifyProc;
	CMapStringToOb		mIPSockObjMap;  ///< SOCKET_OBJ Map ( key : IP )	
	CMapStringToOb		mIDIPMap;		///< ID->IP Map
	CMapStringToOb		m_delSupposedObjMap;
	BOOL m_bLog, m_bChkFilterIP;
	char m_szFilterIP1 [20];
	char m_szFilterIP2 [20];		
	CStringArray m_strAryForUpdate; //업데이트 안되는 사용자 공지용 
	CRITICAL_SECTION m_CS;
	CIocpNetObj* m_pObject ;
	int     makeUserList();
	int     UpdateAllUserList();
	int     makeAllUserList();
	int		Start();	
	//int		Stop(bool wait);	
	int     AlertPendingWork();		
	void	SetNotifyProc(NOTIFYPROC pNotifyProc)
	{
		m_pNotifyProc	= pNotifyProc;
	};	

	int		GetOnePacketLength(char *pData, int iLength);	
	BOOL	GetHeaderInfo(CIocpNetObj * pOverlappedObj);
	BOOL    ProcessRecvData(CIocpNetObj * pOverlappedObj);	
	
	BOOL    doRawSqlWork					( CIocpNetObj* pOverlappedObj ); 
	BOOL	doRawSqlMultiOneTranWork		( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_FSELING_ALLMACK_NOTHANKF	( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_CANCEL_FILE_TRANSFER		( CIocpNetObj* pOverlappedObj );
	BOOL	doWork_Chat						( CIocpNetObj* pOverlappedObj );
	BOOL	doWork_AllMsg					( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_SomeoneWantsSendFiles	( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_GetBuddyGrpList			( CIocpNetObj* pOverlappedObj );	
	BOOL    doWork_CHK_ALREADY_LOGIN		( CIocpNetObj* pOverlappedObj );
	BOOL	doWork_AddBuddyGrp				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_DelBuddyGrp				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_ChgBuddyGrp				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_ChngNic					( CIocpNetObj* pOverlappedObj );
	BOOL	doWork_ChgGrpName				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_SearchID					( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_GetBuddyList				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_AddBuddyList				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_AllowBuddy				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_DelBuddy					( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_STATUS_AWAY				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_STATUS_ONLINE			( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_STATUS_BUSY				( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_MultiChatGetRoomNo		( CIocpNetObj* pOverlappedObj );	
	BOOL	doWork_MultiChat				( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_EXIT_MULTI_CHAT			( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_SLIP_MSG					( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_GET_SLIP_MSG				( CIocpNetObj* pOverlappedObj );	
	BOOL    doWork_ADD_IT_COMP				( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_DEL_IT_COMP				( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_CHG_IT_COMP				( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_LOGIN					( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_FOOD_MENU				( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_ALL_USER_ID_TEAM			( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_Dummy					( CIocpNetObj* pOverlappedObj );
	BOOL    doWork_ALIVE					( CIocpNetObj* pOverlappedObj );
				
	BOOL    doSqlRsltWork_CHNG_NICK (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_2 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_ADD_BUDDY_GRP (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_4 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_5 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_BUDDY_LIST (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_7 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_8 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_9 (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_COMP_LIST_INIT    (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_ALL_USER_INIT (S_CALLBACKINFO* pSCallBackInfo);	
	BOOL    doSqlRsltWork_UPDATE_ALL_USER_INIT  (S_CALLBACKINFO* pSCallBackInfo);	
	BOOL    doSqlRsltWork_ALERT_PENDING_WORK    (S_CALLBACKINFO* pSCallBackInfo);	
	inline BOOL    doDummySqlRsltWork(S_CALLBACKINFO* pSCallBackInfo) { return TRUE; };	
	static void CALLBACK SQLNotifyProc( S_CALLBACKINFO* pSCallBackInfo ) ;	
	int				SendDataToClient(char * szUsage, char * szFromID, CIocpNetObj * pSpckObjMap, char* pData, int nLen);
	void			InformWhenDisconnected( CIocpNetObj * sock);	
	//void			SetLoginInfo(CString strConn, CString strUser, CString strPasswd);			
	BUFFER_OBJ*		GetBufferObj(int buflen);	
	void			FreeBufferObj(BUFFER_OBJ *obj);		
	void			FreeSocketObj(CIocpNetObj *obj);
	void			EnqueueBufferObj(CIocpNetObj *sock, BUFFER_OBJ *obj, BOOL AtHead);
	BUFFER_OBJ*		DequeueBufferObj(CIocpNetObj *sock);
	int				ReceivePendingData(CIocpNetObj *sockobj);
	int				SendPendingData(char* szUsage,CIocpNetObj *sock);		
	int		GetStrByDelim(char* szSrc, char* szOut, const char cDeli);		
};


#endif 
