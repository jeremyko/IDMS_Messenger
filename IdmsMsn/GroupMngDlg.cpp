// GroupMngDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "MsgBoxThread.h"
#include "ChatSession.h"
#include "GroupMngDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupMngDlg dialog


CGroupMngDlg::CGroupMngDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGroupMngDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGroupMngDlg)
	m_StrGrpName = _T("");
	m_StrRole = _T("");
	//}}AFX_DATA_INIT
	memset(m_szTitle , 0x00, sizeof(m_szTitle));
	memset(m_szGroupName , 0x00, sizeof(m_szGroupName));
}


void CGroupMngDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupMngDlg)
	DDX_Control(pDX, IDC_EDIT_GRP_NAME, m_EditGrpName);
	DDX_Control(pDX, IDC_COMBO_CHG_GRP, m_ComBoChgGrpSelect);
	DDX_Text(pDX, IDC_EDIT_GRP_NAME, m_StrGrpName);
	DDV_MaxChars(pDX, m_StrGrpName, 255);
	DDX_Text(pDX, IDC_STATIC_ROLE, m_StrRole);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroupMngDlg, CDialog)
	//{{AFX_MSG_MAP(CGroupMngDlg)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupMngDlg message handlers

void CGroupMngDlg::OnBtnOk() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_nRoleFlag == C_ROLE_CHG_COMP_GRP)
	{
		if(m_ComBoChgGrpSelect.GetCurSel() == -1)
		{			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"그룹을 선택하세요.");
			
			pMsgThread->CreateThread();

			return;
		}

		CString strTmpGrp;
		m_ComBoChgGrpSelect.GetLBText(m_ComBoChgGrpSelect.GetCurSel(),strTmpGrp ) ;		
		
		if(strTmpGrp.Find("|") != -1 )
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this," | 은 사용할수 없습니다 ^^;");
			pMsgThread->CreateThread();
			return;
		}

		strncpy(m_szGroupName, (LPCSTR)strTmpGrp, sizeof(m_szGroupName));
		TRACE("m_szGroupName [%s]\n",m_szGroupName);
		CDialog::OnOK();
	}
	else
	{
		memset(m_szGroupName, 0x00, sizeof(m_szGroupName));
		CString strTmp;
		GetDlgItemText(IDC_EDIT_GRP_NAME, strTmp);
		if(strTmp.GetLength() > 0)
		{
			if(strTmp.Find("|") != -1 )
			{
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this," | 은 사용할수 없습니다 ^^;");
				pMsgThread->CreateThread();
				return;
			}

			strncpy(m_szGroupName, (LPCSTR)strTmp, sizeof(m_szGroupName));
			TRACE("m_szGroupName [%s]\n",m_szGroupName);
			CDialog::OnOK();
		}
		else
		{							
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"그룹 이름을 입력하세요.");
			
			pMsgThread->CreateThread();
			m_EditGrpName.SetFocus();
		}
	}
}

void CGroupMngDlg::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	m_StrGrpName = _T("");
	strncpy(m_szGroupName, "", sizeof(m_szGroupName));
	CDialog::OnCancel();
}

void CGroupMngDlg::setDlgRole(int nRoleFlag)  
{
	m_nRoleFlag = nRoleFlag;
}

BOOL CGroupMngDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetWindowText(m_szTitle);
	
	if(m_nRoleFlag == C_ROLE_CHG_COMP_GRP)
	{
		// 대화상대 그룹 변경
		m_ComBoChgGrpSelect.ShowWindow(SW_SHOW);
		m_EditGrpName.ShowWindow(SW_HIDE);		
		m_StrRole = "그룹선택";
		UpdateData(FALSE);

		m_ComBoChgGrpSelect.Clear();

		CStringArray* pStrAry = CChatSession::Instance().getGrpNames();
		
		for(int i=0; i < pStrAry->GetSize(); i++)
		{
			m_ComBoChgGrpSelect.AddString( pStrAry->ElementAt(i) );
		}
		m_ComBoChgGrpSelect.SetCurSel(0);
	}
	else
	{
		m_ComBoChgGrpSelect.ShowWindow(SW_HIDE);
		m_EditGrpName.ShowWindow(SW_SHOW );
		m_EditGrpName.SetFocus();
		m_StrRole = "그룹명";
		m_StrGrpName.Format("%s", m_szGroupName) ;
		UpdateData(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CGroupMngDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{
		if(::GetDlgCtrlID(pMsg->hwnd) == IDC_EDIT_GRP_NAME)
        {
            pMsg->wParam = VK_TAB; 
		}
		else
		{
			OnBtnOk() ;
			return TRUE;		
		}		
	}
	return CDialog::PreTranslateMessage(pMsg);
}

