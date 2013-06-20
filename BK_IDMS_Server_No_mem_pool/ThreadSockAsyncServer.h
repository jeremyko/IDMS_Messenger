
#if !defined(AFX_THREADSOCKASYNC_H_)
#define AFX_THREADSOCKASYNC_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include "utility.h"
#include "IDMS_ServerDlg.h"
#include"OraMultithrd.h"


typedef void (CALLBACK* NOTIFYPROC)(LPVOID, char*, long* nCode);

typedef vector <string> VEC_STR;
typedef map< string, vector <string>, less<string> > MAP_STR2VEC; 
typedef set<string, less<string> > SET_STR; 
typedef map< string, set <string>, less<string> > MAP_STR2SET; 

class CIpSockObjMapInfo:public CObject
{
public:	
	CIpSockObjMapInfo();
	~CIpSockObjMapInfo();

	SOCKET_OBJ* m_pSockObj;
	THREAD_OBJ* m_pThreadObj;		
};

class CThreadSockAsyncServer  
{
	friend unsigned __stdcall PThreadProc( void* pParam ); 
	

private:
	MAP_STR2VEC m_mapStr2VecCompanion;
	MAP_STR2SET m_mapStr2SetMultiChat;
	string		m_strAllUserList; //! 전체 사용자 목록 (공지사항에서 사용)
	
public:	
	
	CThreadSockAsyncServer(HWND p_hWnd_dlg);
	virtual ~CThreadSockAsyncServer();	

	char	m_strDBConn [255];
	char	m_strUser	[255];
	char	m_strPassWd [255];		
	
	HANDLE		_thread, _threadUpdateAlert;	
	HANDLE		_threadChkAlive;	

	volatile long		m_nMultiChatRoomNo;

	CIDMS_ServerDlg*	    m_pDlg;	
	NOTIFYPROC			m_pNotifyProc;
	
	CMapStringToOb		mSockIdSockObjMap;  ///< SOCKET_OBJ Map ( key : socket id )	
	CMapStringToOb		mIDIPMap;		///< ID->IP Map
	
	BOOL m_bLog, m_bChkFilterIP;
	char m_szFilterIP1 [20];
	char m_szFilterIP2 [20];

	//업데이트 안되는 사용자 공지용 
	CStringArray m_strAryForUpdate;
	
	CCriticalSectionLocalScope m_ScopeCS; // 지역범위 lock 
	
	int     testError();

	int     makeUserList();
	int     UpdateAllUserList();
	int     makeAllUserList();
	int		Start();	
	int		Stop(bool wait);	
	int     AlertPendingWork();	
	void     ChkSockAlive();
	int     LoadTest(int i);
	int		TestClose() ;//TEST
	
	void	SetNotifyProc(NOTIFYPROC pNotifyProc)
	{
		m_pNotifyProc	= pNotifyProc;
	};	

	int		GetOnePacketLength(char *pData, int iLength);	
	BOOL	GetHeaderInfo(SOCKET_OBJ * pSockObj);
	BOOL    ProcessRecvData(SOCKET_OBJ * pSockObj);	
	
	BOOL    doRawSqlWork					( SOCKET_OBJ* pSockObj ); 
	BOOL	doRawSqlMultiOneTranWork		( SOCKET_OBJ* pSockObj );
	BOOL    doWork_FSELING_ALLMACK_NOTHANKF	( SOCKET_OBJ* pSockObj );
	BOOL    doWork_CANCEL_FILE_TRANSFER		( SOCKET_OBJ* pSockObj );
	BOOL	doWork_Chat						( SOCKET_OBJ* pSockObj );
	BOOL	doWork_AllMsg					( SOCKET_OBJ* pSockObj );	
	BOOL	doWork_SomeoneWantsSendFiles	( SOCKET_OBJ* pSockObj );	
	BOOL	doWork_GetBuddyGrpList			( SOCKET_OBJ* pSockObj );	///< 대화상대 GRP목록 
	BOOL    doWork_CHK_ALREADY_LOGIN		( SOCKET_OBJ* pSockObj );
	BOOL	doWork_AddBuddyGrp				( SOCKET_OBJ* pSockObj );	///< 대화상대 GRP add	
	BOOL	doWork_DelBuddyGrp				( SOCKET_OBJ* pSockObj );	///< 대화상대 GRP Delete	
	BOOL	doWork_ChgBuddyGrp				( SOCKET_OBJ* pSockObj );	///< 대화상대 GRP 변경
	BOOL	doWork_ChngNic					( SOCKET_OBJ* pSockObj );
	BOOL	doWork_ChgGrpName				( SOCKET_OBJ* pSockObj );	///< GRP 이름 변경
	BOOL	doWork_SearchID					( SOCKET_OBJ* pSockObj );	
	BOOL	doWork_GetBuddyList				( SOCKET_OBJ* pSockObj );	///< 대화상대 목록 	
	BOOL	doWork_AddBuddyList				( SOCKET_OBJ* pSockObj );	///< 대화상대 추가
	BOOL	doWork_AllowBuddy				( SOCKET_OBJ* pSockObj );	///< 삭제 대화상대 다시 추가	
	BOOL	doWork_DelBuddy					( SOCKET_OBJ* pSockObj );	///< 대화상대 삭제		
	BOOL	doWork_STATUS_AWAY				( SOCKET_OBJ* pSockObj );	///< 대화상대 자리비움
	BOOL	doWork_STATUS_ONLINE			( SOCKET_OBJ* pSockObj );	///< 대화상대 자리비움 해제
	BOOL	doWork_STATUS_BUSY				( SOCKET_OBJ* pSockObj );	///< 대화상대 다른용무중
	BOOL	doWork_MultiChatGetRoomNo		( SOCKET_OBJ* pSockObj );	///< Multi Chat Room No Get
	BOOL	doWork_MultiChat				( SOCKET_OBJ* pSockObj );
	BOOL    doWork_EXIT_MULTI_CHAT			( SOCKET_OBJ* pSockObj );
	BOOL    doWork_SLIP_MSG					( SOCKET_OBJ* pSockObj );
	BOOL    doWork_GET_SLIP_MSG				( SOCKET_OBJ* pSockObj );	
	BOOL    doWork_ADD_IT_COMP				( SOCKET_OBJ* pSockObj );
	BOOL    doWork_DEL_IT_COMP				( SOCKET_OBJ* pSockObj );
	BOOL    doWork_CHG_IT_COMP				( SOCKET_OBJ* pSockObj );
	BOOL    doWork_LOGIN					( SOCKET_OBJ* pSockObj );
	BOOL    doWork_FOOD_MENU				( SOCKET_OBJ* pSockObj );
	BOOL    doWork_ALL_USER_ID_TEAM			( SOCKET_OBJ* pSockObj );
	BOOL    doWork_Dummy					( SOCKET_OBJ* pSockObj );
	BOOL    doWork_ALIVE					( SOCKET_OBJ* pSockObj );
				
	BOOL    doSqlRsltWork_CHNG_NICK(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_2(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_ADD_BUDDY_GRP(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_4(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_5(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_BUDDY_LIST (S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_7(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_8(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_9(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_COMP_LIST_INIT(S_CALLBACKINFO* pSCallBackInfo);
	BOOL    doSqlRsltWork_GET_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo);	
	BOOL    doSqlRsltWork_UPDATE_ALL_USER_INIT(S_CALLBACKINFO* pSCallBackInfo);	
	BOOL    doSqlRsltWork_ALERT_PENDING_WORK(S_CALLBACKINFO* pSCallBackInfo);

	BOOL    doSqlRsltWork_LOAD_TEST(S_CALLBACKINFO* pSCallBackInfo); //TEST ONLY
	

	inline BOOL    doDummySqlRsltWork(S_CALLBACKINFO* pSCallBackInfo)
	{
		return TRUE;
	};
	
	static void CALLBACK SQLNotifyProc( S_CALLBACKINFO* pSCallBackInfo ) ;
		
	int   SendDataToClient(char * szUsage, char * szFromID, CIpSockObjMapInfo * pSpckObjMap, char* pData, int nLen);
	void CThreadSockAsyncServer::InformWhenDisconnected(THREAD_OBJ *thread, SOCKET_OBJ * sock);
	int			_init();		
	unsigned	_run();	
		
	int			_do_recv(SOCKET_OBJ * sock);
		
	void		SetLoginInfo(const char* strConn, const  char* strUser, const char* strPasswd);		

	struct addrinfo*	ResolveAddress(const char *addr, const char *port, int af, int type, int proto);

	BUFFER_OBJ *	GetBufferObj(int buflen);
	
	void	FreeBufferObj(BUFFER_OBJ *obj);	

	SOCKET_OBJ *	GetSocketObj(SOCKET s, int listening);
	void			FreeSocketObj(SOCKET_OBJ *obj);
	THREAD_OBJ *	GetThreadObj(int nLine);
	
	void	FreeThreadObj(THREAD_OBJ *thread);	

	int				InsertSocketObj(THREAD_OBJ *thread, SOCKET_OBJ *sock);
	void			RemoveSocketObj(THREAD_OBJ *thread, SOCKET_OBJ *sock);
	SOCKET_OBJ *	FindSocketObj(THREAD_OBJ *thread, int index);
	void			EnqueueBufferObj(SOCKET_OBJ *sock, BUFFER_OBJ *obj, BOOL AtHead);
	BUFFER_OBJ *	DequeueBufferObj(SOCKET_OBJ *sock);
	int				ReceivePendingData(SOCKET_OBJ *sockobj);
	int				SendPendingData(char* szUsage,SOCKET_OBJ *sock);	
	int				HandleIo(THREAD_OBJ *thread, SOCKET_OBJ *sock);
	void			RenumberThreadArray(THREAD_OBJ *thread);
		
	THREAD_OBJ *    AssignToFreeThread(SOCKET_OBJ *sock);
	
	int		GetStrByDelim(char* szSrc, char* szOut, const char cDeli);		
};

#endif 
