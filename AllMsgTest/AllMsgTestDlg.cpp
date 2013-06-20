// AllMsgTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AllMsgTest.h"
#include "AllMsgTestDlg.h"



#pragma comment(lib, "Ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////
const char DELIM1 = 0x1F ;
const char DELIM2 = 0x1E ;

// 구조체
typedef struct _S_COMMON_HEADER
{
	// 4+9+5 = 18 
	char m_szUsage  [3+1];	///< 데이터 구분
	char m_szLength [8+1];	///< 전체 길이
	char m_szCount  [4+1];  ///< 데이터 갯수	
	
} S_COMMON_HEADER;

typedef struct _S_CHAT_HEADER
{
	char szIDTo   [20+1];	///< chat 상대방 ID
	char szIDFrom [20+1];	///< chat My ID
	
} S_CHAT_HEADER;

const char const* ALL_MSG = "026" ;
const int COMMON_HEADER_LENGTH = 18 ;
const int S_CHAT_HEADER_LENGTH = 42 ;

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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAllMsgTestDlg dialog

CAllMsgTestDlg::CAllMsgTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAllMsgTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAllMsgTestDlg)
	m_strAllMsg = _T("");
	m_strToId1 = _T("");
	m_strToId2 = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAllMsgTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAllMsgTestDlg)
	DDX_Text(pDX, IDC_EDIT_ALL_MSG, m_strAllMsg);
	DDV_MaxChars(pDX, m_strAllMsg, 255);
	DDX_Text(pDX, IDC_EDIT_TOID, m_strToId1);
	DDX_Text(pDX, IDC_EDIT_TOID2, m_strToId2);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAllMsgTestDlg, CDialog)
	//{{AFX_MSG_MAP(CAllMsgTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND, OnBtnSend)
	ON_BN_CLICKED(IDC_BTN_CONNECT, OnBtnConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAllMsgTestDlg message handlers

BOOL CAllMsgTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAllMsgTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAllMsgTestDlg::OnPaint() 
{
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
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAllMsgTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CAllMsgTestDlg::OnBtnSend() 
{
	// TODO: Add your control notification handler code here

	UpdateData(TRUE);
	int nCntTo = 0;
	CString strToIDs; 
	
	if(m_strToId1.GetLength() > 0)
	{
		strToIDs.Format("%s%c", m_strToId1,DELIM1);
		nCntTo++;
	}
	
	if(m_strToId2.GetLength() > 0)
	{
		CString strTmp;
		strTmp.Format("%s%c", m_strToId2,DELIM1);
		strToIDs += strTmp;
		nCntTo++;
	}
	
	TRACE("strToIDs [%s]\n", strToIDs );

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
	sprintf(szTempCnt, "%d%c" , nCntTo /* numListCtrl */, DELIM1);
	
	char szSenderName[50+1+1+1];
	memset(szSenderName, NULL, sizeof(szSenderName));	
	sprintf(szSenderName, "%c%s%c" , DELIM2,(LPCSTR) "FromID",DELIM2); // 보내는 사람 ID
	
	// 헤더부 설정	
	int iLength = S_CHAT_HEADER_LENGTH + strlen(szTempCnt) +  strToIDs.GetLength() + m_strAllMsg.GetLength() + strlen(szSenderName); 
	
	memcpy(sComHeader.m_szUsage, ALL_MSG, sizeof(sComHeader.m_szUsage));		
	int nRtn = sizeof(sComHeader.m_szLength)-1 ;
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));			
	
	memcpy(sChatHeader.szIDFrom, "FromID", sizeof(sChatHeader.szIDFrom) ); 
	memcpy(sChatHeader.szIDTo, "", sizeof(sChatHeader.szIDTo) ); 
	
	// 전송데이터 모으기 
	
	// ToID갯수 DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 메시지 DELIM1 전송자이름
	
	TRACE(" strlen(szTempCnt) [%d]\n", strlen(szTempCnt));
	TRACE(" strToIDs.GetLength() [%d]\n", strToIDs.GetLength() );
	
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );	
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, szTempCnt , strlen(szTempCnt));	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt), (LPCSTR)strToIDs, strToIDs.GetLength()  );			
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strToIDs.GetLength(), 
		(LPCSTR)m_strAllMsg, m_strAllMsg.GetLength()  );	
	
	memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strToIDs.GetLength()+m_strAllMsg.GetLength(), 
		szSenderName, strlen(szSenderName) );	
	
	nTotalLen = COMMON_HEADER_LENGTH + iLength ;
		
	int nRslt = ::send(m_hSocket, pSzBuf, nTotalLen, 0);		
	

	delete [] pSzBuf;
}

void CAllMsgTestDlg::OnBtnConnect() 
{
	// TODO: Add your control notification handler code here
	struct sockaddr_in addr;			
	struct	hostent *hent;	
	long	laddr;
	LPIN_ADDR paddr;
	IN_ADDR       addrHost;
	
	
	SOCKADDR_IN m_InternetAddr;		
	WSADATA wsaData;         	
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
    {
        fprintf(stderr, "WSAStartup() failed");
        exit(1);
    }

	paddr = &addrHost;
	laddr = ::inet_addr( "147.6.116.162" );
	if( laddr == INADDR_NONE ) 
	{ 
		if ( !(hent = ::gethostbyname("147.6.116.162")) ) 
		{
			TRACE("Invalid server hostname( %s ), error [%d]\n", "147.6.116.162", GetLastError() );
			//Log
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "Invalid server hostname [%s] [%s] %d\n" , iDT.Format("%Y%m%d%H%M%S") , lpSocketAddress, GetLastError()  );
			
			return ;
		}
		memcpy( paddr, hent->h_addr, sizeof(IN_ADDR) );
	} 
	else 
	{
		memcpy( &paddr->s_addr, &laddr, 4);
	}

    if ((m_hSocket = socket(PF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
    {
		TRACE("★ socket 생성에러 [%d]\n", GetLastError() );
        
        return ;
    }
    
    ZeroMemory((char*) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("147.6.116.162");
    addr.sin_port = htons((u_short)2000);	
    
    u_long    arg = 1;
    ioctlsocket(m_hSocket, FIONBIO, &arg); // non-blocking mode로 설정	
	struct  timeval     timevalue;	
	fd_set writefds;	

	
	int nRtn = connect(m_hSocket, (struct sockaddr*) &addr, sizeof(addr));
	
	if(	nRtn == SOCKET_ERROR && WSAEWOULDBLOCK != WSAGetLastError() )
	{
		if(	WSAEALREADY == WSAGetLastError() )
		{
			// 이미 연결 시도중 재시도 	
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "connect 실패 error 이미 연결 시도중 재시도 [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );
		}
		else
		{
			//Log
//			CTime iDT =  CTime::GetCurrentTime();					
//			char szTmpPath[100];
//			memset(szTmpPath, 0x00, sizeof(szTmpPath));
//			sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//			writeLogFile(szTmpPath, "connect 실패 error [%s] [%d]\n" , iDT.Format("%Y%m%d%H%M%S") ,WSAGetLastError()  );
		}		
	}	

	FD_ZERO( &writefds );
	FD_SET( m_hSocket, &writefds );
	timevalue.tv_sec = 0;
	timevalue.tv_usec = 500000; // 0.5 sec 

	nRtn = ::select( 0, NULL, &writefds, NULL, &timevalue );

	if(SOCKET_ERROR == nRtn)
	{
		//Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "connect select error [%s] [%d]\n" , iDT.Format("%Y%m%d%H%M%S") ,WSAGetLastError()  );
		
	}

	if ( !FD_ISSET( m_hSocket, &writefds ) )
	{		
		//Log
//		CTime iDT =  CTime::GetCurrentTime();					
//		char szTmpPath[100];
//		memset(szTmpPath, 0x00, sizeof(szTmpPath));
//		sprintf(szTmpPath, "c:\\ClientLog%s.log", iDT.Format("%Y%m%d")  );	
//		writeLogFile(szTmpPath, "connect 실패 [%s] \n" , iDT.Format("%Y%m%d%H%M%S")  );

		//TRACE("★ connect 실패 [%s][%d]\n", GetCurrentTimeByString(), m_hSocket);
		closesocket( m_hSocket );		
		m_hSocket = NULL;
		return ;
	}	
	
	
}
