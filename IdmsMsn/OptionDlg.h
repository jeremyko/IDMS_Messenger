#if !defined(AFX_OPTIONDLG_H__36BF2576_ED55_4EF1_AF9E_8D9388D5B931__INCLUDED_)
#define AFX_OPTIONDLG_H__36BF2576_ED55_4EF1_AF9E_8D9388D5B931__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionDlg.h : header file
//

#include "..\\common\\AllCommonDefines.h"
#include "MyButton.h"
#include "ColourPicker.h"
#include "EmoticonRichEditCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// COptionDlg dialog

class COptionDlg : public CDialog
{
// Construction
public:
	COptionDlg(CWnd* pParent = NULL);   // standard constructor
	COptionDlg(const char* szNick, BOOL bFromMainDlg, CWnd* pParent = NULL); 
	
// Dialog Data
	//{{AFX_DATA(COptionDlg)
	enum { IDD = IDD_DLG_OPTION };
	CButton	m_btnLogFolderPath;
	CEmoticonRichEditCtrl	m_REditExample;
	CColourPicker	m_ColourPickerBG;	
	CColourPicker	m_ColourPickerBuddy;
	CColourPicker	m_ColourPickerMe;
	CButton	m_ChkUseSound;
	CButton	m_ChkUseDownFolder;
	CButton	m_ChkBtnAlwaysOnTop;
	CButton	m_ChkBtnAutoLogIn;
	CButton	m_btnSelectSound;
	CButton	m_btnDownFolderPath;
	CEdit	m_EditNick;
	CString	m_strCtrlOptionNick;
	int		m_nAwayTimeComBo;
	BOOL	m_ChkUseAutoLogin;
	BOOL	m_bAlwaysOnTop;
	BOOL	m_bUseDownFolder;
	CString	m_strDownFolder;
	CString m_strTempSoundPath;
	CString m_strSoundPath ;
	BOOL	m_bUseSound;
	CString	m_strLogFolder;
	BOOL	m_bUseLogFolder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FileDialogOpen(CString &result) ;
	int ApplySoundConfig();

	// Generated message map functions
	//{{AFX_MSG(COptionDlg)
	afx_msg void OnBtnOptOk();
	afx_msg void OnBtnOptCancle();
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnDownFolder();
	afx_msg void OnChkDownFolder();
	afx_msg void OnBtnSelectSound();
	afx_msg void OnBtnStopSound();
	afx_msg void OnBtnPlaySound();
	afx_msg void OnChkUseSound();
	afx_msg void OnButtonFont();
	afx_msg void OnColourpickerMe();
	afx_msg void OnBtnLogFolder();
	afx_msg void OnChkLogFolder();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
public:
	BOOL    m_bNickChg ;
	int     m_nAwayTimeSecOld;
	char    m_szNick[C_NICK_NAME_LEN + 1];	
	int     m_nFlipFlopDownFolder;
	int     m_nFlipFlopLogFolder;
	int m_nFlipFlopSound;
	LOGFONT m_lf;	
	DWORD m_dwMask, m_dwEffect ;		
	CString m_fn;
	CFont m_ExampleFont;
	BOOL m_bFromMainDlg;
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONDLG_H__36BF2576_ED55_4EF1_AF9E_8D9388D5B931__INCLUDED_)
