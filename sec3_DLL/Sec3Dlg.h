#if !defined(AFX_SEC3DLG_H__C4D89795_1286_4E18_B6E0_7B2C4CED51CC__INCLUDED_)
#define AFX_SEC3DLG_H__C4D89795_1286_4E18_B6E0_7B2C4CED51CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sec3Dlg.h : header file
//

#include "ControlPos.h"

#include "DataManager.h"
#include "CalendarCtrl.h"
#include "Tools/MenuXP.h"

/////////////////////////////////////////////////////////////////////////////
// CSec3Dlg dialog

class CSec3Dlg : public CDialog
{
// Construction
public:
	CSec3Dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSec3Dlg)
	enum { IDD = IDD_DIALOG_SEC3 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSec3Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
private:
	HWND m_CapWnd;
// Implementation
protected:

	CDataManager	m_DataManager;
	CCalendarCtrl	m_wndCalendar;
	CImageList		m_ilCalItem;
	
	void SetResize() ;
	
	CControlPos m_cControlPos;

	// Generated message map functions
	//{{AFX_MSG(CSec3Dlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnTest();
	afx_msg void OnClose();
	afx_msg void OnFileDeleteItem();
	afx_msg void OnFileEditItem();
	afx_msg void OnFileNewItem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEC3DLG_H__C4D89795_1286_4E18_B6E0_7B2C4CED51CC__INCLUDED_)
