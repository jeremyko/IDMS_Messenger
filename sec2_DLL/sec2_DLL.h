// sec2_DLL.h : main header file for the SEC2_DLL DLL
//

#if !defined(AFX_SEC2_DLL_H__FDF59ECD_B0EA_44BF_94BC_87AD52A6BD06__INCLUDED_)
#define AFX_SEC2_DLL_H__FDF59ECD_B0EA_44BF_94BC_87AD52A6BD06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSec2_DLLApp
// See sec2_DLL.cpp for the implementation of this class
//

class CSec2_DLLApp : public CWinApp
{
public:
	CSec2_DLLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSec2_DLLApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSec2_DLLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEC2_DLL_H__FDF59ECD_B0EA_44BF_94BC_87AD52A6BD06__INCLUDED_)
