#if !defined(AFX_CNEWLOGIN_H__E84CF5F1_4D7B_4AED_9291_1ABC379ADA47__INCLUDED_)
#define AFX_CNEWLOGIN_H__E84CF5F1_4D7B_4AED_9291_1ABC379ADA47__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CNewLogIn.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCNewLogIn dialog

class CCNewLogIn : public CDialog
{
// Construction
public:
	CCNewLogIn(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCNewLogIn)
	enum { IDD = IDD_DLG_NEW_LOGIN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCNewLogIn)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCNewLogIn)
	afx_msg void OnInfoUpdate();
	afx_msg void OnBnFind();
	afx_msg void OnBnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CNEWLOGIN_H__E84CF5F1_4D7B_4AED_9291_1ABC379ADA47__INCLUDED_)
