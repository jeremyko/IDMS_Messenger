#if !defined(AFX_CONVEDIT_H__DA08E8EF_38AA_411D_8158_98C1AEBDA5B4__INCLUDED_)
#define AFX_CONVEDIT_H__DA08E8EF_38AA_411D_8158_98C1AEBDA5B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConvEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConvEdit window

class CConvEdit : public CEdit
{
// Construction
public:
	CConvEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CConvEdit();
	void Enable(BOOL);
	void FileFindRecurse(LPCTSTR pstr);
	void SetHan(BOOL bFlag);
	CString m_strFileAll;
	int     m_nFileCnt; // 선택한 파일이 몇개인가 
	int m_nParentKind;
	// Generated message map functions
protected:
	//{{AFX_MSG(CConvEdit)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	CFont m_Font;
	CBrush m_Brush;
	BOOL m_bEnabled;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVEDIT_H__DA08E8EF_38AA_411D_8158_98C1AEBDA5B4__INCLUDED_)
