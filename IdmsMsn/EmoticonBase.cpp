// EmoticonBase.cpp : implementation file
//

#include "stdafx.h"
#include "idmsmsn.h"
#include "EmoticonBase.h"
#include "..\\common\\AllCommonDefines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmoticonBase dialog


CEmoticonBase::CEmoticonBase(CWnd* pParent /*=NULL*/)
	: CDialog(CEmoticonBase::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmoticonBase)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEmoticonBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmoticonBase)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmoticonBase, CDialog)
	//{{AFX_MSG_MAP(CEmoticonBase)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmoticonBase message handlers

BOOL CEmoticonBase::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | /*CBRS_SIZE_DYNAMIC*/  CBRS_SIZE_FIXED  ) ||
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR_EMOTICON))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}	
	
	//이모티콘	
	m_imgListFaces.Create(20, 20, ILC_COLOR24|ILC_MASK, 0, 1);
	//CBitmap bmpFaces;
	m_bmpFaces.LoadBitmap(IDB_BITMAP_EMOTICON);
	m_imgListFaces.Add(&m_bmpFaces, RGB(255, 255, 255));	
	
	m_wndToolBar.GetToolBarCtrl().SetImageList(& m_imgListFaces );
		
		
	//for (int i = 0; i < 39; i++)
	for (int i = 7; i < 39; i += 8)
	{
		UINT nStyle = m_wndToolBar.GetButtonStyle(i);
		nStyle |= TBBS_WRAPPED;
		m_wndToolBar.SetButtonStyle( i, nStyle );
	}	
	
	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,   0, reposQuery, rcClientNow);
	
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,	rcClientNow.top - rcClientStart.top);

	CRect  rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);

	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	CRect rcWindow;
	GetWindowRect(rcWindow);

	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();

	MoveWindow(rcWindow, FALSE);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmoticonBase::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_bmpFaces.DeleteObject();

	CWnd *pParent = GetParent();
	if(pParent)
	{
		if(pParent->m_hWnd)
			pParent->PostMessage(WM_FOCUS_INPUT, (WPARAM)this, (LPARAM)NULL); 
	}
	
	CDialog::OnClose();
}
