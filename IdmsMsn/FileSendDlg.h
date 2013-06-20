#if !defined(AFX_FILESENDDLG_H__7825544E_3288_418E_BA1C_989064B72834__INCLUDED_)
#define AFX_FILESENDDLG_H__7825544E_3288_418E_BA1C_989064B72834__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileSendDlg.h : header file
//

#include "IdmsMsnDlg.h"
#include "resource.h"
//#include "TextProgressCtrl.h"
#include "MyProgressCtrl.h"
#include "ColorListCtrl.h"
#include "..\\common\\ControlPos.h"	
#include "..\\common\\AllCommonDefines.h"	
/////////////////////////////////////////////////////////////////////////////
// CFileSendDlg dialog

class CFileSendDlg : public CDialog
{
// Construction
public:
	CFileSendDlg(CWnd* pParent = NULL);  
	CFileSendDlg(int nFileCnt,  CSockObjMapInfoFile * pSpckObjMap, char* szToId, CString& strFiles);   

	CString m_strFiles;
	CString m_strRootPath;
	CControlPos m_cControlPos;
	BOOL IsFileAlreadyOpen(char *filename) ;
	void SetResize();
// Dialog Data
	//{{AFX_DATA(CFileSendDlg)
	enum { IDD = IDD_FILESEND_DLG };
	CButton	m_BtnCancel;
	CColorListCtrl m_ListSend ;
	CMyProgressCtrl m_SentBytesProgress;
	CMyProgressCtrl m_SendFileProgress;
	CString			m_StrSendFileEdit;	
	CString	m_StrSentAmt;
	//}}AFX_DATA

	CStringArray	m_StrArySend ;    // send 할 파일을 저장한다.

protected:
	
	CIdmsMsnDlg* m_pDlgMain;

	char m_szCurFile [1024];
	//HANDLE m_hFile;
	int m_nFileCntToSend;
	int m_nTotalFileCnt;
	int m_nFileSent;
	float m_nPrePercent ;

	__int64 m_nSentBytes;
	__int64  m_nTotalBytes;

	char m_szIP [15+1];
	char m_szToID [20+1];
	char m_szSockObjID [5+1];
	
	CSockObjMapInfoFile * m_pSpckObjMap;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileSendDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileSendDlg)
	afx_msg void OnBtnFileSendCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnSendThisInformedFile (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFolderSelecting (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNoThanksFileTransfer (WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnSendBytes (WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnDisconnected	(WPARAM wParam, LPARAM lParam);	
		

	DECLARE_MESSAGE_MAP()
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILESENDDLG_H__7825544E_3288_418E_BA1C_989064B72834__INCLUDED_)
