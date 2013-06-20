// AutoAppendMsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "idmsmsn.h"
#include "AutoAppendMsgDlg.h"
#include "INIManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoAppendMsgDlg dialog


CAutoAppendMsgDlg::CAutoAppendMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoAppendMsgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutoAppendMsgDlg)
	m_strAutoAppendMsg = _T("");
	//}}AFX_DATA_INIT
}


void CAutoAppendMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoAppendMsgDlg)
	DDX_Text(pDX, IDC_EDIT_AUTO_APPEND_MSG, m_strAutoAppendMsg);
	DDV_MaxChars(pDX, m_strAutoAppendMsg, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoAppendMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CAutoAppendMsgDlg)
	ON_BN_CLICKED(IDC_BTN_AUTOAPPENDMSG_OK, OnBtnAutoappendmsgOk)
	ON_BN_CLICKED(IDC_BTN_AUTOAPPENDMSG_CANCEL, OnBtnAutoappendmsgCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoAppendMsgDlg message handlers

void CAutoAppendMsgDlg::OnBtnAutoappendmsgOk() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CINIManager::Instance().SetValue(INI_AUTO_APPEND_MSG,  "" );
	CINIManager::Instance().SetValue(INI_AUTO_APPEND_MSG,  (LPSTR) (LPCTSTR) m_strAutoAppendMsg );	
	CINIManager::Instance().SaveINIFile();
	
	CWnd *pParent = GetParent();
	if(pParent)
	{
		if(pParent->m_hWnd)
			pParent->PostMessage(WM_FOCUS_INPUT, (WPARAM)this, (LPARAM)NULL); 
	}
	
	CDialog::OnOK();
}

void CAutoAppendMsgDlg::OnBtnAutoappendmsgCancel() 
{
	// TODO: Add your control notification handler code here
	CWnd *pParent = GetParent();
	if(pParent)
	{
		if(pParent->m_hWnd)
			pParent->PostMessage(WM_FOCUS_INPUT, (WPARAM)this, (LPARAM)NULL); 
	}

	CDialog::OnCancel();
}

BOOL CAutoAppendMsgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();		

	// TODO: Add extra initialization here	
	CINIManager::Instance().GetValue(INI_AUTO_APPEND_MSG,  m_strAutoAppendMsg );	
	
	UpdateData(FALSE);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
