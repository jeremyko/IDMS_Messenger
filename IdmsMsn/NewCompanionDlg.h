#if !defined(AFX_NEWCOMPANIONCLG_H__4978085F_47BE_46EE_8FD5_2FA37342FBFF__INCLUDED_)
#define AFX_NEWCOMPANIONCLG_H__4978085F_47BE_46EE_8FD5_2FA37342FBFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewCompanionDlg.h : header file
//

#include "ConvEdit.h"
/////////////////////////////////////////////////////////////////////////////
// CNewCompanionDlg dialog

class CNewCompanionDlg : public CDialog
{
// Construction
public:
	
	CNewCompanionDlg(CWnd* pParent = NULL);   // standard constructor
	CNewCompanionDlg(CWnd* pParent, const char* szSelGrpName)	;
	
	void SetParentHandle(HWND pHwnd)
	{
		m_pHwnd = pHwnd;
	};
	void ClearAllTreeData() ;
	void	RefreshList();
	
public:	
	
// Dialog Data
	//{{AFX_DATA(CNewCompanionDlg)
	enum { IDD = IDD_DIALOG_NEW_COMPANION };
	CButton	m_btnUpdate;
	CListCtrl	m_ListLogInID;
	CButton	m_btnSearch;	
	CConvEdit	m_EditUserName;
	CEdit	m_EditUserID;
	CTreeCtrl	m_TreeListAll;
	CEdit	m_CEditAddID;
	CComboBox	m_ComBoGrpName;
	CString	m_strCompID;
	int		m_nRadioSel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewCompanionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HWND m_pHwnd;
	CString m_strExID;
	CString  m_strID;
	char m_szGrpName	[C_GRP_NAME_LEN];
	BOOL RequestAllUserInfo();
	CString m_strAllUserInfo;
	int	SetTree();
	BOOL m_bTreeMade;
	
	// Generated message map functions
	//{{AFX_MSG(CNewCompanionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnClose();
	afx_msg void OnBtnCancel();
	afx_msg void OnBtnAddClose();
	afx_msg void OnBtnAdd();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnSearch();
	afx_msg void OnRadioListall();
	afx_msg void OnRadioSearch();
	afx_msg void OnRadioSelid();
	afx_msg void OnClickListLoginId(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusEbUsername();
	afx_msg void OnButtonUpdate();
	//}}AFX_MSG
	afx_msg LRESULT OnSearchID (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllUserIdTeam(WPARAM wParam, LPARAM lParam);	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWCOMPANIONCLG_H__4978085F_47BE_46EE_8FD5_2FA37342FBFF__INCLUDED_)
