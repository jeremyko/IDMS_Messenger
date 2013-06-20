#if !defined(AFX_BUTTONICONSELECT_H__1F7A8EE9_DC33_4179_8655_26A590C14A99__INCLUDED_)
#define AFX_BUTTONICONSELECT_H__1F7A8EE9_DC33_4179_8655_26A590C14A99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonIconSelect.h : header file
//

#include "btnst.h"
#include "IconSelector.h"

/////////////////////////////////////////////////////////////////////////////
// CButtonIconSelect window

class CButtonIconSelect : public CButtonST
{
// Construction
public:
	CButtonIconSelect();

// Attributes
public:

// Operations
public:
	void	SetImageList(CImageList* pImageList);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonIconSelect)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonIconSelect();

	// Generated message map functions
protected:
	CImageList*		m_pImageList;
	CIconSelector	m_wndIconSelector;
	//{{AFX_MSG(CButtonIconSelect)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONICONSELECT_H__1F7A8EE9_DC33_4179_8655_26A590C14A99__INCLUDED_)
