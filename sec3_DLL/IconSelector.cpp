// IconSelector.cpp : implementation file
//

#include "stdafx.h"
#include "IconSelector.h"
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ICONSEL_COLOR_BACK		RGB(255, 255, 255)
#define ICONSEL_COLOR_LINE		GetSysColor(COLOR_BTNSHADOW)
#define ICONSEL_COLOR_SELECT	GetSysColor(COLOR_HIGHLIGHT)

#define ICONSEL_MARGIN_X		4
#define ICONSEL_MARGIN_Y		4

/////////////////////////////////////////////////////////////////////////////
// CIconSelector

CIconSelector::CIconSelector()
{
	RegisterWindowClass();

	m_pImageList = NULL;
	m_szIcon.cx = 16;
	m_szIcon.cy = 16;
	m_nRowCount = 0;
	m_nColCount = 0;
	m_nCurRow = 0;
	m_nCurCol = 0;
	m_nMouseRow = -1;
	m_nMouseCol = -1;
}

CIconSelector::~CIconSelector()
{
}

BOOL CIconSelector::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo(hInst, ICONSELECTOR_CLASSNAME, &wndcls)) )
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc		= ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = ICONSELECTOR_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(CIconSelector, CWnd)
	//{{AFX_MSG_MAP(CIconSelector)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIconSelector message handlers

BOOL CIconSelector::Create(CWnd* pParentWnd) 
{
	return CWnd::CreateEx(WS_EX_TOOLWINDOW,
		                  ICONSELECTOR_CLASSNAME,
				          _T(""),
						  WS_POPUP|WS_CHILD,
						  0, 0, 100, 100,
						  pParentWnd->m_hWnd,
						  (HMENU)0
						 );
}

int CIconSelector::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CIconSelector::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(&dc);
	CRect rc, rcClip;
	int i, j;

	GetClientRect(rc);
	memDC.FillSolidRect(rc, ICONSEL_COLOR_BACK);
	
	for(i = 0; i < (int)m_nRowCount; i++)
	{
		for(j = 0; j < (int)m_nColCount; j++)
		{
			GetCellRect(i, j, rcClip);
			DrawCell((CDC *)&memDC, i, j, rcClip);
		}
	}

	DrawCellLine((CDC *)&memDC);
}

void CIconSelector::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int nRow, nCol;

	GetMouseRowCol(&nRow, &nCol, point);
	if( m_nCurRow != nRow || m_nCurCol != nCol )
	{
		m_nCurRow = nRow;
		m_nCurCol = nCol;
		Invalidate(FALSE);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CIconSelector::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	int nImage = m_nCurRow * m_nColCount + m_nCurCol;
	GetParent()->SendMessage(UM_ICONSELECTED, (WPARAM)nImage);
	ShowWindow(SW_HIDE);
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CIconSelector::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	switch( nChar )
	{
	case VK_ESCAPE:
		GetParent()->SendMessage(UM_ICONCANCELED);
		ShowWindow(SW_HIDE);
		break;
	}
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CIconSelector::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	GetParent()->SendMessage(UM_ICONCANCELED);
	ShowWindow(SW_HIDE);
}

BOOL CIconSelector::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}


void CIconSelector::SetIconSize(CSize szIcon)
{
	m_szIcon = szIcon;
}

void CIconSelector::SetIconSize(int cx, int cy)
{
	m_szIcon.cx = cx;
	m_szIcon.cy = cy;
}

bool CIconSelector::SetRowCol(int nRow, int nCol)
{
	if( m_pImageList == NULL ) return false;
	if( nRow * nCol < m_pImageList->GetImageCount() ) return false;

	m_nRowCount = nRow;
	m_nColCount = nCol;

	return true;
}

void CIconSelector::SetImageList(CImageList* pImageList)
{
	m_pImageList = pImageList;
}

void CIconSelector::GetCellRect(int nRow, int nCol, LPRECT rcClip)
{
	rcClip->left = m_szCell.cx * nCol + nCol + 1;
	rcClip->top = m_szCell.cy * nRow + nRow + 1;
	rcClip->right = rcClip->left + m_szCell.cx;
	rcClip->bottom = rcClip->top + m_szCell.cy;
}

void CIconSelector::GetMouseRowCol(int* npRow, int* npCol, CPoint point)
{
	*npRow = (point.y - 1) / (m_szCell.cy + 1);
	*npCol = (point.x - 1) / (m_szCell.cx + 1);
}

void CIconSelector::DrawCell(CDC* pDC, int nRow, int nCol, CRect rcClip)
{
	int nSavedDC;
	int nImage = nRow * m_nColCount + nCol;

	if( nImage > m_pImageList->GetImageCount() ) return;

	nSavedDC = pDC->SaveDC();

	if( m_nCurRow == nRow && m_nCurCol == nCol )
	{
		rcClip.DeflateRect(1, 1);
		CPen pen(PS_SOLID, 1, ICONSEL_COLOR_SELECT);
		pDC->SelectObject(&pen);
		pDC->SelectStockObject(NULL_BRUSH);
		pDC->Rectangle(rcClip);
		rcClip.InflateRect(1, 1);
	}
	rcClip.DeflateRect(ICONSEL_MARGIN_X, ICONSEL_MARGIN_Y);
	m_pImageList->Draw(pDC, nImage, rcClip.TopLeft(), ILD_TRANSPARENT);

	pDC->RestoreDC(nSavedDC);
}

void CIconSelector::DrawCellLine(CDC* pDC)
{
	int nSavedDC;
	int i;

	nSavedDC = pDC->SaveDC();

	CPen penInside(PS_SOLID, 1, ICONSEL_COLOR_LINE);
	pDC->SelectObject(&penInside);

	// 세로선
	for(i = 1; i < m_nRowCount; i++)
	{
		pDC->MoveTo(m_szCell.cx * i + i, 0);
		pDC->LineTo(m_szCell.cx * i + i, m_szCell.cy * m_nRowCount + m_nRowCount + 1);
	}

	// 가로선
	for(i = 1; i < m_nRowCount; i++)
	{
		pDC->MoveTo(0, m_szCell.cy * i + i);
		pDC->LineTo(m_szCell.cx * m_nColCount + m_nColCount + 1, m_szCell.cy * i + i);
	}

	CPen penBorder(PS_SOLID, 1, RGB(0, 0, 0));
	pDC->SelectObject(&penBorder);
	pDC->SelectStockObject(NULL_BRUSH);
	CRect rc;
	GetClientRect(rc);
	pDC->Rectangle(rc);

	pDC->RestoreDC(nSavedDC);
}

void CIconSelector::Show(CPoint ptTopLeft)
{
	CRect rc;

	m_szCell.cx = m_szIcon.cx + ICONSEL_MARGIN_X * 2;
	m_szCell.cy = m_szIcon.cy + ICONSEL_MARGIN_Y * 2;

	rc.left = ptTopLeft.x;
	rc.top = ptTopLeft.y;
	rc.right = rc.left + m_nColCount + 1 + m_szCell.cx * m_nColCount;
	rc.bottom = rc.top + m_nRowCount + 1 + m_szCell.cy * m_nRowCount;
	SetWindowPos( &wndTopMost , rc.left , rc.top , rc.Width() , rc.Height() , SWP_SHOWWINDOW );

	m_nCurRow = -1;
	m_nCurCol = -1;

	CWnd::ShowWindow( SW_SHOW );
}

