#if !defined(AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_)
#define AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorStatic window

class CColorStatic : public CStatic
{
// Construction
public:
	CColorStatic();
	CColorStatic(COLORREF crFore, COLORREF crBack);

// Attributes
public:
	void SetColors(COLORREF, COLORREF);
	COLORREF m_Fore;
	COLORREF m_Back;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorStatic)
	//}}AFX_VIRTUAL
	CBrush Brush;

// Implementation
public:
	virtual ~CColorStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_)
