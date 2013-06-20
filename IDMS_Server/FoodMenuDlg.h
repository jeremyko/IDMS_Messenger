#if !defined(AFX_FOODMENUDLG_H__76CE15D0_CD57_4B91_A6E2_625DB0402169__INCLUDED_)
#define AFX_FOODMENUDLG_H__76CE15D0_CD57_4B91_A6E2_625DB0402169__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FoodMenuDlg.h : header file
//

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

/////////////////////////////////////////////////////////////////////////////
// CFoodMenuDlg dialog

class CFoodMenuDlg : public CDialog
{
// Construction
public:
	CFoodMenuDlg(CWnd* pParent = NULL);   // standard constructor
	
	BOOL m_bChnged;
	CString m_strMenu;
	CString m_strMonDayOfWeek ;
	CString m_strFileNameAll;	
	void SetDateLabel() ;	
	void Load(BOOL bByOtherWnd = FALSE)  ;

// Dialog Data
	//{{AFX_DATA(CFoodMenuDlg)
	enum { IDD = IDD_DLG_FOOD };
	CDateTimeCtrl	m_FoodMenuDateCtrl;
	CString	m_strMenu11;
	CString	m_strMenu12;
	CString	m_strMenu21;
	CString	m_strMenu22;
	CString	m_strMenu31;
	CString	m_strMenu32;
	CString	m_strMenu41;
	CString	m_strMenu42;
	CString	m_strMenu51;
	CString	m_strMenu52;
	CString	m_strDate11;
	CString	m_strDate12;
	CString	m_strDate21;
	CString	m_strDate22;
	CString	m_strDate31;
	CString	m_strDate32;
	CString	m_strDate41;
	CString	m_strDate42;
	CString	m_strDate51;
	CString	m_strDate52;
	CString	m_Cal11;
	CString	m_Cal12;
	CString	m_Cal21;
	CString	m_Cal22;
	CString	m_Cal31;
	CString	m_Cal32;
	CString	m_Cal41;
	CString	m_Cal42;
	CString	m_Cal51;
	CString	m_Cal52;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFoodMenuDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFoodMenuDlg)
	afx_msg void OnBtnSave();
	afx_msg void OnBtnCancel();
	afx_msg void OnDatetimechangeDateselect(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();	
	afx_msg void OnBtnLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOODMENUDLG_H__76CE15D0_CD57_4B91_A6E2_625DB0402169__INCLUDED_)
