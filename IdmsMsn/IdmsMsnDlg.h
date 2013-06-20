 // IdmsMsnDlg.h : header file
//

#if !defined(AFX_IdmsMsnDLG_H__49AD9506_12ED_48B5_B248_417B8979B06A__INCLUDED_)
#define AFX_IdmsMsnDLG_H__49AD9506_12ED_48B5_B248_417B8979B06A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "Resource.h"
#include "AllMsgDlg.h"
#include"ViewAllMsgDlg.h"
#include "ChatSession.h"
#include "ExtTreeCtrl.h"
#include "..\\common\\ControlPos.h"	
#include "..\\common\\utility.h"

#include"MyButton.h"
#include"ColorStatic.h"
//#include "XMLDataManager.h"

#include <set>
#include <map>

#import <msxml4.dll>
using namespace MSXML2;

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnDlg dialog

typedef set <string, less<string> > SET_STR; 
//typedef map< string, set <string>, less<string> > MAP_STR2SET; 

class CIdmsMsnDlg : public CDialog //CDialog
{
// Construction
public:
	CIdmsMsnDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIdmsMsnDlg)
	enum { IDD = IDD_IDMSMSN_DIALOG };
	CMyButton	m_btnSec3;
	CMyButton	m_btnSec2;
	CMyButton	m_btnSec1;
	CColorStatic	m_staNick;
	CMyButton	m_btnMyStatus;
	CExtTreeCtrl	m_TreeCtrl;
	CString		m_strMyNick;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIdmsMsnDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
	
protected:
	int m_nMyStatus;
	CToolTipCtrl m_tooltip;
	HMODULE m_hModuleSec2, m_hModuleSec3 ;
	CDialog* m_pSec2Dlg;
	CDialog* m_pSec3Dlg;

	COLORREF	m_ColorBG;
	COLORREF	m_ColorGrp;
	COLORREF	m_ColorNode;

	//MAP_STR2SET m_mapStr2SetMultiChat;
	int			m_MultiChatDlgCnt;
	//int			m_AwaySec;
	BOOL		m_bHook;
	long		m_MeStatusFlags;
	CStatusBar	m_statusBar;
	CControlPos m_cControlPos;	
	int         m_nCntReConnect;
	BOOL        m_bConnectTrying ; ///< 이미 재접속 시도중이면 TRUE, 아니면 FALSE
	BOOL        m_bReConnectFlag; ///< 재접속하는 경우인지를 알기 위한 Flag

	HICON m_hIcon;
	HBITMAP m_hOnLineBmp;
	HBITMAP m_hBusyBmp;
	HBITMAP m_hAwayBmp;
	HBITMAP m_hOffLineBmp;
	HBITMAP m_hBlockBmp;
	HBITMAP m_hGrpOpen;
	HBITMAP m_hGrpClose;
	HBITMAP m_hITComp;
		
	HTREEITEM m_hContactList;
	HTREEITEM m_hGrpDflt;
	HTREEITEM m_hGrp;
	
	CMapStringToOb	mStrSlipMsgDlgMap;			// 쪽지보기 Dialog MAP	
	CMapStringToOb	mStrFileTransferDlgMap;	// File 전송용 Dialog MAP	
	CMapStringToOb	mStrMultiChatDlgMap;	// Multi chat Dlg 관리 
	
	CString			m_strMyIP;
	CAllMsgDlg*		m_pAllMSgSendDlg;
	CViewAllMsgDlg* m_pAllMSgViewDlg;	
			
	MSXML2::IXMLDOMDocument2Ptr	m_pDoc;
	MSXML2::IXMLDOMNodeListPtr  m_pNodeList;	//Function
	bool m_bDocOpened;

	bool OpenAlarmDoc(const CString strFilename);
	bool IsValidFile(LPCTSTR lpszFilename);
	bool RegisterXMLDll();
	void DoAlarmWork() ;
	bool MakeEmptyFile(CString strFilename);

	HTREEITEM* InsertGrpNode( char* szGrpName);	
	
	
	HTREEITEM m_hEternalItem;
	char m_szMyIPAddr[20];	
		
	// Generated message map functions
	//{{AFX_MSG(CIdmsMsnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();		
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemexpandedTreectrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTreectrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseMsger();
	afx_msg void OnAddpartner();
	afx_msg void OnClickTreectrl(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnGrpMngAddcomp();
	afx_msg void OnToolsAbout();
	afx_msg void OnGrpMngAdd();
	afx_msg void OnGrpMngDel();
	afx_msg void OnDelete();
	afx_msg void OnGrpChg();
	afx_msg void OnGrpMngModify();
	afx_msg void OnGrpMngSendfile();
	afx_msg void OnAlwaysOntop();
	afx_msg void OnToolsOption();
	afx_msg void OnGrpMngAllmsg();
	afx_msg void OnTraymenuRestore();
	afx_msg void OnQuit();
	afx_msg void OnViewChatHist();
	afx_msg void OnViewAllmsgHist();
	afx_msg void OnSendfile();
	afx_msg void OnAllmsg();
	afx_msg void OnBroadcastMsg();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMultichat();	
	afx_msg void OnWriteMsg();
	afx_msg void OnViewSlipmsgLog();
	afx_msg void OnGrpMngAddItcomp();
	afx_msg void OnItcompAdd();
	afx_msg void OnItcompDel();
	afx_msg void OnItcompModify();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBtnSec1();
	afx_msg void OnBtnSec2();
	afx_msg void OnBtnSec3();
	afx_msg void OnRunIdms();
	afx_msg void OnTraymenuIdms();
	afx_msg void OnRunMdb();
	afx_msg void OnRunSockTest();
	afx_msg void OnRunTuxTest();
	afx_msg void OnTraymenuMdb();
	afx_msg void OnTraymenuSocktest();
	afx_msg void OnTraymenuTuxtest();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnRunRsBoard();
	//}}AFX_MSG

	
	afx_msg LRESULT OnServerClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitWork(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBuddyListRslt(WPARAM wParam, LPARAM lParam);			
	afx_msg LRESULT OnAddBuddyRslt(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnSomeOneAddMe(WPARAM wParam, LPARAM lParam);			
	afx_msg LRESULT OnBuddyGrpRslt(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnAddGrpRslt(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnDelGrpRslt(WPARAM wParam, LPARAM lParam);			
	afx_msg LRESULT OnChgBuddyGrpRslt(WPARAM wParam, LPARAM lParam);				
	afx_msg LRESULT OnChgGrpNameRslt(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnSQLRsltErr(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnDelBuddyRslt(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnDelITBuddyRslt(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnSomeOneDelMe(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnBuddyOnline(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnBuddyOffline(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnCancelFileTransfer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrayShow(WPARAM wParam, LPARAM lParam) ;
	afx_msg LRESULT OnAllMsg(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnViewAllMsgDlgClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChatDlgClose(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnSlipMsgDlgClose(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnAllMsgDlgClose(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnAllMsgAck(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnChngNick(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnAwayTrue(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnAwayFalse(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnStatusBusy(WPARAM wParam, LPARAM lParam);			
	afx_msg LRESULT OnChatMsg(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnMChatMsg(WPARAM wParam, LPARAM lParam);			
	afx_msg LRESULT OnSearchID(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllowedComp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnYouAllowedBy(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMultiChatDlgClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMChatServerRoomNo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExitMChatMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSlipMsgSaveSuccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSlipMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddItComp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChgItComp(WPARAM wParam, LPARAM lParam);
	
	afx_msg LRESULT OnFoodMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllUserIdMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllUserIdForAdd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAllMsgAcked(WPARAM wParam, LPARAM lParam);
	
	afx_msg LRESULT OnALive(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnConfigChg(WPARAM wParam, LPARAM lParam);	
	
		
	
	// file transfer	
	afx_msg LRESULT OnSomeOneWantsSendFile (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFolderSelecting (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSendThisInformedFile (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisconnected (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileSendDlgClosed (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileRecvDlgClosed (WPARAM wParam, LPARAM lParam);
	

	//Section
	afx_msg LRESULT OnSectionReq(WPARAM wParam, LPARAM lParam); 

	afx_msg LRESULT OnPowerBroadcast(WPARAM dwPowerEvent, LPARAM dwData) ;
	
	
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()
public:

	CMapStringToOb	mStrChatDlgMap;			// 대화창 Dialog MAP	
	CMapStringToOb mStrFileSendDlgMap;		// File Send Dialog MAP
		
	CObList m_SockList;
	//BOOL    m_bNickChg ;
	//char    m_szNick [C_NICK_NAME_LEN +1 ];	

public:
	BOOL InitFileTransferSocket() ;//소켓 초기화 및 대기
	void doTerminateWork();
	void chgCompGrp(const char* szID, const char* szChgGrpName, const int nAway);
	BOOL chkDupGrp(const char* szGrpName);
	void DeleteTreeItemByID(const char* szID);	
	CString getFileNameOnly(CString strFileWithPath);
	void FolderDialogOpen(char* szResult);
	void setOnOffInTreeList(const char* szIP, BOOL bConn, const char* szRecentNickName, const char* szRecentRealName)	;	
	CDialog * LoadScreen(LPCTSTR szDLLName, HMODULE& hModule);
	BOOL SaveFoldedGrp();
	void SetFoldedGrp();
	
	void AniMinimizeToTray(HWND hwnd);
	void AniMaximiseFromTray(HWND hwnd);
	void GetTrayWndRect(RECT *pRect);
	void Restore();
	void TrayIcon(BOOL add);

	void SetResize() ;	

	BOOL InitAlarmMapData();

public:
	
	CString m_strLoginID;
	long m_nAliveAckFailed;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IdmsMsnDLG_H__49AD9506_12ED_48B5_B248_417B8979B06A__INCLUDED_)
