// UpdateManagerDlg.h : header file
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxinet.h>

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerDlg dialog

class CUpdateManagerDlg : public CDialog
{
	CInternetSession*	m_pSession;
	CFtpConnection*		m_pFtpCon;

	CString				m_strSID;
	CString				m_strUser;
	CString				m_strPW;

	CString				m_strUpPath;				// IDMS_BILL	
	
	BOOL		OpenFtpSession		(CString& strUrl, CString& strId, CString& strPw);
	void		CloseFtpSession		(BOOL bExitSession = FALSE);
	BOOL		FindUpdateFile		(CString& strUpdateDate);
	BOOL		FileUpdate			(CString& strInstallDir);

	/////////////////////////////////////////////////////////////////////////////
	//GET_TNSFILE
	BOOL Get10GOracleHome(HKEY hKey, CString strSection, CStringArray& oraclePathList, BOOL& isOra10GExist);
	BOOL GetRegKeyValue(HKEY hKey, LPCSTR strSection, LPCTSTR szKey, CString &strValue);
	CString GetDefaultOracleHomePath();
	CString GetDefaultTNSNameFilePath();
	/////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////
	//IDMS_BILL
	CString	m_strSetServer;
	CString GetServerFlag();
	CString GetCurPath();
	int		SetConnectInfo(CString& strSvrInfo);
	/////////////////////////////////////////////////////////////////////////////

	
// Construction
public:
	void SetOption(DWORD dwOption);
	CUpdateManagerDlg(CWnd* pParent = NULL);	// standard constructor
	~CUpdateManagerDlg();

// Dialog Data
	//{{AFX_DATA(CUpdateManagerDlg)
	enum { IDD = IDD_UPDATE_MANAGER };
	CAnimateCtrl	m_Animate;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Trace(CString strData);
	CString GetServerInfoCcrcss10();
	CString GetServerInfoCt40(CString strKeyWord);					// IDMS_BILL
	CString GetServerInfoBill(CString strKeyWord);					// IDMS_BILL
	CString GetServerInfoOpera(CString strKeyWord);					// META_DATA

	//CString GetTnsNamesPath();
	BOOL CheckServer();
	BOOL DoUpdate();
	BOOL GetServerInfo();
	BOOL CheckInstDir(CString strDir);
	BOOL CheckUpdateFile(CString strPath);

	DWORD	m_dwOption;
	CMapStringToString m_mapFileList;
	CString	m_strFtpIp;
	CString	m_strFtpId;
	CString m_strFtpPw;
	CString m_strFtpDir;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUpdateManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
