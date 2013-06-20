// IDMS_ServerDlg.cpp : implementation file
//

#define _WIN32_WINNT 0x0500

#include "stdafx.h"
#include "IDMS_Server.h"
#include "IDMS_ServerDlg.h"
#include "ThreadSockAsyncServer.h"
#include "INIManager.h"
#include "DbLoginDlg.h"
#include "MemoryPool.h"
#include "FoodMenuDlg.h"
#include "log.h"

#include <time.h> 
#include  <io.h>

#include <MSWSOCK.H>		// MFC socket extensions

using namespace plugware;

#ifdef _DEBUG
#define new DEBUG_NEW 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


COLORREF g_colorText ,  g_colorBg ;
CThreadSockAsyncServer *g_thread; //�����۾��� Server ������

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

		
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP	

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerDlg dialog

CIDMS_ServerDlg::CIDMS_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIDMS_ServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIDMS_ServerDlg)
	m_nConnCnt = 0;
	m_StrFilterIP1 = _T("");
	m_StrFilterIP2 = _T("");
	m_StrServerRunTime = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	nChkLog = 1 ; 
	nChkFilterIP = 1 ;
}

void CIDMS_ServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIDMS_ServerDlg)
	DDX_Control(pDX, IDC_BUTTON_EXIT, m_BtnExit);
	DDX_Control(pDX, IDC_RICHEDIT1, m_LogWnd);
	DDX_Text(pDX, IDC_EDIT_CONNECTION, m_nConnCnt);
	DDV_MinMaxUInt(pDX, m_nConnCnt, 0, 4294967295);
	DDX_Text(pDX, IDC_EDIT_FILTER_IP1, m_StrFilterIP1);
	DDV_MaxChars(pDX, m_StrFilterIP1, 20);
	DDX_Text(pDX, IDC_EDIT_FILTER_IP2, m_StrFilterIP2);
	DDV_MaxChars(pDX, m_StrFilterIP2, 20);
	DDX_Text(pDX, IDC_STATIC_RUNTIME, m_StrServerRunTime);
	DDV_MaxChars(pDX, m_StrServerRunTime, 60);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIDMS_ServerDlg, CDialog)
	//{{AFX_MSG_MAP(CIDMS_ServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_EXIT, OnButtonExit)		
	ON_WM_CLOSE()	
	ON_BN_CLICKED(IDC_BTN_CLEAR_LIST, OnBtnClearList)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_UPDATE_USER_LIST, OnBtnUpdateUserList)
	ON_BN_CLICKED(IDC_BTN_FOOD, OnBtnFood)
	ON_BN_CLICKED(IDC_CHECK_LOG, OnCheckLog)
	ON_BN_CLICKED(IDC_CHECK_FILTERIP, OnCheckFilterip)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_FILTER_IP1, OnChangeEditFilterIp1)
	ON_EN_CHANGE(IDC_EDIT_FILTER_IP2, OnChangeEditFilterIp2)	
	//}}AFX_MSG_MAP

	
	ON_MESSAGE(WM_CONCNT, OnConnectionCnt)	
	ON_MESSAGE(WM_IMADEUSERLIST, OnIMadeUserList)	
	ON_MESSAGE(WM_IMADE_ALL_USERLIST, OnIMadeAllUserList)	
		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerDlg message handlers

void CIDMS_ServerDlg::ShowOsWarning()
{
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osv);

	if(osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		//m_strStatus = "���� ���� ����";
		UpdateData(FALSE);
		MessageBox("Windows98,95 ������ ���ư��� �ʽ��ϴ�.", "�˸�", 0);
	}	
}

BOOL CIDMS_ServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// MEM POOL
	CMemoryPool::SetupMemoryPool();
	CINIManager::Instance().LoadINIFile();
	
	m_strDailyJobTime ="";
	m_strDailyChkAliveTime = "";

	CDbLoginDlg dlg;
	dlg.DoModal();

	if(dlg.m_bDbRun == FALSE)
	{
		CMemoryPool::AllDeleteMemory(); //20070119
		CINIManager::Instance().SaveINIFile();
		CDialog::OnOK();
		return FALSE;
	}
		
	m_strDBConn = dlg.m_strConn; 
	m_strUser = dlg.m_strUser;
	m_strPassWd = dlg.m_strPasswd;
	
	g_colorText = RGB(238,243,190);  
	g_colorBg   = RGB(0,64,128);

	// TODO: Add extra initialization here
	bServerRuning = FALSE;	
		
	g_thread = NULL;
	g_thread = new CThreadSockAsyncServer(this->GetSafeHwnd());		
		
	g_thread->SetLoginInfo( (LPCSTR) m_strDBConn, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd);
	g_thread->SetNotifyProc(NotifyProc);
		
	m_LogWnd.SetBackgroundColor( FALSE, g_colorBg );
	
	//������ ��ȭ��� ����Ʈ, ��ü����� �ϼ��Ҷ��� ��ٷȴٰ� Start() ȣ����!	
	int nStart = m_LogWnd.GetWindowTextLength();	
	m_LogWnd.SetSel(nStart, nStart);
	m_LogWnd.ReplaceSel("\r\n" );
	m_LogWnd.ReplaceSel("�� ���� �ʱ�ȭ ���Դϴ�. ��ø� ��ٸ�����" );
		
	int lenAft = m_LogWnd.GetWindowTextLength();
	m_LogWnd.SetSel( nStart, lenAft );	
		
	CHARFORMAT2 cf;
	m_LogWnd.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;	
	cf.crTextColor = g_colorText ;
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_LogWnd.SetSelectionCharFormat(cf);      		
	::SendMessage(m_LogWnd.GetSafeHwnd(), EM_SCROLLCARET, 0,0);	
	m_LogWnd.HideSelection(TRUE, TRUE); 

	//OnIMadeAllUserList(NULL, 0) ; //TEST ONLY
	
	g_thread->makeUserList(); 	
			
	SetTimer(200, CHK_TIMER_SEC , NULL); 
			
	SetResize() ;

	CString strFileName;
	char szWinDir[512];			
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset( szDir, 0x00, sizeof(szDir));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle(IDMS_MSN_SVR_NAME),szDir, sizeof(szDir));	
	int nPos = 0;
	int nLen = strlen(szDir);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDir[i] == '\\' && nPos <1 )
		{
			szDir[i] = '\0';
			nPos++;
			break;
		}
	}
	strcat(szDir,"\\server_log\\");

	CLog::Instance().SetLogPath(szDir);

	TRACE("CLog::Instance().GetLogPath(): %s\n" , CLog::Instance().GetLogPath() );		
	
	if (_access( CLog::Instance().GetLogPath() ,0) ==-1 ) 
	{
		CreateDirectory( CLog::Instance().GetLogPath(), NULL);		
	}
	
	CFoodMenuDlg Fooddlg;
	Fooddlg.Load(TRUE) ;
	m_strMenu = Fooddlg.m_strMenu ;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIDMS_ServerDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);		
	m_cControlPos.AddControl(&m_LogWnd	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );		
	m_cControlPos.AddControl(&m_BtnExit	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL	);		
	m_cControlPos.AddControl( IDC_STATIC_GRP_LOG, CP_RESIZE_HORIZONTAL	);		

	m_cControlPos.AddControl( IDC_BTN_CLEAR_LIST, CP_MOVE_VERTICAL	);		
	m_cControlPos.AddControl( IDC_BTN_UPDATE_USER_LIST, CP_MOVE_VERTICAL	);		
	m_cControlPos.AddControl( IDC_BTN_FOOD, CP_MOVE_VERTICAL	);			
	m_cControlPos.AddControl( IDC_BTN_CHK_CONNECTION, CP_MOVE_VERTICAL	);		
}


void CIDMS_ServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIDMS_ServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIDMS_ServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CIDMS_ServerDlg::OnButtonExit() 
{
	// TODO: Add your control notification handler code here
	int nRtn = AfxMessageBox("IDMS �޽��� �����Դϴ�. \n�޽����� ����Ҽ� ���� �˴ϴ�. \n���� ���� �Ͻðڽ��ϱ�? ",MB_OKCANCEL);
	
	if(IDOK == nRtn )
	{
		//OShutdown();
		KillTimer(200);
		
		if(g_thread)
		{
			g_thread->Stop(TRUE);
			delete g_thread; 
			g_thread = NULL;
		}	
				
		CINIManager::Instance().SaveINIFile();
		
		CLog::Instance().WriteFile(FALSE, "ServerLog", "�� ����ڿ� ���� ���� ����\n" );		

		CDialog::OnOK();
	}
}

// ���� ���¸� ȭ�鿡 ��Ÿ��
void CIDMS_ServerDlg::NotifyProc(LPVOID lpParam,  char *msg, long* nCnt)
{
	CIDMS_ServerDlg* pDlg = (CIDMS_ServerDlg*) lpParam;
	
	int nStart = pDlg->m_LogWnd.GetWindowTextLength();	
	pDlg->m_LogWnd.SetSel(nStart, nStart);
	
	CString strMsg = msg;
	strMsg.Replace(DELIM1,'\n');
	pDlg->m_LogWnd.ReplaceSel(strMsg );	
		
	int lenAft = pDlg->m_LogWnd.GetWindowTextLength();
	pDlg->m_LogWnd.SetSel( nStart, lenAft );	
	
	CHARFORMAT2 cf;
	pDlg->m_LogWnd.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;	
	cf.crTextColor = g_colorText ;
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	pDlg->m_LogWnd.SetSelectionCharFormat(cf);      
	
	::SendMessage(pDlg->m_LogWnd.GetSafeHwnd(), EM_SCROLLCARET, 0,0);	
	pDlg->m_LogWnd.HideSelection(TRUE, TRUE);

	if(nCnt)
		pDlg->SendMessage(WM_CONCNT, NULL, (LPARAM) *nCnt );

	////////////////////
	/*
	int nStart = pDlg->m_LogWnd.GetWindowTextLength();
	pDlg->m_LogWnd.SetSel(nStart, nStart);

	CString strMsg = msg;
	strMsg.Replace(DELIM1,'\n');

	pDlg->m_LogWnd.ReplaceSel(strMsg );
	pDlg->m_LogWnd.HideSelection(TRUE, TRUE); 
	pDlg->m_LogWnd.SendMessage(EM_SCROLLCARET, 0);		
		
	if(nCnt)
		pDlg->SendMessage(WM_CONCNT, NULL, (LPARAM) *nCnt );
	*/
}

LRESULT CIDMS_ServerDlg::OnConnectionCnt(WPARAM wParam, LPARAM lParam)
{
	UINT nCnt = (UINT)lParam;
	m_nConnCnt = nCnt;

	UpdateData(FALSE);

	return TRUE;
}

LRESULT CIDMS_ServerDlg::OnIMadeUserList(WPARAM wParam, LPARAM lParam)
{
	// ����ں� ��ȭ��� ����� �����Ͽ���.
	// ����, ��ü ����� ����� �����Ѵ�.	

	// LOG++++++++++++++++++++++++++++++++++++++++++++++	
	CLog::Instance().WriteFile(FALSE, "ServerLog",  "OnIMadeUserList : makeAllUserList CALL! \n");	
	
	g_thread->makeAllUserList();	

	return TRUE;
}

LRESULT CIDMS_ServerDlg::OnIMadeAllUserList(WPARAM wParam, LPARAM lParam)
{	
	// ����� ����� ������ ������ Accept �����ϰ� ������ ���۽�Ų��
	if (0 != g_thread->Start())
	{
		AfxMessageBox("ERROR : CThreadSockAsyncServer Start fail !");
		TRACE(_T("CThreadSockAsyncServer Start ERROR \n"));
		delete g_thread;		
		g_thread = NULL;
	}		

	int nStart = m_LogWnd.GetWindowTextLength();	
	m_LogWnd.SetSel(nStart, nStart);
	m_LogWnd.ReplaceSel("\n�� IDMS �޽��� ������ �⵿�Ǿ����ϴ�" );	
	
	int lenAft = m_LogWnd.GetWindowTextLength();
	m_LogWnd.SetSel( nStart, lenAft );	
	
	CHARFORMAT2 cf;
	m_LogWnd.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
	
	cf.crTextColor = g_colorText ;
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_LogWnd.SetSelectionCharFormat(cf);      
	
	::SendMessage(m_LogWnd.GetSafeHwnd(), EM_SCROLLCARET, 0,0);	
	m_LogWnd.HideSelection(TRUE, TRUE);
		
	SetTimer(300, CHK_TIMER_SEC , NULL);  // chk alive => �ֱ������� �ð�üũ�ؼ�, ���� 4�ÿ� �۾��ϰ���

	CTime iDT =  CTime::GetCurrentTime();	
	
	m_StrServerRunTime.Format("���� ���� �ð� : %s", iDT.Format("%Y/%m/%d - %H:%M:%S") ); 
	
	UpdateData(FALSE);
	return TRUE;
}

	

void CIDMS_ServerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CLog::Instance().WriteFile(FALSE, "ServerLog", "OnClose\n" );	
	CDialog::OnClose();
}

void CIDMS_ServerDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class

	CLog::Instance().WriteFile(FALSE, "ServerLog", "PostNcDestroy\n" );	

	if(g_thread)
	{
		g_thread->Stop(TRUE);
		delete g_thread; 
		g_thread = NULL;
	}

	CDialog::PostNcDestroy();
}


void CIDMS_ServerDlg::OnBtnClearList() 
{
	// TODO: Add your control notification handler code here
		
	m_LogWnd.SetSel(0, -1);	
	m_LogWnd.ReplaceSel( "" );
	m_LogWnd.HideSelection(TRUE, TRUE); 
	m_LogWnd.SendMessage(EM_SCROLLCARET, 0);
	
	//g_thread->TestClose(); //TEST
}

BOOL CIDMS_ServerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		return TRUE;
	}

	
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CIDMS_ServerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if( 200 == nIDEvent )
	{
		int nLineCnt = m_LogWnd.GetLineCount();
		if(nLineCnt > 1000)
		{
			m_LogWnd.SetSel(0, -1);	
			m_LogWnd.ReplaceSel( "" );
			m_LogWnd.HideSelection(TRUE, TRUE); 
			m_LogWnd.SendMessage(EM_SCROLLCARET, 0);	
		}
		
		CTime ctTime = CTime::GetCurrentTime();

		if("" == m_strDailyJobTime)
		{
			if( 10 == atoi(ctTime.Format("%H")) )
			{				
				// �����⵿ �ð��� 10�ø� �ٷ� ����.
				KillTimer(200);
				
				g_thread->AlertPendingWork();				

				CTime		ctCurrent;
				CTime		ctDaily;
				ctCurrent = CTime::GetCurrentTime();
				ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
				ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
								ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());

				m_strDailyJobTime = ctDaily.Format("%Y%m%d%H");
				
				SetTimer(200, CHK_TIMER_SEC , NULL);
			}
			else if( 10 > atoi(ctTime.Format("%H"))  )
			{				
				// �����⵿ �ð��� 10 �� �����̸� ���� 10�÷� ����.
				KillTimer(200);	
				
				CTime		ctDaily;
				ctDaily = CTime::GetCurrentTime();				
		
				m_strDailyJobTime = ctDaily.Format("%Y%m%d10");
				
				SetTimer(200, CHK_TIMER_SEC , NULL);
			}
			else
			{
				// �� �ܴ̿� ������ 10�÷� ���� 
								
				KillTimer(200);		
				
				CTime		ctCurrent;
				CTime		ctDaily;
				ctCurrent = CTime::GetCurrentTime();
				ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
				ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
					ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());
				
				m_strDailyJobTime = ctDaily.Format("%Y%m%d10");				
								
				SetTimer(200, CHK_TIMER_SEC , NULL);				
			}
		}
		else 
		{
			if( ctTime.Format("%Y%m%d%H") == m_strDailyJobTime )
			{
				KillTimer(200); 				
							
				g_thread->AlertPendingWork();					
				
				CTime		ctCurrent;
				CTime		ctDaily;
				
				ctCurrent = CTime::GetCurrentTime();
				ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
				ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
					ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());
				
				m_strDailyJobTime = ctDaily.Format("%Y%m%d10");
				

				SetTimer(200, CHK_TIMER_SEC , NULL);
			}
		}		
	}	
	
	if( 300 == nIDEvent ) // chk alive
	{
		CTime ctTime = CTime::GetCurrentTime();

		//TEST
		/*
		KillTimer(300); 				
							
		g_thread->ChkSockAlive();					
		
		CTime		ctCurrent;
		CTime		ctDaily;
		
		ctCurrent = CTime::GetCurrentTime();
		ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
		ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
			ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());
		
		m_strDailyChkAliveTime = ctDaily.Format("%Y%m%d04");
		
		SetTimer(300, CHK_TIMER_SEC , NULL);
		*/
		//TEST

		
		if("" == m_strDailyChkAliveTime)
		{			
			if( 4 > atoi(ctTime.Format("%H"))  )
			{				
				// �����⵿ �ð��� 04 �� �����̸� ���� 04 �÷� ����.
				KillTimer(300);	
				
				CTime		ctDaily;
				ctDaily = CTime::GetCurrentTime();				
		
				m_strDailyChkAliveTime = ctDaily.Format("%Y%m%d04");
				
				SetTimer(300, CHK_TIMER_SEC , NULL);

				CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "ChkSockAlive ȣ�� �ð� ����[%s]" , (LPCSTR) m_strDailyChkAliveTime);	
			}
			else
			{
				// �� �ܴ̿� ������ 04 �÷� ���� 
								
				KillTimer(300);		
				
				CTime		ctCurrent;
				CTime		ctDaily;
				ctCurrent = CTime::GetCurrentTime();
				ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
				ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
					ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());
				
				m_strDailyChkAliveTime = ctDaily.Format("%Y%m%d04");				
								
				SetTimer(300, CHK_TIMER_SEC , NULL);
				
				CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "ChkSockAlive ȣ�� �ð� ����[%s]" , (LPCSTR) m_strDailyChkAliveTime);	
			}
		}
		else 
		{
			if( ctTime.Format("%Y%m%d%H") == m_strDailyChkAliveTime )
			{
				KillTimer(300); 				
							
				CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "ChkSockAlive ȣ�� ����, �۾��ð� [%s]" , (LPCSTR) m_strDailyChkAliveTime);	

				g_thread->ChkSockAlive();					
				
				CTime		ctCurrent;
				CTime		ctDaily;
				
				ctCurrent = CTime::GetCurrentTime();
				ctCurrent += CTimeSpan(1, 0, 0, 0); // 1 day
				ctDaily = CTime(ctCurrent.GetYear(), ctCurrent.GetMonth(), ctCurrent.GetDay(),
					ctCurrent.GetHour(), ctCurrent.GetMinute(), ctCurrent.GetSecond());
				
				m_strDailyChkAliveTime = ctDaily.Format("%Y%m%d04");

				CLog::Instance().WriteFile(FALSE, "ChkSockAlive",  "ChkSockAlive ȣ�� ����, ���ο� �۾��ð� [%s]", (LPCSTR) m_strDailyChkAliveTime );	
				
				SetTimer(300, CHK_TIMER_SEC , NULL);
			}
		}		
	}

	CDialog::OnTimer(nIDEvent);
}



void CIDMS_ServerDlg::OnBtnUpdateUserList() 
{
	// TODO: Add your control notification handler code here
	CLog::Instance().WriteFile(FALSE, "ServerLog",  "�� OnBtnUpdateUserList \n" );	
		
	g_thread->UpdateAllUserList();	 //TEST

}

LRESULT CIDMS_ServerDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(message == WM_QUERYENDSESSION )		
	{
		CLog::Instance().WriteFile(FALSE, "ServerLog",  "�� ������ OS ���ῡ ���� ���� ���� \n" );			
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}

//class test{
//public:
//	int kkk;
//};


void CIDMS_ServerDlg::OnBtnFood() 
{	
	//test* testt;
	//testt->kkk = 0;

	// TODO: Add your control notification handler code here
	CFoodMenuDlg dlg;

	dlg.DoModal();
	
	if(dlg.m_bChnged == TRUE)
	{
		m_strMenu = dlg.m_strMenu ;		
	}
}

void CIDMS_ServerDlg::OnCheckLog() 
{
	// TODO: Add your control notification handler code here

	if( nChkLog == 1 )
	{
		g_thread->m_bLog = TRUE;
	}
	else
	{
		g_thread->m_bLog = FALSE;
	}
	
	nChkLog *= -1;	
}

void CIDMS_ServerDlg::OnCheckFilterip() 
{
	// TODO: Add your control notification handler code here	

	if( nChkFilterIP == 1 )
	{
		g_thread->m_bChkFilterIP = TRUE;
	}
	else
	{
		g_thread->m_bChkFilterIP = FALSE;
	}
	
	nChkFilterIP *= -1;		

	UpdateData(TRUE);

	strcpy(g_thread->m_szFilterIP1, (LPCSTR)m_StrFilterIP1 );
	strcpy(g_thread->m_szFilterIP2, (LPCSTR)m_StrFilterIP2 );
}

void CIDMS_ServerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
}

void CIDMS_ServerDlg::OnChangeEditFilterIp1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	strcpy(g_thread->m_szFilterIP1, (LPCSTR)m_StrFilterIP1 );	
}

void CIDMS_ServerDlg::OnChangeEditFilterIp2() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	strcpy(g_thread->m_szFilterIP2, (LPCSTR)m_StrFilterIP2 );	
}

