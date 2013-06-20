// 작성자 : 고정현
// DaimsSessionProc.h: interface for the CChatSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__CHATSESSION__)
#define __CHATSESSION__

#if _MSC_VER > 1000

#pragma once
#endif // _MSC_VER > 1000

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "..\\common\\utility.h"
#include "ThreadSockAsync.h"
//#include "IdmsMsnDlg.h"

class CIdmsMsnDlg;
typedef vector <string> VEC_STR;
typedef map< string, set <string>, less<string> > MAP_STR2SET; 

//class CIdmsMsnDlg;
class CChatSession: public CThreadSockAsync 
{
	DECLARE_DYNCREATE(CChatSession)
private:		
	CChatSession();
	CChatSession(HWND hOwnerWnd);	
public:	
	
	virtual ~CChatSession();	
	BOOL SocketInit(CString strIP, BOOL bStartThread = TRUE);	
	BOOL SocketInit(BOOL bStartThread = TRUE);	
	virtual BOOL ProcessRecvData(char* data, int nPacketLen);
	virtual void ProcessWhenConnClosed();
	
	//! 이미 로그인 했는지 여부 확인
	//BOOL RequestCheckAlreadyLogIn(const char* szUserID); 

	//! SQL 문장을 직접 서버로 전송
	BOOL RequestRawSQL( const char* szUsage, const char* szSQL , int nLen);	
	BOOL RequestLogIn(const CString& strID);
	//! 대화상대목록
	BOOL RequestBuddyList(LPCTSTR szUserID);	
	//! 사용자 인증	
	BOOL RequestLogin(LPCTSTR szUserID, LPCTSTR szPassword); 	
	//! 대화상대 Grp 목록
	BOOL RequestBuddyGrp(const CString &  strLoginID);
	//! 대화상대 목록
	BOOL RequestBuddyList(const CString &  strLoginID);
	//! GRP 삭제
	BOOL RequestDelGroups(const char*  szDelGrps);	
	//! 대화상대의 GRP 변경
	BOOL RequestChgBuddyGroup(const char*  szChgGrpBuddy);		
	//! GRP 이름 변경
	BOOL RequestChgGroupName(const char*  szBefAftGrpNames);		
	//! 대화상대 ADD
	BOOL RequestAddCompanion(LPCTSTR szCompanionID, LPCTSTR szCompanionGrp);
	//! 대화상대 DELETE
	BOOL RequestDelBuddys(const char* szDelIDs);
	BOOL RequestAddGroup(LPCTSTR szCompanionGrp);		
	BOOL SendMyStatusInfo(const char* szUsage);
	BOOL RequestExitMultiChat (const char* szServerRoomNo);		
	CStringArray* getGrpNames();
	BOOL AckYouAllowedAsCompanion (const char* szToID, const char*  szChgGrpBuddy);
	BOOL SendAllMsgAck( const char* szToID, const char* szMyNick, const char* szMyRealName);
	BOOL RequestChgNick( const char*  szNickBef, const char*  szChgedNick);		
	BOOL RequestLogInID( const char*  szID, const char*  szName);			
	BOOL RequestMultiChatRoomNo( const char* szDlgID, CString& strIDNames); ///< Multi Chat 	
	BOOL send_WriteSlipMsg(const char* szToID, const char* szSlipMsg);
	BOOL RequestSlipMsg();
	BOOL SendAlive();
	BOOL RequestAddCompIT(const char* szDesc , const char* szUrl);	
	BOOL RequestDelITComp(const char* szITName);	
	BOOL RequestChgCompIT( const char* szDescOld, const char* szDesc, const char* szUrl ) ;
	BOOL RequestFoodMenu(const char* szFromDate, const char* szToDate); // 메뉴
	
	//static CString getFileNameOnly(CString strFileWithPath);
	//static CString getFileNamePathExceptRoot(CString strFileWithPath, CString strRootPath);

	BOOL doWork_STATUS_AWAY (char* data);
	BOOL doWork_STATUS_ONLINE (char* data);
	BOOL doWork_STATUS_BUSY(char* data);
	BOOL doWork_ChatMsg (char* data);
	BOOL doWork_GetBuddyGrp (char* data);
	BOOL doWork_GetBuddyList(char* data);	
	BOOL doWork_SearchID (char* data);
	BOOL doWork_RAWSQL_JOB_GN (char* data);
	BOOL doWork_RAWSQL_DAILYJOB_MST (char* data);
	BOOL doWork_LOGIN (char* data);
	BOOL doWork_RAWSQL_GETUSER_INFO (char* data);
	BOOL doWork_GetTeamInfo( char* data) ;
	BOOL doWork_BuddyInfoAddMe( char* data) ;
	BOOL doWork_DEL_BUDDY_GRP( char* data) ;
	BOOL doWork_DEL_BUDDYS( char* data) ;
	BOOL doWork_CHG_BUDDY_GRP( char* data) ;
	BOOL doWork_ADD_BUDDY_GRP( char* data) ;
	BOOL doWork_CHG_GRP_NAME( char* data) ;
	BOOL doWork_YOUALLOWEDASCOMP( char* data) ;
	BOOL doWork_SOMEONE_DEL_ME( char* data) ;
	BOOL doWork_ACK_NICK_CHG( char* data);	
	BOOL doWork_MULTICHAT_SERVER_ROOMNO( char* data);	
	BOOL doWork_ALL_MSG ( char* data);
	BOOL doWork_ALL_MSG_ACK ( char* data);	
	BOOL doWork_RAWSQL_REG_NEW_USER( char* data);
	BOOL doWork_RAWSQL_SAVE_LAST_IP( char* data);
	BOOL doWork_ALREADY_LOGIN( char* data);
	BOOL doWork_LOGIN_ALLOWED( char* data);
	BOOL doWork_RAWSQL_INSERT_LOGONTIME( char* data);
	BOOL doWork_RAWSQL_CHK_DUP_ID( char* data);
	BOOL doWork_RAWSQL_CHK_USERID( char* data);
	BOOL doWork_RAWSQL_UPDATE_PASSWD( char* data);
	BOOL doWork_ADD_BUDDY_LIST_ERR( char* data);
	BOOL doWork_ACK_ONLINE( char* data);
	BOOL doWork_ACK_OFFLINE( char* data);
	BOOL doWork_YOUALLOWEDASCOMP_INFORM( char* data);
	BOOL doWork_MULTI_CHAT_MSG( char* data);
	BOOL doWork_SET_MULTICHAT_SERVER_ROOMNO( char* data);
	BOOL doWork_EXIT_MULTI_CHAT( char* data);
	BOOL doWork_Dummy(char* szRawData);
	BOOL doWork_SLIP_MSG(char* szRawData);
	BOOL doWork_GET_SLIP_MSG(char* szRawData);	
	BOOL doWork_ADD_IT_COMP(char* szRawData);	
	BOOL doWork_DEL_IT_COMP(char* szRawData);	
	BOOL doWork_CHG_IT_COMP(char* szRawData);	
	BOOL doWork_FOOD_MENU(char* szRawData);
	BOOL doWork_ALL_USER_ID_TEAM(char* szRawData);
	BOOL doWork_ALIVE(char* szRawData);
	
public:	
	plugware::CCriticalSectionLocalScope m_criSecLocalScopeBuddyInfo;	
	MAP_STR2SET m_mapStr2SetMultiChat;
	VEC_STR m_vecUserIDOrdered;
	CMapStringToOb		mBuddyInfoMap;	// ID -> 대화상대 정보 MAP
	CMapStringToString  mIDToNameMap  ;  // ID -> NAME 정보 MAP
	CMapStringToString  mITCompDescToUrlMap ; // IT COMP Desc-> URL Map
	char m_szMyUserID   [20 + 1];
	char m_szMyIPAddr   [15 + 1];
	char m_szMyNickName	[C_NICK_NAME_LEN + 1];
	char m_szRealName	[20 + 1];
	CIdmsMsnDlg* m_pDlgMain;
	CStringArray m_StrAryGrpName;
	CFont	m_Font;
	COLORREF m_ColorBG, m_ColorBuddy, m_ColorMe ;
	CBuddyInfo	mMyInfo;
	CString		m_strMyNick;
	int m_AwaySec;
	CHARFORMAT2 m_cf;

	CString m_LoginID;	
	CString m_LoginName;
	CString m_LoginTeamCd;
	CString m_LoginTeamName;
	CString m_TopTeamCode;
	CString m_TopTeamName;
	CString m_Grade;	
	CString m_Nick;
	CString m_decrpPassWd; ///> 자동 로그인시 사용할 암호회된 비밀번호
	CString m_strIAMS_AUTHORITY;
	CString m_strPROG_SELECT;

	BOOL    m_bDailyJob;	
	char    szChatDlgTitleName [255];
	char    szCurChatDlgTitleName [255];
	char    m_szBuddyImgID[20+14+5+1]; // xxx20061109162810.png
	char    m_szMeImgID[20+14+5+1] ;   // jeremyko20061109162810.jpeg
	//Bitmap* m_pMeImage; // GDI+

public:
		
	static CChatSession& Instance() ;
	
	static void DeleteObject();
	void SetOwnerWnd(HWND hOwnerWnd);
	void SetMainDlg(CIdmsMsnDlg* pDlgMain);	
	void setMyID(char* szMyID);
	void setMyIpAddr(char* szMyIpAddr);
	void SetTimeOut(LPCTSTR szFunctionID, int nTimeOutSecond);	// TimeOut 설정 담당 함수		
};

#endif 
