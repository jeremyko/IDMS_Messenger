// ChatReplyEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "idmsmsn.h"
#include "ChatReplyEditDlg.h"
#include "INIManager.h"
#include "..\\common\\AllCommonDefines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatReplyEditDlg dialog


CChatReplyEditDlg::CChatReplyEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatReplyEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatReplyEditDlg)
	m_Reply0 = _T("");
	m_Reply1 = _T("");
	m_Reply2 = _T("");
	m_Reply3 = _T("");
	m_Reply4 = _T("");
	m_Reply5 = _T("");
	m_Reply6 = _T("");
	m_Reply7 = _T("");
	m_Reply8 = _T("");
	m_Reply9 = _T("");
	//}}AFX_DATA_INIT

	m_pParentDlg = pParent ;
}

void CChatReplyEditDlg::IniSaveAll() 
{
	UpdateData(TRUE);
	//ini ÀúÀå 
	ClearReplyIni();	
	
	CINIManager::Instance().SetValue(INI_REPLY_LIST0, (LPSTR) (LPCTSTR) m_Reply0 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST1, (LPSTR) (LPCTSTR) m_Reply1 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST2, (LPSTR) (LPCTSTR) m_Reply2 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST3, (LPSTR) (LPCTSTR) m_Reply3 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST4, (LPSTR) (LPCTSTR) m_Reply4 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST5, (LPSTR) (LPCTSTR) m_Reply5 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST6, (LPSTR) (LPCTSTR) m_Reply6 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST7, (LPSTR) (LPCTSTR) m_Reply7 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST8, (LPSTR) (LPCTSTR) m_Reply8 );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST9, (LPSTR) (LPCTSTR) m_Reply9 );	
	
	CINIManager::Instance().SaveINIFile();
}

void CChatReplyEditDlg::ClearReplyIni() 
{		
	CINIManager::Instance().SetValue(INI_REPLY_LIST0, "" );				
	CINIManager::Instance().SetValue(INI_REPLY_LIST1, "" );	
	CINIManager::Instance().SetValue(INI_REPLY_LIST2, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST3, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST4, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST5, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST6, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST7, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST8, "" );
	CINIManager::Instance().SetValue(INI_REPLY_LIST9, "" );	
}

void CChatReplyEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatReplyEditDlg)
	DDX_Text(pDX, IDC_EDIT_REPLY0, m_Reply0);
	DDV_MaxChars(pDX, m_Reply0, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY1, m_Reply1);
	DDV_MaxChars(pDX, m_Reply1, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY2, m_Reply2);
	DDV_MaxChars(pDX, m_Reply2, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY3, m_Reply3);
	DDV_MaxChars(pDX, m_Reply3, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY4, m_Reply4);
	DDV_MaxChars(pDX, m_Reply4, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY5, m_Reply5);
	DDV_MaxChars(pDX, m_Reply5, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY6, m_Reply6);
	DDV_MaxChars(pDX, m_Reply6, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY7, m_Reply7);
	DDV_MaxChars(pDX, m_Reply7, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY8, m_Reply8);
	DDV_MaxChars(pDX, m_Reply8, 100);
	DDX_Text(pDX, IDC_EDIT_REPLY9, m_Reply9);
	DDV_MaxChars(pDX, m_Reply9, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatReplyEditDlg, CDialog)
	//{{AFX_MSG_MAP(CChatReplyEditDlg)
	ON_BN_CLICKED(IDC_BTN_REPLY_OK, OnBtnReplyOk)
	ON_BN_CLICKED(IDC_BTN_REPLY_CANCEL, OnBtnReplyCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatReplyEditDlg message handlers

void CChatReplyEditDlg::OnBtnReplyOk() 
{
	// TODO: Add your control notification handler code here
	IniSaveAll() ;

	m_pParentDlg->PostMessage(WM_SETREPLY, 0,0);
	CDialog::OnOK();
}

void CChatReplyEditDlg::OnBtnReplyCancel() 
{
	// TODO: Add your control notification handler code here
	m_pParentDlg->PostMessage(WM_FOCUS_INPUT, 0,0);
	CDialog::OnCancel();
}

BOOL CChatReplyEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	
	CINIManager::Instance().GetValue(INI_REPLY_LIST0,  m_Reply0 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST1,  m_Reply1 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST2,  m_Reply2 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST3,  m_Reply3 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST4,  m_Reply4 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST5,  m_Reply5 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST6,  m_Reply6 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST7,  m_Reply7 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST8,  m_Reply8 );	
	CINIManager::Instance().GetValue(INI_REPLY_LIST9,  m_Reply9 );	
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
