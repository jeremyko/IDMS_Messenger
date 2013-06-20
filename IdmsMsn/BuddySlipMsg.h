#if !defined(AFX_BUDDYSLIPMSG_H__1D9AEFC4_1A58_4E0D_B265_9EDA66C06F21__INCLUDED_)
#define AFX_BUDDYSLIPMSG_H__1D9AEFC4_1A58_4E0D_B265_9EDA66C06F21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BuddySlipMsg.h : header file
//
#include "ConvEdit.h"
/////////////////////////////////////////////////////////////////////////////
// CBuddySlipMsg dialog

class CBuddySlipMsg : public CDialog
{
// Construction
public:
	CBuddySlipMsg(CWnd* pParent = NULL);   // standard constructor

	CBuddySlipMsg(const char* szID, const char* szName, CWnd* pParent = NULL); 

// Dialog Data
	//{{AFX_DATA(CBuddySlipMsg)
	enum { IDD = IDD_DLG_WRITE_MSG };
	CConvEdit m_EditSlipMsg;
	CString	m_strSlipMsg;
	CString	m_staMsgTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuddySlipMsg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	char m_szToID[20 + 1];
	char m_szToName[50 + 1];
	// Generated message map functions
	//{{AFX_MSG(CBuddySlipMsg)
	afx_msg void OnBtnWriteMsg();
	afx_msg void OnBtnWriteMsgCncl();
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusEditWriteMsg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUDDYSLIPMSG_H__1D9AEFC4_1A58_4E0D_B265_9EDA66C06F21__INCLUDED_)
