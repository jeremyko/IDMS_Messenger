#if !defined(AFX_MSGDLG_H__BBC105E4_F0D6_40C1_95C9_A54E1E1515A4__INCLUDED_)
#define AFX_MSGDLG_H__BBC105E4_F0D6_40C1_95C9_A54E1E1515A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg dialog
#include "Resource.h"
class CMsgDlg : public CDialog
{
// Construction
public:
	CMsgDlg(CWnd* pParent = NULL);   // standard constructor
	CMsgDlg(CWnd* pParent , char* szMsg, int nTimeOutSec);
// Dialog Data
	//{{AFX_DATA(CMsgDlg)
	enum { IDD = IDD_DIALOG_MSG };
	CButton	m_BtnOk;
	CStatic	m_staticMsg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
//	CRect rect;
//	void SetMsg(const char* szMsg);
//	void SetTimeOutSec(const int nTimeOutSec);
		
// Implementation
protected:	
		
	char m_szMsg[255];
	int m_nTimeOutSec;
	// Generated message map functions
	//{{AFX_MSG(CMsgDlg)
	afx_msg void OnButtonConfirm();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGDLG_H__BBC105E4_F0D6_40C1_95C9_A54E1E1515A4__INCLUDED_)
