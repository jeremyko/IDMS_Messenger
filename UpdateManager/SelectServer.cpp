// SelectServer.cpp : implementation file
//


//-----------------------------------------------------------------------//
// Create_Date : 2006.08.29 BY LSH   (KEY WORD : ADD_INI_FILE)
//  1.접속 서버 선택 
//-----------------------------------------------------------------------// 


#include "stdafx.h"
#include "updatemanager.h"
#include "SelectServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectServer dialog


CSelectServer::CSelectServer(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectServer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectServer)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectServer, CDialog)
	//{{AFX_MSG_MAP(CSelectServer)
	ON_BN_CLICKED(IDC_IDMS_UP_CUSTOM, OnIdmsUpCustom)
	ON_BN_CLICKED(IDC_IDMS_UP_RATE, OnIdmsUpRate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectServer message handlers

BOOL CSelectServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	
	CheckDlgButton(IDC_IDMS_UP_CUSTOM,	   BST_CHECKED);
	m_strServer=IDMS_CUSTOM;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectServer::OnIdmsUpCustom() 
{
	m_strServer=IDMS_CUSTOM;	
}

void CSelectServer::OnIdmsUpRate() 
{
	m_strServer=IDMS_BILL;		
}

void CSelectServer::OnOK() 
{	
	CDialog::OnOK();
}
