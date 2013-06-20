#if !defined(AFX_FILERECVDLG_H__F4C7B5CE_4151_4EC1_9445_8EF13895B924__INCLUDED_)
#define AFX_FILERECVDLG_H__F4C7B5CE_4151_4EC1_9445_8EF13895B924__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileRecvDlg.h : header file
//
#include "Resource.h"
//#include "TextProgressCtrl.h"
#include "MyProgressCtrl.h"
#include "ColorListCtrl.h"
#include "..\\common\\ControlPos.h"	
#include "..\\common\\AllCommonDefines.h"	
/////////////////////////////////////////////////////////////////////////////
// CFileRecvDlg dialog

class CFileRecvDlg : public CDialog
{
// Construction
public:
	CFileRecvDlg( CWnd* pParent = NULL );   // standard constructor
	CFileRecvDlg( ST_RCV_SOMEONE_WANT_SEND_FILE_S* pFiles ,SOCKET_OBJ_FILE* pSock) ;
	~CFileRecvDlg();
// Dialog Data
	//{{AFX_DATA(CFileRecvDlg)
	enum { IDD = IDD_FILERECV_DLG };
	CButton	m_BtnOpenFolder;
	CButton	m_BtnOK;
	CButton	m_BtnCancel;
	CColorListCtrl		m_ListCtrlFileRecv;
	CMyProgressCtrl	m_RecvBytesProgress;
	CMyProgressCtrl	m_RecvFileProgress;
	CListBox			m_RecvList;
	CEdit		m_EditConFirm;
	CString		m_strConFirmEdit;
	BOOL		m_bIsCanceled;
	CString	m_StrRcvAmt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileRecvDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	protected:
		ST_RCV_SOMEONE_WANT_SEND_FILE_S* m_pFiles ;
		SOCKET_OBJ_FILE * m_pSockObj;
		HANDLE m_hFile ;
		//CSockObjMapInfo* m_pSpckObjMap;
		CStringArray	m_StrAry ;    // 전달받을 파일을 저장한다.
		int m_nRecvedFileCnt ;
		int m_nTotalFileCnt ;

		__int64 m_nRecvBytes;
		__int64 m_nTotalBytes;
		float m_nPrePercent ;
		
		char m_szSavePath[1024];
		char m_szRootPath[360+1];	
		char m_szSockObjID [5+1];
		
		void FolderDialogOpen(char* szResult) ;
		void FileCreate();
		void SetResize();

		CControlPos m_cControlPos;
// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CFileRecvDlg)
	afx_msg void OnBtnFileConfirm();
	afx_msg void OnBtnFileNotConfirm();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBtnOpenFileFolder();
	//}}AFX_MSG
	afx_msg LRESULT OnFileRecved (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCancelFileTransfer (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBytesRecv (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisconnected	(WPARAM wParam, LPARAM lParam);
	
	
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILERECVDLG_H__F4C7B5CE_4151_4EC1_9445_8EF13895B924__INCLUDED_)
