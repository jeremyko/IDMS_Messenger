#if !defined(AFX_CALENDARCTRL_H__583FD8CF_D684_4010_9178_21025BD781B3__INCLUDED_)
#define AFX_CALENDARCTRL_H__583FD8CF_D684_4010_9178_21025BD781B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CalendarCtrl.h : header file
//

#include "MemDC.h"
#include "ICalData.h"
#include "KoreanDate.h"
#include "CalNumberEdit.h"
//#include "TrackLookButton.h"

#define CALENDARCTRL_CLASSNAME  _T("CalendarCtrl")

#define CALBS_NONE     0
#define CALBS_SINGLE   1
#define CALBS_RAISED   EDGE_RAISED
#define CALBS_ETCHED   EDGE_ETCHED
#define CALBS_SUNKEN   EDGE_SUNKEN
#define CALBS_BUMP     EDGE_BUMP

#define IDC_CAL_EDITCTRL	1    // 년월 편집 에디트 컨트롤 아이디
#define	IDC_CAL_TODAYBTN	2    // 오늘 날짜 단추 컨트롤 아이디

#define CAL_ITEM_HEIGHT		16

/////////////////////////////////////////////////////////////////////////////
// CCalendarCtrl window


class CCalendarCtrl : public CWnd
{
	DECLARE_DYNAMIC(CCalendarCtrl)

// Construction
public:
	CCalendarCtrl();

	enum EWeekDay
	{
		WD_SUN = 1,
		WD_MON,
		WD_TUE,
		WD_WED,
		WD_THU,
		WD_FRI,
		WD_SAT
	};


// Attributes
public:
	void		SetDataInterface(ICalData* picd);
	COLORREF	GetCellLineColor();
	void		SetCellLineColor(COLORREF crCellLine);
	COLORREF	GetBorderLineColor();
	void		SetBorderLineColor(COLORREF crBorderLine);
	CString		GetWeekdayName(EWeekDay wd);
	void		SetWeekdayName(EWeekDay wd, CString strName);
	BOOL		SetBorderStyle(DWORD dwStyle);
	void		SetSmallIcon(HICON hIcon);
	void		SetImageList(CImageList* pImageList);
	CImageList*	GetImageList();
	CKoreanDate GetDate() const;
	int	GetSelectedItemIndex();

// Operations
public:
	void	OnItemInserted(CKoreanDate date);
	void	OnItemUpdated(CKoreanDate date, int nIndex);
	void	OnItemDeleted(CKoreanDate date, int nIndex);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCalendarCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCalendarCtrl();

	// Generated message map functions
	
private:
	typedef struct _tagCellInfo
	{
		int		nCount;
		bool	bScrollBar;   // 스크롤바 보여주는지 여부
		CRect	rcScrollBar;  // 스크롤바 위치
		CRect	rcItem;       // 아이템창 위치
		int		nTop;         // 가장 위에 보이는 아이템 인덱스
		int		nSelected;    // 현재 선택된 아이템 인덱스

		void	Clear()
		{
			nCount = 0;
			bScrollBar = false;
			rcScrollBar.SetRect(0, 0, 0, 0);
			rcItem.SetRect(0, 0, 0, 0);
			nTop = 0;
			nSelected = -1;
		}
	} TCellInfo;

	BOOL		GetCellRect(int nRow, int nCol, LPRECT lpRect);  // 0 base
	void		SetRowColByDate();
	void		SetDateByRowCol();
	COLORREF	GetComplementaryColor(COLORREF cr);              // 보색 구하기
	void		GetRowColClicked(CPoint point, int* pnRow, int* pnCol);
	void		RecalcItemPosition(int nRow, int nCol, bool bResized = false);

	int			m_narrPosX[8];          // 셀 가로좌표
	int			m_narrPosY[7];          // 셀 세로좌표
	int			m_nTitleHeight;         // 타이틀 높이
	int			m_nHeaderHeight;        // 헤더 높이
	TCellInfo	m_arrCellInfo[6][7];    // 각 셀의 아이템 정보
	bool		m_bNewMonth;            // 년월이 바뀌었는지 여부
	bool		m_bResized;             // 윈도 크기 변경에 의한 Redraw
	bool		m_bItemNavigating;      // 아이템 선택 중인지 여부
	bool		m_bToolTipShowed;
	int			m_nWeekCount;           // 현재 월의 week 개수
	int			m_nCurYear;             // 현재 년도
	int			m_nCurMonth;            // 현재 월

	//const int	m_nItemHeight;

protected:
	BOOL		RegisterWindowClass();

	// Drawing
	void		DrawTitle(CDC* pDC, const CRect rcClip);     // Draw title part
	void		DrawHeader(CDC* pDC, const CRect rcClip);    // Draw header part
	void		DrawCellLine(CDC* pDC);                      // Draw cell lines
	void		DrawCell(CDC* pDC);                          // Draw each cell
	void		DrawCellItems(int nRow, int nCol,            // Draw items in a cell
		                      CDC* pDC, const CRect rcClip,  
		                      CKoreanDate* pDate); 
	void		ShowYearEdit();
	void		ShowMonthEdit();
	void		OnSetToday();
	virtual void OnDateChanged();

	ICalData*	m_pData;              // 캘린더 데이터 인터페이스
	HICON		m_hIcon;              // 타이틀 바에 사용될 아이콘
	CImageList* m_pImageList;         // 항목 표시에 사용할 이미지 리스트
	CKoreanDate m_kdDate;             // 일자
	CString		m_strWeekdayName[7];  // 요일명
	CFont		m_fontTitle;          // 타이틀 폰트
	CFont		m_fontHeader;         // 요일명 폰트
	CFont		m_fontDaySolar;       // 양력 일자 폰트
	CFont		m_fontDayLunar;       // 음력 일자 폰트
	CFont		m_fontDayName;        // 일자명 폰트
	CFont		m_fontItem;           // 아이템 폰트
	DWORD		m_dwBorderStyle;      // 테두리 모양

	COLORREF	m_crBorderLine;       // 테두리 선 색
	COLORREF	m_crCellLine;         // 셀 라인 색
	COLORREF	m_crNormalDay;        // 평일 일자 색
	COLORREF	m_crHoliday;          // 공휴일 일자 색
	COLORREF	m_crSaturday;         // 토요일 일자 색
	COLORREF	m_crNormalBack;       // 현재 월 셀의 배경색
	COLORREF	m_crNoneBack;         // 핸재 월이 아닌 셀의 배경색

	CToolTipCtrl	m_Tooltip;
	CCalNumberEdit	m_wndEdit;        // 년월 편집을 위한 에디트박스
	CRect		m_rcYear;             // 타이틀바의 년 위치
	CRect		m_rcMonth;            // 타이틀바의 월 위치
	CRect		m_rcTodayBtn;         // 오늘 날짜 버튼 위치

	bool		m_bHasFocus;          // 현재 포커스를 가지는지 여부
	bool		m_bMouseDown;         // 마우스 왼쪽 버튼 눌려졌는지 여부
	bool		m_bMouseOnTodayBtn;   // 마우스 포인터가 오늘 날짜 단추 위에 있는지 여부
	int			m_nCurRow;            // 활성 셀의 줄 번호
	int			m_nCurCol;            // 활성 셀의 칼럼 번호

	//{{AFX_MSG(CCalendarCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	LRESULT OnUmCNENotifyReturn(WPARAM wParam, LPARAM lParam);  // CCalNumberEdit 에서 엔터키 눌렀을 때
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALENDARCTRL_H__583FD8CF_D684_4010_9178_21025BD781B3__INCLUDED_)
