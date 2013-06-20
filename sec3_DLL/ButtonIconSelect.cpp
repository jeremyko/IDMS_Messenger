// ButtonIconSelect.cpp : implementation file
//

#include "stdafx.h"
#include "ButtonIconSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonIconSelect

CButtonIconSelect::CButtonIconSelect()
{
	m_pImageList = NULL;
}

CButtonIconSelect::~CButtonIconSelect()
{
}


BEGIN_MESSAGE_MAP(CButtonIconSelect, CButtonST)
	//{{AFX_MSG_MAP(CButtonIconSelect)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonIconSelect message handlers


void CButtonIconSelect::SetImageList(CImageList* pImageList)
{
	m_pImageList = pImageList;

	m_wndIconSelector.SetImageList(m_pImageList);
	m_wndIconSelector.SetIconSize(16, 16);
	m_wndIconSelector.SetRowCol(7, 7);
}

void CButtonIconSelect::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if( !(m_wndIconSelector.m_hWnd) )
		m_wndIconSelector.Create(this);

	SetState(TRUE);

	CRect rc;
	GetWindowRect(rc);
	rc.OffsetRect(0, rc.Height());
	m_wndIconSelector.Show(rc.TopLeft());

	m_wndIconSelector.SetFocus();
	//CButtonST::OnLButtonDown(nFlags, point);
}

