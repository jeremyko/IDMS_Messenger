/////////////////////////////////////////////////////////////////////////////
// 다음 공개소스를 수정해서 사용함
// Syntax coloring Editor
// http://www.codeguru.com/cpp/controls/richedit/syntaxhilighting/article.php/c2387/

// Copyright (C) 1998 by Juraj Rojko jrojko@twist.cz
// All rights reserved
// TWScriptEdit.cpp : implementation file

/*
How to use this control: 
	Insert a RICHEDIT control into a dialog resource. Add a member variable to appropriate the CDialog or 
	CFormView derived class and subclass the control in the OnInitDialog function. 
	Or create the CEmoticonRichEditCtrl control then you need it. 
	
	To minimize any flickering set the style of the parent window (usually the dialog) to WS_CLIPCHILDREN. 
	Call the Initialize() method of the CEmoticonRichEditCtrl. (This sets default attributes of the text). 

Comments

 The Interesting code sits in the CEmoticonRichEditCtrl control (TWScriptEdit.h and TWScriptEdit.cpp). 
 This control is derived from CRichEditCtrl. The whole miracle occurs in the EN_PROTECTED and EN_CHANGE 
 notification handlers. All text in the control has the CFE_PROTECTED flag, 
 so before any change the OnProtected handler is called. After the change, the OnChange handler is called. 
 This enables us to determine the affected lines that need reformatting. Real formatting occurs in FormatTextRange() 
 which parses the range of text and changes necessary attributes by calling helper functions. 
 The control correctly handles WM_SETTEXT and EM_RELACELSEL, so it's possible to 
 change text programmatically.
 */


#include "stdafx.h"
#include "resource.h"
#include "EmoticonRichEditCtrl.h"
#include "ImageDataObject.h" //이모티콘 
#include "..\\common\\AllCommonDefines.h"	
//#include "ChatMSgDlg.h"
#include "ChatWnd.h"
#include "Inimanager.h"
#include "MultiChatWnd.h"
#include "AllMsgDlg.h"
#include "imm.h"
#pragma comment(lib,"imm32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR szEmoticon = " ec00 ec01 ec02 ec03 ec04 ec05 ec06 ec07 ec08 ec09 ec10 "
"ec11 ec12 ec13 ec14 ec15 ec16 ec17 ec18 ec19 ec20 "
"ec21 ec22 ec23 ec24 ec25 ec26 ec27 ec28 ec29 ec30 "
"ec31 ec32 ec33 ec34 ec35 ec36 ec37 ec38 ec39 " ; //HOME

/////////////////////////////////////////////////////////////////////////////
// CEmoticonRichEditCtrl

CEmoticonRichEditCtrl::CEmoticonRichEditCtrl()
{
	memset(m_szParentName,0x00,sizeof(m_szParentName));
	m_strEmoticon = szEmoticon;
	m_bInForcedChange = FALSE;
	m_changeType = ctUndo;
	m_crOldSel.cpMin = m_crOldSel.cpMax = 0;	
	
}

CEmoticonRichEditCtrl::~CEmoticonRichEditCtrl()
{
	m_pRichEditOle->Release();
	m_bmpFaces.DeleteObject();	
}

void CEmoticonRichEditCtrl::Initialize() 
{
	PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);
	pf.dwMask = PFM_TABSTOPS ;
	pf.cTabCount = MAX_TAB_STOPS;
	for( int itab = 0 ; itab < pf.cTabCount  ; itab++ )
		pf.rgxTabs[itab] = (itab + 1) * 1440/5 ;

	SetParaFormat( pf );		

	SetEventMask(ENM_CHANGE | ENM_SELCHANGE | ENM_PROTECTED);

	//이모티콘		
	m_imgListFaces.Create(20, 20, ILC_COLOR24|ILC_MASK, 0, 1);	
	m_bmpFaces.LoadBitmap(IDB_BITMAP_EMOTICON);
	m_imgListFaces.Add(& m_bmpFaces, RGB(255, 255, 255));	

	m_pRichEditOle = GetIRichEditOle();

	DragAcceptFiles(TRUE);

	/*
	if(m_nRole == ROLE_SEND_INPUT )
	{	
		CHARFORMAT2 cf;
		GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE |CFM_PROTECTED ;		
		cf.crTextColor =  CChatSession::Instance().m_ColorMe;			
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		SetDefaultCharFormat(cf);			
	}
	*/
	
}


void CEmoticonRichEditCtrl::SetParentName(LPCTSTR lpszParentTitle)
{
	strncpy(m_szParentName , lpszParentTitle , sizeof(m_szParentName));
	TRACE("SetParentName 호출 [%s]\n", m_szParentName );
}

LPCTSTR CEmoticonRichEditCtrl::GetParentName()
{
	return m_szParentName ;
}

void CEmoticonRichEditCtrl::AddEmoticon(LPCTSTR lpszKwd)
{
	m_strEmoticon = m_strEmoticon + lpszKwd;	
}


BEGIN_MESSAGE_MAP(CEmoticonRichEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(CEmoticonRichEditCtrl)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(EN_PROTECTED, OnProtected)
	ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_SETFOCUS, OnSetFocus)
	ON_MESSAGE(WM_KILLFOCUS, OnKillFocus)

	ON_MESSAGE(WM_LBUTTONDOWN, OnLButtonDown)
	
	ON_NOTIFY_REFLECT(EN_LINK, whenLinkClick)
	//ON_COMMAND_RANGE(ID_BUTTON32824, ID_BUTTON32862, OnFaceSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmoticonRichEditCtrl message handlers

void CEmoticonRichEditCtrl::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
	BOOL bWorking = FALSE;
	char szFileName[2048];
	memset(szFileName, 0x00, sizeof(szFileName));	
	m_strFileAll = ""; //init;
	m_nFileCnt = 0;
	CString strRootPath = "";
	int nFiles;		
	// 드롭된 파일의 갯수
	// 파일 사이즈가 0 인것은 제외해야함..
	nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, szFileName, 2048);
	
	if(nFiles == 1)
	{
		//파일하나혹은 폴더한개만
		DragQueryFile(hDropInfo, 0, szFileName, 2048);		
		CFileFind fileFind;	
		bWorking = fileFind.FindFile(szFileName);	
		if(bWorking )
		{
			bWorking = fileFind.FindNextFile();		
			
			{
				// C:\\TEST\\DOC 라면 C:\\TEST 를 ROOT 로 한다.		
				CString strTmpPath(szFileName);
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll
			}
		}
		else
		{
			DWORD nErr = GetLastError();
			TRACE("nErr [%d]\n", nErr);

			if(ERROR_INVALID_NAME == nErr)
			{
				return ;
			}
		}

		m_strFileAll = strRootPath + "|" + m_strFileAll  ;// RootPath|파일경로
		TRACE("Drop File Name Len : [%d] Cnt [%d]\n", m_strFileAll.GetLength(), m_nFileCnt);

		if(m_nParentKind == C_CHATDLG)
		{
			CChatWnd *pWnd = (CChatWnd *)GetParent();
			pWnd->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		else 
		if(m_nParentKind == C_MULTICHATDLG)
		{
			CMultiChatWnd *pDlg = (CMultiChatWnd *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		else 
		if(m_nParentKind == C_ALLMSGDLG)
		{
			CAllMsgDlg *pDlg = (CAllMsgDlg *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		
	}
	else
	{	
		// 파일 + 폴더 여러개
		CString strfilesAll;
		CString strfiles;
		for(int index=0 ; index < nFiles ; index++)
		{				
			DragQueryFile(hDropInfo, index, szFileName, 2048);     
			CFileFind fileFind;
			//FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll			
			bWorking = fileFind.FindFile(szFileName);	
			if(bWorking )
			{
				bWorking = fileFind.FindNextFile();		
				
				// C:\\TEST\\DOC 라면 C:\\TEST 를 ROOT 로 한다.		
				CString strTmpPath (szFileName);
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll				
			}
			else
			{
				DWORD nErr = GetLastError();
				TRACE("nErr [%d]\n", nErr);
				
				if(ERROR_INVALID_NAME == nErr)
				{
					return ;
				}
			}
		}

		m_strFileAll = strRootPath + "|" + m_strFileAll  ; // RootPath|파일경로
		TRACE("Drop File Name Len : [%d] Cnt [%d]\n", m_strFileAll.GetLength(), m_nFileCnt);
		if(m_nParentKind == C_CHATDLG)
		{
			CChatWnd *pDlg = (CChatWnd *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);			
		}
		else 
		if(m_nParentKind == C_MULTICHATDLG)
		{
			CMultiChatWnd *pDlg = (CMultiChatWnd *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
	}
	
	CRichEditCtrl::OnDropFiles(hDropInfo);
}


void CEmoticonRichEditCtrl::FileFindRecurse(LPCTSTR pstr)
{	
	BOOL bWorking = FALSE;
	CFileFind finder;
	

	bWorking = finder.FindFile(pstr);	

	if(bWorking )
	{
		bWorking = finder.FindNextFile();		

		if (finder.IsDots() == FALSE && finder.IsDirectory()==FALSE)
		{
			TRACE("file\n");			
			m_strFileAll =  m_strFileAll + CString(pstr) + "|";
			m_nFileCnt++;
		}
		else 
		if (finder.IsDots() == TRUE || finder.IsDirectory()==TRUE)		
		{			
			CString strWildcard(pstr);
			strWildcard += _T("\\*.*");			
			
			bWorking = finder.FindFile(strWildcard);
			
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
												
				if (finder.IsDots())
					continue;
				
				// if it's a directory, recursively search it

				if (finder.IsDirectory())
				{	
					// 폴더이면
					CString str = finder.GetFilePath();
					FileFindRecurse(str);
				}else{
					// 피일이면 (구하는 문자열)
					CString str = finder.GetFilePath();
					m_strFileAll = m_strFileAll + str + "|";// '|' 구분자는 파일경로단위
					m_nFileCnt++;
				}
			} // end of While			
		}		
	}
	finder.Close();
	
}

void CEmoticonRichEditCtrl::setUrlDetection(BOOL bUse)
{
	SetEventMask(GetEventMask() | ENM_LINK );
	SendMessage( EM_AUTOURLDETECT, bUse, NULL);
}

void CEmoticonRichEditCtrl::whenLinkClick( NMHDR * pNotifyStruct, LRESULT * result )
{		
	if( ((ENLINK*)pNotifyStruct)->msg  == WM_LBUTTONDOWN )		
	{		
		TRACE("CEmoticonRichEditCtrl::whenLinkClick\n");
		TCHAR szURL[1024] = {0};		
		//char szURL[1024] = {0};		
		
		SendMessage( EM_EXSETSEL, 0, (LPARAM)(&((ENLINK*)pNotifyStruct)->chrg));		
		SendMessage( EM_GETSELTEXT, 0, (LPARAM)szURL);		
		ShellExecute(NULL, "open", "iexplore.exe", szURL, NULL, SW_SHOWNORMAL);		
	}
}

UINT CEmoticonRichEditCtrl::OnGetDlgCode() 
{
	UINT uCode = CRichEditCtrl::OnGetDlgCode();
	
	uCode = DLGC_WANTALLKEYS | DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTMESSAGE | DLGC_WANTTAB;

	return uCode;
}

void CEmoticonRichEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{	
	if (nChar == '\t' && GetKeyState(VK_CONTROL) >= 0) 
	{
		ReplaceSel("\t");
		return;
	}
	
	CRichEditCtrl::OnChar(nChar, nRepCnt, nFlags);
}

void CEmoticonRichEditCtrl::SetHan(BOOL bFlag)
{
	HIMC hIMC;

	hIMC = ImmGetContext(m_hWnd);
	if( hIMC == NULL )
		return;

	if( ImmSetConversionStatus(hIMC, bFlag, IME_SMODE_NONE) == 0 )
		return;

	if( ImmReleaseContext(m_hWnd, hIMC) == 0 )
		return;
}

int CEmoticonRichEditCtrl::IsEmoticon(LPCTSTR lpszSymbol)
{
	CString strSymbol; 
	strSymbol.Format(" %s ", lpszSymbol);	
	
	TRACE("IsEmoticon [%s]\n", strSymbol );
	
	return m_strEmoticon.Find(strSymbol);
}

void CEmoticonRichEditCtrl::SetFormatRange(int nStart, int nEnd, BOOL bBold, COLORREF clr)
{
	if (nStart >= nEnd)
		return;

	SetSel(nStart, nEnd);

	DWORD dwEffects = bBold?CFE_BOLD:0;

	CHARFORMAT cfm;
	cfm.cbSize = sizeof(cfm);
    GetSelectionCharFormat(cfm);
	
	if ((cfm.dwMask & CFM_COLOR)  && cfm.crTextColor == clr && 
		(cfm.dwMask & CFM_BOLD) && (cfm.dwEffects & CFE_BOLD) == dwEffects)
		return;

	cfm.dwEffects = dwEffects;
	cfm.crTextColor = clr;
	cfm.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;

	SetSelectionCharFormat(cfm);
}

/*
void CEmoticonRichEditCtrl::SetMeTextColor(COLORREF colorMe)
{
	m_ColorMe = colorMe ;
	if(m_nRole == ROLE_SEND_INPUT )
	{	
		CHARFORMAT2 cf;
		GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;	
		cf.crTextColor =  m_ColorMe;			
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		SetSelectionCharFormat(cf);
	}	
}
*/

/*
#ifdef UNICODE
	mdata->SetWideString((LPCTSTR)string);//SK: modified for dynamic allocation
#else
	mdata->SetAnsiString(string);
#endif	

  void CMyButtonMenuData::SetAnsiString(LPCSTR szAnsiString)
{
	USES_CONVERSION;
	SetWideString(A2W(szAnsiString));  //SK:  see MFC Tech Note 059
}

  void CMyButtonMenuData::SetWideString(const wchar_t *szWideString)
{
	delete[] m_szMenuText;//Need not check for NULL because ANSI X3J16 allows "delete NULL"
	
	if (szWideString)
    {
		m_szMenuText = new wchar_t[sizeof(wchar_t)*(wcslen(szWideString)+1)];
		if (m_szMenuText)
			wcscpy(m_szMenuText,szWideString);
    }
	else
		m_szMenuText=NULL;//set to NULL so we need not bother about dangling non-NULL Ptrs
}
 */

void CEmoticonRichEditCtrl::FormatTextRange(int nStart, int nEnd)
{	
	BOOL bEmoticon = FALSE;

	if (nStart >= nEnd)
	{
		TRACE("\nreturn!!\n");
		return;
	}

	m_bInForcedChange = TRUE;

	CHARRANGE crOldSel;
	
	GetSel(crOldSel);	

	TRACE("crOldSel MIN [%d] MAX [%d] \n", crOldSel.cpMin, crOldSel.cpMax );
			
	HideSelection(TRUE, FALSE);
	
	TCHAR *pBuffer = NULL;
	//char *pBuffer = NULL;	

	try 
	{
		TRACE("FormatTextRange : nStart [%d] nEnd [%d] \n", nStart, nEnd);

		SetSel(nStart, nEnd);
								
		pBuffer = new TCHAR[nEnd - nStart + 1];
		//pBuffer = new char[nEnd - nStart + 1];

		long nLen = GetSelText(pBuffer);
		pBuffer[nLen] = 0;

		TRACE("new [%d] pBuffer [%S]\n", nEnd - nStart + 1, pBuffer);
		//ASSERT(nLen <= nEnd - nStart);	

		TCHAR *pStart, *pPtr, *pSymbolStart ;		
		//char* pStart = NULL, *pPtr = NULL;		
		//char* pSymbolStart = NULL;

		pStart = pPtr = pBuffer;
		
		while (*pPtr != 0) 
		{			
			TCHAR ch = *pPtr;					
			//char ch = *pPtr;					
			
			if ( _istalpha(ch) || ch == '_') 
			{ 
				pSymbolStart = pPtr;
				
				do 
				{
					ch = *(++pPtr);
				} 
				while (_istalnum(ch) || ch == '_');

				*pPtr = 0;
				
				//TRACE("pSymbolStart  [%s]\n", pSymbolStart );	
								
				int nPos = IsEmoticon(pSymbolStart);
				
				if (nPos >= 0) 
				{					
					bEmoticon = TRUE;					
		
					SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG);					
		
					//TRACE("이모티콘!!! [%s]\n", pSymbolStart );
					
					SetSel(nStart + pSymbolStart - pBuffer, nStart + pPtr - pBuffer  );
					
					ReplaceSel("  ");					
										
					TRACE("이모티콘영역 [%d] [%d]\n", nStart + pSymbolStart - pBuffer, nStart + pPtr - pBuffer );	
					
					CString strTmp(pSymbolStart);
					int nIndex = atoi( (LPCSTR)strTmp.Mid(2,2) ) ;
					
					HBITMAP hBitmap = GetImage( m_imgListFaces , nIndex );

					if (hBitmap)
					{
						CString strOutID;
						strOutID.Empty();
						CImageDataObject::InsertBitmap(m_pRichEditOle, hBitmap, strOutID ); // strTmp => ec01 , strOutID = 12232132321 

						if(m_nRole == ROLE_SEND_INPUT )
						{
							char* pszID = new char[10+1];
							char* pszVal = new char[10+1];
							strncpy(pszID, (LPCSTR)strOutID, 10);
							strcpy(pszVal, pSymbolStart);
							
							TRACE("WM_EMOTICON_MAP : pszID [%s] pszVal [%s]\n", pszID, pszVal );
							GetParent()->SendMessage( WM_EMOTICON_MAP, (WPARAM) pszID , (LPARAM) pszVal) ;
						}						
					}

					ReplaceSel(" ");
					
					pStart = pPtr;
					pSymbolStart = 0;
										
				}
				else
				{
					pSymbolStart = NULL;
				}
				
				*pPtr = ch;
			}
			else 
			{
				pPtr++;
			}
		}
		
	} 
	catch(...)
	{
		//delete [] pBuffer;
		//pBuffer = NULL ;
	}	
	
	delete [] pBuffer;
	
	if(m_nRole == ROLE_SEND_INPUT )
	{	
		CHARFORMAT2 cf;
		GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE    ; 
		cf.crTextColor =  CChatSession::Instance().m_ColorMe;			
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		SetSelectionCharFormat(cf);
	}
	
	SetSel(crOldSel);
	
	if(m_nRole == ROLE_SEND_INPUT )
	{	
		CHARFORMAT2 cf;
		GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE    ; 
		cf.crTextColor =  CChatSession::Instance().m_ColorMe;			
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		SetSelectionCharFormat(cf);
	}
	

	HideSelection(FALSE, FALSE);
		
	//UnlockWindowUpdate();		
			
	SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG );	
	
	m_bInForcedChange = FALSE;
}

void CEmoticonRichEditCtrl::FormatTextLines(int nLineStart, int nLineEnd)
{		
	long nStart = LineIndex(LineFromChar(nLineStart));
	long nEnd = LineIndex(LineFromChar(nLineEnd));
	nEnd += LineLength(nLineEnd);

	FormatTextRange(nStart, nEnd);
}


void CEmoticonRichEditCtrl::FormatAll()
{
	FormatTextRange(0, GetTextLength());
}

LRESULT CEmoticonRichEditCtrl::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT res = Default();
	
	FormatAll();
	return res;	
}


LRESULT CEmoticonRichEditCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{		
	CRichEditCtrl::OnLButtonDown(wParam, lParam)	;

	return 0;
}

LRESULT CEmoticonRichEditCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{	
	SetHan(TRUE);
	
	TRACE("CEmoticonRichEditCtrl::OnSetFocus\n");

	CRichEditCtrl::OnSetFocus(this)	;
	return 0;
}

LRESULT CEmoticonRichEditCtrl::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
	TRACE("IDMS: CEmoticonRichEditCtrl::OnKillFocus\n");
	
	CRichEditCtrl::OnKillFocus(this) ;
	return 0;
}

void CEmoticonRichEditCtrl::OnChange() 
{
	if (m_bInForcedChange)
		return;

	CHARRANGE crCurSel; 
	GetSel(crCurSel);
	
	if (m_changeType == ctMove && crCurSel.cpMin == crCurSel.cpMax) {
		// cut was canceled, so this is paste operation
		m_changeType = ctPaste;
	}

	switch(m_changeType) {
	case ctReplSel:// old=(x,y) -> cur=(x+len,x+len)
	case ctPaste:  // old=(x,y) -> cur=(x+len,x+len)
		FormatTextLines(m_crOldSel.cpMin, crCurSel.cpMax);
		break;
	case ctDelete: // old=(x,y) -> cur=(x,x)
	case ctBack:   // old=(x,y) -> cur=(x,x), newline del => old=(x,x+1) -> cur=(x-1,x-1)
	case ctCut:    // old=(x,y) -> cur=(x,x)
		FormatTextLines(crCurSel.cpMin, crCurSel.cpMax);
		break;
	case ctUndo:   // old=(?,?) -> cur=(x,y)
		FormatTextLines(crCurSel.cpMin, crCurSel.cpMax);
		break;
	case ctMove:   // old=(x,x+len) -> cur=(y-len,y) | cur=(y,y+len)
		FormatTextLines(crCurSel.cpMin, crCurSel.cpMax);
		if (crCurSel.cpMin > m_crOldSel.cpMin) // move after
			FormatTextLines(m_crOldSel.cpMin, m_crOldSel.cpMin);
		else // move before
			FormatTextLines(m_crOldSel.cpMax, m_crOldSel.cpMax);
		break;
	//case ctPaste:  //kojh
	//	FormatAll();
	default:
		FormatAll();
		break;
	}

	//undo action does not call OnProtected, so make it default
	m_changeType = ctUndo;	
}

void CEmoticonRichEditCtrl::OnProtected(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENPROTECTED* pEP = (ENPROTECTED*)pNMHDR;

	// determine type of change will occur

	switch (pEP->msg) {
	case WM_KEYDOWN:
		switch (pEP->wParam) {
		case VK_DELETE:
			m_changeType = ctDelete;
			break;
		case VK_BACK:
			m_changeType = ctBack;
			break;
		default:
			m_changeType = ctUnknown;
			break;
		}
		break;
	case EM_REPLACESEL:
	case WM_CHAR:
		m_changeType = ctReplSel;		
		break;
	case WM_PASTE:
		m_changeType = (m_changeType == ctCut)?ctMove:ctPaste;
		break;
	case WM_CUT:
		m_changeType = ctCut;
		break;
	default:
		m_changeType = ctUnknown;
		break;
	};

	if (pEP->msg != EM_SETCHARFORMAT && m_changeType != ctMove)
		m_crOldSel = pEP->chrg;

	*pResult = FALSE;
}

void CEmoticonRichEditCtrl::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	SELCHANGE* pSC = (SELCHANGE*)pNMHDR;

	*pResult = 0;
}



/////////

HBITMAP CEmoticonRichEditCtrl::GetImage(CImageList& list, int num)
{
	CBitmap dist; 
	CClientDC dc(NULL);

	IMAGEINFO ii;
	list.GetImageInfo(num, &ii);

	int nWidth = ii.rcImage.right - ii.rcImage.left;
	int nHeight = ii.rcImage.bottom - ii.rcImage.top;

	dist.CreateCompatibleBitmap(&dc, nWidth, nHeight);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap* pOldBitmap = memDC.SelectObject(&dist);
	
	memDC.FillSolidRect(0, 0, nWidth, nHeight, SetBackgroundColor(TRUE, 0));
	list.Draw(&memDC, num, CPoint(0, 0), ILD_NORMAL);

	memDC.SelectObject(pOldBitmap);

	return (HBITMAP)dist.Detach(); 
}

