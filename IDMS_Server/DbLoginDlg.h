#if !defined(AFX_DBLOGINDLG_H__B4979D4E_C8CD_46D9_90A3_BFE8DBE5292B__INCLUDED_)
#define AFX_DBLOGINDLG_H__B4979D4E_C8CD_46D9_90A3_BFE8DBE5292B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DbLoginDlg.h : header file
//

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

/////////////////////////////////////////////////////////////////////////////
// CDbLoginDlg dialog

class CDbLoginDlg : public CDialog
{
// Construction
public:
	CDbLoginDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDbLoginDlg)
	enum { IDD = IDD_DLG_DB_LOGIN };
	CString	m_strConn;
	CString	m_strPasswd;
	CString	m_strUser;
	//}}AFX_DATA
public:
	BOOL m_bDbRun;		
	static void CALLBACK SQLNotifyProc( S_CALLBACKINFO* pSCallBackInfo ) ;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbLoginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	

	// Generated message map functions
	//{{AFX_MSG(CDbLoginDlg)
	afx_msg void OnBtnDbLogin();
	afx_msg void OnBtnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg LRESULT OnConnectOK(WPARAM wParam, LPARAM lParam);	
		
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBLOGINDLG_H__B4979D4E_C8CD_46D9_90A3_BFE8DBE5292B__INCLUDED_)
