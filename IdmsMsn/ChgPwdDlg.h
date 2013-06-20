#if !defined(AFX_CHGPWDDLG_H__00BADEA1_EB13_4DF8_A09E_2CCE47FA3954__INCLUDED_)
#define AFX_CHGPWDDLG_H__00BADEA1_EB13_4DF8_A09E_2CCE47FA3954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChgPwdDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChgPwdDlg dialog

class CChgPwdDlg : public CDialog
{
// Construction
public:
	CChgPwdDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChgPwdDlg)
	enum { IDD = IDD_DLG_CHG_PWD };
	CString	m_strPwOld;
	CString	m_strPwNew;
	CString	m_strPwCfrm;
	CString	m_strId;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChgPwdDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL CheckUserPw(const CString& strSrcPw, int nID );
	void GetDecryptPw(const CString& strId, const CString& strPw, CString& strDecrypt);
	CString GetEncryptPw(const CString& strId, const CString& strPw);
	// Generated message map functions
	//{{AFX_MSG(CChgPwdDlg)
	afx_msg void OnBtnChangepwSave();
	afx_msg void OnBtnChangepwReset();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg LRESULT OnUserChkRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChgPassWdRslt(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHGPWDDLG_H__00BADEA1_EB13_4DF8_A09E_2CCE47FA3954__INCLUDED_)
