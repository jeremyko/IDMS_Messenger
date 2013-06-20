// CNewLogIn.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "CNewLogIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCNewLogIn dialog


CCNewLogIn::CCNewLogIn(CWnd* pParent /*=NULL*/)
	: CDialog(CCNewLogIn::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCNewLogIn)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCNewLogIn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCNewLogIn)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCNewLogIn, CDialog)
	//{{AFX_MSG_MAP(CCNewLogIn)
	ON_BN_CLICKED(ID_INFO_UPDATE, OnInfoUpdate)
	ON_BN_CLICKED(ID_BN_FIND, OnBnFind)
	ON_BN_CLICKED(ID_BN_CANCEL, OnBnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCNewLogIn message handlers

void CCNewLogIn::OnInfoUpdate() 
{
	// TODO: Add your control notification handler code here
	
}

void CCNewLogIn::OnBnFind() 
{
	// TODO: Add your control notification handler code here
	
}

void CCNewLogIn::OnBnCancel() 
{
	// TODO: Add your control notification handler code here
	
}
