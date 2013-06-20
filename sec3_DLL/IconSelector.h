#if !defined(AFX_ICONSELECTOR_H__B28C8103_71B3_4816_860E_98CA92768007__INCLUDED_)
#define AFX_ICONSELECTOR_H__B28C8103_71B3_4816_860E_98CA92768007__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconSelector.h : header file
//

#define ICONSELECTOR_CLASSNAME _T("ICONSELECTOR")
#define UM_ICONCANCELED			WM_USER + 100
#define UM_ICONSELECTED			WM_USER + 101


/////////////////////////////////////////////////////////////////////////////
// CIconSelector window

class CIconSelector : public CWnd
{
// Construction
public:
	CIconSelector();

// Attributes
public:

// Operations
public:
	void	SetImageList(CImageList* pImageList);
	void	SetIconSize(CSize szIcon);
	void	SetIconSize(int cx, int cy);
	bool	SetRowCol(int nRow, int nCol);
	void	Show(CPoint ptTopLeft);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconSelector)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIconSelector();

	// Generated message map functions
protected:
	CSize		m_szIcon;
	CSize		m_szCell;
	int			m_nRowCount;
	int			m_nColCount;
	int			m_nCurRow;
	int			m_nCurCol;
	int			m_nMouseRow;
	int			m_nMouseCol;
	CImageList* m_pImageList;

	BOOL		RegisterWindowClass();
	void		DrawCell(CDC* pDC, int nRow, int nCol, CRect rcClip);
	void		DrawCellLine(CDC* pDC);
	void		GetCellRect(int nRow, int nCol, LPRECT rcClip);
	void		GetMouseRowCol(int* npRow, int* npCol, CPoint point);

	//{{AFX_MSG(CIconSelector)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONSELECTOR_H__B28C8103_71B3_4816_860E_98CA92768007__INCLUDED_)
