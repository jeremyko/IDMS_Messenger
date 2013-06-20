// sec3_DLL.h : main header file for the SEC3_DLL DLL
//

#if !defined(AFX_SEC3_DLL_H__FAB4DC6F_88F7_497E_A938_2C33FE8059F7__INCLUDED_)
#define AFX_SEC3_DLL_H__FAB4DC6F_88F7_497E_A938_2C33FE8059F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSec3_DLLApp
// See sec3_DLL.cpp for the implementation of this class
//

class CSec3_DLLApp : public CWinApp
{
public:
	CSec3_DLLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSec3_DLLApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSec3_DLLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEC3_DLL_H__FAB4DC6F_88F7_497E_A938_2C33FE8059F7__INCLUDED_)
