// Sec3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "sec3_DLL.h"
#include "Sec3Dlg.h"

//#include<vfw.h>
//#pragma comment (lib,"vfw32.lib") 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSec3Dlg dialog


CSec3Dlg::CSec3Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSec3Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSec3Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSec3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSec3Dlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSec3Dlg, CDialog)
	//{{AFX_MSG_MAP(CSec3Dlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_DELETE_ITEM, OnFileDeleteItem)
	ON_COMMAND(ID_FILE_EDIT_ITEM, OnFileEditItem)
	ON_COMMAND(ID_FILE_NEW_ITEM, OnFileNewItem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSec3Dlg message handlers

void CSec3Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if( m_wndCalendar.GetSafeHwnd() )
	{
		CRect rc;
		GetClientRect(&rc);
		m_wndCalendar.MoveWindow(&rc);
		m_wndCalendar.SetFocus();
	}

	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();
}

void CSec3Dlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	//m_cControlPos.AddControl(&m_staNick	, CP_RESIZE_HORIZONTAL );		
	//m_cControlPos.AddControl(&m_btnMyStatus	, CP_MOVE_HORIZONTAL );		
	//m_cControlPos.AddControl(&m_statusBar	, CP_RESIZE_HORIZONTAL	| CP_MOVE_VERTICAL	);	
	//m_cControlPos.AddControl(IDC_BTN_TEST	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );	

	m_cControlPos.AddControl(ID_CLOSE    	, CP_MOVE_HORIZONTAL | CP_MOVE_VERTICAL	 );	
	m_cControlPos.AddControl(IDC_BTN_TEST	, CP_MOVE_HORIZONTAL | CP_MOVE_VERTICAL	 );		
}

BOOL CSec3Dlg::OnInitDialog() 
{
	int i = 0;
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	SetResize();
	
	/* VFW TEST
	//m_CapWnd = capCreateCaptureWindow("capture",WS_CHILD | WS_VISIBLE,0,0,176,144,m_hWnd, NULL );
	m_CapWnd = capCreateCaptureWindow("capture",WS_CHILD | WS_VISIBLE,0,0,320,240,m_hWnd, NULL );

	if(NULL == m_CapWnd)
	{
		AfxMessageBox("Capture Window를 생성할수 없습니다");
		return FALSE;
	}

	if ( capDriverConnect(m_CapWnd, 0) == FALSE) //1 대 
	{
		AfxMessageBox("장치 연결 실패");
		return FALSE;
	}

	capPreviewRate(m_CapWnd, 1) ;
	capPreview (m_CapWnd, TRUE) ;
    */

	m_wndCalendar.Create(CALENDARCTRL_CLASSNAME, _T(""), WS_CHILD|WS_VISIBLE, CRect(0, 0, 100, 100), this, 0);

	//m_ilCalItem.Create(IDB_CALITEM, 16, 0, RGB(0, 128, 128));
	m_ilCalItem.Create(16, 16, ILC_COLOR8|ILC_MASK, 3, 1);
	for( i = 0; i < 2; i++)
	{
		m_ilCalItem.Add(AfxGetApp()->LoadIcon(IDI_ITEM01 + i));
	}

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_MONTHVIEW);
	m_wndCalendar.SetSmallIcon(hIcon);
	m_wndCalendar.SetImageList(&m_ilCalItem);

	m_DataManager.SetOwnerWnd(this);
	m_DataManager.SetImageList(&m_ilCalItem);
	m_DataManager.SetCalendarCtrl(&m_wndCalendar);

	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
	// "D:\DATA\DevelopSource\CD8NotBK\newMessenger\BinRelease\IdmsMsn.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\newMessenger\ 까지 구한다.	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	TRACE("%s\n" , szDirTmp);		
	strFileName.Format("%s\\memocal.xml", szDirTmp);
	TRACE("%s\n" , strFileName);		
	m_DataManager.OpenDocument(_T(strFileName));
	m_wndCalendar.SetDataInterface((ICalData *)&m_DataManager);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSec3Dlg::OnBtnTest() 
{
	// TODO: Add your control notification handler code here
	
}

void CSec3Dlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	/*
	capCaptureStop(m_CapWnd);
	capCaptureAbort(m_CapWnd);
	
	Sleep(500); //간혹 오류나는 경우 딜레이를 좀 준다..!! 
	capDriverDisconnect(m_CapWnd);
	*/

	CDialog::OnClose();
}

void CSec3Dlg::OnFileDeleteItem() 
{
	// TODO: Add your command handler code here
	m_DataManager.OnMenuDeleteItem();
}

void CSec3Dlg::OnFileEditItem() 
{
	// TODO: Add your command handler code here
	m_DataManager.OnMenuEditItem();	
}

void CSec3Dlg::OnFileNewItem() 
{
	// TODO: Add your command handler code here
	m_DataManager.OnMenuNewItem();
}

BOOL CSec3Dlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(pMsg->message == WM_KEYDOWN )
	{
		m_wndCalendar.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam );

		if(pMsg->wParam == VK_RETURN )
		{		
			return TRUE;
		}
	}
		
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}
