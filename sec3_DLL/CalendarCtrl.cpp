// CalendarCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "CalendarCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalendarCtrl

IMPLEMENT_DYNAMIC(CCalendarCtrl, CWnd)

CCalendarCtrl::CCalendarCtrl()
{
	RegisterWindowClass();

	m_strWeekdayName[WD_SUN-1] = _T("일요일");
	m_strWeekdayName[WD_MON-1] = _T("월요일");
	m_strWeekdayName[WD_TUE-1] = _T("화요일");
	m_strWeekdayName[WD_WED-1] = _T("수요일");
	m_strWeekdayName[WD_THU-1] = _T("목요일");
	m_strWeekdayName[WD_FRI-1] = _T("금요일");
	m_strWeekdayName[WD_SAT-1] = _T("토요일");

	//--- 글꼴 설정 ------------------------
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 18;
	lf.lfWeight = FW_BOLD;
	_tcscpy(lf.lfFaceName, _T("굴림체"));
	m_fontTitle.CreateFontIndirect(&lf);

	lf.lfHeight = 12;
	lf.lfWeight = 0;
	_tcscpy(lf.lfFaceName, _T("굴림"));
	m_fontHeader.CreateFontIndirect(&lf);
	m_fontItem.CreateFontIndirect(&lf);

	lf.lfHeight = 16;
	lf.lfWeight = FW_BOLD;
	m_fontDaySolar.CreateFontIndirect(&lf);

	lf.lfHeight = 11;
	lf.lfWeight = 0;
	m_fontDayLunar.CreateFontIndirect(&lf);

	lf.lfHeight = 11;
	lf.lfWeight = 0;
	m_fontDayName.CreateFontIndirect(&lf);
	//--------------------------------------

	//m_nWeekCount = m_kdDate.GetWeekCount();

	//--- 색상 설정 ------------------------
	m_crBorderLine = RGB(0, 0, 0);   // 테두리 : 검정색
	m_crCellLine = GetSysColor(COLOR_BTNSHADOW);     // 셀라인 색 GetSysColor(COLOR_BTNSHADOW);

	m_crNormalDay = RGB(0, 0, 0);          // 평일 : 검정색
	m_crHoliday = RGB(220, 0, 0);          // 공휴일 : 빨간색
	m_crSaturday = RGB(0, 0, 150);         // 토요일 : 파란색
	m_crNormalBack = RGB(255, 255, 255);   // 당월 보통 셀 배경색
	m_crNoneBack = RGB(230, 230, 230);     // 비당월 셀 배경색
	//--------------------------------------

	m_pData = NULL;
	m_pImageList = NULL;
	m_hIcon = NULL;
	m_dwBorderStyle = CALBS_NONE;
	m_bHasFocus = false;
	m_bNewMonth = false;
	m_bResized = false;
	m_bMouseDown = false;
	m_bMouseOnTodayBtn = false;
	m_nCurYear = 0;
	m_nCurMonth = 0;

	SetRowColByDate();
}

CCalendarCtrl::~CCalendarCtrl()
{
	m_fontTitle.DeleteObject();
	m_fontHeader.DeleteObject();
	m_fontItem.DeleteObject();
	m_fontDaySolar.DeleteObject();
	m_fontDayLunar.DeleteObject();
	m_fontDayName.DeleteObject();
}


BEGIN_MESSAGE_MAP(CCalendarCtrl, CWnd)
	//{{AFX_MSG_MAP(CCalendarCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(CNEN_RETURN, OnUmCNENotifyReturn)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCalendarCtrl message handlers

BOOL CCalendarCtrl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if( !(::GetClassInfo(hInst, CALENDARCTRL_CLASSNAME, &wndcls)) )
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc		= ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH)(COLOR_BTNFACE);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = CALENDARCTRL_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
	}

	return TRUE;
}

int CCalendarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_wndEdit.Create(WS_CHILD|WS_BORDER, CRect(0, 0, 100, 20), this, IDC_CAL_EDITCTRL);
	m_wndEdit.SetFont(&m_fontTitle);

	m_Tooltip.Create(this, TTS_NOPREFIX);
	m_Tooltip.AddTool(this, _T(""));
	m_Tooltip.SetMaxTipWidth(600);
	m_Tooltip.SetDelayTime(TTDT_INITIAL, 1000);
	m_Tooltip.SetDelayTime(TTDT_AUTOPOP, 1696);   // 16960 이 최대값인거 같다.

	return 0;
}

BOOL CCalendarCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void CCalendarCtrl::SetSmallIcon(HICON icon)
{
	m_hIcon = icon; 
}

void CCalendarCtrl::SetImageList(CImageList* pImageList)
{
	m_pImageList = pImageList;
}

CImageList* CCalendarCtrl::GetImageList()
{
	return m_pImageList;
}

CKoreanDate CCalendarCtrl::GetDate() const
{
	return m_kdDate;
}

int CCalendarCtrl::GetSelectedItemIndex()
{
	int nIndex;
	TCellInfo* pInfo = &m_arrCellInfo[m_nCurRow][m_nCurCol];

	if( !m_bItemNavigating ) return -1;
	if( pInfo->nCount == 0 ) return -1;

	nIndex = pInfo->nSelected;
	if( nIndex > pInfo->nCount - 1 ) return -1;

	return nIndex;
}

void CCalendarCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(&dc);
	int nSavedDC;
	int i;
	int nWidth, nRemain;
	int nMarginLeft=0, nMarginRight=0, nMarginTop=0, nMarginBottom=0;
	CRect rcClient;
	CRect rcClip;
	CFont *pOldFont;
	CString sTemp;
	CSize szTitle;

	nSavedDC = memDC.SaveDC();

	//--- 년월이 바뀌었는지 검사 ---------------------
	if( m_nCurYear == m_kdDate.GetYear() && m_nCurMonth == m_kdDate.GetMonth() )
	{
		m_bNewMonth = false;
	}
	else
	{
		m_bNewMonth = true;
	}
	//------------------------------------------------

	GetClientRect(&rcClient);

	//--- 화면 바탕색 칠하기 -------------------------
	CBrush br(GetSysColor(COLOR_BTNFACE));
	memDC.FillRect(&rcClient, &br);
	//------------------------------------------------

	//--- 타이틀바 높이구하기 ------------------------
	pOldFont = (CFont *)memDC.SelectObject(&m_fontTitle);
	sTemp = _T("2002년 12월");
	szTitle = memDC.GetTextExtent(sTemp);
	memDC->SelectObject(pOldFont);
	m_nTitleHeight = szTitle.cy + 8;
	//------------------------------------------------

	//--- 마진 설정 및 테두리 그리기 -----------------
	m_nHeaderHeight = GetSystemMetrics(SM_CYCAPTION);
	m_nWeekCount = m_kdDate.GetWeekCount();
	//m_dwBorderStyle = CALBS_NONE;
	//m_dwBorderStyle = CALBS_SUNKEN;
	//m_dwBorderStyle = CALBS_RAISED;
	//m_dwBorderStyle = CALBS_ETCHED;
	//m_dwBorderStyle = CALBS_BUMP;
	if( m_dwBorderStyle == CALBS_NONE )
	{
	}
	else if( m_dwBorderStyle == CALBS_SINGLE )
	{
		nMarginLeft = 1;
		nMarginRight = 1;
		nMarginTop = 1;
		nMarginBottom = 1;

		CPen pen(PS_SOLID, 1, m_crBorderLine);
		CPen* pOldPen = (CPen *)memDC.SelectObject(&pen);
		CBrush *pOldBr = (CBrush *)memDC.SelectStockObject(NULL_BRUSH);
		memDC.Rectangle(&rcClient);
		memDC.SelectObject(pOldPen);
		memDC.SelectObject(pOldBr);
	}
	else
	{
		nMarginLeft = 2;
		nMarginRight = 2;
		nMarginTop = 2;
		nMarginBottom = 2;
		if( m_dwBorderStyle == CALBS_RAISED || m_dwBorderStyle == CALBS_ETCHED )
		{
			nMarginLeft++;
			nMarginTop++;
			nMarginRight++;
			nMarginBottom++;
		}

		memDC.DrawEdge(&rcClient, m_dwBorderStyle, BF_RECT);
	}
	rcClient.DeflateRect(nMarginLeft, nMarginTop + m_nTitleHeight + m_nHeaderHeight, nMarginRight, nMarginBottom);
	//------------------------------------------------

	//--- X 좌표 설정 --------------------------------
	nRemain = rcClient.Width();
	nWidth = (int)(nRemain / 7);
	nRemain %= 7;   // 정확히 7로 나누어 떨어지지 않으므로 나머지를 각 칸에 1씩 분배
	m_narrPosX[0] = rcClient.left;
	for(i = 1; i <= 7; i++)
	{
		m_narrPosX[i] = m_narrPosX[i-1] + nWidth;
		if(nRemain > 0)
		{
			m_narrPosX[i] += 1;
			nRemain--;
		}
	}
	//------------------------------------------------

	//--- Y 좌표 설정 --------------------------------
	nRemain = rcClient.Height();
	nWidth = (int)(nRemain / m_nWeekCount);
	nRemain %= m_nWeekCount;
	m_narrPosY[0] = rcClient.top;
	for(i = 1; i <= m_nWeekCount; i++)
	{
		m_narrPosY[i] = m_narrPosY[i-1] + nWidth;
		if(nRemain > 0)
		{
			m_narrPosY[i] += 1;
			nRemain--;
		}
	}
	//------------------------------------------------

	// 타이틀바 그리기
	rcClip.CopyRect(&rcClient);
	rcClip.top = nMarginTop;
	rcClip.bottom = nMarginTop + m_nTitleHeight;
	DrawTitle(&memDC, rcClip);

	// 요일 헤더 그리기
	rcClip.top = rcClip.bottom;
	rcClip.bottom = rcClip.top + m_nHeaderHeight;
	DrawHeader(&memDC, rcClip);  // Draw Top Header

	// 각 셀 그리기
	DrawCell(&memDC);

	// 셀 라인 그리기
	DrawCellLine(&memDC);

	memDC.RestoreDC(nSavedDC);

	m_nCurYear = m_kdDate.GetYear();
	m_nCurMonth = m_kdDate.GetMonth();
	m_bResized	= false;
}

// 타이틀바 그리기
void CCalendarCtrl::DrawTitle(CDC* pDC, const CRect rcClip)
{
	int nSavedDC;
	CString sDate;
	CRect rc;
	CSize szDate, szTemp;

	nSavedDC = pDC->SaveDC();

	rc.CopyRect(&rcClip);

	pDC->Draw3dRect(&rc, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNSHADOW));
	// 배경색 칠하기
	rc.DeflateRect(2, 2);
	pDC->FillRect(&rc, &CBrush(GetSysColor(COLOR_BTNSHADOW)) );
	pDC->IntersectClipRect(&rc);

	if( m_hIcon )
	{
		int nTemp = (rc.Height() - 16) / 2;
		::DrawIconEx(pDC->m_hDC, rc.left+nTemp, rc.top+nTemp, m_hIcon, 16, 16, 0, NULL, DI_NORMAL);
	}

	// 오늘 날짜 버튼 좌표 구하기
	m_rcTodayBtn.SetRect(rc.right - 67, rc.top + 1, rc.right - 2, rc.bottom - 1);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&m_fontTitle);
	sDate.Format(_T("%4d년 %02d월"), m_kdDate.GetYear(), m_kdDate.GetMonth());
	szDate = pDC->GetTextExtent(sDate);
	rc.OffsetRect( (rc.Width() - szDate.cx) / 2, (rc.Height() - szDate.cy) / 2 );
	rc.right = rc.left + szDate.cx;
	rc.bottom = rc.top + szDate.cy;

	// 일자 출력
	pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	pDC->DrawText(sDate, &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

	// 년도 좌표 구하기
	sDate = _T("0000");
	szDate = pDC->GetTextExtent(sDate);
	m_rcYear.CopyRect(&rc);
	m_rcYear.right = m_rcYear.left + szDate.cx;
	//m_rcYear.bottom = m_rcYear.top + szDate.cy;
	m_rcYear.InflateRect(2, 2);

	// 월 좌표 구하기
	sDate = _T("0000년 ");
	szDate = pDC->GetTextExtent(sDate);
	m_rcMonth.CopyRect(&rc);
	m_rcMonth.left += szDate.cx;
	sDate = _T("월");
	szDate = pDC->GetTextExtent(sDate);
	m_rcMonth.right -= szDate.cx;
	m_rcMonth.InflateRect(2, 2);

	//--- 오늘 일자 단추 그리기 ------------
	pDC->SelectObject(&m_fontHeader);
	pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	rc.CopyRect(&m_rcTodayBtn);
	if( m_bMouseOnTodayBtn )
	{
		if( m_bMouseDown )
		{
			pDC->Draw3dRect(&rc, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_BTNHILIGHT));
			rc.DeflateRect(1, 1);
			rc.OffsetRect(1, 1);
		}
		else
		{
			pDC->Draw3dRect(&rc, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
		}
	}
	pDC->DrawText(_T("오늘 날짜"), &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	//--------------------------------------

	pDC->RestoreDC(nSavedDC);
}

void CCalendarCtrl::DrawHeader(CDC* pDC, const CRect rcClip)
{
	int nSavedDC;
	CRect rc;
	int i;

	nSavedDC = pDC->SaveDC();

	rc.CopyRect(&rcClip);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&m_fontHeader);
	pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));

	for(i = 0; i <= 6; i++)
	{
		rc.left = m_narrPosX[i];
		rc.right = m_narrPosX[i+1];

		/* 헤더 색깔 표시
		switch(i)
		{
		case 0:
			pDC->SetTextColor(m_crHoliday);
			break;
		case 1:
			pDC->SetTextColor(m_crNormalDay);
			break;
		case 6:
			pDC->SetTextColor(m_crSaturday);
			break;
		}
		*/
		pDC->DrawText(m_strWeekdayName[i], &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
		pDC->Draw3dRect(&rc, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNSHADOW));

		// 눌린 단추
		//pDC->Draw3dRect(&rc, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNHILIGHT));
	}

	pDC->RestoreDC(nSavedDC);
}

void CCalendarCtrl::DrawCellLine(CDC* pDC)
{
	int i;

	CPen penLine(PS_SOLID, 1, m_crCellLine);
	CPen* pOldPen = pDC->SelectObject(&penLine);

	// 세로선
	for(i = 1; i < 7; i++)
	{
		pDC->MoveTo(m_narrPosX[i], m_narrPosY[0]);
		pDC->LineTo(m_narrPosX[i], m_narrPosY[m_nWeekCount]);
	}

	// 가로선
	for(i = 1; i < m_nWeekCount; i++)
	{
		pDC->MoveTo(m_narrPosX[0], m_narrPosY[i]);
		pDC->LineTo(m_narrPosX[7], m_narrPosY[i]);
	}

	pDC->SelectObject(pOldPen);
}

void CCalendarCtrl::DrawCell(CDC *pDC)
{
	int nSavedDC;
	int nRow, nCol;
	int nCurrentMonth;
	CKoreanDate dt(m_kdDate.GetYear(), m_kdDate.GetMonth(), 1);
	CString s;
	CRect rcClip;
	CRect rcDayNum;
	CRect rcDayInfo;
	CRect rcList;
	CSize sz;
	int nDayHeight;
	TDayInfo diSolar, diLunar;
	int nRet;
	CFont *pOldFont;

	BOOL bCurCell;
	COLORREF  crBack, crText;

	nSavedDC = pDC->SaveDC();

	pDC->SelectObject(&m_fontDayName);
	s = _T("가");
	sz = pDC->GetTextExtent(s);
	nDayHeight = sz.cy;
	pDC->SelectObject(&m_fontDaySolar);
	sz = pDC->GetTextExtent(s);
	nDayHeight += sz.cy;
	pDC->SetBkMode(TRANSPARENT);

	nCurrentMonth = m_kdDate.GetMonth();
	dt.DateAdd( 0, 0, -(dt.GetDayOfWeek() - 1) );
	for(nRow = 0; nRow < m_nWeekCount; nRow++)
	{
		for(nCol = 0; nCol < 7; nCol++)
		{
			GetCellRect(nRow, nCol, &rcClip);
			bCurCell = ( nRow == m_nCurRow && nCol == m_nCurCol ) ? TRUE : FALSE;

			//--- 배경색 채우기 ------------------------------
			crBack = ( (nCurrentMonth == dt.GetMonth()) ? m_crNormalBack : m_crNoneBack );
			pDC->SetBkColor(crBack);
			pDC->FillRect( &rcClip, &(CBrush(crBack)) );

			rcDayNum.CopyRect(&rcClip);
			rcDayNum.DeflateRect(1, 1);
			rcDayNum.bottom = rcDayNum.top + nDayHeight + (rcDayNum.top - rcClip.top) * 2 + 1;
			if( bCurCell )
			{
				crBack = ( m_bHasFocus && !m_bItemNavigating ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_BTNFACE) );
				pDC->SetBkColor(crBack);
				pDC->FillRect(&rcDayNum, &CBrush(crBack));
			}
			rcClip.DeflateRect(2, 2);
			rcList.CopyRect(&rcClip);
			rcList.left--;
			rcList.top = rcDayNum.bottom + 2;
			//------------------------------------------------

			//--- 요일별 글자색 지정 -------------------------
			if( dt.IsHoliday() )
			{
				crText = m_crHoliday;
			}
			else
			{
				crText = ( dt.GetDayOfWeek() == 7 ) ? m_crSaturday : m_crNormalDay;
			}
			if( bCurCell && m_bHasFocus && !m_bItemNavigating ) crText = GetComplementaryColor(crText);
			pDC->SetTextColor(crText);
			//------------------------------------------------

			//--- 양력 일자 표시 -----------------------------
			s.Format(_T("%d"), dt.GetDay());
			sz = pDC->GetTextExtent(s);
			rcDayNum.CopyRect(&rcClip);
			rcDayNum.right = rcDayNum.left + sz.cx;
			rcDayNum.bottom = rcDayNum.top + sz.cy;
			rcDayNum.IntersectRect(&rcDayNum, &rcClip);
			pDC->DrawText(s, &rcDayNum, DT_LEFT|DT_TOP);
			//------------------------------------------------

			//--- 음력 표시 ----------------------------------
			if( (dt.GetDay(TYPE_LUNAR) % 5) == 1 )  // 5일에 1번만 표시
			{
				rcDayInfo.CopyRect(&rcDayNum);
				rcDayInfo.right = rcClip.right - 1;
				rcDayInfo.left = rcDayNum.right + 2;
				rcDayInfo.NormalizeRect();

				s.Format(_T("%d.%d"), dt.GetMonth(TYPE_LUNAR), dt.GetDay(TYPE_LUNAR));
				pOldFont = (CFont *)pDC->SelectObject(&m_fontDayLunar);

				crText = ( (bCurCell && m_bHasFocus && !m_bItemNavigating) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : RGB(0, 0, 0) );
				pDC->SetTextColor( crText );
				pDC->DrawText(s, &rcDayInfo, DT_SINGLELINE|DT_RIGHT|DT_BOTTOM);
				pDC->SelectObject(pOldFont);
			}
			//------------------------------------------------

			//--- 일자 정보 표시 -----------------------------
			nRet = dt.GetDayInfo(&diSolar, &diLunar);
			if( nRet != 0 )
			{
				rcDayInfo.CopyRect(&rcDayNum);
				rcDayInfo.OffsetRect(0, sz.cy);
				rcDayInfo.right = rcClip.right;

				s = _T("");
				if( nRet & TYPE_SOLAR ) s = diSolar.szDayName;
				if( nRet & TYPE_LUNAR )
				{
					if( nRet & TYPE_SOLAR && _tcslen(diLunar.szDayName) > 0 )
						s += _T(",");
					s += diLunar.szDayName;
				}
				pOldFont = (CFont *)pDC->SelectObject(&m_fontDayName);
				sz = pDC->GetTextExtent(s);
				rcDayInfo.bottom = rcDayInfo.top + sz.cy;
				crText = ( (bCurCell && m_bHasFocus && !m_bItemNavigating) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : RGB(0, 0, 0) );
				pDC->SetTextColor( crText );
				pDC->DrawText(s, &rcDayInfo, DT_SINGLELINE|DT_LEFT|DT_TOP|DT_END_ELLIPSIS);
				pDC->SelectObject(pOldFont);
			}
			//------------------------------------------------

			DrawCellItems(nRow, nCol, pDC, rcList, &dt);

			dt.DateAdd(0, 0, 1);
		}
	}

	pDC->RestoreDC(nSavedDC);
}

void CCalendarCtrl::DrawCellItems(int nRow, int nCol, CDC* pDC, const CRect rcClip, CKoreanDate* pDate)
{
	int nSavedDC;
	CRect rc, rcItem, rcCaption;
	CSize sz;
	int nCount;
	TCellItem item;
	TCellInfo* pInfo;
	int	nHeight, nWidth;

	if( m_pData == NULL ) return;

	nSavedDC = pDC->SaveDC();

	rc.CopyRect(&rcClip);
	pInfo = &m_arrCellInfo[nRow][nCol];
	pDC->IntersectClipRect(&rc);
	pDC->SelectObject(&m_fontItem);

	if( m_bNewMonth )  // 년월이 바뀌었으면
	{
		nCount = m_pData->FindFirstItem(*pDate, &item);
		pInfo->Clear();
		pInfo->nCount = nCount;
		if( nCount == 0 )
		{
			pDC->RestoreDC(nSavedDC);
			return;
		}

		nHeight = nCount * CAL_ITEM_HEIGHT;
		//--- 스크롤바 표시 ---------------------------
		if( nHeight > rc.Height() )
		{
			pInfo->bScrollBar = true;
			nHeight = (int)(rc.Height() * (float)(rc.Height() / (float)nHeight));
			pInfo->rcScrollBar.SetRect(rc.left, rc.top, rc.left + 6, rc.bottom);
			pDC->Draw3dRect(&(pInfo->rcScrollBar),
							GetSysColor(COLOR_BTNFACE),
							GetSysColor(COLOR_3DDKSHADOW));
			pDC->FillSolidRect(pInfo->rcScrollBar.left + 1,
							   pInfo->rcScrollBar.top + 1,
							   4,
							   nHeight,
							   GetSysColor(COLOR_BTNSHADOW));
			rc.left += 7;
		}
		else
		{
			pInfo->bScrollBar = false;
			pInfo->rcScrollBar.SetRect(0, 0, 0, 0);
		}
		pInfo->rcItem.CopyRect(&rc);
		/*
		if( pInfo->rcItem.Height() > nHeight )
			pInfo->rcItem.bottom = pInfo->rcItem.top + nHeight;
		*/
		//---------------------------------------------

		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(255, 255, 255));

		rcItem.CopyRect(&rc);
		rcItem.bottom = rcItem.top + CAL_ITEM_HEIGHT;
		nWidth = ( m_pImageList ) ? 18 : 0;
		while( true )
		{
			if( m_pImageList )
			{
				m_pImageList->Draw(pDC, item.nImage, rcItem.TopLeft(), ILD_TRANSPARENT);
				rcItem.left += nWidth;
			}
			pDC->DrawText(item.strCaption, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

			if( rcItem.top > rcClip.bottom ) break;
			if( !m_pData->FindNextItem(&item) ) break;

			rcItem.left -= nWidth;
			rcItem.OffsetRect(0, CAL_ITEM_HEIGHT);
		}
	}
	else  // 년월이 바뀌지 않았으면
	{
		int nScrolled = 0;
		float ratio;
		nCount = pInfo->nCount;
		if( nCount == 0 )
		{
			pDC->RestoreDC(nSavedDC);
			return;
		}

		nHeight = nCount * CAL_ITEM_HEIGHT;
		if( m_bResized )
		{
			pInfo->rcItem.CopyRect(&rc);
			if( nHeight > rc.Height() )
			{
				pInfo->rcItem.left -= 7;
			}
			RecalcItemPosition(nRow, nCol, true);
		}

		//--- 스크롤바 표시 ---------------------------
		if( nHeight > rc.Height() )
		{
			pInfo->bScrollBar = true;
			ratio = rc.Height() / (float)nHeight;
			nHeight = (int)(rc.Height() * ratio);
			nScrolled = (int)((pInfo->nTop * CAL_ITEM_HEIGHT) * ratio);

			pInfo->rcScrollBar.SetRect(rc.left, rc.top, rc.left + 6, rc.bottom);
			//--- Scrollbar thumb이 범위 넘어가지 않게 조정 ---
			if( pInfo->rcScrollBar.Height() - 2 < nHeight + nScrolled )
			{
				nHeight = pInfo->rcScrollBar.Height() - 2 - nScrolled;
			}
			//-------------------------------------------------
			pDC->Draw3dRect(&(pInfo->rcScrollBar),
							GetSysColor(COLOR_BTNFACE),
							GetSysColor(COLOR_3DDKSHADOW));
			pDC->FillSolidRect(pInfo->rcScrollBar.left + 1,
							   pInfo->rcScrollBar.top + 1 + nScrolled,
							   4,
							   nHeight,
							   GetSysColor(COLOR_BTNSHADOW));
			rc.left += 7;
		}
		else
		{
			pInfo->bScrollBar = false;
			pInfo->rcScrollBar.SetRect(0, 0, 0, 0);
		}
		pInfo->rcItem.CopyRect(&rc);
		/*
		if( pInfo->rcItem.Height() > nHeight )
			pInfo->rcItem.bottom = pInfo->rcItem.top + nHeight;
		*/
		//---------------------------------------------

		pDC->SetTextColor(RGB(0, 0, 0));
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);

		rcItem.CopyRect(&rc);
		rcItem.bottom = rcItem.top + CAL_ITEM_HEIGHT;
		nWidth = ( m_pImageList ) ? 18 : 0;

		int nIndex = pInfo->nTop;
		UINT nImageStyle;
		m_pData->GetItem(*pDate, nIndex, &item);
		while( true )
		{
			if( m_pImageList )
			{
				if( pInfo->nSelected == nIndex && m_bItemNavigating &&
					nRow == m_nCurRow && nCol == m_nCurCol )
				{
					nImageStyle = ILD_SELECTED;
				}
				else
				{
					nImageStyle = ILD_TRANSPARENT;
				}

				m_pImageList->Draw(pDC, item.nImage, rcItem.TopLeft(), nImageStyle);
				rcItem.left += nWidth;
			}

			rcCaption.CopyRect(&rcItem);
			sz = pDC->GetTextExtent(item.strCaption);
			sz.cx += 1;
			if( rcCaption.Width() > sz.cx )
				rcCaption.right = rcCaption.left + sz.cx;

			// 현재 선택된 아이템이면 선택 표시
			if( pInfo->nSelected == nIndex && m_bItemNavigating && 
				nRow == m_nCurRow && nCol == m_nCurCol )
			{
				if( m_bHasFocus )
				{
					pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
					pDC->FillSolidRect( &rcCaption, GetSysColor(COLOR_HIGHLIGHT) );
				}
				else
				{
					pDC->SetTextColor(RGB(0, 0, 0));
				}
				pDC->DrawFocusRect(&rcCaption);
			}
			else
			{
				pDC->SetTextColor(RGB(0, 0, 0));
			}
			pDC->DrawText(item.strCaption , &rcCaption, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

			if( rcItem.top > rcClip.bottom ) break;
			if( !m_pData->FindNextItem(&item) ) break;

			rcItem.left -= nWidth;
			rcItem.OffsetRect(0, CAL_ITEM_HEIGHT);
			nIndex++;
		}
	}  // end of if( m_bNewMonth )

	pDC->RestoreDC(nSavedDC);
}

void CCalendarCtrl::RecalcItemPosition(int nRow, int nCol, bool bResized)
{
	TCellInfo* pInfo;
	int nTotalHeight, nSelHeight;
	int nOffset;

	pInfo = &(m_arrCellInfo[nRow][nCol]);
	nTotalHeight = pInfo->nCount * CAL_ITEM_HEIGHT;
	if( pInfo->rcItem.Height() >= nTotalHeight )
	{
		pInfo->nTop = 0;
		return;
	}

	if( bResized )   // 셀 크기가 변경되었을 때
	{
		nSelHeight = (pInfo->nSelected + 1) * CAL_ITEM_HEIGHT;
		if( pInfo->rcItem.Height() >= nSelHeight )
		{
			pInfo->nTop = 0;
		}
		else
		{
			int nMaxView = (int)(pInfo->rcItem.Height() / CAL_ITEM_HEIGHT);
			if( nMaxView <= 1 )
			{
				pInfo->nTop = pInfo->nSelected;
			}
			else
			{
				pInfo->nTop = pInfo->nSelected - nMaxView + 1;
				if( pInfo->nTop < 0 ) pInfo->nTop = 0;
				if( pInfo->nTop > pInfo->nSelected ) pInfo->nTop = pInfo->nSelected;
			}
		}
	}
	else
	{
		nSelHeight = (pInfo->nSelected - pInfo->nTop + 1) * CAL_ITEM_HEIGHT;
		if( pInfo->rcItem.Height() < nSelHeight )
		{
			nOffset = (int)((nSelHeight - pInfo->rcItem.Height()) / CAL_ITEM_HEIGHT) + 1;
			pInfo->nTop += nOffset;
		}
		if( pInfo->nTop > pInfo->nSelected ) pInfo->nTop = pInfo->nSelected;
	}
}

void CCalendarCtrl::SetDataInterface(ICalData* picd)
{
	m_pData = picd;
}

COLORREF CCalendarCtrl::GetCellLineColor()
{
	return m_crCellLine;
}

void CCalendarCtrl::SetCellLineColor(COLORREF crCellLine)
{
	m_crCellLine = crCellLine;
}

COLORREF CCalendarCtrl::GetBorderLineColor()
{
	return m_crBorderLine;
}

void CCalendarCtrl::SetBorderLineColor(COLORREF crBorderLine)
{
	m_crBorderLine = crBorderLine;
}

void CCalendarCtrl::SetWeekdayName(EWeekDay wd, CString strName)
{
	if( wd < WD_SUN || wd > WD_SAT )
		return;

	m_strWeekdayName[wd-1] = strName;
}

CString CCalendarCtrl::GetWeekdayName(EWeekDay wd)
{
	if( wd < WD_SUN || wd > WD_SAT )
		return _T("");

	return m_strWeekdayName[wd-1];
}

BOOL CCalendarCtrl::SetBorderStyle(DWORD dwStyle)
{
	if( (dwStyle == CALBS_NONE) || 
		(dwStyle == CALBS_RAISED) || 
		(dwStyle == CALBS_ETCHED) || 
		(dwStyle == CALBS_SUNKEN) || 
		(dwStyle == CALBS_BUMP) )
	{
		m_dwBorderStyle = dwStyle;
		return TRUE;
	}

	return FALSE;
}

void CCalendarCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

	m_bHasFocus = false;
	Invalidate(FALSE);
}

void CCalendarCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	m_bHasFocus = true;	
	Invalidate(FALSE);
}

// nRow, nCol 셀의 좌표를 구한다.
// nRow, nCol은 0 base
BOOL CCalendarCtrl::GetCellRect(int nRow, int nCol, LPRECT lpRect)
{
	if( lpRect == NULL ) return FALSE;
	if( nRow < 0 || nRow > (m_nWeekCount - 1) ) return FALSE;
	if( nCol < 0 || nCol > 6 ) return FALSE;

	lpRect->left = m_narrPosX[nCol]+1;
	lpRect->right = m_narrPosX[nCol+1];
	lpRect->top = m_narrPosY[nRow]+1;
	lpRect->bottom = m_narrPosY[nRow+1];

	if( nRow == 0 ) lpRect->top--;
	if( nCol == 0 ) lpRect->left--;

	return TRUE;
}

void CCalendarCtrl::ShowYearEdit()
{
	CString sYear;
	sYear.Format(_T("%d"), m_kdDate.GetYear());
	m_wndEdit.m_nWhatDoing = 1;  //Year
	m_wndEdit.SetWindowText( (LPCTSTR)sYear );
	m_wndEdit.MoveWindow(&m_rcYear);
	m_wndEdit.m_nMinValue = 3;
	m_wndEdit.m_nMaxValue = 9999;
	m_wndEdit.ShowWindow(SW_SHOW);
	m_wndEdit.SetSel(0, sYear.GetLength());
	m_wndEdit.SetFocus();
}

void CCalendarCtrl::ShowMonthEdit()
{
	CString sMonth;
	sMonth.Format(_T("%d"), m_kdDate.GetMonth());
	m_wndEdit.m_nWhatDoing = 2;  //Month
	m_wndEdit.SetWindowText( (LPCTSTR)sMonth );
	m_wndEdit.MoveWindow(&m_rcMonth);
	m_wndEdit.m_nMinValue = 1;
	m_wndEdit.m_nMaxValue = 12;
	m_wndEdit.ShowWindow(SW_SHOW);
	m_wndEdit.SetSel(0, sMonth.GetLength());
	m_wndEdit.SetFocus();
}

void CCalendarCtrl::OnSetToday()
{
	m_kdDate.SetDateAsCurrent();
	SetRowColByDate();
	Invalidate(FALSE);
}

void CCalendarCtrl::GetRowColClicked(CPoint point, int* pnRow, int* pnCol)
{
	int i;
	*pnRow = -1;
	*pnCol = -1;

	for(i = 0; i < 7; i++)
	{
		if( point.x > m_narrPosX[i] && point.x < m_narrPosX[i+1] )
		{
			*pnCol = i;
			break;
		}
	}

	for(i = 0; i < m_nWeekCount; i++)
	{
		if( point.y > m_narrPosY[i] && point.y < m_narrPosY[i+1] )
		{
			*pnRow = i;
			break;
		}
	}
}

void CCalendarCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int nRow=-1, nCol=-1;

	m_bMouseDown = true;
	SetCapture();
	if( m_rcTodayBtn.PtInRect(point) )
	{
		InvalidateRect(&m_rcTodayBtn, FALSE);
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	m_bItemNavigating = false;

	if( m_rcYear.PtInRect(point) )
	{
		ShowYearEdit();
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	if( m_rcMonth.PtInRect(point) )
	{
		ShowMonthEdit();
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	LockWindowUpdate();
	SetFocus();
	UnlockWindowUpdate();

	GetRowColClicked(point, &nRow, &nCol);

	if( nRow >= 0 && nCol >= 0 )
	{
		m_nCurRow = nRow;
		m_nCurCol = nCol;
		SetDateByRowCol();

		TCellInfo* pInfo = &m_arrCellInfo[nRow][nCol];
		if( pInfo->rcScrollBar.PtInRect(point) ) m_bItemNavigating = true;
		if( pInfo->rcItem.PtInRect(point) )
		{
			m_bItemNavigating = true;

			int nIndex;
			nIndex = (int)((point.y - pInfo->rcItem.top) / CAL_ITEM_HEIGHT) + pInfo->nTop;
			if( nIndex < pInfo->nCount )
			{
				pInfo->nSelected = nIndex;
				RecalcItemPosition(nRow, nCol);
			}
			else
			{
				pInfo->nSelected = pInfo->nCount - 1;
				RecalcItemPosition(nRow, nCol);
			}
		}

		Invalidate(FALSE);
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CCalendarCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bMouseDown = false;
	ReleaseCapture();
	if( m_rcTodayBtn.PtInRect(point) )
	{
		OnSetToday();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CCalendarCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	//--- 오늘 날짜 단추 처리 -----------------------
	if( m_rcTodayBtn.PtInRect(point) ? true : false )
	{
		if( !m_bMouseOnTodayBtn )
		{
			m_bMouseOnTodayBtn = true;
			if(!m_bMouseDown) SetCapture();
			InvalidateRect(&m_rcTodayBtn , FALSE);
		}
	}
	else
	{
		if( m_bMouseOnTodayBtn )
		{
			m_bMouseOnTodayBtn = false;
			if(!m_bMouseDown) ReleaseCapture();
			InvalidateRect(&m_rcTodayBtn , FALSE);
		}
	}
	//-----------------------------------------------
	
	CWnd::OnMouseMove(nFlags, point);
}

void CCalendarCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int nYear = 0;
	int nMonth = 0;


	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);
	
	m_Tooltip.Activate(FALSE);

	if( !m_bItemNavigating )  // Item navigating 중이 아닐 때 => 일자 이동
	{
		switch( nChar )
		{
		case VK_RETURN:
			{
				TCellInfo* pInfo = &(m_arrCellInfo[m_nCurRow][m_nCurCol]);
				if( pInfo->nCount == 0 )
				{
					m_pData->OnDayKeyDown(nChar, nRepCnt, nFlags);
				}
				else
				{
					m_bItemNavigating = true;
					if( pInfo->nSelected < 0 ) pInfo->nSelected = pInfo->nTop;
					Invalidate(FALSE);
				}
				break;
			}
		case VK_LEFT:
			m_kdDate.DateAdd(0, 0, -1);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(0, 0, 1);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		case VK_RIGHT:
			m_kdDate.DateAdd(0, 0, 1);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(0, 0, -1);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		case VK_DOWN:
			m_kdDate.DateAdd(0, 0, 7);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(0, 0, -7);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		case VK_UP:
			m_kdDate.DateAdd(0, 0, -7);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(0, 0, 7);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		case 33:
			if( ::GetAsyncKeyState( VK_CONTROL ) )
				nYear = 1;
			else
				nMonth = 1;
			m_kdDate.DateAdd(-nYear, -nMonth, 0);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(nYear, nMonth, 0);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		case 34:  // PageDown
			if( ::GetAsyncKeyState( VK_CONTROL ) )
				nYear = 1;
			else
				nMonth = 1;
			m_kdDate.DateAdd(nYear, nMonth, 0);
			if( m_kdDate.GetYear() <= 2 )
			{
				m_kdDate.DateAdd(-nYear, -nMonth, 0);
				break;
			}
			SetRowColByDate();
			Invalidate(FALSE);
			break;
		}
	}
	else   // Item navigating 중일 때
	{
		TCellInfo* pInfo = &(m_arrCellInfo[m_nCurRow][m_nCurCol]);
		int nMaxView;

		switch( nChar )
		{
		case VK_LEFT:
		case VK_RIGHT:
			m_bItemNavigating = false;
			OnKeyDown(nChar, nRepCnt, nFlags);
			break;
		case VK_ESCAPE:
			m_bItemNavigating = false;
			Invalidate(FALSE);
			break;
		case VK_UP:
			if( pInfo->nSelected > 0 )
			{
				pInfo->nSelected--;
				RecalcItemPosition(m_nCurRow, m_nCurCol);
				Invalidate(FALSE);
			}
			break;
		case VK_DOWN:
			if( pInfo->nSelected < pInfo->nCount - 1 )
			{
				pInfo->nSelected++;
				RecalcItemPosition(m_nCurRow, m_nCurCol);
				Invalidate(FALSE);
			}
			break;
		case 33:
			nMaxView = (int)(pInfo->rcItem.Height() / CAL_ITEM_HEIGHT);
			if( pInfo->nSelected > 0 )
			{
				pInfo->nSelected -= nMaxView;
				if( pInfo->nSelected < 0 ) pInfo->nSelected = 0;
				RecalcItemPosition(m_nCurRow, m_nCurCol);
				Invalidate(FALSE);
			}
			break;
		case 34:
			nMaxView = (int)(pInfo->rcItem.Height() / CAL_ITEM_HEIGHT);
			if( pInfo->nSelected < pInfo->nCount - 1 )
			{
				pInfo->nSelected += nMaxView;
				if( pInfo->nSelected > pInfo->nCount - 1 ) pInfo->nSelected = pInfo->nCount - 1;
				RecalcItemPosition(m_nCurRow, m_nCurCol);
				Invalidate(FALSE);
			}
			break;
		case VK_DELETE:
			TRACE("VK_DELETE\n");
			//(CDataManager*)m_pData->OnMenuDeleteItem();
			break;
		default:
			//m_pData->OnItemKeyDown(nChar, nRepCnt, nFlags);
			break;
		}
	}
	AfxSetResourceHandle(hInstResourceMain);
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

// OnUmCNENotifyReturn: On User Message Calendar Number Edit Notify Detect Return KeyDown
// wParam : Edit Window Text -> Numeric Value
// lParam : WhatDoing
LRESULT CCalendarCtrl::OnUmCNENotifyReturn(WPARAM wParam, LPARAM lParam)
{
	if( (int)lParam == 1 )  // Year editing
	{
		m_kdDate.SetDate((int)wParam, m_kdDate.GetMonth(), m_kdDate.GetDay());
		SetRowColByDate();
		Invalidate(FALSE);
	}
	else if( (int)lParam == 2 )  // Month editing
	{
		m_kdDate.SetDate(m_kdDate.GetYear(), (int)wParam, m_kdDate.GetDay());
		SetRowColByDate();
		Invalidate(FALSE);
	}

	return TRUE;
}

// 현재 셀로 새 일자 구하기
void CCalendarCtrl::SetRowColByDate()
{
	m_nCurRow = m_kdDate.GetWeekNumber() - 1;
	m_nCurCol = m_kdDate.GetDayOfWeek() - 1;
	OnDateChanged();
}

// 현재 일자로 선택될 셀 구하기
void CCalendarCtrl::SetDateByRowCol()
{
	int nDiff;

	nDiff = (m_nCurRow * 7 + m_nCurCol) -
		( m_kdDate.GetWeekNumber() - 1) * 7 - (m_kdDate.GetDayOfWeek() - 1 );
	if( nDiff == 0 ) return;

	m_kdDate.DateAdd(0, 0, nDiff);
	if( m_kdDate.GetYear() <= 2 )  // 3년 보다 작으면 변경 취소
	{
		m_kdDate.DateAdd(0, 0, -nDiff);
		m_nCurRow = m_kdDate.GetWeekNumber() - 1;
		m_nCurCol = m_kdDate.GetDayOfWeek() - 1;
		return;
	}

	m_nCurRow = m_kdDate.GetWeekNumber() - 1;
	m_nCurCol = m_kdDate.GetDayOfWeek() - 1;
	OnDateChanged();
}

// 보색 구하기(예:흰색 -> 검정색)
COLORREF CCalendarCtrl::GetComplementaryColor(COLORREF cr)
{
	return RGB(255 - GetRValue(cr), 255 - GetGValue(cr), 255 - GetBValue(cr));
}

void CCalendarCtrl::OnDateChanged()
{

}


BOOL CCalendarCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	
	if( !m_bItemNavigating )
	{
		if( zDelta > 0 )
		{
			SendMessage(WM_KEYDOWN, (WPARAM)33);  // PageUp
		}
		else
		{
			SendMessage(WM_KEYDOWN, (WPARAM)34);  // PageDown
		}
	}
	else
	{
		if( zDelta > 0 )
		{
			SendMessage(WM_KEYDOWN, (WPARAM)33);  // PageUp
		}
		else
		{
			SendMessage(WM_KEYDOWN, (WPARAM)34);  // PageDown
		}
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CCalendarCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_bResized = true;
	if( m_wndEdit.IsWindowVisible() ) m_wndEdit.ShowWindow(SW_HIDE);
}


BOOL CCalendarCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);

	if( m_rcYear.PtInRect(pt) || m_rcMonth.PtInRect(pt) )
	{
		::SetCursor(::LoadCursor(NULL, IDC_UPARROW));
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CCalendarCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);
	OnLButtonUp(nFlags, point);
	
	CWnd::OnRButtonDown(nFlags, point);
}

void CCalendarCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	int nRow=-1, nCol=-1;

	GetRowColClicked(point, &nRow, &nCol);
	if( m_pData != NULL && nRow >= 0 && nCol >= 0 )
	{
		TCellInfo* pInfo = &m_arrCellInfo[nRow][nCol];
		if( pInfo->rcItem.PtInRect(point) )
		{
			ClientToScreen(&point);
			m_pData->OnItemContextmenu(m_kdDate, pInfo->nSelected, point);
		}
		else
		{
			ClientToScreen(&point);
			m_pData->OnDayContextMenu(m_kdDate, point);
		}
	}
	
	CWnd::OnRButtonUp(nFlags, point);
}

void CCalendarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int nRow=-1, nCol=-1;

	GetRowColClicked(point, &nRow, &nCol);

	if( nRow >= 0 && nCol >= 0 )
	{
		TCellInfo* pInfo = &m_arrCellInfo[nRow][nCol];
		if( pInfo->rcItem.PtInRect(point) )
		{
			m_pData->OnItemDblClick(m_kdDate, pInfo->nSelected);
		}
		else
		{
			m_pData->OnDayDblClick(m_kdDate);
		}
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CCalendarCtrl::OnItemInserted(CKoreanDate date)
{
	int nRow, nCol;
	TCellInfo* pInfo;

	nRow = date.GetWeekNumber() - 1;
	nCol = date.GetDayOfWeek() - 1;

	m_bItemNavigating = true;
	pInfo = &m_arrCellInfo[nRow][nCol];
	pInfo->nCount++;
	pInfo->nSelected = pInfo->nCount - 1;

	RecalcItemPosition(nRow, nCol);
	Invalidate(FALSE);
}

void CCalendarCtrl::OnItemUpdated(CKoreanDate date, int nIndex)
{
	Invalidate(FALSE);
}

void CCalendarCtrl::OnItemDeleted(CKoreanDate date, int nIndex)
{
	int nRow, nCol;
	TCellInfo* pInfo;

	nRow = date.GetWeekNumber() - 1;
	nCol = date.GetDayOfWeek() - 1;

	pInfo = &m_arrCellInfo[nRow][nCol];
	if( pInfo->nCount <= 0 ) return;

	pInfo->nCount--;
	if( pInfo->nCount == 0 )
	{
		pInfo->nSelected = -1;
		pInfo->rcScrollBar.SetRect(0, 0, 0, 0);
		pInfo->rcItem.SetRect(0, 0, 0, 0);
		m_bItemNavigating = false;
	}
	else if( pInfo->nSelected > pInfo->nCount - 1 )
	{
		pInfo->nSelected = pInfo->nCount - 1;
	}

	RecalcItemPosition(nRow, nCol);
	Invalidate(FALSE);
}



BOOL CCalendarCtrl::PreTranslateMessage(MSG* pMsg) 
{
    if(pMsg->message == WM_MOUSEMOVE && m_pData != NULL )
    {
		CPoint point(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		int nRow=-1, nCol=-1;

		GetRowColClicked(point, &nRow, &nCol);
		if( nRow == m_nCurRow && nCol == m_nCurCol )
		{
			CString strTip;
			CRect rcTip;
			bool bShowTip = false;
			TCellInfo* pInfo = &m_arrCellInfo[nRow][nCol];

			rcTip.CopyRect(pInfo->rcItem);
			if( !rcTip.IsRectEmpty() )
			{
				rcTip.bottom = rcTip.top - 1;
				rcTip.top = m_narrPosY[nRow];
			}
			else
			{
				GetCellRect(nRow, nCol, &rcTip);
			}

			if( !m_bItemNavigating && rcTip.PtInRect(point) )
			{
				strTip = m_pData->OnQueryDayTooltip(m_kdDate);
				bShowTip = true;
			}
			else
			{
				if( m_bItemNavigating && pInfo->nSelected >= 0 )
				{
					rcTip.CopyRect(pInfo->rcItem);
					rcTip.OffsetRect(0, (pInfo->nSelected - pInfo->nTop) * CAL_ITEM_HEIGHT);
					rcTip.bottom = rcTip.top + CAL_ITEM_HEIGHT;
					if( rcTip.PtInRect(point) )
					{
						TCellItem item;
						m_pData->GetItem(m_kdDate, pInfo->nSelected, &item);

						CDC* pDC = GetDC();
						CFont* pOldFont = pDC->SelectObject(&m_fontItem);
						CSize sz = pDC->GetTextExtent(item.strCaption);
						sz.cx += 1;
						pDC->SelectObject(pOldFont);

						rcTip.left += (CAL_ITEM_HEIGHT + 2);  // 아이콘 너비 빼주기
						if( sz.cx > rcTip.Width() )
						{
							strTip = item.strCaption;
							bShowTip = true;
						}
					}
				}
			}  // end of outer if( rcTip.PtInRect(point) )

			m_Tooltip.Activate(bShowTip);
			if( bShowTip )
			{
				m_Tooltip.UpdateTipText(strTip, this);
				m_Tooltip.RelayEvent(pMsg);
			}
		}  // end of if( nRow == m_nCurRow && nCol == m_nCurCol )
		else
		{
			m_Tooltip.Activate(false);
		}
    }  // if(pMsg->message == WM_MOUSEMOVE)
	
	return CWnd::PreTranslateMessage(pMsg);
}
