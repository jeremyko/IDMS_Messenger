#if !defined(AFX_AUTOAPPENDMSGDLG_H__B819E2EF_2493_495C_8100_9A9B842ACAB2__INCLUDED_)
#define AFX_AUTOAPPENDMSGDLG_H__B819E2EF_2493_495C_8100_9A9B842ACAB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoAppendMsgDlg.h : header file
//
#include "..\\common\\AllCommonDefines.h"
/////////////////////////////////////////////////////////////////////////////
// CAutoAppendMsgDlg dialog

class CAutoAppendMsgDlg : public CDialog
{
// Construction
public:
	CAutoAppendMsgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAutoAppendMsgDlg)
	enum { IDD = IDD_DLG_AUTO_APPEND_MSG };
	CString	m_strAutoAppendMsg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoAppendMsgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CAutoAppendMsgDlg)
	afx_msg void OnBtnAutoappendmsgOk();
	afx_msg void OnBtnAutoappendmsgCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOAPPENDMSGDLG_H__B819E2EF_2493_495C_8100_9A9B842ACAB2__INCLUDED_)
