 // IdmsMsnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogInDlg.h"
#include "OptionDlg.h"
#include "IdmsMsn.h"
#include "IdmsMsnDlg.h"
//#include "ChatMSgDlg.h"
#include "ChatWnd.h"
#include "NewCompanionDlg.h"
#include "INIManager.h"
#include "MsgBoxThread.h"
#include "..\\common\\AllCommonDefines.h"
#include "FileTransSock.h"
#include "GroupMngDlg.h"
#include "utility.h"
extern "C"
{
    #include <direct.h>
}
#include "FileSendDlg.h"
#include "MultiChatWnd.h"
#include "BuddySlipMsg.h"
#include "ViewBuddySlipMsg.h"
#include <MSWSOCK.H>		// MFC socket extensions
#include "ITCompDlgMng.h"
#include<CDERR.H>

#include "ChatWnd.h"
#include "FileSendDlg.h"
#include "FileRecvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

namespace Dormant
{
	extern "C" __declspec(dllimport) BOOL Initiate();
	extern "C" __declspec(dllimport) time_t GetDormantTime();
	extern "C" __declspec(dllimport) BOOL EndUp();
}

const int HEARTBEAT_MILSEC = 30000 ;
const int MAX_ALIVE_FAIL = 6; // 20 * 6 = 120 sec 

 


// DLL �ε��� ȣ��Ǵ� �Լ�����
/*
extern "C"
{
	EXPORT void __callback_print(char * lpszMessage) 
	{
		MessageBox(NULL,lpszMessage,"From Exe",MB_OK);
	}
}
*/

// File ���� ���� 
DWORD WINAPI SendFileThread(LPVOID lParam);

struct st_ThreadInfo
{
	char file_name[MAX_PATH_LEN];
	char szToIp [15+1];	
	SOCKET m_Sock;
	
};

// File ���� ���� 
FileTransInfo ServerInfoForPrg;

//ULONG_PTR gpToken;  //GDI+

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT WM_TRAY = ::RegisterWindowMessage("CreateTrayIcon");
UINT g_uShellRestart = ::RegisterWindowMessage("TaskbarCreated"); 

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnDlg dialog

CIdmsMsnDlg::CIdmsMsnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIdmsMsnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIdmsMsnDlg)
	m_strMyNick = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		
	m_pAllMSgViewDlg = NULL; 
	m_pAllMSgSendDlg = NULL; 
	m_MultiChatDlgCnt = 0;	
}

void CIdmsMsnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIdmsMsnDlg)
	DDX_Control(pDX, IDC_BTN_SEC3, m_btnSec3);
	DDX_Control(pDX, IDC_BTN_SEC2, m_btnSec2);
	DDX_Control(pDX, IDC_BTN_SEC1, m_btnSec1);
	DDX_Control(pDX, IDC_STATIC_MY_NICK, m_staNick);
	DDX_Control(pDX, IDC_BTN_MY_STATUS, m_btnMyStatus);
	DDX_Control(pDX, IDC_TREECTRL, m_TreeCtrl);
	DDX_Text(pDX, IDC_STATIC_MY_NICK, m_strMyNick);
	DDV_MaxChars(pDX, m_strMyNick, 130);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIdmsMsnDlg, CDialog)
	//{{AFX_MSG_MAP(CIdmsMsnDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREECTRL, OnItemexpandedTreectrl)
	ON_NOTIFY(NM_DBLCLK, IDC_TREECTRL, OnDblclkTreectrl)
	ON_COMMAND(ID_CLOSE_MSGER, OnCloseMsger)
	ON_COMMAND(IDC_ADDPARTNER, OnAddpartner)
	ON_NOTIFY(NM_CLICK, IDC_TREECTRL, OnClickTreectrl)
	ON_COMMAND(ID_GRP_MNG_ADDCOMP, OnGrpMngAddcomp)
	ON_COMMAND(ID_TOOLS_ABOUT, OnToolsAbout)
	ON_COMMAND(ID_GRP_MNG_ADD, OnGrpMngAdd)
	ON_COMMAND(ID_GRP_MNG_DEL, OnGrpMngDel)
	ON_COMMAND(IDRCLICK_DELETE, OnDelete)
	ON_COMMAND(IDRCLICK_GRP_CHG, OnGrpChg)
	ON_COMMAND(ID_GRP_MNG_MODIFY, OnGrpMngModify)
	ON_COMMAND(ID_GRP_MNG_SENDFILE, OnGrpMngSendfile)
	ON_COMMAND(IDC_ALWAYS_ONTOP, OnAlwaysOntop)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOption)
	ON_COMMAND(ID_GRP_MNG_ALLMSG, OnGrpMngAllmsg)
	ON_COMMAND(ID_TRAYMENU_RESTORE, OnTraymenuRestore)
	ON_COMMAND(IDRTRAYMENU_QUIT, OnQuit)
	ON_COMMAND(ID_VIEW_CHAT_HIST, OnViewChatHist)
	ON_COMMAND(ID_VIEW_ALLMSG_HIST, OnViewAllmsgHist)
	ON_COMMAND(IDRCLICK_SENDFILE, OnSendfile)
	ON_COMMAND(IDRCLICK_ALLMSG, OnAllmsg)
	ON_COMMAND(ID_BROADCAST_MSG, OnBroadcastMsg)
	ON_WM_TIMER()
	ON_COMMAND(IDRCLICK_MULTICHAT, OnMultichat)	
	ON_COMMAND(IDRCLICK_WRITE_MSG, OnWriteMsg)
	ON_COMMAND(ID_VIEW_SLIPMSG_LOG, OnViewSlipmsgLog)
	ON_COMMAND(ID_GRP_MNG_ADD_ITCOMP, OnGrpMngAddItcomp)
	ON_COMMAND(IDRCLICK_ITCOMP_ADD, OnItcompAdd)
	ON_COMMAND(IDRCLICK_ITCOMP_DEL, OnItcompDel)
	ON_COMMAND(IDRCLICK_ITCOMP_MODIFY, OnItcompModify)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_SEC1, OnBtnSec1)
	ON_BN_CLICKED(IDC_BTN_SEC2, OnBtnSec2)
	ON_BN_CLICKED(IDC_BTN_SEC3, OnBtnSec3)
	ON_COMMAND(ID_RUN_IDMS, OnRunIdms)
	ON_COMMAND(ID_TRAYMENU_IDMS, OnTraymenuIdms)
	ON_COMMAND(ID_RUN_MDB, OnRunMdb)
	ON_COMMAND(ID_RUN_SOCK_TEST, OnRunSockTest)
	ON_COMMAND(ID_RUN_TUX_TEST, OnRunTuxTest)
	ON_COMMAND(ID_TRAYMENU_MDB, OnTraymenuMdb)
	ON_COMMAND(ID_TRAYMENU_SOCKTEST, OnTraymenuSocktest)
	ON_COMMAND(ID_TRAYMENU_TUXTEST, OnTraymenuTuxtest)
	ON_WM_COPYDATA()
	ON_COMMAND(ID_TOOLS_OPTION, OnToolsOption)
	ON_WM_GETMINMAXINFO()	
	ON_COMMAND(ID_RUN_RS_BOARD, OnRunRsBoard)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(WM_TRAY, OnTrayMessage)	
	ON_REGISTERED_MESSAGE(g_uShellRestart, OnTrayShow) 

	
	ON_MESSAGE(WM_SERVER_CLOSED	,	OnServerClosed		)
	ON_MESSAGE(WM_INIT_WORK		,	OnInitWork			)
	ON_MESSAGE(WM_BUDDY_LIST	,	OnBuddyListRslt		)
	ON_MESSAGE(WM_SQL_ERR		,	OnSQLRsltErr		)	
	ON_MESSAGE(WM_ADD_BUDDY		,	OnAddBuddyRslt		)	
	ON_MESSAGE(WM_SOMEONE_ADD_ME,	OnSomeOneAddMe		)				
	ON_MESSAGE(WM_BUDDY_GRP		,	OnBuddyGrpRslt		)
	ON_MESSAGE(WM_ADD_BUDDY_GRP	,	OnAddGrpRslt		)
	ON_MESSAGE(WM_DEL_BUDDY_GRP	,	OnDelGrpRslt		)
	ON_MESSAGE(WM_DEL_BUDDY_ID	,	OnDelBuddyRslt		)
	ON_MESSAGE(WM_DEL_IT_COMP	,	OnDelITBuddyRslt	)
	ON_MESSAGE(WM_SOMEONE_DEL_ME,	OnSomeOneDelMe		)
	ON_MESSAGE(WM_CHG_BUDDY_GRP	,	OnChgBuddyGrpRslt	)
	ON_MESSAGE(WM_CHG_GRP_NAME	,	OnChgGrpNameRslt	)	
	ON_MESSAGE(WM_CHATMSG		,   OnChatMsg			)		
	ON_MESSAGE(WM_BUDDY_ONLINE	,   OnBuddyOnline		)
	ON_MESSAGE(WM_BUDDY_OFFLINE	,   OnBuddyOffline		)	
	ON_MESSAGE(WM_CHATDLG_CLOSE	,   OnChatDlgClose		)
	ON_MESSAGE(WM_SLIPMSG_DLG_CLOSE	,   OnSlipMsgDlgClose)
	ON_MESSAGE(WM_ALLMSG_CLOSE	,   OnAllMsgDlgClose	)	
	ON_MESSAGE(WM_ALLOWED_COMP	,   OnAllowedComp		)	
	ON_MESSAGE(WM_YOU_ALLOWED_BY,   OnYouAllowedBy		)	
	ON_MESSAGE(WM_ALLMSG		,   OnAllMsg			)	
	ON_MESSAGE(WM_VIEWALLMSG_CLOSE, OnViewAllMsgDlgClose)
	ON_MESSAGE(WM_ALLMSG_ACK	,   OnAllMsgAck			)
	ON_MESSAGE(WM_CHNG_NICK  	,   OnChngNick			)
	ON_MESSAGE(WM_AWAY_TRUE  	,   OnAwayTrue			)
	ON_MESSAGE(WM_AWAY_FALSE  	,   OnAwayFalse  		)
	ON_MESSAGE(WM_STATUS_BUSY  	,   OnStatusBusy  		)	
	ON_MESSAGE(WM_SEARCH_ID  	,   OnSearchID   		)
	ON_MESSAGE(WM_MULTI_CHATDLG_CLOSE,   OnMultiChatDlgClose)		
	ON_MESSAGE(WM_MCHAT_SERVER_ROOMNO,OnMChatServerRoomNo)	
	ON_MESSAGE(WM_MULTI_CHATMSG,OnMChatMsg)	
	ON_MESSAGE(WM_EXIT_MULTI_CHAT,OnExitMChatMsg)	
	ON_MESSAGE(WM_SLIP_MSG_SUCCESS,OnSlipMsgSaveSuccess)	
	ON_MESSAGE(WM_SLIP_MSG,OnSlipMsg)	
	ON_MESSAGE(WM_ADD_IT_COMP,OnAddItComp)	
	ON_MESSAGE(WM_CHG_IT_COMP,OnChgItComp)	
	ON_MESSAGE(WM_FOOD_MENU,OnFoodMenu)	
	ON_MESSAGE(WM_ALL_USER_ID_TEAM,OnAllUserIdMenu)	
	ON_MESSAGE(WM_ALL_USER_ID_TEAM_FORADD,OnAllUserIdForAdd)	
	ON_MESSAGE(WM_SENDALLMSGACKED,OnAllMsgAcked)	
	ON_MESSAGE(WM_ALIVE,OnALive)	
	ON_MESSAGE(WM_CHG_CONFIG,OnConfigChg)	
	
	
	
	
	//���� ���۰���
	ON_MESSAGE(WM_SOMEONE_WANTS_SEND_FILES, OnSomeOneWantsSendFile		) //	������ ������ �����Ϸ��� ���	
	ON_MESSAGE(WM_FOLDER_SELECTING,			OnFolderSelecting	)	
	ON_MESSAGE(WM_SEND_THIS_INFORMED_FILE,	OnSendThisInformedFile	)
	ON_MESSAGE(WM_DISCONNECTED,				OnDisconnected	)	
	ON_MESSAGE(WM_FILE_SENDDLG_CLOSE,		OnFileSendDlgClosed	)	
	ON_MESSAGE(WM_FILE_RECVDLG_CLOSE,		OnFileRecvDlgClosed	)	
			

	//Section ���� ���� �޽���
	ON_MESSAGE(WM_SECTION_REQUEST,   OnSectionReq)

	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast) 
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnDlg message handlers

char* strdel(char* s, const char* delstr) //s���� delstr�� ����� �����Ѵ�.
{
	char* start,*end;
	
	if((start=strstr(s,delstr))==NULL) return NULL;  //��ã���� NULL����
	end=start+strlen(delstr);
	memcpy(start,end,strlen(s)-(end-s)+1); 
	return start;
}

	// ������ ������ ������ �������� �Ѵ�. => ���� �޴� �۾� ó��
LRESULT CIdmsMsnDlg::OnSomeOneWantsSendFile (WPARAM wParam, LPARAM lParam)
{
	int nPos = 0;
	
	ST_RCV_SOMEONE_WANT_SEND_FILE_S* pFile = (ST_RCV_SOMEONE_WANT_SEND_FILE_S*) wParam ;	
	SOCKET_OBJ_FILE* pSockObj = (SOCKET_OBJ_FILE*) lParam ;
					
	CFileRecvDlg* pDlg = new CFileRecvDlg( pFile, pSockObj );
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();				
	pDlg->Create(MAKEINTRESOURCE(IDD_FILERECV_DLG) , GetDesktopWindow() /*this*/);
	pDlg->ShowWindow(SW_SHOW);
	pDlg->FlashWindow(TRUE);
	
	pSockObj->m_HwndFileRecv = pDlg->GetSafeHwnd();

	CFileTransSock::Instance().mStrFileRecvDlgMap.SetAt( pSockObj->m_szSockObjID, pDlg);	
	
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnFolderSelecting (WPARAM wParam, LPARAM lParam)
{
	FILE_HEADER *psHeader = (FILE_HEADER *) wParam ;
	SOCKET_OBJ_FILE* pSockObj = (SOCKET_OBJ_FILE*) lParam ;
	
	//�̹� �����ִ� FileSendDlg �� �޽��� ������ szFromIP
	CWnd* pWnd =  CWnd::FromHandle(pSockObj->m_HwndFileSend);
	if(pWnd != NULL)
	{			
		::SendMessage(pWnd->m_hWnd, WM_FOLDER_SELECTING, NULL, NULL);
	}
		
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnSendThisInformedFile (WPARAM wParam, LPARAM lParam)
{
	char* szFile = (char *) wParam ;
	SOCKET_OBJ_FILE* pSockObj = (SOCKET_OBJ_FILE*) lParam ;
	
	//�̹� �����ִ� FileSendDlg �� �޽��� ������	
	
	if(pSockObj->m_HwndFileSend)
	{			
		::SendMessage(pSockObj->m_HwndFileSend, WM_SEND_THIS_INFORMED_FILE, wParam, lParam);
	}
			
	return TRUE;
}



LRESULT CIdmsMsnDlg::OnFileSendDlgClosed (WPARAM wParam, LPARAM lParam)
{
	CString strID = (char*)wParam;
	TRACE("OnFileSendDlgClosed [%s]", strID);
	
	CFileSendDlg * pFileSendDlg = NULL;
	
	if( CFileTransSock::Instance().mStrFileSendDlgMap.Lookup(strID, ( CObject*& ) pFileSendDlg ))
	{
		CFileSendDlg * pDlg = static_cast<CFileSendDlg*>(pFileSendDlg);
		if(pDlg)
		{
			delete pDlg;
			CFileTransSock::Instance().mStrFileSendDlgMap.RemoveKey(strID);
			pDlg = NULL;
		}
	}
	
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnFileRecvDlgClosed (WPARAM wParam, LPARAM lParam)
{
	CString strID = (char*)wParam;
	TRACE("OnFileRecvDlgClosed [%s]", strID);
	
	CFileRecvDlg * pFileRecvDlg = NULL;
	
	if( CFileTransSock::Instance().mStrFileRecvDlgMap.Lookup(strID, ( CObject*& ) pFileRecvDlg ))
	{
		CFileRecvDlg * pDlg = static_cast<CFileRecvDlg*>(pFileRecvDlg);
		if(pDlg)
		{
			delete pDlg;
			CFileTransSock::Instance().mStrFileRecvDlgMap.RemoveKey(strID);
			pDlg = NULL;
		}
	}
	
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnDisconnected (WPARAM wParam, LPARAM lParam)
{		
	SOCKET_OBJ_FILE* pSock = (SOCKET_OBJ_FILE*) lParam ;
	
	//WSACleanup();			

	CFileRecvDlg * pFileRecvDlg = NULL;
	if( CFileTransSock::Instance().mStrFileRecvDlgMap.Lookup(pSock->m_szSockObjID , (CObject*&)pFileRecvDlg) )
	{
		if(pFileRecvDlg)
		{				
			::PostMessage( pFileRecvDlg->GetSafeHwnd() , WM_DISCONNECTED, (WPARAM)NULL, (LPARAM) NULL );  
		}
	}		

	CFileSendDlg * pFileSendDlg = NULL;
	if( CFileTransSock::Instance().mStrFileSendDlgMap.Lookup(pSock->m_szSockObjID , (CObject*&)pFileSendDlg) )
	{
		if(pFileSendDlg)
		{				
			::PostMessage( pFileSendDlg->GetSafeHwnd() , WM_DISCONNECTED, (WPARAM)NULL, (LPARAM) NULL );  
		}
	}
			
	return TRUE;
}

void CIdmsMsnDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	m_cControlPos.AddControl(&m_staNick	, CP_RESIZE_HORIZONTAL );		
	//m_cControlPos.AddControl(&m_btnMyStatus	, CP_MOVE_HORIZONTAL );		
	m_cControlPos.AddControl(&m_TreeCtrl	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );		
	m_cControlPos.AddControl(&m_statusBar	, CP_RESIZE_HORIZONTAL	| CP_MOVE_VERTICAL	);	
	
}


/*
LRESULT CIdmsMsnDlg::OnInformFolderSelIndirect_NO(WPARAM wParam, LPARAM lParam)
{	
	S_SOMEONE_WANT_SEND_FILES*  pSendFileInfo = (S_SOMEONE_WANT_SEND_FILES*) wParam ;
	TRACEX("szUserID [%s] " , pSendFileInfo->szFromID);

	CChatSession::Instance().DenyTransferFile( pSendFileInfo->szFromID ) ; 
		
	delete pSendFileInfo;
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnInformFolderSelIndirect(WPARAM wParam, LPARAM lParam)
{	
	S_SOMEONE_WANT_SEND_FILES*  pSendFileInfo = (S_SOMEONE_WANT_SEND_FILES*) wParam ;
	
	TRACEX("szUserID [%s] " , pSendFileInfo->szFromID);

	// ���������� �˸�		
	CChatSession::Instance().InformFolderSelect( pSendFileInfo->szFromID ) ;  

	char szSavePath[1024];
	memset(szSavePath,0x00,sizeof(szSavePath));		
	
	FolderDialogOpen(szSavePath); 	
	
	if( strlen(szSavePath) == 0)
	{
		// ���� �ޱ� ����� �˸�
		
		CChatSession::Instance().DenyTransferFile( pSendFileInfo->szFromID ) ; 
		
		delete pSendFileInfo;
		return TRUE;
	}
	
	
	// ������ �޴µ��� �Ǵٸ� ���� ���۽� ���	
	CFileTransferDlg* pDlg = new CFileTransferDlg(  pSendFileInfo->szFromIP , pSendFileInfo->strRootPath, pSendFileInfo->strFilePath, szSavePath);
				
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();		
	
	//pDlg->Create(MAKEINTRESOURCE(IDD_FILERECV_DLG) , FromHandle(hDeskTop));
	pDlg->Create(MAKEINTRESOURCE(IDD_FILERECV_DLG) , this);
	pDlg->ShowWindow(SW_SHOW);
	
	mStrFileTransferDlgMap.SetAt( pSendFileInfo->szFromIP, pDlg);
	
	delete pSendFileInfo;

	return TRUE;
}
*/


static UINT auIDStatusBar[] = {
	ID_SEPARATOR
};



LRESULT CIdmsMsnDlg::OnMultiChatDlgClose(WPARAM wParam, LPARAM lParam)
{	
	CString strID = (char*)wParam; // IP Address

	TRACE("OnMultiChatDlgClose [%s]", strID);
	//CMultiChatMsgDlg* pa;	
	CMultiChatWnd* pa;	
	
	if(mStrMultiChatDlgMap.Lookup(strID, ( CObject*& ) pa ))
	{
		CMultiChatWnd * pDlg = static_cast<CMultiChatWnd*>(pa);
		if(pDlg)
		{
			TRACE("OnMultiChatDlgClose delete !\n");
			delete pDlg;
			mStrMultiChatDlgMap.RemoveKey(strID);
		}
	}

	return TRUE;
}

BOOL CIdmsMsnDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	WORD		wVersionRequested;
	WSADATA		wsaData;	

	wVersionRequested = MAKEWORD( 2, 0 );

	if ( WSAStartup( wVersionRequested, &wsaData ) != 0 )
	{
		int nRtn = WSAGetLastError ();	
		CString str;
		str.Format("WSAStartup Fail [%ld]", nRtn) ;
        AfxMessageBox(str);		
		CDialog::OnOK();
		return TRUE;
	}	

	m_nAliveAckFailed = 0;
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here	

	// GDI+ �ʱ�ȭ 
	/*
	GdiplusStartupInput gpsi;

    if (GdiplusStartup(&gpToken,&gpsi,NULL) != Ok) 
	{
		AfxMessageBox("GDI+ ���̺귯���� �ʱ�ȭ�� �� �����ϴ�.");
        CINIManager::Instance().GetValue(INI_PASSWD, CChatSession::Instance().m_decrpPassWd) ;
		CINIManager::Instance().SaveINIFile();
		CDialog::OnOK();
		return TRUE;
    }
	*/
	
	char		name[255];	
	PHOSTENT	hostinfo;
	memset(m_szMyIPAddr, 0x00,sizeof(m_szMyIPAddr));
    if( gethostname ( name, sizeof(name)) == 0)
	{
		if((hostinfo = gethostbyname(name)) != NULL)
		{
			IN_ADDR in;
			memcpy( &in, hostinfo->h_addr_list[0], 4 );            
			strcpy(m_szMyIPAddr , inet_ntoa(in) );

			//strcpy(m_szMyIPAddr , inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list[0] ));

			
			for(int i=0; hostinfo->h_addr_list[i] != NULL; i++)
			{
				//char* ip = inet_ntoa (*(struct in_addr *)hostinfo->h_addr_list[i]);
				//AfxMessageBox(inet_ntoa (*(struct in_addr *)hostinfo->h_addr_list[i])); 

				TRACE( "\n"); 
				TRACE( inet_ntoa (*(struct in_addr *)hostinfo->h_addr_list[i]) ); 
			}	
			
		}
	}

	//writeLogFile("c:\\ClientLog.log", "OnInitDialog m_szMyIPAddr[%s]\n" , m_szMyIPAddr);	

	CChatSession::Instance().setMyIpAddr(m_szMyIPAddr);

	CINIManager::Instance().LoadINIFile();
	
	// status	
	if (m_statusBar.Create(this))
	{
		m_statusBar.SetIndicators(auIDStatusBar, sizeof(auIDStatusBar) / sizeof(unsigned int));

		m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0),
			SBPS_STRETCH, NULL );
	}
	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
				   0, reposQuery, rcClientNow);
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
					rcClientNow.top - rcClientStart.top);

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
		
	m_staNick.SetColors(RGB(128,255,128), RGB(0,0,0));
	//m_staNick.SetTextStyle(CMyStatic::ST_MULTILINE);
	
	m_nCntReConnect= 0;
	m_bConnectTrying = FALSE;
	m_bReConnectFlag = FALSE;
	m_pSec2Dlg = NULL;
	m_pSec3Dlg = NULL;
	
	CChatSession::Instance().SetMainDlg(this);	
	
	BOOL bRtn = CChatSession::Instance().SocketInit();
	
	if(bRtn == FALSE)
	{
		AfxMessageBox("������ �����Ҽ� �����ϴ�. �����ڿ��� �����ϼ���.");
		CINIManager::Instance().GetValue(INI_PASSWD, CChatSession::Instance().m_decrpPassWd) ;
		CINIManager::Instance().SaveINIFile();
		CDialog::OnOK();
		return TRUE;
	}

	SetResize() ;	

	// �������ۿ� ���� ������ �⵿		
	CFileTransSock::Instance().Init ( this->GetSafeHwnd() );	
	CFileTransSock::Instance().Start();

	// LogIn Dlg
	TRACE("Log Dlg ���\n");
	CLogInDlg dlg;	
	dlg.DoModal();	
	if(dlg.m_Rslt == FALSE)
	{
		TRACE("Log In ����\n");

		CDialog::OnOK();
		return FALSE;
	}
	
	m_strLoginID = dlg.m_strID;	

	strncpy(CChatSession::Instance().mMyInfo.m_szBuddyID, (LPCSTR)dlg.m_strID, sizeof(CChatSession::Instance().mMyInfo.m_szBuddyID));
	strncpy(CChatSession::Instance().mMyInfo.m_szUserName, (LPCSTR)dlg.m_LoginName, sizeof(CChatSession::Instance().mMyInfo.m_szUserName));
	strncpy(CChatSession::Instance().mMyInfo.m_szNickName, CChatSession::Instance().m_Nick, sizeof(CChatSession::Instance().mMyInfo.m_szUserName));
	

	strcpy(CChatSession::Instance().m_szMyUserID , (LPSTR)(LPCSTR)m_strLoginID);


	// �θ� ������ ����
	TRACE("�θ� ������ ���� ���\n");
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());	
				
	m_hBusyBmp = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_STATUS_BUSY ) );
	m_hOnLineBmp = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) );
	m_hAwayBmp = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_AWAY ) );
	m_hOffLineBmp = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_OffLine ) );
	m_hGrpOpen = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_GRPOPEN ) );
	m_hGrpClose = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_GRPCLOSE ) );	
	m_hBlockBmp = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_BLOCK) );	
	m_hITComp = ::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ITCOMP) );	
	
	m_TreeCtrl.ModifyStyle( 0, TVS_TRACKGROUPSELECT | TVS_SINGLECLICKEXPAND );

	// Font ����
	CFont font;	
	font.CreatePointFont( 110, "����" );	

	m_TreeCtrl.SetFont( &font );
	m_TreeCtrl.SetFocus();
	
	m_ColorBG  = RGB( 255,255,255 );
	m_ColorGrp = RGB( 80, 130, 50 );
	m_ColorNode = RGB( 60,94,172);
	
	//m_ColorBG  = RGB( 57,66,143 );
	//m_ColorGrp = RGB( 0, 64, 64 );
	//m_ColorNode = RGB( 223,238,17);

	// ���� ���̾�α� ���� ����
	//m_TreeCtrl.SetBkColor( RGB( 255,255,255 ) );	
	m_TreeCtrl.SetBkColor( m_ColorBG );	
	
	m_MeStatusFlags = 0l;

	// Hooking ����
	m_bHook = Dormant::Initiate(); 
    if(!m_bHook)
        AfxMessageBox("��ŷ����." );

	CChatSession::Instance().m_AwaySec = 60 * 3 ; // Detault => 3 ��
	CChatSession::Instance().m_AwaySec = CINIManager::Instance().GetValueInt( INI_AWAY_SEC);

	// always On Top
	CString strAlwaysTop;
	CINIManager::Instance().GetValue(INI_ALWAYS_ON_TOP, strAlwaysTop);

	if(strAlwaysTop == "Y")
		SetWindowPos(&wndTopMost, 0, 0, 0, 0 ,SWP_NOSIZE | SWP_NOMOVE); 
	else
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0 ,SWP_NOSIZE | SWP_NOMOVE); 
	
	//m_AwaySec = 10 ; // 
	//SetTimer(100, 10000, 0);  // Hooking Timer Set	

	PostMessage(WM_INIT_WORK, (WPARAM)NULL, (LPARAM)NULL);		
	
	m_strMyNick = " (�¶���) " + CChatSession::Instance().m_Nick ;
		
	m_nMyStatus = IDB_BITMAP_ONLINE_B;

	m_tooltip.Create(this);
    // AddTool �Լ��� ��Ʈ�� ��ü�� �����Ϳ� ���� ���ڿ��� ����
    m_tooltip.AddTool(&m_btnMyStatus, "������ ����");

	
	// ������ �޴���ư 
	m_btnMyStatus.AutoSize(this, IDB_BITMAP_MYSTATUS_CLICK);
	m_btnMyStatus.SetBitmaps(IDB_BITMAP_MYSTATUS_CLICK, RGB(255, 0, 255), 
				IDB_BITMAP_MYSTATUS_OVER, RGB(255, 0, 255));
	m_btnMyStatus.DrawBorder(FALSE);
	m_btnMyStatus.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);
	m_btnMyStatus.SetMenu(IDR_MENU_MY_STATUS, m_hWnd , FALSE, NULL);
	m_btnMyStatus.SetMenuCallback(m_hWnd, WM_USER + 1000);	
	
	// secBtn 1 
	m_tooltip.AddTool(&m_btnSec1, "ä��");	
	m_btnSec1.AutoSize(this, IDB_BITMAP_SEC_BTN1_CLICKED);
	m_btnSec1.SetBitmaps(IDB_BITMAP_SEC_BTN1_CLICKED, RGB(255, 0, 255), 
				IDB_BITMAP_SEC_BTN1_CLICKED, RGB(255, 0, 255));
	m_btnSec1.DrawBorder(FALSE);
	m_btnSec1.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

	// secBtn 2
	m_tooltip.AddTool(&m_btnSec2, "�Ĵ� �޴�");	
	m_btnSec2.AutoSize(this, IDB_BITMAP_SEC_BTN2_CLICKED);
	m_btnSec2.SetBitmaps(IDB_BITMAP_SEC_BTN2_OVER, RGB(255, 0, 255), 
				IDB_BITMAP_SEC_BTN2_HIDE, RGB(255, 0, 255));
	m_btnSec2.DrawBorder(FALSE);
	m_btnSec2.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

	// secBtn 3
	m_tooltip.AddTool(&m_btnSec3, "������");	
	m_btnSec3.AutoSize(this, IDB_BITMAP_SEC_BTN1_CLICKED);
	m_btnSec3.SetBitmaps(IDB_BITMAP_SEC_BTN1_OVER, RGB(255, 0, 255), 
				IDB_BITMAP_SEC_BTN1_HIDE, RGB(255, 0, 255));
	m_btnSec3.DrawBorder(FALSE);
	m_btnSec3.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

	/*
	WINDOWPLACEMENT *wpl;
	unsigned int sz = sizeof(WINDOWPLACEMENT);
	if (AfxGetApp()->GetProfileBinary("Settings", "WindowPos", (LPBYTE *)&wpl, &sz))
	{
		SetWindowPlacement(wpl);
		delete [] wpl;
	}
	*/

	// ���� �����ߴ� ũ��� ��ġ�� �̵�..
	CString strPosSrc, strPosX, strPosY, strWidth, strHight;
	CINIManager::Instance().GetValue(INI_PREV_POS_SIZE_MAIN, strPosSrc);	

	int nLen = strPosSrc.GetLength();
	int nPos = -1;	
	nPos = strPosSrc.Find('|');  strPosX = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strPosY = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strWidth = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	nPos = strPosSrc.Find('|');  strHight = strPosSrc.Mid(0, nPos); strPosSrc = strPosSrc.Mid(nPos+1, nLen - nPos);
	
	int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);

	if(  atoi((LPCSTR)strPosX) >=0 && atoi((LPCSTR)strPosY) >= 0 && atoi((LPCSTR)strWidth) <= nWidth && atoi((LPCSTR)strHight) <= nHeight )
	{
		MoveWindow( atoi((LPCSTR)strPosX),atoi((LPCSTR)strPosY),atoi((LPCSTR)strWidth),atoi((LPCSTR)strHight));
	}	

	UpdateData(FALSE);
	
	/*
	HBITMAP aAnimation[] =	{	::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_OffLine ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_OffLine ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_OffLine ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) ),
								::LoadBitmap( AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BITMAP_ONLINE ) )
							};	
	*/
	
	//m_TreeCtrl.AddItemAnimation( hICQContact1, ppFirstLeft, sizeof( aAnimation ) / sizeof( HBITMAP ), aAnimation );
	
	// �˶������ ���ؼ� xml ������ �о map�� ���� 
	/*
	if ( ! InitAlarmMapData() )
	{
		AfxMessageBox("�˶� ������ ó���� �����Դϴ�. �����ڿ��� �����ϼ���.");				
		CDialog::OnOK();
		return TRUE;
	}
    */

	//Color
	//Bg
	CString szColorBGR, szColorBGG , szColorBGB;	
	CINIManager::Instance().GetValue( INI_COLOR_BG_R, szColorBGR);
	CINIManager::Instance().GetValue( INI_COLOR_BG_G, szColorBGG);
	CINIManager::Instance().GetValue( INI_COLOR_BG_B, szColorBGB);	
	CChatSession::Instance().m_ColorBG = RGB	(
						atoi( szColorBGR),
						atoi( szColorBGG),
						atoi( szColorBGB)
					);	
	//Buddy
	CString szColorCompR, szColorCompG , szColorCompB;	
	CINIManager::Instance().GetValue( INI_COLOR_COMP_R, szColorCompR);
	CINIManager::Instance().GetValue( INI_COLOR_COMP_G, szColorCompG);
	CINIManager::Instance().GetValue( INI_COLOR_COMP_B, szColorCompB);
	
	CChatSession::Instance().m_ColorBuddy = RGB	(
							atoi( (LPCSTR)szColorCompR),
							atoi( (LPCSTR)szColorCompG),
							atoi( (LPCSTR)szColorCompB)
						);	
	//Me
	CString szColorMeR, szColorMeG , szColorMeB;	
	CINIManager::Instance().GetValue( INI_COLOR_ME_R, szColorMeR);
	CINIManager::Instance().GetValue( INI_COLOR_ME_G, szColorMeG);
	CINIManager::Instance().GetValue( INI_COLOR_ME_B, szColorMeB);

	CChatSession::Instance().m_ColorMe = RGB	(
						atoi( (LPCSTR)szColorMeR),
						atoi( (LPCSTR)szColorMeG),
						atoi( (LPCSTR)szColorMeB)
					);	

	//Font			
	unsigned int sz1 = sizeof(BYTE);	
	LOGFONT* lf;
	unsigned int sz2 = sizeof(LOGFONT);	

	if (AfxGetApp()->GetProfileBinary(INI_SEC_CONVER_FONT, INI_FONT, (LPBYTE *)&lf, &sz2))
	{	
		TRACE("Font ����ã��!!\n");
					
		CChatSession::Instance().m_cf.yHeight = - lf->lfHeight*15;		
		CChatSession::Instance().m_cf.crTextColor = CChatSession::Instance().m_ColorMe;	
		CChatSession::Instance().m_cf.bCharSet =  lf->lfCharSet;		
		CChatSession::Instance().m_cf.bPitchAndFamily = lf->lfPitchAndFamily;			
		CChatSession::Instance().m_cf.crBackColor = CChatSession::Instance().m_ColorBG;        
		strcpy(CChatSession::Instance().m_cf.szFaceName, (LPCTSTR) lf->lfFaceName );

		CChatSession::Instance().m_Font.CreateFontIndirect(lf);	
	}
	else
	{		
		TRACE("Font ������ ��ã��!!\n");
		lf = new LOGFONT;
		CFont* font = GetFont();			
		if (font)
		{			
			font->GetLogFont(lf);			
			lf->lfHeight = 12;
			
			strcpy(lf->lfFaceName, "����");			
			CChatSession::Instance().m_Font.CreateFontIndirect(lf);	
		}		
		delete [] lf;		
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CIdmsMsnDlg::InitAlarmMapData()
{		
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
	// "D:\AA\BB\CC\DD\IdmsMsn.exe" �̷� ���ڿ����� D:\AA\BB\CC\DD\ ���� ���Ѵ�.	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	TRACE("%s\n" , szDirTmp);		
	strFileName.Format("%s\\memocal.xml", szDirTmp);
	TRACE("%s\n" , strFileName);		
	
	//CXMLDataManager::Instance().OpenDocument(_T(strFileName));

	return TRUE; 
}

LRESULT CIdmsMsnDlg::OnViewAllMsgDlgClose(WPARAM wParam, LPARAM lParam)
{	
	delete m_pAllMSgViewDlg;
	m_pAllMSgViewDlg = NULL;

	return TRUE;
}

// ���� ���� �����Ϳ� ���� ���������� Ȯ�� ����
LRESULT CIdmsMsnDlg::OnAllMsgAck(WPARAM wParam, LPARAM lParam)
{	
	char* szFromID = (char*)(lParam);
	
	if(m_pAllMSgSendDlg)
	{			
		CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
		::SendMessage(pWnd->m_hWnd , WM_ALLMSG_ACK, wParam, lParam);			
	}
	else
	{
		// â�� ������������ Ȯ�� ���·� ǥ������.		
	}

	return TRUE;
}



void CIdmsMsnDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();		
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIdmsMsnDlg::OnPaint() 
{
	/*
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	*/

	// ������ => ����
	CRect rc;
	GetClientRect(rc);
	int x = 0, y = 0;
	CPaintDC dc(this); // device context for painting
	
	COLORREF CrBg = RGB(162, 153, 203) ;
	//COLORREF CrBg = RGB(255, 137, 104) ;
	COLORREF CrWhiteBg = RGB(255, 255, 255) ;

	dc.FillSolidRect(0, 0, rc.right, 40, CrBg);  // ��ܺκ� 
	dc.FillSolidRect(50, 40, rc.right - 70 , 11, CrWhiteBg);  // ��ܹ� ��κ� 

	dc.FillSolidRect(0, 28, 40, rc.bottom - 56, CrBg );  // ���� 	
	dc.FillSolidRect(40, 50, 21, rc.bottom - 100, CrWhiteBg );  // ���� ��κ� 
		
	dc.FillSolidRect(rc.right - 10, 40, 20, rc.bottom - 56, CrBg ); //���� 	
	dc.FillSolidRect(rc.right - 23, 50, 13, rc.bottom - 99, CrWhiteBg ); //���� ��κ� 	
		
	dc.FillSolidRect(0, rc.bottom - 40, rc.right - 10, 20, CrBg ); //�ϴ�
	dc.FillSolidRect(50, rc.bottom - 75 ,rc.right - 70 , 35, CrWhiteBg);  // �ϴܹ� ��κ�	

	CBitmap Bitmap;
	// ���� ���� ǥ�� �׸�
	
	Bitmap.LoadBitmap(m_nMyStatus);
					
	dc.DrawState(CPoint(7, 10), CSize(32, 32), Bitmap, NULL, NULL); 
	Bitmap.DeleteObject();


	// Tree �����̵�	
	Bitmap.LoadBitmap(IDB_TOPLEFT);
	dc.DrawState(CPoint(40, 40), CSize(10, 10), Bitmap, NULL, NULL); // topleft
	Bitmap.DeleteObject();

	Bitmap.LoadBitmap(IDB_TOPRIGHT);
	dc.DrawState(CPoint(rc.right - 20, 40), CSize(10, 10), Bitmap, NULL, NULL);
	Bitmap.DeleteObject();

	Bitmap.LoadBitmap(IDB_BOTTOMLEFT );
	dc.DrawState(CPoint(40, rc.bottom - 50), CSize(10, 10), Bitmap, NULL, NULL);
	Bitmap.DeleteObject();

	Bitmap.LoadBitmap( IDB_BOTTOMRIGHT);
	dc.DrawState(CPoint(rc.right - 20, rc.bottom - 50), CSize(10, 10), Bitmap, NULL, NULL);
	Bitmap.DeleteObject();	

	
	// Nick ���� ǥ�� static ����
	//COLORREF CrBgStatusTxt = RGB(162, 153, 203) ;
	COLORREF CrStatusTxtBg = RGB(0, 0, 0) ;
	
	dc.FillSolidRect(53 + 24, 15, rc.right - 93 , 2, CrStatusTxtBg);  // ���
	
	dc.FillSolidRect(51+ 24, 17, 2, rc.top + 12, CrStatusTxtBg );  // ����
	
	dc.FillSolidRect(rc.right - 40 + 24, rc.top + 17, 2 , rc.top + 12, CrStatusTxtBg ); //����	
	
	dc.FillSolidRect(53+ 24, 29, rc.right - 93 , 2, CrStatusTxtBg);  // �ϴ�

	// Nick ���� ǥ�� static ���� Round
	//CrStatusTxtBg = RGB(255, 0, 0) ;
	dc.FillSolidRect(52+ 24, 16, 1 , 1, CrStatusTxtBg);  // �� ��� �𼭸�
	dc.FillSolidRect(52+ 24, 29, 1 , 1, CrStatusTxtBg);  // �� �ϴ� �𼭸�
	dc.FillSolidRect(rc.right - 16, rc.top + 16, 1 , 1, CrStatusTxtBg ); //�� ��� �𼭸�	
	dc.FillSolidRect(rc.right - 16, rc.top + 29, 1 , 1, CrStatusTxtBg ); //�� �ϴ� �𼭸�		
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIdmsMsnDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


// ���������� ����.
LRESULT CIdmsMsnDlg::OnServerClosed(WPARAM wParam, LPARAM lParam)
{
	POSITION pos;
	CString key;
			
	//ä��
	CChatWnd * pa1;
	
	for( pos = mStrChatDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pa1 );			
			if(pa1)
			{
				delete pa1;
			}
		}		
	}
	mStrChatDlgMap.RemoveAll();		
	
	// Multi Chat	
	CMultiChatWnd* pa2;	
	for( pos = mStrMultiChatDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrMultiChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pa2 );			
			if(pa2)
			{				
				delete pa2;
			}
		}		
	}
	mStrMultiChatDlgMap.RemoveAll();		

	// Slip
	CViewBuddySlipMsg* pa3;	
	for( pos = mStrSlipMsgDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrSlipMsgDlgMap.GetNextAssoc( pos, key, (CObject*&)pa3 );			
			if(pa3)
			{				
				delete pa3;
				pa3 = NULL;
			}
		}		
	}
	mStrSlipMsgDlgMap.RemoveAll();		
	
	MAP_STR2SET::iterator it;
	SET_STR::iterator itSet;

	for (it= CChatSession::Instance().m_mapStr2SetMultiChat.begin(); it != CChatSession::Instance().m_mapStr2SetMultiChat.end(); ++it) 
	{
		if(it != CChatSession::Instance().m_mapStr2SetMultiChat.end() )  
		{
			CChatSession::Instance().m_mapStr2SetMultiChat.erase(it);	
		}
	}

	CChatSession::Instance().m_vecUserIDOrdered.clear();	
	
	CBuddyInfo * paComp;	
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnServerClosed"); 
	for( pos =  CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); pos != NULL; ){		
		if(pos)
		{
			CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( pos, key, (CObject*&)paComp );			
			if(paComp)
			{				
				delete paComp;
				paComp = NULL;
			}
		}		
	}
	CChatSession::Instance().mBuddyInfoMap.RemoveAll();

	CChatSession::Instance().m_StrAryGrpName.RemoveAll();

	CChatSession::Instance().mIDToNameMap.RemoveAll();

	CChatSession::Instance().mITCompDescToUrlMap.RemoveAll();
		
	CChatSession::Instance().Stop(FALSE); // 20070529 
	////////////////////////////////////////////////////////////////////

	// ��� Off Line ǥ��
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
		
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked)
				{					
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, 
												szRealName, szGrpName, FALSE, nBockMode, n_StatusFlags);
					
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					if(nBockMode != 9)
					{
						m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);					
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hOffLineBmp, ppFirstLeft );
						CRect rectItem;
						m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
						m_TreeCtrl.InvalidateRect(rectItem);
					}					
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}

	m_TreeCtrl.EnableWindow(FALSE);
	////////////////////////////////////////////////////////////////////
	char szMsg[100];
	memset(szMsg, 0x00, sizeof(szMsg));
	sprintf(szMsg, "���� ���� ������ %d�� �õ�", ++m_nCntReConnect);
	m_statusBar.SetWindowText(szMsg);	
	
	
	KillTimer(100); 	
	KillTimer(500); //Alive
	KillTimer(600); //Alarm 
	
	KillTimer(300);
	SetTimer (300, 15000, NULL); // ������ �õ�
		
	FlashWindow(TRUE);	
	
	return TRUE;
}


// �ʱ�ȭ �۾�
LRESULT CIdmsMsnDlg::OnInitWork(WPARAM wParam, LPARAM lParam)
{
	TRACE("OnInitWork\n");
	m_hContactList = m_TreeCtrl.InsertItem( "ROOT",  TVI_LAST );	
	m_TreeCtrl.SetItemColor( m_hContactList, m_ColorGrp );		
	
	//InsertGrpNode("�⺻�׷�"); //�⺻�׷� ����
	//InsertGrpNode("���ͳ� ���ã��"); //���ã�� �׷� ����
	
	//LogReconn
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "RequestBuddyGrp Call [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

	// ��ȭ��밡 ���� �׷� ������ ��û	
	if(CChatSession::Instance().RequestBuddyGrp( m_strLoginID ) != TRUE)
	{
		//LogReconn
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "RequestBuddyGrp Fail! [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

		return FALSE ;
	}
	
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnSQLRsltErr(WPARAM wParam, LPARAM lParam)
{
	S_COM_RESPONSE* pErrInfo = (S_COM_RESPONSE*)lParam;
	
	CSP_Utility<S_COM_RESPONSE> spTest = pErrInfo;

	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, pErrInfo->szErrMsg);	
	pMsgThread->CreateThread();

	return FALSE;
}




LRESULT CIdmsMsnDlg::OnDelGrpRslt(WPARAM wParam, LPARAM lParam)
{	
	int i = 0;
	int j = 0;
	BOOL bChildExists = FALSE;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CArray<HTREEITEM,HTREEITEM> tmpDelNodeAry;		
		
	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(i = 0; i < pArray->GetSize(); i++)
	{
		S_BUDDY_GRP *pData = static_cast<S_BUDDY_GRP*>(pArray->GetAt(i));
	
		HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
		CString ItemText;	
					
		while (hCurrent != NULL)
		{
			if ( !m_TreeCtrl.ItemHasChildren(hCurrent) )
			{
				memset(szSelItemIP, 0x00, sizeof(szSelItemIP));
				memset(szNickName, 0x00, sizeof(szNickName));
				memset(szRealName, 0x00, sizeof(szRealName));
				memset(szGrpName, 0x00, sizeof(szGrpName));
				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode,&n_StatusFlags);
				
				if(pData->m_strBuddyGrp.Compare(szGrpName)==0)
				{
					// �׷� ����
					tmpDelNodeAry.Add( hCurrent);
					break;
				}
			}
			
			hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);			
		}	
		
	}
	
	for ( i=0;i <= tmpDelNodeAry.GetUpperBound();i++)
	{
	   HTREEITEM hDel  = tmpDelNodeAry.GetAt(i);
	   m_TreeCtrl.DeleteItem(hDel) ; 		
	}
	
	// Clear
	for( j = 0; j < pArray->GetSize(); j++)
	{
		S_BUDDY_GRP *pData = static_cast<S_BUDDY_GRP*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;

	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "�׷��� �����Ǿ����ϴ�.");	
	pMsgThread->CreateThread();

	return TRUE;
}



LRESULT CIdmsMsnDlg::OnChgGrpNameRslt(WPARAM wParam, LPARAM lParam)
{	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;		

	int nCntGrp = 0;
	int nFoundDefaultGrp = 0;

	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_GRP_CHG_NAME* pInfo = static_cast<S_GRP_CHG_NAME*>(pArray->GetAt(i));
		
		//pInfo->m_szBefGrp 	pInfo->m_szAftGrp
		
		// �������� �׷쿡 �Ҽӵ� ��ȭ����� �׷��� ����Ȱ����� �ٲ�.
		HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
		CString ItemText;
		HTREEITEM hChildItem;
		TreeItemData* pData = NULL;

		while (hCurrent != NULL)
		{
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;	
	
			m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, 
										bConnStatus, &nBockMode, &n_StatusFlags);
			if ( strcmp(pInfo->m_szBefGrp, szGrpName)==0) 
			{
				//Grp Text ����
				m_TreeCtrl.SetItemUserInfo(	hCurrent, szSelItemIP, szSelItemID, szNickName, szRealName, 
											pInfo->m_szAftGrp, bConnStatus, nBockMode, n_StatusFlags);
				m_TreeCtrl.SetItemText(hCurrent, pInfo->m_szAftGrp);

				CRect rectItem;
				m_TreeCtrl.GetItemRect(hCurrent, &rectItem,FALSE);
				m_TreeCtrl.InvalidateRect(rectItem);
			}			
			
			if (m_TreeCtrl.ItemHasChildren(hCurrent))
			{
				//Child
				HTREEITEM hNextItem;
				hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
				
				while (hChildItem != NULL)
				{				
					bConnStatus = FALSE;
					nBockMode = 0;
					n_StatusFlags = 0l;

					BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, 
															szGrpName, bConnStatus, &nBockMode,&n_StatusFlags);
					
					if (bWorked && strcmp(pInfo->m_szBefGrp, szGrpName)==0) 
					{						
						m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, 
													pInfo->m_szAftGrp, bConnStatus, nBockMode, n_StatusFlags);					
						// MAP�� ����
						CBuddyInfo*  pCompanion = NULL;

						plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnChgGrpNameRslt"); 
					
						if( CChatSession::Instance().mBuddyInfoMap.Lookup(szSelItemID, ( CObject*& ) pCompanion ))
						{
							strcpy(pCompanion->m_szBuddyGrp,  pInfo->m_szAftGrp);
							CChatSession::Instance().mBuddyInfoMap.SetAt(szSelItemID, pCompanion);
						}
					}

					hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
					hChildItem = hNextItem;				
				}
			}

			hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);		
		}
	}
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_GRP_CHG_NAME *pData = static_cast<S_GRP_CHG_NAME*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;

	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "�׷���� ����Ǿ����ϴ�.");	
	pMsgThread->CreateThread();

	return TRUE;
}

// �׷캯�� 
LRESULT CIdmsMsnDlg::OnChgBuddyGrpRslt(WPARAM wParam, LPARAM lParam)
{	
	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_BUDDY_CHG *pInfo = static_cast<S_BUDDY_CHG*>(pArray->GetAt(i));

		// pInfo->m_szBuddyID;
		// pInfo->m_szBuddyGrp;

		// ���� �������� �׷��� ����, ȭ��󿡼��� �̵���Ŵ
		CBuddyInfo*  pCompanion = NULL;

		if( CChatSession::Instance().mBuddyInfoMap.Lookup(pInfo->m_szBuddyID, ( CObject*& ) pCompanion ))
		{
			if(pCompanion)
			{
				plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnChgBuddyGrpRslt"); 

				strcpy(pCompanion->m_szBuddyGrp, pInfo->m_szBuddyGrp);
				CChatSession::Instance().mBuddyInfoMap.SetAt(pInfo->m_szBuddyID, pCompanion);

				// ȭ��󿡼� Ʈ������ ��Ŵ
				chgCompGrp(pInfo->m_szBuddyID, pInfo->m_szBuddyGrp, pCompanion->m_nAway );				
			}
		}	
	}		
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_BUDDY_CHG *pData = static_cast<S_BUDDY_CHG*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;

	//CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "�׷��� ����Ǿ����ϴ�.");	
	//pMsgThread->CreateThread();

	return TRUE;
}

void CIdmsMsnDlg::chgCompGrp(const char* szID, const char* szChgGrpName, const int nAway)
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
				
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	CString ItemText;
	HTREEITEM hChildItem;
	TreeItemData* pData = NULL;
				
	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			//Child
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{			
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, 
					szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp(szSelItemID, szID)==0)
				{						
					// Old ����..					
					pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					DWORD nTest1 = pData->aPostImages.GetCount();
					DWORD nTest2 = pData->aPreImages.GetCount();
					DWORD nIndex = m_TreeCtrl.DeleteItem(hChildItem);
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);		
	}
				
EXIT_LOOP: ;
		   
	char szSelItemIP2	[15 + 1];	
	char szSelItemID2   [20 + 1];
	char szRealName2	[20 + 1];
	char szNickName2	[C_NICK_NAME_LEN +1];
	char szGrpName2     [C_GRP_NAME_LEN +1];
	BOOL bConnStatus2 = FALSE;
	int nBockMode2 = 0;
	long n_StatusFlags2 = 0l;
		   
	hCurrent = m_TreeCtrl.GetRootItem();
		   
	while (hCurrent != NULL)
	{		
		bConnStatus2 = FALSE;
		nBockMode2 = 0;
		n_StatusFlags2 = 0l;

	   //Grp
	   BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP2, szSelItemID2, szNickName2, 
				   szRealName2, szGrpName2, bConnStatus2, &nBockMode2, &n_StatusFlags2);
			   
	   // GRP NODE, ����� �׷쿡 Insert
	   if	(
				strcmp(szGrpName2, szChgGrpName) == 0 && 
				strncmp(szSelItemIP2, "GRP", sizeof(szSelItemIP2)) == 0
			)
	   {
		   char szTmp[130];
		   memset(szTmp,0x00, sizeof(szTmp));
		   sprintf(szTmp,"[%s] %s",szRealName, szNickName);
		   
		   HTREEITEM hUser = m_TreeCtrl.InsertItem( szTmp, hCurrent, TVI_LAST );
		   m_TreeCtrl.SetItemUserInfo(	hUser,	szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, bConnStatus, 0, nAway);
		   m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
		   
		   DWORD nIndex = 0;
		   
		   if(bConnStatus == TRUE)
		   {	
			   if ( nAway & CFL_AWAY)
			   {
				   m_TreeCtrl.AddItemBitmap( hUser, m_hAwayBmp, ppFirstLeft );
			   }
			   else
			   {
				   m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppFirstLeft );			
			   }
		   }
		   else
		   {				
			   nIndex = m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppFirstLeft );				
		   }						
		   
		   m_TreeCtrl.Expand(hCurrent,TVE_EXPAND );					
		   
		   goto EXIT_LOOP2;
	   }		
	   
	   hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);		
	}
		   
EXIT_LOOP2: ;  
}							


LRESULT CIdmsMsnDlg::OnBuddyOffline(WPARAM wParam, LPARAM lParam)
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	S_BUDDY_OFFLINE * pOfflineBuddy = (S_BUDDY_OFFLINE*) lParam;	
	CSP_Utility<S_BUDDY_OFFLINE> spTest = pOfflineBuddy;

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OffBuddyOffline"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{	
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pOfflineBuddy->m_szBuddyID, szSelItemID)==0)
				{					
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, 
												szRealName, szGrpName, FALSE, nBockMode, n_StatusFlags);
					
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);
					m_TreeCtrl.AddItemBitmap( hChildItem, m_hOffLineBmp, ppFirstLeft );
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					m_TreeCtrl.InvalidateRect(rectItem);
					
					// MAP���� ������� ����					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pOfflineBuddy->m_szBuddyID, ( CObject*& ) pa ))
					{
						if(pa)
						{
							pa->m_bConnStatus = FALSE;
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pOfflineBuddy->m_szBuddyID , pa);							
						}
					}
					
					// Chat �����쿡 ����
					CChatWnd* pa;	
					if(mStrChatDlgMap.Lookup(pOfflineBuddy->m_szBuddyID, ( CObject*& ) pa ))
					{	
						CChatWnd * pDlg = static_cast<CChatWnd*>(pa);
						if(pDlg)
						{							
							::PostMessage(pDlg->GetSafeHwnd(), WM_BUDDY_OFFLINE, 0, (LPARAM)pOfflineBuddy->m_szBuddyID); 
						}						
					}

					// Multi chat
					CMultiChatWnd * pMCDlg;	
					CString key;					
					POSITION pos;
					for( pos = mStrMultiChatDlgMap.GetStartPosition(); pos != NULL; )
					{
						if(pos)
						{
							mStrMultiChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pMCDlg );			
							if(pMCDlg)
							{								
								pMCDlg->PostMessage(WM_BUDDY_OFFLINE, 0, (LPARAM)pOfflineBuddy->m_szBuddyID); // 20061227
							}
						}		
					}

					//allMsg
					if(m_pAllMSgSendDlg)
					{
						//CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());						
						m_pAllMSgSendDlg->PostMessage(WM_BUDDY_OFFLINE, 0, (LPARAM)pOfflineBuddy->m_szBuddyID); // 20061227
					}
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
		   
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnALive(WPARAM wParam, LPARAM lParam)
{
	//InterlockedDecrement(&m_nAliveAckFailed);
	
	TRACE("����: m_nAliveAckFailed [%d]\n",m_nAliveAckFailed);

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAwayTrue(WPARAM wParam, LPARAM lParam)
{
	//TRACEX("CIdmsMsnDlg::OnAwayTrue Start\n");

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	
	S_AWAY_INFO * pAwayInfo = (S_AWAY_INFO*) lParam;	
	CSP_Utility<S_AWAY_INFO> spTest = pAwayInfo; 
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnAwayTrue"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{			
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;

				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
														szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pAwayInfo->szIDFrom, szSelItemID)==0)
				{						
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);					
					
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					
					//�ڸ����
					n_StatusFlags |= CFL_AWAY;						
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP,szSelItemID, szNickName,
													szRealName, szGrpName, TRUE, nBockMode,n_StatusFlags ) ; 
					
					if(nBockMode==4)
					{						
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hBlockBmp, ppFirstLeft );
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hAwayBmp, ppFirstLeft );
					}
					
					// MAP���� AwayInfo ����
					CBuddyInfo *pa = NULL;
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pAwayInfo->szIDFrom, ( CObject*& ) pa ))
					{
						if(pa)
						{
							pa->m_bConnStatus = TRUE;							
							pa->m_nAway = 1; // 0 : Online, 1 : awy , 2 : Busy
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pAwayInfo->szIDFrom , pa);							
						}
					}
					m_TreeCtrl.InvalidateRect(rectItem);
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;

	//TRACEX("CIdmsMsnDlg::OnAwayTrue End\n");

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAwayFalse(WPARAM wParam, LPARAM lParam)
{
	//TRACEX("CIdmsMsnDlg::OnAwayFalse Start\n");

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	S_AWAY_INFO * pAwayInfo = (S_AWAY_INFO*) lParam;	
	
	CSP_Utility<S_AWAY_INFO> spTest = pAwayInfo; 

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnAwayFalse"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pAwayInfo->szIDFrom, szSelItemID)==0)
				{						
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);					
					
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
										
					//OnLine						
					n_StatusFlags &= ~CFL_AWAY;	
					n_StatusFlags &= ~CFL_BUSY;						
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP,szSelItemID, szNickName,
													szRealName, szGrpName, TRUE, nBockMode,n_StatusFlags ) ;

					if(nBockMode==4)
					{						
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hBlockBmp, ppFirstLeft );
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hOnLineBmp, ppFirstLeft );
					}
					
					// MAP���� AwayInfo ����
					CBuddyInfo *pa = NULL;
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pAwayInfo->szIDFrom, ( CObject*& ) pa ))
					{
						if(pa)
						{	
							pa->m_bConnStatus = TRUE;							
							pa->m_nAway = 0; // 0 : Online, 1 : awy , 2 : Busy
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pAwayInfo->szIDFrom , pa);							
						}
					}

					m_TreeCtrl.InvalidateRect(rectItem);
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
	
	//TRACEX("CIdmsMsnDlg::OnAwayFalse End\n");
	return TRUE;
}



LRESULT CIdmsMsnDlg::OnStatusBusy(WPARAM wParam, LPARAM lParam)
{
	TRACEX("CIdmsMsnDlg::OnStatusBusy Start\n");

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	S_AWAY_INFO * pAwayInfo = (S_AWAY_INFO*) lParam;	
	
	CSP_Utility<S_AWAY_INFO> spTest = pAwayInfo; 

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnAwayFalse"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pAwayInfo->szIDFrom, szSelItemID)==0)
				{						
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);					
					
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
										
					//OnLine						
					n_StatusFlags |= CFL_BUSY;		
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP,szSelItemID, szNickName,
													szRealName, szGrpName, TRUE, nBockMode,n_StatusFlags ) ;
					m_TreeCtrl.AddItemBitmap( hChildItem, m_hBusyBmp, ppFirstLeft );
					
					// MAP���� AwayInfo ����
					CBuddyInfo *pa = NULL;
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pAwayInfo->szIDFrom, ( CObject*& ) pa ))
					{
						if(pa)
						{	
							pa->m_bConnStatus = TRUE;							
							pa->m_nAway = 2; // 0 : Online, 1 : awy , 2 : Busy
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pAwayInfo->szIDFrom , pa);							
						}
					}

					m_TreeCtrl.InvalidateRect(rectItem);
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
	
	TRACEX("CIdmsMsnDlg::OnAwayFalse End\n");
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnChngNick(WPARAM wParam, LPARAM lParam)
{//S_CHG_NICK
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	S_CHG_NICK * pChngNic = (S_CHG_NICK*) lParam;	
	
	CSP_Utility<S_CHG_NICK> spTest = pChngNic; 

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnChngNick"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{	
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;

				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pChngNic->szIDFrom, szSelItemID)==0)
				{						
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, pChngNic->szNickAft, 
												szRealName, szGrpName, TRUE, nBockMode, n_StatusFlags);
					
					CRect rectItem;
					char szTmp[C_NICK_NAME_LEN+50+1+1];
					memset(szTmp, 0x00, sizeof(szTmp));
					sprintf(szTmp, "[%s] %s", szRealName, pChngNic->szNickAft);						
					m_TreeCtrl.SetItemText(hChildItem, szTmp); 
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					m_TreeCtrl.InvalidateRect(rectItem);

					// MAP���� NICK ����
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pChngNic->szIDFrom, ( CObject*& ) pa ))
					{
						if(pa)
						{	
							pa->m_bConnStatus = TRUE;							
							strncpy(pa->m_szNickName , pChngNic->szNickAft, sizeof(pa->m_szNickName));
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pChngNic->szIDFrom , pa);							
						}
					}
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
		   
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnBuddyOnline(WPARAM wParam, LPARAM lParam)
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;
	S_BUDDY_ONLINE * pOnlineBuddy = (S_BUDDY_ONLINE*) lParam;	
	CSP_Utility<S_BUDDY_ONLINE> spTest = pOnlineBuddy;

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnBuddyOnline"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
	
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pOnlineBuddy->m_szBuddyID, szSelItemID)==0)
				{				
					TRACEX("pOnlineBuddy->m_szBuddyIP [%s]\n", pOnlineBuddy->m_szBuddyIP);

					m_TreeCtrl.SetItemUserInfo(	hChildItem, pOnlineBuddy->m_szBuddyIP, szSelItemID, szNickName, szRealName, szGrpName, TRUE, nBockMode, n_StatusFlags);
					
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);

					// ��ȭ���ܵ� �����̸� �������� ����ǥ�÷�..
					if(nBockMode == 4)
					{
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hBlockBmp, ppFirstLeft );
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hOnLineBmp, ppFirstLeft );
					}
					
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					rectItem.bottom+=3;
					m_TreeCtrl.InvalidateRect(rectItem);
					
					// MAP���� ������� ����								
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pOnlineBuddy->m_szBuddyID, ( CObject*& ) pa ))
					{
						if(pa)
						{
							pa->m_bConnStatus = TRUE;							
							strncpy(pa->m_szIpAddr , pOnlineBuddy->m_szBuddyIP, sizeof(pa->m_szIpAddr));
							// �ٽ� ���� UPDATE
							CChatSession::Instance().mBuddyInfoMap.SetAt(pOnlineBuddy->m_szBuddyID , pa);							
						}
					}

					// Chat �����쿡 ����
					CChatWnd* pa;	
					if(mStrChatDlgMap.Lookup(pOnlineBuddy->m_szBuddyID, ( CObject*& ) pa ))
					{	
						CChatWnd * pDlg = static_cast<CChatWnd*>(pa);
						if(pDlg)
						{
							//::SendMessage(pDlg->GetSafeHwnd(), WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID);		
							::PostMessage(pDlg->GetSafeHwnd(), WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID);	 //20061227	
						}						
					}

					// Multi chat
					CMultiChatWnd * pMCDlg;	
					CString key;					
					POSITION pos;
					for( pos = mStrMultiChatDlgMap.GetStartPosition(); pos != NULL; )
					{
						if(pos)
						{
							mStrMultiChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pMCDlg );			
							if(pMCDlg)
							{
								CWnd* pWnd = CWnd::FromHandle(pMCDlg->GetSafeHwnd());
								//pWnd->SendMessage(WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID);
								pWnd->PostMessage(WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID); //20061227
							}
						}		
					}

					//allMsg
					if(m_pAllMSgSendDlg)
					{
						CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
						//pWnd->SendMessage(WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID);
						pWnd->PostMessage(WM_BUDDY_ONLINE, 0, (LPARAM)pOnlineBuddy->m_szBuddyID); //20061227
					}
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
		   
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnSomeOneDelMe(WPARAM wParam, LPARAM lParam)
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;

	CBuddyInfo *pa = NULL;
	
	S_SOMEONE_DEL_ME* pWhoDelMeInfo  = (S_SOMEONE_DEL_ME*)lParam;
	
	CSP_Utility<S_SOMEONE_DEL_ME> spTest = pWhoDelMeInfo;

	///////////////
	// icon�ٲٰ�
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
	
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnSomeOneDelMe"); 

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{		
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;

				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && strcmp( pWhoDelMeInfo->m_szWhoDelMeID, szSelItemID)==0)
				{					
					// ��ȭ���� ���·�	: 3 ���� ������ 4 ���濡 ���ؼ� ���ܴ���
					
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, FALSE, 4, n_StatusFlags);
					
					TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
					m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);
					m_TreeCtrl.AddItemBitmap( hChildItem, m_hBlockBmp, ppFirstLeft );
					CRect rectItem;
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					m_TreeCtrl.InvalidateRect(rectItem);
					
					// MAP���� ���� ������ ������ m_nYouBlocked 4 ���� �����Ѵ�!			
					// ��. �׻������� ���� �޽����� ���ܵǾ� ������ ����. 
					
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(pWhoDelMeInfo->m_szWhoDelMeID, ( CObject*& ) pa ))
					{
						if(pa)
						{
							// ���� �����ߴٴ� �޽����� ���� ������, ���� �����ߴ� ����� ���� ���� (�޸�,DB)  
							// DB������ �̹� �����Ŀ� �޽����� �´�.. Ʈ�������� �̹� ������ �ʴ� �����̴�.								
							if(pa->m_nYouBlocked == 3)
							{
								// ���� ����
								TRACEX("���� ����\n");
								CChatSession::Instance().mBuddyInfoMap.RemoveKey(pWhoDelMeInfo->m_szWhoDelMeID);
								delete pa;
								pa = NULL;
							}
							else
							{								
								pa->m_nYouBlocked = 4;
								// �ٽ� ���� UPDATE
								CChatSession::Instance().mBuddyInfoMap.SetAt(pWhoDelMeInfo->m_szWhoDelMeID , pa);
							}
						}							
					}
					
					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
	EXIT_LOOP: ;		
	
	
	if(pWhoDelMeInfo != NULL)
	{			
		char szMsg[200];
		memset(szMsg, 0x00, sizeof(szMsg));
		sprintf(szMsg, "%s �� ��ȭ��� ��Ͽ��� ���� �߽��ϴ�. ������ ������ ����� ��ȭ���� �ٽ� �߰��Ҷ��� �޽����� ������ �ֽ��ϴ�.", pWhoDelMeInfo->m_szWhoDelMeID);

		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, szMsg);	
		pMsgThread->CreateThread();
	}

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnDelITBuddyRslt(WPARAM wParam, LPARAM lParam)
{
	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_IT_COMP_DEL* pData = static_cast<S_IT_COMP_DEL*>(pArray->GetAt(i));		
		//DeleteTreeItemByID(pData->m_szName);		

		char szSelItemIP	[15 + 1];	
		char szSelItemID    [20 + 1];
		char szRealName		[20 + 1];
		char szNickName		[C_NICK_NAME_LEN +1];
		char szGrpName      [C_GRP_NAME_LEN +1];
		BOOL bConnStatus = FALSE;
		int nBockMode = 0;
		long n_StatusFlags = 0l;

		HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
		
		CBuddyInfo *pa = NULL;
		
		while (hCurrent != NULL)
		{
			if (m_TreeCtrl.ItemHasChildren(hCurrent))
			{
				HTREEITEM hNextItem;
				HTREEITEM hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
				
				while (hChildItem != NULL)
				{			
					bConnStatus = FALSE;
					nBockMode = 0;
					n_StatusFlags = 0l;
					
					BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP, szSelItemID, szNickName, 
															szRealName, szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
					if (bWorked && strcmp(szNickName, pData->m_szName)==0)
					{					
						m_TreeCtrl.DeleteItem(hChildItem); //����..

						goto EXIT_LOOP;
					}

					hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
					hChildItem = hNextItem;				
				}
			}

			hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);		
		}

		EXIT_LOOP: ;

	}

	// Clear	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_IT_COMP_DEL *pData = static_cast<S_IT_COMP_DEL*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;
	
	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "���ͳ� ���ã�Ⱑ �����Ǿ����ϴ�.");	
	pMsgThread->CreateThread();

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnDelBuddyRslt(WPARAM wParam, LPARAM lParam)
{
	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_BUDDY_DEL* pData = static_cast<S_BUDDY_DEL*>(pArray->GetAt(i));		
		DeleteTreeItemByID(pData->m_szBuddyID);
	}
	
	// Clear	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_BUDDY_DEL *pData = static_cast<S_BUDDY_DEL*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;

	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "��ȭ��밡 �����Ǿ����ϴ�.");	
	pMsgThread->CreateThread();

	return TRUE;
}


void CIdmsMsnDlg::DeleteTreeItemByID(const char* szID)
{
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"DeleteTreeItemByID"); 

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	
	CBuddyInfo *pa = NULL;
	
	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			HTREEITEM hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{	
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;

				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP, szSelItemID, szNickName, 
														szRealName, szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
				if (bWorked && strcmp(szSelItemID, szID)==0)
				{
					TRACE("szSelItemID [%s]\n", szSelItemID); 
					
					m_TreeCtrl.DeleteItem(hChildItem); //����..
										
					if( CChatSession::Instance().mBuddyInfoMap.Lookup(szID, ( CObject*& ) pa ))
					{
						if(pa)
						{						
							if(pa->m_nYouBlocked == 4)
							{
								// �̹� A���濡 ���� B������ ���¿��� B�� A �� �����Ҷ� m_nYouBlocked == 4 �ΰ���.
								// �̶��� m_nYouBlocked == 5 �� ���� (A,B ��ȣ ����)
								pa->m_nYouBlocked = 5;
							}
							else
							{
								// MAP���� ���� ������ ������ m_nYouBlocked 3 ���� �����Ѵ�!
								pa->m_nYouBlocked = 3;
							}
							
							// �ٽ� ����
							CChatSession::Instance().mBuddyInfoMap.SetAt(szID , pa);
						}							
					}

					goto EXIT_LOOP;
				}

				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}

		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);		
	}

	EXIT_LOOP: ;

	//SaveCompanionList();

}

LRESULT CIdmsMsnDlg::OnAddGrpRslt(WPARAM wParam, LPARAM lParam)
{	
	S_ADD_BUDDY_CLISVR* pGrpInfo = (S_ADD_BUDDY_CLISVR*)lParam;

	CSP_Utility<S_ADD_BUDDY_CLISVR> spTest = pGrpInfo;

	// Ʈ���� �׷� �߰�!!!
	InsertGrpNode( pGrpInfo->m_szBuddyGrp ); 

	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "�׷��� �߰��Ǿ����ϴ�." );			
	pMsgThread->CreateThread();	

	return TRUE;
}


// ��ȭ��� �׷�
LRESULT CIdmsMsnDlg::OnBuddyGrpRslt(WPARAM wParam, LPARAM lParam)
{
	CMsgBoxThread* pMsgThread = NULL;
	CPtrArray* pArray  = (CPtrArray*)lParam;
	
	m_TreeCtrl.DeleteAllItems();
	InsertGrpNode("�⺻�׷�"); //�⺻�׷� ����
	InsertGrpNode("���ͳ� ���ã��"); //���ã�� �׷� ����
	
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_BUDDY_GRP *pData = static_cast<S_BUDDY_GRP*>(pArray->GetAt(i));		
		
		HTREEITEM* pTreeItem =  InsertGrpNode((LPSTR)(LPCSTR)pData->m_strBuddyGrp); //�׷� ����		
	}
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_BUDDY_GRP *pData = static_cast<S_BUDDY_GRP*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;
	
	// ������ �ƴѰ�츸 Tray Iconó��
	if(m_bReConnectFlag == FALSE)
	{
		// Ÿ�̸� ���� 
		SetTimer(200, 100, 0); // Tray Icon			
	}

	// ��ȭ ��� ����� ��û.. GET_BUDDY_LIST
	if(CChatSession::Instance().RequestBuddyList(m_strLoginID) != TRUE)
	{
		//LogReconn
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "RequestBuddyList Fail! [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

		return FALSE ;
	}
	

	return TRUE;
}


// ��ȭ��� �߰� ����!!
LRESULT CIdmsMsnDlg::OnAddBuddyRslt(WPARAM wParam, LPARAM lParam)
{
	CMsgBoxThread* pMsgThread = NULL;
	CPtrArray* pArray  = (CPtrArray*)lParam;

	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnAddBuddyRslt"); 

	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_ADD_BUDDY_RCV *pData = static_cast<S_ADD_BUDDY_RCV*>(pArray->GetAt(i));
				
		// MAP �� �߰�
		CBuddyInfo*	psDataTemp = new CBuddyInfo;
		strncpy(psDataTemp->m_szIpAddr , (LPCSTR)pData->m_strBuddyIP , sizeof(psDataTemp->m_szIpAddr) );					
		strncpy(psDataTemp->m_szUserName , (LPCSTR)pData->m_strBuddyName, sizeof(psDataTemp->m_szUserName) );					
		strncpy(psDataTemp->m_szNickName , (LPCSTR)pData->m_strBuddyNick, sizeof(psDataTemp->m_szNickName) );					
		strncpy(psDataTemp->m_szBuddyID , (LPCSTR)pData->m_strBuddyID, sizeof(psDataTemp->m_szBuddyID) );					
		strncpy(psDataTemp->m_szBuddyGrp , (LPCSTR)pData->m_strBuddyGrp, sizeof(psDataTemp->m_szBuddyGrp) );					
		/*			
		KIND ģ�� ���� 
		* 1 - ���� ����� ���
		* 2 - ������ ���� �߰��ؼ� ������� ���� ��ȭ��뿡 �߰���
		* 3 - ��ȭ �ź�
		* 4 - ��ȭ��밡 ���� ������
		*/
		psDataTemp->m_nYouBlocked = pData->m_nBlock;
		psDataTemp->m_bConnStatus = pData->m_bConnStatus;

		CChatSession::Instance().mBuddyInfoMap.SetAt(psDataTemp->m_szBuddyID , psDataTemp);

		// Ʈ���� �߰�
		HTREEITEM* pTreeItem =  InsertGrpNode(psDataTemp->m_szBuddyGrp); //�׷� ����
		CString strTmp;
		strTmp.Format("[%s] %s", psDataTemp->m_szUserName, psDataTemp->m_szNickName);

		HTREEITEM hUser = m_TreeCtrl.InsertItem( strTmp, *pTreeItem, TVI_LAST );
				
		TRACEX("psDataTemp->m_szIpAddr [%s]\n", psDataTemp->m_szIpAddr);

		m_TreeCtrl.EnsureVisible( hUser );				
		// �׷캰�� �����鼭 Ʈ���� �����ؾ���.
		m_TreeCtrl.SetItemUserInfo(	hUser, psDataTemp->m_szIpAddr, psDataTemp->m_szBuddyID,
									psDataTemp->m_szNickName, 
									psDataTemp->m_szUserName, 
									psDataTemp->m_szBuddyGrp, psDataTemp->m_bConnStatus, psDataTemp->m_nYouBlocked, 0l);

		m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
		if(psDataTemp->m_bConnStatus)
			m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppLastLeft ); 
		else
			m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft ); 		
	}
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_ADD_BUDDY_RCV *pData = static_cast<S_ADD_BUDDY_RCV*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}
	

	delete pArray;
		
	pMsgThread = new CMsgBoxThread(this, "��ȭ ��� �߰��� ���� �Ͽ����ϴ�." );	
	pMsgThread->CreateThread();			

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnSomeOneAddMe(WPARAM wParam, LPARAM lParam)
{	
	CMsgBoxThread* pMsgThread = NULL;
	CPtrArray* pArray  = (CPtrArray*)lParam;

	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnSomeOneAddMe"); 

	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_ADD_BUDDY_RCV *pData = static_cast<S_ADD_BUDDY_RCV*>(pArray->GetAt(i));
				
		// MAP �� �߰�
		CBuddyInfo*	psDataTemp = new CBuddyInfo;
		strncpy(psDataTemp->m_szIpAddr , (LPCSTR)pData->m_strBuddyIP , sizeof(psDataTemp->m_szIpAddr) );					
		strncpy(psDataTemp->m_szUserName , (LPCSTR)pData->m_strBuddyName, sizeof(psDataTemp->m_szUserName) );					
		strncpy(psDataTemp->m_szNickName , (LPCSTR)pData->m_strBuddyNick, sizeof(psDataTemp->m_szNickName) );					
		strncpy(psDataTemp->m_szBuddyID , (LPCSTR)pData->m_strBuddyID, sizeof(psDataTemp->m_szBuddyID) );					
		strncpy(psDataTemp->m_szBuddyGrp , (LPCSTR)pData->m_strBuddyGrp, sizeof(psDataTemp->m_szBuddyGrp) );					
		
		//KIND ģ�� ���� 
		// 1 - ���� ����� ���
		// 2 - ������ ���� �߰��ؼ� ������� ���� ��ȭ��뿡 �߰���
		// 3 - ��ȭ �ź�
		// 4 - ��ȭ��밡 ���� ������
		
		psDataTemp->m_nYouBlocked = pData->m_nBlock;
		psDataTemp->m_bConnStatus = pData->m_bConnStatus;		
		
		CChatSession::Instance().mBuddyInfoMap.SetAt(psDataTemp->m_szBuddyID , psDataTemp);

		// Ʈ���� �߰�
		HTREEITEM* pTreeItem =  InsertGrpNode(psDataTemp->m_szBuddyGrp); //�׷� ����
		CString strTmp;
		strTmp.Format("[%s] %s", psDataTemp->m_szUserName, psDataTemp->m_szNickName);

		HTREEITEM hUser = m_TreeCtrl.InsertItem( strTmp, *pTreeItem, TVI_LAST );
				
		m_TreeCtrl.EnsureVisible( hUser );				

		TRACEX("psDataTemp->m_szIpAddr [%s]\n", psDataTemp->m_szIpAddr);
		
		// �׷캰�� �����鼭 Ʈ���� �����ؾ���.
		m_TreeCtrl.SetItemUserInfo(	hUser, psDataTemp->m_szIpAddr, psDataTemp->m_szBuddyID,
									psDataTemp->m_szNickName, 
									psDataTemp->m_szUserName, 
									psDataTemp->m_szBuddyGrp, psDataTemp->m_bConnStatus, psDataTemp->m_nYouBlocked, 0l);

		m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
		if(psDataTemp->m_bConnStatus)
			m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppLastLeft ); 
		else
			m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft ); 		
	}
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_ADD_BUDDY_RCV *pData = static_cast<S_ADD_BUDDY_RCV*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;	

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnBuddyListRslt(WPARAM wParam, LPARAM lParam)
{		
	CBuddyInfo * pCompanion;
	//POSITION pos;	

	plugware::CAutoLock LockCriticlaSection(CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnBuddyListRslt"); 

	// ��ȭ������ �����ϱ����ؼ�..
	VEC_STR::iterator itV;
	for (itV= CChatSession::Instance().m_vecUserIDOrdered.begin(); itV != CChatSession::Instance().m_vecUserIDOrdered.end(); ++itV) 
	{
		if(CChatSession::Instance().mBuddyInfoMap.Lookup((*itV).c_str(), ( CObject*& ) pCompanion ))
		{		
			if(pCompanion)
			{
				//1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
				if(pCompanion && pCompanion->m_nYouBlocked != 3 && pCompanion && pCompanion->m_nYouBlocked != 5)
				{			
					pCompanion = static_cast<CBuddyInfo*>(pCompanion);
									
					HTREEITEM* pTreeItem =  InsertGrpNode(pCompanion->m_szBuddyGrp); //�׷� ����

					CString strTmp;
					if(pCompanion->m_nYouBlocked == 9)
					{
						strTmp.Format("%s", pCompanion->m_szNickName);
					}
					else
					{
						strTmp.Format("[%s] %s", pCompanion->m_szUserName, pCompanion->m_szNickName);						
					}
					

					HTREEITEM hUser = m_TreeCtrl.InsertItem( strTmp, *pTreeItem, TVI_LAST );
					
					m_TreeCtrl.EnsureVisible( hUser );
					
					TRACE("pCompanion->m_bConnStatus[%d] pCompanion->m_nYouBlocked [%d]\n", pCompanion->m_bConnStatus , pCompanion->m_nYouBlocked );

					// �׷캰�� �����鼭 Ʈ���� �����ؾ���.
					m_TreeCtrl.SetItemUserInfo(	hUser, pCompanion->m_szIpAddr, pCompanion->m_szBuddyID,
												pCompanion->m_szNickName, 
												pCompanion->m_szUserName, 
												pCompanion->m_szBuddyGrp, pCompanion->m_bConnStatus, pCompanion->m_nYouBlocked, 0l);
					
					//ID->NAME MAP
					CChatSession::Instance().mIDToNameMap.SetAt(pCompanion->m_szBuddyID, pCompanion->m_szUserName);

					m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
									
					if(pCompanion->m_nYouBlocked == 4)
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hBlockBmp, ppLastLeft ); 
					}
					else if(pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2) 
					{
						if(pCompanion->m_bConnStatus == TRUE)
						{
							// 0 : Online, 1 : awy , 2 : Busy
							if(pCompanion->m_nAway == 0)
							{
								m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppLastLeft ); 
								
							}
							else if(pCompanion->m_nAway == 1)
							{
								m_TreeCtrl.AddItemBitmap( hUser, m_hAwayBmp, ppLastLeft ); 
							}
							else if(pCompanion->m_nAway == 2)
							{
								m_TreeCtrl.AddItemBitmap( hUser, m_hBusyBmp, ppLastLeft ); 
							}
							else
							{
								m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft ); 
							}
						}
						else
							m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft ); 		    
					}	
					else if(pCompanion->m_nYouBlocked == 9)
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hITComp, ppLastLeft );  // ����ģ�� ������ ����!
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft ); 
					}
				}				
			}		
		}
	}

	m_TreeCtrl.EnsureVisible( m_TreeCtrl.GetRootItem() );	
	
	// ���� �������� -> �̰��� �׷��� ������ ���Ŀ� ó���Ǿ��Ѵ�! 
	// �������� ID �� ���� ������ ������ �׷��� ������ ����!!!!!!! 
	SetTimer(400, 5000, 0); // ���� ��������

	SetFoldedGrp();

	// Alarm ������ ���� �غ�	
	m_bDocOpened = 0;
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
	// "D:\DATA\DevelopSource\CD8NotBK\newMessenger\BinRelease\IdmsMsn.exe"
	// �̷� ���ڿ����� D:\DATA\DevelopSource\CD8NotBK\newMessenger\ ���� ���Ѵ�.	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	TRACE("%s\n" , szDirTmp);		
	strFileName.Format("%s\\memocal.xml", szDirTmp);
	TRACE("%s\n" , strFileName);		
	OpenAlarmDoc(_T(strFileName));
	
	if(m_bDocOpened)
	{
		SetTimer(600, 1000*30, 0); // �˶� ó��
	}

	// To Server Alive Send
	//m_nAliveAckFailed = 0; // init!	
	//KillTimer(500);
	//SetTimer(500, 3000 /*HEARTBEAT_MILSEC*/ , 0); 

	KillTimer(100);
	SetTimer(100, 60000, 0);  // �ڸ����

	return TRUE;
}

bool CIdmsMsnDlg::OpenAlarmDoc(const CString strFilename)
{
	HRESULT hr;
	CWaitCursor wait;

	CoInitialize(NULL);
	
	hr = m_pDoc.CreateInstance(__uuidof(DOMDocument40));
	if( FAILED(hr) )
	{
		if( !RegisterXMLDll() )
		{
			TRACE(_T("Failed to install XML DOM.\r\n"));
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"MSXML 4.0'�� ����ϴµ� ���� �Ͽ����ϴ�");
			pMsgThread->CreateThread();						

			//AfxMessageBox(_T("'MSXML 4.0'�� ����ϴµ� ���� �Ͽ����ϴ�.\n"), MB_ICONERROR);
			return false;
		}
		else
		{
			hr = m_pDoc.CreateInstance(__uuidof(DOMDocument40));
			if( FAILED(hr) )
			{
				TRACE(_T("Failed to instantiate an XML DOM.\r\n"));
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"MSXML 4.0' �ʱ�ȭ ���� �Ͽ����ϴ�");
				pMsgThread->CreateThread();						

				//AfxMessageBox(_T("'MSXML 4.0' �ʱ�ȭ ���� �Ͽ����ϴ�."), MB_ICONERROR);
				return false;
			}
		}
	}

	if( !IsValidFile((LPCTSTR)strFilename) )
		MakeEmptyFile(strFilename);
	
	if( m_pDoc->load((LPCTSTR)strFilename) != VARIANT_TRUE )
	{
		CString sError;
		sError.Format(_T("%s\r\n\r\nLine : %d\r\nCol : %d\r\nreason : %s"),
			(LPCTSTR)"������ ���� ������ �־� �˶� ����� �������� �ʽ��ϴ�.",
			m_pDoc->parseError->Getline(),	m_pDoc->parseError->Getlinepos(), (LPCTSTR)m_pDoc->parseError->Getreason());
		//AfxMessageBox(sError, MB_ICONERROR);
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,sError);
		pMsgThread->CreateThread();						

		return false;
	}
	
	m_bDocOpened = true;

	return true;
}

bool CIdmsMsnDlg::MakeEmptyFile(CString strFilename)
{
	CFile file;
	CFileException e;

	if( !file.Open(strFilename, CFile::modeCreate|CFile::modeWrite, &e) )
		return false;

	CString strData;
	strData = _T("<?xml version=\"1.0\" encoding=\"EUC-KR\" standalone=\"yes\"?>\r\n" \
		         "<!--XML data for Memo Calendar Application-->\r\n" \
				 "<memos version=\"1.0\">\r\n" \
                 "</memos>\r\n");
	file.Write((LPCTSTR)strData, strData.GetLength());
	file.Close();

	return true;
}


int ExCopyFile(CString strFrom, CString strTo) 
{ 
    SHFILEOPSTRUCT shfo;  
    char pszFrom[1024] = {0}; 
    char pszTo[1024] = {0}; 
    WORD wFlag = 0; 
    
	strcpy(pszFrom, (LPCTSTR)strFrom); 
    strcpy(pszTo, (LPCTSTR)strTo); 

    pszFrom[lstrlen(pszFrom) + 1] = 0; 

    shfo.hwnd =  NULL;
    shfo.wFunc = FO_COPY; 
    shfo.pFrom = pszFrom; 
    shfo.pTo = pszTo; 
    shfo.fFlags = 0; 
    shfo.lpszProgressTitle = "File Copy.."; 

    return SHFileOperation(&shfo); 
} 

// ���� ���� ���� �˻�
bool CIdmsMsnDlg::IsValidFile(LPCTSTR lpszFilename)
{
	OFSTRUCT of;
	
	// Validate filename pointer
	if(lpszFilename == NULL || !*lpszFilename){
		return false;
	}

	// open the file for checking existance
	if(::OpenFile(lpszFilename, &of, OF_EXIST) == -1){
		return false;
	}

	return true;
}

bool CIdmsMsnDlg::RegisterXMLDll()
{
	TCHAR szSysPath[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];
	CString strFilename;
		
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
	// "D:\DATA\DevelopSource\CD8NotBK\newMessenger\BinRelease\IdmsMsn.exe"
	// �̷� ���ڿ����� D:\DATA\DevelopSource\CD8NotBK\newMessenger\ ���� ���Ѵ�.	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	TRACE("%s\n" , szDirTmp);		
	CString strTmp;
	strTmp.Format("%s\\msxml4.dll", szDirTmp);
	TRACE("%s\n" , strFileName);	

	// msxml4.dll ���
	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	strFilename = _tcscat(szBuffer, _T("\\msxml4.dll"));
	
	if( !IsValidFile((LPCTSTR)strFilename) ) 
	{
		ExCopyFile(strTmp, strFilename);
	}
	
	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename2 = _tcscat(szBuffer, _T("\\msxml4.inf"));	
	if( !IsValidFile((LPCTSTR)strFilename2) ) 
	{		
		strTmp.Format("%s\\msxml4.inf", szDirTmp);
		ExCopyFile(strTmp, strFilename2);
	
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename3 = _tcscat(szBuffer, _T("\\msxml4a.dll"));	
	if( !IsValidFile((LPCTSTR)strFilename3) ) 
	{		
		strTmp.Format("%s\\msxml4a.dll", szDirTmp);
		ExCopyFile(strTmp, strFilename3);
	
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename4 = _tcscat(szBuffer, _T("\\msxml4r.dll"));	
	if( !IsValidFile((LPCTSTR)strFilename4) ) 
	{		
		strTmp.Format("%s\\msxml4r.dll", szDirTmp);
		ExCopyFile(strTmp, strFilename4);
	
	}
	
	strFilename = _T("/s ") + strFilename;

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.hwnd = this->m_hWnd;
	sei.lpVerb = _T("open");
	sei.lpFile = _T("regsvr32.exe");
	sei.lpParameters = (LPTSTR)(LPCTSTR)strFilename;
	sei.lpDirectory = NULL;
	sei.nShow = SW_HIDE;
	sei.fMask = SEE_MASK_NOCLOSEPROCESS; // WaitForSingleObject �Լ��� ȣ���ϱ� ����

	if( ShellExecuteEx(&sei) == 0 )
	{
		if(sei.hProcess)
			TerminateProcess(sei.hProcess, 0);
		return false;
	}
	else
	{
		WaitForSingleObject(sei.hProcess, INFINITE);
	}
	TerminateProcess(sei.hProcess, 0);

	return true;
}

HTREEITEM* CIdmsMsnDlg::InsertGrpNode( char* szGrpName)
{	
	// ���� �׷��� �����ϴ��� Ȯ���ϰ� ������ ���� 
	// �׷��� �����ϸ� �ű�ٰ� ����
	
	char szIP			[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN];
	char szGroupName	[C_GRP_NAME_LEN];
	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;
	m_hGrp = m_TreeCtrl.GetRootItem();
	CString ItemText;
		
	while (m_hGrp != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;				
		memset(szIP, 0x00, sizeof(szIP));
		memset(szSelItemID, 0x00, sizeof(szSelItemID));
		memset(szNickName, 0x00, sizeof(szNickName));
		memset(szRealName, 0x00, sizeof(szRealName));
		memset(szGroupName, 0x00, sizeof(szGroupName));

		BOOL bWorked = m_TreeCtrl.GetItemUserInfo(m_hGrp, szIP, szSelItemID, szNickName,szRealName,
													szGroupName,bConnStatus,&nBockMode,&n_StatusFlags);		
		if(strcmp(szGrpName, szGroupName)==0)
		{
			// �׷��� �����ϸ� �ű�ٰ� ����
			
			return &m_hGrp;			
		}

		m_hGrp = m_TreeCtrl.GetNextItem(m_hGrp, TVGN_NEXT);	
	}
	
	// Create Grp name

	if(strcmp("�⺻�׷�", szGrpName)==0)
	{
		// szNickName => OPEN : ���� ������ �ǹ�
		m_hGrpDflt = m_TreeCtrl.InsertItem( szGrpName, m_hContactList, TVI_LAST ); 
		m_TreeCtrl.SetItemUserInfo(	m_hGrpDflt, "GRP","GRP","OPEN", "", "�⺻�׷�", FALSE, 1, 0l);	
		m_TreeCtrl.SetItemTextAttributes( m_hGrpDflt, true, false );
		m_TreeCtrl.SetItemColor( m_hGrpDflt, m_ColorGrp );
		m_TreeCtrl.AddItemBitmap( m_hGrpDflt, m_hGrpOpen, ppLastLeft );
		m_TreeCtrl.EnsureVisible( m_hGrpDflt );

		return &m_hGrpDflt;
	}
	else
	{
		// szNickName => OPEN : ���� ������ �ǹ�
		m_hGrp = m_TreeCtrl.InsertItem( szGrpName, m_hContactList, TVI_LAST ); 
		m_TreeCtrl.SetItemUserInfo(	m_hGrp, "GRP", "GRP", "OPEN", "GRP", szGrpName, FALSE, 1, 0l);	
		m_TreeCtrl.SetItemTextAttributes( m_hGrp, true, false );
		m_TreeCtrl.SetItemColor( m_hGrp, m_ColorGrp );	
		m_TreeCtrl.AddItemBitmap( m_hGrp, m_hGrpOpen, ppLastLeft ); // Icon!
		m_TreeCtrl.EnsureVisible( m_hGrp );

		return &m_hGrp;
	}	
}


void CIdmsMsnDlg::setOnOffInTreeList(const char* szID, BOOL bConn, const char* szRecentNickName, const char* szRecentRealName)	
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	CString ItemText;
	
	TRACE("szRecentNickName [%s]\n", szRecentNickName);
	TRACE("szRecentRealName [%s]\n", szRecentRealName);
	//m_hContactList: Root
	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			HTREEITEM hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{			
				memset(szSelItemIP, 0x00, sizeof(szSelItemIP));
				memset(szNickName, 0x00, sizeof(szNickName));
				memset(szRealName, 0x00, sizeof(szRealName));
				memset(szGrpName, 0x00, sizeof(szGrpName));
				
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;

				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP, szSelItemID, szNickName,
														szRealName,szGrpName, bConnStatus,&nBockMode,&n_StatusFlags);
			
				if (bWorked && strcmp( szID, szSelItemID)==0)
				{
					if(bConn == FALSE)
					{
						// OffLine ���·�
						if( strlen(szRecentNickName) > 0 || strlen(szRecentRealName) > 0 )
						{	
							// ���ܴ��� ���¿��� ������ OffLine�̸� ��� ���ܴ��� ���·�
							m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szRecentNickName, szRecentRealName, szGrpName,FALSE, nBockMode, n_StatusFlags);
						}
						else
						{	
							m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, FALSE, nBockMode, n_StatusFlags);
						}
						
						//1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
						/*
						if(nBockMode == 0)
						{							
							TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
							m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);
							m_TreeCtrl.AddItemBitmap( hChildItem, m_hOffLineBmp, ppFirstLeft );						
						}
						else
						*/
						{							
							TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
							m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1);
							m_TreeCtrl.AddItemBitmap( hChildItem, m_hBlockBmp, ppFirstLeft );						
						}

						CRect rectItem;
						m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
						m_TreeCtrl.InvalidateRect(rectItem);						
						
					}else{
						if( strlen(szRecentNickName) > 0 || strlen(szRecentRealName) > 0)
						{	
							m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szRecentNickName, szRecentRealName, szGrpName, TRUE, 0 , n_StatusFlags);
						}
						else
						{
							m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, TRUE, 0 , n_StatusFlags);
						}
						
								
						TreeItemData* pData = m_TreeCtrl.GetCustomItemData( hChildItem );
						m_TreeCtrl.DeleteItemImage(hChildItem, pData->dwNextImageId -1 );							
						m_TreeCtrl.AddItemBitmap( hChildItem, m_hOnLineBmp, ppFirstLeft );
												
						CRect rectItem;
						CString strTmp;
						if( strlen(szRecentNickName) > 0 || strlen(szRecentRealName) > 0)
						{
							strTmp.Format("[%s] %s", szRecentRealName, szRecentNickName);
						}else
						{
							strTmp.Format("[%s] %s", szRealName, szNickName);
						}
						m_TreeCtrl.SetItemText(hChildItem, strTmp); 
						m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
						m_TreeCtrl.InvalidateRect(rectItem);
					}

					break;
				}
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;
				//ItemText = m_TreeCtrl.GetItemText(hNextItem);
			}
		}

		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}	
}

LRESULT CIdmsMsnDlg::OnChatMsg(WPARAM wParam, LPARAM lParam)
{
	BOOL bDllLoad = FALSE;
	HINSTANCE hInstResourceMain;
	if(m_pSec3Dlg)
	{
		hInstResourceMain = AfxGetResourceHandle();
		HMODULE hModule = ::GetModuleHandle(NULL);
		AfxSetResourceHandle(hModule);

		bDllLoad = TRUE;
	}	

	S_CHAT_CLIENT * pChatMsgSrc = (S_CHAT_CLIENT*)(lParam);	

	CSP_Utility<S_CHAT_CLIENT> spTest = pChatMsgSrc;
	
	//CChatMSgDlg* pa;	
	CChatWnd* pa;	
	
	if(mStrChatDlgMap.Lookup(pChatMsgSrc->strIDFrom, ( CObject*& ) pa ))
	{	
		/*
		CChatMSgDlg * pDlg = static_cast<CChatMSgDlg*>(pa);
		if(pDlg)
		{				
			::SendMessage(pDlg->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);
		}
		*/
		CChatWnd * pWnd = static_cast<CChatWnd*>(pa);
		if(pWnd)
		{				
			::SendMessage(pWnd->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);
		}
		
	}else{
								
		CBuddyInfo * pCompanion = NULL;
		
		if(CChatSession::Instance().mBuddyInfoMap.Lookup(pChatMsgSrc->strIDFrom, ( CObject*& ) pCompanion ))
		{
			if( pCompanion->m_bConnStatus == TRUE)
			{
				// ����� ��츸 ��ȭâ ���				
				CChatWnd* pWnd = new CChatWnd(pCompanion , & CChatSession::Instance().mMyInfo, this );		
	
				pWnd->CreateEx(WS_EX_NOPARENTNOTIFY  ,
					AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
					NULL, 
					WS_TILEDWINDOW| WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX, 					
					100, 100, 
					420, 320, 
					NULL, NULL);				
				
				pWnd->SetWindowPos( &wndBottom, 0, 0, 0, 0, 
					SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW );				
				
				mStrChatDlgMap.SetAt( (LPCTSTR)pChatMsgSrc->strIDFrom, pWnd);				

				::SendMessage(pWnd->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);					

				////////////////////////////////////////////////////////////////////
				/*
				CChatMSgDlg* pDlg = new CChatMSgDlg( pCompanion , &mMyInfo);
				mStrChatDlgMap.SetAt( (LPCTSTR)pChatMsgSrc->strIDFrom, pDlg);				
				pDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_CHAT), GetDesktopWindow() );
				CWnd* pWnd = CWnd::FromHandle(pDlg->GetSafeHwnd());				
				pDlg->ShowWindow(SW_SHOW); 
												
				::SendMessage(pDlg->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);					
				*/
				
			}
		}
	}	
	
	if(bDllLoad)
	{
		AfxSetResourceHandle(hInstResourceMain);
	}

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnMChatMsg(WPARAM wParam, LPARAM lParam)
{
	BOOL bDllLoad = FALSE;
	HINSTANCE hInstResourceMain;
	if(m_pSec3Dlg)
	{
		hInstResourceMain = AfxGetResourceHandle();
		HMODULE hModule = ::GetModuleHandle(NULL);
		AfxSetResourceHandle(hModule);

		bDllLoad = TRUE;
	}	

	S_CHAT_CLIENT * pMChatMsgSrc = (S_CHAT_CLIENT*)(lParam);
	CSP_Utility<S_CHAT_CLIENT> spTest = pMChatMsgSrc;

	//CMultiChatMsgDlg * pMCDlg;	
	CMultiChatWnd* pMCWnd = NULL ; 
			
	// pMChatMsgSrc->strIDTo => Server Room No
	if(mStrMultiChatDlgMap.Lookup(pMChatMsgSrc->strIDTo, ( CObject*& ) pMCWnd ) )
	{
		if(pMCWnd)
		{
			::SendMessage(pMCWnd->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);				
		}
	}
	else
	{								
		// ���� ������ ���� 
		CBuddyInfo * pCompanion;
		
		if(CChatSession::Instance().mBuddyInfoMap.Lookup(pMChatMsgSrc->strIDFrom, ( CObject*& ) pCompanion ))
		{
			if( pCompanion)
			{	
				// strIDTo => Server Room NO
				CMultiChatWnd* pWnd = new CMultiChatWnd( NULL, (LPCSTR)pMChatMsgSrc->strIDTo );							
				pWnd->CreateEx(WS_EX_NOPARENTNOTIFY  ,
					AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
					NULL, 
					WS_TILEDWINDOW| WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX, 					
					100, 100, 
					420, 320, 
					NULL, NULL);						
				
				// Wnd Map �� ���� 
				mStrMultiChatDlgMap.SetAt( pMChatMsgSrc->strIDTo, pWnd);
								
				pWnd->SetWindowPos( &wndBottom, 0, 0, 0, 0, 
					SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW );				
				
				::PostMessage(pWnd->GetSafeHwnd(), WM_LBUTTONDOWN , NULL, NULL);

				::SendMessage(pWnd->GetSafeHwnd(), WM_CHATMSG, wParam, lParam);					
			}
		}		
	}

	if(bDllLoad)
	{
		AfxSetResourceHandle(hInstResourceMain);
	}

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnAllowedComp(WPARAM wParam, LPARAM lParam)
{
	// A �� B�� �ٽ� ��ȭ���� �߰��ϴ� ��쿡 A���� ����
	// ������ ��ȭ��븦 �ٽ� �߰��ϴ� ���. �ٽ� ����.
	S_YOU_ALLOWED* pInfo = (S_YOU_ALLOWED*) lParam ;
	CSP_Utility<S_YOU_ALLOWED> spTest = pInfo;

	CBuddyInfo * pCompanion;
	plugware::CAutoLock LockCriticlaSection(CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnAllowedComp"); 
	if( CChatSession::Instance().mBuddyInfoMap.Lookup(pInfo->m_szYouAllowedID, ( CObject*& ) pCompanion ) ) 
	{	
		pCompanion->m_nYouBlocked = 1 ;
		CChatSession::Instance().mBuddyInfoMap.SetAt(pInfo->m_szYouAllowedID, pCompanion) ;
		
		// Ʈ�� ȭ�鿡 �ٽ� ǥ����
	
		// Tree�� �߰�
		char szTmp[50+1+100+1];
		memset(szTmp,0x00, sizeof(szTmp));
		sprintf(szTmp ,"[%s] %s", pCompanion->m_szUserName,pCompanion->m_szNickName);

		// �׷��� ã�Ƽ� Insert
				
		char szSelItemIP	[15 + 1];	
		char szSelItemID    [20 + 1];
		char szRealName		[50 + 1];
		char szNickName		[C_NICK_NAME_LEN +1];
		char szGrpName      [C_GRP_NAME_LEN +1];
		BOOL bConnStatus = FALSE;
		int nBockMode = 0;	
		long n_StatusFlags = 0l;

		HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
		CString ItemText;
					
		while (hCurrent != NULL)
		{
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;

			BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP, szSelItemID, szNickName,szRealName, 
									szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);	

			if(strcmp(szGrpName, pInfo->m_szGrpName)==0)
			{			
				HTREEITEM hUser = m_TreeCtrl.InsertItem( szTmp, hCurrent, TVI_LAST );
				
				strncpy(pCompanion->m_szBuddyGrp, szGrpName, sizeof(pCompanion->m_szBuddyGrp));
				
				CChatSession::Instance().mBuddyInfoMap.SetAt( pInfo->m_szYouAllowedID, pCompanion);  // �׷��� �ٲ���������Ƿ� �ٽ� set

				TRACEX("pCompanion->m_szIpAddr [%s]\n", pCompanion->m_szIpAddr);
					
				m_TreeCtrl.SetItemUserInfo(	hUser,	pCompanion->m_szIpAddr, pCompanion->m_szBuddyID, pCompanion->m_szNickName,
											pCompanion->m_szUserName, pCompanion->m_szBuddyGrp, pCompanion->m_bConnStatus, 1, n_StatusFlags);
				
				m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
				
				if(pCompanion->m_bConnStatus == TRUE)
				{						
					if ( n_StatusFlags & CFL_AWAY)
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hAwayBmp, ppLastLeft );
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppLastLeft );			
					}
				}
				else
				{
					m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft );
				}
				
				m_TreeCtrl.Expand(hCurrent,TVE_EXPAND );		
				
				break;
			}
			
			hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
		}
	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnYouAllowedBy(WPARAM wParam, LPARAM lParam)
{
	// A �� B�� �ٽ� ��ȭ���� �߰��ϴ� ��쿡 B ���� �ٽ� ���Ǿ����� �˸�
	// icon ���������� ����!
	S_YOU_ALLOWED_BY * pInfo = (S_YOU_ALLOWED_BY*) lParam;
	CSP_Utility<S_YOU_ALLOWED_BY> spTest = pInfo;

	CBuddyInfo * pCompanion;
	plugware::CAutoLock LockCriticlaSection(CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnYouAllowedBy"); 
	if( CChatSession::Instance().mBuddyInfoMap.Lookup(pInfo->m_szYouAllowedByID, ( CObject*& ) pCompanion ) ) 
	{	
		if(pCompanion->m_nYouBlocked == 5)
		{
			// A �� B������, B�� A���� ���� Ʈ���� ����. �߰��ؾ���.
			///////////////////////////////////////////////////////////////////////////////////
			// Tree�� �߰�
			char szTmp[50+1+100+1];
			memset(szTmp,0x00, sizeof(szTmp));
			sprintf(szTmp ,"[%s] %s", pCompanion->m_szUserName,pCompanion->m_szNickName);
			
			// �׷��� ã�Ƽ� Insert			
			char szSelItemIP	[15 + 1];	
			char szSelItemID    [20 + 1];
			char szRealName		[50 + 1];
			char szNickName		[C_NICK_NAME_LEN +1];
			char szGrpName      [C_GRP_NAME_LEN +1];
			BOOL bConnStatus = FALSE;
			int nBockMode = 0;	
			long n_StatusFlags = 0l;
			
			HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
			CString ItemText;
			
			while (hCurrent != NULL)
			{
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
				
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP, szSelItemID, szNickName,szRealName, 
					szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);	
				
				if(strcmp(szGrpName, pCompanion->m_szBuddyGrp)==0)
				{			
					HTREEITEM hUser = m_TreeCtrl.InsertItem( szTmp, hCurrent, TVI_LAST );
					
					strncpy(pCompanion->m_szBuddyGrp, szGrpName, sizeof(pCompanion->m_szBuddyGrp));
					
					pCompanion->m_nYouBlocked = 2;
					CChatSession::Instance().mBuddyInfoMap.SetAt( pCompanion->m_szBuddyID, pCompanion);  
					
					TRACEX("pCompanion->m_szIpAddr [%s]\n", pCompanion->m_szIpAddr);
					
					m_TreeCtrl.SetItemUserInfo(	hUser,	pCompanion->m_szIpAddr, pCompanion->m_szBuddyID, pCompanion->m_szNickName,
						pCompanion->m_szUserName, pCompanion->m_szBuddyGrp, pCompanion->m_bConnStatus, 1, n_StatusFlags);
					
					m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
					
					if(pCompanion->m_bConnStatus == TRUE)
					{						
						if ( n_StatusFlags & CFL_AWAY)
						{
							m_TreeCtrl.AddItemBitmap( hUser, m_hAwayBmp, ppLastLeft );
						}
						else
						{
							m_TreeCtrl.AddItemBitmap( hUser, m_hOnLineBmp, ppLastLeft );			
						}
					}
					else
					{
						m_TreeCtrl.AddItemBitmap( hUser, m_hOffLineBmp, ppLastLeft );
					}
					
					m_TreeCtrl.Expand(hCurrent,TVE_EXPAND );		
					
					break;
				}
				
				hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
			}
			///////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			// 
			pCompanion->m_nYouBlocked = 2 ;
			CChatSession::Instance().mBuddyInfoMap.SetAt(pInfo->m_szYouAllowedByID, pCompanion) ;

			setOnOffInTreeList(pInfo->m_szYouAllowedByID, TRUE, "", "");
		}		
	}
	
	return TRUE;
}

// ���� ���� ����
// �޽������� ù 20����Ʈ�� �̸��� ���´�!
LRESULT CIdmsMsnDlg::OnAllMsg(WPARAM wParam, LPARAM lParam)
{	
	BOOL bDllLoad = FALSE;
	HINSTANCE hInstResourceMain;
	if(m_pSec3Dlg)
	{
		hInstResourceMain = AfxGetResourceHandle();
		HMODULE hModule = ::GetModuleHandle(NULL);
		AfxSetResourceHandle(hModule);

		bDllLoad = TRUE;
	}	

	S_CHAT_CLIENT * pChatMsgSrc = (S_CHAT_CLIENT*)(lParam);	
	
	S_ALLMSG_CLIENT * pAllMsg = new S_ALLMSG_CLIENT();
			
	pAllMsg->strIDFrom =	pChatMsgSrc->strIDFrom;

	string strOutMsg, strOutName;
	
	getDelimitData((LPCSTR)pChatMsgSrc->strMsg, DELIM2, strOutMsg, 0); //�޽��� 
	getDelimitData((LPCSTR)pChatMsgSrc->strMsg, DELIM2, strOutName, 1); //�޽��� 
	
	pAllMsg->strNameFrom = strOutName.c_str();
	pAllMsg->strMsg =	strOutMsg.c_str();
	
	if(m_pAllMSgViewDlg)			
	{			
		CWnd* pWnd = CWnd::FromHandle(m_pAllMSgViewDlg->GetSafeHwnd());		
		::SendMessage(pWnd->m_hWnd , WM_ALLMSG, wParam, (LPARAM)pAllMsg);		
		//pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
	}
	else
	{		
		if(!m_pAllMSgViewDlg)
		{
			m_pAllMSgViewDlg = new CViewAllMsgDlg();

			TRACE("[%d]\n", sizeof(m_pAllMSgViewDlg) );

			m_pAllMSgViewDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_VIEW_ALLMSG), GetDesktopWindow() );
			CWnd* pWnd = CWnd::FromHandle(m_pAllMSgViewDlg->GetSafeHwnd());
			m_pAllMSgViewDlg->ShowWindow(SW_SHOW);
			
			// ��޸��� Ȱ��ȭ!
			//::PostMessage(m_pAllMSgViewDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);				
			//m_pAllMSgViewDlg->SetForegroundWindow();
			//m_pAllMSgViewDlg->SetActiveWindow();
			::SendMessage(m_pAllMSgViewDlg->GetSafeHwnd(), WM_ALLMSG, wParam, (LPARAM)pAllMsg);				
		}	
	}
			
	// ���������� ���������� �˸�.
	if(pAllMsg->strNameFrom.GetLength() > 0)
	{
		char* pszID = new char [100];
		memset(pszID, 0x00, 100);
		strncpy(pszID, (LPCSTR)pChatMsgSrc->strIDFrom, 100);

		PostMessage(WM_SENDALLMSGACKED, (WPARAM) pszID , (LPARAM) NULL );		
	}

	delete pChatMsgSrc;
	delete pAllMsg;	

	if(bDllLoad)
	{
		AfxSetResourceHandle(hInstResourceMain);
	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAllMsgDlgClose(WPARAM wParam, LPARAM lParam)
{	
	delete m_pAllMSgSendDlg;
	m_pAllMSgSendDlg = NULL;

	return TRUE;
}




LRESULT CIdmsMsnDlg::OnChatDlgClose(WPARAM wParam, LPARAM lParam)
{	
	CString strID = (char*)wParam;
	TRACE("OnChatDlgClose [%s]", strID);
	CChatWnd* pa;	
	
	if(mStrChatDlgMap.Lookup(strID, ( CObject*& ) pa ))
	{
		CChatWnd * pDlg = static_cast<CChatWnd*>(pa);
		if(pDlg)
		{
			delete pDlg;
			mStrChatDlgMap.RemoveKey(strID);
			pDlg = NULL;
		}
	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnSlipMsgDlgClose(WPARAM wParam, LPARAM lParam)
{	
	CString strKey = (char*)wParam;
	TRACE("OnSlipMsgDlgClose [%s]", strKey);
	CViewBuddySlipMsg* pa;	
	
	if(mStrSlipMsgDlgMap.Lookup(strKey, ( CObject*& ) pa ))
	{
		CViewBuddySlipMsg * pDlg = static_cast<CViewBuddySlipMsg*>(pa);
		if(pDlg)
		{
			delete pDlg;
			mStrSlipMsgDlgMap.RemoveKey(strKey);
			pDlg = NULL;
		}
	}

	return TRUE;
}



void CIdmsMsnDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CRect rcMainRect;		
	GetWindowRect(&rcMainRect);
	
	CString strPosMain;
	
	int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);

	if(  rcMainRect.left >=0 && rcMainRect.top >= 0 && rcMainRect.Width() <= nWidth && rcMainRect.Height() <= nHeight )
	{
		strPosMain.Format("%ld|%ld|%d|%d|", rcMainRect.left, rcMainRect.top, rcMainRect.Width(), rcMainRect.Height());

		CINIManager::Instance().SetValue(INI_PREV_POS_SIZE_MAIN, (LPSTR)(LPCSTR)strPosMain);	
		CINIManager::Instance().SaveINIFile();
	}

	TrayIcon(TRUE);
}

void CIdmsMsnDlg::doTerminateWork()
{
	//File Send 
	CFileRecvDlg * pFileRecvDlg;
	POSITION posFileRecv;
	CString keyFileRecv;
	for( posFileRecv = CFileTransSock::Instance().mStrFileRecvDlgMap.GetStartPosition(); posFileRecv != NULL; ){		
		if(posFileRecv)
		{
			CFileTransSock::Instance().mStrFileRecvDlgMap.GetNextAssoc( posFileRecv, keyFileRecv, (CObject*&)pFileRecvDlg );			
			if(pFileRecvDlg)
			{				
				delete pFileRecvDlg;
				pFileRecvDlg = NULL ;
			}
		}		
	}
	CFileTransSock::Instance().mStrFileRecvDlgMap.RemoveAll();
	

	CFileSendDlg * pFileSendDlg;
	POSITION posFileSend;
	CString keyFileSend;
	for( posFileSend = CFileTransSock::Instance().mStrFileSendDlgMap.GetStartPosition(); posFileSend != NULL; ){		
		if(posFileSend)
		{
			CFileTransSock::Instance().mStrFileSendDlgMap.GetNextAssoc( posFileSend, keyFileSend, (CObject*&)pFileSendDlg );			
			if(pFileSendDlg)
			{				
				delete pFileSendDlg;
				pFileSendDlg = NULL ;
			}
		}		
	}
	CFileTransSock::Instance().mStrFileSendDlgMap.RemoveAll();


	if(m_pSec2Dlg && 0xcccccccc != (DWORD)m_pSec2Dlg)
	{
		delete m_pSec2Dlg;
		m_pSec2Dlg = NULL;
	}
	if(m_pSec3Dlg && 0xcccccccc != (DWORD)m_pSec2Dlg)
	{
		delete m_pSec3Dlg;
		m_pSec3Dlg = NULL;
	}

	if(m_pAllMSgViewDlg)
	{		
		delete m_pAllMSgViewDlg;
		m_pAllMSgViewDlg = NULL;
	}
	
	if(m_pAllMSgSendDlg)
	{		
		m_pAllMSgSendDlg->ClearAllTreeData() ;
		
		delete m_pAllMSgSendDlg;
		m_pAllMSgSendDlg = NULL;
	}
	
	
	POSITION pos;
	CString key;
	
	
	//ä��
	CChatWnd * pa1;
	
	for( pos = mStrChatDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pa1 );			
			if(pa1){				
				delete pa1;
			}
		}		
	}
	mStrChatDlgMap.RemoveAll();		
	
	// Multi Chat	
	CMultiChatWnd* pa2;	
	for( pos = mStrMultiChatDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrMultiChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pa2 );			
			if(pa2)
			{				
				delete pa2;
			}
		}		
	}
	mStrMultiChatDlgMap.RemoveAll();		
		

	// Slip
	CViewBuddySlipMsg* pa3;	
	for( pos = mStrSlipMsgDlgMap.GetStartPosition(); pos != NULL; ){		
		if(pos){
			mStrSlipMsgDlgMap.GetNextAssoc( pos, key, (CObject*&)pa3 );			
			if(pa3)
			{				
				delete pa3;
			}
		}		
	}
	mStrSlipMsgDlgMap.RemoveAll();		

	plugware::CAutoLock LockCriticlaSection(CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"PostNcDestroy"); 
	
	CBuddyInfo * paComp;	
	for( pos =  CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); pos != NULL; ){		
		if(pos)
		{
			CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( pos, key, (CObject*&)paComp );			
			if(paComp)
			{				
				delete paComp;
				paComp = NULL;
			}
		}		
	}
	CChatSession::Instance().mBuddyInfoMap.RemoveAll();

	CChatSession::Instance().Stop(TRUE);

	CINIManager::Instance().SaveINIFile();

	// Hooking ����	
	if(m_bHook)
	{
        BOOL bRtn = Dormant::EndUp(); 
		
		if(!bRtn)
		{
			//AfxMessageBox("Hooking ���� ����");
		}
	}

	WSACleanup();

}

void CIdmsMsnDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class

	//WINDOWPLACEMENT windowPlacement;
	//GetWindowPlacement(&windowPlacement);
	//if (windowPlacement.showCmd != SW_SHOWMINIMIZED)
	//	AfxGetApp()->WriteProfileBinary("Settings", "WindowPos", (BYTE*)&windowPlacement, sizeof(WINDOWPLACEMENT));	

	//GdiplusShutdown(gpToken); //GDI+
	
	doTerminateWork();
	
	

	//Log	
//	CTime iDT =  CTime::GetCurrentTime();					
//	char szTmpPath[100];
//	memset(szTmpPath, 0x00, sizeof(szTmpPath));
//	sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//	writeLogFile(szTmpPath, "���α׷� ����� [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

	CDialog::PostNcDestroy();
}

void CIdmsMsnDlg::OnSize(UINT nType, int cx, int cy) 
{		
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	m_cControlPos.MoveControls();	
	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	
	RedrawWindow();	
}


void CIdmsMsnDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize = CPoint( 353, 490 );
}

void CIdmsMsnDlg::OnItemexpandedTreectrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if( pNMTreeView->itemNew.hItem == m_hEternalItem && pNMTreeView->action == TVE_EXPAND )
	{
		HTREEITEM hChild = m_TreeCtrl.GetChildItem( m_hEternalItem );
		m_TreeCtrl.SetItemColor( m_TreeCtrl.InsertItem( "Eternal Item", hChild, TVI_LAST ), 
			RGB( 255, 0, 0 ) );
		m_hEternalItem = hChild;
	}

	*pResult = 0;
	
}

void CIdmsMsnDlg::OnDblclkTreectrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	HTREEITEM hSel = m_TreeCtrl.GetFirstSelectedItem();	
	
	TreeItemData* pTreeItemDta = m_TreeCtrl.GetCustomItemData(hSel);
		
	if (hSel != NULL)
	{		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName,szRealName, 
								szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);		

		
		TRACE("szSelItemIP [%s]\n", szSelItemIP); 
		TRACE("szNickName  [%s]\n", szNickName); 
		TRACE("szRealName  [%s]\n", szRealName); 
		TRACE("bConnStatus  [%d]\n", bConnStatus); 
		
		if (m_TreeCtrl.ItemHasChildren(hSel))
		{
			TRACE("OnDblclkTreectrl => m_TreeCtrl.ItemHasChildren TRUE\n");

			if( strcmp("OPEN" , szNickName)==0)
			{
				// ���� �׷� ������ Ŭ���� ���� ���·� 
				m_TreeCtrl.SetItemUserInfo(hSel, szSelItemIP,szSelItemID,"CLOSE",szRealName, 
								szGrpName, bConnStatus, nBockMode, n_StatusFlags);		

				m_TreeCtrl.DeleteItemImage(hSel, pTreeItemDta->dwNextImageId -1);
				m_TreeCtrl.AddItemBitmap( hSel, m_hGrpClose, ppFirstLeft );

				CRect rectItem;
				m_TreeCtrl.GetItemRect(hSel, &rectItem,FALSE);
				m_TreeCtrl.InvalidateRect(rectItem);				

			}
			else if( strcmp("CLOSE" , szNickName)==0)
			{
				// �����׷� ������ Ŭ���� �������·� 
				m_TreeCtrl.SetItemUserInfo(hSel, szSelItemIP, szSelItemID, "OPEN",szRealName, 
								szGrpName, bConnStatus, nBockMode, n_StatusFlags);		

				m_TreeCtrl.DeleteItemImage(hSel, pTreeItemDta->dwNextImageId -1);
				m_TreeCtrl.AddItemBitmap( hSel, m_hGrpOpen, ppFirstLeft );

				CRect rectItem;
				m_TreeCtrl.GetItemRect(hSel, &rectItem,FALSE);
				m_TreeCtrl.InvalidateRect(rectItem);				

			}
		}
		else
		{
			//CWinApp *pApp = AfxGetApp();
			//CWnd *pMainWnd = pApp->GetMainWnd();			

			// ������ ����â���� üũ IP
			//CChatMSgDlg* pa;	
			CChatWnd* pa;	

			CString strUrl("");

			if(mStrChatDlgMap.Lookup(szSelItemID, ( CObject*& ) pa ))
			{	
				//CChatMSgDlg * pDlg = static_cast<CChatMSgDlg*>(pa);
				CChatWnd * pWnd = static_cast<CChatWnd*>(pa);				
				
				if(pWnd)
				{
					strcpy(pWnd->m_pCompanion->m_szIpAddr , szSelItemIP);
					pWnd->ShowWindow(SW_SHOW);
					pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
					pWnd->SetForegroundWindow();
					pWnd->SetActiveWindow();
					::PostMessage(pWnd->GetSafeHwnd(), WM_LBUTTONDOWN , NULL, NULL);

					/*
					strcpy(pDlg->m_pCompanion->m_szIpAddr , szSelItemIP);
					CWnd* pWnd = CWnd::FromHandle(pDlg->GetSafeHwnd());
					pWnd->ShowWindow(SW_SHOW);
					::PostMessage(pDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);
					pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
					pWnd->SetForegroundWindow();
					pWnd->SetActiveWindow();
					*/
				}
			}else{
								
				CBuddyInfo * pCompanion;
				if(CChatSession::Instance().mBuddyInfoMap.Lookup(szSelItemID, ( CObject*& ) pCompanion ))
				{
					if( pCompanion->m_bConnStatus == TRUE)					
					{
						/*			
						KIND ģ�� ���� 
						* 1 - ���� ����� ���
						* 2 - ������ ���� �߰��ؼ� ������� ���� ��ȭ��뿡 �߰���
						* 3 - ��ȭ �ź�
						* 4 - ��ȭ��밡 ���� ������
						  5 - ������ ���� �����ϰ� ���� ������ ������
						*/
						// 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
						if( pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2)
						{
							// ����ǰ� ���ܵ� ��� �ƴҶ��� ��ȭâ ���	
							CChatWnd* pWnd = new CChatWnd(pCompanion , &CChatSession::Instance().mMyInfo, this );			
							pWnd->CreateEx(WS_EX_NOPARENTNOTIFY  ,
								AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
								NULL, 
								WS_TILEDWINDOW| WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX, 					
								100, 100, 
								420, 320, 
								NULL, NULL);		
							
							strcpy(pWnd->m_pCompanion->m_szIpAddr , szSelItemIP);	
							
							pWnd->SetWindowPos( &wndTop, 0, 0, 0, 0, 
								SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW );				
							
							::PostMessage(pWnd->GetSafeHwnd(), WM_LBUTTONDOWN , NULL, NULL);
							
							mStrChatDlgMap.SetAt( szSelItemID, pWnd);							
							
							/*
							CChatMSgDlg* pDlg = new CChatMSgDlg( pCompanion , &mMyInfo);						
							strcpy(pDlg->m_pCompanion->m_szIpAddr , szSelItemIP);
							mStrChatDlgMap.SetAt( szSelItemID, pDlg);
							pDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_CHAT), GetDesktopWindow() );
							//strcpy(pDlg->m_szIP, m_szMyIPAddr);							
							CWnd* pWnd = CWnd::FromHandle(pDlg->GetSafeHwnd());
							pDlg->ShowWindow(SW_SHOW);
							// ��޸��� Ȱ��ȭ!
							::PostMessage(pDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);
							pDlg->SetForegroundWindow();
							pDlg->SetActiveWindow();
							*/
							
						}
						else if( pCompanion->m_nYouBlocked == 4)
						{							
							CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ����� ��ȭ��뿡�� �����Ͽ����ϴ�. �޽����� ������ �����ϴ�.");
							
							pMsgThread->CreateThread();						
						}
					}
					else
					{						
						// 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)

						if( pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2)
						{
							char szMsg[100];
							memset(szMsg, 0x00, sizeof(szMsg));
							sprintf(szMsg, "OffLine ��ȭ��� [%s]���� \r\n�޽����� ������ �����ϴ�\r\n���� ����� ����� �̿��ϼ���.", pCompanion->m_szBuddyID );
														
							CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);							
							
							pMsgThread->CreateThread();						
						}
						else if( pCompanion->m_nYouBlocked == 4)
						{							
							CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ����� ��ȭ��뿡�� �����Ͽ����ϴ�. �޽����� ������ �����ϴ�.");							
							
							pMsgThread->CreateThread();						
						}
					}
				}
				////////
				else if(CChatSession::Instance().mITCompDescToUrlMap.Lookup(szNickName, strUrl))
				{
					TRACEX("strUrl [%s]", strUrl);
					ShellExecute(NULL, "open", "iexplore.exe", strUrl, NULL, SW_SHOWNORMAL);		
				}
				///////
			}			
		}
	}

	//TRACE("LockCriticlaSection # 10 E\n");
	SaveFoldedGrp();
	*pResult = 0;
}

void CIdmsMsnDlg::AniMinimizeToTray(HWND hwnd)
{
    RECT rectTo, rectFrom;
    ::GetWindowRect(hwnd, &rectFrom);
    GetTrayWndRect(&rectTo);
	
    DrawAnimatedRects(hwnd, IDANI_CAPTION, &rectFrom, &rectTo);
	//DrawAnimatedRects(IDANI_CAPTION, &rectFrom, &rectTo);
}


void CIdmsMsnDlg::AniMaximiseFromTray(HWND hwnd)
{
    RECT rectFrom;
    GetTrayWndRect(&rectFrom);
    WINDOWPLACEMENT wndpl;
    ::GetWindowPlacement(hwnd, &wndpl);    
    
    DrawAnimatedRects(hwnd, IDANI_CAPTION, &rectFrom, &wndpl.rcNormalPosition);
	//DrawAnimatedRects(IDANI_CAPTION, &rectFrom, &wndpl.rcNormalPosition);
}


void CIdmsMsnDlg::GetTrayWndRect(RECT *pRect)
{
    HWND hwndTaskBar=::FindWindow(_T("Shell_TrayWnd"), NULL);
    if (hwndTaskBar){
        HWND hwndTray=::FindWindowEx(hwndTaskBar, NULL, _T("TrayNotifyWnd"), NULL);
        if (hwndTray)
            ::GetWindowRect(hwndTray, pRect);
        else
		{            
            ::GetWindowRect(hwndTaskBar, pRect);
            pRect->left=pRect->right-20;
            pRect->top=pRect->bottom-20;
        }
    }
    else{
        
        int nWidth = GetSystemMetrics(SM_CXSCREEN);
        int nHeight = GetSystemMetrics(SM_CYSCREEN);
        SetRect(pRect, nWidth-40, nHeight-20, nWidth, nHeight);
    }
}


void CIdmsMsnDlg::Restore()
{	
	if( IsIconic() )
		ShowWindow(SW_RESTORE);
	else if( IsWindowVisible() == FALSE )
	{
		AniMaximiseFromTray(m_hWnd);
		ShowWindow(SW_SHOW);
	}

	SetForegroundWindow();

	// Ʈ���̾����� ����
	TrayIcon(FALSE);
}

void CIdmsMsnDlg::TrayIcon(BOOL add)
{
	if( add )
	{		
		AniMinimizeToTray(m_hWnd);
		
		ShowWindow(SW_HIDE);
	}
	
	
	HICON Icon;
	Icon = (HICON) AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	NOTIFYICONDATA nid;
	CString pcstr;
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hIcon =	Icon;
	nid.hWnd = AfxGetMainWnd()->GetSafeHwnd();
	nid.uCallbackMessage = WM_TRAY;
	nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nid.uID = 1;
	AfxGetMainWnd()->GetWindowText(pcstr);
	_tcscpy(nid.szTip, pcstr);
	//::Shell_NotifyIcon(add ? NIM_ADD : NIM_DELETE, &nid);
	::Shell_NotifyIcon(NIM_ADD , &nid);
	
}

LRESULT CIdmsMsnDlg::OnTrayShow(WPARAM wParam, LPARAM lParam) 
{
    TrayIcon(TRUE);

	return 1;
} 

LRESULT CIdmsMsnDlg::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
	
	UINT msg = (UINT)lParam;
	switch (msg)
	{
	case WM_LBUTTONDBLCLK:
		Restore();
		break;
	case WM_RBUTTONUP:
		CPoint pos;
		CMenu theMenu;
		CWnd* pTarget = AfxGetMainWnd();
		CMenu menu;
		theMenu.LoadMenu(IDR_TRAY_MENU);
		CMenu *subMenu = (CMenu*)theMenu.GetSubMenu(0);
		GetCursorPos(&pos);
		menu.LoadMenu(IDR_TRAY_MENU);
		::SetMenuDefaultItem(menu, 0, TRUE);
		pTarget->SetForegroundWindow(); 
		::TrackPopupMenu(subMenu->GetSafeHmenu(), 0, pos.x, pos.y, 0, 
		pTarget->GetSafeHwnd(), NULL);
		pTarget->PostMessage(WM_NULL, 0, 0);
		break;
	}
	

	return 1;
}

void CIdmsMsnDlg::OnCloseMsger() 
{
	// TODO: Add your command handler code here
	TrayIcon(FALSE);

	KillTimer(10);
	KillTimer(100);
	KillTimer(500); // Alive
	KillTimer(600); //Alarm 
	
	if(CChatSession::Instance().m_bDailyJob == TRUE)
	{
		// Log Off �ð� ���� => ���� ���� �ȹ���!
		CTime ctToday = CTime::GetCurrentTime();		

		char szSql [200];
		memset(szSql , 0x00, sizeof(szSql));
		sprintf(szSql, "UPDATE KM_DAILYJOBMST SET LOGOFF_TIME=SYSDATE WHERE USER_ID='%s' AND JOB_DAY='%s'", (LPCSTR)m_strLoginID, (LPCSTR) (ctToday.Format("%Y%m%d"))  ) ;

		if(CChatSession::Instance().RequestRawSQL(RAWSQL_UPDATE_LOGOFFTIME, szSql, strlen(szSql) ) != TRUE)
		{
			TRACE("Log Off �ð� ���� ����!\n");
		}
	}

	CRect rcMainRect;		
	GetWindowRect(&rcMainRect);
	
	int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);

	if(  rcMainRect.left >=0 && rcMainRect.top >= 0 && rcMainRect.Width() <= nWidth && rcMainRect.Height() <= nHeight )
	{
		CString strPosMain;
		strPosMain.Format("%ld|%ld|%d|%d|", rcMainRect.left, rcMainRect.top, rcMainRect.Width(), rcMainRect.Height());

		CINIManager::Instance().SetValue(INI_PREV_POS_SIZE_MAIN, (LPSTR)(LPCSTR)strPosMain);	
		CINIManager::Instance().SaveINIFile();
	}
	
	CFileTransSock::Instance().Stop(TRUE);

	CDialog::OnOK();		
}

// �޴����� ��ȭ��� �߰�
void CIdmsMsnDlg::OnAddpartner() 
{
	// TODO: Add your command handler code here	

	CNewCompanionDlg dlg(NULL, "");
	dlg.SetParentHandle(this->GetSafeHwnd());
	dlg.DoModal();	
}

void CIdmsMsnDlg::OnClickTreectrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	HTREEITEM hSel = m_TreeCtrl.GetFirstSelectedItem();	
	
	TreeItemData* pTreeItemDta = m_TreeCtrl.GetCustomItemData(hSel);
	
	if (hSel != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName,szRealName, 
								szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
		
		TRACE("szSelItemIP [%s]\n", szSelItemIP); 
		TRACE("szNickName  [%s]\n", szNickName); 
		TRACE("szRealName  [%s]\n", szRealName); 
		TRACE("bConnStatus  [%d]\n", bConnStatus); 
		
		if (m_TreeCtrl.ItemHasChildren(hSel))
		{
			TRACE("OnClickTreectrl => m_TreeCtrl.ItemHasChildren TRUE\n");

			if( strcmp("OPEN" , szNickName)==0)
			{
				// ���� �׷� ������ Ŭ���� ���� ���·� 
				m_TreeCtrl.SetItemUserInfo(hSel, szSelItemIP, szSelItemID, "CLOSE",szRealName, 
								szGrpName, bConnStatus, nBockMode, n_StatusFlags);		

				m_TreeCtrl.DeleteItemImage(hSel, pTreeItemDta->dwNextImageId -1);
				m_TreeCtrl.AddItemBitmap( hSel, m_hGrpClose, ppFirstLeft );

				CRect rectItem;
				m_TreeCtrl.GetItemRect(hSel, &rectItem,FALSE);
				m_TreeCtrl.InvalidateRect(rectItem);				

			}
			else if( strcmp("CLOSE" , szNickName)==0)
			{
				// �����׷� ������ Ŭ���� �������·� 
				m_TreeCtrl.SetItemUserInfo(hSel, szSelItemIP, szSelItemID, "OPEN",szRealName, 
								szGrpName, bConnStatus, nBockMode, n_StatusFlags);		

				m_TreeCtrl.DeleteItemImage(hSel, pTreeItemDta->dwNextImageId -1);
				m_TreeCtrl.AddItemBitmap( hSel, m_hGrpOpen, ppFirstLeft );

				CRect rectItem;
				m_TreeCtrl.GetItemRect(hSel, &rectItem,FALSE);
				m_TreeCtrl.InvalidateRect(rectItem);				

			}
			else
			{
				TRACE("Some Error\n");
			}
		}
		else
		{
			// ���ã��� ��Ŭ������ ����	
			
			if(pTreeItemDta && pTreeItemDta->m_nBockMode == 9)
			{
				CString strUrl="";
				if(CChatSession::Instance().mITCompDescToUrlMap.Lookup(szNickName, strUrl))
				{
					TRACEX("strUrl [%s]", strUrl);
					ShellExecute(NULL, "open", "iexplore.exe", strUrl, NULL, SW_SHOWNORMAL);		
				}
			}
		}
	}

	SaveFoldedGrp();
	*pResult = 0;
}



void CIdmsMsnDlg::FolderDialogOpen(char* szResult)
{	
	/*
	char file[260];
	bool bResult;
	strncpy(file, szResult, sizeof(file));
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetSafeHwnd();
	ofn.lpstrFile = file;
	ofn.nMaxFile = sizeof(file);
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	//ofn.Flags = bSending ? OFN_FILEMUSTEXIST : OFN_OVERWRITEPROMPT;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	
	//if (Sending)
	//	bResult = GetOpenFileName(&ofn) ? true : false;
	//else
	
	bResult = GetSaveFileName(&ofn) ? true : false;
	
	if (bResult)
	{
		strcpy(file, ofn.lpstrFile);		
		strcpy(szResult, ofn.lpstrFile);		
		return ;
	}
    */
	//FolderBrowserDialog .ShowDailog() 
	
	BROWSEINFO bi = { 0 };
    //TCHAR path[1024];
    bi.lpszTitle = _T("���� ���� ��� ����");
    bi.pszDisplayName = szResult;
	bi.ulFlags = BIF_NEWDIALOGSTYLE ; // platform SDK �ʿ��� 

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
        // get the name of the folder        
		::SHGetPathFromIDList(pidl, szResult);

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
	}
	
}


void CIdmsMsnDlg::OnGrpMngAddcomp() 
{
	// TODO: Add your command handler code here
	// �׷� �ϳ��� ���õǾ��־����.
	CStringArray strIPAry, strRNameAry;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;	
	int nBockMode = 0;			
	long n_StatusFlags = 0l;
	BOOL bFail = FALSE;

	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();

	if(nCnt > 1)
	{
		char szMsg[100];
		memset(szMsg,0x00, sizeof(szMsg));
		sprintf(szMsg,"��ȭ��� �߰��� �� �׷쿡 ���ؼ��� �����մϴ�. �׷� �ϳ��� �����ϼ���");
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);	
		
		pMsgThread->CreateThread();	

		return;
	}

	TRACE("nCnt [%d]\n", nCnt);	
	POSITION pos = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� ������ [%s]\n", szSelItemIP);
		
		if( strcmp(szSelItemIP, "GRP") ==0 )
		{
			CNewCompanionDlg dlg(NULL, szGrpName);
			dlg.SetParentHandle(this->GetSafeHwnd());
			dlg.DoModal();				
		}
	}
}

void CIdmsMsnDlg::OnToolsAbout() 
{
	// TODO: Add your command handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}

void CAboutDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CIdmsMsnDlg::OnGrpMngAdd() 
{
	// TODO: Add your command handler code here
	CGroupMngDlg dlg;
	dlg.setDlgRole(C_ROLE_ADD_GRP);	
	strcpy(dlg.m_szTitle, "�׷� �߰�");
	dlg.DoModal();
	if( strlen(dlg.m_szGroupName) >0 )
	{
		// �ߺ� Check => �������� ������ ���޵ȴ�.
		// ������ ����				
		if(CChatSession::Instance().RequestAddGroup(dlg.m_szGroupName) != TRUE)
		{
			return ;
		}		
	}
}

// �׷� ����
void CIdmsMsnDlg::OnGrpMngDel() 
{
	// TODO: Add your command handler code here
	// �⺻�׷� �� �����Ұ�
	// ��ȭ��븦 CHILD �� ������ ���� ���� ��츸 ���� ������.		
	BOOL bChildExists = FALSE;
	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
		
	char szDelGrps[1024];
	memset(szDelGrps , 0x00, sizeof(szDelGrps));

	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent) && m_TreeCtrl.IsSelected(hCurrent) )
		{			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"�׷� ������ ��ȭ��밡 �ϳ��� ���� ���¿����� ���� �����մϴ�.\n���� ��ȭ��� �׷��� �����ϼ��� ");	
			
			pMsgThread->CreateThread();		
			
			return;			
		}
		

		if ( !m_TreeCtrl.ItemHasChildren(hCurrent) && m_TreeCtrl.IsSelected(hCurrent) )
		{
			memset(szSelItemIP, 0x00, sizeof(szSelItemIP));
			memset(szNickName, 0x00, sizeof(szNickName));
			memset(szRealName, 0x00, sizeof(szRealName));
			memset(szGrpName, 0x00, sizeof(szGrpName));
			
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;
				
			BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP,szSelItemID,szNickName,szRealName,
													szGrpName, bConnStatus,&nBockMode,&n_StatusFlags);

			if(strcmp("�⺻�׷�", szGrpName)==0)
			{				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,"�⺻�׷��� �����Ҽ� �����ϴ�.");	
				
				pMsgThread->CreateThread();	
				
				return;				
			}

			if(strcmp("���ͳ� ���ã��", szGrpName)==0)
			{				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,"���ͳ� ���ã�� �׷��� �����Ҽ� �����ϴ�.");	
				
				pMsgThread->CreateThread();	
				
				return;				
			}
			
			

			TRACE("�� OnRButtonDown ���û���: ���õ� GRP [%s]\n", szGrpName);				
			
			// �׷� ����
			strcat(szDelGrps, szGrpName);
			szDelGrps [strlen(szDelGrps)]  = DELIM1;
		}

		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
		
	}
	
	// ������ ����! �����ڷ� ���е� ���ڿ��� ����!
		
	if(CChatSession::Instance().RequestDelGroups(szDelGrps) != TRUE)
	{
		return ;
	}	
	
}
// ��ȭ��� ����
void CIdmsMsnDlg::OnDelete() 
{
	// TODO: Add your command handler code here
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;
	CTreeItemList itemList;
	char szDelIDs[1024];
	memset(szDelIDs , 0x00, sizeof(szDelIDs));

	m_TreeCtrl.GetSelectedList( itemList );
	
	int nCnt = itemList.GetCount();

	TRACE("nCnt [%d]\n", nCnt);
	if( nCnt == 0)
	{						
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ������ �����ϼ���");
		
		pMsgThread->CreateThread();
	}
	
	int nRtn = MessageBox( "��ȭ��븦 �����Ͻðڽ��ϱ�?", "�� �� Ȯ ��", MB_OK | MB_OKCANCEL | MB_ICONINFORMATION );
	
	if(IDOK != nRtn)
	{
		return;
	}

	POSITION pos = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);		
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, 
									bConnStatus,&nBockMode,&n_StatusFlags);
		TRACE("�� Delete Comp ���õ� ID [%s]\n", szSelItemID);
		
		strcat(szDelIDs, szSelItemID);
		szDelIDs [strlen(szDelIDs)]  = DELIM1;		

		/*  KIND : 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
		 *	A�� B�� �����ϴ°��
			B.nBockMode = 1,2 �ΰ�� => 1. A�� ������ ��û�ؼ� BUDDY_LIST���� A.KIND = '3', B.KINE = '4' UPDATE !
										2. ������ A�� �޽��������� A Ʈ������ B����, MAP A.nBockMode = 3 ����.
										3. ������ B�� �޽��������� B Ʈ������ A���ܻ��� ǥ��, MAP A.nBockMode = 4 ����.

			B.nBockMode = 4 �ΰ��	=>	KIND 5 UPDATE!	
										(B�� A�� �ٽ� �߰��ÿ��� �߰��Ұ� ó���� Client���� �Ѵ�. ��ȭ������� DB�����ü��ְ� ������ �ʴ´�)		

		 *	A�� B�� �ٽ� �߰�		=>	������ ��û�ؼ� BUDDY_LIST���� A.KIND = '1', B.KINE = '2' UPDATE !
		 */		
	}
	
	if(nBockMode !=9)
	{
		// ������ ����! �����ڷ� ���е� ���ڿ��� ����!		
		if(CChatSession::Instance().RequestDelBuddys(szDelIDs) != TRUE)
		{
			return ;
		}	
	}	
}

// �Ҽ� �׷� ����
void CIdmsMsnDlg::OnGrpChg() 
{
	// TODO: Add your command handler code here
	CGroupMngDlg dlg;
	dlg.setDlgRole(C_ROLE_CHG_COMP_GRP);	
	strcpy(dlg.m_szTitle, "��ȭ��� �׷� ����");
	dlg.DoModal();

	if( strlen(dlg.m_szGroupName) >0 )
	{		
		CString strChgGrpBuddy = "";
		
		char szSelItemIP	[15 + 1];	
		char szSelItemID    [20 + 1];
		char szRealName		[50 + 1];
		char szNickName		[C_NICK_NAME_LEN +1];
		char szGrpName      [C_GRP_NAME_LEN +1];
		BOOL bConnStatus = FALSE;
		int nBockMode = 0;
		long n_StatusFlags = 0l;		
		
		CTreeItemList itemList;
		m_TreeCtrl.GetSelectedList( itemList );
		
		int nCnt = itemList.GetCount();

		TRACE("nCnt [%d]\n", nCnt);

		strChgGrpBuddy+= dlg.m_szGroupName;
		strChgGrpBuddy+= DELIM1;			

		POSITION pos = itemList.GetHeadPosition();
		for (int i=0;i < nCnt; i++)
		{
			HTREEITEM hSel = itemList.GetNext(pos);
			
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;
				
			m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, 
										bConnStatus, &nBockMode, &n_StatusFlags);
			TRACE("�� ���õ� ������ [%s]\n", szSelItemIP);
		    
			//strcat((LPSTR)(LPCSTR)strChgGrpBuddy, szSelItemID);
			strChgGrpBuddy+= szSelItemID;
			strChgGrpBuddy+= DELIM1;			
		}
		
		if(nBockMode != 9)
		{
			// ������ ����				
			if(CChatSession::Instance().RequestChgBuddyGroup((LPCSTR)strChgGrpBuddy) != TRUE)
			{
				return ;
			}		
		}
	}	
}

// �׷�� ����
void CIdmsMsnDlg::OnGrpMngModify() 
{
	// TODO: Add your command handler code here
	/////////////////////////////////////////
	
	//���� ���� ������ �׷���� �˰��־����,	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	//char szGrpName      [C_GRP_NAME_LEN +1];
	char szBefGrpName	[C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;		

	int nCntGrp = 0;
	int nFoundDefaultGrp = 0, nFoundITGrp = 0;

	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();

	TRACE("nCnt [%d]\n", nCnt);	
	
	POSITION pos = itemList.GetHeadPosition();
	
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID, szNickName,szRealName,
									szBefGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� GRP NAME [%s]\n", szBefGrpName);

		if(strcmp( "GRP", szSelItemIP )==0)
		{
			nCntGrp++;			
		}
		else
		{			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"�׷��� �����ϼ���.");	
			
			pMsgThread->CreateThread();	
		
			return ;
		}

		if(nCntGrp >= 2)
		{			
			// �׷��� �ϳ��� �����ؼ� �̸� ���� �����ϰ���.			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"�׷� �̸� ������ �ѹ��� 1���� �����մϴ�.");	
			
			pMsgThread->CreateThread();	
		
			return ;
		}
		else
		{
			if(strcmp("�⺻�׷�", szBefGrpName)==0)
			{
				nFoundDefaultGrp = 1;
				break;
			}

			if(strcmp("���ͳ� ���ã��", szBefGrpName)==0)
			{
				nFoundITGrp = 1;
				break;
			}

			
		}
	}
	
	if(nFoundDefaultGrp == 1 )
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"�⺻�׷��� ���� �Ҽ������ϴ�.");			
		pMsgThread->CreateThread();
		return ;
	}

	if(nFoundITGrp == 1 )
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"���ͳ� ���ã�� �׷��� ���� �Ҽ������ϴ�.");			
		pMsgThread->CreateThread();
		return ;
	}
	

	TRACE("���� szBefGrpName [%s]\n", szBefGrpName);	
		 
	CGroupMngDlg dlg;
	strcpy(dlg.m_szTitle, "�׷� �̸� ����");
	strcpy(dlg.m_szGroupName, szBefGrpName);
	
	dlg.setDlgRole(C_ROLE_CHG_GRP_NAME);	
	dlg.DoModal();
	
	if(strlen(dlg.m_szGroupName) == 0)
	{
		return;
	}

	TRACE("���� dlg.m_szGroupName [%s]\n", dlg.m_szGroupName);	
	
	//�ߺ� üũ
	if(!chkDupGrp(dlg.m_szGroupName))
	{
		TRACE("�ߺ��� GRP NAME!! \n");		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"�׷���� �����ϰų� �ߺ��Ǿ����ϴ�. �ٸ��̸��� �����ϼ���.");
		
		pMsgThread->CreateThread();

		return ;
	}
	
	// ������ ����
	CString strBefAftGrpNames;			
	strBefAftGrpNames.Format("%s%c%s%c",szBefGrpName, DELIM1, dlg.m_szGroupName, DELIM1 );

	if(CChatSession::Instance().RequestChgGroupName((LPCSTR)strBefAftGrpNames) != TRUE)
	{
		return ;
	}	
}

// �׷� ���� ���¸� ini ���Ͽ� �����Ѵ�
BOOL CIdmsMsnDlg::SaveFoldedGrp()
{	
	CString strFoldedGrp ;
	strFoldedGrp.Empty();
		
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;	
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	CString ItemText;
		
	while (hCurrent != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP,szSelItemID, szNickName,szRealName,
												szGrpName,bConnStatus,&nBockMode,&n_StatusFlags);		
		//if(strcmp(szGrpName, szGrpName_par)==0)
		//{
		//	TRACE("�ߺ��� GRP NAME ! \n");
		//	return FALSE;
		//	break;
		//}
  		
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			if( strcmp("OPEN" , szNickName)==0)
			{
								
			}
			else if( strcmp("CLOSE" , szNickName)==0)
			{	
				strFoldedGrp+= CString(szGrpName); 
				strFoldedGrp+= CString("|"); 				
			}
		}		
  
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}	

	TRACE("strFoldedGrp [%s] \n",strFoldedGrp );
	CINIManager::Instance().SetValue(INI_GRP_FOLDED, (LPSTR)(LPCSTR)strFoldedGrp);	

	CINIManager::Instance().SaveINIFile();

	return TRUE;

	//CINIManager::Instance().GetValue(INI_GRP_FOLDED, strPassWd);
}


void CIdmsMsnDlg::SetFoldedGrp()
{
	CString strFoldedGrp ;
	strFoldedGrp.Empty();
		
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;	
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	CString ItemText;
	
	CINIManager::Instance().GetValue(INI_GRP_FOLDED, strFoldedGrp);
	TRACE("strFoldedGrp [%s] \n",strFoldedGrp );	

	while (hCurrent != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP,szSelItemID, szNickName,szRealName,
												szGrpName,bConnStatus,&nBockMode,&n_StatusFlags);		
		  		
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			TreeItemData* pTreeItemDta = m_TreeCtrl.GetCustomItemData(hCurrent);
			string strOutFoledGrp("");
			
			// ȿ���������� �ʴ�!! ����...
			for(int i=0; i< 3000; i++) // 3000 �� �̻� �׷� �����???... ^^ ~~
			{
				getDelimitData( (LPCSTR)strFoldedGrp, '|', strOutFoledGrp , i );

				TRACE("szGrpName [%s] strOutFoledGrp [%s]\n" , szGrpName, strOutFoledGrp.c_str() );

				if( strcmp( strOutFoledGrp.c_str() , szGrpName)==0)
				{
					// �׷� �������� ó��!
					m_TreeCtrl.SetItemUserInfo(hCurrent, szSelItemIP, szSelItemID, "CLOSE",szRealName, 
								szGrpName, bConnStatus, nBockMode, n_StatusFlags);		
					
					m_TreeCtrl.DeleteItemImage(hCurrent, pTreeItemDta->dwNextImageId -1);
					m_TreeCtrl.AddItemBitmap( hCurrent, m_hGrpClose, ppFirstLeft );

					CRect rectItem;
					m_TreeCtrl.GetItemRect(hCurrent, &rectItem,FALSE);
					m_TreeCtrl.InvalidateRect(rectItem);	

					m_TreeCtrl.Expand(hCurrent, TVE_COLLAPSE ) ; 
					break;
				}				
								
				if(strOutFoledGrp.empty())
				{
					TRACE("Ln [%d] break\n" , __LINE__);
					break;
				}				
			}
		}		
  
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}	

	
}

// ����Ʈ �� �ߺ� ����ڰ� ������ FALSE
BOOL CIdmsMsnDlg::chkDupGrp(const char* szGrpName_par)
{	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];	
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;	
	
	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	CString ItemText;
		
	while (hCurrent != NULL)
	{
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hCurrent, szSelItemIP,szSelItemID, szNickName,szRealName,
												szGrpName,bConnStatus,&nBockMode,&n_StatusFlags);		
		if(strcmp(szGrpName, szGrpName_par)==0)
		{
			TRACE("�ߺ��� GRP NAME ! \n");
			return FALSE;
			break;
		}

		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}	

	return TRUE;
}

// �׷쿡 ���� ������ 
void CIdmsMsnDlg::OnGrpMngSendfile() 
{
	// TODO: Add your command handler code here
	//CStringArray strIPAry, strRNameAry;
	int nSelCnt = 0;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	
	//���� ������ ������ ������
	CFileDialog dlg(TRUE, NULL, NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT); //| OFN_ALLOWMULTISELECT
	dlg.DoModal();	
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
			strPath = strRootPath + "|" + strTmp  ; // RootPath|���ϰ��	
			nFileCnt++;
		}
		else{
			strPath = strPath + "|" + strTmp  ;
			nFileCnt++;
		}		
	}	
	
	strPath = strPath + "|";
	TRACE("strPath [%s]\n", strPath);

	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();
	TRACE("nCnt [%d]\n", nCnt);	

	if(nCnt == 0)
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ������ �׷��� �����ϼ���");
		pMsgThread->CreateThread();
		return ;
	}

	POSITION posTItem = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(posTItem);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID,szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� ������ [%s]\n", szSelItemIP);
				
		if( strcmp(szSelItemIP, "GRP")==0)
		{
			// ���õ� �׷쿡 �Ҽӵ� ��� ��ȭ���.
			CBuddyInfo * pCompanion;
			POSITION posMap;
			
			for( posMap = CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); posMap != NULL; )
			{		
				if(posMap)
				{
					CString key;
					CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( posMap, key, (CObject*&)pCompanion );

					if(	strcmp(szGrpName, pCompanion->m_szBuddyGrp) == 0 )
					{
						//0 �����ȵ� 1 ���� ������ 2 ���濡 ���ؼ� ���ܴ���
						if	(
								pCompanion && pCompanion->m_bConnStatus == TRUE && 
								( pCompanion->m_nYouBlocked == 1  || pCompanion->m_nYouBlocked == 2 )
							)
						{
							nSelCnt++ ;
							
							CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr );
						
							if( NULL == pSpckObjMap )
							{					
								TRACE("socket connect fail! [%s]\n", strPath);	
								
								CString strErrMsg;
								strErrMsg.Format("%s �� �����Ҽ� �����ϴ�", pCompanion->m_szIpAddr);
								CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
								pMsgThread->CreateThread();
							}
							else
							{				
								TRACE("\nsocket connected! [%s]\n", strPath);
								
								CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID,
									CString(pCompanion->m_szIpAddr), (LPSTR)(LPCSTR) strPath , nFileCnt) ;
							} 
						}
						else if (pCompanion->m_nYouBlocked == 4 )
						{
							char szFmt[100] ;
							memset(szFmt,0x00,sizeof(szFmt));
							sprintf(szFmt,"��ȭ�ź� ��� [%s] ���� ������ �����Ҽ� �����ϴ�", pCompanion->m_szUserName);				
						
							CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
							
							pMsgThread->CreateThread();
						}
						else
						{
							char szFmt[100] ;
							memset(szFmt,0x00,sizeof(szFmt));
							sprintf(szFmt,"OffLine ��� [%s] ���� ������ �����Ҽ� �����ϴ�", pCompanion->m_szUserName);				
						
							CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
							
							pMsgThread->CreateThread();
						}
					}					
					
					key.Empty();
				}		
			}

			break; // �׷��Ѱ� !
		}	
	}
}

void CIdmsMsnDlg::OnAlwaysOntop() 
{
	// TODO: Add your command handler code here
	
}

LRESULT CIdmsMsnDlg::OnConfigChg(WPARAM wParam, LPARAM lParam)
{
	m_strMyNick = CChatSession::Instance().m_strMyNick;
	UpdateData(FALSE); 

	return TRUE;
}



void CIdmsMsnDlg::OnToolsOption() 
{
	char szSetVal[10+1];
	memset(szSetVal, 0x00,sizeof(szSetVal));
	// TODO: Add your command handler code here
	COptionDlg dlg( (LPCSTR)(CChatSession::Instance().m_Nick),TRUE,this ) ;

	int nAwayTime = CINIManager::Instance().GetValueInt(INI_AWAY_SEC);

	dlg.m_nAwayTimeSecOld = nAwayTime;
	dlg.DoModal();
	
	
	if(dlg.m_bNickChg == TRUE)
	{		
		// ��ȭ�� ���� �۾�				
		//if(CChatSession::Instance().RequestChgNick( CChatSession::Instance().mMyInfo.m_szNickName, dlg.m_szNick) != TRUE)
		//{
		//	TRACE("CChatSession::RequestChgNick return FAIL!\n");		
		//	return ;
		//}			
		//CChatSession::Instance().m_Nick = dlg.m_szNick;
		//strncpy(CChatSession::Instance().mMyInfo.m_szNickName, dlg.m_szNick, sizeof(CChatSession::Instance().mMyInfo.m_szNickName) ); 

		m_strMyNick = CChatSession::Instance().m_strMyNick;
		m_strMyNick.Replace("''","'");

		UpdateData(FALSE); 
	}
	
	switch(dlg.m_nAwayTimeComBo)	
	{
	case 0: // 1 min
		CChatSession::Instance().m_AwaySec = 1 * 60; 
		break;
	case 1: // 3 min
		CChatSession::Instance().m_AwaySec = 3 * 60; 
		break;
	case 2: // 5 min
		CChatSession::Instance().m_AwaySec = 5 * 60; 
		break;
	case 3: // 10 min
		CChatSession::Instance().m_AwaySec = 10 * 60; 
		break;
	case 4: // 15 min
		CChatSession::Instance().m_AwaySec = 15 * 60; 
		break;
	case 5: // 20 min
		CChatSession::Instance().m_AwaySec = 20 * 60; 
		break;
	case 6: // 30 min
		CChatSession::Instance().m_AwaySec = 30 * 60; 
		break;
	case 7: // ��������
		CChatSession::Instance().m_AwaySec = -1;  
		break;
	}

	sprintf(szSetVal, "%d", CChatSession::Instance().m_AwaySec);
	
	CINIManager::Instance().SetValue(INI_AWAY_SEC , szSetVal);

	// always On Top
	
	CString strAlwaysTop;
	CINIManager::Instance().GetValue(INI_ALWAYS_ON_TOP, strAlwaysTop);

	if(strAlwaysTop == "Y")
		SetWindowPos(&wndTopMost, 0, 0, 0, 0 ,SWP_NOSIZE | SWP_NOMOVE); 
	else
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0 ,SWP_NOSIZE | SWP_NOMOVE); 
    
  }

//�׷쿡 ����������
void CIdmsMsnDlg::OnGrpMngAllmsg() 
{
	// TODO: Add your command handler code here
	// �׷� �ϳ��� ���õǾ��־����.	
	CStringArray strIDAry, strRNameAry;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	
	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();
	TRACE("nCnt [%d]\n", nCnt);	
	POSITION pos = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID,szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� ������ [%s]\n", szSelItemIP);
				
		if( strcmp(szSelItemIP, "GRP")==0)
		{
			// ���õ� �׷쿡 �Ҽӵ� ��� ��ȭ���.
			CBuddyInfo * pCompanion;
			POSITION pos;			
			
			for( pos = CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); pos != NULL; )
			{		
				if(pos)
				{
					CString key;
					CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( pos, key, (CObject*&)pCompanion );
					
					if(pCompanion && pCompanion->m_bConnStatus == TRUE && strcmp(szGrpName, pCompanion->m_szBuddyGrp) == 0 )
					{
						strIDAry.Add( pCompanion->m_szBuddyID);	
						strRNameAry.Add( pCompanion->m_szUserName);
					}
					
					key.Empty();
				}		
			}

			break; // �׷��Ѱ� !
		}	
	}
	
	if(strIDAry.GetSize() > 0 )	
	{			
		if(m_pAllMSgSendDlg)
		{			
			CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
			pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
			pWnd->SendMessage(WM_TO_ALLMSG, (WPARAM)&strIDAry, (LPARAM)&strRNameAry);
		}
		else
		{
			m_pAllMSgSendDlg = new CAllMsgDlg(strIDAry, strRNameAry);
			m_pAllMSgSendDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_ALLMSG), GetDesktopWindow() );			
			CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
			m_pAllMSgSendDlg->ShowWindow(SW_SHOW);			
			// ��޸��� Ȱ��ȭ!
			//::PostMessage(m_pAllMSgSendDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);				
			//m_pAllMSgSendDlg->SetForegroundWindow();
			//m_pAllMSgSendDlg->SetActiveWindow();
		}
	}
	
	else
	{
		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"OnLine�� �ƴϰų� ���ñ׷쿡 ��ȭ��밡 �����ϴ�.");	
		
		pMsgThread->CreateThread();
	}
}

void CIdmsMsnDlg::OnTraymenuRestore() 
{
	// TODO: Add your command handler code here
	Restore();	
}

void CIdmsMsnDlg::OnQuit() 
{
	// TODO: Add your command handler code here
	TrayIcon(FALSE);

	KillTimer(10);
	KillTimer(100);
	KillTimer(500); //Alive
	KillTimer(600); //Alarm 
	if(CChatSession::Instance().m_bDailyJob == TRUE)
	{
		// Log Off �ð� ����
		CTime ctToday = CTime::GetCurrentTime();		

		char szSql [200];
		memset(szSql , 0x00, sizeof(szSql));
		sprintf(szSql, "UPDATE KM_DAILYJOBMST SET LOGOFF_TIME=SYSDATE WHERE USER_ID='%s' AND JOB_DAY='%s'", (LPCSTR)m_strLoginID, (LPCSTR) (ctToday.Format("%Y%m%d"))  ) ;

		if(CChatSession::Instance().RequestRawSQL(RAWSQL_UPDATE_LOGOFFTIME, szSql, strlen(szSql) ) != TRUE)
		{
			TRACE("Log Off �ð� ���� ����!\n");
		}
	}

	CDialog::OnOK();
	
}

void CIdmsMsnDlg::OnViewChatHist() 
{
	// TODO: Add your command handler code here
	char szWinDir[512];	
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	char szDir[1024];		
	memset(szDir, 0x00, sizeof(szDir));	
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));

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
				
		strFileName.Format("%s\\��ȭ�̷�\\",szDirTmp);	
	}
	else
	{
		/////////////////////////////////////////////////////////						
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));
		
		// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
		// �̷� ���ڿ����� D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ ���� ���Ѵ�.
		
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
		
		strFileName.Format("%s\\��ȭ�̷�\\",szDirTmp);	
	}


	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	
	sprintf(szDir, "%s\\explorer.exe %s",szWinDir, (LPCSTR)strFileName);			
	WinExec(szDir, SW_SHOWDEFAULT);		
	
	strFileName.Empty();
}

void CIdmsMsnDlg::OnViewAllmsgHist() 
{
	// TODO: Add your command handler code here

	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
	
	char szWinDir[512];	
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDir, 0x00, sizeof(szDir));			
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));

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
				
		strFileName.Format("%s\\��������\\",szDirTmp);	
	}
	else
	{		
		char szDirTmp[512];				
		memset(szDirTmp, 0x00, sizeof(szDirTmp));			
		
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

		// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
		// �̷� ���ڿ����� D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ ���� ���Ѵ�.
		
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
		
		strFileName.Format("%s\\��������\\",szDirTmp);	
	}

	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	
	sprintf(szDir, "%s\\explorer.exe %s",szWinDir, (LPCSTR)strFileName);			
	WinExec(szDir, SW_SHOWDEFAULT);		
	
	strFileName.Empty();	
}

void CIdmsMsnDlg::OnSendfile() 
{
	// TODO: Add your command handler code here
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;

	BOOL bFound = FALSE;

	// �����Ѱ��� �ְų� ��Ƽ������ ��� �������� IP	
	
	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );
	
	int nCnt = itemList.GetCount();

	TRACE("nCnt [%d]\n", nCnt);

	//���� ������ ������ ������
	CFileDialog dlg(TRUE, NULL, NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT); //| OFN_ALLOWMULTISELECT
	dlg.DoModal();	

	if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
	{
		TRACE("FNERR_BUFFERTOOSMALL!!\n");
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"���� ���� ���ϸ� ���� �ʰ��Ͽ����ϴ�. �������� ������ �ٿ�������~. ");				
		pMsgThread->CreateThread();	
		return;
	}

	
	POSITION pos = dlg.GetStartPosition();
	CString strPath="" , strTmp ="", strRootPath = "";
	int nFileCnt = 0;
	//strPath = dlg.GetPathName();	
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
			strPath = strRootPath + "|" + strTmp  ; // RootPath|���ϰ��	
			nFileCnt++;
		}
		else{
			strPath = strPath + "|" + strTmp  ;
			nFileCnt++;
		}		
	}	
	
	strPath = strPath + "|";
	TRACE("strPath [%s]\n", strPath);

	/////////////////////////////////////////////////////////////
	
	POSITION posTItem = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(posTItem);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID,szNickName,szRealName,
									szGrpName,bConnStatus,&nBockMode, &n_StatusFlags);
		TRACE("�� R Click Send File ���� USER [%s] [%s]\n", szSelItemID , szSelItemIP);

		CBuddyInfo*  pCompanion = NULL;

							
		if( CChatSession::Instance().mBuddyInfoMap.Lookup(szSelItemID, ( CObject*& ) pCompanion ))
		{		
			bFound = TRUE;
			if(pCompanion->m_bConnStatus == TRUE && ( pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2))
			{
				CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)szSelItemIP );
						
				if( NULL == pSpckObjMap )
				{					
					TRACE("socket connect fail! [%s]\n", strPath);	
					
					CString strErrMsg;
					strErrMsg.Format("%s �� �����Ҽ� �����ϴ�", szSelItemIP);
					CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
					pMsgThread->CreateThread();
				}
				else
				{				
					TRACE("\nsocket connected! [%s]\n", strPath);
					
					CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID, CString(szSelItemIP), (LPSTR)(LPCSTR) strPath , nFileCnt) ;
				} 
			}
			else if(pCompanion->m_nYouBlocked == 4)
			{
				//< 1 (�����߰�) 2(������ ���� �߰�) 3(���� ������) 4(������ ������) 5(������ ���� �����ϰ� ���� ������ ������)
				char szFmt[100] ;
				memset(szFmt,0x00,sizeof(szFmt));
				sprintf(szFmt,"��ȭ���� ��� [%s] ���� ������ �����Ҽ� �����ϴ�", pCompanion->m_szUserName);				
			
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
				
				pMsgThread->CreateThread();				
			}
			else
			{
				char szFmt[100] ;
				memset(szFmt,0x00,sizeof(szFmt));
				sprintf(szFmt,"OffLine ��� [%s] ���� ������ �����Ҽ� �����ϴ�", pCompanion->m_szUserName);				
			
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
				
				pMsgThread->CreateThread();
				
			}
		}
	}

	if(nCnt == 0)
	{
		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ������ ������ �����ϼ���");
		
		pMsgThread->CreateThread();
	}
}

//R ��ư Ŭ���� ���� ������ ���ý�..
void CIdmsMsnDlg::OnAllmsg() 
{
	// TODO: Add your command handler code here
	TRACEX("OnAllmsg");

	CStringArray strIDAry, strRNameAry;

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	
	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();
	TRACE("nCnt [%d]\n", nCnt);	
	POSITION pos = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID,szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� ������ [%s]\n", szSelItemIP);
		
		//nBockMode == 9 ���ã�� 
		if(bConnStatus == FALSE && strcmp(szSelItemIP, "GRP")!=0 && nBockMode !=9 )
		{
			char szMsg[100];
			memset(szMsg,0x00, sizeof(szMsg));
			sprintf(szMsg,"OFF Line ��ȭ��� IP[%s] %s���� ������ �����ϴ�.", szSelItemIP, szRealName );
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);	
			
			pMsgThread->CreateThread();	
		}		
		else 		
		if( strcmp(szSelItemIP, "GRP")==0)
		{
			TRACE("�� �׷켱�ý� �������׺����� [%s]\n", szGrpName);
		}
		else 		
		if( nBockMode == 9)
		{
			TRACE("�� ���ͳ� ���ã�� ����\n");
		}
		else
		{
			CBuddyInfo * pCompanion;
						
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(szSelItemID, ( CObject*& ) pCompanion ))
			{
				if(pCompanion)
				{
					strIDAry.Add(szSelItemID);	
					strRNameAry.Add(szRealName);
				}
			}
		}		
	}
	
	if(strIDAry.GetSize() > 0 )	 
	{		
		if(m_pAllMSgSendDlg)
		{							
			CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
			pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
			pWnd->SendMessage(WM_TO_ALLMSG, (WPARAM)&strIDAry, (LPARAM)&strRNameAry);
		}
		else
		{
			m_pAllMSgSendDlg = new CAllMsgDlg(strIDAry, strRNameAry);
			m_pAllMSgSendDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_ALLMSG), GetDesktopWindow() );			
			CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
			m_pAllMSgSendDlg->ShowWindow(SW_SHOW);			
			
			// ��޸��� Ȱ��ȭ!
			//::PostMessage(m_pAllMSgSendDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);				
			//m_pAllMSgSendDlg->SetForegroundWindow();
			//m_pAllMSgSendDlg->SetActiveWindow();
		}		
	}
}

// �޴��� ���� ������
void CIdmsMsnDlg::OnBroadcastMsg() 
{
	// TODO: Add your command handler code here
	CStringArray strIDAry, strRNameAry;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	BOOL bFail = FALSE;

	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();
	TRACE("nCnt [%d]\n", nCnt);	
	POSITION pos = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID,szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���õ� ������ [%s]\n", szSelItemID);
		
		if(bConnStatus == FALSE && strcmp(szSelItemIP, "GRP")!=0 && nBockMode != 9)
		{
			char szMsg[100];
			memset(szMsg,0x00, sizeof(szMsg));
			sprintf(szMsg,"OFF Line ��ȭ��� IP[%s] %s���� ������ �����ϴ�.", szSelItemIP, szRealName );
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);	
			
			pMsgThread->CreateThread();	

			bFail = TRUE;
		}	
		else if( nBockMode == 4)
		{
			char szMsg[100];
			memset(szMsg,0x00, sizeof(szMsg));
			sprintf(szMsg,"��ȭ���� ��� IP[%s] %s���� ������ �����ϴ�.", szSelItemIP, szRealName );
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);	
			
			pMsgThread->CreateThread();	

			bFail = TRUE;
		}			
		else 
		if( strcmp(szSelItemIP, "GRP")==0)
		{
			TRACE("�� �׷켱�ý� �������׺����� [%s]\n", szGrpName);
		}
		else
		{
			CBuddyInfo * pCompanion;
						
			if(CChatSession::Instance().mBuddyInfoMap.Lookup(szSelItemID, ( CObject*& ) pCompanion ))
			{
				if(pCompanion)
				{
					strIDAry.Add(szSelItemID);	
					strRNameAry.Add(szRealName);
				}
			}
		}		
	}
	
	//if(strIDAry.GetSize() > 0 )���þ��� ���¿��� ȭ���� ����� ������ ���õ� �����ϰ��Ѵ�.
	//{			
	if(m_pAllMSgSendDlg)
	{			
		CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
		pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
		pWnd->SendMessage(WM_TO_ALLMSG, (WPARAM)&strIDAry, (LPARAM)&strRNameAry);
	}
	else
	{
		m_pAllMSgSendDlg = new CAllMsgDlg(strIDAry, strRNameAry);
		m_pAllMSgSendDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_ALLMSG), GetDesktopWindow() );			
		CWnd* pWnd = CWnd::FromHandle(m_pAllMSgSendDlg->GetSafeHwnd());
		m_pAllMSgSendDlg->ShowWindow(SW_SHOW);			
		// ��޸��� Ȱ��ȭ!
		//::PostMessage(m_pAllMSgSendDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);				
		//m_pAllMSgSendDlg->SetForegroundWindow();
		//m_pAllMSgSendDlg->SetActiveWindow();
	}	
	/*
	}	
	else
	{
		if(bFail == FALSE)
		{				
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"��ȭ��븦 �����ϼ���.");	
			
			pMsgThread->CreateThread();						
			
		}		
	}
	*/
}

/*
LRESULT CIdmsMsnDlg::OnFolderSelecting(WPARAM wParam, LPARAM lParam)
{	
	char szWndTitleTmp[120];
	memset(szWndTitleTmp, 0x00, sizeof(szWndTitleTmp));

	// IP �� ID�� ����.		
	char szTmpID[20+1];
	CString strName;
	CString key;
	memset(szTmpID, 0x00, sizeof(szTmpID));
	POSITION pos;
	CBuddyInfo * paComp;	
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnServerClosed"); 
	for( pos =  CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); pos != NULL; )
	{		
		if(pos)
		{
			CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( pos, key, (CObject*&)paComp );			
			if(paComp)
			{				
				if (paComp->m_bConnStatus && strcmp(paComp->m_szIpAddr, (char*)lParam)==0)
				{
					strncpy(szTmpID, paComp->m_szBuddyID, sizeof(szTmpID));					
				
					strName = paComp->m_szUserName ;
					
					break;
				}					
			}
		}		
	}
		
	//sprintf(szWndTitleTmp, "File Send To [%s]", FileInfo->szToIp);
	sprintf(szWndTitleTmp, "File Send To [%s] [%s] [%s]", szTmpID, strName, (char*)lParam);
	
		
	CWnd* pWnd = FindWindow(NULL, szWndTitleTmp);
	if(pWnd != NULL)
	{			
		//::SendMessage(pWnd->m_hWnd, WM_FOLDER_SELECTING, NULL, NULL);
		::PostMessage(pWnd->m_hWnd, WM_FOLDER_SELECTING, NULL, NULL); //20061227
	}
	
	delete (char*)lParam;

	return TRUE;
}
*/

LRESULT CIdmsMsnDlg::OnSearchID(WPARAM wParam, LPARAM lParam)
{
	//�̹� �����ִ� ID �˻� Dlg �� �޽��� ������
	LRESULT rslt;
	HWND hWnd = ::FindWindow(NULL, "��ȭ ��� �߰�");
	if(hWnd != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle(hWnd);
			
		//rslt = ::SendMessage(pWnd->m_hWnd, WM_SEARCH_ID, NULL, lParam);
		rslt = ::PostMessage(pWnd->m_hWnd, WM_SEARCH_ID, NULL, lParam); //20061227
	}
     
	return TRUE;
}


/*
LRESULT CIdmsMsnDlg::OnNoThanksFileTransfer(WPARAM wParam, LPARAM lParam)
{
	
	//�̹� �����ִ� FileSendDlg �� ���� �޽��� ������ szFromIP
	
	char szWndTitleTmp[120];
	memset(szWndTitleTmp, 0x00, sizeof(szWndTitleTmp));

	// IP �� ID�� ����.		
	char szTmpID[20+1];
	CString strName;
	CString key;
	memset(szTmpID, 0x00, sizeof(szTmpID));
	POSITION pos;
	CBuddyInfo * paComp;	
	plugware::CAutoLock LockCriticlaSection( CChatSession::Instance().m_criSecLocalScopeBuddyInfo,"OnServerClosed"); 
	for( pos =  CChatSession::Instance().mBuddyInfoMap.GetStartPosition(); pos != NULL; )
	{		
		if(pos)
		{
			CChatSession::Instance().mBuddyInfoMap.GetNextAssoc( pos, key, (CObject*&)paComp );			
			if(paComp)
			{				
				if ( paComp->m_bConnStatus && strcmp(paComp->m_szIpAddr, (char*)lParam)==0)
				{
					strncpy(szTmpID, paComp->m_szBuddyID, sizeof(szTmpID));
					
					
					strName = paComp->m_szUserName ;
					
					break;
				}					
			}
		}		
	}
		
	
	sprintf(szWndTitleTmp, "File Send To [%s] [%s] [%s]", szTmpID, strName, (char*)lParam);

	
	LRESULT rslt;
	HWND hWnd = ::FindWindow(NULL, szWndTitleTmp);
	if(hWnd != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle(hWnd);
		TRACEX("CIdmsMsnDlg::OnNoThanksFileTransfer => WM_FILE_SEND_CANCEL SendMsg\n");	
		
		rslt = ::PostMessage(pWnd->m_hWnd, WM_FILE_SEND_CANCEL, NULL, NULL); //20061227
	}
	
	char szMsg[100];
	memset(szMsg,0x00, sizeof(szMsg));
	sprintf(szMsg,"[%s]���� ���� ������ �ź��Ͽ����ϴ�. ���������� �����մϴ�.", (char*)lParam );
	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);
	
	pMsgThread->CreateThread();	

	delete (char*)lParam;
     
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnCancelFileTransfer(WPARAM wParam, LPARAM lParam)
{	
	
	S_CANCEL_FILE_TRANSFER * pCancelFileTran = (S_CANCEL_FILE_TRANSFER*)(lParam );	
	CSP_Utility<S_CANCEL_FILE_TRANSFER> spTest = pCancelFileTran;

	char szWndTitleTmp[30];
	memset(szWndTitleTmp, 0x00, sizeof(szWndTitleTmp));
	sprintf(szWndTitleTmp, "File Recv From [%s]", pCancelFileTran->szFromIP);
	
	char szMsg[100];
	memset(szMsg,0x00, sizeof(szMsg));
	sprintf(szMsg,"[%s]���� ���� ������ ����Ͽ����ϴ�. ���������� �����մϴ�.", pCancelFileTran->szFromIP );
	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);	
	
	pMsgThread->CreateThread();	

	CWnd* pWnd = FindWindow(NULL, szWndTitleTmp);
	if(pWnd != NULL)
	{			
		//::SendMessage(pWnd->m_hWnd, WM_FILE_SEND_CANCEL, NULL, NULL);
		::PostMessage(pWnd->m_hWnd, WM_FILE_SEND_CANCEL, NULL, NULL); //20061227
	}
	

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnFileTranDlgClose(WPARAM wParam, LPARAM lParam)
{	
	
	CString strFromUserID = (char*)wParam;
	TRACE("OnFileTranDlgClose [%s]", strFromUserID);
	CFileTransferDlg* pa;	
	if(mStrFileTransferDlgMap.Lookup(strFromUserID, ( CObject*& ) pa ))
	{
		CFileTransferDlg * pDlg = static_cast<CFileTransferDlg*>(pa);
		if(pDlg)
		{
			delete pDlg;
			mStrFileTransferDlgMap.RemoveKey(strFromUserID);
		}
	}
    
	return TRUE;
}
*/


void CIdmsMsnDlg::OnRunIdms() 
{
	// TODO: Add your command handler code here
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	char szPath1[256];
	
	getcwd(szPath1, sizeof(szPath1));

	CString CmdLineStr;	
	CString strDBKind;
	//CINIManager::Instance().GetValue(INI_DB_KIND, strDBKind);

	CINIManager::Instance().GetValue("SERVER", strDBKind); //DB���� !!! 20070117

	CmdLineStr.Format("%s %s %s %s %s %s %s %s %s %s %s",
		IDMS_MAIN,
		CChatSession::Instance().m_LoginID,
		CChatSession::Instance().m_LoginName,
		CChatSession::Instance().m_LoginTeamCd,
		CChatSession::Instance().m_LoginTeamName,
		CChatSession::Instance().m_TopTeamCode,
		CChatSession::Instance().m_TopTeamName,
		CChatSession::Instance().m_Grade,
		CChatSession::Instance().m_strIAMS_AUTHORITY,
		CChatSession::Instance().m_strPROG_SELECT,
		strDBKind
		);		

     if( !CreateProcess( NULL, // No module name (use command line). 
						CmdLineStr.GetBuffer(256),
						NULL,
						NULL,
						FALSE,
						0,
						NULL,
						NULL,
						&si,
						&pi )             
    ) 
    {
        TRACE( "CreateProcess failed." );
    }
	

    // Wait until child process exits.
    // WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );	
}

void CIdmsMsnDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 100) 
	{
		//TRACEX("�� �ڸ���� ó�� Timer!!\n");

		time_t timeGap = Dormant::GetDormantTime(); 
		
		if(	CChatSession::Instance().m_AwaySec != -1 && 
			timeGap > CChatSession::Instance().m_AwaySec)  // 10 min. : 60*10 = 600
		{
			// �ٸ� �빫���� �ƴ� ��츸 �ڸ����� ǥ��			
			/*			
			if ( !(m_MeStatusFlags & CFL_BUSY))
			{
				if ( !(m_MeStatusFlags & CFL_AWAY)
					)
				{		
					TRACEX("�� �ڸ���� ó��\n");
					//m_MeStatusFlags |= CFL_BUSY;
					m_MeStatusFlags |= CFL_AWAY;
					CChatSession::Instance().SendMyStatusInfo(STATUS_AWAY);					
				}
			}		
            */ 
			//if ( !(m_MeStatusFlags & CFL_BUSY))
			
			if	( 
				!(m_MeStatusFlags & CFL_AWAY) &&
				!(m_MeStatusFlags & CFL_BUSY) &&
				!(m_MeStatusFlags & CFL_LOOK_AWAY)
				)
			{		
				TRACEX("�� �ڸ���� ó��\n");
				//m_MeStatusFlags |= CFL_BUSY;
				m_MeStatusFlags |= CFL_AWAY;
				CChatSession::Instance().SendMyStatusInfo(STATUS_AWAY);
			}
			
		}
		else if(CChatSession::Instance().m_AwaySec != -1 && timeGap < 5) 
		{			
			if (	
					!(m_MeStatusFlags & CFL_BUSY) &&
					!(m_MeStatusFlags & CFL_LOOK_AWAY)
				)
			{
				if ( m_MeStatusFlags & CFL_AWAY)
				{					
					TRACE("�� �ڸ���� ����\n");

					m_MeStatusFlags &= ~CFL_AWAY;
					m_MeStatusFlags &= ~CFL_BUSY;
					CChatSession::Instance().SendMyStatusInfo(STATUS_ONLINE);				
				}
			}			
		}
	}
	else 
	if (nIDEvent == 200) 
	{
		//���� �⵿�� Ʈ���� ������
		TrayIcon(TRUE);

		KillTimer(200);
	}
	else if (nIDEvent == 300) 
	{
		// ���� ���� ������ �õ�
		char szMsg[100];
		memset(szMsg, 0x00, sizeof(szMsg));
		sprintf(szMsg, "���� ������ %d�� �õ���\r\n", ++m_nCntReConnect);
		m_statusBar.SetWindowText(szMsg);	

		TRACEX(szMsg);

		FlashWindow(TRUE);	
		
		//Log
		//CString strTmp;	
		//CTime iDT =  CTime::GetCurrentTime();
		//CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
		//writeLogFile("c:\\ClientLog.log", "[%d]�� ������ �õ��� [%s]\n" , m_nCntReConnect - 1, strtmDetailed);				
		
		BOOL bRtn = FALSE;
		
		if(m_bConnectTrying == FALSE)
		{
			m_bConnectTrying = TRUE;
			bRtn = CChatSession::Instance().SocketInit();

			if(bRtn == TRUE)
			{				
				KillTimer(300);			
				
				m_TreeCtrl.EnableWindow(TRUE);
				CString strTmp,cstr;	
				CTime iDT =  CTime::GetCurrentTime();
				strTmp = iDT.Format("%Y�� %m�� %d�� / %H�� %M�� %S��");
				cstr.Format("������ �Ǿ����ϴ� : %s", strTmp);
				m_statusBar.SetWindowText(cstr);			
				CChatSession::Instance().mbConnected = TRUE; 
				KillTimer(100);
				KillTimer(500);
				m_nCntReConnect = 0;
				m_bReConnectFlag = TRUE;
				m_TreeCtrl.DeleteAllItems();

				PostMessage(WM_INIT_WORK, (WPARAM)NULL, (LPARAM)NULL); 
			}

			m_bConnectTrying = FALSE ;
		}		
	}
	else if (nIDEvent == 400) 
	{
		KillTimer(400); // ���α׷� �⵿�� 1����.

		// ���� �������� -> �̰��� �׷��� ������ ���Ŀ� ó���Ǿ��Ѵ�! 
		// �������� ID �� ���� ������ ������ �׷��� ������ ����!!!!!!! 
		
		CChatSession::Instance().RequestSlipMsg();		
	}
	else if (nIDEvent == 500) 
	{		
		//CChatSession::Instance().SendAlive();				
	}
	else if (nIDEvent == 600) 
	{
		//�˶� ó��
		//DoAlarmWork();
	}
	

	CDialog::OnTimer(nIDEvent);
}

//20070619
LRESULT CIdmsMsnDlg::OnPowerBroadcast(WPARAM dwPowerEvent, LPARAM dwData) 
{ 
    TRACE("WM_POWERBROADCAST : \r\n");
	// WM_POWERBROADCAST �ý��� �������� �޼���
    // PBT_APMQUERYSUSPEND ������� ���� �Ҷ� ������ �޼���
    // BROADCAST_QUERY_DENY �ý��� ���� �޼��� �ź�

    if (dwPowerEvent == PBT_APMQUERYSUSPEND) 
    {
        TRACE("PBT_APMQUERYSUSPEND -> BROADCAST_QUERY_DENY\r\n");
        //return BROADCAST_QUERY_DENY; 
		return TRUE;
    }
	else if (dwPowerEvent == PBT_APMRESUMESUSPEND) 
	{
		// �ѹ� ������ heart beat ������ ������ ó���ǰ� �Ѵ�.		
		// Alive to Server ����
		CChatSession::Instance().SendAlive();		
	}
    else 
    {
        TRACE("etc return TRUE;\r\n");
        return TRUE; 
    }
	return TRUE; 
} 

void CIdmsMsnDlg::DoAlarmWork() 
{
	//TRACE("Alarm Timer!!\n");
//	long lCount = 0;
//	CString sQuery;
//
//	if( !m_bDocOpened ) return false;
//	if( nIndex < 0 ) return false;
//
//	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
//		date.GetYear(), date.GetMonth(), date.GetDay() );
//
//	IXMLDOMNodeListPtr pNodeList;
//	IXMLDOMNodePtr     pNode;
//
//	try
//	{
//		pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
//		lCount = pNodeList->length;
//		if( lCount == 0 || nIndex > lCount - 1 ) return false;
//
//		pNode = pNodeList->Getitem(nIndex);
//
//		pData->nIcon = _ttoi( (LPCTSTR)(pNode->attributes->getNamedItem(L"icon")->text) );
//		pData->strTitle = (LPCTSTR)(pNode->selectSingleNode(L"title")->text);
//		pData->strContent = (LPCTSTR)(pNode->selectSingleNode(L"content")->text);
//		pData->strContent.Replace(_T("\n"), _T("\r\n"));
//		pData->strContent.Replace(_T("\r\r\n"), _T("\r\n"));
//	}
//	catch(...)
//	{
//		return false;
//	}
//
//	return true;


}

void CIdmsMsnDlg::OnMultichat() 
{
	// TODO: Add your command handler code here	
	int i = 0;
	CStringArray strIDAry, strRNameAry ; //, strIPAryOffLine, strRNameAryOffLine;
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;

	CTreeItemList itemList;
	m_TreeCtrl.GetSelectedList( itemList );	
	int nCnt = itemList.GetCount();
	TRACE("nCnt [%d]\n", nCnt);	
	POSITION pos = itemList.GetHeadPosition();
	for (i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos);
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;
				
		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP,szSelItemID, szNickName,szRealName,szGrpName, 
									bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("�� ���� [%s] [%s]\n", szSelItemID, szRealName);
		
		if( bConnStatus != FALSE && strcmp(szSelItemIP, "GRP") !=0 && nBockMode != 4 && nBockMode != 9) 
		{
			strIDAry.Add(szSelItemID);	
			strRNameAry.Add(szRealName);						
		}
	}

	// ��Ƽä���� ��밡 2�� �̻��϶��� ����	
    
	if(strIDAry.GetSize() < 2)
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"��Ƽä���� �¶��� ��ȭ��밡\r\n2�� �̻��϶��� �����մϴ�.");	
	
		pMsgThread->CreateThread();		
		
		return; 
	}
	
	// ���� ������ �ȵ����Ƿ� �Է��ؾ���.
	strIDAry.Add(CChatSession::Instance().mMyInfo.m_szBuddyID);
	strRNameAry.Add(CChatSession::Instance().mMyInfo.m_szUserName);	

	// �̹� ������ ����ڵ�� ������ ��Ƽä�� Session�� �����ϴ��� üũ.
	// �� ����ڰ� ������ ��Ƽä���� session���� MAX_MULTI_CHAT_SESSION ���� �����Ѵ�.
	// key�� ������ �����ϰ�, �������� Room�� �����ߴٸ� ���� Room��ȣ�� �����ش�
	// �̹�ȣ�� ������ ���̾�α� Ÿ��Ʋ�� ����, ���� map�� ����.
	// ���� Room��ȣ�� ����� ��츸 ��Ƽä���� �����ϰ� Ȱ��ȭ ��Ų��.

	SET_STR setMultiChatID; 
	int nSize = strIDAry.GetSize();	
	CString strID;
	for( i=0; i < nSize; i++)
	{
		strID = strIDAry.ElementAt(i);	
		setMultiChatID.insert( (LPCSTR)strID ) ; 
	}
		
	//CMultiChatMsgDlg * pMCDlg;	
	CMultiChatWnd* pMCWnd = NULL ;
	CString key;
	int ntotalClgCnt = 0;

	for( pos = mStrMultiChatDlgMap.GetStartPosition(); pos != NULL; )
	{
		if(pos)
		{
			mStrMultiChatDlgMap.GetNextAssoc( pos, key, (CObject*&)pMCWnd );			
			if(pMCWnd)
			{
				ntotalClgCnt++;
				if(ntotalClgCnt >= MAX_MULTI_CHAT_SESSION )
				{
					char szMsg [200];
					memset(szMsg , 0x00, sizeof(szMsg));
					sprintf(szMsg, "��Ƽä���� %d�������� �����մϴ�. �ٸ� ��Ƽä���� ������ �õ��ϼ���.", MAX_MULTI_CHAT_SESSION); 
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szMsg);		
					pMsgThread->CreateThread();
					return ; 
				}

				if(setMultiChatID == pMCWnd->m_setID)
				{					
					//CWnd* pWnd = CWnd::FromHandle(pMCWnd->GetSafeHwnd());
					pMCWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
					//pMCWnd->SetForegroundWindow();
					//pMCWnd->SetActiveWindow();					
					return ; 
				}
			}
		}		
	}

		
	//key ���� 
	char szDlgKey [15+1] ;
	memset(szDlgKey , 0x00, sizeof(szDlgKey));
	sprintf(szDlgKey , "L%d", m_MultiChatDlgCnt++);	

	if(strIDAry.GetSize() > 0 )	
	{	
		// ���� Multi Chat�� ���������� Host�� ����.

		CMultiChatWnd* pWnd = new CMultiChatWnd( NULL, szDlgKey, strIDAry , strRNameAry );			
		strncpy(pWnd->m_szHostID, CChatSession::Instance().mMyInfo.m_szBuddyID, sizeof(pWnd->m_szHostID) ); 

		pWnd->CreateEx(WS_EX_NOPARENTNOTIFY  ,
					AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
					NULL, 
					WS_TILEDWINDOW| WS_THICKFRAME | WS_BORDER | WS_MINIMIZEBOX, 					
					100, 100, 
					420, 320, 
					NULL, NULL);									
		
		// Wnd Map �� ���� 
		mStrMultiChatDlgMap.SetAt( szDlgKey, pWnd);				
		// Multi Chat ������ ID �� ������ MAp�� ����.
		CChatSession::Instance().m_mapStr2SetMultiChat.insert( MAP_STR2SET::value_type( szDlgKey , setMultiChatID ) );	
					
		pWnd->SetWindowPos( &wndTop, 0, 0, 0, 0, 
							SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW );				
							
		::PostMessage(pWnd->GetSafeHwnd(), WM_LBUTTONDOWN , NULL, NULL);
		

		////////////////////////////////////////////////////////////
		/*
		//CMultiChatMsgDlg* pDlg = new CMultiChatMsgDlg( NULL, szDlgKey, strIDAry , strRNameAry , & CChatSession::Instance().mMyInfo);
		//strncpy(pDlg->m_szHostID, CChatSession::Instance().mMyInfo.m_szBuddyID, sizeof(pDlg->m_szHostID) ); 
		// Dlg Map �� ���� 
		//mStrMultiChatDlgMap.SetAt( szDlgKey, pDlg);				
		// Multi Chat ������ ID �� ������ MAp�� ����.
		//CChatSession::Instance().m_mapStr2SetMultiChat.insert( MAP_STR2SET::value_type( szDlgKey , setMultiChatID ) );	
		//pDlg->Create(MAKEINTRESOURCE(IDD_DIALOG_MULTI_CHAT), GetDesktopWindow() );		
		//CWnd* pWnd = CWnd::FromHandle(pDlg->GetSafeHwnd());
		//pDlg->ShowWindow(SW_SHOW);		
		//::PostMessage(pDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);						
		//pDlg->SetForegroundWindow();
		//pDlg->SetActiveWindow();						
		//pDlg->m_CEditMsg.SetFocus();
		*/
	}	
	else
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"��ȭ��븦 �����ϼ���.");	
	
		pMsgThread->CreateThread();
	}
}

LRESULT CIdmsMsnDlg::OnMChatServerRoomNo(WPARAM wParam, LPARAM lParam)
{	
	//toDo!\ 
	
	CMultiChatWnd * pMCDlg;	

	S_SERVER_ROOMNO * psMCSrvRoomNo =  (S_SERVER_ROOMNO*) lParam ;
	
	CSP_Utility<S_SERVER_ROOMNO> spTest = psMCSrvRoomNo;

	// Map Key���� �����Ѵ�. => Client ���ǹ�ȣ->�������� ���� RoomNo��,,
	if(mStrMultiChatDlgMap.Lookup(psMCSrvRoomNo->m_szClientDlgID, ( CObject*& ) pMCDlg ) )
	{
		if(pMCDlg)
		{
			// Dlg ID ����
			strncpy(pMCDlg->m_szDlgID, psMCSrvRoomNo->m_szServerRoomNo, sizeof(pMCDlg->m_szDlgID));
			mStrMultiChatDlgMap.RemoveKey(psMCSrvRoomNo->m_szClientDlgID);
			mStrMultiChatDlgMap.SetAt(psMCSrvRoomNo->m_szServerRoomNo, pMCDlg); // �ٽ� Set...
			pMCDlg->m_statusBar.SetWindowText("");	
			pMCDlg->m_pBtnSnd->EnableWindow(TRUE);
			pMCDlg->m_pChatREdit->EnableWindow(TRUE);
			pMCDlg->m_pREditSendMsg->EnableWindow(TRUE);
			
			//�������� ���� ��ȣ�� ����.
			char szTmp[50];
			memset(szTmp, 0x00, sizeof(szTmp));
			sprintf(szTmp, "Multi Chat[%s]", psMCSrvRoomNo->m_szServerRoomNo);				
			pMCDlg->SetWindowText(szTmp); //20061107
		}
	}
	
	// MultiChat ������ map�� key����
	MAP_STR2SET::iterator it ;
	it =  CChatSession::Instance().m_mapStr2SetMultiChat.find(psMCSrvRoomNo->m_szClientDlgID);
	
	if(it != CChatSession::Instance().m_mapStr2SetMultiChat.end() )  
	{	
		TRACEX("�������� ���� RoomNo �� ���� [%s]", psMCSrvRoomNo->m_szServerRoomNo);

		// �������� ���� RoomNo �� ����,
		CChatSession::Instance().m_mapStr2SetMultiChat.insert( MAP_STR2SET::value_type( psMCSrvRoomNo->m_szServerRoomNo , (*it).second ) );	

		// Client Dlg ID Key�� Erase ...
		CChatSession::Instance().m_mapStr2SetMultiChat.erase(it);	
	}	

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnSlipMsgSaveSuccess(WPARAM wParam, LPARAM lParam)
{	
	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"������ ����Ǿ����ϴ�.");		
	pMsgThread->CreateThread();
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAddItComp(WPARAM wParam, LPARAM lParam)
{
	S_ADD_IT_COMP * pAddItComp = (S_ADD_IT_COMP*)lParam;
	CSP_Utility<S_ADD_IT_COMP> spTest = pAddItComp;
	
	//desc DELIM1 url

	// Ʈ���� �߰�
	HTREEITEM* pTreeItem =  InsertGrpNode("���ͳ� ���ã��");
	CString strTmp;
	strTmp.Format("%s", pAddItComp->m_szDesc );

	HTREEITEM hUser = m_TreeCtrl.InsertItem( strTmp, *pTreeItem, TVI_LAST );				
	m_TreeCtrl.EnsureVisible( hUser );				
	
	m_TreeCtrl.SetItemUserInfo(	hUser, "","",pAddItComp->m_szDesc,"","���ͳ� ���ã��", TRUE, 9, 0l);

	m_TreeCtrl.SetItemColor( hUser, m_ColorNode );
	
	m_TreeCtrl.AddItemBitmap( hUser, m_hITComp, ppLastLeft ); 
	

	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"���ͳ� ���ã�⿡ ����Ǿ����ϴ�.");		
	pMsgThread->CreateThread();

	
	return TRUE;
}

LRESULT CIdmsMsnDlg::OnChgItComp(WPARAM wParam, LPARAM lParam)
{
	S_CHG_IT_COMP * pChgItComp = (S_CHG_IT_COMP*)lParam;
	CSP_Utility<S_CHG_IT_COMP> sp = pChgItComp;
	
	//descOld DELIM1 desc DELIM1 url

	// Old�� ã�Ƽ� Ʈ������ ����	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	CBuddyInfo *pa = NULL;	

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();	
	HTREEITEM hChildItem ;
		
	while (hCurrent != NULL)
	{
		if (m_TreeCtrl.ItemHasChildren(hCurrent))
		{
			HTREEITEM hNextItem;
			hChildItem = m_TreeCtrl.GetChildItem(hCurrent);
			
			while (hChildItem != NULL)
			{	
				bConnStatus = FALSE;
				nBockMode = 0;
				n_StatusFlags = 0l;
				
				BOOL bWorked = m_TreeCtrl.GetItemUserInfo(hChildItem, szSelItemIP,szSelItemID,szNickName,szRealName,
					szGrpName, bConnStatus,&nBockMode, &n_StatusFlags);
				
				if (bWorked && nBockMode==9 && strcmp( pChgItComp->m_szDescOld, szNickName)==0)
				{					
					m_TreeCtrl.SetItemUserInfo(	hChildItem, szSelItemIP, szSelItemID, pChgItComp->m_szDesc, 
												szRealName, szGrpName, FALSE, nBockMode, n_StatusFlags);					

					CRect rectItem;
					char szTmp[C_NICK_NAME_LEN+1];
					memset(szTmp, 0x00, sizeof(szTmp));
					sprintf(szTmp, "%s", pChgItComp->m_szDesc);						
					m_TreeCtrl.SetItemText(hChildItem, szTmp); 
					m_TreeCtrl.GetItemRect(hChildItem, &rectItem,FALSE);
					m_TreeCtrl.InvalidateRect(rectItem);

					goto EXIT_LOOP;
				}
				
				hNextItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);				
				hChildItem = hNextItem;				
			}
		}
		
		hCurrent = m_TreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);	
	}
EXIT_LOOP: ;
	

	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"���ͳ� ���ã�⿡ ����Ǿ����ϴ�.");		
	pMsgThread->CreateThread();

	
	return TRUE;
}


LRESULT CIdmsMsnDlg::OnSlipMsg(WPARAM wParam, LPARAM lParam)
{
	BOOL bDllLoad = FALSE;
	HINSTANCE hInstResourceMain;
	if(m_pSec3Dlg)
	{
		hInstResourceMain = AfxGetResourceHandle();
		HMODULE hModule = ::GetModuleHandle(NULL);
		AfxSetResourceHandle(hModule);

		bDllLoad = TRUE;
	}	

	CPtrArray* pArray  = (CPtrArray*)lParam;
		
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_SLIP_MSG *pData = static_cast<S_SLIP_MSG*>(pArray->GetAt(i));		
		
		//CViewBuddySlipMsg dlg(pData);
		//dlg.DoModal();

		CViewBuddySlipMsg* pDlg = new CViewBuddySlipMsg( pData );
		pDlg->Create(MAKEINTRESOURCE(IDD_DLG_VIEW_SLIP_MDG), GetDesktopWindow() );
		CWnd* pWnd = CWnd::FromHandle(pDlg->GetSafeHwnd());
		pDlg->ShowWindow(SW_SHOW);
		// ��޸��� Ȱ��ȭ!
		::PostMessage(pDlg->GetSafeHwnd(), WM_ACTIVATE, MAKELONG(WA_CLICKACTIVE, 0), NULL);

		pDlg->SetForegroundWindow();
		pDlg->SetActiveWindow();	
		
		char szKey [20+14+1+1];
		memset(szKey, 0x00, sizeof(szKey));

		sprintf(szKey, "%s%s", pData->m_szFromID ,pData->m_szRegDate);
		mStrSlipMsgDlgMap.SetAt( szKey, pDlg);		
	}	
	
	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_SLIP_MSG *pData = static_cast<S_SLIP_MSG*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;

	if(bDllLoad)
	{
		AfxSetResourceHandle(hInstResourceMain);
	}

	return TRUE;
}


LRESULT CIdmsMsnDlg::OnExitMChatMsg(WPARAM wParam, LPARAM lParam)
{
	CMultiChatWnd * pMCDlg;	

	char* szMCSrvRoomNo =  (char*) lParam ;	
	
	//CMsgBoxThread* pMsgThread = new CMsgBoxThread(this, (char*) wParam);
	//pMsgThread->CreateThread();

	if(mStrMultiChatDlgMap.Lookup(szMCSrvRoomNo, ( CObject*& ) pMCDlg ) )
	{
		if(pMCDlg)
		{			
			// lParam -> MChat Room No
			// wParam -> MChat ���� ID
			pMCDlg->PostMessage(WM_EXIT_MULTI_CHAT, wParam, (LPARAM)NULL ); 
		}
	}
	
	delete[] szMCSrvRoomNo ;

	return TRUE;
}


// ���� ����� 
void CIdmsMsnDlg::OnWriteMsg() 
{
	// TODO: Add your command handler code here

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;	
	
	HTREEITEM hSel = m_TreeCtrl.GetFirstSelectedItem();	
	
	TreeItemData* pTreeItemDta = m_TreeCtrl.GetCustomItemData(hSel);
		
	if (hSel != NULL)
	{	
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;

		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName,szRealName, 
								szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);		
		
		TRACE("szSelItemIP [%s]\n", szSelItemIP); 
		TRACE("szNickName  [%s]\n", szNickName); 
		TRACE("szRealName  [%s]\n", szRealName); 
		TRACE("bConnStatus  [%d]\n", bConnStatus); 
		
		if (m_TreeCtrl.ItemHasChildren(hSel))
		{
			
		}
		else
		{	
			if(nBockMode !=9)
			{
				CBuddySlipMsg dlg( szSelItemID, szRealName ) ;
				dlg.DoModal();			
			}			
		}
	}	
}

void CIdmsMsnDlg::OnViewSlipmsgLog() 
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
				
		strFileName.Format("%s\\��������\\",szDirTmp);	
	}
	else
	{
		char szWinDir[512];		
		char szDirTmp[512];
		char szDir[1024];

		memset(szWinDir, 0x00, sizeof(szWinDir));		
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
		memset(szDir, 0x00, sizeof(szDir));	

		GetWindowsDirectory(szWinDir, sizeof(szWinDir));
		
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

		// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
		// �̷� ���ڿ����� D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ ���� ���Ѵ�.
		
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

		strFileName.Format("%s\\��������\\",szDirTmp);	
	}

	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	
	sprintf(szDir, "%s\\explorer.exe %s",szWinDir, (LPCSTR)strFileName);			
	WinExec(szDir, SW_SHOWDEFAULT);		
	
	strFileName.Empty();
}

// ���� ģ�� �߰� 
void CIdmsMsnDlg::OnGrpMngAddItcomp() 
{
	// TODO: Add your command handler code here
	
	CITCompDlgMng dlg;
	dlg.m_nRole = C_ROLE_ADD_IT_COMP;
	dlg.DoModal();	
}

void CIdmsMsnDlg::OnItcompAdd() 
{
	// TODO: Add your command handler code here
	CITCompDlgMng dlg;
	dlg.m_nRole = C_ROLE_ADD_IT_COMP;
	dlg.DoModal();	
}

void CIdmsMsnDlg::OnItcompDel() 
{
	// TODO: Add your command handler code here

	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;	
	
	CTreeItemList itemList;
	char szDelIDs[2048];
	BOOL bNotItComp = FALSE;
	memset(szDelIDs , 0x00, sizeof(szDelIDs));
				
	m_TreeCtrl.GetSelectedList( itemList );
	int nCnt = itemList.GetCount();
				
	POSITION pos1 = itemList.GetHeadPosition();
	for (int i=0;i < nCnt; i++)
	{
		HTREEITEM hSel = itemList.GetNext(pos1);		
		
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;

		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, 
			bConnStatus,&nBockMode,&n_StatusFlags);
		
		TRACE("�� ���õ� Name [%s]\n", szNickName);

		strcat(szDelIDs, szNickName);
		szDelIDs [strlen(szDelIDs)]  = DELIM1;
	}

	if( !CChatSession::Instance().RequestDelITComp(szDelIDs))
	{
		return;
	}	
}

void CIdmsMsnDlg::OnItcompModify() 
{
	// TODO: Add your command handler code here
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[50 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;	
	long n_StatusFlags = 0l;	

	HTREEITEM hSel = m_TreeCtrl.GetFirstSelectedItem();		
		
	if (hSel != NULL)
	{	
		bConnStatus = FALSE;
		nBockMode = 0;
		n_StatusFlags = 0l;

		m_TreeCtrl.GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName,szRealName, 
								szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
	}

	/////////////////

	CITCompDlgMng dlg;
	dlg.m_nRole = C_ROLE_CHG_IT_COMP;	
	
	//strncpy(dlg.m_szDesc, szNickName, sizeof(dlg.m_szDesc) );
	dlg.m_strDESC =  szNickName;
	 

	CString strUrl;
	if(CChatSession::Instance().mITCompDescToUrlMap.Lookup(szNickName, strUrl))
	{
		//strncpy(dlg.m_szUrl, (LPCSTR)strUrl, sizeof(dlg.m_szUrl));
		dlg.m_strURL =  strUrl;
		dlg.DoModal();	
	}	
}

void CIdmsMsnDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default	

	TRACE("x [%d] y [%d]\n", point.x, point.y);
	CDialog::OnLButtonDown(nFlags, point);
}

CDialog * CIdmsMsnDlg::LoadScreen(LPCTSTR szDLLName, HMODULE& hModule)
{
	typedef CDialog *(*F)();
	hModule = LoadLibrary(szDLLName);
	
	if(hModule == NULL)
	{
		DWORD dwRet = GetLastError();

		TRACE("%d\n", dwRet);

		return NULL;
	}

	F pF= (F)GetProcAddress(hModule, "CreateDlg");

	if(pF == NULL)
	{
		DWORD dwRet = GetLastError();

		TRACE("%d\n", dwRet);

		FreeLibrary(hModule);
		return NULL;
	}

	CDialog *pBaseWnd = pF();

	if(pBaseWnd == NULL)
	{
		FreeLibrary(hModule);
		return NULL;
	}

	return pBaseWnd;
}

void CIdmsMsnDlg::OnBtnSec1() 
{
	// TODO: Add your control notification handler code here
	if(m_btnSec1.chkBtnPressed())
	{
		TRACE("Btn1 �̹� �����ִ�  .\n");
	}
	else
	{
		TRACE("Btn1 not Pressed-> ��ư1 ���� ���·�.\n");	

		m_btnSec1.AutoSize(this, IDB_BITMAP_SEC_BTN1_CLICKED);
		m_btnSec1.SetBitmaps(IDB_BITMAP_SEC_BTN1_CLICKED, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN1_CLICKED, RGB(255, 0, 255));
		m_btnSec1.DrawBorder(FALSE);
		m_btnSec1.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		// ������ ��ư���� ������·�
		m_btnSec2.AutoSize(this, IDB_BITMAP_SEC_BTN2_CLICKED);
		m_btnSec2.SetBitmaps(IDB_BITMAP_SEC_BTN2_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN2_HIDE, RGB(255, 0, 255));
		m_btnSec2.DrawBorder(FALSE);
		m_btnSec2.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);
		
		m_btnSec3.AutoSize(this, IDB_BITMAP_SEC_BTN3_CLICKED);
		m_btnSec3.SetBitmaps(IDB_BITMAP_SEC_BTN3_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN3_HIDE, RGB(255, 0, 255));
		m_btnSec3.DrawBorder(FALSE);
		m_btnSec3.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);
		
		if(m_pSec2Dlg)
		{
			m_cControlPos.RemoveControl(m_pSec2Dlg);
			delete m_pSec2Dlg;
			m_pSec2Dlg = NULL;
			FreeLibrary(m_hModuleSec2);
		}

		if(m_pSec3Dlg)
		{
			m_cControlPos.RemoveControl(m_pSec3Dlg);
			delete m_pSec3Dlg;
			m_pSec3Dlg = NULL;
			FreeLibrary(m_hModuleSec3);
		}

		//Tree Show
		m_TreeCtrl.ShowWindow(SW_SHOW);
	}	
}

void CIdmsMsnDlg::OnBtnSec2() 
{
	// TODO: Add your control notification handler code here
	if(m_btnSec2.chkBtnPressed())
	{
		TRACE("Btn2 �̹� �����ִ�  .\n");
	}
	else
	{
		TRACE("Btn2 not Pressed-> ��ư2 ���� ���·�.\n");	

		m_btnSec2.AutoSize(this, IDB_BITMAP_SEC_BTN2_CLICKED);
		m_btnSec2.SetBitmaps(IDB_BITMAP_SEC_BTN2_CLICKED, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN2_CLICKED, RGB(255, 0, 255));
		m_btnSec2.DrawBorder(FALSE);
		m_btnSec2.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		// ������ ��ư���� ������·�
		m_btnSec1.AutoSize(this, IDB_BITMAP_SEC_BTN1_CLICKED);
		m_btnSec1.SetBitmaps(IDB_BITMAP_SEC_BTN1_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN1_HIDE, RGB(255, 0, 255));
		m_btnSec1.DrawBorder(FALSE);
		m_btnSec1.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		// ������ ��ư���� ������·�
		m_btnSec3.AutoSize(this, IDB_BITMAP_SEC_BTN3_CLICKED);
		m_btnSec3.SetBitmaps(IDB_BITMAP_SEC_BTN3_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN3_HIDE, RGB(255, 0, 255));
		m_btnSec3.DrawBorder(FALSE);
		m_btnSec3.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		//Tree Hide
		m_TreeCtrl.ShowWindow(SW_HIDE);

		if(m_pSec3Dlg)
		{
			m_cControlPos.RemoveControl(m_pSec3Dlg);
			delete m_pSec3Dlg;
			m_pSec3Dlg = NULL;
			FreeLibrary(m_hModuleSec3);
		}

		// DLL Load �Ѵ� 
		if(m_pSec2Dlg == NULL)
		{			
			m_pSec2Dlg = LoadScreen("sec2_DLL.dll", m_hModuleSec2 );
		
			if(m_pSec2Dlg != NULL)
			{
				RECT cliRect;
				m_TreeCtrl.GetWindowRect(&cliRect);
				ScreenToClient( &cliRect );
				//ClientToScreen( &cliRect );

				m_pSec2Dlg->MoveWindow(	cliRect.left, cliRect.top, cliRect.right-60, cliRect.bottom - 60 );

				m_cControlPos.AddControl(&*m_pSec2Dlg	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );		

				//char szMsg[100];
				//memset(szMsg, 0x00, sizeof(szMsg));
				//sprintf(szMsg, "������ ��ȸ���Դϴ�. ��ø� ��ٸ�����.");
				//m_statusBar.SetWindowText(szMsg);	
			}		
		}
	}		
}

void CIdmsMsnDlg::OnBtnSec3() 
{
	// TODO: Add your control notification handler code here
	if(m_btnSec3.chkBtnPressed())
	{
		TRACE("Btn3 �̹� �����ִ�  .\n"); 
	}
	else
	{
		TRACE("Btn3 not Pressed-> ��ư3 ���� ���·�.\n");	

		m_btnSec3.AutoSize(this, IDB_BITMAP_SEC_BTN3_CLICKED);
		m_btnSec3.SetBitmaps(IDB_BITMAP_SEC_BTN3_CLICKED, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN3_CLICKED, RGB(255, 0, 255));
		m_btnSec3.DrawBorder(FALSE);
		m_btnSec3.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		// ������ ��ư���� ������·�
		m_btnSec1.AutoSize(this, IDB_BITMAP_SEC_BTN1_CLICKED);
		m_btnSec1.SetBitmaps(IDB_BITMAP_SEC_BTN1_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN1_HIDE, RGB(255, 0, 255));
		m_btnSec1.DrawBorder(FALSE);
		m_btnSec1.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);
		
		m_btnSec2.AutoSize(this, IDB_BITMAP_SEC_BTN2_CLICKED);
		m_btnSec2.SetBitmaps(IDB_BITMAP_SEC_BTN2_OVER, RGB(255, 0, 255), 
					IDB_BITMAP_SEC_BTN2_HIDE, RGB(255, 0, 255));
		m_btnSec2.DrawBorder(FALSE);
		m_btnSec2.OffsetColor(CMyButton::BTNST_COLOR_BK_IN, 30);

		//Tree Hide
		m_TreeCtrl.ShowWindow(SW_HIDE);
		
		if(m_pSec2Dlg)
		{
			m_cControlPos.RemoveControl(m_pSec2Dlg);
			delete m_pSec2Dlg;
			m_pSec2Dlg = NULL;
			FreeLibrary(m_hModuleSec2);
		}

		// DLL Load �Ѵ� 
		if(m_pSec3Dlg == NULL)
		{			
			m_pSec3Dlg = LoadScreen("sec3_DLL.dll", m_hModuleSec3 );
		
			if(m_pSec3Dlg != NULL)
			{
				RECT cliRect;
				m_TreeCtrl.GetWindowRect(&cliRect);
				ScreenToClient( &cliRect );
				
				m_pSec3Dlg->MoveWindow(	cliRect.left, cliRect.top, cliRect.right-60, cliRect.bottom - 60 );

				m_cControlPos.AddControl(&*m_pSec3Dlg	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );		
			}		
		}
		// CSec3Dlg dlg; 
	}	
}



BOOL CIdmsMsnDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	switch(pMsg->message)  
	{
    case WM_LBUTTONDOWN:            
    case WM_LBUTTONUP:              
    case WM_MOUSEMOVE:
        // ������ ������
        m_tooltip.RelayEvent(pMsg);
    }
	
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		return TRUE;
	}

	
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CIdmsMsnDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class

	WORD wID, wNotifyCode;
 
    wNotifyCode = HIWORD(wParam);   // ON_COMMAND Notification Codes
    wID			= LOWORD(wParam);   // Control ID


    switch(wNotifyCode)
    {	
    case BN_CLICKED :
            if( wID  == IDM_MYSTATUS_ONLINE )
			{
				// �̹��� �¶������� ����
				m_nMyStatus = IDB_BITMAP_ONLINE_B;				
				CRgn rgn;
				rgn.CreateRectRgn(7,10,39,42);
				InvalidateRgn(&rgn);
				m_strMyNick = " (�¶���) " + CChatSession::Instance().m_Nick ;
				UpdateData(FALSE); 
				
				// �¶����̵Ǿ����� ����鿡�� �˸�
				TRACE("�� �¶���\n");
				m_MeStatusFlags &= ~CFL_AWAY;
				m_MeStatusFlags &= ~CFL_BUSY;
				CChatSession::Instance().SendMyStatusInfo(STATUS_ONLINE);

				break;
			}
			else if( wID  == IDM_MYSTATUS_BUSY )
			{
				// �̹��� �ٸ��빫������ ����								
				m_nMyStatus = IDB_BITMAP_BUSY_B;
				CRgn rgn;
				rgn.CreateRectRgn(7,10,39,42);
				InvalidateRgn(&rgn);

				m_strMyNick = " (�ٸ� �빫��) " + CChatSession::Instance().m_Nick ;
				UpdateData(FALSE); 

				// �ٸ��빫�� �Ǿ����� ����鿡�� �˸�
				TRACEX("�� �ٸ��빫�� ó��\n");
				m_MeStatusFlags |= CFL_BUSY;
				
				CChatSession::Instance().SendMyStatusInfo(STATUS_BUSY);		

				break;
			}
			else if( wID  == IDM_MYSTATUS_AWAY )
			{
				// �̹��� �ڸ���� ����		
				TRACEX("�� �ڸ���� ó��\n");
				m_nMyStatus = IDB_BITMAP_BUSY_B;
				CRgn rgn;
				rgn.CreateRectRgn(7,10,39,42);
				InvalidateRgn(&rgn);
				
				m_MeStatusFlags |= CFL_LOOK_AWAY;
				CChatSession::Instance().SendMyStatusInfo(STATUS_AWAY);
				m_strMyNick = " (�ڸ����) " + CChatSession::Instance().m_Nick ;
				UpdateData(FALSE); 				

				break;
			}

            break;
	//case BN_DOUBLECLICKED : 

    default :
        break;
    }	
	
	return CDialog::OnCommand(wParam, lParam);
}



void CIdmsMsnDlg::OnTraymenuIdms() 
{
	// TODO: Add your command handler code here
	OnRunIdms() ;
}

void CIdmsMsnDlg::OnRunMdb() 
{
	// TODO: Add your command handler code here
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	char szWinDir[512];		
	char szDirTmp[512];	
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
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
	if (_chdir( szDirTmp) == 0)
	{
		if( !CreateProcess( NULL, // No module name (use command line). 
			"MdbManager",
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi )             
			) 
		{
			TRACE( "CreateProcess failed." );
		}
	}   
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

void CIdmsMsnDlg::OnRunSockTest() 
{
	// TODO: Add your command handler code here
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	char szWinDir[512];		
	char szDirTmp[512];	
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
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
	if (_chdir( szDirTmp) == 0)
	{
		if( !CreateProcess( NULL, // No module name (use command line). 
			"PacketSender",
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi )             
			) 
		{
			TRACE( "CreateProcess failed." );
		}
	}   
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

void CIdmsMsnDlg::OnRunTuxTest() 
{
	// TODO: Add your command handler code here
	HKEY hKey;
	DWORD dwDisp,dwType,dwSize ;

	char szValue[255];

	memset(szValue, 0,255);

	if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, "NICIS_ED VERSION\\��\\",0,NULL,
		REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp))
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0, // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			);
		// Process any inserts in lpMsgBuf.
		// Display the string.
		//MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
	}


	dwSize = sizeof(char)*255;
	dwType = REG_SZ;
	RegQueryValueEx(hKey, "ccrHome", 0, &dwType,(LPBYTE)szValue,&dwSize);  

	RegCloseKey(hKey);	

	strcat(szValue, "\\bin");
	TRACE("%s\n", szValue);	

	//////////////////////////////////////////////////////////////////////////
	
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );	
			
	if (_chdir( szValue) == 0)
	{
		if( !CreateProcess( NULL, // No module name (use command line). 
			"TestTux",
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi )             
			) 
		{
			TRACE( "CreateProcess failed." );
		}
	}   
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

}

void CIdmsMsnDlg::OnTraymenuMdb() 
{
	// TODO: Add your command handler code here
	OnRunMdb();
}

void CIdmsMsnDlg::OnTraymenuSocktest() 
{
	// TODO: Add your command handler code here
	OnRunSockTest() ;
	
}

void CIdmsMsnDlg::OnTraymenuTuxtest() 
{
	// TODO: Add your command handler code here
	OnRunTuxTest() ;
}


//section Data Request ó��
LRESULT CIdmsMsnDlg::OnSectionReq(WPARAM wParam, LPARAM lParam) 
{
	TRACE("%s\n",(char*)lParam);
	
	// �뵵�� �´� ���а����� �з����� ó��
	
	if( strcmp("Sec2_Req_LunchMenu", (char*)lParam) == 0 )
	{
		if(m_pSec2Dlg)
		{
			// ������ ��û����..
			// �Ĵ�޴� ��ȸ�Ⱓ�� ����. �̹����� ù°�Ϻ��� ������ ������ 

			/*
			UINT DayOfWeek[] = {
			   LOCALE_SDAYNAME7,   // Sunday
			   LOCALE_SDAYNAME1,   
			   LOCALE_SDAYNAME2,
			   LOCALE_SDAYNAME3,
			   LOCALE_SDAYNAME4, 
			   LOCALE_SDAYNAME5, 
			   LOCALE_SDAYNAME6   // Saturday
			};

			TCHAR strWeekday[256];
			CTime time(CTime::GetCurrentTime());   // Initialize CTime with current time

			// Get string for day of the week from system Get day of week from CTime
			::GetLocaleInfo(LOCALE_USER_DEFAULT, DayOfWeek[time.GetDayOfWeek()-1], strWeekday, sizeof(strWeekday));

			TRACE("%s\n", strWeekday);               // Print out day of the week
			*/

			CString strFromDate, strToDate;
			CTime iDT =  CTime::GetCurrentTime();						
			CTime tmFrom = CTime::GetCurrentTime();

			TRACE("[%s]\n" , iDT.Format("%Y%m%d%H%M%S") );
			int nCurDay = iDT.GetDayOfWeek();	// 1 = Sunday, 2 = Monday, ..., 7 = Saturday
			
			if(nCurDay == 2)
			{
				//nCurDay �� �������̸� �������� ���� 
				strFromDate = iDT.Format("%Y%m%d") ;
			}
			else if(nCurDay == 3)
			{
				// nCurDay �� ȭ�����̸� �������� ���� - 1 ��				
				tmFrom -= CTimeSpan(1, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			else if(nCurDay == 4)
			{
				//nCurDay �� �������̸� �������� ���� - 2 �� ...
				tmFrom -= CTimeSpan(2, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			else if(nCurDay == 5)
			{
				//��
				tmFrom -= CTimeSpan(3, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			else if(nCurDay == 6)
			{
				//��
				tmFrom -= CTimeSpan(4, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			else if(nCurDay == 7)
			{
				//��
				tmFrom -= CTimeSpan(5, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			else if(nCurDay == 1)
			{
				//��
				tmFrom -= CTimeSpan(6, 0, 0, 0);
				strFromDate = tmFrom.Format("%Y%m%d") ;
			}
			
			// 5 �� �ĸ� ������� ���Եȴ�.. 4 => �ݿ��ϱ���
			CTime tm5DaysAfter(tmFrom.GetYear(), tmFrom.GetMonth(), tmFrom.GetDay() , 0,0,0,0);
			tm5DaysAfter += CTimeSpan(4, 0, 0, 0);
			
			strToDate = tm5DaysAfter.Format("%Y%m%d");

			TRACE("[%s]\n" , strToDate);			

			if(CChatSession::Instance().RequestFoodMenu((LPCSTR)strFromDate, (LPCSTR)strToDate) != TRUE)
			{
				return FALSE ;
			}			
		}
	}
	

	return TRUE;
}



LRESULT CIdmsMsnDlg::OnAllMsgAcked(WPARAM wParam, LPARAM lParam)
{
	char* pszID = (char*) wParam ;
	TRACE("pszID [%s]\n", pszID);

	CChatSession::Instance().SendAllMsgAck( pszID, "", "" ) ;

	delete pszID;
	pszID = NULL;

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAllUserIdForAdd(WPARAM wParam, LPARAM lParam)
{
	
	//LRESULT rslt;
	HWND hWnd = ::FindWindow(NULL, "��ȭ ��� �߰�");
	if(hWnd != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle(hWnd);			
		
		::PostMessage( pWnd->m_hWnd, WM_ALL_USER_ID_TEAM_FORADD, (WPARAM)wParam, (LPARAM)lParam);
	}	
	else
	{
		delete [] (char*)lParam;
	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnAllUserIdMenu(WPARAM wParam, LPARAM lParam)
{		
	if(m_pAllMSgSendDlg)
	{
		::PostMessage( m_pAllMSgSendDlg->GetSafeHwnd(), WM_ALL_USER_ID_TEAM, (WPARAM)wParam, (LPARAM)lParam);
	}
	else
	{
		delete [] (char*)lParam;
	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::OnFoodMenu(WPARAM wParam, LPARAM lParam)
{
	if(m_pSec2Dlg)
	{
		::PostMessage( m_pSec2Dlg->GetSafeHwnd(), WM_SECTION_RESPONSE, (WPARAM)"Sec2_Res_FoodMenu", (LPARAM)lParam);
	}
	else
	{
		CPtrArray* pArray  = (CPtrArray*)lParam;
		
		for(int j = 0; j < pArray->GetSize(); j++)
		{			
			S_FOOD_MENU_CLI *pData = static_cast<S_FOOD_MENU_CLI*>(pArray->GetAt(j));
			
			if(pData != NULL)
			{				
				delete pData;
				pData = NULL;
			}
		}	
		
		delete pArray;	

	}

	return TRUE;
}

LRESULT CIdmsMsnDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(message == WM_QUERYENDSESSION )		
	{
		//writeLogFile("c:\\ClientLog.log", "WM_QUERYENDSESSION [%s]\n" , m_strLoginID);
		
		if(CChatSession::Instance().m_bDailyJob == TRUE)
		{
			// Log Off �ð� ���� => ���� ���� �ȹ���!
			CTime ctToday = CTime::GetCurrentTime();		
			
			char szSql [200];
			memset(szSql , 0x00, sizeof(szSql));
			sprintf(szSql, "UPDATE KM_DAILYJOBMST SET LOGOFF_TIME=SYSDATE WHERE USER_ID='%s' AND JOB_DAY='%s'", (LPCSTR)m_strLoginID, (LPCSTR) (ctToday.Format("%Y%m%d"))  ) ;
			
			if(CChatSession::Instance().RequestRawSQL(RAWSQL_UPDATE_LOGOFFTIME, szSql, strlen(szSql) ) != TRUE)
			{
//				CTime iDT =  CTime::GetCurrentTime();					
//				char szTmpPath[100];
//				memset(szTmpPath, 0x00, sizeof(szTmpPath));
//				sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//				writeLogFile(szTmpPath, "Log Off �ð� ���� ���� [%s]\n" , iDT.Format("%Y%m%d%H%M%S") );

				TRACE("Log Off �ð� ���� ����!\n");
			}
			Sleep(100);

			doTerminateWork();
		}
		
	}
	
	return CDialog::DefWindowProc(message, wParam, lParam);
}

BOOL CIdmsMsnDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	// TODO: Add your message handler code here and/or call default
	
	int iDataLength = pCopyDataStruct->cbData;

	//int nType = (int)pCopyDataStruct->dwData;			
	
    char szData[10240] = "";

    memcpy(szData, pCopyDataStruct->lpData, iDataLength);
	
	// ID ������ ���� ������ 
	// ����ڿ��� ���� �������� �����Ѵ�.

	string strOutID, strOutMsg;
	getDelimitData(szData, DELIM1, strOutID, 0); //ID
	getDelimitData(szData, DELIM1, strOutMsg, 1); //�޽��� 
	
	strOutID+=DELIM1 ;

	char szTemp[20];
	char* pSzBuf = NULL;	
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;	
	S_CHAT_HEADER sChatHeader;
	
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sChatHeader, NULL, sizeof(sChatHeader));	
	memset(szTemp, NULL, sizeof(szTemp));	
	
	char szTempCnt[4+1+1];
	memset(szTempCnt, NULL, sizeof(szTempCnt));	
	sprintf(szTempCnt, "%d%c" , 1, DELIM1);
	
	char szSenderName[50+1+1+1];
	memset(szSenderName, NULL, sizeof(szSenderName));	
	sprintf(szSenderName, "%c%s%c" , DELIM2, (LPCSTR) CChatSession::Instance().m_LoginName,DELIM2);
	
	// ����� ����
	
	int iLength = S_CHAT_HEADER_LENGTH + strlen(szTempCnt) +  strOutID.length() + strOutMsg.length() + strlen(szSenderName); 
	
	memcpy(sComHeader.m_szUsage, ALL_MSG, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));			
	
	memcpy(sChatHeader.szIDFrom, CChatSession::Instance().m_szMyUserID, sizeof(sChatHeader.szIDFrom) ); 
	memcpy(sChatHeader.szIDTo, "", sizeof(sChatHeader.szIDTo) ); 
	
	// ���۵����� ������ 
	
	// ToID���� DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 �޽��� DELIM1 �������̸�
	
	TRACE(" strlen(szTempCnt) [%d]\n", strlen(szTempCnt));
	TRACE(" strOutID.length() [%d]\n", strOutID.length() );
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );	
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, szTempCnt , strlen(szTempCnt));	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt), (LPCSTR)strOutID.c_str(), strOutID.length()  );			
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strOutID.length(), 
		(LPCSTR)strOutMsg.c_str(), strOutMsg.length()  );	
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strOutID.length()+strOutMsg.length(), szSenderName, strlen(szSenderName) );	
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength ;
	
	TRACE(" pSzBuf [%s]\n", (char*) pSzBuf );
	
	if(CChatSession::Instance().Send(pSzBuf, nTotalLen,"OnCopyData") < 0)	
	{
		TRACEX("Send return FAIL!\n");		
		return  FALSE;
	}
	//
    
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}


void CIdmsMsnDlg::OnRunRsBoard() 
{
	// TODO: Add your command handler code here
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	char szPath1[256];
	
	getcwd(szPath1, sizeof(szPath1));

	CString CmdLineStr;
	CmdLineStr = "MiniBoard.exe" ;		

     if( !CreateProcess( NULL, // No module name (use command line). 
						CmdLineStr.GetBuffer(256),
						NULL,
						NULL,
						FALSE,
						0,
						NULL,
						NULL,
						&si,
						&pi )             
    ) 
    {
        TRACE( "CreateProcess failed." );
    }
	

    // Wait until child process exits.
    // WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}
