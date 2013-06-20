#if !defined(AFX_CHATWND_H__9458C6E5_8944_4646_8626_5C2C64F3EFE2__INCLUDED_)
#define AFX_CHATWND_H__9458C6E5_8944_4646_8626_5C2C64F3EFE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatWnd window
#include "dib256.h"	
#include"MyButton.h"
#include "ChatSession.h"
#include "..\\common\\ControlPos.h"	
#include "ConvEdit.h"
#include "EmoticonRichEditCtrl.h"
#include "..\\common\\AllCommonDefines.h"
#include "SplitterControl.h"
#include "EmoticonBase.h" //이모티콘
#include "AutoAppendMsgDlg.h"
#include "BuddySlipMsg.h"
#include "ChatReplyEditDlg.h"

class CChatWnd : public CWnd
{
// Construction
public:
	CChatWnd();

// Attributes
public:
    CMyButton*		m_pChkSelectSend;
	CMyButton*		m_pBtnEmoticon;
	CMyButton*		m_pBtnAddReplyList;
	CMyButton*		m_pBtnChgSepLine;
	CMyButton*		m_pBtnBuddyMsg;
	CMyButton*		m_pBtnSaveFile;
	CMyButton*		m_pBtnSendFile;
	CMyButton*		m_pChkSepLineUse;
	CMyButton*		m_pChkAlwaysOnTop;		
	CButton*		m_pStaticGroup1;
	CButton*		m_pStaticGroup2;
	CStatic *		m_pStaticReply;
	CComboBox*		m_pComboReply;
	CMyButton*		m_pBtnSnd;
	CEmoticonRichEditCtrl*	m_pChatREdit;
	CEmoticonRichEditCtrl*	m_pREditSendMsg;
	CSplitterControl* m_pWndSplitter; 	    
	
	char m_szToIP[20]; // 이 대화창의 대화상대 IP	
	char m_szMyIP[20]; // 이 대화창의 대화상대 IP	
	CBuddyInfo * m_pCompanion;
	CBuddyInfo * m_pMyInfo;			
	CFont   m_FontCtrl;	
	CWnd*	m_pParent;
	COLORREF m_ColorWndBg ;	

protected:
	CMapStringToString 	m_EmoStorageMap;  // 이모티콘 
	CEmoticonBase*      m_pEmoticonDialog; // 이모티콘
	CAutoAppendMsgDlg*  m_pAutoAppendMsgDlg;
	CBuddySlipMsg*      m_pBuddySlipMsgDlg ;
	CChatReplyEditDlg*  m_pChatReplyEditDlg;	
	IRichEditOle*       m_pRichEditOle; //이모티콘
	CControlPos         m_cControlPos;
	CString             m_strlastMsgDate;
	CStatusBar          m_statusBar;
		
	int m_nWidthChatREdit;
	int m_nWidthSendEdit, m_nHeightSendEdit;
	int m_nWidthReplyCombo, m_nHeightReplyCombo;
	int m_nTopMostFlag ;

	BOOL	m_bFirstMsgFlag;
	BOOL	m_bLastMsgCompanion;
	BOOL	m_bSelectSend;
	BOOL	m_bAutoAppendMsgUse;

	//CDIBitmap m_bmpBackground;		
	int			m_nType;		// see enum above
	CBrush		m_HollowBrush;

	enum {
		BITMAP_TOP = 0,
		BITMAP_TILE,		// tile the bitmap to fill the dialog
		BITMAP_STRETCH,			// stretch the bitmap so it fits to the dialog
		BITMAP_CENTER			// center the bitmap inside the dialog
	};

// Operations
public:
    
    void DoResize(int delta, BOOL bFromDefProc) ;// delta -> Y 변동값!!
	CChatWnd( CBuddyInfo * pCompanion, CBuddyInfo * pMyInfo ,CWnd* pParanrWnd );	
	void FileDropped(const char *FileName, int nLen, int nFileCnt);	
	BOOL CreateFromFile(LPCTSTR lpszFileName);

	void SetHan(BOOL bFlag);
	void SetResize();
	void SetReplyList();
	STDMETHODIMP GetSendText( /*CString& strMsg */ BSTR *pbsStr );	
	BOOL	SetBitmap(UINT uResource, int Type = CChatWnd::BITMAP_TOP);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChatWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChatWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);	
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	LRESULT OnEmoticonMap(WPARAM wParam, LPARAM lParam);
	
	LRESULT OnChatMsg(WPARAM wParam, LPARAM lParam);
	
	void OnButtonSendmsg() ;
	void OnConfig();
	//void OnColorMe() ;
	//void OnFont() ;
	//void OnColorBg();
	//void OnColorComp();
	void OnFilesave() ;
	void OnBtnSendFile() ;
	void OnBtnBuddyMsg() ;
	void OnBtnAddList() ;
	void OnBtnChgSepline() ;
	void OnSelchangeComboReply();
	void OnConvClose ();
	void OnAlwaysOntop() ;
	void OnCheckAlwaysTop();
	void OnBtnEmoticon() ;
	void OnFaceSelect(UINT nID);
	void OnServerClosed();
	void OnChatResume();
	void OnChatBuddyOffLine();	
	void FileDialogOpen(CString &result) ;
	
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATWND_H__9458C6E5_8944_4646_8626_5C2C64F3EFE2__INCLUDED_)
