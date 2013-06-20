// ChatWnd.cpp : implementation file
//

#include "stdafx.h"
#include "idmsmsn.h"
#include "ChatWnd.h"

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


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int RESOURCE_CHAT_REDIT       = 40001 ;
const int RESOURCE_SPLITTER         = 40002 ;
const int RESOURCE_GRPBOX1          = 40003 ;
const int RESOURCE_GRPBOX2          = 40004 ;
const int RESOURCE_STATIC_REPLY     = 40005 ;
const int RESOURCE_COMBO_REPLY      = 40006 ;
const int RESOURCE_ADDLIST_BUTTON   = 40007 ;
const int RESOURCE_CHK_SEL_SEND_BUTTON  = 40008 ;
const int RESOURCE_CHK_TOP_BUTTON       = 40009 ;
const int RESOURCE_FILE_SEND_BUTTON     = 40010 ;
const int RESOURCE_FILE_SAVE_BUTTON     = 40011 ;
const int RESOURCE_OFFLINE_MSG_BUTTON   = 40012 ;
const int RESOURCE_EMOTICON_BUTTON      = 40013 ;
const int RESOURCE_CHK_USE_SEPLINE_BUTTON    = 40014 ;
const int RESOURCE_CHG_SEPLINE_BUTTON   = 40015 ;
const int RESOURCE_INPUT_CHAT_REDIT     = 40016 ;
const int RESOURCE_SEND_BUTTON          = 40017 ;




/////////////////////////////////////////////////////////////////////////////
// CChatWnd

CChatWnd::CChatWnd()
{
	//m_nType = BITMAP_TOP;	
	m_nType = BITMAP_STRETCH ; //BITMAP_TOP;
	m_nTopMostFlag = -1;

	m_pBtnEmoticon = NULL;
	m_pBtnAddReplyList = NULL;
	m_pBtnChgSepLine = NULL;
	m_pBtnBuddyMsg= NULL;
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

	m_bSelectSend = FALSE;
	m_bAutoAppendMsgUse = FALSE;	
	m_bLastMsgCompanion = FALSE;
	m_bFirstMsgFlag = TRUE;
}


BEGIN_MESSAGE_MAP(CChatWnd, CWnd)
	//{{AFX_MSG_MAP(CChatWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_ACTIVATEAPP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_COMMAND(IDC_CONV_CLOSE, OnConvClose)
	//ON_COMMAND(IDC_COLOR_ME, OnColorMe)
	//ON_COMMAND(IDC_COLOR_COMP, OnColorComp)
	ON_COMMAND(IDC_FILESAVE, OnFilesave)	
	//ON_COMMAND(IDC_FONT, OnFont)
	//ON_COMMAND(IDC_COLOR_BG, OnColorBg)
	ON_COMMAND(IDM_OPTION, OnConfig)	
	
	ON_CBN_SELCHANGE(RESOURCE_COMBO_REPLY, OnSelchangeComboReply)
	ON_COMMAND_RANGE(ID_BUTTON32824, ID_BUTTON32862, OnFaceSelect)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChatWnd message handlers

CChatWnd::CChatWnd( CBuddyInfo * pCompanion, CBuddyInfo * pMyInfo, CWnd* pParent)
{		
	m_pParent = pParent ;
    m_nType = BITMAP_STRETCH ; //BITMAP_TOP;
	m_nTopMostFlag = -1;

	m_pBtnEmoticon = NULL;
	m_pBtnAddReplyList = NULL;
	m_pBtnChgSepLine = NULL;
	m_pBtnBuddyMsg= NULL;
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
	
	////////////////////////////////////
	
	m_bSelectSend = FALSE;
	m_bAutoAppendMsgUse = FALSE;
	m_bLastMsgCompanion = FALSE;
	m_bFirstMsgFlag = TRUE;
	
	m_pCompanion = pCompanion;
	m_pMyInfo = pMyInfo;

	memset(m_szToIP, 0x00, sizeof(m_szToIP));
	memset(m_szMyIP, 0x00, sizeof(m_szMyIP));
	strncpy(m_szToIP, m_pCompanion->m_szIpAddr, sizeof(m_szToIP));
	strncpy(m_szMyIP, m_pMyInfo->m_szIpAddr, sizeof(m_szMyIP));
	
	m_pEmoticonDialog = NULL; //이모티콘
	m_pAutoAppendMsgDlg = NULL;
	m_pBuddySlipMsgDlg = NULL;
	m_pChatReplyEditDlg = NULL;	

}

CChatWnd::~CChatWnd()
{	
	delete m_pBtnEmoticon;
	delete m_pBtnAddReplyList;
	delete	m_pBtnChgSepLine;
	delete	m_pBtnBuddyMsg;
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

	TRACE("CChatWnd::~CChatWnd() \n");
}

void CChatWnd::OnConvClose() 
{
	// TODO: Add your command handler code here
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
	CINIManager::Instance().SaveINIFile();	

	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();

	pMainWnd->PostMessage(WM_CHATDLG_CLOSE, (WPARAM)m_pCompanion->m_szBuddyID, (LPARAM)0);	

	CWnd::OnClose();
}


void CChatWnd::OnConfig() 
{
	char szSetVal[10+1];
	memset(szSetVal, 0x00,sizeof(szSetVal));	
	COptionDlg dlg( (LPCSTR)(CChatSession::Instance().m_Nick),FALSE,this ) ;

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

		m_pChatREdit->SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );		
		m_pREditSendMsg->SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );

		m_pChatREdit->SetFont(&CChatSession::Instance().m_Font);
		m_pChatREdit->SetSel(0,-1);
		
		CHARFORMAT2 cf;
		m_pChatREdit->GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
		cf.crTextColor = CChatSession::Instance().m_ColorBuddy;
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		LOGFONT lf;
		CChatSession::Instance().m_Font.GetLogFont(&lf);		
		strcpy(cf.szFaceName, lf.lfFaceName);
		m_pChatREdit->SetSelectionCharFormat(cf);      
		m_pChatREdit->HideSelection(TRUE,TRUE);

		m_pREditSendMsg->SetFont(&CChatSession::Instance().m_Font);			
	}

	m_pChatREdit->HideSelection(TRUE, TRUE);
	
	m_pREditSendMsg->SetFocus();
	
	m_pParent->PostMessage(WM_CHG_CONFIG, NULL, NULL);	
	
	/*
	 void CEditCtrlEx::lf2cf( LOGFONT& lf, CHARFORMAT2& cf )
{
    cf.dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
    cf.dwEffects &= ~(CFE_PROTECTED | CFE_LINK| CFM_COLOR);
    
    HDC hdc;
    LONG yPixPerInch;
    
    // Set CHARFORMAT structure
    cf.cbSize = sizeof(CHARFORMAT2);
    hdc = ::GetDC(::GetDesktopWindow());
    yPixPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
    cf.yHeight = abs(lf.lfHeight*72*20/yPixPerInch);
    ::ReleaseDC(::GetDesktopWindow(), hdc);
    
    cf.yOffset = 0;
    //cf.crTextColor = 0;
    
    if(lf.lfWeight < FW_BOLD) cf.dwEffects &= ~CFE_BOLD;
    if(!lf.lfItalic) cf.dwEffects &= ~CFE_ITALIC;
    if(!lf.lfUnderline) cf.dwEffects &= ~CFE_UNDERLINE;
    if(!lf.lfStrikeOut) cf.dwEffects &= ~CFE_STRIKEOUT;
    
    cf.dwMask = CFM_ALL | CFM_BACKCOLOR;
    cf.bCharSet = lf.lfCharSet;
    cf.bPitchAndFamily = lf.lfPitchAndFamily;
    
#ifndef UNICODE
    _tcscpy(cf.szFaceName, lf.lfFaceName);
#else
    USES_CONVERSION;
    wcscpy( cf.szFaceName, A2W(lf.lfFaceName));
#endif
}

bool CEditCtrlEx::ft2cf( HFONT font, CHARFORMAT2& cf )
{
    LOGFONT lf;
    // Get LOGFONT for default font
    if (!font)
    font = (HFONT)GetStockObject( SYSTEM_FONT );

    // Get LOGFONT for passed hfont
    if (!GetObject(font, sizeof(LOGFONT), &lf))
        return false;
    
    lf2cf( lf, cf );
    return true;
}

아래는 완벽한 CFontDialog와 CRichEdit와의 관계입니다.
void CEditCtrlEx::DoFontDialog()
{
    CFontDialog dlg;
    if( dlg.DoModal() == IDOK )
    {
        CHARFORMAT2 cf;
        COLORREF cr;
        LOGFONT lf;
        cr = dlg.GetColor();
        memcpy(&lf, dlg.m_cf.lpLogFont, sizeof(LOGFONT));
        lf2cf( lf, cf );
        cf.crTextColor = cr;
        SetFont( cf );
    }
}
	
	 */
}


void CChatWnd::OnFilesave() 
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

void CChatWnd::OnBtnSendFile() 
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
		
	CBuddyInfo*  pCompanion = NULL;	
	
	if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_pCompanion->m_szBuddyID, ( CObject*& ) pCompanion ))
	{
		if(pCompanion->m_bConnStatus == TRUE && ( pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2))
		{
			TRACE("strPath [%s]\n",strPath);
		
			if( strlen(m_pCompanion->m_szIpAddr) == 0 )
			{
				CString strErrMsg;
				strErrMsg.Format("상대방 IP주소를 알수없습니다. \n만약 새로 추가된 대화상대라면, 재 접속후 전송하세요\n(향후 보완 예정 입니다).");
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
				pMsgThread->CreateThread();

				return;
			}

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
}

void CChatWnd::FileDialogOpen(CString &result)
{
	char szFilter[] = "txt Files (*.txt)|*.txt; *.txt|All Files (*.*)|*.*||";
	CFileDialog tmpFileDlg(FALSE  ,"txt",NULL,NULL,szFilter,NULL);
	int rtn = tmpFileDlg.DoModal();
	if(IDCANCEL == rtn)
		return;	
	result = tmpFileDlg.GetPathName();
}

void CChatWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default	
	TRACE("%d / %d \n", point.x, point.y);
	m_pREditSendMsg->SetFocus();
	CWnd::OnLButtonDown(nFlags, point);
}

BOOL CChatWnd::OnEraseBkgnd(CDC* pDC) 
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

BOOL CChatWnd::SetBitmap(UINT uResource, int nType /*BITMAP_TILE*/) 
{
	m_nType = nType;
	
	//return m_bmpBackground.LoadResource(uResource);
	return TRUE;
}

HBRUSH CChatWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if	(
			pWnd->GetDlgCtrlID() == RESOURCE_STATIC_REPLY ||
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_SEL_SEND_BUTTON ||
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_TOP_BUTTON || 
			pWnd->GetDlgCtrlID() == RESOURCE_CHK_USE_SEPLINE_BUTTON
		)
	{		
		//pDC->SetTextColor(RGB(255, 0, 0));		
		pDC->SetBkMode(TRANSPARENT);
		
		return (HBRUSH)::GetStockObject(NULL_BRUSH); 
	}

	return hbr;


	
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
	
	return hbr;
	*/
}

BOOL CChatWnd::OnQueryNewPalette() 
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

void CChatWnd::OnPaletteChanged(CWnd* pFocusWnd) 
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


int CChatWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	// TODO: Add your specialized creation code here
	
	//Menu
	CMenu newMenu;
	newMenu.LoadMenu(IDR_MENU_CONVERSATION);
	SetMenu( &newMenu );	
	DrawMenuBar();
	
	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	DWORD dwResult;  	
	dwResult = GetSysColor(COLOR_3DFACE); 
 
	m_ColorWndBg = RGB( (unsigned int)GetRValue(dwResult), 
		 (unsigned int)GetGValue(dwResult), 
		 (unsigned int)GetBValue(dwResult) ); 
	

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
	//대화창
	m_pChatREdit = new CEmoticonRichEditCtrl;
	m_pChatREdit->SetRole(ROLE_DISPLAY);	
	m_pChatREdit->Create(
					WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_BORDER|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL   ,
					CRect(3,4,409,148-nSplitterUp), this, RESOURCE_CHAT_REDIT );		
	m_pChatREdit->Initialize();	
	m_pChatREdit->SetFont(& CChatSession::Instance().m_Font);	
	m_pChatREdit->setUrlDetection(TRUE);
		
	
	//
	m_pWndSplitter = new CSplitterControl ;
	CRect rc(3,149-nSplitterUp,410,159-nSplitterUp);		
	m_pWndSplitter->Create(WS_CHILD | WS_VISIBLE, rc, this, RESOURCE_SPLITTER );
	m_pWndSplitter->SetRange(100, 100, -1);
	m_pWndSplitter->SetStyle(SPS_HORIZONTAL);
	

	//group box	
	m_pStaticGroup1 = new CButton;
	m_pStaticGroup1->Create(_T(""), WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 
			CRect(3,149-nSplitterUp,409, 189-nSplitterUp), this, RESOURCE_GRPBOX1 );

	m_pStaticGroup2 = new CButton;
	m_pStaticGroup2->Create(_T(""), WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 
			CRect(3,181-nSplitterUp,409, 221-nSplitterUp), this, RESOURCE_GRPBOX2 );
	
	//static 응답
	m_pStaticReply = new CStatic ;
	m_pStaticReply->Create(_T("응답"), WS_CHILD|WS_VISIBLE|SS_CENTER, 
				CRect(5,166-nSplitterUp,40,180-nSplitterUp), this , RESOURCE_STATIC_REPLY );		
	m_pStaticReply->SetFont(&m_FontCtrl,TRUE);	


	//Combo
	m_pComboReply = new CComboBox;
	m_pComboReply->Create(  WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST,
							CRect(40,161-nSplitterUp,190,400-nSplitterUp), this, RESOURCE_COMBO_REPLY);
	m_pComboReply->SetFont(&m_FontCtrl,TRUE);	

	//Addlist Btn
	m_pBtnAddReplyList = new CMyButton;
	m_pBtnAddReplyList->Create(_T("설정"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(192,161-nSplitterUp,225,185-nSplitterUp), this, RESOURCE_ADDLIST_BUTTON);
	m_pBtnAddReplyList->SetFont(&m_FontCtrl);	

	//m_pBtnSendFile
	m_pBtnSendFile = new CMyButton;
	m_pBtnSendFile->Create(_T("파일전송"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(8,192-nSplitterUp,70,217-nSplitterUp), this, RESOURCE_FILE_SEND_BUTTON );	
	m_pBtnSendFile->SetFont(&m_FontCtrl);	

	//m_pBtnSaveFile
	m_pBtnSaveFile = new CMyButton;
	m_pBtnSaveFile->Create(_T("내용저장"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_BORDER, 
					CRect(72, 192-nSplitterUp, 134, 217 - nSplitterUp), this, RESOURCE_FILE_SAVE_BUTTON );
	m_pBtnSaveFile->SetFont(&m_FontCtrl);
	
	//쪽지
	m_pBtnBuddyMsg = new CMyButton;
	m_pBtnBuddyMsg->Create(_T("쪽지"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(74+62, 192-nSplitterUp, 72+128, 217-nSplitterUp), this, RESOURCE_OFFLINE_MSG_BUTTON);
	m_pBtnBuddyMsg->SetFont(&m_FontCtrl);
	
	//Emoticon
	m_pBtnEmoticon = new CMyButton;
	m_pBtnEmoticon->Create(_T("이모티콘"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(74+62+66, 192-nSplitterUp, 72+66+68+62, 217-nSplitterUp), this, RESOURCE_EMOTICON_BUTTON);
	m_pBtnEmoticon->SetFont(&m_FontCtrl);
	
	// m_pChkSelectSend
	m_pChkSelectSend = new CMyButton ;	
	m_pChkSelectSend->Create(_T("응답선택시 전송"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(227,161-nSplitterUp,340,185-nSplitterUp), this, RESOURCE_CHK_SEL_SEND_BUTTON);
	m_pChkSelectSend->SetFont(&m_FontCtrl);
	m_pChkSelectSend->SetIcon( IDI_ICON_CHK_ON, IDI_ICON_CHK_OFF);
	m_pChkSelectSend->DrawBorder(FALSE);	

	//자동부착글
	m_pChkSepLineUse = new CMyButton ;
	m_pChkSepLineUse->Create(_T("자동 부착글"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(280, 192-nSplitterUp, 370, 217-nSplitterUp), this, RESOURCE_CHK_USE_SEPLINE_BUTTON);
	m_pChkSepLineUse->SetFont(&m_FontCtrl);	
	m_pChkSepLineUse->SetIcon(IDI_ICON_CHK_ON,IDI_ICON_CHK_OFF);
	m_pChkSepLineUse->DrawBorder(FALSE);
	

	// m_pChkAlwaysOnTop
	m_pChkAlwaysOnTop = new CMyButton ;	
	m_pChkAlwaysOnTop->Create(_T("항상 위"), WS_CHILD |WS_VISIBLE|BS_AUTOCHECKBOX, 
					CRect(342,164-nSplitterUp,416,183-nSplitterUp), this, RESOURCE_CHK_TOP_BUTTON );	
	m_pChkAlwaysOnTop->SetFont(&m_FontCtrl);
	//m_pChkAlwaysOnTop->SetIcon( IDI_ICON_PIN2, IDI_ICON_PIN1);
	m_pChkAlwaysOnTop->SetIcon(IDI_ICON_CHK_ON,IDI_ICON_CHK_OFF);
	m_pChkAlwaysOnTop->DrawBorder(FALSE);
	m_pChkAlwaysOnTop->EnableBalloonTooltip();


	//자동부착글설정
	m_pBtnChgSepLine = new CMyButton;
	m_pBtnChgSepLine->Create(_T("설정"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |WS_BORDER, 
					CRect(375, 194-nSplitterUp, 410, 215-nSplitterUp), this, RESOURCE_CHG_SEPLINE_BUTTON);
	m_pBtnChgSepLine->SetFont(&m_FontCtrl);
	

	//입력창
	m_pREditSendMsg = new CEmoticonRichEditCtrl;
	m_pREditSendMsg->SetRole(ROLE_SEND_INPUT);		
	m_pREditSendMsg->Create(
					WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_BORDER|WS_EX_ACCEPTFILES|ES_AUTOVSCROLL|
					WS_VSCROLL|WS_HSCROLL|ES_WANTRETURN,
					CRect(3,221-nSplitterUp,360,265), this, RESOURCE_INPUT_CHAT_REDIT );			
	
	m_pREditSendMsg->Initialize();
	m_pREditSendMsg->SetFont(&CChatSession::Instance().m_Font);
	
	m_pREditSendMsg->m_nParentKind = C_CHATDLG;
	m_pREditSendMsg->SendMessage(EM_LIMITTEXT, 10240, 0);
	
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

	
		
	char szTmp[255];
	memset(szTmp, 0x00, sizeof(szTmp));
	sprintf(szTmp, "대화상대 [%s] IP [%s]",m_pCompanion->m_szUserName, m_pCompanion->m_szIpAddr);
	
	SetWindowText(szTmp);

	TRACE("IDMS: CChatMSgDlg::OnInitDialog=>SetParentName [%s]\n", szTmp);
	
	
	m_pChatREdit->SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG);
	m_pREditSendMsg->SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG);
	
			
	SetResize();	
	
	CString strPosSrc, strPosX, strPosY, strWidth, strHight;
	CString strPosLeft, strPosTop, strRight, strBottom, strDelta;
	
	CINIManager::Instance().GetValue(INI_PREV_POS_SIZE_CHAT, strPosSrc);	
	
	nLen = strPosSrc.GetLength();
	nPos = -1;	
	nPos = strPosSrc.Find('|');  strPosX = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strPosY = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strWidth = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strHight = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);

	MoveWindow( atoi((LPCSTR)strPosX),atoi((LPCSTR)strPosY),atoi((LPCSTR)strWidth),atoi((LPCSTR)strHight));
	
	/*
	CINIManager::Instance().GetValue(INI_PREV_POS_CHAT_SPLITTER, strPosSrc);		
	if( !strPosSrc.IsEmpty() )
	{
		nLen = strPosSrc.GetLength();
		nPos = -1;	
		nPos = strPosSrc.Find('|');  strPosLeft = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
		nPos = strPosSrc.Find('|');  strPosTop = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
		nPos = strPosSrc.Find('|');  strRight = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
		nPos = strPosSrc.Find('|');  strBottom = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
		nPos = strPosSrc.Find('|');  strDelta = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
		
		CRect rectSplitter(atoi((LPCSTR)strPosLeft),atoi((LPCSTR)strPosTop),atoi((LPCSTR)strRight),atoi((LPCSTR)strBottom));
		//ScreenToClient(&rectSplitter);
		
		m_pWndSplitter->MoveWindow( &rectSplitter );
		DoResize( atoi((LPCSTR)strDelta) , FALSE) ;
	}
	*/
		
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
	
	UpdateData(FALSE);

	return 0;
}


void CChatWnd::OnBtnEmoticon() 
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

void CChatWnd::SetResize() 
{	
	m_cControlPos.SetParent(this);	
	
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
	m_cControlPos.AddControl(m_pBtnBuddyMsg,  CP_MOVE_VERTICAL	);			
	m_cControlPos.AddControl(m_pBtnEmoticon,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pChkSepLineUse,  CP_MOVE_VERTICAL	);	
	m_cControlPos.AddControl(m_pBtnChgSepLine,  CP_MOVE_VERTICAL	);		
	
	m_cControlPos.AddControl(m_pBtnSnd	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL	);		
}


void CChatWnd::OnBtnAddList() 
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

void CChatWnd::OnBtnChgSepline() 
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

void CChatWnd::OnSize(UINT nType, int cx, int cy) 
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

BOOL CChatWnd::PreTranslateMessage(MSG* pMsg) 
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
				m_pREditSendMsg->HideSelection(TRUE, FALSE);
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

void CChatWnd::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
		
	CWnd::OnTimer(nIDEvent);
}

void CChatWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	// m_pREditSendMsg->SetFocus();
	// TODO: Add your message handler code here
	
}

LRESULT CChatWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == WM_NOTIFY)
	{
		if (wParam == RESOURCE_SPLITTER )
		{	
			SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
			DoResize(pHdr->delta, TRUE);
		}
	}	
	return CWnd::DefWindowProc(message, wParam, lParam);
}

void CChatWnd::DoResize(int delta, BOOL bFromDefProc) // delta -> Y 변동값!!
{	
	CSplitterControl::ChangeHeight(m_pChatREdit, delta);
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
	CSplitterControl::ChangePos(m_pBtnBuddyMsg,0 , delta ) ;
	CSplitterControl::ChangePos(m_pBtnEmoticon,0 , delta ) ;    
		
	Invalidate();
	UpdateWindow();	
	
	/*
	if(bFromDefProc)
	{
		CRect rcSplitter;	
		m_pWndSplitter->GetWindowRect( &rcSplitter ) ;
		ScreenToClient( &rcSplitter);
		
		TRACE( "left [%d] top [%d] width [%d] right [%d] bottom [%d]\n", 
			rcSplitter.left, rcSplitter.top, rcSplitter.right, rcSplitter.bottom , delta);
		
		CString strPosSplitter;
		if(rcSplitter.left < 0 || rcSplitter.top < 0)
		{				
			strPosSplitter = "3|149|410|159|0|";
		}
		else
		{
			strPosSplitter.Format("%ld|%ld|%ld|%ld|%d|", rcSplitter.left, rcSplitter.top, rcSplitter.right, rcSplitter.bottom, delta );
		}	
		TRACE("strPosSplitter [%s]\n", strPosSplitter);
		CINIManager::Instance().SetValue(INI_PREV_POS_CHAT_SPLITTER, (LPSTR)(LPCSTR)strPosSplitter);	
		CINIManager::Instance().SaveINIFile();
	}
	*/
}

void CChatWnd::OnBtnBuddyMsg() 
{
	// TODO: Add your control notification handler code here	
	
	CBuddyInfo*  pCompanion = NULL;	
	if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_pCompanion->m_szBuddyID, ( CObject*& ) pCompanion ))
	{
		if(pCompanion->m_bConnStatus == TRUE )
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"OffLine 인 경우만 쪽지를 남길수 있습니다.");
			pMsgThread->CreateThread();				
			
			return;
		}
	}

	if(m_pBuddySlipMsgDlg)
	{
		delete m_pBuddySlipMsgDlg;
		m_pBuddySlipMsgDlg = NULL;
	}	
	
	m_pBuddySlipMsgDlg = new CBuddySlipMsg(m_pCompanion->m_szBuddyID, m_pCompanion->m_szUserName);
	
	if(m_pBuddySlipMsgDlg != NULL)
	{		
		BOOL ret = m_pBuddySlipMsgDlg->Create(IDD_DLG_WRITE_MSG, this);
		
		if(!ret)
		{
			return ;
		}
		
		m_pBuddySlipMsgDlg->ShowWindow(SW_SHOW);
		
		CButton * pWndBtn = (CButton *)GetDlgItem(RESOURCE_OFFLINE_MSG_BUTTON);	
		CRect	rWnd;
		pWndBtn->GetWindowRect(rWnd);
		m_pBuddySlipMsgDlg->MoveWindow(rWnd.left+10,rWnd.top-150, 440, 370);	
	}
}

void CChatWnd::OnFaceSelect(UINT nID)
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


LRESULT CChatWnd::OnEmoticonMap(WPARAM wParam, LPARAM lParam)
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

LRESULT CChatWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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
		else if(strBtnText == "쪽지")
		{			
			OnBtnBuddyMsg() ;		
		}
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
		else if(strBtnText == "응답선택시 전송")
		{
			TRACE("ddddd");
		}
		else if(strBtnText == "자동 부착글")
		{
			TRACE("eeeeeee");
		}
	}
	else if (message == WM_MOUSEACTIVATE )
	{
		TRACE("CChatWnd => WM_MOUSEACTIVATE \n");				
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
	else if (message == WM_BUDDY_ONLINE || message == WM_SERVER_RE_CONNECTED)
	{		
		OnChatResume();		
	}
	else if (message == WM_BUDDY_OFFLINE )
	{		
		OnChatBuddyOffLine();		
	}
	//else if (message == WM_CHG_CONFIG )
	//{		
	//	OnApplyConfig();		
	//}	
	
	return CWnd::WindowProc(message, wParam, lParam);
}



void CChatWnd::OnChatBuddyOffLine()
{	
	m_pREditSendMsg->SetWindowText("OffLine 대화상대에게 메시지를 보낼수 없습니다");
	m_pREditSendMsg->EnableWindow(FALSE);
	m_pBtnSnd->EnableWindow(FALSE);		
}

void CChatWnd::OnServerClosed()
{	
	m_pREditSendMsg->SetWindowText("서버 연결이 종료되었습니다.");
	m_pREditSendMsg->EnableWindow(FALSE);
	m_pBtnSnd->EnableWindow(FALSE);	
}

void CChatWnd::OnChatResume()
{		
	m_pREditSendMsg->SetWindowText("");
	m_pREditSendMsg->EnableWindow(TRUE);	
	m_pBtnSnd->EnableWindow(TRUE);	
}

LRESULT CChatWnd::OnChatMsg(WPARAM wParam, LPARAM lParam)
{
	S_CHAT_CLIENT * pChatMsg = (S_CHAT_CLIENT*) lParam;
	CString strMsg;
		
	int nStart = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(nStart, nStart);		
	
	m_pChatREdit->ReplaceSel("\r\n");

	if(m_bLastMsgCompanion == FALSE)
	{
		m_bLastMsgCompanion = TRUE;

		if(m_bFirstMsgFlag)
		{
			m_bFirstMsgFlag = FALSE;
			strMsg.Format("%s [%s]\r\n   %s", m_pCompanion->m_szNickName, m_pCompanion->m_szUserName,  pChatMsg->strMsg);				
		}
		else
		{
			strMsg.Format("\r\n%s [%s]\r\n   %s", m_pCompanion->m_szNickName, m_pCompanion->m_szUserName,  pChatMsg->strMsg);	
		}
	}
	else
	{		
		strMsg.Format("\r\n   %s",  pChatMsg->strMsg);			
	}
	
	m_pChatREdit->ReplaceSel(strMsg);

	int lenAft = m_pChatREdit->GetWindowTextLength();
	m_pChatREdit->SetSel(nStart, lenAft);	
	
	CHARFORMAT2 cf;
	m_pChatREdit->GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
	cf.crTextColor = CChatSession::Instance().m_ColorBuddy;
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	
	strcpy(cf.szFaceName, lf.lfFaceName);

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
	strFileNameAll.Format("%s\\%s [%s].txt", strFileName, strtmSimple, m_pCompanion->m_szUserName);
	strTmp.Format("%s: %s\n", strtmDetailed, strMsg);	
	writeLogFile(strFileNameAll, "%s\n", strTmp);	
	//Log		
	
	FlashWindow(TRUE);		

	//WINDOWPLACEMENT Stpl;
	//GetWindowPlacement(&Stpl);
	//if (Stpl.showCmd == SW_SHOWMINIMIZED ) // minimized 상태에서만 : focus위함
	//{
	//	FlashWindow(TRUE);		
	//}
    
  
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

void CChatWnd::DoDataExchange(CDataExchange* pDX) 
{
	// TODO: Add your specialized code here and/or call the base class
	DDX_Check(pDX, RESOURCE_CHK_SEL_SEND_BUTTON, m_bSelectSend);
	DDX_Check(pDX, RESOURCE_CHK_USE_SEPLINE_BUTTON, m_bAutoAppendMsgUse);
	
	CWnd::DoDataExchange(pDX);
}

void CChatWnd::SetReplyList()
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


void CChatWnd::OnButtonSendmsg() 
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
			
	//strMsg.Empty();
	//GetSendText(strMsg) ;

	BSTR bsStr;
	GetSendText(&bsStr) ;
	
	USES_CONVERSION;
	strMsg.Format("%s", W2A(bsStr));
	SysFreeString(bsStr);

	/*
	int BSTRToLocal(LPTSTR pLocal, BSTR pWide, DWORD dwChars)
	{
		*pLocal = 0;
		WideCharToMultiByte( CP_ACP, 0, pWide, -1, pLocal, dwChars, NULL, NULL );
		return lstrlen(pLocal);
	}

	int LocalToBSTR(BSTR pWide, LPTSTR pLocal, DWORD dwChars)
	{
		*pWide = 0;
		MultiByteToWideChar( CP_ACP, 0, pLocal, -1, pWide, dwChars );
		return lstrlenW(pWide);
	}

	//////////

	BSTR bstr;
	CString strValue;

	// BSTR 변환
	USES_CONVERSION;
	strValue.Format("%s", W2A(bstr));
	SysFreeString(bstr);
	*/
	
	if(strMsg.GetLength() ==0)
		return ;
			
	BOOL bCar = m_bLastMsgCompanion;

	if(m_bAutoAppendMsgUse)
	{
		CString strAutoAppend;
		CINIManager::Instance().GetValue(INI_AUTO_APPEND_MSG, strAutoAppend) ;
		strMsg += CString("\r\n") ;
		strMsg += strAutoAppend;
		
		m_bLastMsgCompanion = FALSE;
	}	
	else
	{
		m_bLastMsgCompanion = FALSE;		
	}
	

	// 성능을 고려해서 Max 전송양을 정하자
	if(strMsg.GetLength() > MAX_CHAT_LEN )
		strMsg = strMsg.Mid(0, MAX_CHAT_LEN);

	int iLength = strMsg.GetLength() + S_CHAT_HEADER_LENGTH ; 
		
	memcpy(sComHeader.m_szUsage, CHAT_MSG, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	//memcpy(sFromIDHeader.m_szIP, m_pMyInfo->m_szIpAddr, sizeof(sFromIDHeader.m_szIP) ); 
	memcpy(sChatHeader.szIDFrom, m_pMyInfo->m_szBuddyID, sizeof(sChatHeader.szIDFrom) ); 
	memcpy(sChatHeader.szIDTo, m_pCompanion->m_szBuddyID, sizeof(sChatHeader.szIDTo) ); 
	
	// 전송데이터 모으기	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, (LPCSTR)strMsg, strMsg.GetLength()  );	
	nTotalLen = COMMON_HEADER_LENGTH + iLength;
	
	if(CChatSession::Instance().Send(pSzBuf, nTotalLen,"002 Chat") < 0)	
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
	//strMsgTmp.Format("\n%s [%s]\n %s\r\n",m_pMyInfo->m_szNickName, m_pMyInfo->m_szUserName, strMsg);

	if(bCar)
	{
		strMsgTmp.Format("\r\n\r\n%s [%s]\n   %s",m_pMyInfo->m_szNickName, m_pMyInfo->m_szUserName, strMsg);
	}
	else
	{
		//strMsgTmp.Format("\r\n%s [%s]\n   %s",m_pMyInfo->m_szNickName, m_pMyInfo->m_szUserName, strMsg);
		if(m_bAutoAppendMsgUse)
		{
			if(m_bFirstMsgFlag)
			{
				m_bFirstMsgFlag = FALSE;
				strMsgTmp.Format("\r\n%s [%s]\r\n   %s",m_pMyInfo->m_szNickName, m_pMyInfo->m_szUserName, strMsg);				
			}
			else
			{
				strMsgTmp.Format("\r\n   %s", strMsg);
			}
		}
		else
		{
			if(m_bFirstMsgFlag)
			{				
				m_bFirstMsgFlag = FALSE;
				strMsgTmp.Format("\r\n\r\n%s [%s]\r\n   %s",m_pMyInfo->m_szNickName, m_pMyInfo->m_szUserName, strMsg);				
			}
			else
			{
				strMsgTmp.Format("\r\n\r\n   %s", strMsg);
			}
		}
	}
	
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
		
	/////////////////////////////////////////////////////////
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
	strFileNameAll.Format("%s\\%s [%s].txt", strFileName, strtmSimple, m_pCompanion->m_szUserName );
	strTmp.Format("%s: %s\n", strtmDetailed, strMsg);	
	writeLogFile(strFileNameAll, "%s\n", strTmp);
	
}

STDMETHODIMP CChatWnd::GetSendText( /*CString& strMsg*/ BSTR *pbsStr)
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

void CChatWnd::FileDropped(const char* szFileName, int nLen, int nFileCnt)
{	
	///////////////
	// 파일 보낸다.
	TRACE("szFileName [%s]\n",szFileName);
		
	if( strlen(m_szToIP) == 0 )
	{
		CString strErrMsg;
		strErrMsg.Format("상대방 IP주소를 알수없습니다. \n만약 새로 추가된 대화상대라면, 재 접속후 전송하세요\n(향후 보완 예정 입니다).");
		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
		pMsgThread->CreateThread();

		return;
	}

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
}

void CChatWnd::OnSelchangeComboReply() 
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

void CChatWnd::OnCheckAlwaysTop() 
{
	// TODO: Add your control notification handler code here
	OnAlwaysOntop() ;
}

void CChatWnd::OnAlwaysOntop() 
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


void CChatWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	TRACE("CChatWnd => OnNcLButtonDown \n");				
	m_pREditSendMsg->SetFocus();

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CChatWnd::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CWnd::OnActivateApp(bActive, hTask);
	
	// TODO: Add your message handler code here
	TRACE("CChatWnd => OnActivateApp \n");					
}

void CChatWnd::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	


	CWnd::PostNcDestroy();
}

void CChatWnd::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
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
	
	//m_bSelectSend
	if( m_pChkSelectSend->GetCheck() )
	{
		CINIManager::Instance().SetValue(INI_USE_SELECT_SEND, "Y");		
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_SELECT_SEND, "N");		
	}
	
	//m_bAutoAppendMsgUse
	if(m_pChkSepLineUse->GetCheck() )
	{
		CINIManager::Instance().SetValue(INI_USE_AUTO_APPEND, "Y");		
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_AUTO_APPEND, "N");		
	}
		

	CINIManager::Instance().SaveINIFile();	

	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();

	pMainWnd->PostMessage(WM_CHATDLG_CLOSE, (WPARAM)m_pCompanion->m_szBuddyID, (LPARAM)0);	
	
	CWnd::OnClose();
}
