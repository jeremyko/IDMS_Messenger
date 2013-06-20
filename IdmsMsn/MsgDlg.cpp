// MsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "MsgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg dialog


CMsgDlg::CMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgDlg::IDD, pParent)
{
	memset(m_szMsg, 0x00, sizeof(m_szMsg));
	//{{AFX_DATA_INIT(CMsgDlg)
	//}}AFX_DATA_INIT
}

CMsgDlg::CMsgDlg(CWnd* pParent , char* szMsg, int nTimeOutSec)
: CDialog(CMsgDlg::IDD, pParent)	
{
	memset(m_szMsg, 0x00, sizeof(m_szMsg));
	strncpy(m_szMsg, szMsg, sizeof(m_szMsg));
	m_nTimeOutSec = nTimeOutSec;
}

void CMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgDlg)
	DDX_Control(pDX, IDC_BUTTON_CONFIRM, m_BtnOk);
	DDX_Control(pDX, IDC_STATIC_MSG, m_staticMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgDlg)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM, OnButtonConfirm)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg message handlers

void CMsgDlg::OnButtonConfirm() 
{
	// TODO: Add your control notification handler code here
	//KillTimer(1);
	//KillTimer(2);

	CDialog::OnOK();
}

/*
void CMsgDlg::SetMsg(const char* szMsg)
{	
	strncpy(m_szMsg , szMsg, sizeof(m_szMsg));	
}

void CMsgDlg::SetTimeOutSec(const int nTimeOutSec)
{
	m_nTimeOutSec = nTimeOutSec; 	
}
*/

void CMsgDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
		/*
		CRect rW;
		CRect rT;
		m_pdlgMsg->GetWindowRect(rW);
		
		m_pdlgMsg->CenterWindow(pParent);
		m_pdlgMsg->GetClientRect(rT);
		rT.top    += 10;
		rT.left   += 90;
		rT.right  -= 20;
		rT.bottom -= 10;
		
		m_pdlgMsg->GetDlgItem(ID_TXT_MSG)->MoveWindow(rT);
		m_pdlgMsg->m_strMsg = m_strMsg;
		
		m_pdlgMsg->UpdateData(FALSE);
		m_pdlgMsg->UpdateWindow();	
		 */

		//CenterWindow();

		m_staticMsg.SetWindowText(m_szMsg);
		UpdateData(FALSE);		

		KillTimer(1);
	}
	else if(nIDEvent == 2)
	{
		KillTimer(2);
		CDialog::OnOK();
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CMsgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	
	//MoveWindow(&rect);

	// TODO: Add extra initialization here	
	SetTimer(1, 10, NULL);
	
	if(m_nTimeOutSec > 0)
	{
		m_BtnOk.ShowWindow(SW_HIDE);
		SetTimer(2, m_nTimeOutSec*1000, NULL);
	}	

	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
