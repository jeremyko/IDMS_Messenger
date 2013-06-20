// dormant.h : main header file for the DORMANT DLL
//

#if !defined(AFX_DORMANT_H__81C30F35_BC8E_48CD_8D71_432B21591635__INCLUDED_)
#define AFX_DORMANT_H__81C30F35_BC8E_48CD_8D71_432B21591635__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDormantApp
// See dormant.cpp for the implementation of this class
//

class CDormantApp : public CWinApp
{
public:
	CDormantApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDormantApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDormantApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DORMANT_H__81C30F35_BC8E_48CD_8D71_432B21591635__INCLUDED_)
