#if !defined(AFX_CALNUMBEREDIT_H__95FC9123_3D73_4895_B441_E20B6AC3D183__INCLUDED_)
#define AFX_CALNUMBEREDIT_H__95FC9123_3D73_4895_B441_E20B6AC3D183__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CalNumberEdit.h : header file
//

// CalNumberEdit Notify Message
#define CNEN_RETURN		WM_USER + 101

/////////////////////////////////////////////////////////////////////////////
// CCalNumberEdit window

class CCalNumberEdit : public CEdit
{
// Construction
public:
	CCalNumberEdit();

// Attributes
public:
	int		m_nMinValue;
	int		m_nMaxValue;
	int		m_nWhatDoing;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCalNumberEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCalNumberEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCalNumberEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALNUMBEREDIT_H__95FC9123_3D73_4895_B441_E20B6AC3D183__INCLUDED_)
