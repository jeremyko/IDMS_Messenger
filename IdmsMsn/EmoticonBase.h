#if !defined(AFX_EMOTICONBASE_H__1573DE49_98D1_4413_8ACD_F03178CB0EE2__INCLUDED_)
#define AFX_EMOTICONBASE_H__1573DE49_98D1_4413_8ACD_F03178CB0EE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmoticonBase.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmoticonBase dialog

class CEmoticonBase : public CDialog
{
// Construction
public:
	CEmoticonBase(CWnd* pParent = NULL);   // standard constructor
	CImageList m_imgListFaces ;
	
// Dialog Data
	//{{AFX_DATA(CEmoticonBase)
	enum { IDD = IDD_DLG_EMOTICON_BASE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmoticonBase)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CToolBar m_wndToolBar ;
	CBitmap m_bmpFaces;
	
	// Generated message map functions
	//{{AFX_MSG(CEmoticonBase)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMOTICONBASE_H__1573DE49_98D1_4413_8ACD_F03178CB0EE2__INCLUDED_)
