// ITCompDlgMng.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "ITCompDlgMng.h"
#include "ChatSession.h"
#include "MsgBoxThread.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CITCompDlgMng dialog


CITCompDlgMng::CITCompDlgMng(CWnd* pParent /*=NULL*/)
	: CDialog(CITCompDlgMng::IDD, pParent)
{
	//{{AFX_DATA_INIT(CITCompDlgMng)
	m_strDesc = _T("");
	m_strUrl = _T("");
	//}}AFX_DATA_INIT

	m_strURL = "";
	m_strDESC = "";
	m_strDescOld = "";
	m_strUrlOld = "";
	//memset(m_szUrl, 0x00, sizeof(m_szUrl));
	//memset(m_szDesc, 0x00, sizeof(m_szDesc));
	//memset(m_szDescOld, 0x00, sizeof(m_szDescOld));	
	
	m_nRole = 0;
}


void CITCompDlgMng::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CITCompDlgMng)
	DDX_Control(pDX, IDC_BTN_ADD_MODIFY, m_BtnAddModify);
	DDX_Text(pDX, IDC_EDIT_DESC, m_strDesc);
	DDV_MaxChars(pDX, m_strDesc, 100);
	DDX_Text(pDX, IDC_EDIT_URL, m_strUrl);
	DDV_MaxChars(pDX, m_strUrl, 300);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CITCompDlgMng, CDialog)
	//{{AFX_MSG_MAP(CITCompDlgMng)	
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_ADD_MODIFY, OnBtnAddModify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CITCompDlgMng message handlers


void CITCompDlgMng::OnBtnClose() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//m_strURL, m_strDESC;
	GetDlgItemText(IDC_EDIT_URL, m_strURL);
	GetDlgItemText(IDC_EDIT_DESC, m_strDESC);

	m_strURL.TrimLeft();
	m_strURL.TrimRight();

	m_strDESC.TrimLeft();
	m_strDESC.TrimRight();

	if(m_nRole == C_ROLE_ADD_IT_COMP)
	{		
		//if(strncmp(m_szDescOld, m_szDesc, sizeof(m_szDescOld)) !=0)
		if(m_strDescOld != m_strDESC)
		{
			if(CChatSession::Instance().RequestAddCompIT( (LPCSTR)m_strDESC, (LPCSTR)m_strURL ) != TRUE)
			{
				TRACE("RequestAddCompIT 실패!\n");	
			}
		}
	}
	else if(m_nRole == C_ROLE_CHG_IT_COMP)
	{	
		//if( strlen(m_szDesc)!=0 && strlen(m_szUrl)!=0  )
		if( m_strDESC.GetLength()!=0 && m_strURL.GetLength() !=0  )
		{
			if	(
					//strncmp(m_szDescOld, m_szDesc, sizeof(m_szDescOld)) !=0 || 
					//strncmp(m_szUrlOld, m_szUrl, sizeof(m_szUrlOld)) !=0
					m_strDescOld != m_strDESC || m_strUrlOld != m_strURL					
				)
			{
				if(CChatSession::Instance().RequestChgCompIT( (LPCSTR)m_strDescOld, (LPCSTR)m_strDESC, (LPCSTR)m_strURL ) != TRUE)
				{
					TRACE("RequestChgCompIT 실패!\n");	
				}
			}

			//CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"내용을 입력하세요.");
			//pMsgThread->CreateThread();			
		}		
	}

	CDialog::OnOK();
}

BOOL CITCompDlgMng::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//C_ROLE_ADD_IT_COMP 
	//C_ROLE_CHG_IT_COMP

	if(m_nRole == C_ROLE_ADD_IT_COMP)
	{
		m_BtnAddModify.SetWindowText("추 가");
	}
	else if(m_nRole == C_ROLE_CHG_IT_COMP)
	{
		m_BtnAddModify.SetWindowText("변 경");

		m_strUrl=  m_strURL;
		m_strDesc = m_strDESC; 

		//strncpy(m_szUrlOld, m_szUrl, sizeof(m_szUrlOld));
		//strncpy(m_szDescOld, m_szDesc, sizeof(m_szDescOld));

		m_strUrlOld = m_strURL;
		m_strDescOld = m_strDESC;
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CITCompDlgMng::OnBtnAddModify() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
    //m_strURL, m_strDESC;
	GetDlgItemText(IDC_EDIT_URL, m_strURL);
	GetDlgItemText(IDC_EDIT_DESC, m_strDESC);

	m_strURL.TrimLeft();
	m_strURL.TrimRight();

	m_strDESC.TrimLeft();
	m_strDESC.TrimRight();

	//m_strDESC.Replace("'","''");
		
	if(m_nRole == C_ROLE_ADD_IT_COMP)
	{	
		//if( strlen(m_szDesc)==0 || strlen(m_szUrl)==0  )
		if( m_strDESC.GetLength() ==0 || m_strURL.GetLength() ==0  )
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"내용을 입력하세요.");
			pMsgThread->CreateThread();

			return;
		}

		// 중복 Check => 서버에서 오류로 전달된다.		
		if(CChatSession::Instance().RequestAddCompIT( (LPCSTR)m_strDESC, (LPCSTR)m_strURL ) != TRUE)
		{
			TRACE("OnGrpMngAddItcomp 실패!\n");
			return;
		}	

		//strncpy(m_szDescOld, m_szDesc, sizeof(m_szDescOld));
		m_strDescOld = m_strDESC ;
	}
	else if(m_nRole == C_ROLE_CHG_IT_COMP)
	{
		//if( strlen(m_szDesc)==0 || strlen(m_szUrl)==0  )
		if( m_strDESC.GetLength() ==0 || m_strURL.GetLength() ==0  )
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"내용을 입력하세요.");
			pMsgThread->CreateThread();

			return;
		}

		if	(
				//strncmp(m_szDescOld, m_szDesc, sizeof(m_szDescOld)) !=0 || 
				//strncmp(m_szUrlOld, m_szUrl, sizeof(m_szUrlOld)) !=0

				m_strDescOld != m_strDESC || m_strUrlOld != m_strURL
			)
		{
			if(CChatSession::Instance().RequestChgCompIT( (LPCSTR)m_strDescOld, (LPCSTR)m_strDESC, (LPCSTR)m_strURL ) != TRUE)
			{
				TRACE("RequestChgCompIT 실패!\n");	
			}
		}

		//strncpy(m_szUrlOld, m_szUrl, sizeof(m_szUrlOld));
		//strncpy(m_szDescOld, m_szDesc, sizeof(m_szDescOld));
		m_strUrlOld = m_strURL;
		m_strDescOld = m_strDESC;
	}
}
