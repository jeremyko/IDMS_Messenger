// UpdateManager.h : main header file for the DOCMANAGERUPDATE application
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define _OPTION_NONE		0
#define _OPTION_HIDE		1 << 23			// ¼û±è¼Ó¼º

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerApp:
// See DocManagerUpdate.cpp for the implementation of this class
//

class CUpdateManagerApp : public CWinApp
{
public:
	CUpdateManagerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateManagerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CUpdateManagerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	DWORD GetOption(LPTSTR szCmdLine);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
