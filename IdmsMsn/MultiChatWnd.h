#if !defined(AFX_MULTICHATWND_H__7DEFE39A_3BC7_47D7_B078_007ACE5A7205__INCLUDED_)
#define AFX_MULTICHATWND_H__7DEFE39A_3BC7_47D7_B078_007ACE5A7205__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiChatWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiChatWnd window


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
#include <set>
#include <map>

typedef set <string, less<string> > SET_STR; 

class CMultiChatWnd : public CWnd
{
// Construction
public:
	CMultiChatWnd();

	// 내가 요청하는경우
	CMultiChatWnd(	CWnd* pParent, char* szDlgId, CStringArray& AryID, 
						CStringArray& AryName	);
	
	// 요청 받은 경우
	CMultiChatWnd( CWnd* pParent, const char* szServerRoomNo);
	
	void Init();
// Attributes
public:
    CMyButton*		m_pChkSelectSend;
	CMyButton*		m_pBtnEmoticon;
	CMyButton*		m_pBtnAddReplyList;
	CMyButton*		m_pBtnChgSepLine;	
	CMyButton*		m_pBtnSaveFile;
	CMyButton*		m_pBtnSendFile;
	CMyButton*		m_pChkSepLineUse;
	CButton*        m_pChkExitMultiChat;
	CMyButton*		m_pChkAlwaysOnTop;		
	CButton*		m_pStaticGroup1;
	CButton*		m_pStaticGroup2;
	CStatic *		m_pStaticReply;
	CComboBox*		m_pComboReply;
	CMyButton*		m_pBtnSnd;
	CEmoticonRichEditCtrl*	m_pChatREdit;
	CEmoticonRichEditCtrl*	m_pREditSendMsg;
	CSplitterControl* m_pWndSplitter; 	    
	CListCtrl*	m_pCListCtrlMultiChatID;
	
	char m_szToIP[20]; // 이 대화창의 대화상대 IP	
	char m_szMyIP[20]; // 이 대화창의 대화상대 IP	
	//CBuddyInfo * m_pCompanion;
	//CBuddyInfo * m_pMyInfo;			
	CFont   m_FontCtrl;	
	CStatusBar          m_statusBar;
	
	CStringArray	m_StringAryID ;		// Multi chat 대화상대 ID 	
	CStringArray	m_StringAryName ;	// Multi chat 대화상대 Name 		
	char			m_szHostID[20+1];	// MultiChat Host ID	
	char			m_szDlgID [15+1];   // 각 화면당 고유 아이디	
	SET_STR				m_setID; // 이 대화방이 관리하는 ID
	SET_STR::iterator	m_SetIter ; 
	CString				m_strIDNames;
	BOOL				m_bHostClosed ;	
	

protected:
	CMapStringToString 	m_EmoStorageMap;  // 이모티콘 
	CEmoticonBase*      m_pEmoticonDialog; // 이모티콘
	CAutoAppendMsgDlg*  m_pAutoAppendMsgDlg;
	CBuddySlipMsg*      m_pBuddySlipMsgDlg ;
	CChatReplyEditDlg*  m_pChatReplyEditDlg;	
	IRichEditOle*       m_pRichEditOle; //이모티콘
	CControlPos         m_cControlPos;
	CString             m_strlastMsgDate;
	COLORREF m_ColorWndBg;
		
	int m_nWidthChatREdit;
	int m_nWidthSendEdit, m_nHeightSendEdit;
	int m_nWidthReplyCombo, m_nHeightReplyCombo;
	int m_nTopMostFlag ;

	BOOL	m_bSelectSend;
	BOOL	m_bAutoAppendMsgUse;
	BOOL	m_bLastMsgCompanion;

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
    
    void DoResize(int delta) ;// delta -> Y 변동값!!
	CMultiChatWnd( CBuddyInfo * pCompanion, CBuddyInfo * pMyInfo);	
	void FileDropped(const char *FileName, int nLen, int nFileCnt);	
	BOOL CreateFromFile(LPCTSTR lpszFileName);

	void SetHan(BOOL bFlag);
	void SetResize();
	void SetReplyList();
	STDMETHODIMP GetSendText(BSTR *pbsStr );
	BOOL	SetBitmap(UINT uResource, int Type = CMultiChatWnd::BITMAP_TOP);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiChatWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMultiChatWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMultiChatWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	//}}AFX_MSG
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	LRESULT OnEmoticonMap(WPARAM wParam, LPARAM lParam);
	
	LRESULT OnChatMsg(WPARAM wParam, LPARAM lParam);
	
	void OnButtonSendmsg() ;
	void OnConfig();	
	void OnFilesave() ;
	void OnBtnSendFile() ;	
	void OnBtnAddList() ;
	void OnBtnChgSepline() ;
	void OnSelchangeComboReply();
	void OnConvClose ();
	void OnAlwaysOntop() ;
	void OnCheckAlwaysTop();
	void OnBtnEmoticon() ;
	void OnFaceSelect(UINT nID);
	void OnServerClosed();
	void OnChatResume(WPARAM wParam, LPARAM lParam);
	void OnChatBuddyOffLine(WPARAM wParam, LPARAM lParam);	
	void FileDialogOpen(CString &result) ;
	void OnExitMultiChat(WPARAM wParam, LPARAM lParam);	
	
	DECLARE_MESSAGE_MAP()
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTICHATWND_H__7DEFE39A_3BC7_47D7_B078_007ACE5A7205__INCLUDED_)
