#if !defined(AFX_REGNEWUSERDLG_H__8AC8479A_3AD5_4E8D_BD27_FC8AA4FCC88F__INCLUDED_)
#define AFX_REGNEWUSERDLG_H__8AC8479A_3AD5_4E8D_BD27_FC8AA4FCC88F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegNewUserDlg.h : header file
//
#include "IDMSComboBox.h"

/////////////////////////////////////////////////////////////////////////////
// CRegNewUserDlg dialog

const int SAVE_WORK = 0;
const int CHK_WORK = 1;

class CRegNewUserDlg : public CDialog
{
// Construction
public:
	CRegNewUserDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegNewUserDlg)
	enum { IDD = IDD_DIALOG_NEW_USER };
	CIDMSComboBox	m_cbGrade;
	CIDMSComboBox	m_cbTeam;
	CIDMSComboBox	m_cbCompany;	
	CEdit	m_CEditRegUserNick;		
	CString	m_strRegUserNick;	
	CString	m_strId;
	CString	m_strJuminNo;
	CString	m_strName;
	CString	m_strPw1;
	CString	m_strPw2;
	int		m_nDocYesNo;
	CString	m_strTelOfc1;
	CString	m_strTelOfc2;
	CString	m_strTelOfc3;
	CString	m_strTelIn;
	CString	m_strTelHp1;
	CString	m_strTelHp2;
	CString	m_strTelHp3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegNewUserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CString m_strStatus;

protected:
	CString	m_strIdValid;
	CString m_strIdOld;
	BOOL	m_bModify;
	int		m_nEndFlag;
	CString	m_strTelOfc;
	CString	m_strTelHp;
	CString	m_strJobWrite;
	CString m_strCompanyCode;
	CString m_strCompanyName;
	CString m_strTeamCode;
	CString	m_strTeamName;	
	
	CString GetQueryTeam(const CString& strCode);
	CString GetQueryClass();	
	CString GetQueryTopTeam(const CString& strTeamCd);
	void	SetMainFrameInfo();	
	CString GetQuerySearch(const CString& strId);	
	CString GetQueryUpdate(const CString& strId,const CString& strOldId);
	CString GetQueryInsert();
	BOOL CheckRegData();
	BOOL DoSave();
	BOOL InitInfo();	
	BOOL IsModify();
	
	CString GetEncryptPw(const CString& strId, const CString& strPw);
	void ReqDocListUpdSql(const CString& strNewId, const CString& strOldId, CString& strQuery);
	CString GetQueryCompany();
	CString ReqDtlInfoUpdSql_1(const CString& strNewId,const CString& strOldId);
	CString ReqDtlInfoUpdSql_2(const CString& strNewId,const CString& strOldId);
	CString ReqInfoUpdSql_1(const CString& strNewId,const CString& strOldId);
	CString ReqInfoUpdSql_2(const CString& strNewId,const CString& strOldId);
	CString ModuleInfoUpdSql_1(const CString& strNewId, const CString& strOldId);
	CString ModuleInfoUpdSql_2(const CString& strNewId, const CString& strOldId);
	CString ModuleInfoUpdSql_3(const CString& strNewId, const CString& strOldId);
	CString ModuleInfoUpdSql_4(const CString& strNewId, const CString& strOldId);
	CString ModuleInfoUpdSql_5(const CString& strNewId, const CString& strOldId);
	
	int     m_nUseFlag; 	
	CString m_strTeamGn , m_strGrade , m_strCompany;

	BOOL chkValidation();
	void CheckString(const CString& strSrc, int& nAlphaCap, int& nAlphaLow, int& nNumeric, int& nOther);
	BOOL CheckUserId(const CString& strSrcId, int nID/*=-1*/);
	BOOL CheckUserPw(const CString& strSrcPw, int nID/*=-1*/);
	// Generated message map functions
	//{{AFX_MSG(CRegNewUserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBnSave();
	virtual void OnCancel();
	afx_msg void OnBtnRegReset();
	afx_msg void OnBtnDupidCheck();
	afx_msg void OnSelchangeCbCompany();
	//}}AFX_MSG
	afx_msg LRESULT OnRegNewUserRslt(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnChkDupUserIDRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetUserInfoRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTeamInfoRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCompanyInfoRslt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClassInfoRslt(WPARAM wParam, LPARAM lParam);
	
	
	//afx_msg LRESULT OnRegNewUserRslt(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGNEWUSERDLG_H__8AC8479A_3AD5_4E8D_BD27_FC8AA4FCC88F__INCLUDED_)
