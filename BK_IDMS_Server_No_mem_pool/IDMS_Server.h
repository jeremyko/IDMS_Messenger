// IDMS_Server.h : main header file for the IDMS_SERVER application
//

#if !defined(AFX_IDMS_SERVER_H__12232367_FD10_4ABD_995C_F1733E7472FF__INCLUDED_)
#define AFX_IDMS_SERVER_H__12232367_FD10_4ABD_995C_F1733E7472FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerApp:
// See IDMS_Server.cpp for the implementation of this class
//

class CIDMS_ServerApp : public CWinApp
{
public:
	CIDMS_ServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDMS_ServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIDMS_ServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDMS_SERVER_H__12232367_FD10_4ABD_995C_F1733E7472FF__INCLUDED_)
