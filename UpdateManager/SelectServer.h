#if !defined(AFX_SELECTSERVER_H__F1E31E8F_C3C6_4EC9_82CE_271099139102__INCLUDED_)
#define AFX_SELECTSERVER_H__F1E31E8F_C3C6_4EC9_82CE_271099139102__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectServer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectServer dialog

#include "resource.h"

class CSelectServer : public CDialog
{
// Construction
public:
	CSelectServer(CWnd* pParent = NULL);   // standard constructor
	CString m_strServer;
	

// Dialog Data
	//{{AFX_DATA(CSelectServer)
	enum { IDD = IDD_IDMS_SET_SERVER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnIdmsUpCustom();
	afx_msg void OnIdmsUpRate();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTSERVER_H__F1E31E8F_C3C6_4EC9_82CE_271099139102__INCLUDED_)
