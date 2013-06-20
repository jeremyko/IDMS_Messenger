#if !defined(AFX_MEMOITEMDLG_H__21CEF78B_7DEB_4609_93FB_E34459821A61__INCLUDED_)
#define AFX_MEMOITEMDLG_H__21CEF78B_7DEB_4609_93FB_E34459821A61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MemoItemDlg.h : header file
//

#include "ControlPos.h"
#include "ButtonIconSelect.h"
//#include "btnst.h"
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CMemoItemDlg dialog

class CMemoItemDlg : public CDialog
{
// Construction
public:
	bool	m_bNewItem;
	CString	m_strDlgTitle;
	CString strAlarmTimeYYYYMMDDHHMI;
	CString m_strAlarmTimeYYYY;
	CString m_strAlarmTimeMonth;
	CString m_strAlarmTimeDD;
	CString m_strAlarmTimeHH;
	CString m_strAlarmTimeMM;
	int		GetIconIndex();
	CString GetTitle();
	CString	GetContent();
	void	SetIconIndex(const int nIndex);
	void	SetTitle(const CString strTitle);
	void	SetContent(const CString strContent);
	void	SetImageList(CImageList* pImageList);

	CMemoItemDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMemoItemDlg)
	enum { IDD = IDD_MEMOITEM };
	CDateTimeCtrl	m_DateCtrl;
	CDateTimeCtrl	m_DateTimeCtrl;
	CButtonIconSelect	m_btnIcon;
	BOOL m_bUseAlarm;	
	CTime	m_DTPickTime;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemoItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImageList*		m_pImageList;
	int				m_nIconIndex;
	CString			m_strTitle;
	CString			m_strContent;
	int m_nAlarmChked ;

	void SetResize() ;
	
	CControlPos m_cControlPos;

	// Generated message map functions
	//{{AFX_MSG(CMemoItemDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCheckAlarm();
	afx_msg void OnClose();
	
	//}}AFX_MSG
	LRESULT OnIconCanceled(WPARAM wParam, LPARAM lParam);
	LRESULT OnIconSelected(WPARAM wParam, LPARAM lParam);	

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMOITEMDLG_H__21CEF78B_7DEB_4609_93FB_E34459821A61__INCLUDED_)
