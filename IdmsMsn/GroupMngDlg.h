#if !defined(AFX_GROUPMNGDLG_H__2FCA6266_A435_4809_BA7A_84AA633541E6__INCLUDED_)
#define AFX_GROUPMNGDLG_H__2FCA6266_A435_4809_BA7A_84AA633541E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupMngDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupMngDlg dialog
#include "..\\common\\AllCommonDefines.h"

class CGroupMngDlg : public CDialog
{
// Construction
public:
	CGroupMngDlg(CWnd* pParent = NULL);   // standard constructor
	char m_szTitle[30];
	char m_szGroupName [C_GRP_NAME_LEN + 1];
	void setDlgRole(int nRoleFlag)  ;
	
// Dialog Data
	//{{AFX_DATA(CGroupMngDlg)
	enum { IDD = IDD_DLG_GRP_MNG };
	CEdit	m_EditGrpName;
	CComboBox	m_ComBoChgGrpSelect;
	CString	m_StrGrpName;
	CString	m_StrRole;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupMngDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nRoleFlag;
	// Generated message map functions
	//{{AFX_MSG(CGroupMngDlg)
	afx_msg void OnBtnOk();
	afx_msg void OnBtnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPMNGDLG_H__2FCA6266_A435_4809_BA7A_84AA633541E6__INCLUDED_)
