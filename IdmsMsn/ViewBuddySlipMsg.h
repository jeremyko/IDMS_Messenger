#if !defined(AFX_VIEWBUDDYSLIPMSG_H__4B37525E_6915_4A86_9245_468E5D4F2D19__INCLUDED_)
#define AFX_VIEWBUDDYSLIPMSG_H__4B37525E_6915_4A86_9245_468E5D4F2D19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewBuddySlipMsg.h : header file
//

#include "..\\common\\AllCommonDefines.h"
/////////////////////////////////////////////////////////////////////////////
// CViewBuddySlipMsg dialog

class CViewBuddySlipMsg : public CDialog
{
// Construction
public:
	CViewBuddySlipMsg(CWnd* pParent = NULL);   // standard constructor

	CViewBuddySlipMsg(S_SLIP_MSG *pData, CWnd* pParent = NULL);   

		
// Dialog Data
	//{{AFX_DATA(CViewBuddySlipMsg)
	enum { IDD = IDD_DLG_VIEW_SLIP_MDG };
	CString	m_stsFromInfo;
	CString	m_REditMsg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewBuddySlipMsg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	S_SLIP_MSG m_sSlipMsg;
	// Generated message map functions
	//{{AFX_MSG(CViewBuddySlipMsg)
	afx_msg void OnBtnSlipMsgClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWBUDDYSLIPMSG_H__4B37525E_6915_4A86_9245_468E5D4F2D19__INCLUDED_)
