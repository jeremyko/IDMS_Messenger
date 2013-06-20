/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Juraj Rojko jrojko@twist.cz
// All rights reserved
//
#if !defined(AFX_TWSCRIPTEDIT_H__82F5E419_1C74_11D1_87FA_00403395B157__INCLUDED_)
#define AFX_TWSCRIPTEDIT_H__82F5E419_1C74_11D1_87FA_00403395B157__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TWScriptEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmoticonRichEditCtrl window

class CEmoticonRichEditCtrl : public CRichEditCtrl
{
// Construction 
public:
	CEmoticonRichEditCtrl();

	void Initialize() ;
// Attributes
public:
	int m_nParentKind;			
	void AddEmoticon(LPCTSTR lpszKwd);
	void SetParentName(LPCTSTR lpszParentTitle);
	LPCTSTR GetParentName();
	HBITMAP GetImage(CImageList& list, int num) ; //
		
// Operations
public:
	//void SetMeTextColor(COLORREF colorMe);	
	void FormatAll();
	void SetHan(BOOL bFlag);
	void setUrlDetection(BOOL bUse);
	void SetRole(int nRole)
	{
		m_nRole = nRole ;
	}

		

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmoticonRichEditCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEmoticonRichEditCtrl();

	// Generated message map functions
protected:
	int IsEmoticon(LPCTSTR lpszSymbol);	
	void SetFormatRange(int nStart, int nEnd, BOOL bBold, COLORREF clr);
	void FormatTextRange(int nStart, int nEnd);
	void FormatTextLines(int nStart, int nEnd);
	void ChangeCase(int nStart, int nEnd, LPCTSTR lpszStr);
	
	enum ChangeType {ctUndo, ctUnknown, ctReplSel, ctDelete, ctBack, ctCut, ctPaste, ctMove};
	

	CString m_strEmoticon;
	int m_nRole ;
	CBitmap m_bmpFaces;
	char m_szParentName[255];
	//COLORREF m_ColorMe ; 
	BOOL m_bInForcedChange;
	ChangeType m_changeType;
	CHARRANGE m_crOldSel;
	
	void Enable(BOOL);
	void FileFindRecurse(LPCTSTR pstr);
	
	CString m_strFileAll;
	int     m_nFileCnt; // 선택한 파일이 몇개인가 
	
	
	IRichEditOle	*m_pRichEditOle; //이모티콘
	CImageList m_imgListFaces ;

	//{{AFX_MSG(CEmoticonRichEditCtrl)
	afx_msg void OnChange();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);	
	//}}AFX_MSG
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnProtected(NMHDR*, LRESULT* pResult);
	afx_msg void OnSelChange(NMHDR*, LRESULT* pResult);
	afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg LRESULT OnSetFocus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKillFocus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);	
    afx_msg void whenLinkClick( NMHDR * pNotifyStruct, LRESULT * result );
	//afx_msg void OnFaceSelect (UINT nID);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TWSCRIPTEDIT_H__82F5E419_1C74_11D1_87FA_00403395B157__INCLUDED_)
