#if !defined(AFX_MYPROGRESSCTRL_H__2213531E_6545_4653_93A9_5CDBF5623608__INCLUDED_)
#define AFX_MYPROGRESSCTRL_H__2213531E_6545_4653_93A9_5CDBF5623608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyProgressCtrl.h : header file
//


#include "MemDC.h"

/////////////////////////////////////////////////////////////////////////////
// CMyProgressCtrl window

class CMyProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CMyProgressCtrl();

	
// Attributes
private:
	int			m_nLower; 
	int			m_nUpper;
	int			m_nStep;
	int			m_nCurrentPosition;
	int			m_nFontSize;
	
	LOGFONT		m_logFont;

	COLORREF	m_clrStart;
	COLORREF	m_clrEnd;
	COLORREF	m_clrBkGround;
	COLORREF	m_clrStartText;
	COLORREF	m_clrEndText;

	BOOL		m_bShowPercent;
	BOOL		m_IsVertical;


// Overrides
public:
	void	 SetRange(int nLower, int nUpper);
	int		 SetPos(int nPos);
	int		 SetStep(int nStep);
	int		 StepIt(void);


// Operations
public:
	// Set Functions
	void	 SetBkColor(COLORREF color)		 {m_clrBkGround = color;}
	void	 SetStartColor(COLORREF color)	{m_clrStart = color;}
	void	 SetEndColor(COLORREF color)	{m_clrEnd = color;}

	// TRUE를 인자로 넘기면 수직 프로그래스바입니다.
	void	 SetOrient(BOOL IsVertical)		{m_IsVertical = IsVertical;}

	// Show the percent caption
	void	 ShowPercent(BOOL bShowPercent = TRUE)	{m_bShowPercent = bShowPercent;}
	
	// Get Functions
	COLORREF GetBkColor(void)		 {return m_clrBkGround;}
	COLORREF GetStartColor(void)	{return m_clrStart;}
	COLORREF GetEndColor(void)	{return m_clrEnd;}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyProgressCtrl)
	//}}AFX_VIRTUAL


// Implementation
public:
	CString m_strText;
	void SetText(CString str);
	virtual ~CMyProgressCtrl();

	// Generated message map functions
protected:
	void DrawGradient(CPaintDC *pDC, const RECT &rectClient, const int &nMaxWidth);	
	

	//{{AFX_MSG(CMyProgressCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYPROGRESSCTRL_H__2213531E_6545_4653_93A9_5CDBF5623608__INCLUDED_)
