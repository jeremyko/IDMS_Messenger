// AllMsgTestDlg.h : header file
//

#if !defined(AFX_ALLMSGTESTDLG_H__960A155E_DDDA_484D_B9A0_7980476CC195__INCLUDED_)
#define AFX_ALLMSGTESTDLG_H__960A155E_DDDA_484D_B9A0_7980476CC195__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAllMsgTestDlg dialog

class CAllMsgTestDlg : public CDialog
{
// Construction
public:
	CAllMsgTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAllMsgTestDlg)
	enum { IDD = IDD_ALLMSGTEST_DIALOG };
	CString	m_strAllMsg;
	CString	m_strToId1;
	CString	m_strToId2;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAllMsgTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	SOCKET		m_hSocket;	

	// Generated message map functions
	//{{AFX_MSG(CAllMsgTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnSend();
	afx_msg void OnBtnConnect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALLMSGTESTDLG_H__960A155E_DDDA_484D_B9A0_7980476CC195__INCLUDED_)
