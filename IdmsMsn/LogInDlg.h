#if !defined(AFX_LOGINDLG_H__EFE6E819_1EDF_4F4B_A4FA_C1291E366141__INCLUDED_)
#define AFX_LOGINDLG_H__EFE6E819_1EDF_4F4B_A4FA_C1291E366141__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogInDlg.h : header file
//
#include "Resource.h"
#include "IDMSBitmapDialog.h"
#include "ColorStatic.h"
/////////////////////////////////////////////////////////////////////////////
// CLogInDlg dialog

class CLogInDlg : public CIDMSBitmapDialog
//class CLogInDlg : public CDialog
{
// Construction
public:
	CLogInDlg(CWnd* pParent = NULL);   // standard constructor

	CString m_LoginID;
	CString m_strKTGN;
	CString m_LoginName;
	CString m_LoginTeamCd;
	CString m_LoginTeamName;
	CString m_TopTeamCode;
	CString m_TopTeamName;
	CString m_Grade;
	int     m_nCompensated;

// Dialog Data
	//{{AFX_DATA(CLogInDlg)
	enum { IDD = IDD_DIALOG_LOGIN };
	CEdit	m_EditID;
	CStatic	m_StaMsg;
	CButton	m_BtnLogIn;
	CEdit	m_EditPassWd;
	CColorStatic	m_GreetStatic;
	CString	m_strID;
	CString	m_strPwd;
	CString	m_strGreeting;
	CString	m_strLoginMSg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogInDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//void GetLoginQuery(const CString& strUserId, CString & strQuery);
	void GetDecryptPw(const CString& strId, const CString& strPw, CString& strDecrypt);
	//void GetQueryJobWrite(const CString& strLoginId, CString& strOut);	
	//void GetQueryInsertLogonTime(const CString& strLoginId, const CString& strDate, CString& strQuery);
	//void GetQueryUpdateLogonTime(const CString& strLoginId, const CString& strDate, CString& strQuery);
	CString GetGhostToday();
	//void GetQuerySelectDailyjobMst( const CString& strLoginId, const CString& strDate, CString&strQuery);
	//void GetQueryUpdateIp(const CString& strLoginId, const CString& strIp, CString& strOut);
	
	//void GetQueryInsertLogoffTime(const CString& strLoginId, const CString& strDate, CString& strQuery);
	//void GetQueryUpdateLogoffTime(const CString& strLoginId, const CString& strDate, CString& strQuery);
	void GetLocalIP(CString& strIP);

	// Generated message map functions
	//{{AFX_MSG(CLogInDlg)	
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnRegUser();
	afx_msg void OnBtnLoginCancel();
	afx_msg void OnBtnLoginRegist();	
	afx_msg void OnBtnChgPwd();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnNewLogin();
	afx_msg void OnBtnClose();
	//}}AFX_MSG
	afx_msg LRESULT OnLogInRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitWork(WPARAM wParam, LPARAM lParam);
	
	//afx_msg LRESULT OnSaveLastIPRslt(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnInsertLogOnTmRslt(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnJobGnRslt(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnDailyJobMstRslt(WPARAM wParam, LPARAM lParam);	
	//afx_msg LRESULT OnAlreadyLogIn(WPARAM wParam, LPARAM lParam);	
	//afx_msg LRESULT OnLogInAllowed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSQLRsltErr (WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:	
	BOOL m_Rslt;
	int m_nTryCount;
	CString m_strLoginID;	
	CString m_strStatus;
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINDLG_H__EFE6E819_1EDF_4F4B_A4FA_C1291E366141__INCLUDED_)
