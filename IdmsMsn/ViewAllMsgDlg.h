#if !defined(AFX_VIEWALLMSGDLG_H__987C435F_4A11_4891_9607_B94AA6B2B14C__INCLUDED_)
#define AFX_VIEWALLMSGDLG_H__987C435F_4A11_4891_9607_B94AA6B2B14C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewAllMsgDlg.h : header file
//
#include "..\\common\\AllCommonDefines.h"
#include "..\\common\\ControlPos.h"	
#include "EmoticonRichEditCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CViewAllMsgDlg dialog

class CViewAllMsgDlg : public CDialog
{
// Construction
public:
	CViewAllMsgDlg(CWnd* pParent = NULL);   // standard constructor
	CViewAllMsgDlg( CBuddyInfo * pCompanion);
	~CViewAllMsgDlg();
	
	BOOL LoadAllMsgHistData();
	CBuddyInfo * m_pCompanion;
	void FileDialogOpen(CString &result);
	
// Dialog Data
	//{{AFX_DATA(CViewAllMsgDlg)
	enum { IDD = IDD_DIALOG_VIEW_ALLMSG };
	CEmoticonRichEditCtrl	m_RichEditViewAllMsg;
	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewAllMsgDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//COLORREF m_ColorALLMSG;
	CString m_StrRcvDate;
	CString m_strRcvDateDetail;
	CString m_StrSender;
	CString m_strSenderID ; 
	CControlPos m_cControlPos;	
	
	//CFont m_Font;
	void SetResize ();
	// Generated message map functions
	//{{AFX_MSG(CViewAllMsgDlg)
	afx_msg void OnBtnViewAllmsgOk();
	virtual BOOL OnInitDialog();	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAllmsgClose();
	afx_msg void OnAllmsgFilesave();
	afx_msg void OnAllmsgHist();
	afx_msg void OnClose();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnAllMsg(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWALLMSGDLG_H__987C435F_4A11_4891_9607_B94AA6B2B14C__INCLUDED_)
