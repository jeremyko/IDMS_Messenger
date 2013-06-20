// IDMS_ServerDlg.h : header file
//

#if !defined(AFX_IDMS_SERVERDLG_H__E33BA1E8_6167_4E6B_A3D6_9D2ECE4F3E26__INCLUDED_)
#define AFX_IDMS_SERVERDLG_H__E33BA1E8_6167_4E6B_A3D6_9D2ECE4F3E26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include "resource.h"
#include "ControlPos.h"	


/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerDlg dialog

class CIDMS_ServerDlg : public CDialog
{
// Construction
public:
	CIDMS_ServerDlg(CWnd* pParent = NULL);	// standard constructor
	CString m_strMenu;
	
// Dialog Data
	//{{AFX_DATA(CIDMS_ServerDlg)
	enum { IDD = IDD_IDMS_SERVER_DIALOG };
	CButton	m_BtnExit;
	CRichEditCtrl	m_LogWnd;
	UINT	m_nConnCnt;
	CString	m_StrFilterIP1;
	CString	m_StrFilterIP2;
	CString	m_StrServerRunTime;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDMS_ServerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL	
	
	BOOL	bServerRuning;
	int nChkLog, nChkFilterIP;

	static void CALLBACK NotifyProc(LPVOID lpParam,  char *msg, long* nCnt);

	CString m_strDBConn; 
	CString m_strUser;   
	CString m_strPassWd; 
	CString	m_strDailyJobTime, m_strDailyChkAliveTime;	
	void ShowOsWarning();	
	void SetResize() ;
		
	CControlPos m_cControlPos;	
// Implementation
protected:
	HICON m_hIcon;
	CMapStringToOb mStrFileTransferDlgMap; // File 전송용 다이얼로그 저장
	// Generated message map functions
	//{{AFX_MSG(CIDMS_ServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonExit();		
	afx_msg void OnClose();	
	afx_msg void OnBtnClearList();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnUpdateUserList();
	afx_msg void OnBtnFood();
	afx_msg void OnCheckLog();
	afx_msg void OnCheckFilterip();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEditFilterIp1();
	afx_msg void OnChangeEditFilterIp2();	
	//}}AFX_MSG

	afx_msg LRESULT OnConnectionCnt(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIMadeUserList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIMadeAllUserList(WPARAM wParam, LPARAM lParam);
			
	DECLARE_MESSAGE_MAP()	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDMS_SERVERDLG_H__E33BA1E8_6167_4E6B_A3D6_9D2ECE4F3E26__INCLUDED_)
