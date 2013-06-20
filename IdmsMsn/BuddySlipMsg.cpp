// BuddySlipMsg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "BuddySlipMsg.h"
#include "ChatSession.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuddySlipMsg dialog


CBuddySlipMsg::CBuddySlipMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CBuddySlipMsg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBuddySlipMsg)
	m_strSlipMsg = _T("");
	m_staMsgTo = _T("");
	//}}AFX_DATA_INIT
}

CBuddySlipMsg::CBuddySlipMsg(const char* szID, const char* szName, CWnd* pParent /*=NULL*/ )
	: CDialog(CBuddySlipMsg::IDD, pParent)
{	
	m_strSlipMsg = _T("");
	m_staMsgTo = _T("");		

	memset(m_szToID, 0x00, sizeof(m_szToID));
	memset(m_szToName, 0x00, sizeof(m_szToName));

	strncpy(m_szToID , szID, sizeof(m_szToID) );
	strncpy(m_szToName , szName, sizeof(m_szToName) );	
}

void CBuddySlipMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuddySlipMsg)
	DDX_Control(pDX, IDC_EDIT_WRITE_MSG, m_EditSlipMsg);
	DDX_Text(pDX, IDC_EDIT_WRITE_MSG, m_strSlipMsg);
	DDV_MaxChars(pDX, m_strSlipMsg, 1024);
	DDX_Text(pDX, IDC_STATIC_MSGTO, m_staMsgTo);
	DDV_MaxChars(pDX, m_staMsgTo, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuddySlipMsg, CDialog)
	//{{AFX_MSG_MAP(CBuddySlipMsg)
	ON_BN_CLICKED(IDC_BTN_WRITE_MSG, OnBtnWriteMsg)
	ON_BN_CLICKED(IDC_BTN_WRITE_MSG_CNCL, OnBtnWriteMsgCncl)
	ON_EN_SETFOCUS(IDC_EDIT_WRITE_MSG, OnSetfocusEditWriteMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuddySlipMsg message handlers

void CBuddySlipMsg::OnBtnWriteMsg() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_strSlipMsg.Replace("'","''");
	CChatSession::Instance().send_WriteSlipMsg( m_szToID, (LPCSTR)m_strSlipMsg);

	CWnd *pParent = GetParent();
	if(pParent)
	{
		if(pParent->m_hWnd)
			pParent->PostMessage(WM_FOCUS_INPUT, (WPARAM)this, (LPARAM)NULL); 
	}
		
}

void CBuddySlipMsg::OnBtnWriteMsgCncl() 
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

BOOL CBuddySlipMsg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here

	char szToInfo [100 + 1];
	memset(szToInfo, 0x00, sizeof(szToInfo));

	sprintf(szToInfo, "쪽지 보낼 사람 : %s [%s] ", m_szToName, m_szToID);
	m_staMsgTo = szToInfo ;

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBuddySlipMsg::OnSetfocusEditWriteMsg() 
{
	// TODO: Add your control notification handler code here
	m_EditSlipMsg.SetHan(TRUE);
}
