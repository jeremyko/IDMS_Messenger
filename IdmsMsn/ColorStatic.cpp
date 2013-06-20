// ColorStatic.cpp : implementation file
//

#include "stdafx.h"

#include "ColorStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//CColorStatic!! mainly used in the conversation dialog

CColorStatic::CColorStatic()
{
	Brush.CreateSolidBrush(RGB(0, 0, 0));
	m_Back = RGB(0, 0, 0);
	m_Fore = RGB(0, 0, 0);
}

CColorStatic::~CColorStatic()
{
}


BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
	//{{AFX_MSG_MAP(CColorStatic)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorStatic message handlers

void CColorStatic::SetColors(COLORREF crFore, COLORREF crBack)
{
	m_Back = crBack;
	m_Fore = crFore;
	Brush.DeleteObject();
	Brush.CreateSolidBrush(crBack);
	Invalidate();
}

HBRUSH CColorStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	pDC->SetBkColor(m_Back);
	pDC->SetTextColor(m_Fore);
	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return (HBRUSH) Brush;
}
