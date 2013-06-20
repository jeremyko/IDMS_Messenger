// DbLoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OraMultithrd.h" // Oracle
#include "IDMS_server.h"
#include "DbLoginDlg.h"
#include "MemoryPool.h"
#include "INIManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDbLoginDlg dialog


CDbLoginDlg::CDbLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDbLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDbLoginDlg)
	m_strConn = _T("");
	m_strPasswd = _T("");
	m_strUser = _T("");
	//}}AFX_DATA_INIT

	m_bDbRun = FALSE;
}


void CDbLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDbLoginDlg)
	DDX_Text(pDX, IDC_EDIT_CONNECTION, m_strConn);
	DDV_MaxChars(pDX, m_strConn, 255);
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_strPasswd);
	DDV_MaxChars(pDX, m_strPasswd, 22);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUser);
	DDV_MaxChars(pDX, m_strUser, 20);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDbLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CDbLoginDlg)
	ON_BN_CLICKED(IDC_BTN_DB_LOGIN, OnBtnDbLogin)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DB_VALID,   OnConnectOK)	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDbLoginDlg message handlers

void CDbLoginDlg::OnBtnDbLogin() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	char szSql [100];	
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "SELECT 1 FROM DUAL") ; // ID/PASS 확인		 
	
	COracleWorkerMT * pSqlWork = CMemoryPool::New_COracleWorkerMT();
	
	CServerGlobal::Instance().m_strDBConn = m_strConn;  
	CServerGlobal::Instance().m_strUser = m_strUser ; 
	CServerGlobal::Instance().m_strPassWd = m_strPasswd;

	pSqlWork->SetSQL(szSql);    
	pSqlWork->SetID(USAGE_CHK_DB, "", "", "");
	HWND DlgHwnd = this->GetSafeHwnd();
	pSqlWork->setNotiProcWnd(SQLNotifyProc , DlgHwnd ); 
	
	global::CThreadPool::instance().queue_request( pSqlWork );	

	//CLog::Instance().WriteFile(FALSE, "test",  "New_COracleWorkerMT index [%d] [%d]\n" , pSqlWork->m_nindex , (DWORD)pSqlWork);	
}


// DB 작업 요청시 call back..
void CDbLoginDlg::SQLNotifyProc( S_CALLBACKINFO* pSCallBackInfo  )
{
	
	//S_CALLBACKINFO* pSCallBack = new S_CALLBACKINFO;
	//memcpy(pSCallBack , pSCallBackInfo, sizeof(S_CALLBACKINFO) ) ;

	if(strcmp( USAGE_CHK_DB, pSCallBackInfo->m_szUsage) == 0)
	{		
		if(pSCallBackInfo->m_bRslt)
		{
			::PostMessage(pSCallBackInfo->m_pWnd , WM_DB_VALID, 0,0);		
		}
		else
		{
			AfxMessageBox(pSCallBackInfo->m_szErrMsg );			
		}
	}	

	CMemoryPool::Delete_COracleWorkerMT( pSCallBackInfo->m_pCOracleWorkerMT);
}


	
LRESULT CDbLoginDlg::OnConnectOK(WPARAM wParam, LPARAM lParam)
{
	m_bDbRun = TRUE;
	CDialog::OnOK();	

	return TRUE;
}


void CDbLoginDlg::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	m_bDbRun = FALSE;
	CDialog::OnCancel();
}

BOOL CDbLoginDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//m_strConn = "IDMS";
	//m_strUser = "sicc";
	//m_strPasswd = "sicc";
	
	CINIManager::Instance().GetValue(INI_SERVER_DB_SID, m_strConn);	
	CINIManager::Instance().GetValue(INI_SERVER_DB_USER, m_strUser);	
	CINIManager::Instance().GetValue(INI_SERVER_DB_PASS, m_strPasswd);	

	UpdateData(FALSE);
		
	global::CThreadPool::instance().initialize();   // Thread_Pool 구동.. 

	//OnBtnDbLogin() ; //20070119 자동 로그인되게 

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



