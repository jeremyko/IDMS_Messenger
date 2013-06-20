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

#define IDC_CAL_EDITCTRL	1    // ��� ���� ����Ʈ ��Ʈ�� ���̵�
#define	IDC_CAL_TODAYBTN	2    // ���� ��¥ ���� ��Ʈ�� ���̵�

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
		bool	bScrollBar;   // ��ũ�ѹ� �����ִ��� ����
		CRect	rcScrollBar;  // ��ũ�ѹ� ��ġ
		CRect	rcItem;       // ������â ��ġ
		int		nTop;         // ���� ���� ���̴� ������ �ε���
		int		nSelected;    // ���� ���õ� ������ �ε���

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
	COLORREF	GetComplementaryColor(COLORREF cr);              // ���� ���ϱ�
	void		GetRowColClicked(CPoint point, int* pnRow, int* pnCol);
	void		RecalcItemPosition(int nRow, int nCol, bool bResized = false);

	int			m_narrPosX[8];          // �� ������ǥ
	int			m_narrPosY[7];          // �� ������ǥ
	int			m_nTitleHeight;         // Ÿ��Ʋ ����
	int			m_nHeaderHeight;        // ��� ����
	TCellInfo	m_arrCellInfo[6][7];    // �� ���� ������ ����
	bool		m_bNewMonth;            // ����� �ٲ������ ����
	bool		m_bResized;             // ���� ũ�� ���濡 ���� Redraw
	bool		m_bItemNavigating;      // ������ ���� ������ ����
	bool		m_bToolTipShowed;
	int			m_nWeekCount;           // ���� ���� week ����
	int			m_nCurYear;             // ���� �⵵
	int			m_nCurMonth;            // ���� ��

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

	ICalData*	m_pData;              // Ķ���� ������ �������̽�
	HICON		m_hIcon;              // Ÿ��Ʋ �ٿ� ���� ������
	CImageList* m_pImageList;         // �׸� ǥ�ÿ� ����� �̹��� ����Ʈ
	CKoreanDate m_kdDate;             // ����
	CString		m_strWeekdayName[7];  // ���ϸ�
	CFont		m_fontTitle;          // Ÿ��Ʋ ��Ʈ
	CFont		m_fontHeader;         // ���ϸ� ��Ʈ
	CFont		m_fontDaySolar;       // ��� ���� ��Ʈ
	CFont		m_fontDayLunar;       // ���� ���� ��Ʈ
	CFont		m_fontDayName;        // ���ڸ� ��Ʈ
	CFont		m_fontItem;           // ������ ��Ʈ
	DWORD		m_dwBorderStyle;      // �׵θ� ���

	COLORREF	m_crBorderLine;       // �׵θ� �� ��
	COLORREF	m_crCellLine;         // �� ���� ��
	COLORREF	m_crNormalDay;        // ���� ���� ��
	COLORREF	m_crHoliday;          // ������ ���� ��
	COLORREF	m_crSaturday;         // ����� ���� ��
	COLORREF	m_crNormalBack;       // ���� �� ���� ����
	COLORREF	m_crNoneBack;         // ���� ���� �ƴ� ���� ����

	CToolTipCtrl	m_Tooltip;
	CCalNumberEdit	m_wndEdit;        // ��� ������ ���� ����Ʈ�ڽ�
	CRect		m_rcYear;             // Ÿ��Ʋ���� �� ��ġ
	CRect		m_rcMonth;            // Ÿ��Ʋ���� �� ��ġ
	CRect		m_rcTodayBtn;         // ���� ��¥ ��ư ��ġ

	bool		m_bHasFocus;          // ���� ��Ŀ���� �������� ����
	bool		m_bMouseDown;         // ���콺 ���� ��ư ���������� ����
	bool		m_bMouseOnTodayBtn;   // ���콺 �����Ͱ� ���� ��¥ ���� ���� �ִ��� ����
	int			m_nCurRow;            // Ȱ�� ���� �� ��ȣ
	int			m_nCurCol;            // Ȱ�� ���� Į�� ��ȣ

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
	LRESULT OnUmCNENotifyReturn(WPARAM wParam, LPARAM lParam);  // CCalNumberEdit ���� ����Ű ������ ��
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALENDARCTRL_H__583FD8CF_D684_4010_9178_21025BD781B3__INCLUDED_)
