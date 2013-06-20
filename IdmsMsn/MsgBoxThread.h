#if !defined(AFX_MSGBOXTHREAD_H__40078C64_7091_4517_8772_32539442164C__INCLUDED_)
#define AFX_MSGBOXTHREAD_H__40078C64_7091_4517_8772_32539442164C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgBoxThread.h : header file
//

#include <istream>
using namespace std;


/////////////////////////////////////////////////////////////////////////////
// CMsgBoxThread thread

class CMsgBoxThread : public CWinThread
{
	DECLARE_DYNCREATE(CMsgBoxThread)
protected:
	

// Attributes
public:
	CMsgBoxThread(  );           
	CMsgBoxThread(CWnd* PWndParent, const char* szMsg , int nTimeOutSec = 0) ;
	virtual ~CMsgBoxThread();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBoxThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:	
	int     m_nTimeOutSec;
	CWnd* m_PWndParent;
	char m_szMsg[255];

	// Generated message map functions
	//{{AFX_MSG(CMsgBoxThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGBOXTHREAD_H__40078C64_7091_4517_8772_32539442164C__INCLUDED_)
