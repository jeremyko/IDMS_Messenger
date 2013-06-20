// IdmsMsn.h : main header file for the IdmsMsn application
//

#if !defined(AFX_IdmsMsn_H__FFD3D941_90D6_48C6_92F2_9AB47C8E1EAE__INCLUDED_)
#define AFX_IdmsMsn_H__FFD3D941_90D6_48C6_92F2_9AB47C8E1EAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnApp:
// See IdmsMsn.cpp for the implementation of this class
//

class CIdmsMsnApp : public CWinApp
{
public:
	CIdmsMsnApp();
	CString m_strLogInID;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIdmsMsnApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIdmsMsnApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IdmsMsn_H__FFD3D941_90D6_48C6_92F2_9AB47C8E1EAE__INCLUDED_)
