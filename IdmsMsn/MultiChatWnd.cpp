// MultiChatWnd.cpp : implementation file
//

#include "stdafx.h"
#include "idmsmsn.h"
#include "MultiChatWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiChatWnd


#include "stdafx.h"
#include "idmsmsn.h"
#include "MultiChatWnd.h"

#include "INIManager.h"
#include "MsgBoxThread.h" 
#include "FileTransSock.h"
#include "OptionDlg.h"
#include <windows.h>

extern "C"
{
    #include <direct.h>
}
#include <atlconv.h>
#include<CDERR.H>
#include "ImageDataObject.h" //이모티콘

const int RESOURCE_CHAT_REDIT				= 10101 ;
const int RESOURCE_SPLITTER					= 10102 ;
const int RESOURCE_GRPBOX1					= 10103 ;
const int RESOURCE_GRPBOX2					= 10104 ;
const int RESOURCE_STATIC_REPLY				= 10105 ;
const int RESOURCE_COMBO_REPLY				= 10106 ;
const int RESOURCE_ADDLIST_BUTTON			= 10107 ;
const int RESOURCE_CHK_SEL_SEND_BUTTON		= 10108 ;
const int RESOURCE_CHK_TOP_BUTTON			= 10109 ;
const int RESOURCE_FILE_SEND_BUTTON			= 10110 ;
const int RESOURCE_FILE_SAVE_BUTTON			= 10111 ;
const int RESOURCE_OFFLINE_MSG_BUTTON		= 10112 ;
const int RESOURCE_EMOTICON_BUTTON			= 10113 ;
const int RESOURCE_CHK_USE_SEPLINE_BUTTON   = 10114 ;
const int RESOURCE_CHG_SEPLINE_BUTTON		= 10115 ;
const int RESOURCE_INPUT_CHAT_REDIT			= 10116 ;
const int RESOURCE_SEND_BUTTON				= 10117 ;
const int RESOURCE_ID_LISTCTRL				= 10118 ;
const int RESOURCE_CHK_EXIT_MULTICHAT		= 10119 ;

/////////////////////////////////////////////////////////////////////////////
// CMultiChatWnd

void CMultiChatWnd::Init()
{
	//m_nType = BITMAP_TOP;	
	//m_ChkExitMultiChat = FALSE;
	m_nType = BITMAP_STRETCH ; //BITMAP_TOP;
	m_nTopMostFlag = -1;
	m_pBtnEmoticon = NULL;
	m_pBtnAddReplyList = NULL;
	m_pBtnChgSepLine = NULL;	
	m_pBtnSaveFile= NULL;
	m_pBtnSendFile= NULL;
	m_pChkSepLineUse= NULL;
	m_pChkAlwaysOnTop= NULL;	
	m_pChkSelectSend= NULL;
	m_pComboReply= NULL;
	m_pBtnSnd= NULL;
	m_pChatREdit= NULL;
	m_pREditSendMsg= NULL;
	m_pWndSplitter= NULL;
	m_pStaticGroup1 = NULL;
	m_pStaticGroup2 = NULL;
	m_pStaticReply = NULL;
	m_pChkExitMultiChat = NULL;

	m_pCListCtrlMultiChatID = NULL;
	m_bSelectSend = FALSE;
	m_bAutoAppendMsgUse = FALSE;
	m_bLastMsgCompanion = FALSE;

	m_pEmoticonDialog = NULL; //이모티콘
	m_pAutoAppendMsgDlg = NULL;
	m_pBuddySlipMsgDlg = NULL;
	m_pChatReplyEditDlg = NULL;
}

CMultiChatWnd::CMultiChatWnd()
{
	Init();	
}

// 내가 요청하는경우
CMultiChatWnd::CMultiChatWnd(	CWnd* pParent, char* szDlgId, CStringArray& AryID, CStringArray& AryName )
{
	Init();	
	
	m_bHostClosed = FALSE;
	strncpy(m_szDlgID, szDlgId, sizeof(m_szDlgID));
		
	m_StringAryID.Copy(AryID) ; 
	m_StringAryName.Copy(AryName) ; 
	
	int nSize = m_StringAryID.GetSize();	
	CString strID;
	for(int i=0; i < nSize; i++)
	{
		strID = m_StringAryID.ElementAt(i);	
		m_setID.insert( (LPCSTR)strID ) ; 
	}

	//m_CEditMsg.m_nParentKind = C_MULTICHATDLG;
	
}
	
// 요청 받은 경우
CMultiChatWnd::CMultiChatWnd( CWnd* pParent, const char* szServerRoomNo)
{
	Init();	

	m_bHostClosed = FALSE;
	strncpy(m_szDlgID, szServerRoomNo, sizeof(m_szDlgID));

	MAP_STR2SET::iterator it ;
	it =  CChatSession::Instance().m_mapStr2SetMultiChat.find(szServerRoomNo);
	
	if(it != CChatSession::Instance().m_mapStr2SetMultiChat.end() )  
	{		
		SET_STR::iterator itSet;
		CString strName;
		for (itSet=(*it).second.begin(); itSet != (*it).second.end(); ++itSet) 
		{			
			m_StringAryID.Add((*itSet).c_str()) ; 
			
			if(CChatSession::Instance().mIDToNameMap.Lookup((*itSet).c_str() , strName))
			{
				m_StringAryName.Add(strName) ;
			}
			else
			{
				m_StringAryName.Add("모르는사람") ;
			}
		}		

		m_setID = (*it).second ;
	}
}

BEGIN_MESSAGE_MAP(CMultiChatWnd, CWnd)
	//{{AFX_MSG_MAP(CMultiChatWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
	ON_COMMAND(IDC_CONV_CLOSE, OnConvClose)	
	ON_COMMAND(IDC_FILESAVE, OnFilesave)		
	ON_COMMAND(IDM_OPTION, OnConfig)	
	
	ON_CBN_SELCHANGE(RESOURCE_COMBO_REPLY, OnSelchangeComboReply)
	ON_COMMAND_RANGE(ID_BUTTON32824, ID_BUTTON32862, OnFaceSelect)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMultiChatWnd message handlers


CMultiChatWnd::~CMultiChatWnd()
{	
	delete m_pBtnEmoticon;
	delete m_pBtnAddReplyList;
	delete	m_pBtnChgSepLine;	
	delete	m_pBtnSaveFile;
	delete	m_pBtnSendFile;
	delete	m_pChkSepLineUse;
	delete	m_pChkAlwaysOnTop;	
	delete	m_pChkSelectSend;
	delete	m_pComboReply;
	delete	m_pBtnSnd;
	delete	m_pChatREdit;
	delete	m_pREditSendMsg;
	delete m_pWndSplitter;
	delete m_pStaticGroup1;
	delete m_pStaticGroup2;
	delete m_pStaticReply;
	delete m_pCListCtrlMultiChatID;
	delete m_pChkExitMultiChat;
	
	TRACE("CMultiChatWnd::~CMultiChatWnd() \n");
}

void CMultiChatWnd::OnConvClose() 
{
	// TODO: Add your command handler code here
	OnClose();	
}


void CMultiChatWnd::OnConfig() 
{
	char szSetVal[10+1];
	memset(szSetVal, 0x00,sizeof(szSetVal));	
	COptionDlg dlg( (LPCSTR)(CChatSession::Instance().m_Nick),FALSE, this ) ;

	int nAwayTime = CINIManager::Instance().GetValueInt(INI_AWAY_SEC);

	dlg.m_nAwayTimeSecOld = nAwayTime;
	if(IDOK == dlg.DoModal())
	{	
		CString strMSg;
	
		m_pREditSendMsg->GetWindowText(strMSg);
		
		if(strMSg.IsEmpty() )
		{
			m_pREditSendMsg->ReplaceSel(" ");
		}
		
		m_pREditSendMsg->SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );
		m_pChatREdit->SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );

		m_pREditSendMsg->SetFont(&CChatSession::Instance().m_Font);		
		m_pChatREdit->SetFont(&CChatSession::Instance().m_Font);
			
		m_pChatREdit->SetDefaultCharFormat(CChatSession::Instance().m_cf);	
		m_pREditSendMsg->SetDefaultCharFormat(CChatSession::Instance().m_cf);
		
		m_pChatREdit->HideSelection(TRUE, TRUE);
	}		
	
	m_pREditSendMsg->SetFocus();
}


void CMultiChatWnd::OnFilesave() 
{
	// TODO: Add your command handler code here
	CString strPath, strContents;

	m_pChatREdit->GetWindowText(strContents);

	if(strContents.IsEmpty())
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "저장할 내용이 없습니다.");	
		pMsgThread->CreateThread();
	
		return;
	}

	FileDialogOpen(strPath);	

	CString strWrite, strBlock;
	
	if(strPath.GetLength() == 0)
	{
		return;
	}

	DeleteFile(strPath); 

	CStdioFile* p_fCmdDat = new CStdioFile(
								strPath,CFile::modeCreate | CFile::modeNoTruncate | 
								CFile::modeWrite | CFile::typeText | 
								CFile::shareDenyNone
							);

	
	p_fCmdDat->WriteString(strContents);
	p_fCmdDat->Close();
	delete p_fCmdDat;
	
	m_pREditSendMsg->SetFocus();
}

void CMultiChatWnd::OnBtnSendFile() 
{
	// TODO: Add your control notification handler code here	
	CFileDialog dlg(TRUE, NULL, NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT); 
	dlg.DoModal();	

	if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
	{
		TRACE("FNERR_BUFFERTOOSMALL!!\n");
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"선택 가능 파일명 길이 초과하였습니다. 선택파일 갯수를 줄여보세요~. ");				
		pMsgThread->CreateThread();	
		return;
	}

	
	POSITION pos = dlg.GetStartPosition();
	CString strPath="" , strTmp ="", strRootPath = "";
	int nFileCnt = 0;
	
	while(pos)
	{
		strTmp = dlg.GetNextPathName(pos);
		
		if(strTmp == "")
		{
			return;
		}
		
		TRACE("strTmp [%s]\n", strTmp);
		
		if(strRootPath == "")
		{
			CString strTmpPath = strTmp;
			int nPos = strTmpPath.ReverseFind('\\') ;
			strRootPath = strTmpPath.Mid(0, nPos);
			strPath = strRootPath + "|" + strTmp  ; // RootPath|파일경로	
			nFileCnt++;
		}
		else{
			strPath = strPath + "|" + strTmp  ;
			nFileCnt++;
		}		
	}	
	
	strPath = strPath + "|";
	TRACE("strPath [%s]\n", strPath);
	
	int nSize = m_StringAryID.GetSize();	
	
	CString strID;

	for(int i=0; i < nSize; i++)
	{
		strID = m_StringAryID.ElementAt(i);	
		
		if(strncmp(CChatSession::Instance().mMyInfo.m_szBuddyID, (LPCSTR)strID, sizeof(CChatSession::Instance().mMyInfo.m_szBuddyID)) != 0 )
		{
			CBuddyInfo * pCompanion;
						
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(strID, ( CObject*& ) pCompanion ))
			{
				if(pCompanion)
				{
					TRACE("szFileName [%s]\n",strPath);
					
					CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr);
					
					if( NULL == pSpckObjMap )
					{					
						TRACE("socket connect fail! [%s]\n", strPath);	
						
						CString strErrMsg;
						strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
						CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
						pMsgThread->CreateThread();
					}
					else
					{				
						TRACE("\nsocket connected! [%s]\n", strPath);
						
						CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID,
							CString(pCompanion->m_szIpAddr), 
							(LPSTR)(LPCSTR) strPath , nFileCnt) ;
					}
				}
			}			
		}		
	}
	
	/*
	CBuddyInfo*  pCompanion = NULL;	
	
	if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_pCompanion->m_szBuddyID, ( CObject*& ) pCompanion ))
	{
		if(pCompanion->m_bConnStatus == TRUE && ( pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2))
		{			
			TRACE("strPath [%s]\n",strPath);
			
			CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)m_pCompanion->m_szIpAddr);
			
			if( NULL == pSpckObjMap )
			{					
				TRACE("socket connect fail! [%s]\n", strPath);	
										
				CString strErrMsg;
				strErrMsg.Format("%s 에 접속할수 없습니다", m_pCompanion->m_szIpAddr);
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
				pMsgThread->CreateThread();
			}
			else
			{				
				TRACE("\nsocket connected! [%s]\n", strPath);
				
				CFileTransSock::Instance().InformFileSend( pSpckObjMap, m_pCompanion->m_szBuddyID,
							CString(m_pCompanion->m_szIpAddr), (LPSTR)(LPCSTR) strPath , nFileCnt) ;
			}

		}
		else if(pCompanion->m_nYouBlocked == 4)
		{
			//< 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함)
			char szFmt[100] ;
			memset(szFmt,0x00,sizeof(szFmt));
			sprintf(szFmt,"대화차단 상대 [%s] 에게 파일을 전송할수 없습니다", m_pCompanion->m_szUserName);				
			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
			
			pMsgThread->CreateThread();				
		}
		else
		{
			char szFmt[100] ;
			memset(szFmt,0x00,sizeof(szFmt));
			sprintf(szFmt,"OffLine 상대 [%s] 에게 파일을 전송할수 없습니다", m_pCompanion->m_szUserName);				
			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
			
			pMsgThread->CreateThread();			
		}
	}
	*/
}

void CMultiChatWnd::FileDialogOpen(CString &result)
{
	char szFilter[] = "txt Files (*.txt)|*.txt; *.txt|All Files (*.*)|*.*||";
	CFileDialog tmpFileDlg(FALSE  ,"txt",NULL,NULL,szFilter,NULL);
	int rtn = tmpFileDlg.DoModal();
	if(IDCANCEL == rtn)
		return;	
	result = tmpFileDlg.GetPathName();
}

void CMultiChatWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default	
	TRACE("%d / %d \n", point.x, point.y);
	m_pREditSendMsg->SetFocus();
	CWnd::OnLButtonDown(nFlags, point);
}

BOOL CMultiChatWnd::OnEraseBkgnd(CDC* pDC) 
{		
	CBrush backBrush( m_ColorWndBg /*RGB(235, 235, 235)*/ ); 
	//CBrush backBrush(RGB(215, 124, 255)); 
    CBrush* pOldBrush = pDC->SelectObject(&backBrush); 
    CRect rect; pDC->GetClipBox(&rect); 
    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush); 
    
	return TRUE; 
     
	/*	
	if(m_bmpBackground.GetPixelPtr() != 0) 
	{				
		CRect rc;
		GetClientRect(rc);
		int x = 0, y = 0;

		switch(m_nType) {
			case BITMAP_TOP:
				CWnd::OnEraseBkgnd(pDC);
				x = 0;
				y = 0;
				m_bmpBackground.DrawDIB(pDC, x, y);
				break;
			case BITMAP_CENTER:
				// center the bitmap
				CWnd::OnEraseBkgnd(pDC);
				x = (rc.Width() - m_bmpBackground.GetWidth()) / 2;
				y = (rc.Height() - m_bmpBackground.GetHeight()) / 2;
				m_bmpBackground.DrawDIB(pDC, x, y);
				break;

			case BITMAP_STRETCH:
				// stretch bitmap so it will best fit to the dialog
				m_bmpBackground.DrawDIB(pDC, 0, 0, rc.Width(), rc.Height());
				break;

			default:
				// tile the bitmap
				while(y < rc.Height()) {
					while(x < rc.Width()) {
						m_bmpBackground.DrawDIB(pDC, x, y);
						x += m_bmpBackground.GetWidth();
					}
					x = 0;
					y += m_bmpBackground.GetHeight();
				}
				break;
		}
	} else
		// no bitmap set. behave like a normal dialog
		return CWnd::OnEraseBkgnd(pDC);

	return TRUE;
	*/
}

BOOL CMultiChatWnd::SetBitmap(UINT uResource, int nType /*BITMAP_TILE*/) 
{
	m_nType = nType;
	
	//return m_bmpBackground.LoadResource(uResource);
	return TRUE;
}

HBRUSH CMultiChatWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if	(
			pWnd->GetDlgCtrlID() == RESOURCE_STATIC_REPLY ||
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_SEL_SEND_BUTTON ||
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_TOP_BUTTON || 
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_USE_SEPLINE_BUTTON || 
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_EXIT_MULTICHAT
		)
	{		
		//pDC->SetTextColor(RGB(255, 0, 0));		
		pDC->SetBkMode(TRANSPARENT);
		
		return (HBRUSH)::GetStockObject(NULL_BRUSH); 
	}

	return hbr;

	
	// TODO: Return a different brush if the default is not desired
	/*
	if(m_bmpBackground.GetPixelPtr() != 0) 
	{
		switch(nCtlColor) 
		{
			case CTLCOLOR_STATIC:
				// The Slider Control has CTLCOLOR_STATIC, but doesn't let
				// the background shine through,
				TCHAR lpszClassName[255];
				GetClassName(pWnd->m_hWnd, lpszClassName, 255);
				if(_tcscmp(lpszClassName, TRACKBAR_CLASS) == 0)
					return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

			case CTLCOLOR_BTN:
				// let static controls shine through
				pDC->SetBkMode(TRANSPARENT);
				return HBRUSH(m_HollowBrush);

			default:
				break;
		}
	}
	*/
	return hbr;
}

BOOL CMultiChatWnd::OnQueryNewPalette() 
{
	// TODO: Add your message handler code here and/or call default
	/*
	CPalette * pPal = m_bmpBackground.GetPalette();
	
	if( pPal != 0 && GetSafeHwnd() != 0 ) 
	{
		CClientDC dc(this);
		CPalette * pOldPalette = dc.SelectPalette(pPal, FALSE);
		UINT nChanged = dc.RealizePalette();
		dc.SelectPalette(pOldPalette, TRUE);

		if (nChanged == 0)
			return FALSE;

		Invalidate();
		return TRUE;
	}
	*/
	return CWnd::OnQueryNewPalette();
}

void CMultiChatWnd::OnPaletteChanged(CWnd* pFocusWnd) 
{
	/*
	CPalette * pPal = m_bmpBackground.GetPalette();
	if( pPal != 0 && GetSafeHwnd() != 0 && pFocusWnd != this && ! IsChild(pFocusWnd) ) {
		CClientDC dc(this);
		CPalette * pOldPalette = dc.SelectPalette(pPal, TRUE);
		UINT nChanged = dc.RealizePalette();
		dc.SelectPalette(pOldPalette, TRUE);

		if( nChanged )
			Invalidate();
	} else
		CWnd::OnPaletteChanged(pFocusWnd);
	*/
	CWnd::OnPaletteChanged(pFocusWnd);
	// TODO: Add your message handler code here
	
}

static UINT auIDStatusBar[] = 
{
	ID_SEPARATOR
};


int CMultiChatWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	int i = 0;
	
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	// TODO: Add your specialized creation code here
	DWORD dwResult;  	
	dwResult = GetSysColor(COLOR_3DFACE); 
 
	m_ColorWndBg = RGB( (unsigned int)GetRValue(dwResult), 
		 (unsigned int)GetGValue(dwResult), 
		 (unsigned int)GetBValue(dwResult) ); 

	//Menu
	CMenu newMenu;
	newMenu.LoadMenu(IDR_MENU_CONVERSATION);
	SetMenu( &newMenu );	
	DrawMenuBar();
	
	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);


	m_FontCtrl.CreateFont(
		12,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		DEFAULT_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"굴림"); 	
    
	int nLen = 0;
	int nPos = 0;
	/*
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
		
	nPos = 0;
	nLen = strlen(szDirTmp);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	strFileName.Format("%s\\chat_bg.bmp", szDirTmp);

	m_bmpBackground.Load(strFileName);	
	*/

	SetFont(&m_FontCtrl);
	////////////////////////////////////////////////////////////
	// 컨트롤 동적 생성!!!
	int nSplitterUp  = 20; 
	
	// m_pChkExitMultiChat
	m_pChkExitMultiChat = new CButton ;	
	m_pChkExitMultiChat->Create(_T("Multi Chat 나가기"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(5,4,180,18), this, RESOURCE_CHK_EXIT_MULTICHAT);
	m_pChkExitMultiChat->SetFont(&m_FontCtrl);

	//ListCtrl
	m_pCListCtrlMultiChatID = new CListCtrl;
	m_pCListCtrlMultiChatID->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT,
								CRect(3,20,170,148), this, RESOURCE_ID_LISTCTRL );					
	m_pCListCtrlMultiChatID->SetFont(& m_FontCtrl);


	//대화창
	m_pChatREdit = new CEmoticonRichEditCtrl;
	m_pChatREdit->SetRole(ROLE_DISPLAY);
	m_pChatREdit->Create(
					WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_BORDER|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL   ,
					CRect(172,20,409,148), this, RESOURCE_CHAT_REDIT );			
	m_pChatREdit->Initialize();
	m_pChatREdit->SetFont(& CChatSession::Instance().m_Font);		
	m_pChatREdit->setUrlDetection(TRUE);

	
	
	//
	m_pWndSplitter = new CSplitterControl ;
	CRect rc(3,149,410,159);		
	m_pWndSplitter->Create(WS_CHILD | WS_VISIBLE, rc, this, RESOURCE_SPLITTER );
	m_pWndSplitter->SetRange(100, 100, -1);
	m_pWndSplitter->SetStyle(SPS_HORIZONTAL);
	

	//group box	
	m_pStaticGroup1 = new CButton;
	m_pStaticGroup1->Create(_T(""), WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 
			CRect(3,149,409, 189), this, RESOURCE_GRPBOX1 );

	m_pStaticGroup2 = new CButton;
	m_pStaticGroup2->Create(_T(""), WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 
			CRect(3,181,409, 221), this, RESOURCE_GRPBOX2 );
	
	//static 응답
	m_pStaticReply = new CStatic ;
	m_pStaticReply->Create(_T("응답"), WS_CHILD|WS_VISIBLE|SS_CENTER, 
				CRect(5,166,40,180), this , RESOURCE_STATIC_REPLY );		
	m_pStaticReply->SetFont(&m_FontCtrl,TRUE);	


	//Combo
	m_pComboReply = new CComboBox;
	m_pComboReply->Create(  WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST,
							CRect(40,161,190,400), this, RESOURCE_COMBO_REPLY);
	m_pComboReply->SetFont(&m_FontCtrl,TRUE);	

	//Addlist Btn
	m_pBtnAddReplyList = new CMyButton;
	m_pBtnAddReplyList->Create(_T("설정"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(192,161,225,185), this, RESOURCE_ADDLIST_BUTTON);
	m_pBtnAddReplyList->SetFont(&m_FontCtrl);	
		
	//m_pBtnSendFile
	m_pBtnSendFile = new CMyButton;
	m_pBtnSendFile->Create(_T("파일전송"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(8,192,70,217), this, RESOURCE_FILE_SEND_BUTTON );	
	m_pBtnSendFile->SetFont(&m_FontCtrl);	

	//m_pBtnSaveFile
	m_pBtnSaveFile = new CMyButton;
	m_pBtnSaveFile->Create(_T("내용저장"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(72, 192, 70+2+62, 217), this, RESOURCE_FILE_SAVE_BUTTON );
	m_pBtnSaveFile->SetFont(&m_FontCtrl);
	
	//쪽지
	/*
	m_pBtnBuddyMsg = new CMyButton;
	m_pBtnBuddyMsg->Create(_T("쪽지"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(136, 192, 200, 217), this, RESOURCE_OFFLINE_MSG_BUTTON);
	m_pBtnBuddyMsg->SetFont(&m_FontCtrl);
	*/
	
	//Emoticon
	m_pBtnEmoticon = new CMyButton;
	m_pBtnEmoticon->Create(_T("이모티콘"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(136, 192, 200, 217), this, RESOURCE_EMOTICON_BUTTON);
	m_pBtnEmoticon->SetFont(&m_FontCtrl);
	
	// m_pChkSelectSend
	m_pChkSelectSend = new CMyButton ;	
	m_pChkSelectSend->Create(_T("응답선택시 전송"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(227,161,340,185), this, RESOURCE_CHK_SEL_SEND_BUTTON);
	m_pChkSelectSend->SetFont(&m_FontCtrl);
	m_pChkSelectSend->SetIcon(IDI_ICON_CHK_ON,IDI_ICON_CHK_OFF);
	m_pChkSelectSend->DrawBorder(FALSE);


	//자동부착글
	m_pChkSepLineUse = new CMyButton ;
	m_pChkSepLineUse->Create(_T("자동 부착글"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(280, 192, 370, 217), this, RESOURCE_CHK_USE_SEPLINE_BUTTON);
	m_pChkSepLineUse->SetFont(&m_FontCtrl);	
	m_pChkSepLineUse->SetIcon(IDI_ICON_CHK_ON,IDI_ICON_CHK_OFF);
	m_pChkSepLineUse->DrawBorder(FALSE);
	
	// m_pChkAlwaysOnTop
	m_pChkAlwaysOnTop = new CMyButton ;	
	m_pChkAlwaysOnTop->Create(_T("항상 위"), WS_CHILD |WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(342,164,416,183), this, RESOURCE_CHK_TOP_BUTTON );	

	//m_pChkAlwaysOnTop->Create(_T("항상 위"), WS_CHILD |WS_VISIBLE|BS_AUTOCHECKBOX, 
	//				CRect(342,164-nSplitterUp,416,183-nSplitterUp), this, RESOURCE_CHK_TOP_BUTTON );	
	m_pChkAlwaysOnTop->SetFont(&m_FontCtrl);	
	m_pChkAlwaysOnTop->SetIcon(IDI_ICON_CHK_ON,IDI_ICON_CHK_OFF);
	m_pChkAlwaysOnTop->DrawBorder(FALSE);
	m_pChkAlwaysOnTop->EnableBalloonTooltip();
	
	//자동부착글설정
	m_pBtnChgSepLine = new CMyButton;
	m_pBtnChgSepLine->Create(_T("설정"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(375, 194, 410, 215), this, RESOURCE_CHG_SEPLINE_BUTTON);
	m_pBtnChgSepLine->SetFont(&m_FontCtrl);
	
	
	//입력창
	m_pREditSendMsg = new CEmoticonRichEditCtrl;
	m_pREditSendMsg->SetRole(ROLE_SEND_INPUT);	
	m_pREditSendMsg->Create(
					WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_BORDER|WS_EX_ACCEPTFILES|ES_WANTRETURN |ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL ,
					CRect(3,221,360,265), this, RESOURCE_INPUT_CHAT_REDIT );		
	
	m_pREditSendMsg->Initialize();
	m_pREditSendMsg->SetFont(&CChatSession::Instance().m_Font);
	m_pREditSendMsg->m_nParentKind = C_MULTICHATDLG;
	
	//왜 처음에 한글치면 색이 안먹힐까??? 
	m_pREditSendMsg->SetSel(0, -1);		
	m_pREditSendMsg->ReplaceSel("ㄱ");
	m_pREditSendMsg->SetSel(0, -1);		
	m_pREditSendMsg->ReplaceSel("");
	
	// Send 버튼
	m_pBtnSnd = new CMyButton;
	m_pBtnSnd->Create(_T("Send"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(362,228,410,263), this, RESOURCE_SEND_BUTTON );
	m_pBtnSnd->SetFont(&m_FontCtrl);


	m_pChatREdit->SetDefaultCharFormat(CChatSession::Instance().m_cf);	
	m_pREditSendMsg->SetDefaultCharFormat(CChatSession::Instance().m_cf);
	////////////////////////////////////////////////////////////
	//Status
	
	if (m_statusBar.Create(this))
	{
		m_statusBar.SetIndicators(auIDStatusBar, sizeof(auIDStatusBar) / sizeof(unsigned int));

		m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL );
	}

	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,   0, reposQuery, rcClientNow);
	
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,	rcClientNow.top - rcClientStart.top);

	CRect  rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);

	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	CRect rcWindow;
	GetWindowRect(rcWindow);

	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();

	MoveWindow(rcWindow, FALSE);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);	
	// statusbar
	
	//List구성
	m_pCListCtrlMultiChatID->SetExtendedStyle(m_pCListCtrlMultiChatID->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ); //| LVS_EX_CHECKBOXES

	int nSize = m_StringAryID.GetSize();
		
	m_pCListCtrlMultiChatID->InsertColumn(0,"ID");	
	m_pCListCtrlMultiChatID->InsertColumn(1,"이름");	
	m_pCListCtrlMultiChatID->InsertColumn(2,"상태");	
	m_pCListCtrlMultiChatID->InsertColumn(3,"ServerRoomNo");	
	
	m_pCListCtrlMultiChatID->SetColumnWidth(0, 90);	
	m_pCListCtrlMultiChatID->SetColumnWidth(1, 80);
	m_pCListCtrlMultiChatID->SetColumnWidth(2, 1);
	m_pCListCtrlMultiChatID->SetColumnWidth(3, 1);
	
	for( i=0; i < nSize; i++)
	{
		CString strID = m_StringAryID.ElementAt(i);	
		CString strName = m_StringAryName.ElementAt(i);	
		m_strIDNames = m_strIDNames + strID + "|" + strName + "|"; // ID|NAME|ID|NAME|ID|NAME|
		//m_strIDs = m_strIDs + strID + "|" ; // ID|ID|ID|
	}
	TRACE("m_strIDNames [%s]\n", m_strIDNames);

	for( i=0; i < nSize; i++)
	{
		CString strID = m_StringAryID.ElementAt(i);	
		CString strName = m_StringAryName.ElementAt(i);

		// 자신의 정보는 Multi Chat 화면에 안나오게한다. 
		//if( strcmp(strIp, m_pMyInfo->m_szIP) != 0 )
		{
			
			CString strItem;		
			LVITEM lvi;		
			lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
			int tempCnt = m_pCListCtrlMultiChatID->GetItemCount();
			lvi.iItem = tempCnt;		
			
			//ID
			lvi.iSubItem = 0;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strID);
			lvi.iImage = 1;
			m_pCListCtrlMultiChatID->InsertItem(&lvi);
			m_pCListCtrlMultiChatID->SetItem(&lvi);
			//Name
			lvi.iSubItem = 1;
			lvi.pszText = (LPTSTR)(LPCTSTR)(strName);		
			lvi.iImage = -1;
			m_pCListCtrlMultiChatID->InsertItem(&lvi);		
			m_pCListCtrlMultiChatID->SetItem(&lvi);		
			//상태
			lvi.iSubItem = 2;
			if( strcmp(strID, m_szHostID) == 0 )
			{
				lvi.pszText = (LPTSTR)(LPCTSTR)("Host");
			}
			else
			{
				lvi.pszText = (LPTSTR)(LPCTSTR)("");
			}
			
			lvi.iImage = -1;
			m_pCListCtrlMultiChatID->InsertItem(&lvi);		
			m_pCListCtrlMultiChatID->SetItem(&lvi);
		}
	}
	//////////
		
	m_pChatREdit->SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG);
	m_pREditSendMsg->SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG);
	
	
		
	SetResize();	
	
	CString strPosSrc, strPosX, strPosY, strWidth, strHight;
	CINIManager::Instance().GetValue(INI_PREV_POS_SIZE_CHAT, strPosSrc);	
	
	nLen = strPosSrc.GetLength();
	nPos = -1;	
	nPos = strPosSrc.Find('|');  strPosX = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strPosY = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strWidth = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strHight = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);

	MoveWindow( atoi((LPCSTR)strPosX),atoi((LPCSTR)strPosY),atoi((LPCSTR)strWidth),atoi((LPCSTR)strHight));
	TRACE("SetTimer\n");
	
	//Title
	char szTmp[50];
	memset(szTmp, 0x00, sizeof(szTmp));
	sprintf(szTmp, "Multi Chat[%s]", m_szDlgID);	
	SetWindowText(szTmp);

	// Reply Msg
	CString strTmpUseReplySelectSend;
	CINIManager::Instance().GetValue(INI_USE_SELECT_SEND,  strTmpUseReplySelectSend );	

	if(strTmpUseReplySelectSend.Compare("Y") == 0)
		m_bSelectSend = TRUE ;
	else
		m_bSelectSend = FALSE ;
	
	//Auto Append Msg
	CString strTmpUseAutoAppend;
	CINIManager::Instance().GetValue(INI_USE_AUTO_APPEND,  strTmpUseAutoAppend );	
	
	if(strTmpUseAutoAppend.Compare("Y") == 0)
		m_bAutoAppendMsgUse = TRUE ;
	else
		m_bAutoAppendMsgUse = FALSE ;

	SetReplyList();	
	
	if( strcmp(m_szHostID,  CChatSession::Instance().mMyInfo.m_szBuddyID) == 0 )
	{		
		// 서버로부터 고유 RoomNo를 부여받기전 상태는 Disabled
		m_pBtnSnd->EnableWindow(FALSE);
		m_pChatREdit->EnableWindow(FALSE);
		m_pREditSendMsg->EnableWindow(FALSE);
		m_statusBar.SetWindowText("멀티채팅 설정중입니다. 잠시만 기다리세요.");	

		SetTimer(0, 100, NULL);	
	}

	UpdateData(FALSE);

	RedrawWindow();

	return 0;
}


void CMultiChatWnd::OnBtnEmoticon() 
{
	// TODO: Add your control notification handler code here
	if(m_pEmoticonDialog)
	{
		delete m_pEmoticonDialog;
		m_pEmoticonDialog = NULL;
	}	
	
	m_pEmoticonDialog = new CEmoticonBase();
	
	if(m_pEmoticonDialog != NULL)
	{		
		BOOL ret = m_pEmoticonDialog->Create(IDD_DLG_EMOTICON_BASE, this);
		
		if(!ret)
		{
			return ;
		}
		
		m_pEmoticonDialog->ShowWindow(SW_SHOW);
		
		
		CRect	rWnd;
		m_pBtnEmoticon->GetWindowRect(rWnd);		

		m_pEmoticonDialog->MoveWindow(rWnd.left+10,rWnd.top-150, 230, 170);

		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}
}

void CMultiChatWnd::SetResize() 
{	
	m_cControlPos.SetParent(this);	
	
	m_cControlPos.AddControl(m_pCListCtrlMultiChatID	, CP_RESIZE_VERTICAL	 );			
	m_cControlPos.AddControl(m_pChatREdit	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );			
	m_cControlPos.AddControl(m_pREditSendMsg	, CP_RESIZE_HORIZONTAL |  CP_MOVE_VERTICAL );		
	m_cControlPos.AddControl(m_pWndSplitter	, CP_MOVE_VERTICAL | CP_RESIZE_HORIZONTAL  	 );	
	m_cControlPos.AddControl(m_pStaticGroup1	, CP_MOVE_VERTICAL | CP_RESIZE_HORIZONTAL	);	
	m_cControlPos.AddControl(m_pStaticGroup2	, CP_MOVE_VERTICAL | CP_RESIZE_HORIZONTAL	);		
	m_cControlPos.AddControl(m_pComboReply	,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pStaticReply	,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pBtnAddReplyList	,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pChkSelectSend	,  CP_MOVE_VERTICAL	);
	m_cControlPos.AddControl(m_pChkAlwaysOnTop,  CP_MOVE_VERTICAL);	
	m_cControlPos.AddControl(m_pBtnSendFile,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pBtnSaveFile,  CP_MOVE_VERTICAL	);				
	m_cControlPos.AddControl(m_pBtnEmoticon,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pChkSepLineUse,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pBtnChgSepLine,  CP_MOVE_VERTICAL	);		
	
	m_cControlPos.AddControl(m_pBtnSnd	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL	);		
}


void CMultiChatWnd::OnBtnAddList() 
{
	// TODO: Add your control notification handler code here

	if(m_pChatReplyEditDlg)
	{
		delete m_pChatReplyEditDlg;
		m_pChatReplyEditDlg = NULL;
	}	
	
	m_pChatReplyEditDlg = new CChatReplyEditDlg(this);
	
	if(m_pChatReplyEditDlg != NULL)
	{		
		BOOL ret = m_pChatReplyEditDlg->Create(IDD_DLG_MODFY_REPLY, this);
		
		if(!ret)
		{
			return ;
		}
		
		m_pChatReplyEditDlg->ShowWindow(SW_SHOW);		
	
		CRect	rWnd;
		m_pBtnAddReplyList->GetWindowRect(rWnd);
		m_pChatReplyEditDlg->MoveWindow(rWnd.left+10,rWnd.top-150, 335, 340);	
	}	
}

void CMultiChatWnd::OnBtnChgSepline() 
{
	// TODO: Add your control notification handler code here
	
	if(m_pAutoAppendMsgDlg)
	{
		delete m_pAutoAppendMsgDlg;
		m_pAutoAppendMsgDlg = NULL;
	}	
	
	m_pAutoAppendMsgDlg = new CAutoAppendMsgDlg();
	
	if(m_pAutoAppendMsgDlg != NULL)
	{		
		BOOL ret = m_pAutoAppendMsgDlg->Create(IDD_DLG_AUTO_APPEND_MSG, this);
		
		if(!ret)
		{
			return ;
		}
		
		m_pAutoAppendMsgDlg->ShowWindow(SW_SHOW);
				
		CRect	rWnd;
		m_pBtnChgSepLine->GetWindowRect(rWnd);
		m_pAutoAppendMsgDlg->MoveWindow(rWnd.left+10,rWnd.top+10, 380, 160);	
	}
}

void CMultiChatWnd::OnSize(UINT nType, int cx, int cy) 
{
	//CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	RedrawWindow();

	if(SIZE_RESTORED == nType)
	{		
		::SendMessage(m_pChatREdit->GetSafeHwnd(), WM_VSCROLL, SB_BOTTOM,0);	
	}	
}

BOOL CMultiChatWnd::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{		
		HWND hWnd1 = ::GetFocus();
		HWND hWnd2 = m_pREditSendMsg->GetSafeHwnd();
		
		if( hWnd1 == hWnd2 )		
		{			
			if(GetKeyState(VK_SHIFT)<0)			
			{
				TRACE("Shift\n");
				//20080709
				m_pREditSendMsg->ReplaceSel("\r\n");	
				m_pREditSendMsg->HideSelection(TRUE, TRUE);
			}
			else
			{
				OnButtonSendmsg();
				TRACE("OnButtonSendmsg()\n");
				return TRUE;	
			}						
		}
	}

	
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CMultiChatWnd::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 0)
	{
		// 내가 멀티패팅을 처음으로 시작하는 경우라면 서버에게 RoomNo를 요청한다 
		if( strcmp(m_szHostID, CChatSession::Instance().mMyInfo.m_szBuddyID) == 0 )
		{		
			//CChatSession::Instance().RequestMultiChatRoomNo( m_szDlgID, m_strIDs );
			CChatSession::Instance().RequestMultiChatRoomNo( m_szDlgID, m_strIDNames );
			
		}
		
		KillTimer(0);
	}
	
	CWnd::OnTimer(nIDEvent);
}

void CMultiChatWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	// m_pREditSendMsg->SetFocus();
	// TODO: Add your message handler code here
	
}

LRESULT CMultiChatWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == WM_NOTIFY)
	{
		if (wParam == RESOURCE_SPLITTER )
		{	
			SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
			DoResize(pHdr->delta);
		}
	}	
	return CWnd::DefWindowProc(message, wParam, lParam);
}

void CMultiChatWnd::DoResize(int delta) // delta -> Y 변동값!!
{	
	CSplitterControl::ChangeHeight(m_pChatREdit, delta);
	CSplitterControl::ChangeHeight(m_pCListCtrlMultiChatID, delta);
	
	CSplitterControl::ChangeHeight(m_pREditSendMsg, - delta, CW_BOTTOMALIGN);
	
	CSplitterControl::ChangePos(m_pStaticReply,0 , delta ) ;
	CSplitterControl::ChangePos(m_pComboReply,0 , delta ) ;
	CSplitterControl::ChangePos(m_pBtnAddReplyList,0 , delta ) ;	
	CSplitterControl::ChangePos(m_pChkSelectSend,0 , delta ) ;
	CSplitterControl::ChangePos(m_pStaticGroup1,0 , delta ) ;
	CSplitterControl::ChangePos(m_pStaticGroup2,0 , delta ) ;
	
	CSplitterControl::ChangePos(m_pChkAlwaysOnTop,0 , delta ) ;
	CSplitterControl::ChangePos(m_pBtnChgSepLine,0 , delta ) ;
	CSplitterControl::ChangePos(m_pChkSepLineUse,0 , delta ) ;
	CSplitterControl::ChangePos(m_pBtnSaveFile,0 , delta ) ;
	CSplitterControl::ChangePos(m_pBtnSendFile,0 , delta ) ;	
	CSplitterControl::ChangePos(m_pBtnEmoticon,0 , delta ) ;    
		
	Invalidate();
	UpdateWindow();	
}


void CMultiChatWnd::OnFaceSelect(UINT nID)
{
	long nStart , nEnd ;

	int nFace = nID - ID_BUTTON32824;
	
	CString strTmp;
	strTmp.Format(" ec%02d ", nFace ); 
				
	m_pREditSendMsg->GetSel(nStart, nEnd);
		
	TRACE("nStart[%d] nEnd[%d]\n", nStart, nEnd);	
		
	m_pREditSendMsg->ReplaceSel(strTmp);				
				
	m_pREditSendMsg->HideSelection(TRUE, TRUE);	

	m_pREditSendMsg->SetFocus();
}


LRESULT CMultiChatWnd::OnEmoticonMap(WPARAM wParam, LPARAM lParam)
{
	char* pszKey = NULL ;
	char* pszVal = NULL ;
	pszKey = (char*)wParam;
	pszVal = (char*)lParam;
		
	CString strVal(pszVal);
	m_EmoStorageMap.SetAt( pszKey , strVal) ;

	TRACE("OnEmoticonMap : pszKey [%s] strVal [%s]\n", pszKey, strVal );

	delete[] pszKey;
	delete[] pszVal;

	return TRUE;
}

LRESULT CMultiChatWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(message == WM_CHATMSG)
	{
		OnChatMsg(wParam, lParam);
	}
	else if(message == WM_EMOTICON_MAP)
	{
		OnEmoticonMap(wParam, lParam);
	}
	else if(message == WM_BTNDOWN)
	{
		CMyButton* pBtn = (CMyButton*)wParam;
		CString strBtnText;
		pBtn->GetWindowText(strBtnText);

		TRACE("%s\n", strBtnText);

		if(strBtnText == "Send")
		{
			OnButtonSendmsg();
		}
		else if(strBtnText == "내용저장")
		{
			OnFilesave() ;
		}
		else if(strBtnText == "파일전송")
		{
			OnBtnSendFile() ;
		}
		/*
		else if(strBtnText == "쪽지")
		{
			OnBtnBuddyMsg() ;
		}
		*/
		else if(strBtnText == "설정")
		{
			if( RESOURCE_ADDLIST_BUTTON == pBtn->GetDlgCtrlID() )
			{
				OnBtnAddList() ;
			}
			else if( RESOURCE_CHG_SEPLINE_BUTTON == pBtn->GetDlgCtrlID() )
			{
				OnBtnChgSepline();
			}
		}
		else if(strBtnText == "이모티콘")
		{
			OnBtnEmoticon();
		}
		else if(strBtnText == "항상 위")
		{
			OnCheckAlwaysTop();
			//m_pREditSendMsg->SetFocus();
		}		
	}
	else if (message == WM_MOUSEACTIVATE )
	{
		TRACE("CMultiChatWnd => WM_MOUSEACTIVATE \n");				
		m_pREditSendMsg->SetFocus();
	}	
	else if (message == WM_SETREPLY )
	{
		SetReplyList();
		m_pREditSendMsg->SetFocus();
	}
	else if (message == WM_FOCUS_INPUT )
	{		
		m_pREditSendMsg->SetFocus();
	}
	else if (message == WM_SERVER_CLOSED )
	{		
		OnServerClosed();		
	}
	else if (message == WM_BUDDY_ONLINE )
	{		
		OnChatResume(wParam, lParam);
	}
	else if (message == WM_BUDDY_OFFLINE )
	{		
		OnChatBuddyOffLine(wParam, lParam);		
	}
	else if (message == WM_EXIT_MULTI_CHAT )
	{		
		OnExitMultiChat(wParam, lParam);		
	}	
	
	return CWnd::WindowProc(message, wParam, lParam);
}

void CMultiChatWnd::OnExitMultiChat(WPARAM wParam, LPARAM lParam)
{	
	// wParam -> MChat 나간 ID

	int nSize = m_StringAryID.GetSize();
	char* szID = (char*)wParam ;
		
	CString strID;
	for(int i=0; i < nSize; i++)
	{
		strID = m_StringAryID.ElementAt(i);	
		if( strcmp((LPCSTR)strID, szID )==0 )
		{
			m_StringAryID.RemoveAt(i);
			break;
		}
	}

	SET_STR::iterator itSet;
	itSet = m_setID.find(szID) ; 
	if(itSet != m_setID.end())
	{
		m_setID.erase(itSet) ;
	}	
	
	// List 에서 제거
	LVFINDINFO info;
	int nIndex = -1;

	info.flags = LVFI_PARAM;
	info.flags = LVFI_PARTIAL|LVFI_STRING;
	info.psz = szID;		
	nIndex = m_pCListCtrlMultiChatID->FindItem(&info);
	
	if(nIndex != -1)
	{
		m_pCListCtrlMultiChatID->DeleteItem(nIndex);
	}
	
	UpdateData(FALSE);
	
	char szMsg [300];
	memset(szMsg, 0x00, sizeof(szMsg));
	
	CString strName;
	if(CChatSession::Instance().mIDToNameMap.Lookup(szID , strName))
	{
		sprintf(szMsg,"\n★ %s [%s]사용자가 Multi Chat을 종료하였습니다.", szID, (LPCSTR)strName);		
	}
	else
	{
		sprintf(szMsg,"\n★ %s 사용자가 Multi Chat을 종료하였습니다.", szID);		
	}

	int len = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(len, len);	
	
	m_pChatREdit->ReplaceSel(szMsg);	
	m_pChatREdit->HideSelection(TRUE, TRUE);

	int lenAft = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(len, lenAft);		
	CHARFORMAT2 cf;
	m_pChatREdit->GetSelectionCharFormat(cf);
	cf.crTextColor = CChatSession::Instance().m_ColorBuddy ;
	cf.dwMask = CFM_COLOR | CFM_FACE;	
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	strcpy(cf.szFaceName, lf.lfFaceName);
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_pChatREdit->SetSelectionCharFormat(cf);      
		
	delete [] szID ;

	::SendMessage(m_pChatREdit->GetSafeHwnd(), EM_SCROLLCARET, 0,0);	

	m_pChatREdit->HideSelection(TRUE, TRUE);
		
}


void CMultiChatWnd::OnServerClosed()
{	
	m_pREditSendMsg->SetWindowText("서버 연결이 종료되었습니다.");
	m_pREditSendMsg->EnableWindow(FALSE);
	m_pBtnSnd->EnableWindow(FALSE);	
}

void CMultiChatWnd::OnChatResume(WPARAM wParam, LPARAM lParam)
{		
	char szFromID[20+1];
	char szTmpID[20+1+10];
	memset(szFromID , 0x00, sizeof(szFromID));
	memset(szTmpID , 0x00, sizeof(szTmpID));
	
	strncpy(szFromID , (char*) lParam , sizeof(szFromID)); 
	sprintf(szTmpID, "(OffLine)%s", szFromID);
		
	// Tree  
	// 트리에서 ONLINE 했음으로 변경	
	LVFINDINFO info;
	int nIndex = -1;

	info.flags = LVFI_PARAM;
	info.flags = LVFI_PARTIAL|LVFI_STRING;
	info.psz = szTmpID;		
	nIndex = m_pCListCtrlMultiChatID->FindItem(&info);
	
	if(nIndex != -1)
	{				
		LVITEM rItem;		
					
		ZeroMemory ( &rItem, sizeof(LVITEM) );
		rItem.iItem = nIndex;
		rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
		rItem.iSubItem = 0;			
		rItem.pszText = szFromID;
		rItem.iImage = -1;		
		m_pCListCtrlMultiChatID->SetItem(&rItem);

		ZeroMemory ( &rItem, sizeof(LVITEM) );
		rItem.iItem = nIndex;
		rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
		rItem.iSubItem = 2;	
		rItem.pszText = "참여";		
		rItem.iImage = -1;		
		m_pCListCtrlMultiChatID->SetItem(&rItem);	
		
	}
}

LRESULT CMultiChatWnd::OnChatMsg(WPARAM wParam, LPARAM lParam)
{
	S_CHAT_CLIENT * pChatMsg = (S_CHAT_CLIENT*) lParam;
	
	CString strMsg;	
	int iLength = pChatMsg->strMsg.GetLength() ;		

	int nStart = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(nStart, nStart);		
			
	m_pChatREdit->ReplaceSel("\r\n");
	
	CString strName;
	if(CChatSession::Instance().mIDToNameMap.Lookup(pChatMsg->strIDFrom , strName))
	{
		strMsg.Format("%s [%s]\n %s\r\n", strName, pChatMsg->strIDFrom, pChatMsg->strMsg);		
	}
	else
	{
		strMsg.Format("UnKnown [%s]\n %s\r\n", pChatMsg->strIDFrom, pChatMsg->strMsg);		
	}
    	
	m_pChatREdit->ReplaceSel(strMsg);

	int lenAft = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(nStart, lenAft);		
	CHARFORMAT2 cf;
	m_pChatREdit->GetSelectionCharFormat(cf);
	cf.crTextColor = CChatSession::Instance().m_ColorBuddy;
	cf.dwMask = CFM_COLOR | CFM_FACE;	
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	strcpy(cf.szFaceName, lf.lfFaceName);
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_pChatREdit->SetSelectionCharFormat(cf);      

	m_pChatREdit->SendMessage(EM_SCROLLCARET, 0);	
	
	m_pChatREdit->HideSelection(TRUE, TRUE); // ! 
	
	CString cstr;	
	CTime iDT =  CTime::GetCurrentTime();
	m_strlastMsgDate = iDT.Format("%Y년 %m월 %d일 / %H시 %M분 %S초");

	cstr.Format("최종시간 : %s", m_strlastMsgDate);
	m_statusBar.SetWindowText(cstr);
	m_strlastMsgDate.Empty();

	//Log
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);

	if(strUseLogFolder == "Y")
	{
		// Download Path		
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strFileName.Format("%s\\대화이력\\",strLogFolderPath);	
	}
	else
	{
		/////////////////////////////////////////////////////////		
		//char szWinDir[512];		
		char szDirTmp[512];
		char szDir[1024];
		
		//memset(szWinDir, 0x00, sizeof(szWinDir));		
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
		memset(szDir, 0x00, sizeof(szDir));	
		
		//GetWindowsDirectory(szWinDir, sizeof(szWinDir));
		
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));
		
		// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
		// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ 까지 구한다.
		
		int nPos = 0;
		int nLen = strlen(szDirTmp);
		for(int i = nLen; i>=0 ; i--)
		{
			if(szDirTmp[i] == '\\' && nPos <1 )
			{
				szDirTmp[i] = '\0';
				nPos++;
				break;
			}
		}
		
		TRACE("%s\n" , szDirTmp);	
		
		strFileName.Format("%s\\대화이력\\",szDirTmp);	
	}
	
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}

	CString strTmp;	
	CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
	CString strtmSimple = iDT.Format("%Y%m%d");

	CString strFileNameAll;	
	strFileNameAll.Format("%s\\%sMultiChat.txt", strFileName, strtmSimple);
	strTmp.Format("%s: %s\n", strtmDetailed, strMsg);	
	writeLogFile(strFileNameAll, "%s\n", strTmp);	

	//m_CEditMsg.SetFocus();

	FlashWindow(TRUE);	
	
	// sound
	CString strVal;
	CINIManager::Instance().GetValue(INI_USE_SOUND, strVal ) ;
	if( "Y" == strVal )
	{
		CString strStartMusicPath;
		sndPlaySound(NULL, SND_ASYNC);
		CINIManager::Instance().GetValue(INI_SOUND_PATH, strStartMusicPath);
		//strStartMusicPath = "notify.wav";
		sndPlaySound(strStartMusicPath, SND_ASYNC );	
		
		//Play time
		//SetTimer(300, CINIManager::Instance().GetValueInt(INI_START_MUSIC_PLAY_TIME_SEC) * 1000 , 0);  
	}	

	return TRUE;
}

void CMultiChatWnd::DoDataExchange(CDataExchange* pDX) 
{
	// TODO: Add your specialized code here and/or call the base class
	DDX_Check(pDX, RESOURCE_CHK_SEL_SEND_BUTTON, m_bSelectSend);
	DDX_Check(pDX, RESOURCE_CHK_USE_SEPLINE_BUTTON, m_bAutoAppendMsgUse);
	
	CWnd::DoDataExchange(pDX);
}

void CMultiChatWnd::SetReplyList()
{	
	m_pComboReply->ResetContent();

	CString strReply ;
	int nCnt = 0;
	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST0,  strReply );	
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST1,  strReply );	
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST2,  strReply );	
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST3,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST4,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST5,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST6,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST7,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST8,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);

	strReply.Empty();
	CINIManager::Instance().GetValue(INI_REPLY_LIST9,  strReply );		
	if(strReply.GetLength() > 0)
		m_pComboReply->AddString(strReply);
}


void CMultiChatWnd::OnButtonSendmsg() 
{
	// TODO: Add your control notification handler code here	

	UpdateData(TRUE);
	char szTemp[20];
	char* pSzBuf = NULL;
	CString strMsg;
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_CHAT_HEADER sChatHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sChatHeader, NULL, sizeof(sChatHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
			
	BSTR bsStr;
	GetSendText(&bsStr) ;
	
	USES_CONVERSION;
	strMsg.Format("%s", W2A(bsStr));
	SysFreeString(bsStr);
	
	if(strMsg.GetLength() ==0)
		return ;
			
	if(m_bAutoAppendMsgUse)
	{
		CString strAutoAppend;
		CINIManager::Instance().GetValue(INI_AUTO_APPEND_MSG, strAutoAppend) ;		
		strMsg += CString("\r\n") ;
		strMsg += strAutoAppend;
	}	
		
	// 성능을 고려해서 Max 전송양을 정하자
	if(strMsg.GetLength() > MAX_CHAT_LEN )
		strMsg = strMsg.Mid(0, MAX_CHAT_LEN);

	int iLength = strMsg.GetLength() + S_CHAT_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, MULTI_CHAT_MSG, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sChatHeader.szIDFrom, CChatSession::Instance().mMyInfo.m_szBuddyID, sizeof(sChatHeader.szIDFrom) ); 
	memcpy(sChatHeader.szIDTo, m_szDlgID, sizeof(sChatHeader.szIDTo) );  // m_szDlgID => ServerRoomNo !

	// 전송데이터 모으기	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, (LPCSTR)strMsg, strMsg.GetLength()  );	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(CChatSession::Instance().Send(pSzBuf, nTotalLen,"MultiChat ") < 0)	
	{
		TRACEX("Send return FAIL!\n");		
		return  ;
	}
		
	m_pREditSendMsg->SetSel(0, -1);
	m_pREditSendMsg->Clear();	 // delete 

	
	int len = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(len, len);
	
	CString cstr;	
	CTime iDT =  CTime::GetCurrentTime();
	m_strlastMsgDate = iDT.Format("%Y년 %m월 %d일 / %H시 %M분 %S초");

	cstr.Format("최종시간 : %s", m_strlastMsgDate);
	m_statusBar.SetWindowText(cstr);
	m_strlastMsgDate.Empty();
	
	CString strMsgTmp;
	strMsgTmp.Format("\r\n%s [%s]\r\n %s\r\n",CChatSession::Instance().mMyInfo.m_szNickName, CChatSession::Instance().mMyInfo.m_szUserName, strMsg);
	
	m_pChatREdit->ReplaceSel(strMsgTmp);	
	m_pChatREdit->HideSelection(TRUE, TRUE);

	int lenAft = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(len, lenAft);	
	
	CHARFORMAT2 cf;
	m_pChatREdit->GetSelectionCharFormat(cf);

	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	strcpy(cf.szFaceName, lf.lfFaceName);

	cf.crTextColor = CChatSession::Instance().m_ColorMe;
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_pChatREdit->SetSelectionCharFormat(cf);      

	m_pChatREdit->HideSelection(TRUE, TRUE); // ! 

	::SendMessage(m_pChatREdit->GetSafeHwnd(), EM_SCROLLCARET, 0,0);	
		
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);

	if(strUseLogFolder == "Y")
	{
		// Download Path		
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strFileName.Format("%s\\대화이력\\",strLogFolderPath);	
	}
	else
	{
		////////////////				
		char szDirTmp[512];
		char szDir[1024];	
		
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
		memset(szDir, 0x00, sizeof(szDir));	
						
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));
		
		int nPos = 0;
		int nLen = strlen(szDirTmp);
		for(int i = nLen; i>=0 ; i--)
		{
			if(szDirTmp[i] == '\\' && nPos <1 )
			{
				szDirTmp[i] = '\0';
				nPos++;
				break;
			}
		}
		
		TRACE("%s\n" , szDirTmp);	
		
		strFileName.Format("%s\\대화이력\\",szDirTmp);	
	}	
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}

	CString strTmp;	
	CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
	CString strtmSimple = iDT.Format("%Y%m%d");

	CString strFileNameAll;		
	strFileNameAll.Format("%s\\%sMultiChat.txt", strFileName, strtmSimple);
	strTmp.Format("%s: %s\n", strtmDetailed, strMsg);	
	writeLogFile(strFileNameAll, "%s\n", strTmp);
}

STDMETHODIMP CMultiChatWnd::GetSendText( BSTR *pbsStr)
{        
    HRESULT hr = S_OK;
    IRichEditOle* pOlePtr = NULL ;
    
	// 한번에 MAX_CHAT_LEN 이상은 허용하지 않는다.
	TCHAR tc[ MAX_CHAT_LEN ]; 
	//char tc[ MAX_CHAT_LEN ]; 

    // 중간 변형에 쓰일 버퍼.
    TCHAR tcInterim[ 50000 ];
	//char tcInterim [ MAX_CHAT_LEN ]; 
	//char tcInterim [ 50000 ]; 

    ZeroMemory( tc , MAX_CHAT_LEN );
    ZeroMemory( tcInterim , sizeof(tcInterim) ); 

    ::SendMessage( m_pREditSendMsg->m_hWnd , EM_GETOLEINTERFACE , 0 , (LPARAM)&pOlePtr );    

    int nCount  = pOlePtr->GetObjectCount(); 

    GETTEXTLENGTHEX ex ;
    //ex.flags = GTL_DEFAULT ; //GTL_USECRLF
	ex.flags = GTL_USECRLF ; 
    ex.codepage = CP_ACP ; 
	
    int nLength = ::SendMessage( m_pREditSendMsg->m_hWnd , EM_GETTEXTLENGTHEX , (WPARAM)&ex , 0 );
    int nFound = 0 ;
    int nLastIndex = 0 ;
    REOBJECT rObject ;
    TEXTRANGE  tr ; 

    for( int index = 0 ; index< nCount   ; index++ )
    {
        ZeroMemory( &rObject, sizeof(REOBJECT));
        rObject.cbStruct = sizeof(REOBJECT);
        rObject.cp = index; 

        hr = pOlePtr->GetObject( index , &rObject , REO_GETOBJ_ALL_INTERFACES );

        if( SUCCEEDED( hr ))
        {
            // first get text .
            if( rObject.cp >= 0 )
            {
                // retrieve string from last index to the current pos minus 1.
                if( nLastIndex != rObject.cp )
                {
                    tr.chrg.cpMin = nLastIndex  ;
                    tr.chrg.cpMax = rObject.cp -1   ;
					//tr.chrg.cpMax = rObject.cp  ;
                    tr.lpstrText  = tcInterim   ; 
                    // OK , Get String.
                    int nLen = ::SendMessage( m_pREditSendMsg->m_hWnd , EM_GETTEXTRANGE , NULL , (LPARAM)&tr );
					
                    // buffer check.
                    if( _tcslen( tc ) + nLen + 5 > MAX_CHAT_LEN )
                        break ; // buffer copy stop.
					
                    // append string.
                    _tcsncat( tc , tcInterim , nLen );
                }
            }

			// modify last index .
			nLastIndex = rObject.cp + 1 ; 
			
			// get emoticon string.
			char  szKey[10+1];
			memset(szKey, 0x00, sizeof(szKey)) ;
			ltoa((DWORD)rObject.pstg , szKey, 10);
				
			CString pa;
			
			if(m_EmoStorageMap.Lookup(szKey, pa ) ) 
			{
				TRACE("이모티콘 [%s]\n", pa);				
			}
			else
			{
				continue ;
			}			
			
			// buffer check.
			if( _tcslen( tc ) + _tcslen( (LPCTSTR) pa  ) > MAX_CHAT_LEN )
			{
				break; 
			}
			
			// append string.
			_tcsncat( tc , (LPCTSTR) pa  , _tcslen( (LPCTSTR) pa  )); 
			
			nFound++;
			
			// copy left strings.
			if( nFound == nCount )
			{
				if( rObject.cp == nLength -1  )
					break ; //더 이상 Copy할필요는 없다.
				
				//그렇지 않으면 복사한다.
				tr.chrg.cpMin = nLastIndex  ;
				tr.chrg.cpMax = nLength -1  ;				
				tr.lpstrText  = tcInterim   ;
				
				// OK , Get String.
				int nLen = ::SendMessage( m_pREditSendMsg->m_hWnd , EM_GETTEXTRANGE , NULL , (LPARAM)&tr );
				
				// buffer check.
				if( _tcslen( tc ) + nLen > MAX_CHAT_LEN )
				{
					_tcsncat( tc , tcInterim , nLen - ( MAX_CHAT_LEN - _tcslen( tc ) ) );  
					break ; // buffer copy stop.
				}
				
				// append string.
				_tcsncat( tc , tcInterim , nLen );  
				
				break;
			}			
        }
    }

    if( nLastIndex == 0 )
    {
        // OK , Get String.
        GETTEXTEX tex ;
        ZeroMemory(&tex , sizeof(GETTEXTEX ));
        tex.cb = MAX_CHAT_LEN ;
        tex.codepage = CP_ACP ;		
        tex.flags = GT_USECRLF ; 

        //::SendMessage( m_pREditSendMsg->m_hWnd , EM_GETTEXTEX , (WPARAM)&tex , (LPARAM)tc );        
		CString strTmp;
		m_pREditSendMsg->GetWindowText(strTmp) ;
		_tcsncpy( tc, (LPCSTR)strTmp, MAX_CHAT_LEN ) ;
    } 

    USES_CONVERSION ;
    
	*pbsStr = SysAllocString( A2W(tc) ); 
	
    return S_OK;

}

void CMultiChatWnd::FileDropped(const char* szFileName, int nLen, int nFileCnt)
{	
	///////////////
	int nSize = m_StringAryID.GetSize();	
	
	CString strID;

	for(int i=0; i < nSize; i++)
	{
		strID = m_StringAryID.ElementAt(i);	
		
		if(strncmp(CChatSession::Instance().mMyInfo.m_szBuddyID, (LPCSTR)strID, sizeof(CChatSession::Instance().mMyInfo.m_szBuddyID)) != 0 )
		{
			CBuddyInfo * pCompanion;
						
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(strID, ( CObject*& ) pCompanion ))
			{
				if(pCompanion)
				{
					TRACE("szFileName [%s]\n",szFileName);
					
					CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr);
					
					if( NULL == pSpckObjMap )
					{					
						TRACE("socket connect fail! [%s]\n", szFileName);	
						
						CString strErrMsg;
						strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
						CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
						pMsgThread->CreateThread();
					}
					else
					{				
						TRACE("\nsocket connected! [%s]\n", szFileName);
						
						CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID,
							CString(pCompanion->m_szIpAddr), 
							(LPSTR)(LPCSTR) szFileName , nFileCnt) ;
					}
				}
			}			
		}		
	}

	// 파일 보낸다.
	/*
	TRACE("szFileName [%s]\n",szFileName);
			
	CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)m_szToIP);
	
	if( NULL == pSpckObjMap )
	{					
		TRACE("socket connect fail! [%s]\n", szFileName);	
								
		CString strErrMsg;
		strErrMsg.Format("%s 에 접속할수 없습니다", m_szToIP);
		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
		pMsgThread->CreateThread();
	}
	else
	{				
		TRACE("\nsocket connected! [%s]\n", szFileName);
		
		CFileTransSock::Instance().InformFileSend( pSpckObjMap, m_pCompanion->m_szBuddyID, 
								CString(m_szToIP), (LPSTR)(LPCSTR) szFileName , nFileCnt) ;
	}
	*/
}

void CMultiChatWnd::OnSelchangeComboReply() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString strSelReply;
	m_pComboReply->GetLBText( m_pComboReply->GetCurSel(), strSelReply ) ;	
	m_pREditSendMsg->SetWindowText(strSelReply);
	
	UpdateData(FALSE);
		
	if(m_bSelectSend)
	{		
		OnButtonSendmsg();
	}

	m_pREditSendMsg->SetFocus();
}

void CMultiChatWnd::OnCheckAlwaysTop() 
{
	// TODO: Add your control notification handler code here
	OnAlwaysOntop() ;
}

void CMultiChatWnd::OnAlwaysOntop() 
{
	// TODO: Add your command handler code here
	if(m_nTopMostFlag == -1)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0 , SWP_NOSIZE | SWP_NOMOVE);    		
	}
	else
	{
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0 ,SWP_NOSIZE | SWP_NOMOVE);    
	}

	m_nTopMostFlag *= -1; // Toggle..
}

void CMultiChatWnd::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	
	UpdateData(TRUE);
	
	//////////////////////////////////////////////////////
		
	if(  1 == m_pChkExitMultiChat->GetCheck() )
	{
		// Multi chat 나가기를 선택한 경우. 
	
		CRect rcChatRect;		
		GetWindowRect(&rcChatRect);
			
		CString strPosChat;
		if(rcChatRect.left < 0 || rcChatRect.top < 0)
		{				
			strPosChat = "450|256|434|373|";
		}
		else
		{
			strPosChat.Format("%ld|%ld|%d|%d|", rcChatRect.left, rcChatRect.top, rcChatRect.Width(), rcChatRect.Height());
		}	

			
		TRACE("strPosChat [%s]\n", strPosChat);
		CINIManager::Instance().SetValue(INI_PREV_POS_SIZE_CHAT, (LPSTR)(LPCSTR)strPosChat);	
		
		if(m_bSelectSend)
		{
			CINIManager::Instance().SetValue(INI_USE_SELECT_SEND, "Y");		
		}
		else
		{
			CINIManager::Instance().SetValue(INI_USE_SELECT_SEND, "N");		
		}
		
		
		if(m_bAutoAppendMsgUse)
		{
			CINIManager::Instance().SetValue(INI_USE_AUTO_APPEND, "Y");		
		}
		else
		{
			CINIManager::Instance().SetValue(INI_USE_AUTO_APPEND, "N");		
		}

		CINIManager::Instance().SaveINIFile();		

		// multi chat map 제거
		CChatSession::Instance().m_mapStr2SetMultiChat.erase(m_szDlgID);	
		
		// 서버로 전송
		CChatSession::Instance().RequestExitMultiChat(m_szDlgID) ;

		pMainWnd->PostMessage(WM_MULTI_CHATDLG_CLOSE, (WPARAM)m_szDlgID, (LPARAM)0);

		if(m_pEmoticonDialog)
		{
			delete m_pEmoticonDialog;
			m_pEmoticonDialog = NULL;
		}

		if(m_pAutoAppendMsgDlg)
		{
			delete m_pAutoAppendMsgDlg;
			m_pAutoAppendMsgDlg = NULL;
		}

		if(m_pBuddySlipMsgDlg)
		{
			delete m_pBuddySlipMsgDlg;
			m_pBuddySlipMsgDlg = NULL;
		}

		if(m_pChatReplyEditDlg)
		{
			delete m_pChatReplyEditDlg;
			m_pChatReplyEditDlg = NULL;
		}
			
		m_FontCtrl.DeleteObject();
	}	
	
	this->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);	
}


void CMultiChatWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	TRACE("CMultiChatWnd => OnNcLButtonDown \n");				
	m_pREditSendMsg->SetFocus();

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CMultiChatWnd::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CWnd::OnActivateApp(bActive, hTask);
	
	// TODO: Add your message handler code here
	TRACE("CMultiChatWnd => OnActivateApp \n");				
}

void CMultiChatWnd::OnChatBuddyOffLine( WPARAM wParam, LPARAM lParam)
{ 
 char szFromID[20+1];
 char szMsg[20+1+10];
 memset(szFromID , 0x00, sizeof(szFromID));
 memset(szMsg , 0x00, sizeof(szMsg));
 
 strncpy(szFromID , (char*) lParam , sizeof(szFromID));  // 상대방 ID
  
 // Tree  
 // 트리에서 OFFLINE 했음으로 변경 
 LVFINDINFO info;
 int nIndex = -1;

 info.flags = LVFI_PARAM;
 info.flags = LVFI_PARTIAL|LVFI_STRING;
 info.psz = szFromID;  
 nIndex = m_pCListCtrlMultiChatID->FindItem(&info);
 
 if(nIndex != -1)
 {
  LVITEM rItem;  
     
  ZeroMemory ( &rItem, sizeof(LVITEM) );
  rItem.iItem = nIndex;
  rItem.mask =  LVIF_IMAGE | LVIF_TEXT;   
  rItem.iSubItem = 0;   
  strcpy(szMsg, "(OffLine)");
  strcat(szMsg, szFromID);
  rItem.pszText = szMsg;
  rItem.iImage = -1;  
  m_pCListCtrlMultiChatID->SetItem(&rItem);

  ZeroMemory ( &rItem, sizeof(LVITEM) );
  rItem.iItem = nIndex;
  rItem.mask =  LVIF_IMAGE | LVIF_TEXT;   
  rItem.iSubItem = 2; 
  rItem.pszText = "OffLine";  
  rItem.iImage = -1;  
  m_pCListCtrlMultiChatID->SetItem(&rItem);   
 }  
}
