// MsgBoxThread.cpp : implementation file
//

#include "stdafx.h"
//#include "EasyP2P_Messenger.h"
#include "MsgDlg.h"
#include "MsgBoxThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxThread

IMPLEMENT_DYNCREATE(CMsgBoxThread, CWinThread)

CMsgBoxThread::CMsgBoxThread()
{
	memset(m_szMsg, 0x00, sizeof(m_szMsg));
}

CMsgBoxThread::CMsgBoxThread(CWnd* PWndParent, const char* szMsg , int nTimeOutSec) 
{
	memset(m_szMsg, 0x00, sizeof(m_szMsg));
	m_PWndParent = PWndParent;
	strncpy(m_szMsg, szMsg, sizeof(m_szMsg));
	m_nTimeOutSec = nTimeOutSec;	
}

CMsgBoxThread::~CMsgBoxThread()
{
	
}

BOOL CMsgBoxThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	
	CMsgDlg dlg(m_PWndParent, m_szMsg, m_nTimeOutSec);
	dlg.DoModal();
	
	PostThreadMessage(WM_QUIT, 0, 0);
			
	return TRUE;
}

int CMsgBoxThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CMsgBoxThread, CWinThread)
	//{{AFX_MSG_MAP(CMsgBoxThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxThread message handlers
