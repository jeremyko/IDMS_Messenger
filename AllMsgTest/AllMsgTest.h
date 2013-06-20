// AllMsgTest.h : main header file for the ALLMSGTEST application
//

#if !defined(AFX_ALLMSGTEST_H__D888CDB6_8194_4200_B82F_93C72549EF77__INCLUDED_)
#define AFX_ALLMSGTEST_H__D888CDB6_8194_4200_B82F_93C72549EF77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAllMsgTestApp:
// See AllMsgTest.cpp for the implementation of this class
//

class CAllMsgTestApp : public CWinApp
{
public:
	CAllMsgTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAllMsgTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAllMsgTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALLMSGTEST_H__D888CDB6_8194_4200_B82F_93C72549EF77__INCLUDED_)
