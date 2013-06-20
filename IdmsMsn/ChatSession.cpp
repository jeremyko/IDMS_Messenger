// �ۼ��� : ������
// DaimsSessionProc.cpp: implementation of the CChatSession class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChatSession.h"
#include "INIManager.h"
#include "IdmsMsnDlg.h"
#include "MsgBoxThread.h"
#include "FileSendDlg.h"
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//static member init ..
IMPLEMENT_DYNCREATE(CChatSession, CThreadSockAsync)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatSession::CChatSession()
{
	m_bDailyJob = FALSE;	
	memset(szChatDlgTitleName,0x00,sizeof(szChatDlgTitleName));
	memset(szCurChatDlgTitleName,0x00,sizeof(szCurChatDlgTitleName));
	//m_pMeImage = NULL; //GDI+

	CString strPath;
	CINIManager::Instance().GetValue(INI_MY_IMAGE_PATH, strPath);
	if(strPath.GetLength() == 0)
	{
		//m_pMeImage = NULL; //GDI+
	}
	else
	{
		//m_pMeImage  = Bitmap::FromFile( strPath.AllocSysString() ); //GDI+
	}
}

CChatSession::~CChatSession()
{
	//m_bDailyJob = FALSE;
	TRACE("~CChatSession()\n\n");	
}

CChatSession& CChatSession::Instance()
{
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	static CChatSession obj(pMainWnd->GetSafeHwnd());
	
	return obj;
}

CChatSession::CChatSession(HWND hOwnerWnd)
{		
	m_hOwnerWnd = hOwnerWnd;	
	m_bDailyJob = FALSE;
	mbConnected = FALSE;	
	//m_pMeImage = NULL; //GDI+	
	m_strMyNick = "";
	m_AwaySec = 180;

	CString strPath;
	CINIManager::Instance().GetValue(INI_MY_IMAGE_PATH, strPath);
	
	if(strPath.GetLength() == 0)
	{
		//m_pMeImage = NULL;  //GDI+
	}
	else
	{
		//m_pMeImage  = Bitmap::FromFile( strPath.AllocSysString() ); //GDI+
	}
}


void CChatSession::DeleteObject()
{	
	TRACE("CChatSession::DeleteObject() \n");	

	
}

BOOL CChatSession::SocketInit(BOOL bStartThread)
{	
	char szIP[15+1], szPort[10];
	memset(szIP,0x00,sizeof(szIP));
	memset(szPort,0x00,sizeof(szPort));

	int nServerPort = CINIManager::Instance().GetValueInt(INI_SERVER_PORT);
	CString strIP;
	CINIManager::Instance().GetValue(INI_SERVER_IP, strIP);	
	
	::ResetEvent(m_kill_event);
	::ResetEvent(m_work_event);
	
	
	BOOL b_Rtn = SocketConnect( nServerPort, (LPCSTR)strIP, bStartThread);
	
	if(b_Rtn)
	{
		mbConnected = TRUE;			
	}
	else
	{
		// 20081107 
		CString strNewIP = "10.225.231.225" ;
		b_Rtn = SocketConnect( nServerPort, (LPCSTR) strNewIP, bStartThread);
		
		if(b_Rtn)
		{
			mbConnected = TRUE;	
			CINIManager::Instance().SetValue(INI_SERVER_IP, (LPSTR) (LPCTSTR) strNewIP );
			CINIManager::Instance().SaveINIFile();
		}
	}
		
	return b_Rtn;
}


BOOL CChatSession::SocketInit(CString strIP, BOOL bStartThread)
{	
	int nServerPort = CINIManager::Instance().GetValueInt(INI_SERVER_PORT);
	
	BOOL b_Rtn = SocketConnect(nServerPort, strIP, bStartThread);

	if(b_Rtn)
	{
		mbConnected = TRUE;	
	}	
		
	return b_Rtn;
}



// TimeOut ���� ��� �Լ�
void CChatSession::SetTimeOut(LPCTSTR szFunctionID, int nTimeOutSecond)
{	
	//if(::IsAvailableWindow(m_hOwnerWnd) == TRUE)
	//	::SendMessage(m_hOwnerWnd, WM_SET_TIMEOUT, (WPARAM)&sSOM, NULL);
}

void CChatSession::setMyIpAddr(char* szMyIpAddr)
{
	memset(m_szMyIPAddr, 0x00, sizeof(m_szMyIPAddr));
	strncpy(m_szMyIPAddr,szMyIpAddr, sizeof(m_szMyIPAddr));
}

void CChatSession::setMyID(char* szMyID)
{
	memset(m_szMyUserID, 0x00, sizeof(m_szMyUserID));
	strncpy(m_szMyUserID,szMyID, sizeof(m_szMyUserID));
}

void CChatSession::SetOwnerWnd(HWND hOwnerWnd)
{
	m_hOwnerWnd = hOwnerWnd;
}

void CChatSession::SetMainDlg(CIdmsMsnDlg* pDlgMain)
{
	m_pDlgMain = pDlgMain;
}
	
BOOL CChatSession::RequestFoodMenu(const char* szFromDate, const char* szToDate)
{
	//S_FOOD_MENU_PERIOD
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	S_FOOD_MENU_PERIOD sFoodMenuPeriod;

	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(&sFoodMenuPeriod, NULL, sizeof(sFoodMenuPeriod));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = sizeof(S_FOOD_MENU_PERIOD) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, FOOD_MENU, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	memcpy(sFoodMenuPeriod.m_szFromDate , szFromDate, sizeof(sFoodMenuPeriod.m_szFromDate));
	memcpy(sFoodMenuPeriod.m_szToDate , szToDate, sizeof(sFoodMenuPeriod.m_szToDate));
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, &sFoodMenuPeriod, sizeof(S_FOOD_MENU_PERIOD));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestFoodMenu") < 0)	
	{
		TRACEX("CChatSession::RequestFoodMenu -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestLogIn(const CString& strID)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	char szTemp[20];
	CString strMsg;

	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(szTemp, NULL, sizeof(szTemp));	
		
	S_FROMTOID_HEADER sFromIPIDHeader;		
	memset(&sFromIPIDHeader, NULL, sizeof(sFromIPIDHeader));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH + 1; 
	
	memcpy(sComHeader.m_szUsage, LOGIN, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	//writeLogFile("c:\\ClientLog.log", "m_szMyIPAddr[%s]\n" , m_szMyIPAddr);	

	memcpy(sFromIPIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIPIDHeader.m_szIP), strlen(m_szMyIPAddr))); 	
	memcpy(sFromIPIDHeader.m_szFromID, strID, min(sizeof(sFromIPIDHeader.m_szFromID), strlen(strID))); 	
	
	// ���۵����� ������
	// RawSQL �� ���� NULL�� �����ϴ� ���ڿ��� ���޵�.
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + FROMIP_HEADER_LENGTH + iLength  ];  
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIPIDHeader, FROMTOID_HEADER_LENGTH );	
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;	

	/////////////////////////////////////////////////////////////////////////////////////////

	if(Send(pSzBuf, nTotalLen,"RequestLogIn") < 0)	
	{		
		TRACE("CChatSession::RequestLogIn -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestRawSQL(const char* szUsage , const char* szSQL, int iLength)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	char szTemp[20];
	CString strMsg;

	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(szTemp, NULL, sizeof(szTemp));	
	if	( 
			strcmp(szUsage, RAWSQL_CHK_DUP_ID) == 0		|| 
			strcmp(szUsage, RAWSQL_CHK_USERID) == 0		|| 			
			strcmp(szUsage, RAWSQL_GETUSER_INFO) == 0	||
			strcmp(szUsage, RAWSQL_TEAM) == 0			||
			strcmp(szUsage, RAWSQL_GETCOMPANY_INFO) == 0	||
			strcmp(szUsage, RAWSQL_GETCLASS_INFO) == 0	|| 
			strcmp(szUsage, RAWSQL_UPDATE_PASSWD) == 0	 ||
			//strcmp(szUsage, RAWSQL_LOGIN) == 0			|| 
			strcmp(szUsage, RAWSQL_SAVE_LAST_IP) == 0	|| 
			strcmp(szUsage, RAWSQL_JOB_GN) == 0	|| 
			strcmp(szUsage, RAWSQL_INSERT_LOGONTIME) == 0	|| 			
			strcmp(szUsage, RAWSQL_DAILYJOB_MST) == 0	|| 			
			strcmp(szUsage, RAWSQL_REG_NEW_USER) == 0			
		)
	{
		// RawSQL �� ��� �α��� ���� USERID�� ���� ��� S_FROMIP_HEADER ����Ѵ�.
		S_FROMIP_HEADER sFromIPHeader;		
		memset(&sFromIPHeader, NULL, sizeof(sFromIPHeader));	
				
		// ����� ����
		//RawSQL �� ���� NULL�� �����ϴ� ���ڿ��� ���޵�.
		iLength = iLength + FROMIP_HEADER_LENGTH + 1; 
		
		memcpy(sComHeader.m_szUsage, szUsage, sizeof(sComHeader.m_szUsage));		
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
		memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
		memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
			
		memcpy(sFromIPHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIPHeader.m_szIP), strlen(m_szMyIPAddr))); 
		
		// ���۵����� ������
		// RawSQL �� ���� NULL�� �����ϴ� ���ڿ��� ���޵�.
		
		pSzBuf = new char [COMMON_HEADER_LENGTH + FROMIP_HEADER_LENGTH + iLength  ];  // +1 => NULL
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
		memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIPHeader, FROMIP_HEADER_LENGTH );

		memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMIP_HEADER_LENGTH, szSQL, iLength);	
		nTotalLen = COMMON_HEADER_LENGTH + iLength;	
	}	
	else 	
	{
		// ����, �α��� ���̵�� ���� ������ ������.
		S_FROMTOID_HEADER sFromIDHeader;		
		memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
				
		// ����� ����
		//RawSQL �� ���� NULL�� �����ϴ� ���ڿ��� ���޵�.
		iLength = iLength + FROMTOID_HEADER_LENGTH + 1; 
		
		TRACEX("szUsage [%s]\n",szUsage );

		memcpy(sComHeader.m_szUsage, szUsage, sizeof(sComHeader.m_szUsage));		
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
		memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
		memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
			
		memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
		memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
		memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
		
		// ���۵����� ������
		// RawSQL �� ���� NULL�� �����ϴ� ���ڿ��� ���޵�.
		TRACE("--------- szUsage [%s]\n", szUsage );
		pSzBuf = new char [COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH + iLength  ];  // +1 => NULL
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
		memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );

		memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szSQL, iLength);	
		nTotalLen = COMMON_HEADER_LENGTH + iLength;	
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////

	if(Send(pSzBuf, nTotalLen,szUsage) < 0)	
	{
		TRACE("CChatSession::RequestRawSQL -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

// �ڸ���� ó��
BOOL CChatSession::SendMyStatusInfo(const char* szUsage)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH ; 		
	
	memcpy(sComHeader.m_szUsage, szUsage, sizeof(sComHeader.m_szUsage));
	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromToIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromToIDHeader.m_szFromID), strlen(m_szMyUserID))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"SendMyStatusInfo") < 0)	
	{
		TRACEX("CChatSession::SendAwayInfo -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}

// ���ȵ�

//BOOL CChatSession::RequestCheckAlreadyLogIn(const char* szUserID)
//{
//	char* pSzBuf = NULL;
//	S_COMMON_HEADER sComHeader;			
//	int nTotalLen = 0;	
//	S_FROMTOID_HEADER sFromToIDHeader;
//	char szTemp[20];
//	memset(&sComHeader, NULL, sizeof(sComHeader));		
//	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
//	memset(szTemp, NULL, sizeof(szTemp));	
//				
//	// ����� ����	
//	int iLength = FROMTOID_HEADER_LENGTH ; 
//		
//	memcpy(sComHeader.m_szUsage, CHK_ALREADY_LOGIN, sizeof(sComHeader.m_szUsage));		
//	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
//	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
//	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
//	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
//	
//	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
//	memcpy(sFromToIDHeader.m_szFromID, szUserID, min(sizeof(sFromToIDHeader.m_szFromID), strlen(szUserID))); 
//		
//	// ���۵����� ������			
//	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
//	memset(pSzBuf, NULL, sizeof(pSzBuf));	
//	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
//	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
//	
//	nTotalLen = COMMON_HEADER_LENGTH + iLength;		
//
//	if(Send(pSzBuf, nTotalLen,"RequestCheckAlreadyLogIn") < 0)	
//	{
//		TRACEX("CChatSession::RequestCheckAlreadyLogIn -> Send return FAIL!\n");		
//		return FALSE ;
//	}
//	
//	return TRUE;
//}
	


// ��ȭ��� Grp ���
BOOL CChatSession::RequestBuddyGrp(const CString &  strLoginID)
{
	// GET_BUDDY_GRP
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, GET_BUDDY_GRP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen( (LPCSTR)m_szMyIPAddr))); 
	memcpy(sFromToIDHeader.m_szFromID, (LPCSTR)strLoginID, min(sizeof(sFromToIDHeader.m_szFromID), strlen((LPCSTR)strLoginID))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	TRACE("CChatSession::RequestBuddyGrp -> Send!\n");	

	//LogReconn
//	CTime iDT = CTime::GetCurrentTime();
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "RequestBuddyGrp [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

	if(Send(pSzBuf, nTotalLen,"RequestBuddyGrp") < 0)	
	{
		TRACEX("CChatSession::RequestBuddyGrp -> Send return FAIL!\n");	
		
		//LogReconn
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "RequestBuddyGrp Send Fail [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

		return FALSE ;
	}
	
	//Log
	
	
	

	return TRUE;
}


BOOL CChatSession::RequestAddCompIT(const char* szDesc, const char* szUrl )
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	S_ADD_IT_COMP sAddItComp;

	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(&sAddItComp, NULL, sizeof(sAddItComp));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = sizeof(S_ADD_IT_COMP) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, ADD_IT_COMP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	memcpy(sAddItComp.m_szDesc , szDesc, sizeof(sAddItComp.m_szDesc));
	memcpy(sAddItComp.m_szUrl , szUrl, sizeof(sAddItComp.m_szUrl));
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, &sAddItComp, sizeof(S_ADD_IT_COMP));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestAddCompIT" ) < 0)	
	{
		TRACEX("CChatSession::RequestAddCompIT -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestChgCompIT( const char* szDescOld, const char* szDesc, const char* szUrl ) 
{
	//S_CHG_IT_COMP
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	S_CHG_IT_COMP sChgItComp;

	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(&sChgItComp, NULL, sizeof(sChgItComp));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = sizeof(S_CHG_IT_COMP) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, CHG_IT_COMP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	memcpy(sChgItComp.m_szDescOld , szDescOld, sizeof(sChgItComp.m_szDescOld));
	memcpy(sChgItComp.m_szDesc , szDesc, sizeof(sChgItComp.m_szDesc));
	memcpy(sChgItComp.m_szUrl , szUrl, sizeof(sChgItComp.m_szUrl));
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, &sChgItComp, sizeof(S_CHG_IT_COMP));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestChgCompIT") < 0)	
	{
		TRACEX("CChatSession::RequestChgCompIT -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestDelITComp(const char* szITName)
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szITName) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, DEL_IT_COMP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szITName, strlen(szITName));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestDelITComp") < 0)	
	{
		TRACEX("CChatSession::RequestDelITComp -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::send_WriteSlipMsg(const char* szToID, const char* szSlipMsg)
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szSlipMsg) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, SLIP_MSG, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, szToID, sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szSlipMsg, strlen(szSlipMsg));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"send_WriteSlipMsg") < 0)	
	{
		TRACEX("CChatSession::send_WriteSlipMsg -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}


BOOL CChatSession::SendAlive()
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, IM_ALIVE, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );	
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"SendAlive") < 0)	
	{
		TRACEX("CChatSession::SendAlive -> Send return FAIL!\n");		
		return FALSE ;
	}

	/*
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "SendAlive Send OK [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );
	*/

	return TRUE;
}



BOOL CChatSession::RequestSlipMsg()
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, GET_SLIP_MSG, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "VER200807102008", sizeof(sFromIDHeader.m_szToID) ); // 20080521, 20080707
	//memcpy(sFromIDHeader.m_szToID, "VERSION_CHK", sizeof(sFromIDHeader.m_szToID) ); // 20080521, 20080707
	
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );	
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestSlipMsg") < 0)	
	{
		TRACEX("CChatSession::RequestSlipMsg -> Send return FAIL!\n");		
		return FALSE ;
	}

	//Log
//	CTime iDT =  CTime::GetCurrentTime();
//	CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
//	writeLogFile("c:\\ClientLog.log", "RequestSlipMsg Send [%s]\n" , strtmDetailed);


	return TRUE;
}


BOOL CChatSession::RequestDelGroups(const char*  szDelGrps)
{
	// DEL_BUDDY_GRP
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szDelGrps) + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, DEL_BUDDY_GRP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szDelGrps, strlen(szDelGrps));	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestDelGroups") < 0)	
	{
		TRACEX("CChatSession::RequestDelGroups -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestChgBuddyGroup( const char*  szChgGrpBuddy)
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szChgGrpBuddy)  + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, CHG_BUDDY_GRP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szChgGrpBuddy, strlen(szChgGrpBuddy)  );	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestChgBuddyGroup") < 0)	
	{
		TRACEX("CChatSession::RequestChgBuddyGroup -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}
	
BOOL CChatSession::RequestChgGroupName(const char*  szBefAftGrpNames)
{
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szBefAftGrpNames)  + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, CHG_GRP_NAME, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szBefAftGrpNames, strlen(szBefAftGrpNames)  );	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestChgGroupName") < 0)	
	{
		TRACEX("CChatSession::RequestChgGroupName -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}


BOOL CChatSession::RequestDelBuddys(const char* szDelIDs)
{
	// DEL_BUDDY
	char szTemp[20];
	char* pSzBuf = NULL;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// ����� ����	
	int iLength = strlen(szDelIDs) /*strDelIDs.GetLength()*/ + FROMTOID_HEADER_LENGTH + 1; 
		
	memcpy(sComHeader.m_szUsage, DEL_BUDDY, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromIDHeader.m_szFromID), strlen(m_szMyUserID))); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
	
	// ���۵����� ������	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szDelIDs, strlen(szDelIDs) /*strDelIDs.GetLength()*/ );	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(Send(pSzBuf, nTotalLen,"RequestDelBuddys") < 0)	
	{
		TRACEX("CChatSession::RequestDelBuddys -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}

BOOL CChatSession::RequestBuddyList(const CString & strID) 
{
	// GET_BUDDY_LIST	
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];

	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, GET_BUDDY_LIST, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromToIDHeader.m_szFromID, (LPCSTR)strID, min(sizeof(sFromToIDHeader.m_szFromID), strlen((LPCSTR)strID))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestBuddyList") < 0)	
	{
		TRACEX("CChatSession::RequestBuddyList -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	//Log
	
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "RequestBuddyList Send! [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

	
	return TRUE;
}



CStringArray* CChatSession::getGrpNames()
{	
	char szIP			[15+1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN];
	char szGroupName	[C_GRP_NAME_LEN];
	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;
	HTREEITEM hCurrent = m_pDlgMain->m_TreeCtrl.GetRootItem();
	CString ItemText;
		
	m_StrAryGrpName.RemoveAll();
	while (hCurrent != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;	
		n_StatusFlags = 0l;
		memset(szIP, 0x00, sizeof(szIP));
		memset(szSelItemID, 0x00, sizeof(szSelItemID));
		memset(szNickName, 0x00, sizeof(szNickName));
		memset(szRealName, 0x00, sizeof(szRealName));
		memset(szGroupName, 0x00, sizeof(szGroupName));

		BOOL bWorked = m_pDlgMain->m_TreeCtrl.GetItemUserInfo(hCurrent, szIP, szSelItemID, szNickName,szRealName,szGroupName,bConnStatus,&nBockMode,&n_StatusFlags);		
		
		m_StrAryGrpName.Add(szGroupName);
		
		hCurrent = m_pDlgMain->m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}	

	return &m_StrAryGrpName;
}

// ��ȭ��� GROUP �߰�
BOOL CChatSession::RequestAddGroup(LPCTSTR szCompanionGrp)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_ADD_BUDDY_GRP sAddBuddy;
	char szTemp[20];

	memset(&sAddBuddy, NULL, sizeof(sAddBuddy));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = sizeof(S_ADD_BUDDY_GRP); 
		
	memcpy(sComHeader.m_szUsage, ADD_BUDDY_GRP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sAddBuddy.m_szUserID, m_szMyUserID, min(sizeof(sAddBuddy.m_szUserID), strlen(m_szMyUserID))); 	
	memcpy(sAddBuddy.m_szBuddyGrp, szCompanionGrp, min(sizeof(sAddBuddy.m_szBuddyGrp), strlen(szCompanionGrp))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sAddBuddy, sizeof(S_ADD_BUDDY_GRP) );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestAddGroup") < 0)	
	{
		TRACEX("CChatSession::RequestAddGroup -> Send return FAIL!\n");		
		return FALSE ;
	}

//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "RequestAddGroup Send OK [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

	return TRUE;

}

// ��ȭ��� �߰�
BOOL CChatSession::RequestAddCompanion(LPCTSTR szCompanionID, LPCTSTR szCompanionGrp)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_ADD_BUDDY sAddBuddy;
	char szTemp[20];

	memset(&sAddBuddy, NULL, sizeof(sAddBuddy));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = sizeof(S_ADD_BUDDY); 
		
	memcpy(sComHeader.m_szUsage, ADD_BUDDY_LIST, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sAddBuddy.m_szUserID, m_szMyUserID, min(sizeof(sAddBuddy.m_szUserID), strlen(m_szMyUserID))); 
	memcpy(sAddBuddy.m_szBuddyID, szCompanionID, min(sizeof(sAddBuddy.m_szBuddyID), strlen(szCompanionID))); 
	memcpy(sAddBuddy.m_szBuddyGrp, szCompanionGrp, min(sizeof(sAddBuddy.m_szBuddyGrp), strlen(szCompanionGrp))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sAddBuddy, sizeof(S_ADD_BUDDY) );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestAddCompanion") < 0)	
	{
		TRACEX("CChatSession::RequestAddCompanion -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}


//! �ٽ� ��ȭ���� �߰��Ǿ����� �˸���
BOOL CChatSession::AckYouAllowedAsCompanion (const char* szCompanionID, const char* szCompanionGrp)
{	
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_ADD_BUDDY sAddBuddy;
	char szTemp[20];

	memset(&sAddBuddy, NULL, sizeof(sAddBuddy));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = sizeof(S_ADD_BUDDY); 
		
	memcpy(sComHeader.m_szUsage, YOUALLOWEDASCOMP, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sAddBuddy.m_szUserID, m_szMyUserID, min(sizeof(sAddBuddy.m_szUserID), strlen(m_szMyUserID))); 
	memcpy(sAddBuddy.m_szBuddyID, szCompanionID, min(sizeof(sAddBuddy.m_szBuddyID), strlen(szCompanionID))); 
	memcpy(sAddBuddy.m_szBuddyGrp, szCompanionGrp, min(sizeof(sAddBuddy.m_szBuddyGrp), strlen(szCompanionGrp))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sAddBuddy, sizeof(S_ADD_BUDDY) );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"AckYouAllowedAsCompanion") < 0)	
	{
		TRACEX("CChatSession::AckYouAllowedAsCompanion -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;

}


// Multi Chat ������ 
BOOL CChatSession::RequestExitMultiChat (const char* szServerRoomNo)
{
	TRACEX("CChatSession::RequestExitMultiChat szServerRoomNo [%s]\n", szServerRoomNo);		

	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];

	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, EXIT_MULTI_CHAT, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromToIDHeader.m_szToID, (LPCSTR)szServerRoomNo, min(sizeof(sFromToIDHeader.m_szToID), strlen( szServerRoomNo) ) ); 
	memcpy(sFromToIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromToIDHeader.m_szFromID), strlen(m_szMyUserID) ) ); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestExitMultiChat") < 0)	
	{
		TRACEX("CChatSession::RequestExitMultiChat -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}

// �α��� ���̵� �˻�
BOOL CChatSession::RequestLogInID( const char*  szID, const char*  szName)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_SEARCH_ID sSearchID;
	char szTemp[20];

	memset(&sSearchID, NULL, sizeof(sSearchID));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = sizeof(S_SEARCH_ID); 
		
	memcpy(sComHeader.m_szUsage, SEARCH_ID, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sSearchID.szIDFrom, m_szMyUserID, min(sizeof(sSearchID.szIDFrom), strlen(m_szMyUserID))); 
	memcpy(sSearchID.szID, szID, min(sizeof(sSearchID.szID), strlen(szID))); 	
	memcpy(sSearchID.szName, szName, min(sizeof(sSearchID.szName), strlen(szName))); 
	
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sSearchID, sizeof(S_SEARCH_ID) );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestLogInID") < 0)	
	{
		TRACEX("CChatSession::RequestLogInID -> Send return FAIL!\n");		
		return FALSE ;
	}
	
	return TRUE;
}


// ��ȭ�� ����
BOOL CChatSession::RequestChgNick( const char*  szNickBef, const char*  szChgedNick)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_CHG_NICK sChngNick;
	char szTemp[20];

	memset(&sChngNick, NULL, sizeof(sChngNick));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = sizeof(S_CHG_NICK); 
		
	memcpy(sComHeader.m_szUsage, CHNG_NICK, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sChngNick.szIDFrom, m_szMyUserID, min(sizeof(sChngNick.szIDFrom), strlen(m_szMyUserID))); 	
	memcpy(sChngNick.szNickBef, szNickBef, min(sizeof(sChngNick.szNickBef), strlen(szNickBef))); 
	memcpy(sChngNick.szNickAft, szChgedNick, min(sizeof(sChngNick.szNickAft), strlen(szChgedNick))); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChngNick, sizeof(S_CHG_NICK) );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"RequestChgNick") < 0)	
	{
		TRACEX("CChatSession::RequestChgNick -> Send return FAIL!\n");		
		return FALSE ;
	}

	return TRUE;
}

void CChatSession::ProcessWhenConnClosed()
{
	TRACE("CChatSession::ProcessWhenConnClosed() ���� ������ �����!!\n");
	mbConnected = FALSE;	
	
	//Log
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "CChatSession::ProcessWhenConnClosed() ���� ������ �����! mbConnected = FALSE [%s]\n" , iDT.Format("%Y%m%d%H%M%S"));

	::PostMessage(m_hOwnerWnd, WM_SERVER_CLOSED, (WPARAM)NULL, (LPARAM)NULL);
	
}

BOOL CChatSession::doWork_STATUS_AWAY (char* data)
{
	S_AWAY_INFO * psAwayInfo = new S_AWAY_INFO;
	
	memcpy(psAwayInfo, (S_AWAY_INFO *) (data + COMMON_HEADER_LENGTH), sizeof(S_AWAY_INFO));
	
	::PostMessage(m_hOwnerWnd, WM_AWAY_TRUE, (WPARAM)NULL, (LPARAM)psAwayInfo);

	return TRUE;
}

BOOL CChatSession::doWork_STATUS_ONLINE (char* data)
{
	S_AWAY_INFO * psAwayInfo = new S_AWAY_INFO;	
	memcpy(psAwayInfo, (S_AWAY_INFO *) (data + COMMON_HEADER_LENGTH), sizeof(S_AWAY_INFO));
	
	::PostMessage(m_hOwnerWnd, WM_AWAY_FALSE, (WPARAM)NULL, (LPARAM)psAwayInfo);

	return TRUE;
}


BOOL CChatSession::doWork_STATUS_BUSY (char* data)
{
	S_AWAY_INFO * psAwayInfo = new S_AWAY_INFO;	
	memcpy(psAwayInfo, (S_AWAY_INFO *) (data + COMMON_HEADER_LENGTH), sizeof(S_AWAY_INFO));
	
	::PostMessage(m_hOwnerWnd, WM_STATUS_BUSY, (WPARAM)NULL, (LPARAM)psAwayInfo);

	return TRUE;
}

BOOL CChatSession::doWork_ChatMsg (char* data)
{
	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	S_CHAT_HEADER *psChatHeader = (S_CHAT_HEADER *) (data + COMMON_HEADER_LENGTH);
		
	S_CHAT_CLIENT * pChatMsg = new S_CHAT_CLIENT;
	pChatMsg->strIDTo = psChatHeader->szIDTo;		
	pChatMsg->strIDFrom = psChatHeader->szIDFrom;
	pChatMsg->strMsg = data + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH;
	pChatMsg->strMsg = pChatMsg->strMsg.Mid(0, atoi(psHeader->m_szLength) -  S_CHAT_HEADER_LENGTH -1) ;

	
	::PostMessage(m_hOwnerWnd, WM_CHATMSG, (WPARAM)NULL, (LPARAM)pChatMsg);	
	
	/*
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	S_FROMTOID_HEADER *psFromToIDHeader = (S_FROMTOID_HEADER *) (data + COMMON_HEADER_LENGTH);
		
	S_CHAT_CLIENT * pChatMsg = new S_CHAT_CLIENT;
	pChatMsg->strIDTo = psFromToIDHeader->m_szToID;		
	pChatMsg->strIDFrom = psFromToIDHeader->m_szFromID;
	pChatMsg->strMsg = data + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH;
	pChatMsg->strMsg = pChatMsg->strMsg.Mid(0, atoi(psHeader->m_szLength) -  FROMTOID_HEADER_LENGTH -1) ;
	TRACEX("CHAT_MSG\n");
	TRACEX("szIDTo   [%s]\n", psFromToIDHeader->m_szToID);
	TRACEX("szIDFrom [%s]\n", psFromToIDHeader->m_szFromID);		
	
	::PostMessage(m_hOwnerWnd, WM_CHATMSG, (WPARAM)NULL, (LPARAM)pChatMsg);	
	*/

	//S_FROMTOID_HEADER
	
	return TRUE;
}

BOOL CChatSession::doWork_GetBuddyGrp (char* data)
{
	//LogReconn
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "doWork_GetBuddyGrp [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );
		
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return TRUE;
	}

	char *szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;

	int nTotalCnt = atoi(psHeader->m_szCount);		
				
	CPtrArray *pPtrAry = new CPtrArray;		
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����	
	int nIndex = 0;		
	S_BUDDY_GRP *psDataTemp = NULL;
	
	char szTmpOut[256];
	int nPos = strlen(szData);
	CString strData(szData);	

	while(1)
	{
		//AfxMessageBox(strData); //TEST
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		//AfxMessageBox(szTmpOut); //TEST
		
		if( nTotalCnt >0 &&  strcmp("NotFound",szTmpOut)!=0 )
		{					
			psDataTemp = new S_BUDDY_GRP;				
			psDataTemp->m_strBuddyGrp =  szTmpOut;
			
			pPtrAry->Add(psDataTemp);				
			nTotalCnt--;					
		}
		
		if( nTotalCnt == 0 ||  strcmp("NotFound",szTmpOut)==0 )		
		{					
			//AfxMessageBox("Break"); //TEST
			break;
		}
		
		nIndex++;		
	}	
	
	::PostMessage(m_hOwnerWnd, WM_BUDDY_GRP, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}

BOOL CChatSession::doWork_GetBuddyList (char* data)
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 )
	{			
		if( strcmp( NODATA_FOUND , psSqlRslt->szErrMsg)!=0 )
		{
			S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
			strcpy(psDataTemp->szRsltCd , "N");
			strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
			
			::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
			return FALSE;
		}
		else
		{
			return TRUE;
		}			
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);
				
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ���� (CBuddyInfo)
	
	int nIndex = 0;		
	CBuddyInfo *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	
	plugware::CAutoLock LockCriticlaSection( m_criSecLocalScopeBuddyInfo,"doWork_GetBuddyList"); 

	//Log
//	CTime iDT =  CTime::GetCurrentTime();
//	CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
//	writeLogFile("c:\\ClientLog.log", "doWork_GetBuddyList ���� [%s]\n" , strData);

	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		//if( nTotalCnt == 0 )
		if( strcmp(szTmpOut ,"NotFound") == 0 || nPos < 0 || nTotalCnt == 0)
		{
			break;
		}
		
		if( strcmp(szTmpOut ,"NotFound") != 0 )
		//if( nTotalCnt >0 )
		{
			// KIND, NAME, NICK, COMPID, COMPGRP, IP, CON_STATUS, AWAY_INFO
			
			switch(nIndex % 8)
			{
			case 0:
				/*			
				KIND ģ�� ���� 
				* 1 - ���� ����� ���
				* 2 - ������ ���� �߰��ؼ� ������� ���� ��ȭ��뿡 �߰���
				* 3 - ��ȭ �ź�
				* 4 - ��ȭ��밡 ���� ������
				* 9 - ����ģ�� NICK �� �̸�, COMPID �� �ּ�����
				*/
				psDataTemp = new CBuddyInfo;
				psDataTemp->m_nYouBlocked = atoi(szTmpOut);
				break;				
			case 1:	
				// NAME
				strncpy(psDataTemp->m_szUserName , szTmpOut, sizeof(psDataTemp->m_szUserName) );					
				break;			
			case 2:	
				// NICK
				strncpy(psDataTemp->m_szNickName , szTmpOut, sizeof(psDataTemp->m_szNickName) );					
				break;			
			case 3:	
				// COMPID
				if(psDataTemp->m_nYouBlocked == 9)
				{
					//����ģ���� ���� �ּ����� ������ ���� 300 �Ѿ�´�.
					TRACEX("����ģ�� ��� MAP!\n");
					mITCompDescToUrlMap.SetAt(psDataTemp->m_szNickName, szTmpOut);					
				}
				else
				{
					strncpy(psDataTemp->m_szBuddyID , szTmpOut, sizeof(psDataTemp->m_szBuddyID) );					
				}				
				break;				
			case 4:	
				// COMPGRP
				strncpy(psDataTemp->m_szBuddyGrp , szTmpOut, sizeof(psDataTemp->m_szBuddyGrp) );
				//AfxMessageBox(psDataTemp->m_szBuddyGrp); //TEST
				break;
			case 5:
				// IP
				strncpy(psDataTemp->m_szIpAddr , szTmpOut, sizeof(psDataTemp->m_szIpAddr) );					
				break;				
			case 6:
				// CON_STATUS
				if(szTmpOut[0] == 'Y')
					psDataTemp->m_bConnStatus = TRUE;
				else
					psDataTemp->m_bConnStatus = FALSE;
				
				break;	
			case 7:
				// AWAY_INFO
				psDataTemp->m_nAway = atoi(szTmpOut);				
				
				if(psDataTemp->m_nYouBlocked == 9)
				{						
					m_vecUserIDOrdered.push_back(psDataTemp->m_szNickName); 
					mBuddyInfoMap.SetAt(psDataTemp->m_szNickName , psDataTemp); // nick name �� �ּҼ����̴�...					
				}
				else
				{					
					m_vecUserIDOrdered.push_back(psDataTemp->m_szBuddyID); 
					mBuddyInfoMap.SetAt(psDataTemp->m_szBuddyID , psDataTemp);
				}
				
				nTotalCnt--;

				break;
			}
		}
		
		nIndex++;
	}		
	
	::PostMessage(m_hOwnerWnd, WM_BUDDY_LIST, (WPARAM)NULL, (LPARAM)NULL);
	
	return TRUE;
}


BOOL CChatSession::doWork_SearchID (char* data)
{		
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 )
	{			
		if( strcmp( NODATA_FOUND , psSqlRslt->szErrMsg)!=0 )
		{
			S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
			strcpy(psDataTemp->szRsltCd , "N");
			strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
			
			::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
			return FALSE;
		}
		else
		{
			CPtrArray *pPtrAry = new CPtrArray;	
			
			::PostMessage(m_hOwnerWnd, WM_SEARCH_ID, (WPARAM)NULL, (LPARAM)pPtrAry);
			return TRUE;
		}			
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);
	CPtrArray *pPtrAry = new CPtrArray;	
	int nIndex = 0;	
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	S_SEARCHID_CLI* psTemp = NULL;
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		if( nTotalCnt == 0 )
		{
			break;
		}

		//if( strcmp(szTmpOut ,"NotFound") != 0 )
		if( nTotalCnt > 0 )
		{
			switch(nIndex % 12)
			{
			case 0:				
				psTemp = new S_SEARCHID_CLI;
				strncpy(psTemp->szUserName , szTmpOut, sizeof(psTemp->szUserName) );					
				break;			
			case 1:	
				strncpy(psTemp->szUserId , szTmpOut, sizeof(psTemp->szUserId) );					
				break;			
			case 2:	
				strncpy(psTemp->szCompany , szTmpOut, sizeof(psTemp->szCompany) );					
				break;			
			case 3:	
				strncpy(psTemp->szTopTeamName , szTmpOut, sizeof(psTemp->szTopTeamName) );					
				break;			
			case 4:	
				strncpy(psTemp->szTeamName , szTmpOut, sizeof(psTemp->szTeamName) );					
				break;
			case 5:	
				strncpy(psTemp->szTel1 , szTmpOut, sizeof(psTemp->szTel1) );					
				break;			
			case 6:	
				strncpy(psTemp->szTel2 , szTmpOut, sizeof(psTemp->szTel2) );					
				break;
			case 7:	
				strncpy(psTemp->szJobClassName , szTmpOut, sizeof(psTemp->szJobClassName) );					
				break;
			case 8:	
				strncpy(psTemp->szCompanyuCode , szTmpOut, sizeof(psTemp->szCompanyuCode) );					
				break;
			case 9:	
				strncpy(psTemp->szTopTeamCode , szTmpOut, sizeof(psTemp->szTopTeamCode) );					
				break;			
			case 10:	
				strncpy(psTemp->szTeamCode , szTmpOut, sizeof(psTemp->szTeamCode) );					
				break;
			case 11:	
				strncpy(psTemp->szJobClassCode , szTmpOut, sizeof(psTemp->szJobClassCode) );					
				pPtrAry->Add(psTemp);				
				nTotalCnt--;
				break;				
			}
		}
		
		nIndex++;
	}
	
	::PostMessage(m_hOwnerWnd, WM_SEARCH_ID, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}


BOOL CChatSession::doWork_RAWSQL_JOB_GN (char* data)
{	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}		
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;		
	
	int nIndex = 0;		
	S_JOBGN *psDataTemp = NULL;		
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
				
		//if( strcmp(szTmpOut ,"NotFound") != 0 )
		if( nTotalCnt >0 )
		{
			switch(nIndex % 2)
			{
			case 0:				
				psDataTemp = new S_JOBGN;				
				psDataTemp->strUserID =  szTmpOut;
				break;				
			case 1:
				psDataTemp->strJobGn =  szTmpOut;
				pPtrAry->Add(psDataTemp);				
				nTotalCnt--;
				break;
			}
		}
		
		if( nTotalCnt ==0 )
		{				
			break;
		}
		
		nIndex++;
	}
	
	
	::PostMessage(m_hOwnerWnd, WM_JOBGN, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_DAILYJOB_MST (char* data)
{	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	// S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)
	{  
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
				
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}	
	
	if( memcmp(NODATA_FOUND, psSqlRslt->szErrMsg, sizeof(NODATA_FOUND)) == 0)
	{
		//ToDo!
		CPtrArray *pPtrAry = new CPtrArray;	
		S_DAILYJOB_MST *psDataTemp = new S_DAILYJOB_MST;
		pPtrAry->Add(psDataTemp);			
		
		::PostMessage(m_hOwnerWnd, WM_DAILYJOB_MST, (WPARAM)NULL, (LPARAM)pPtrAry);
		
		return TRUE;
		
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;		
	
	int nIndex = 0;		
	S_DAILYJOB_MST *psDataTemp = NULL;		
	char szTmpOut[256];
	int nPos = strlen(szData);
	CString strData(szData);
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		
		//if( strcmp(szTmpOut ,"NotFound") != 0 )
		if( nTotalCnt >0 )
		{
			switch(nIndex % 3)
			{
			case 0:				
				psDataTemp = new S_DAILYJOB_MST;				
				psDataTemp->strJobDay =  szTmpOut;
				break;				
			case 1:	
				psDataTemp->strLogOnTime =  szTmpOut;
				break;			
			case 2:
				psDataTemp->strLogOffTime =  szTmpOut;
				pPtrAry->Add(psDataTemp);				
				nTotalCnt--;
				break;
			}
		}
		
		if( nTotalCnt == 0 )
		{				
			break;
		}
		
		nIndex++;
	}


	::PostMessage(m_hOwnerWnd, WM_DAILYJOB_MST, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}


BOOL CChatSession::doWork_ALL_USER_ID_TEAM(char* szRawData)
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)szRawData;	
	
	char* szData = szRawData + COMMON_HEADER_LENGTH;		
	
	int nTotalLen = atoi(psHeader->m_szLength);
	
	char* pDataNew = new char[nTotalLen];
	strncpy(pDataNew, szData, nTotalLen);

	if(strcmp(ALL_USER_ID_TEAM_FORADD, psHeader->m_szUsage) ==0 )
		::PostMessage(m_hOwnerWnd, WM_ALL_USER_ID_TEAM_FORADD, (WPARAM)NULL, (LPARAM)pDataNew);
	else
		::PostMessage(m_hOwnerWnd, WM_ALL_USER_ID_TEAM, (WPARAM)NULL, (LPARAM)pDataNew);
	
	return TRUE;
}

BOOL CChatSession::doWork_FOOD_MENU(char* szRawData)
{	
	
	//FOOD_MENU
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)szRawData;	
	
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (szRawData + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 )
	{			
		if( strcmp( NODATA_FOUND , psSqlRslt->szErrMsg)!=0 )
		{
			S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
			strcpy(psDataTemp->szRsltCd , "N");
			strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
			
			::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
			return FALSE;
		}
		else
		{
			CPtrArray *pPtrAry = new CPtrArray;	
			
			::PostMessage(m_hOwnerWnd, WM_SEARCH_ID, (WPARAM)NULL, (LPARAM)pPtrAry);
			return TRUE;
		}			
	}
	
	char* szData = szRawData + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);
	CPtrArray *pPtrAry = new CPtrArray;	
	int nIndex = 0;	
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	S_FOOD_MENU_CLI* psTemp = NULL;
	
	//while( 1)
	for(int nT =0; nT < 5*4*2; nT++) // 5�ϰ����� ���� => 5�� * 4 �÷� * ��,���� 2
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		//if( nTotalCnt == 0 )
		//{
		//	break;
		//}

		
		//if( nTotalCnt > 0 )
		{
			switch(nIndex % 4)
			{
			case 0:				
				psTemp = new S_FOOD_MENU_CLI;
				memset(psTemp, 0x00, sizeof(S_FOOD_MENU_CLI));
				strncpy(psTemp->m_szDate , szTmpOut, sizeof(psTemp->m_szDate) );					
				break;			
			case 1:	
				strncpy(psTemp->m_szLD_FLAG , szTmpOut, sizeof(psTemp->m_szLD_FLAG) );					
				break;			
			case 2:	
				strncpy(psTemp->m_szMenu , szTmpOut, sizeof(psTemp->m_szMenu) );					
				break;						
			case 3:	
				strncpy(psTemp->m_szCal , szTmpOut, sizeof(psTemp->m_szCal) );					
				pPtrAry->Add(psTemp);				
				nTotalCnt--;
				break;				
			}
		}
		
		nIndex++;
	}
	
	::PostMessage(m_hOwnerWnd, WM_FOOD_MENU, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}

BOOL CChatSession::doWork_LOGIN (char* data)
{	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	// �̰�쿡�� S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
				
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{
		CPtrArray *pPtrAry = new CPtrArray;		
		S_LOGIN *psDataTemp = new S_LOGIN;
		psDataTemp->bIsSuccess = FALSE;
		psDataTemp->strErrorMSg = psSqlRslt->szErrMsg ;
		pPtrAry->Add(psDataTemp);
		
		::PostMessage(m_hOwnerWnd, WM_LOGIN, (WPARAM)NULL, (LPARAM)pPtrAry);
		return TRUE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;		
	
	int nIndex = 0;		
	S_LOGIN *psDataTemp = NULL;		
	char szTmpOut[256];
	CString strData(szData);
	int nPos = strlen(szData);
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
				
		//if( strcmp(szTmpOut ,"NotFound") != 0 )
		if( nTotalCnt > 0 )
		{
			switch(nIndex % 12)
			{
			case 0:				
				psDataTemp = new S_LOGIN;	
				psDataTemp->bIsSuccess = TRUE;
				psDataTemp->strKT_GN =  szTmpOut;
				break;
			case 1:
				psDataTemp->strUSER_NAME =  szTmpOut;				
				break;
			case 2:
				psDataTemp->strPASSWD =  szTmpOut;				
				break;
			case 3:
				psDataTemp->strTEAM_GN =  szTmpOut;				
				break;
			case 4:
				psDataTemp->strTEAM_NAME =  szTmpOut;				
				break;
			case 5:
				psDataTemp->strUSER_GRADE =  szTmpOut;				
				break;
			case 6:
				psDataTemp->strTOP_TEAM_GN =  szTmpOut;				
				break;
			case 7:
				psDataTemp->strTOP_TEAM_NAME =  szTmpOut;					
				break;
			case 8:
				psDataTemp->strSTATUS =  szTmpOut;
				break;
			case 9:
				psDataTemp->strNick =  szTmpOut;
				break;
			case 10:
				psDataTemp->strIAMS_AUTHORITY =  szTmpOut;
				break;
			case 11:
				psDataTemp->strPROG_SELECT =  szTmpOut;
				pPtrAry->Add(psDataTemp);				
				nTotalCnt--;
				break;
			}
		}
		
		if( nTotalCnt == 0 )
		{				
			break;
		}
		
		nIndex++;			
	}
	
	::PostMessage(m_hOwnerWnd, WM_LOGIN, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_GETUSER_INFO (char* data)
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	// �̰�쿡�� S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}	
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	
	if(nTotalCnt > 0)
	{
		CPtrArray *pPtrAry = new CPtrArray;		
		
		// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
		//string strCompListRaw(szData);		
		//string strTmpOut;
		char szTmpOut[256];
		int nIndex = 0;		
		SUSER_INFO *psDataTemp = NULL;
		int nPos = strlen(szData);
		CString strData(szData);
		
		while( 1)
		{
			memset(szTmpOut, 0x00, sizeof(szTmpOut));
			nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
			
			//if( strcmp(szTmpOut ,"NotFound") != 0 )
			if( nTotalCnt > 0 )
			{
				switch(nIndex % 11)
				{
				case 0:				
					psDataTemp = new SUSER_INFO;				
					psDataTemp->m_strJUMIN_NO =  szTmpOut;
					break;
				case 1:
					psDataTemp->m_strUSER_NAME =  szTmpOut;				
					break;
				case 2:
					psDataTemp->m_strUSER_ID =  szTmpOut;				
					break;
				case 3:
					psDataTemp->m_strPASSWD =  szTmpOut;				
					break;
				case 4:
					psDataTemp->m_strTEAM_GN =  szTmpOut;				
					break;
				case 5:
					psDataTemp->m_strJOB_CLASS =  szTmpOut;				
					break;
				case 6:
					psDataTemp->m_strJOB_WRITE_GN =  szTmpOut;				
					break;
				case 7:
					psDataTemp->m_strOFF_TEL_NO =  szTmpOut;					
					break;
				case 8:
					psDataTemp->m_strOFF_IN_NO =  szTmpOut;					
					break;
				case 9:
					psDataTemp->m_strHP_TEL_NO =  szTmpOut;					
					break;
				case 10:
					psDataTemp->m_strCOMPANY_POS =  szTmpOut;						
					pPtrAry->Add(psDataTemp);				
					nTotalCnt--;
					break;
				}
			}
			
			if( nTotalCnt == 0 )
			{					
				break;
			}
			
			nIndex++;			
		}
			
		::PostMessage(m_hOwnerWnd, WM_REGUSER_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
	}	
	
	return TRUE;
}

BOOL CChatSession::doWork_GetTeamInfo( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	// �̰�쿡�� S_COMMON_HEADER ���� ������ : S_SQL_RSLTDATA + ������ !
	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	
	char szWndTitleTmp[20];
	memset(szWndTitleTmp, 0x00, sizeof(szWndTitleTmp));
	strcpy(szWndTitleTmp, "IDMS ����� ���");
	BOOL bSent = FALSE; 
	
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{
		CPtrArray *pPtrAry = new CPtrArray;		
		SCOMBO_INFO *psDataTemp = new SCOMBO_INFO;
		psDataTemp->bIsSuccess = FALSE;
		psDataTemp->strErrorMSg = psSqlRslt->szErrMsg ;
		pPtrAry->Add(psDataTemp);
		
		if(	strcmp(psHeader->m_szUsage, RAWSQL_TEAM ) == 0	)
		{			
			CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
			if(pWnd != NULL)
			{			
				::PostMessage(pWnd->m_hWnd, WM_TEAMCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
				//bSent = TRUE; 
			}			
		}
		else if(	strcmp(psHeader->m_szUsage, RAWSQL_GETCOMPANY_INFO ) == 0 )
		{
			CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
			if(pWnd != NULL)
			{				
				::PostMessage(m_hOwnerWnd, WM_COMPANYCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
				//bSent = TRUE; 
			}			
		}
		else if(	strcmp(psHeader->m_szUsage, RAWSQL_GETCLASS_INFO ) == 0	)	
		{
			CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
			if(pWnd != NULL)
			{
				::PostMessage(m_hOwnerWnd, WM_CLASSCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
				//bSent = TRUE; 
			}
		}
		
		/*
		if(bSent == FALSE)
		{
			for(int j = 0; j < pPtrAry->GetSize(); j++)
			{
				SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pPtrAry->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
				}
			}
			delete pPtrAry;
		}
		*/
		
		return TRUE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;			
	int nIndex = 0;		
	SCOMBO_INFO *psDataTemp = NULL;
	
	char szTmpOut[256];
	int nPos = strlen(szData);
	CString strData(szData);
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		//if(strTmpOut.compare("NotFound") != 0 && strTmpOut.length() > 0)
		//if(nPos != -1 && strlen(szTmpOut) > 0 )
		if( nTotalCnt > 0 )
		{
			switch(nIndex % 2)
			{
			case 0:						
				psDataTemp = new SCOMBO_INFO;
				psDataTemp->m_strCodeName  =  szTmpOut;					
				break;				
			case 1:
				psDataTemp->m_strCode =  szTmpOut;	
				pPtrAry->Add(psDataTemp);				
				nTotalCnt--;
				break;
			}
		}
		
		if( nTotalCnt == 0 )
		{				
			break;
		}
		
		nIndex++;
	}
				
	memset(szWndTitleTmp, 0x00, sizeof(szWndTitleTmp));
	strcpy(szWndTitleTmp, "IDMS ����� ���");
	bSent = FALSE; 
	if(	strcmp(psHeader->m_szUsage, RAWSQL_TEAM ) == 0	)
	{			
		CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
		if(pWnd != NULL)
		{
			::PostMessage(pWnd->m_hWnd, WM_TEAMCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
			//bSent = TRUE; 
		}			
	}
	else if(	strcmp(psHeader->m_szUsage, RAWSQL_GETCOMPANY_INFO ) == 0 )
	{
		CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
		if(pWnd != NULL)
		{
			::PostMessage(m_hOwnerWnd, WM_COMPANYCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
			//bSent = TRUE; 
		}			
	}
	else if(	strcmp(psHeader->m_szUsage, RAWSQL_GETCLASS_INFO ) == 0	)	
	{
		CWnd* pWnd = CWnd::FindWindow(NULL, szWndTitleTmp);
		if(pWnd != NULL)
		{
			::PostMessage(m_hOwnerWnd, WM_CLASSCOMBO_INFO, (WPARAM)NULL, (LPARAM)pPtrAry);	
			//bSent = TRUE; 
		}
	}
	
	/*
	if(bSent == FALSE)
	{
		for(int j = 0; j < pPtrAry->GetSize(); j++)
		{
			SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pPtrAry->GetAt(j));
			if(pData != NULL)
			{
				delete pData;
			}
		}
		delete pPtrAry;
	}
	*/
	
	return TRUE;
}


BOOL CChatSession::doWork_BuddyInfoAddMe( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		if( strcmp(NODATA_FOUND, psSqlRslt->szErrMsg)==0)
		{
			strcpy(psDataTemp->szErrMsg , "��ȭ��븦 ã���� �����ϴ�.");			
		}
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	
	int nTotalCnt = atoi(psHeader->m_szCount);		
	
	if(nTotalCnt > 0)
	{
		CPtrArray *pPtrAry = new CPtrArray;		
		
		// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����			
		int nIndex = 0;		
		S_ADD_BUDDY_RCV *psDataTemp = NULL;
		int nPos = strlen(szData);
		CString strData(szData);
		char szTmpOut[256];
		
		while( 1)
		{
			memset(szTmpOut, 0x00, sizeof(szTmpOut));
			nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
						
			
			//if( strcmp(szTmpOut ,"NotFound") != 0 )
			if( nTotalCnt > 0 )
			{
				//A.USER_ID, A.USER_NAME, A.NICK_NAME, A.IP_ADDR,B.COMP_GROUP, kind
				switch(nIndex % 7)
				{
				case 0:				
					psDataTemp = new S_ADD_BUDDY_RCV;				
					psDataTemp->m_strBuddyID =  szTmpOut;
					break;
				case 1:
					psDataTemp->m_strBuddyName =  szTmpOut;				
					break;
				case 2:
					psDataTemp->m_strBuddyNick =  szTmpOut;				
					break;					
				case 3:
					psDataTemp->m_strBuddyIP =  szTmpOut;						
					break;					
				case 4:
					psDataTemp->m_strBuddyGrp =  szTmpOut;						
					break;
				case 5:
					psDataTemp->m_nBlock = atoi(szTmpOut);
					break;
				case 6:
					if( strncmp(szTmpOut, "Y", 1) == 0)
						psDataTemp->m_bConnStatus = TRUE;
					else
						psDataTemp->m_bConnStatus = FALSE;
					pPtrAry->Add(psDataTemp);				
					nTotalCnt--;
					break;
				}
			}
			
			if( nTotalCnt == 0 )
			{					
				break;
			}
			
			nIndex++;
		}
		
		// ���� �б� !
		if( strcmp(psHeader->m_szUsage, RAWSQL_GETBUDDY_INFO ) == 0 )			
		{		
			::PostMessage(m_hOwnerWnd, WM_ADD_BUDDY, (WPARAM)NULL, (LPARAM)pPtrAry);	
		}
		else if( strcmp(psHeader->m_szUsage, SOMEONE_ADD_ME ) == 0 )
		{		
			// SOMEONE_ADD_ME
			::PostMessage(m_hOwnerWnd, WM_SOMEONE_ADD_ME, (WPARAM)NULL, (LPARAM)pPtrAry);	
		}
	}
	
	return TRUE;
}


BOOL CChatSession::doWork_DEL_BUDDY_GRP( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);	
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_BUDDY_GRP *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		
		if( strcmp(szTmpOut ,"NotFound") != 0 )
		//if( nTotalCnt > 0 )
		{					
			psDataTemp = new S_BUDDY_GRP;				
			psDataTemp->m_strBuddyGrp =  szTmpOut;						
			pPtrAry->Add(psDataTemp);
			nTotalCnt--;
		}
		
		if( nPos < 0 )
		{			
			break;
		}
		
		nIndex++;			
		
	}
	
	::PostMessage(m_hOwnerWnd, WM_DEL_BUDDY_GRP, (WPARAM)NULL, (LPARAM)pPtrAry);	
	
	return TRUE;
}

BOOL CChatSession::doWork_DEL_BUDDYS( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	int nTotalCnt = atoi(psHeader->m_szCount);		
	CPtrArray *pPtrAry = new CPtrArray;
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_BUDDY_DEL *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		
		if( strcmp(szTmpOut ,"NotFound") != 0 )
		//if( nTotalCnt > 0 )
		{					
			psDataTemp = new S_BUDDY_DEL;				
			strncpy(psDataTemp->m_szBuddyID ,  szTmpOut, sizeof(psDataTemp->m_szBuddyID) );
			pPtrAry->Add(psDataTemp);
		}
		
		if( nPos < 0 )
		{			
			break;
		}
		
		nIndex++;			
	}
	
	::PostMessage(m_hOwnerWnd, WM_DEL_BUDDY_ID, (WPARAM)NULL, (LPARAM)pPtrAry);	
	
	return TRUE;
	
}


BOOL CChatSession::doWork_ALIVE( char* data) 
{
	//S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)szRawData;	
	//int nTotalLen = atoi(psHeader->m_szLength);	
		
	::PostMessage(m_hOwnerWnd, WM_ALIVE, (WPARAM)NULL, (LPARAM)NULL);
	
	return TRUE;
}

BOOL CChatSession::doWork_SOMEONE_DEL_ME( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ���� => �ʿ����� ����!!
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;		
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_SOMEONE_DEL_ME *psDataTemp = NULL;
	psDataTemp = new S_SOMEONE_DEL_ME;				
	strncpy( psDataTemp->m_szWhoDelMeID, szData,  atoi(psHeader->m_szLength) - SQL_RSLTDATA_LENGTH ); 
	
	::PostMessage(m_hOwnerWnd, WM_SOMEONE_DEL_ME, (WPARAM)NULL, (LPARAM)psDataTemp);
	
	return TRUE;
}


BOOL CChatSession::doWork_CHG_BUDDY_GRP( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	
	CPtrArray *pPtrAry = new CPtrArray;
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_BUDDY_CHG *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpGrp[50+1];
	char szTmpID[20+1];
	
	// ���� �׷����� ��´�
	memset(szTmpGrp, 0x00, sizeof(szTmpGrp));
	nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpGrp, DELIM1);
	
	while( 1)
	{
		memset(szTmpID, 0x00, sizeof(szTmpID));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpID, DELIM1);
		
		if( strcmp(szTmpID ,"NotFound") != 0 )
		{	
			psDataTemp = new S_BUDDY_CHG;				
			strncpy(psDataTemp->m_szBuddyGrp, szTmpGrp, sizeof(psDataTemp->m_szBuddyGrp));				
			strncpy(psDataTemp->m_szBuddyID, szTmpID, sizeof(psDataTemp->m_szBuddyID));
			
			pPtrAry->Add(psDataTemp);
		}
		
		if(nPos < 0)
		{			
			break;
		}
		
		nIndex++;			
		
	}
	
	::PostMessage(m_hOwnerWnd, WM_CHG_BUDDY_GRP, (WPARAM)NULL, (LPARAM)pPtrAry);
	
	return TRUE;

}


BOOL CChatSession::doWork_ADD_BUDDY_GRP( char* data) 
{
	//Log 20070528
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "[%s] CChatSession::doWork_ADD_BUDDY_GRP \n" , iDT.Format("%Y%m%d%H%M%S")  ); 

	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);			
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
				
	int nTotalCnt = atoi(psHeader->m_szCount);		
	
	if(nTotalCnt > 0)
	{			
		S_ADD_BUDDY_CLISVR* pSGrpInfo = new S_ADD_BUDDY_CLISVR;
		strncpy(pSGrpInfo->m_szBuddyGrp, data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, sizeof(S_ADD_BUDDY_CLISVR));
				
		::PostMessage(m_hOwnerWnd, WM_ADD_BUDDY_GRP, (WPARAM)NULL, (LPARAM)pSGrpInfo); 
	}		

	return TRUE;

}


BOOL CChatSession::doWork_CHG_GRP_NAME( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	
	CPtrArray *pPtrAry = new CPtrArray;
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_GRP_CHG_NAME *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpBefGrp[50+1];		
	char szTmpAftGrp[50+1];		
	
	memset(szTmpBefGrp, 0x00, sizeof(szTmpBefGrp));
	nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpBefGrp, DELIM1);
	
	while( 1)
	{
		memset(szTmpAftGrp, 0x00, sizeof(szTmpAftGrp));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpAftGrp, DELIM1);
		
		if( nPos != -1 )
		{	
			psDataTemp = new S_GRP_CHG_NAME;				
			strncpy(psDataTemp->m_szBefGrp, szTmpBefGrp, sizeof(psDataTemp->m_szBefGrp));				
			strncpy(psDataTemp->m_szAftGrp, szTmpAftGrp, sizeof(psDataTemp->m_szAftGrp));
			
			pPtrAry->Add(psDataTemp);
		}
		
		if(nPos < 0)
		{			
			break;
		}
		
		nIndex++;			
		
	}
	
	::PostMessage(m_hOwnerWnd, WM_CHG_GRP_NAME, (WPARAM)NULL, (LPARAM)pPtrAry);	
	
	return TRUE;
}

BOOL CChatSession::doWork_ACK_NICK_CHG( char* data) 
{
	S_CHG_NICK * psChngNick = new S_CHG_NICK;	
	memcpy(psChngNick, (S_CHG_NICK *) (data + COMMON_HEADER_LENGTH), sizeof(S_CHG_NICK));
		
	::PostMessage(m_hOwnerWnd, WM_CHNG_NICK, (WPARAM)NULL, (LPARAM)psChngNick);

	return TRUE;
}



BOOL CChatSession::doWork_MULTICHAT_SERVER_ROOMNO ( char* data) 
{
	S_SERVER_ROOMNO *psMCSrvRoomNo = new S_SERVER_ROOMNO ;
	memcpy( psMCSrvRoomNo , data + COMMON_HEADER_LENGTH , sizeof( S_SERVER_ROOMNO )  ); 
	
	::PostMessage(m_hOwnerWnd, WM_MCHAT_SERVER_ROOMNO, (WPARAM)NULL, (LPARAM)psMCSrvRoomNo);

	return TRUE;
}


BOOL CChatSession::doWork_SLIP_MSG( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 

	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}

	::PostMessage(m_hOwnerWnd, WM_SLIP_MSG_SUCCESS, (WPARAM)NULL, (LPARAM)NULL); 

	return TRUE;
}


BOOL CChatSession::doWork_DEL_IT_COMP(char* szRawData)
{
	/*
	S_COMMON_HEADER * psHeader = (S_COMMON_HEADER *)szRawData;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (szRawData + COMMON_HEADER_LENGTH ); 

	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)	
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* pSzDelITName = new char[100+1];
	char szDelITName[100+1];
	memset( szDelITName, 0x00, sizeof(szDelITName));
	
	strncpy(szDelITName, szRawData + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, atoi(psHeader->m_szLength) - SQL_RSLTDATA_LENGTH  );
	memcpy(pSzDelITName, &szDelITName, sizeof(szDelITName));
	::PostMessage(m_hOwnerWnd, WM_DEL_IT_COMP, (WPARAM)NULL, (LPARAM)pSzDelITName);

	return TRUE;
	*/

	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)szRawData;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (szRawData + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = szRawData + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
	
	CPtrArray *pPtrAry = new CPtrArray;
	
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;		
	S_IT_COMP_DEL *psDataTemp = NULL;
	int nPos = strlen(szData);
	CString strData(szData);
	char szTmpOut[256];
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpOut, DELIM1);
		
		if( strcmp(szTmpOut ,"NotFound") != 0 )
		{					
			psDataTemp = new S_IT_COMP_DEL;				
			strncpy(psDataTemp->m_szName ,  szTmpOut, sizeof(psDataTemp->m_szName) );
			pPtrAry->Add(psDataTemp);
		}
		
		if(nPos < 0)
		{			
			break;
		}
		
		nIndex++;			
	}
	
	::PostMessage(m_hOwnerWnd, WM_DEL_IT_COMP, (WPARAM)NULL, (LPARAM)pPtrAry);	
	
	return TRUE;

}

//����ģ�� �߰� ���
BOOL CChatSession::doWork_ADD_IT_COMP(char* szRawData)
{
	S_COMMON_HEADER * psHeader = (S_COMMON_HEADER *)szRawData;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (szRawData + COMMON_HEADER_LENGTH ); 

	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)	
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	S_ADD_IT_COMP * pAddItComp = new S_ADD_IT_COMP;
	
	////////////
	char szDescDeliUrlDeli[sizeof(S_ADD_IT_COMP)+2];
	
	memset(pAddItComp, 0x00, sizeof(S_ADD_IT_COMP));

	memset(szDescDeliUrlDeli, 0x00, sizeof(szDescDeliUrlDeli));
	
	strncpy(szDescDeliUrlDeli, szRawData + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, atoi(psHeader->m_szLength) - SQL_RSLTDATA_LENGTH  );
	
	GetStrByDelim(szDescDeliUrlDeli, pAddItComp->m_szDesc, DELIM1);
	GetStrByDelim(szDescDeliUrlDeli, pAddItComp->m_szUrl, DELIM1);	
	
	mITCompDescToUrlMap.SetAt(pAddItComp->m_szDesc, pAddItComp->m_szUrl);		

	::PostMessage(m_hOwnerWnd, WM_ADD_IT_COMP, (WPARAM)NULL, (LPARAM)pAddItComp);

	return TRUE;
}

BOOL CChatSession::doWork_CHG_IT_COMP( char* szRawData) 
{
	S_COMMON_HEADER * psHeader = (S_COMMON_HEADER *)szRawData;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (szRawData + COMMON_HEADER_LENGTH ); 

	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)	
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	S_CHG_IT_COMP * pChgItComp = new S_CHG_IT_COMP;
	
	////////////
	char szDescDeliUrlDeli[sizeof(S_CHG_IT_COMP)+3];
	
	memset(pChgItComp, 0x00, sizeof(S_CHG_IT_COMP));

	memset(szDescDeliUrlDeli, 0x00, sizeof(szDescDeliUrlDeli));
	
	strncpy(szDescDeliUrlDeli, szRawData + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH, atoi(psHeader->m_szLength) - SQL_RSLTDATA_LENGTH  );
	
	GetStrByDelim(szDescDeliUrlDeli, pChgItComp->m_szDescOld, DELIM1);
	GetStrByDelim(szDescDeliUrlDeli, pChgItComp->m_szDesc, DELIM1);
	GetStrByDelim(szDescDeliUrlDeli, pChgItComp->m_szUrl, DELIM1);	
	
	mITCompDescToUrlMap.RemoveKey(pChgItComp->m_szDescOld); 

	mITCompDescToUrlMap.SetAt(pChgItComp->m_szDesc, pChgItComp->m_szUrl); // reset �ȴ�.		

	::PostMessage(m_hOwnerWnd, WM_CHG_IT_COMP, (WPARAM)NULL, (LPARAM)pChgItComp);

	return TRUE;
}


BOOL CChatSession::doWork_GET_SLIP_MSG( char* data) 
{
	// FROM_ID NAME MSG REG_DATE... 
	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 

	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0 && strcmp(NODATA_FOUND, psSqlRslt->szErrMsg) != 0)	
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); //ERROR ���� ����
		return FALSE;
	}
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH ;
		
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;	
	int nPos = strlen(szData);
	CString strData(szData);	
	
	char szFromUserID[20+1];
	char szFromUserName[50+1];	
	char szMSg[1024+1];	
	char szDate[14+1];
		
	S_SLIP_MSG* psTemp = NULL;
	CPtrArray *pPtrAry = new CPtrArray;	

	while( 1)
	{
		memset(szFromUserID, 0x00, sizeof(szFromUserID));
		memset(szFromUserName, 0x00, sizeof(szFromUserName));
		memset(szMSg, 0x00, sizeof(szMSg));		
		memset(szDate, 0x00, sizeof(szDate));		

		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szFromUserID, DELIM1); //ID
		
		if(nPos < 0)
		{			
			break;
		}

		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szFromUserName, DELIM1); //NAME
		
		if(nPos < 0)
		{			
			break;
		}

		
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szMSg, DELIM1); //MSG
			
		if(nPos < 0)
		{			
			break;
		}

		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szDate, DELIM1); //REG_DATE
			
		if(nPos < 0)
		{			
			break;
		}
		
		psTemp = new S_SLIP_MSG ;

		strncpy( psTemp->m_szFromID,szFromUserID, sizeof(psTemp->m_szFromID) );
		strncpy( psTemp->m_szFromName,szFromUserName, sizeof(psTemp->m_szFromName) );
		strncpy( psTemp->m_szMsg,szMSg, sizeof(psTemp->m_szMsg) );
		strncpy( psTemp->m_szRegDate,szDate, sizeof(psTemp->m_szRegDate) );

		pPtrAry->Add(psTemp);				

		nIndex++;
	}
	
	::PostMessage(m_hOwnerWnd, WM_SLIP_MSG, (WPARAM)NULL, (LPARAM)pPtrAry);	

	return TRUE;
}


BOOL CChatSession::doWork_EXIT_MULTI_CHAT( char* data) 
{
	S_FROMTOID_HEADER* psFromToIdHeader = (S_FROMTOID_HEADER *) (data + COMMON_HEADER_LENGTH);
	
	//psFromToIdHeader->m_szToID;   // RoomNo
	//psFromToIdHeader->m_szFromID; // ���� ���� ID 

	MAP_STR2SET::iterator it ;
	SET_STR::iterator itSet ; 

	// �������� ���� RoomNo �� ã�Ƽ� set ����
	it =  m_mapStr2SetMultiChat.find(psFromToIdHeader->m_szToID);
	
	if(it != m_mapStr2SetMultiChat.end() )  
	{
		itSet = (*it).second.find(psFromToIdHeader->m_szFromID);
		
		if(itSet != (*it).second.end() )  
		{
			(*it).second.erase(itSet);
		}		
		
		// Multi Chat Dialog �� �˸�.
		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		//::SendMessage(pMainWnd->GetSafeHwnd(), WM_EXIT_MULTI_CHAT, (WPARAM)psFromToIdHeader->m_szToID, (LPARAM)psFromToIdHeader->m_szFromID);	
		char* pData   = new char [20 + 1];
		char* pDataID = new char [20 + 1];
		memset(pData, 0x00, 21 );
		memset(pDataID, 0x00, 21 );
		strncpy(pData, psFromToIdHeader->m_szToID, 21 );
		strncpy(pDataID, psFromToIdHeader->m_szFromID, 21 );		
		

		::PostMessage(pMainWnd->GetSafeHwnd(), WM_EXIT_MULTI_CHAT, (WPARAM)pDataID, (LPARAM) pData );	

		// ���� set �� �����Ͱ� 1 ���� (���� ����, ��� ����) map ���� �����Ѵ�. 
		if( (*it).second.size() == 1)
		{
			TRACEX("set �� �����Ͱ� 1 ���� (���� ����, ��� ����) map ���� ����\n");
			m_mapStr2SetMultiChat.erase(it);
		}		
	}	


	return TRUE;
}

BOOL CChatSession::doWork_SET_MULTICHAT_SERVER_ROOMNO( char* data) 
{
	//ToDo
	// �������� ���� RoomNo �� Map ����,
	
	//ROOMNO|ID|NAME|ID|NAME...
	
	char* szData = data + COMMON_HEADER_LENGTH ;
		
	// szData ���� DELIM1 ���� parse �Ͽ� ����ü ����
	int nIndex = 0;	
	int nPos = strlen(szData);
	CString strData(szData);
	
	char szTmpRoomNo[5+1];	
	char szTmpUserID[20+1];	
	char szTmpUserName[50+1];	
	memset(szTmpRoomNo, 0x00, sizeof(szTmpRoomNo));
		
	nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpRoomNo, DELIM1); // RoomNo
	
	SET_STR setMultiChatID; 
	
	while( 1)
	{
		memset(szTmpUserID, 0x00, sizeof(szTmpUserID));
		memset(szTmpUserName, 0x00, sizeof(szTmpUserName));

		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpUserID, '|'); //ID
		
		if(nPos != -1)
		{
			setMultiChatID.insert( szTmpUserID ) ; 
		}		
		
		nPos = GetStrByDelim((LPSTR)(LPCSTR)strData, szTmpUserName, '|'); //NAME
		
		CChatSession::Instance().mIDToNameMap.SetAt(szTmpUserID, szTmpUserName);
			
		if(nPos < 0)
		{			
			break;
		}
		
		nIndex++;
	}

	// Multi Chat �������� ������ Server RoomNo �� Key�� ����..
	m_mapStr2SetMultiChat.insert( MAP_STR2SET::value_type( szTmpRoomNo , setMultiChatID ) );	
	
	return TRUE;
}



BOOL CChatSession::doWork_ALL_MSG ( char* data) 
{
	
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;	
	S_CHAT_HEADER *psChatHeader = (S_CHAT_HEADER *) (data + COMMON_HEADER_LENGTH);		
	S_CHAT_CLIENT * pChatMsg = new S_CHAT_CLIENT;

	pChatMsg->strIDTo = psChatHeader->szIDTo;		
	pChatMsg->strIDFrom = psChatHeader->szIDFrom;
	pChatMsg->strMsg = data + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH ;
	pChatMsg->strMsg = pChatMsg->strMsg.Mid(0, atoi(psHeader->m_szLength) -  S_CHAT_HEADER_LENGTH - 1) ;
		
	TRACEX("szIDTo   [%s]\n", pChatMsg->strIDTo);
	TRACEX("szIDFrom [%s]\n", pChatMsg->strIDFrom);		
	
	::PostMessage(m_hOwnerWnd, WM_ALLMSG, (WPARAM)NULL, (LPARAM)pChatMsg);		
	
	//

	
//	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;	
//	S_FROMTOID_HEADER *psFromToIDHeader = (S_FROMTOID_HEADER *) (data + COMMON_HEADER_LENGTH);		
//	S_CHAT_CLIENT * pChatMsg = new S_CHAT_CLIENT;
//	pChatMsg->strIDTo = psFromToIDHeader->m_szToID;		
//	pChatMsg->strIDFrom = psFromToIDHeader->m_szFromID;
//	pChatMsg->strMsg = data + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH ;
//	pChatMsg->strMsg = pChatMsg->strMsg.Mid(0, atoi(psHeader->m_szLength) -  FROMTOID_HEADER_LENGTH - 20 - 1) ;
//		
//	TRACEX("szIDTo   [%s]\n", psFromToIDHeader->m_szToID);
//	TRACEX("szIDFrom [%s]\n",  psFromToIDHeader->m_szFromID);		
//	
//	::PostMessage(m_hOwnerWnd, WM_ALLMSG, (WPARAM)NULL, (LPARAM)pChatMsg);		
	

	return TRUE;
}

BOOL CChatSession::doWork_ALL_MSG_ACK ( char* data) 
{
	S_FROMTOID_HEADER* psFromToIdHeader = (S_FROMTOID_HEADER *) (data + COMMON_HEADER_LENGTH);		
	
	char* pData = new char[21];
	memset(pData, 0x00, 21);
	strncpy(pData, psFromToIdHeader->m_szFromID, 21);

	//::SendMessage(m_hOwnerWnd, WM_ALLMSG_ACK, (WPARAM)NULL, (LPARAM)psFromToIdHeader->m_szFromID);				
	::PostMessage(m_hOwnerWnd, WM_ALLMSG_ACK, (WPARAM)NULL, (LPARAM)pData);				
	
	return TRUE;
}




BOOL CChatSession::doWork_RAWSQL_REG_NEW_USER( char* data) 
{
	char *szData  = data + COMMON_HEADER_LENGTH;
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));

	TRACEX("RAWSQL_REG_NEW_USER\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);			
	
	::PostMessage(m_hOwnerWnd, WM_REG_NEW_USER, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_SAVE_LAST_IP( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;	
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));

	TRACEX("RAWSQL_SAVE_LAST_IP\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);
	
	::PostMessage(m_hOwnerWnd, WM_SAVE_LAST_IP, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_ALREADY_LOGIN( char* data) 
{
	::PostMessage(m_hOwnerWnd, WM_ALREADY_LOGIN, (WPARAM)NULL, (LPARAM)NULL);
	return TRUE;
}

BOOL CChatSession::doWork_LOGIN_ALLOWED( char* data) 
{
	::PostMessage(m_hOwnerWnd, WM_LOGIN_ALLOWED, (WPARAM)NULL, (LPARAM)NULL);
	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_INSERT_LOGONTIME( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;		
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));
	TRACE("RAWSQL_INSERT_LOGONTIME\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);			
	
	::PostMessage(m_hOwnerWnd, WM_INSERT_LOGONTIME, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_CHK_DUP_ID( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;		
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));

	TRACE("RAWSQL_CHK_DUP_ID\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);			
	
	
	::PostMessage(m_hOwnerWnd, WM_CHK_DUP_ID, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_CHK_USERID( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;		
	S_CHK_USERID * pRslt = new S_CHK_USERID;
	memcpy(pRslt, (S_CHK_USERID*) szData, sizeof(S_CHK_USERID));

	TRACE("RAWSQL_CHK_USERID\n");
	TRACE("szPasswd [%s]\n", pRslt->szPasswd);			
	
	::PostMessage(m_hOwnerWnd, WM_CHK_USERID, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_RAWSQL_UPDATE_PASSWD( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;		
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));

	TRACE("RAWSQL_UPDATE_PASSWD\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);			
	
	::PostMessage(m_hOwnerWnd, WM_UPDATE_PASSWD, (WPARAM)NULL, (LPARAM)pRslt);		

	return TRUE;
}

BOOL CChatSession::doWork_ADD_BUDDY_LIST_ERR( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH;		
	S_COM_RESPONSE * pRslt = new S_COM_RESPONSE;
	memcpy(pRslt, (S_COM_RESPONSE*) szData, sizeof(S_COM_RESPONSE));

	TRACE("ADD_BUDDY_LIST\n");
	TRACE("szRsltCd [%s]\n", pRslt->szRsltCd);			
	TRACE("szErrMsg   [%s]\n", pRslt->szErrMsg);			
	// Tricky!: �������ڿ��� 1400 (NULL���� ����)�� ���� �߰� ��ȭ��밡 ��ϵȻ���� �ƴѰ���̹Ƿ�
	// ������ �޽����� ��ȯ�Ѵ�.
	if(strstr(pRslt->szErrMsg, "1400"))
	{
		strcpy(pRslt->szErrMsg,"��ϵ� �����ID�� �ƴմϴ�!");
	}	
	
	::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)pRslt); // CIdmsMsnDlg �� ���޵ȴ�!!		

	return TRUE;
}

BOOL CChatSession::doWork_ACK_ONLINE( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH ;
	int nIndex = 0;		
	S_BUDDY_ONLINE *psDataTemp = NULL;		
	psDataTemp = new S_BUDDY_ONLINE;				
	memcpy(psDataTemp , szData , sizeof(S_BUDDY_ONLINE));
	
	::PostMessage(m_hOwnerWnd, WM_BUDDY_ONLINE, (WPARAM)NULL, (LPARAM)psDataTemp);				

	return TRUE;
}

BOOL CChatSession::doWork_ACK_OFFLINE( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH ;
	int nIndex = 0;		
	S_BUDDY_OFFLINE *psDataTemp = NULL;		
	psDataTemp = new S_BUDDY_OFFLINE;				
	memcpy(psDataTemp , szData , sizeof(S_BUDDY_OFFLINE));
	
	::PostMessage(m_hOwnerWnd, WM_BUDDY_OFFLINE, (WPARAM)NULL, (LPARAM)psDataTemp);					

	return TRUE;
}

BOOL CChatSession::doWork_YOUALLOWEDASCOMP_INFORM( char* data) 
{
	char* szData = data + COMMON_HEADER_LENGTH ;
	S_YOU_ALLOWED_BY *psDataTemp = NULL;
	psDataTemp = new S_YOU_ALLOWED_BY;				
	memcpy(psDataTemp , szData , sizeof(S_YOU_ALLOWED_BY));
	
	::PostMessage(m_hOwnerWnd, WM_YOU_ALLOWED_BY, (WPARAM)NULL, (LPARAM)psDataTemp);			

	return TRUE;
}

BOOL CChatSession::doWork_YOUALLOWEDASCOMP( char* data) 
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;

	S_SQL_RSLTDATA *psSqlRslt = (S_SQL_RSLTDATA *) (data + COMMON_HEADER_LENGTH ); 
	if( strncmp("N", psSqlRslt->szRsltCd, 1) == 0)
	{			
		S_COM_RESPONSE *psDataTemp = new S_COM_RESPONSE;
		strcpy(psDataTemp->szRsltCd , "N");
		strcpy(psDataTemp->szErrMsg , psSqlRslt->szErrMsg);	
		
		::PostMessage(m_hOwnerWnd, WM_SQL_ERR, (WPARAM)NULL, (LPARAM)psDataTemp); 
		return FALSE;
	}		
	
	char* szData = data + COMMON_HEADER_LENGTH + SQL_RSLTDATA_LENGTH;		
	
	S_YOU_ALLOWED *psDataTemp = NULL;
	
	psDataTemp = new S_YOU_ALLOWED;				
	memcpy(psDataTemp , szData , sizeof(S_YOU_ALLOWED));
	
	::PostMessage(m_hOwnerWnd, WM_ALLOWED_COMP, (WPARAM)NULL, (LPARAM)psDataTemp);
	
	return TRUE;
}

BOOL CChatSession::doWork_MULTI_CHAT_MSG( char* data)
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;	
	S_CHAT_HEADER *psChatHeader = (S_CHAT_HEADER *) (data + COMMON_HEADER_LENGTH);
		
	S_CHAT_CLIENT * pChatMsg = new S_CHAT_CLIENT;
	pChatMsg->strIDTo = psChatHeader->szIDTo; // Server RoomNo		
	pChatMsg->strIDFrom = psChatHeader->szIDFrom;
	pChatMsg->strMsg = data + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH;
	pChatMsg->strMsg = pChatMsg->strMsg.Mid(0, atoi(psHeader->m_szLength) -  S_CHAT_HEADER_LENGTH -1) ;
		
	::PostMessage(m_hOwnerWnd, WM_MULTI_CHATMSG, (WPARAM)NULL, (LPARAM)pChatMsg);	
	
	return TRUE;
}

BOOL CChatSession::doWork_Dummy(char* szRawData)
{
	return TRUE;
}

BOOL CChatSession::ProcessRecvData(char* data, int nPacketLen)
{
	S_COMMON_HEADER *psHeader = (S_COMMON_HEADER *)data;
	
	
	static BOOL( CChatSession::* fPArySession [] )( char* ) = 
	{	
		&CChatSession::doWork_STATUS_AWAY ,					//0
		&CChatSession::doWork_STATUS_ONLINE ,
		&CChatSession::doWork_ChatMsg ,
		&CChatSession::doWork_GetBuddyGrp ,
		&CChatSession::doWork_GetBuddyList,		
		&CChatSession::doWork_SearchID ,
		&CChatSession::doWork_RAWSQL_JOB_GN ,
		&CChatSession::doWork_RAWSQL_DAILYJOB_MST ,
		&CChatSession::doWork_Dummy ,
		&CChatSession::doWork_RAWSQL_GETUSER_INFO ,		
		&CChatSession::doWork_GetTeamInfo,					//10
		&CChatSession::doWork_GetTeamInfo,					
		&CChatSession::doWork_GetTeamInfo,					
		&CChatSession::doWork_BuddyInfoAddMe,				
		&CChatSession::doWork_BuddyInfoAddMe,				
		&CChatSession::doWork_DEL_BUDDY_GRP,				//15
		&CChatSession::doWork_DEL_BUDDYS,
		&CChatSession::doWork_CHG_BUDDY_GRP,
		&CChatSession::doWork_ADD_BUDDY_GRP,
		&CChatSession::doWork_CHG_GRP_NAME,				
		&CChatSession::doWork_YOUALLOWEDASCOMP,			//20
		&CChatSession::doWork_SOMEONE_DEL_ME,
		&CChatSession::doWork_ACK_NICK_CHG,
		&CChatSession::doWork_Dummy,
		&CChatSession::doWork_MULTICHAT_SERVER_ROOMNO,		
		&CChatSession::doWork_Dummy ,		//25
		&CChatSession::doWork_ALL_MSG ,
		&CChatSession::doWork_ALL_MSG_ACK ,
		&CChatSession::doWork_Dummy ,
		&CChatSession::doWork_Dummy,		
		&CChatSession::doWork_RAWSQL_REG_NEW_USER,			//30
		&CChatSession::doWork_RAWSQL_SAVE_LAST_IP,
		&CChatSession::doWork_ALREADY_LOGIN,
		&CChatSession::doWork_LOGIN_ALLOWED,
		&CChatSession::doWork_RAWSQL_INSERT_LOGONTIME,		
		&CChatSession::doWork_RAWSQL_CHK_DUP_ID,			//35
		&CChatSession::doWork_RAWSQL_CHK_USERID,
		&CChatSession::doWork_RAWSQL_UPDATE_PASSWD,
		&CChatSession::doWork_ADD_BUDDY_LIST_ERR,
		&CChatSession::doWork_ACK_ONLINE,					
		&CChatSession::doWork_ACK_OFFLINE,					//40
		&CChatSession::doWork_YOUALLOWEDASCOMP_INFORM,		
        &CChatSession::doWork_Dummy,
		&CChatSession::doWork_Dummy,
		&CChatSession::doWork_Dummy,
		&CChatSession::doWork_Dummy,						//45
		&CChatSession::doWork_Dummy,
		&CChatSession::doWork_MULTI_CHAT_MSG,              
		&CChatSession::doWork_SET_MULTICHAT_SERVER_ROOMNO, 
		&CChatSession::doWork_EXIT_MULTI_CHAT,				
		&CChatSession::doWork_SLIP_MSG       ,             //50 
		&CChatSession::doWork_GET_SLIP_MSG   ,             
		&CChatSession::doWork_Dummy,                       //���� �ȿ´�.
		&CChatSession::doWork_ADD_IT_COMP    ,             //
		&CChatSession::doWork_DEL_IT_COMP    ,             //
		&CChatSession::doWork_CHG_IT_COMP    ,             //55
		&CChatSession::doWork_Dummy          ,             //56  GET_COMP_LIST_INIT
		&CChatSession::doWork_STATUS_BUSY   ,              //57 
		&CChatSession::doWork_LOGIN         ,              //58
		&CChatSession::doWork_FOOD_MENU      ,             //59
		&CChatSession::doWork_ALL_USER_ID_TEAM ,           //60
		&CChatSession::doWork_Dummy          ,             //61  GET_ALL_USER_INIT 
		&CChatSession::doWork_ALL_USER_ID_TEAM,            //62
		&CChatSession::doWork_Dummy           ,            //63  UPDATE_ALL_USER_INIT
		&CChatSession::doWork_Dummy           ,            //64  ALERT_PENDING_WORK 
		&CChatSession::doWork_ALIVE                        //65  IM_ALIVE
	} ;
	
	TRACE("�� CChatSession::ProcessRecvData [%s] \n", psHeader->m_szUsage );

//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "[%s] m_szUsage [%s]\n" , iDT.Format("%Y%m%d%H%M%S") ,psHeader->m_szUsage );		
	
	(this->*fPArySession[ atoi(psHeader->m_szUsage) ]) (data);	

	return TRUE;
}



BOOL CChatSession::SendAllMsgAck(const char* szToID, const char* szMyNick, const char* szMyRealName)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];

	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
				
	// ����� ����	
	int iLength = FROMTOID_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, ALL_MSG_ACK, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 
	memcpy(sFromToIDHeader.m_szToID, szToID, min(sizeof(sFromToIDHeader.m_szToID), strlen(szToID))); 
	memcpy(sFromToIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromToIDHeader.m_szFromID), strlen(m_szMyUserID) ) ); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		

	if(Send(pSzBuf, nTotalLen,"SendAllMsgAck") < 0)	
	{
		TRACEX("CChatSession::SendAllMsgAck -> Send return FAIL!\n");		
		return FALSE ;
	}				
	

	return TRUE;
}

// Multi Chat RoomNo Get
BOOL CChatSession::RequestMultiChatRoomNo( const char* szDlgID, CString& strIDNames)
{
	char* pSzBuf = NULL;
	S_COMMON_HEADER sComHeader;			
	int nTotalLen = 0;	
	S_FROMTOID_HEADER sFromToIDHeader;
	char szTemp[20];

	memset(&sFromToIDHeader, NULL, sizeof(sFromToIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	// S_COMMON_HEADER + S_FROMTOID_HEADER + DlgMapKey������UserId������UserId...
	// ����� ����	
	char szDlgKeyDelim [15+1+1]; // ������ ����
	memset(szDlgKeyDelim, 0x00, sizeof(szDlgKeyDelim));
	sprintf(szDlgKeyDelim, "%s%c", szDlgID, DELIM1);

	int iLength = FROMTOID_HEADER_LENGTH + strIDNames.GetLength() + strlen(szDlgKeyDelim) + 1; // +1 NULL	 ; 
		
	memcpy(sComHeader.m_szUsage, MULTICHAT_GET_ROOMNO, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));
	
	memcpy(sFromToIDHeader.m_szIP, m_szMyIPAddr, min(sizeof(sFromToIDHeader.m_szIP), strlen(m_szMyIPAddr))); 	
	memcpy(sFromToIDHeader.m_szFromID, m_szMyUserID, min(sizeof(sFromToIDHeader.m_szFromID), strlen(m_szMyUserID) ) ); 
		
	// ���۵����� ������			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ];
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf + COMMON_HEADER_LENGTH, &sFromToIDHeader, FROMTOID_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH, szDlgKeyDelim, strlen(szDlgKeyDelim) );
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + FROMTOID_HEADER_LENGTH + strlen(szDlgKeyDelim), (LPCSTR)strIDNames, strIDNames.GetLength());
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;		
	
	if(Send(pSzBuf, nTotalLen,"RequestMultiChatRoomNo") < 0)	
	{
		TRACEX("CChatSession::RequestMultiChatRoomNo -> Send return FAIL!\n");		
		return FALSE ;
	}	
    
	return TRUE;
}


/* Performance !!

int CChatSession::GetStrByDelim(string& strSrc, string& strOut, char cDeli)
{
	string strRtn = "NotFound";

	int nPos = strSrc.find(cDeli);
	
	if(nPos == -1)
	{
		strOut = strRtn;		
		return nPos;
	}

	strRtn = strSrc.substr(0, nPos);
	strOut = strRtn;
	strSrc = strSrc.substr(nPos+1, strSrc.length());
	
	return nPos;	
}

*/

/*

* ���ڿ����� �������� ��ġ���� �ش�Ǵ� �����͸� ���Ѵ�.
* (I) 1. char* pstr   : ������ϴ� ���ڿ� ������
*     2. char delimit : ������
* 	  3. char* pdat	  : ������ ������
*	  4. int position : �������� ��ġ(0���� ����)
* (R) 1. int          : ������ �������� ����

int		GetDelimitData(char *pstr, char delimit, char *pdat, int position)
{
	char	data[256];
	int		size = strlen(pstr);
	int		count, point;
	int		i;

	memset(data, 0x00, sizeof(data));

	for ( i=0, count=0, point=0; i<size; i++ ) {
		char ch = pstr[i];
		if ( ch == delimit ) {
			count++;
			continue;
		}
		
		if ( position != count ) continue;

		data[point++] = ch;	
	}

	return sprintf(pdat, "%s", data);	
}	
 */

/*
int CChatSession::GetStrByDelimIndexWithChar(CString strSrc,CString& strOut, int nDeliNum, char cDeli)
{
	CString strRtn = "NotFound";
	int nLen = strSrc.GetLength();
	int nPos = 0;
	int i = 0;
	
	while(1){
		if(i == nDeliNum){
			
			int nPos = strSrc.Find(cDeli);
			if(nPos == -1)
			{
				strOut = strRtn;
				return nPos;
			}
			strRtn = strSrc.Mid(0, nPos);
			strOut = strRtn;
			return nPos;
		}else{
			nPos = strSrc.Find(cDeli);			
			strSrc = strSrc.Mid(nPos+1);			
		}
		
		i++;
	}	
	
	strOut = strRtn;
	return -1;
	
}
*/


// Full Path ���� Root Path �� �����Ѵ� 
/*
CString CChatSession::getFileNamePathExceptRoot(CString strFileWithPath, CString strRootPath)
{
	CString strFileName;
	int nPos = 0;
	int nLen = 0;
	int nRootLen = 0;	
	
	strFileName = strFileWithPath;
	nLen = strFileName.GetLength();
	nRootLen = strRootPath.GetLength();
	
	TRACE("nLen [%d] nRootLen [%d]\n", nLen, nRootLen);

	strFileName = strFileName.Mid(nRootLen+1, nLen-nRootLen);	
	
	return strFileName;
}
*/

// ���ϸ� ����
/*
CString CChatSession::getFileNameOnly(CString strFileWithPath)
{
	CString strFileName;
	int nPos = 0;
	int nLen = 0;
	
	strFileName = strFileWithPath;
	nPos = strFileName.Find("\\");
	
	while(nPos >= 0)
	{
		nLen = strFileName.GetLength();
		nPos = strFileName.Find("\\");
		if(nPos <= 0)
			break;
		
		strFileName = strFileName.Mid(nPos+1, nLen-nPos);
	}	
	
	return strFileName;
}
*/


