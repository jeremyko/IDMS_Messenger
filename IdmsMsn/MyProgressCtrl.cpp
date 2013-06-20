// MyProgressCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MyProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyProgressCtrl

CMyProgressCtrl::CMyProgressCtrl()
{
	// Defaults assigned by CProgressCtrl()
	m_nLower = 0;
	m_nUpper = 100;
	m_nCurrentPosition = 0;
	m_nStep = 10;	
	m_nFontSize = 9;
	
	// Initial colors
	m_clrStart	= RGB(236 ,245, 253);
	m_clrEnd	= RGB(133,180,100);
	m_clrBkGround = RGB(212, 212, 212);
	
	m_clrStartText	= RGB(0, 51, 135);
	m_clrEndText	= RGB(255, 255, 255);

	// Initial show percent
	m_bShowPercent = TRUE;

	// 초기에는 수평 프로그래스바로 설정합니다.
	m_IsVertical = FALSE;

	// 폰트 설정
	HDC hDC = ::GetDC(this->m_hWnd);
	m_logFont.lfHeight			= MulDiv(m_nFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	m_logFont.lfWidth			= MulDiv(m_nFontSize, GetDeviceCaps(hDC, LOGPIXELSX), 150);
	m_logFont.lfWeight			= FW_NORMAL;
	m_logFont.lfItalic			= FALSE;
	m_logFont.lfUnderline		= FALSE;
	m_logFont.lfStrikeOut		= FALSE;
	m_logFont.lfEscapement		= 0;
	m_logFont.lfOrientation		= 0;
	m_logFont.lfCharSet			= DEFAULT_CHARSET;
	m_logFont.lfOutPrecision	= OUT_CHARACTER_PRECIS;
	m_logFont.lfClipPrecision	= CLIP_CHARACTER_PRECIS;
	m_logFont.lfQuality			= DEFAULT_QUALITY;
	m_logFont.lfPitchAndFamily	= DEFAULT_PITCH|FF_DONTCARE;
	_tcscpy(m_logFont.lfFaceName, "돋움");
	::ReleaseDC(this->m_hWnd, hDC);
}

CMyProgressCtrl::~CMyProgressCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CMyProgressCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyProgressCtrl message handlers

/************************************************************************/
// The main drawing routine.  Consists of two parts
// (1) Call the DrawGradient routine to draw the visible part of the progress gradient
// (2) If needed, show the percentage text
/************************************************************************/
void CMyProgressCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	// If the current positionis  invalid then we should fade into the  background
	if (m_nCurrentPosition <= m_nLower || m_nCurrentPosition > m_nUpper)
	{
		CRect rect;
		GetClientRect(rect);
		CBrush brush;
		brush.CreateSolidBrush(m_clrBkGround);
		dc.FillRect(&rect, &brush);
		VERIFY(brush.DeleteObject());
		return;
	}


	// Figure out what part should be visible so we can stop the gradient when needed
    CRect LeftRect, RightRect;
   	CRect rectClient;
	GetClientRect(&rectClient);
	LeftRect = RightRect = rectClient;


	float maxWidth;
	
	if(m_IsVertical)
		maxWidth = (float)m_nCurrentPosition/(float)m_nUpper * (float)rectClient.bottom;
	else
		maxWidth = (float)m_nCurrentPosition/(float)m_nUpper * (float)rectClient.right;

	  	
  
    double Fraction = (double)(m_nCurrentPosition - m_nLower) / ((double)(m_nUpper - m_nLower));

#ifdef PBS_VERTICAL
    DWORD dwStyle = GetStyle();
    if (dwStyle & PBS_VERTICAL)
    {
        LeftRect.top = LeftRect.bottom - (int)((LeftRect.bottom - LeftRect.top)*Fraction);
        RightRect.bottom = LeftRect.top;
    }
    else
#endif
    {
        LeftRect.right = LeftRect.left + (int)((LeftRect.right - LeftRect.left)*Fraction);
        RightRect.left = LeftRect.right;
    }


	//시작색과 끝색이 같은 색으로 지정되었다면 끝색을 조금 다른 색으로 강제 변환
	if(m_clrEnd == m_clrStart)
	{
		
		int r, g, b;
		r = GetRValue(m_clrStart);
		g = GetGValue(m_clrStart);
		b = GetBValue(m_clrStart);
		m_clrEnd = RGB(r, g, b-20); 
	}

	// Draw the gradient
	DrawGradient(&dc, rectClient, (int)maxWidth);

	// Show percent indicator if needed
	if (m_bShowPercent)
	{
		CFont font;
		CFont *pOldFont;
		font.CreateFontIndirect(&m_logFont);	//사용자 폰트 사용 

		pOldFont = dc.SelectObject(&font); 

  	    DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
		dc.SetBkMode(TRANSPARENT);


		CString percent;
		percent.Format("%s %.0f%%", m_strText, 100.0f*(float)m_nCurrentPosition/(float)m_nUpper);

		CRgn rgn;
        rgn.CreateRectRgn(LeftRect.left, LeftRect.top, LeftRect.right, LeftRect.bottom);
        dc.SelectClipRgn(&rgn);
        dc.SetTextColor(m_clrEndText);
        dc.DrawText(percent, &rectClient, dwTextStyle);
        rgn.DeleteObject();
       
		rgn.CreateRectRgn(RightRect.left, RightRect.top, RightRect.right, RightRect.bottom);
        dc.SelectClipRgn(&rgn);
        dc.SetTextColor(m_clrStartText);
        dc.DrawText(percent, &rectClient, dwTextStyle);
		rgn.DeleteObject();
		
		dc.SelectObject(pOldFont); 
	}

	// Do not call CProgressCtrl::OnPaint() for painting messages
}


/*************************************************************************/
// Need to keep track of wher the indicator thinks it is.
/*************************************************************************/
void CMyProgressCtrl:: SetRange(int nLower, int nUpper)
{
	m_nLower = nLower;
	m_nUpper = nUpper;
	m_nCurrentPosition = nLower;
	CProgressCtrl::SetRange32(nLower, nUpper);
}


/*************************************************************************/
// Need to keep track of wher the indicator thinks it is.
/*************************************************************************/
int CMyProgressCtrl:: SetPos(int nPos)
{
	m_nCurrentPosition = nPos;
	return (CProgressCtrl::SetPos(nPos));
}

/*************************************************************************/
// Need to keep track of wher the indicator thinks it is.
/*************************************************************************/
int CMyProgressCtrl:: SetStep(int nStep)
{
	m_nStep = nStep;
	return (CProgressCtrl::SetStep(nStep));
}

/*************************************************************************/
// Need to keep track of wher the indicator thinks it is.
/*************************************************************************/
int CMyProgressCtrl:: StepIt(void)
{
	m_nCurrentPosition += m_nStep;
	return (CProgressCtrl::StepIt());
}

/*************************************************************************/
// Where most of the actual work is done.  The general version would fill the entire rectangle with
// a gradient, but we want to truncate the drawing to reflect the actual progress control position.
/*************************************************************************/
void CMyProgressCtrl::DrawGradient(CPaintDC *pDC, const RECT &rectClient, const int &nMaxWidth)
{
	RECT rectFill;			   // Rectangle for filling band
	float fStep;              // How wide is each band?
	CBrush brush;			// Brush to fill in the bar	

	
	CMem2DC memDC(pDC);

	// First find out the largest color distance between the start and end colors.  This distance
	// will determine how many steps we use to carve up the client region and the size of each
	// gradient rect.
	int r, g, b;							// First distance, then starting value
	float rStep, gStep, bStep;		// Step size for each color

	// Get the color differences
	r = (GetRValue(m_clrEnd) - GetRValue(m_clrStart));
	g = (GetGValue(m_clrEnd) - GetGValue(m_clrStart));
	b =  (GetBValue(m_clrEnd) - GetBValue(m_clrStart));


	// Make the number of steps equal to the greatest distance
	int nSteps = max(abs(r), max(abs(g), abs(b)));

	// Determine how large each band should be in order to cover the
	// client with nSteps bands (one for every color intensity level)
	if(m_IsVertical)
		fStep = (float)rectClient.bottom / (float)nSteps;
	else
		fStep = (float)rectClient.right / (float)nSteps;

	// Calculate the step size for each color
	rStep = r/(float)nSteps;
	gStep = g/(float)nSteps;
	bStep = b/(float)nSteps;

	// Reset the colors to the starting position
	r = GetRValue(m_clrStart);
	g = GetGValue(m_clrStart);
	b = GetBValue(m_clrStart);


	// Start filling bands
	for (int iOnBand = 0; iOnBand < nSteps; iOnBand++) 
	{
		if(m_IsVertical)		
			::SetRect(&rectFill,
					 0,													// 좌상단 x
					rectClient.bottom - (int)((iOnBand + 1) * fStep),   // 좌상단 y
					rectClient.right,									// 우하단 x
					rectClient.bottom - (int)(iOnBand * fStep));        // 우하단 y
		else
			::SetRect(&rectFill,
					(int)(iOnBand * fStep),								// Upper left X
					 0,													// Upper left Y
					(int)((iOnBand+1) * fStep),							// Lower right X
					rectClient.bottom+1);								// Lower right Y

	
		// CDC::FillSolidRect is faster, but it does not handle 8-bit color depth
		VERIFY(brush.CreateSolidBrush(RGB(r+rStep*iOnBand, g + gStep*iOnBand, b + bStep *iOnBand)));
		
		memDC.FillRect(&rectFill,&brush);
		VERIFY(brush.DeleteObject());


		// If we are past the maximum for the current position we need to get out of the loop.
		// Before we leave, we repaint the remainder of the client area with the background color.
		if(m_IsVertical)
		{
			if (rectClient.bottom - rectFill.top > nMaxWidth
				&& rectFill.top > 0)
			{
				::SetRect(&rectFill, 0, 0, rectClient.right, rectFill.top);
				VERIFY(brush.CreateSolidBrush(m_clrBkGround));
				memDC.FillRect(&rectFill, &brush);
				VERIFY(brush.DeleteObject());
				return;
			}
		}
		else
		{
			if (rectFill.right > nMaxWidth
				&& rectFill.right < rectClient.right)
			{
				::SetRect(&rectFill, rectFill.right, 0, rectClient.right, rectClient.bottom);
				VERIFY(brush.CreateSolidBrush(m_clrBkGround));
				memDC.FillRect(&rectFill, &brush);
				VERIFY(brush.DeleteObject());
				return;
			}
		}
	}
}

/*************************************************************************/
// All drawing is done in the OnPaint function
/*************************************************************************/
BOOL CMyProgressCtrl::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CMyProgressCtrl::SetText(CString str)
{
	m_strText = str;
}
