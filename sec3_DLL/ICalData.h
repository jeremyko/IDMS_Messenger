#if !defined(_ICALDATA_H_INCLUDED_)
#define _ICALDATA_H_INCLUDED_

#include "KoreanDate.h"

typedef struct _tagCellItem {
	int		nImage;
	CString	strCaption;
} TCellItem;

// CCalendarCtrl 클래스와 데이터 제공자와의 독립성을 위해
// ICalData 인터페이스를 제공한다.
interface ICalData
{
	virtual long	FindFirstItem(CKoreanDate date, TCellItem* pItem) = 0;
	virtual bool	FindNextItem(TCellItem* pItem) = 0;
	virtual bool	GetItem(CKoreanDate date, int nIndex, TCellItem* pItem) = 0;
	virtual bool	MoveItem(CKoreanDate dateFrom, int nIndex, CKoreanDate dateTo) = 0;
	virtual void	OnDayContextMenu(CKoreanDate date, CPoint point) = 0;
	virtual void	OnItemContextmenu(CKoreanDate date, int nIndex, CPoint point) = 0;
	virtual void	OnDayDblClick(CKoreanDate date) = 0;
	virtual void	OnItemDblClick(CKoreanDate date, int nIndex) = 0;
	virtual CString	OnQueryDayTooltip(CKoreanDate date) = 0;
	virtual void	OnDayKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) = 0;
	virtual void	OnItemKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) = 0;

};

typedef interface ICalData	ICalData;

#endif  // _ICALDATA_H_INCLUDED_
