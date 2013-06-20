#if !defined(AFX_SEC2DLG_H__49E236F9_2A63_473A_BFBD_311D8A8C989C__INCLUDED_)
#define AFX_SEC2DLG_H__49E236F9_2A63_473A_BFBD_311D8A8C989C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sec2Dlg.h : header file
//

#include "..\\common\\ControlPos.h"
#include "MultilineList.h"
/////////////////////////////////////////////////////////////////////////////
// CSec2Dlg dialog

class CSec2Dlg : public CDialog
{
// Construction
public:
	CSec2Dlg(CWnd* pParent = NULL);   // standard constructor
	void SetResize() ;
// Dialog Data
	//{{AFX_DATA(CSec2Dlg)
	enum { IDD = IDD_DLG_SEC2 };
	CMultilineList	m_LunchList;
	CTime	m_TmCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSec2Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CControlPos m_cControlPos;	
	
	// Generated message map functions
	//{{AFX_MSG(CSec2Dlg)
	afx_msg void OnButton1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	//Section
	afx_msg LRESULT OnMainResponse(WPARAM wParam, LPARAM lParam); 

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEC2DLG_H__49E236F9_2A63_473A_BFBD_311D8A8C989C__INCLUDED_)
