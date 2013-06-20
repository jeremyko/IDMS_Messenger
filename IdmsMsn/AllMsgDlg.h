#if !defined(AFX_ALLMSGDLG_H__4AB78AE3_1C88_4E33_B1FE_3ACE02E495CC__INCLUDED_)
#define AFX_ALLMSGDLG_H__4AB78AE3_1C88_4E33_B1FE_3ACE02E495CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AllMsgDlg.h : header file
//
//#include "ChatSession.h"
#include "..\\common\\ControlPos.h"	
#include "ConvEdit.h"
#include "CheckedTreeCtrl.h"
#include "ChatSession.h"
/////////////////////////////////////////////////////////////////////////////
// CAllMsgDlg dialog



class CAllMsgDlg : public CDialog
{
// Construction
public:
	CAllMsgDlg(CWnd* pParent = NULL);   // standard constructor
	CAllMsgDlg::CAllMsgDlg(CStringArray& AryIP, CStringArray& AryName);
	
	//CStringArray* pStringAryIP ; 
	//CStringArray* pStringAryName ; 
		
	//CImageList	m_NormalImageList; //IDB_BITMAP_ALLMSG_TREE
	//CImageList  m_StatusImageList;
	
	CString m_strAllUserInfo; 
	CStringArray m_StringAryID ; 
	CStringArray m_StringAryName ; 
	CString      m_UserIDSelectedOnLine, m_UserIDSelectedOffLine;
	int m_nCntSelIDOnLine, m_nCntSelIDOffLine ;
	BOOL	m_bCheckAll, m_bExpandAll;	
	BOOL	m_bTreeMade;
	COLORREF m_ColorBG;
	void FileDropped(const char *FileName, int nLen, int nFileCnt);

	BOOL RequestAllUserInfo();
	int	 SetTree();
	int	 SelRcvPerson();
	void ClearAllTreeData();
// Dialog Data
	//{{AFX_DATA(CAllMsgDlg)
	enum { IDD = IDD_DIALOG_ALLMSG };
	CButton	m_btnExpandAll;	
	CButton	m_btnResend;
	CCheckedTreeCtrl m_TreeAllMsg;
	CButton	m_btnUpdate;
	CButton	m_chkAll;
	CButton	m_BtnSendAllMsg;
	CConvEdit	m_CEditTitle;
	CListCtrl	m_CListCtrlAllMsgStatus;
	CConvEdit	m_CEditAllMsg;
	CString	m_StrAllMsg;
	BOOL	m_bChkResend;
	int		m_RadioAllMsgTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAllMsgDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImageList m_imageSmall;
	CControlPos m_cControlPos;	
	void SetResize ();
	// Generated message map functions
	//{{AFX_MSG(CAllMsgDlg)
	afx_msg void OnBtnAllmsgOk();	
	afx_msg void OnBtnSendallmsg();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetfocusEditAllmsg();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();	
	afx_msg void OnChkSelectAll();
	afx_msg void OnRadioSelid();
	afx_msg void OnRadioSelteam();
	afx_msg void OnButtonUpdate();
	afx_msg void OnClickTreeAllMsg(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonExpandall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnAllMsgAck(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllMsgStop(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllMsgResume(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnAllMsgIPData(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnAllUserIdTeam(WPARAM wParam, LPARAM lParam);		
	
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALLMSGDLG_H__4AB78AE3_1C88_4E33_B1FE_3ACE02E495CC__INCLUDED_)
