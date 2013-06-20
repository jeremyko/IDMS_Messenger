#if !defined(AFX_COLORLISTCTRL_H__CF5D512D_DF2F_4C48_B2A4_F20938002B70__INCLUDED_)
#define AFX_COLORLISTCTRL_H__CF5D512D_DF2F_4C48_B2A4_F20938002B70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl window

//#define COLOR_TXT RGB(0, 0, 0)
//#define COLOR_BG  RGB(225,204,247) 


class CColorListCtrl : public CListCtrl
{
// Construction
public:
	CColorListCtrl();

// Attributes
public:
	COLORREF m_ColorBg;
	COLORREF m_ColorBgError;
	COLORREF m_ColorText;	
	COLORREF m_ColorTextError;	

// Operations
public:
	void SetBgColor(COLORREF Color)
	{
		m_ColorBg = Color;
	};

	void SetTextColor(COLORREF Color)
	{
		m_ColorText = Color;
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorListCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	
	afx_msg void OnCustomdrawList ( NMHDR* pNMHDR, LRESULT* pResult );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORLISTCTRL_H__CF5D512D_DF2F_4C48_B2A4_F20938002B70__INCLUDED_)
