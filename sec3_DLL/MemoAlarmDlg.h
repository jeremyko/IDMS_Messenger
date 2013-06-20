 #if !defined(AFX_MEMOALARMDLG_H__CBB708C2_095F_4C8F_B1B1_9CAD97942A3E__INCLUDED_)
#define AFX_MEMOALARMDLG_H__CBB708C2_095F_4C8F_B1B1_9CAD97942A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MemoAlarmDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMemoAlarmDlg dialog

class CMemoAlarmDlg : public CDialog
{
// Construction
public:
	CMemoAlarmDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMemoAlarmDlg)
	enum { IDD = IDD_MEMO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemoAlarmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMemoAlarmDlg)
	afx_msg void OnBtnClose();
	afx_msg void OnBtnCancel();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMOALARMDLG_H__CBB708C2_095F_4C8F_B1B1_9CAD97942A3E__INCLUDED_)
