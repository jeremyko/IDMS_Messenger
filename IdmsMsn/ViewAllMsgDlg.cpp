// ViewAllMsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\\common\\AllCommonDefines.h"
#include "IdmsMsn.h"
#include "ViewAllMsgDlg.h"
#include "MsgBoxThread.h"
#include "ChatWnd.h"
#include "ChatSession.h"

extern "C"
{
    #include <direct.h>
}
#include "INIManager.h"
#include "SP_Utility.h"
#include "ChatSession.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CViewAllMsgDlg dialog
CViewAllMsgDlg::~CViewAllMsgDlg()
{
	
}


CViewAllMsgDlg::CViewAllMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewAllMsgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewAllMsgDlg)	
	//}}AFX_DATA_INIT
}

CViewAllMsgDlg::CViewAllMsgDlg( CBuddyInfo * pCompanion)
{
	//m_pCompanion = pCompanion;
}


void CViewAllMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewAllMsgDlg)
	DDX_Control(pDX, IDC_REDIT_VIEW_ALLMSG, m_RichEditViewAllMsg);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewAllMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CViewAllMsgDlg)
	ON_BN_CLICKED(IDC_BTN_VIEW_ALLMSG_OK, OnBtnViewAllmsgOk)	
	ON_WM_SIZE()
	ON_COMMAND(IDC_ALLMSG_CLOSE, OnAllmsgClose)
	ON_COMMAND(IDC_ALLMSG_FILESAVE, OnAllmsgFilesave)
	ON_COMMAND(IDC_ALLMSG_HIST, OnAllmsgHist)
	ON_WM_CLOSE()	
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ALLMSG,   OnAllMsg)	
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewAllMsgDlg message handlers

void CViewAllMsgDlg::OnBtnViewAllmsgOk() 
{
	// TODO: Add your control notification handler code here
	//DeleteObject(m_Font);
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_VIEWALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);

	//CDialog::OnOK();
}

LRESULT CViewAllMsgDlg::OnAllMsg(WPARAM wParam, LPARAM lParam)
{
	S_ALLMSG_CLIENT* pAllMsg = (S_ALLMSG_CLIENT*)(lParam);
	
	CTime iDT =  CTime::GetCurrentTime();
	m_StrRcvDate = iDT.Format("%Y%m%d/%H%M%S");	
	m_StrSender = pAllMsg->strNameFrom ;
	m_strSenderID = pAllMsg->strIDFrom ;
	m_strRcvDateDetail = iDT.Format("%Y년 %m월 %d일/%H시 %M분 %S초");	
	
	CString strMsg;
	int nStart = m_RichEditViewAllMsg.GetWindowTextLength();
	m_RichEditViewAllMsg.SetSel(nStart, nStart);	
		
	CHARFORMAT cf;
	cf.cbSize       = sizeof(CHARFORMAT);
	cf.dwMask       = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
	cf.dwEffects    &=(unsigned long) ~( CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);	
	cf.crTextColor  = CChatSession::Instance().m_ColorBuddy;
	m_RichEditViewAllMsg.SetSelectionCharFormat(cf);	
		
	m_RichEditViewAllMsg.ReplaceSel("\n");
	strMsg.Format("▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\r\n★ 받은날자: %s\r\n★ 보낸사람: %s\r\n\r\n%s\r\n\r\n", m_strRcvDateDetail, m_StrSender, pAllMsg->strMsg);
	m_RichEditViewAllMsg.ReplaceSel(strMsg);


	int lenAft = m_RichEditViewAllMsg.GetWindowTextLength();
	m_RichEditViewAllMsg.SetSel(nStart, lenAft);		
	
	m_RichEditViewAllMsg.GetSelectionCharFormat(cf);
	cf.crTextColor = CChatSession::Instance().m_ColorBuddy;
	cf.dwMask = CFM_COLOR | CFM_FACE;	
	
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	strcpy(cf.szFaceName, lf.lfFaceName);
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_RichEditViewAllMsg.SetSelectionCharFormat(cf);      
	
	lenAft = m_RichEditViewAllMsg.GetWindowTextLength();
	m_RichEditViewAllMsg.SetSel(lenAft, lenAft);		
	m_RichEditViewAllMsg.HideSelection(TRUE, TRUE);

	m_RichEditViewAllMsg.SendMessage(EM_SCROLLCARET, 0);	

	/*
	// ListCtrl 에 표시	
	CString strItem;		
	LVITEM lvi;		
	lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
	int tempCnt = m_CListCtrlViewAllMsg.GetItemCount();
	lvi.iItem = tempCnt;		
	//받은 날자	
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(cstr);
	lvi.iImage = -1;
	m_CListCtrlViewAllMsg.InsertItem(&lvi);	
	m_CListCtrlViewAllMsg.SetItem(&lvi);		
	
	//보낸 사람
	lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(pAllMsg->strNameFrom);		
	lvi.iImage = -1;
	m_CListCtrlViewAllMsg.InsertItem(&lvi);
	m_CListCtrlViewAllMsg.SetItem(&lvi);		
	
	//내용 
	// TODO 20060419 .. 내용을 저장할 구조가 있어야한다..!!!
	lvi.iSubItem = 2;
	CString strimsi = pAllMsg->strMsg.Mid(0, 100);
	strimsi.Replace("\r\n", " ");	
	//lvi.pszText = (LPTSTR)(LPCTSTR)(pAllMsg->strMsg);		
	lvi.pszText = (LPTSTR)(LPCTSTR)(strimsi);		
	lvi.iImage = -1;
	m_CListCtrlViewAllMsg.InsertItem(&lvi);
	m_CListCtrlViewAllMsg.SetItem(&lvi);		

	m_CListCtrlViewAllMsg.SetHotItem(tempCnt);
	m_CListCtrlViewAllMsg.SetSelectionMark(tempCnt);
		
	*/

	UpdateData(FALSE);

	// 파일에 저장	
	CString strTmp;	
	//CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
	CString strtmSimple = iDT.Format("%Y%m%d");	

	/////////////////////////////////////////////////////////
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
		
	char szDirTmp[512];		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	

	if(strUseLogFolder == "Y")
	{
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strcpy( szDirTmp, (LPCSTR)strLogFolderPath);
		
		if(szDirTmp[ strlen(szDirTmp) - 1 ] == '\\')
		{
			szDirTmp[ strlen(szDirTmp) - 1 ] = 0x00 ;
		}
				
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}
	else
	{		
		char szDirTmp[512];
		char szDir[1024];
				
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
		memset(szDir, 0x00, sizeof(szDir));	
						
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
		
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}	
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	
	CString strFileNameAll;	
	strFileNameAll.Format("%s\\공지사항%s.txt", strFileName, strtmSimple);
	writeLogFile(strFileNameAll, "▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\r\n");
	writeLogFile(strFileNameAll, "★ 받은날자: %s\r\n", m_StrRcvDate);			//받은 날자
	writeLogFile(strFileNameAll, "★ 보낸사람: %s\r\n", m_StrSender);	//보낸 사람
	writeLogFile(strFileNameAll, "\r\n");		//공지메시지
	writeLogFile(strFileNameAll, "   %s\r\n", pAllMsg->strMsg);		//공지메시지
	writeLogFile(strFileNameAll, "\r\n\r\n");			//공지메시지
	

	FlashWindow(TRUE);

	/*
	FLASHWINFO fwi;
	fwi.cbSize  = sizeof(FLASHWINFO);
	fwi.hwnd =  this->GetSafeHwnd();
	//fwi.dwFlags = FLASHW_ALL ;
	fwi.dwFlags = FLASHW_TIMERNOFG | FLASHW_TRAY ;
	fwi.uCount  = 3; 
	fwi.dwTimeout = 0; 
	FlashWindowEx(&fwi);		
	*/

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


BOOL CViewAllMsgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	/* 보류..
	m_CListCtrlViewAllMsg.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES);		
	//m_CListCtrlViewAllMsg.SetExtendedStyle(m_CListCtrlViewAllMsg.GetExtendedStyle() | LVS_EX_GRIDLINES ); //| LVS_EX_CHECKBOXES
	
	m_CListCtrlViewAllMsg.InsertColumn(0,"받은 날자");
	m_CListCtrlViewAllMsg.InsertColumn(1,"보낸 사람");	
	m_CListCtrlViewAllMsg.InsertColumn(2,"메시지 ");	
	m_CListCtrlViewAllMsg.SetColumnWidth(0, 120);
	m_CListCtrlViewAllMsg.SetColumnWidth(1, 120);
	m_CListCtrlViewAllMsg.SetColumnWidth(2, 400);


	// TODO: Add extra initialization here
	// 파일에서 공지사항이력을 읽어온다.
	BOOL bRslt = LoadAllMsgHistData();
	*/
	
	m_RichEditViewAllMsg.SetBackgroundColor( FALSE, CChatSession::Instance().m_ColorBG );

	SetResize();	

	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	
	m_RichEditViewAllMsg.Initialize();
	m_RichEditViewAllMsg.setUrlDetection(TRUE);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CViewAllMsgDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);	
		
	m_cControlPos.AddControl(IDC_REDIT_VIEW_ALLMSG	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );	
	m_cControlPos.AddControl(IDC_BTN_VIEW_ALLMSG_OK	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );		
	
}

void CViewAllMsgDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
}

void CViewAllMsgDlg::OnAllmsgClose() 
{
	// TODO: Add your command handler code here
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_VIEWALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);

	//CDialog::OnOK();
}

void CViewAllMsgDlg::OnAllmsgFilesave() 
{
	// TODO: Add your command handler code here
	UpdateData(TRUE);

	CString strPath, strContents;

	m_RichEditViewAllMsg.GetWindowText(strContents);	
	
	if(strContents.IsEmpty())
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"저장할 내용이 없습니다.");
		
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
}

void CViewAllMsgDlg::FileDialogOpen(CString &result)
{
	char szFilter[] = "txt Files (*.txt)|*.txt; *.txt|All Files (*.*)|*.*||";
	CFileDialog tmpFileDlg(FALSE  ,"txt",NULL,NULL,szFilter,NULL);
	int rtn = tmpFileDlg.DoModal();
	if(IDCANCEL == rtn)
		return;	
	result = tmpFileDlg.GetPathName();
}


BOOL CViewAllMsgDlg::LoadAllMsgHistData()
{
	/*
	.20060419/163700
	.Insterer
	.3333333333333333333ㅇㅎㄹㅇㅎ

	// 공지사항 폴더에 있는 파일목록을 모두 얻음
	BOOL bWorking = FALSE;
	CFileFind finder;
	

	bWorking = finder.FindFile(pstr);		
 */

	return TRUE;
}

void CViewAllMsgDlg::OnAllmsgHist() 
{
	// TODO: Add your command handler code here	
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
	
	char szWinDir[512];		
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));

	char szDirTmp[512];	
	char szDir[1024];
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	if(strUseLogFolder == "Y")
	{
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strcpy( szDirTmp, (LPCSTR)strLogFolderPath);
		
		if(szDirTmp[ strlen(szDirTmp) - 1 ] == '\\')
		{
			szDirTmp[ strlen(szDirTmp) - 1 ] = 0x00 ;
		}
				
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}
	else
	{
		char szDirTmp[512];				
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
						
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
		
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}
	

	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}

	sprintf(szDir, "%s\\explorer.exe %s",szWinDir, (LPCSTR)strFileName);		
	WinExec(szDir, SW_SHOWDEFAULT);
	
	strFileName.Empty();	
}

BOOL CViewAllMsgDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CViewAllMsgDlg message handlers

void CViewAllMsgDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_VIEWALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);
	

	CDialog::PostNcDestroy();
}

void CViewAllMsgDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_VIEWALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);

	CDialog::OnClose();
}

