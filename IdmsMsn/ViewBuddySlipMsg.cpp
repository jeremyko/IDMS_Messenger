// ViewBuddySlipMsg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "ViewBuddySlipMsg.h"
#include "ChatSession.h"
#include "INIManager.h"

extern "C"
{
    #include <direct.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewBuddySlipMsg dialog


CViewBuddySlipMsg::CViewBuddySlipMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewBuddySlipMsg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewBuddySlipMsg)
	m_stsFromInfo = _T("");
	m_REditMsg = _T("");
	//}}AFX_DATA_INIT
}


CViewBuddySlipMsg::CViewBuddySlipMsg(S_SLIP_MSG *pData, CWnd* pParent /*=NULL*/)
	: CDialog(CViewBuddySlipMsg::IDD, pParent)
{
	memcpy( &m_sSlipMsg, pData, sizeof(m_sSlipMsg));
	TRACE("");
}


void CViewBuddySlipMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewBuddySlipMsg)
	DDX_Text(pDX, IDC_STATIC_MSG_FROM, m_stsFromInfo);
	DDX_Text(pDX, IDC_REDIT_SLIP_MSG, m_REditMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewBuddySlipMsg, CDialog)
	//{{AFX_MSG_MAP(CViewBuddySlipMsg)
	ON_BN_CLICKED(IDC_BTN_SLIP_MSG_CLOSE, OnBtnSlipMsgClose)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewBuddySlipMsg message handlers

void CViewBuddySlipMsg::OnBtnSlipMsgClose() 
{
	// TODO: Add your control notification handler code here

	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();

	//CChatSession::Instance().RequestSlipMsgAck(m_sSlipMsg.m_szFromID, m_sSlipMsg.m_szRegDate);

	char szSql [200];
	memset(szSql , 0x00, sizeof(szSql));
	//sprintf(szSql, "UPDATE BUDDY_MSG SET CHKED='Y' WHERE FROM_ID ='%s' AND TO_ID ='%s' AND REG_DATE=TO_DATE('%s','YYYYMMDDHH24MISS')", 
	//	m_sSlipMsg.m_szFromID, CChatSession::Instance().m_szMyUserID,  m_sSlipMsg.m_szRegDate  ) ;

	//삭제
	sprintf(szSql, "DELETE FROM BUDDY_MSG WHERE FROM_ID ='%s' AND TO_ID ='%s' AND REG_DATE=TO_DATE('%s','YYYYMMDDHH24MISS')", 
		m_sSlipMsg.m_szFromID, CChatSession::Instance().m_szMyUserID,  m_sSlipMsg.m_szRegDate  ) ;

	if(CChatSession::Instance().RequestRawSQL(RAWSQL_ACK_SLIP_MSG, szSql, strlen(szSql) ) != TRUE)
	{
		TRACE("OnBtnSlipMsgClose 실패!\n");
	}


	char szKey [20+14+1+1];
	memset(szKey, 0x00, sizeof(szKey));
	sprintf(szKey, "%s%s", m_sSlipMsg.m_szFromID ,m_sSlipMsg.m_szRegDate);

	pMainWnd->PostMessage(WM_SLIPMSG_DLG_CLOSE, (WPARAM)szKey, (LPARAM)0);

	CDialog::OnOK();
	
}

BOOL CViewBuddySlipMsg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	char szSentTm[40]; //2006년12월12일 11시23분11초
	string strSrc(m_sSlipMsg.m_szRegDate);

	memset(szSentTm, 0x00, sizeof(szSentTm));

	sprintf	(szSentTm, "%s년 %s월 %s일 %s시 %s분 %s초", 
		              (strSrc.substr(0,4)).c_str(),
					  (strSrc.substr(4,2)).c_str(),
					  (strSrc.substr(6,2)).c_str(),
					  (strSrc.substr(8,2)).c_str(),
					  (strSrc.substr(10,2)).c_str(),
					  (strSrc.substr(12,2)).c_str()
			);
	

	m_stsFromInfo.Format("%s(%s)님이 [%s]시간에 보낸 쪽지입니다", m_sSlipMsg.m_szFromName, m_sSlipMsg.m_szFromID, szSentTm) ;
	m_REditMsg = m_sSlipMsg.m_szMsg;
	UpdateData(FALSE);

	//Log
	/////////////////////////////////////////////////////////
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
	
	//char szWinDir[512];		
	//memset(szWinDir, 0x00, sizeof(szWinDir));		
	//GetWindowsDirectory(szWinDir, sizeof(szWinDir));

	char szDirTmp[512];	
	char szDir[1024];
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	if(strUseLogFolder == "Y")
	{
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strcpy( szDirTmp, (LPCSTR)strLogFolderPath);
		
		if(szDirTmp[ strlen(szDirTmp) - 1 ] == '\\')
		{
			szDirTmp[ strlen(szDirTmp) - 1 ] = 0x00 ;
		}
				
		strFileName.Format("%s\\받은쪽지\\",szDirTmp);	
	}
	else
	{		
		char szDirTmp[512];				
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
						
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

		// "D:\DATA\DevelopSource\CD8NotBK\newMessenger\BinRelease\IdmsMsn.exe"
		// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\newMessenger\ 까지 구한다.
		
		int nPos = 0;
		int nLen = strlen(szDirTmp);
		for(int i = nLen; i>=0 ; i--)
		{
			if(szDirTmp[i] == '\\' && nPos <1 )
			{
				szDirTmp[i] = '\0';
				nPos++;
				break;
			}
		}

		TRACE("%s\n" , szDirTmp);	
		
		strFileName.Format("%s\\받은쪽지\\",szDirTmp);	
	}
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}

	CString strTmp;	
	CTime iDT =  CTime::GetCurrentTime();
	CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
	CString strtmSimple = iDT.Format("%Y%m%d");

	CString strFileNameAll;	
	strFileNameAll.Format("%s\\[%s]님이 보낸쪽지_%s.txt", strFileName, m_sSlipMsg.m_szFromName, strtmSimple);
	strTmp.Format("보낸시간[%s] 받은 시간[%s] :\r\n%s\n", m_sSlipMsg.m_szRegDate, strtmDetailed, m_sSlipMsg.m_szMsg);	
	writeLogFile(strFileNameAll, "▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\r\n");
	writeLogFile(strFileNameAll, "%s\n", strTmp);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CViewBuddySlipMsg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();

	char szKey [20+14+1+1];
	memset(szKey, 0x00, sizeof(szKey));
	sprintf(szKey, "%s%s", m_sSlipMsg.m_szFromID ,m_sSlipMsg.m_szRegDate);

	pMainWnd->PostMessage(WM_SLIPMSG_DLG_CLOSE, (WPARAM)szKey, (LPARAM)0);
	
	CDialog::OnClose();
}
