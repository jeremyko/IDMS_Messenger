#if !defined(AFX_CHATREPLYEDITDLG_H__F582DE65_86D2_49A9_A5EA_255B0C73195F__INCLUDED_)
#define AFX_CHATREPLYEDITDLG_H__F582DE65_86D2_49A9_A5EA_255B0C73195F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatReplyEditDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CChatReplyEditDlg dialog

class CChatReplyEditDlg : public CDialog
{
// Construction
public:
	CChatReplyEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatReplyEditDlg)
	enum { IDD = IDD_DLG_MODFY_REPLY };
	CString	m_Reply0;
	CString	m_Reply1;
	CString	m_Reply2;
	CString	m_Reply3;
	CString	m_Reply4;
	CString	m_Reply5;
	CString	m_Reply6;
	CString	m_Reply7;
	CString	m_Reply8;
	CString	m_Reply9;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatReplyEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChatReplyEditDlg)
	afx_msg void OnBtnReplyOk();
	afx_msg void OnBtnReplyCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void IniSaveAll() ;
	void ClearReplyIni() ;

	CWnd* m_pParentDlg;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATREPLYEDITDLG_H__F582DE65_86D2_49A9_A5EA_255B0C73195F__INCLUDED_)
