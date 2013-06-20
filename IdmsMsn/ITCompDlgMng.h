#if !defined(AFX_ITCOMPDLGMNG_H__1BB973A6_F08D_45AF_B807_E354732DA68E__INCLUDED_)
#define AFX_ITCOMPDLGMNG_H__1BB973A6_F08D_45AF_B807_E354732DA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ITCompDlgMng.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CITCompDlgMng dialog

class CITCompDlgMng : public CDialog
{
// Construction
public:
	CITCompDlgMng(CWnd* pParent = NULL);   // standard constructor

	char m_szUrl[300+1];
	char m_szDesc[100+1];
	CString m_strDescOld, m_strUrlOld;
	CString m_strURL, m_strDESC;
	int  m_nRole; 
	
	/*
	C_ROLE_CHG_IT_COMP	=	0;	
	C_ROLE_ADD_IT_COMP	=	2;*	
	*/

// Dialog Data
	//{{AFX_DATA(CITCompDlgMng)
	enum { IDD = IDD_DLG_IT_COMP_MNG };
	CButton	m_BtnAddModify;
	CString	m_strDesc;
	CString	m_strUrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CITCompDlgMng)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//char m_szDescOld[100+1];
	//char m_szUrlOld[300+1];
	// Generated message map functions
	//{{AFX_MSG(CITCompDlgMng)	
	afx_msg void OnBtnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddModify();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITCOMPDLGMNG_H__1BB973A6_F08D_45AF_B807_E354732DA68E__INCLUDED_)
