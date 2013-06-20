#ifndef _DATA_MANAGER_H_INCLUDED_
#define _DATA_MANAGER_H_INCLUDED_

// DataManager.h: interface for the CDataManager class.
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ICalData.h"
#include "CalendarCtrl.h"

class CDataManager : public ICalData
{
public:
	CDataManager();
	virtual ~CDataManager();

	void	SetOwnerWnd(CWnd* pWnd);
	void	SetCalendarCtrl(CCalendarCtrl* pCtrl);
	void	SetImageList(CImageList* pImageList);
	bool	OpenDocument(const CString strFilename);
	bool	SaveDocument();
	bool	IsDocumentOpend();
	void	OnMenuNewItem();
	void	OnMenuEditItem();
	void	OnMenuDeleteItem();

	// ICalData interface
	virtual long	FindFirstItem(CKoreanDate date, TCellItem* pItem);
	virtual bool	FindNextItem(TCellItem* pItem);
	virtual bool	GetItem(CKoreanDate date, int nIndex, TCellItem* pItem);
	virtual bool	MoveItem(CKoreanDate dateFrom, int nIndex, CKoreanDate dateTo);
	virtual void	OnDayContextMenu(CKoreanDate date, CPoint point);
	virtual void	OnItemContextmenu(CKoreanDate date, int nIndex, CPoint point);
	virtual void	OnDayDblClick(CKoreanDate date);
	virtual void	OnItemDblClick(CKoreanDate date, int nIndex);
	virtual CString	OnQueryDayTooltip(CKoreanDate date);
	virtual void	OnDayKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void	OnItemKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
	typedef struct _tagItemData
	{
		_tagItemData()
		{
			strAlarmTimeYYYYMMDDHHMI = "";
			strTitle = "";
			strContent= "";
			strAlarmTimeHH= "";
			strAlarmTimeMM= "";		
			
			m_strAlarmTimeYYYY= "";
			m_strAlarmTimeMonth= "";
			m_strAlarmTimeDD= "";
		};

		int nIcon;
		CString strTitle;
		CString strContent;
		CString strAlarmTimeHH;
		CString strAlarmTimeMM;
		
		CString strAlarmTimeYYYYMMDDHHMI;
		CString m_strAlarmTimeYYYY;
		CString m_strAlarmTimeMonth;
		CString m_strAlarmTimeDD;

	} TItemData;

	CWnd*				m_pOwnerWnd;
	CCalendarCtrl*		m_pCalendarCtrl;
	MSXML2::IXMLDOMDocument2Ptr	m_pDoc;
	MSXML2::IXMLDOMNodeListPtr  m_pNodeList;
	CImageList*			m_pImageList;
	int					m_nIndex;
	CString				m_strFilename;
	bool				m_bDocOpened;
	
	bool	IsValidFile(LPCTSTR lpszFilename);
	bool	MakeEmptyFile(CString strFilename);
	bool	GetItemData(CKoreanDate date, int nIndex, TItemData* pData);
	bool	InsertItem(CKoreanDate date, TItemData* pData);
	bool	UpdateItem(CKoreanDate date, int nIndex, TItemData* pData);
	bool	DeleteItem(CKoreanDate date, int nIndex);

	//bool	CopyFileFromResourceToSystemFolder(int nIDResource, CString strFilename);
	bool	RegisterXMLDll();
};

#endif  // _DATA_MANAGER_H_INCLUDED_