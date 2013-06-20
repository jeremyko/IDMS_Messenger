// LogInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "INIManager.h"
#include "LogInDlg.h"
#include "..\\common\\AllCommonDefines.h"
#include "ChatSession.h"
#include "RegNewUserDlg.h"
#include "ChgPwdDlg.h"
#include "MsgBoxThread.h"
#include "DesWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogInDlg dialog


CLogInDlg::CLogInDlg(CWnd* pParent /*=NULL*/)
	: CIDMSBitmapDialog(CLogInDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogInDlg)
	m_strID = _T("");
	m_strPwd = _T("");
	m_strGreeting = _T("");
	m_strLoginMSg = _T("");
	//}}AFX_DATA_INIT
	
}


void CLogInDlg::DoDataExchange(CDataExchange* pDX)
{
	CIDMSBitmapDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogInDlg)		
	DDX_Control(pDX, IDC_ED_ID, m_EditID);
	DDX_Control(pDX, IDC_STATIC_LOGIN_MSG, m_StaMsg);
	DDX_Control(pDX, IDC_BTN_LOGIN, m_BtnLogIn);
	DDX_Control(pDX, IDC_ED_PWD, m_EditPassWd);
	DDX_Text(pDX, IDC_ED_ID, m_strID);
	DDV_MaxChars(pDX, m_strID, 20);
	DDX_Text(pDX, IDC_ED_PWD, m_strPwd);
	DDV_MaxChars(pDX, m_strPwd, 8);
	DDX_Text(pDX, IDC_STATIC_LOGIN_MSG, m_strLoginMSg);
	DDV_MaxChars(pDX, m_strLoginMSg, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogInDlg, CIDMSBitmapDialog)
	//{{AFX_MSG_MAP(CLogInDlg)	
	ON_BN_CLICKED(IDC_BTN_REG_USER, OnBtnRegUser)
	ON_BN_CLICKED(IDC_BTN_CHG_PWD, OnBtnChgPwd)
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_NEW_LOGIN, OnBtnNewLogin)
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	//}}AFX_MSG_MAP
	
	ON_MESSAGE(WM_LOGIN,			OnLogInRslt)
	ON_MESSAGE(WM_INIT_WORK,		OnInitWork)			
	ON_MESSAGE(WM_SQL_ERR		,	OnSQLRsltErr  )
	
	//ON_MESSAGE(WM_ALREADY_LOGIN,	OnAlreadyLogIn)	
	//ON_MESSAGE(WM_JOBGN,			OnJobGnRslt)		
	//ON_MESSAGE(WM_SAVE_LAST_IP,		OnSaveLastIPRslt)
	//ON_MESSAGE(WM_DAILYJOB_MST,		OnDailyJobMstRslt)
	//ON_MESSAGE(WM_INSERT_LOGONTIME,	OnInsertLogOnTmRslt)
	//ON_MESSAGE(WM_LOGIN_ALLOWED,	OnLogInAllowed)
	
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogInDlg message handlers


LRESULT CLogInDlg::OnSQLRsltErr(WPARAM wParam, LPARAM lParam)
{
	S_COM_RESPONSE* pErrInfo = (S_COM_RESPONSE*)lParam;
	
	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this, pErrInfo->szErrMsg );		
	
	pMsgThread->CreateThread();	

	m_StaMsg.SetWindowText("");				
	RedrawWindow();

	m_BtnLogIn.EnableWindow(TRUE);
	
	return FALSE;
}

BOOL CLogInDlg::OnInitDialog() 
{
	CIDMSBitmapDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	
	//SetBitmap(IDB_BITMAP_LOGIN_BG);
    
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

	// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\ 까지 구한다.
	
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
	strFileName.Format("%s\\login_bg.bmp", szDirTmp);

	m_bmpBackground.Load(strFileName);	
	
	m_Rslt = FALSE;
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());

	CINIManager::Instance().GetValue( INI_LOGIN_ID, m_strID);	
	
	CString strVal, strPassWd, strDecryptPWD;
	CINIManager::Instance().GetValue(INI_USE_AUTO_LOGIN, strVal);

	if(strVal.Compare("Y") == 0)
	{
		CINIManager::Instance().GetValue(INI_PASSWD, strPassWd);
		// 
		CDesWrapper	des;
		des.SetKey(m_strID);
		des.GetDecrypt(strPassWd, strDecryptPWD);

		m_strPwd = strDecryptPWD ;

		UpdateData(FALSE);

		OnBtnLogin() ;

		return TRUE;
	}

	UpdateData(FALSE);

	PostMessage(WM_INIT_WORK, 0,0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLogInDlg::OnBtnLoginCancel() 
{
	// TODO: Add your control notification handler code here
	
}

void CLogInDlg::OnBtnRegUser() 
{
	// TODO: Add your control notification handler code here
	CRegNewUserDlg dlg;
	dlg.m_strStatus = _T("00");
	dlg.DoModal();

	//! 다시 윈도우 핸들을 로그인의 것으로 설정.
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());
}

void CLogInDlg::OnBtnChgPwd() 
{
	// TODO: Add your control notification handler code here

	CChgPwdDlg dlg;
	dlg.DoModal();
	//! 다시 윈도우 핸들을 로그인의 것으로 설정.
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());
}



LRESULT CLogInDlg::OnInitWork(WPARAM wParam, LPARAM lParam)
{
	m_EditPassWd.SetFocus();
	return TRUE;
}


// LOGIN 결과 
LRESULT CLogInDlg::OnLogInRslt(WPARAM wParam, LPARAM lParam)
{
	// S_LOGIN

	//writeLogFile("c:\\ClientLog.log", "OnLogInRslt #1!\n");	
	
	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_LOGIN* pData = static_cast<S_LOGIN*>(pArray->GetAt(i));
		
		if(!pData->bIsSuccess)
		{	
			if(pData->strErrorMSg == NODATA_FOUND)
			{
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "등록되지 않은 사용자입니다." );
				pMsgThread->CreateThread();		
			}
			else
			{
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,(LPCSTR)pData->strErrorMSg );
				pMsgThread->CreateThread();		
			}						
					
			m_BtnLogIn.EnableWindow(TRUE);
			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;
									
			if( ++m_nTryCount == TRY_COUNT )
			{				
				char szErrMSg[255];
				memset(szErrMSg, 0x00, sizeof(szErrMSg));
				sprintf(szErrMSg, "%d번 로그인 실패로 프로그램을 종료합니다\n관리자에게 문의하세요", TRY_COUNT);
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,szErrMSg );
				
				pMsgThread->CreateThread();

				m_Rslt = FALSE;
				
				CDialog::OnCancel();
				
				return FALSE;
			}

			//strMsg.Format("로그인시도 %d / %d", m_nTryCount, TRY_COUNT);
			//SetPanelText(this, strMsg);
			
			return FALSE;
		}
		
		CString strUserPw;
		GetDecryptPw(m_strID, pData->strPASSWD, strUserPw);

		if( m_strPwd != strUserPw )
		{			
			CString strMsg;
			if( ++m_nTryCount == TRY_COUNT )
			{
				// Clear	
				for(int j = 0; j < pArray->GetSize(); j++)
				{
					S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
					if(pData != NULL)
					{
						delete pData;
						pData = NULL;
					}
				}
				delete pArray;	
				pArray = NULL;
								

				char szErrMSg[255];
				memset(szErrMSg, 0x00, sizeof(szErrMSg));
				sprintf(szErrMSg, "%d번 로그인 실패로 프로그램을 종료합니다\n관리자에게 문의하세요", TRY_COUNT);
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,szErrMSg );
				
				pMsgThread->CreateThread();

				m_Rslt = FALSE;
				
				CDialog::OnCancel();				
				return FALSE;
			}
			else
			{				
				// Clear	
				for(int j = 0; j < pArray->GetSize(); j++)
				{
					S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
					if(pData != NULL)
					{
						delete pData;
						pData = NULL;
					}
				}
				delete pArray;	
				pArray = NULL;
				
				m_StaMsg.SetWindowText("");				
				RedrawWindow();
								
				m_BtnLogIn.EnableWindow(TRUE);

				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,"비밀번호가 틀렸습니다\n다시 시도하세요" );				
				pMsgThread->CreateThread();
				
				m_EditPassWd.SetFocus();
				m_EditPassWd.SetSel(0,-1);
				
				m_Rslt = FALSE;				

				return FALSE;
			}
			
			m_Rslt = FALSE;
		}
		else
		{
			m_strKTGN = pData->strKT_GN;
			m_LoginID = m_strID;
			m_LoginName = pData->strUSER_NAME;
			//m_LoginTeamCd = pData->strTEAM_GN;
			//m_LoginTeamName = pData->strTEAM_NAME;
			//m_TopTeamCode = pData->strTOP_TEAM_GN;
			//m_TopTeamName = pData->strTOP_TEAM_NAME;
			//m_Grade = pData->strUSER_GRADE;
			m_strStatus = pData->strSTATUS;	
			
			CINIManager::Instance().SetValue(INI_LOGIN_ID, (LPSTR)(LPCSTR)m_LoginID);
			
			CChatSession::Instance().m_LoginID = m_strID;			
			CChatSession::Instance().m_LoginName = pData->strUSER_NAME;
			CChatSession::Instance().m_LoginTeamCd = pData->strTEAM_GN;
			CChatSession::Instance().m_LoginTeamName = pData->strTEAM_NAME;
			CChatSession::Instance().m_TopTeamCode = pData->strTOP_TEAM_GN;
			CChatSession::Instance().m_TopTeamName = pData->strTOP_TEAM_NAME;
			CChatSession::Instance().m_Grade = pData->strUSER_GRADE;	
			CChatSession::Instance().m_Nick = pData->strNick;
			CChatSession::Instance().m_strMyNick = " (온라인) " + CChatSession::Instance().m_Nick ;
			CChatSession::Instance().m_decrpPassWd = pData->strPASSWD;
			CChatSession::Instance().m_strIAMS_AUTHORITY = pData->strIAMS_AUTHORITY;
			CChatSession::Instance().m_strPROG_SELECT = pData->strPROG_SELECT;
						
			// 사용중인 사용자가 최초 Login을 시도한 상태
			if(m_strStatus == NEW_LOG_IN)
			{	
				// Clear	
				for(int j = 0; j < pArray->GetSize(); j++)
				{
					S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
					if(pData != NULL)
					{
						delete pData;
						pData = NULL;
					}
				}
				delete pArray;	
				pArray = NULL;
				
				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,"최초 IDMS LOGIN을 하였습니다\r\n기존 개발관리 사용자는 ID를 검색하여 정리 하시고 LOGIN 하십시요" );	
								
				pMsgThread->CreateThread();

				m_StaMsg.SetWindowText("");				
				RedrawWindow();

				m_BtnLogIn.EnableWindow(TRUE);

				m_Rslt = FALSE;				
				return FALSE;
			}
			else if(m_strStatus == DEL_LOG_IN)
			{		
				// Clear	
				for(int j = 0; j < pArray->GetSize(); j++)
				{
					S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
					if(pData != NULL)
					{
						delete pData;
						pData = NULL;
					}
				}
				delete pArray;	
				pArray = NULL;
				
				char szErrMSg[255];
				memset(szErrMSg, 0x00, sizeof(szErrMSg));
				sprintf(szErrMSg, "사용하신 LogIn ID[%s]는 삭제된 ID 입니다", m_LoginID);
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,szErrMSg );				
				pMsgThread->CreateThread();

				m_EditID.SetFocus();
				m_EditID.SetSel(0,-1);

				m_StaMsg.SetWindowText("");				
				RedrawWindow();
				m_BtnLogIn.EnableWindow(TRUE);

				m_Rslt = FALSE;
				return FALSE;
			}

			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_LOGIN *pData = static_cast<S_LOGIN*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;

			
			//로그인 성공
			char tmpStr[255];
			memset(tmpStr, 0x00, sizeof(tmpStr));
			sprintf(tmpStr, "   %s 님 환영합니다!!", (LPCSTR)m_LoginName) ;

			CWnd* pMain = AfxGetMainWnd();
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(pMain, tmpStr , 1 );	
		
			pMsgThread->CreateThread();
								
			UpdateData(FALSE);

			m_Rslt = TRUE;
			CDialog::OnOK();

			return TRUE;		

			// 마지막 로그인한 IP를 저장한다.
			// 불필요한 SQL 작업을 막는다. 20090210
			//CString strIp;
			//GetLocalIP(strIp);
			//CString		strSql;	
			//GetQueryUpdateIp(m_LoginID, strIp, strSql);
			//if(CChatSession::Instance().RequestRawSQL(RAWSQL_SAVE_LAST_IP, (LPCSTR)strSql, strlen((LPCSTR)strSql) ) != TRUE)
			//{
			//	return FALSE ;
			//}			
		
			

			break; //!!!
		}
	}	
				
	
	UpdateData(FALSE);
	
	return TRUE;
}

void CLogInDlg::GetDecryptPw(const CString& strId, const CString& strPw, CString& strDecrypt)
{
	CDesWrapper	des;
	//CString		strDecrypt;

	des.SetKey(strId);
	des.GetDecrypt(strPw, strDecrypt);

	//return strDecrypt;
}

void CLogInDlg::OnBtnLogin() 
{
	// TODO: Add your control notification handler code here

	UpdateData(TRUE);

	if( m_strID.IsEmpty() ) 
	{
		AfxMessageBox("사용자이름을 입력하세요");
		GetDlgItem(IDC_ED_ID)->SetFocus();
		return;
	}
	else if( m_strPwd.IsEmpty() )
	{
		m_EditPassWd.SetFocus();		
		AfxMessageBox("비밀번호를 입력하세요");		

		return;
	}

	// 먼저 로그인 이미 했는지 여부 확인.
	
	//if(CChatSession::Instance().RequestCheckAlreadyLogIn((LPCSTR)m_strID) != TRUE)
	//{
	//	return ;
	//}
	

	if(CChatSession::Instance().RequestLogIn(m_strID) != TRUE)
	{
		m_StaMsg.SetWindowText("");				
		RedrawWindow();

		m_BtnLogIn.EnableWindow(TRUE);
		return ;
	}
	
	
	m_BtnLogIn.EnableWindow(FALSE);

	m_strLoginMSg = " 로그인 하고 있습니다. 잠시만 기다리세요.";

	//writeLogFile("c:\\ClientLog.log", "#Debug1\n");	
	
}


void CLogInDlg::GetLocalIP(CString& strIP)
{
	//WORD		wVersionRequested;
	//WSADATA		wsaData;
	char		name[255];	
	PHOSTENT	hostinfo;

	//wVersionRequested = MAKEWORD( 2, 0 );

	//if ( WSAStartup( wVersionRequested, &wsaData ) == 0 )
	{
        if( gethostname ( name, sizeof(name)) == 0)
        {
			if((hostinfo = gethostbyname(name)) != NULL)
			{
				//strIP.Format("%s",  inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list) );
				IN_ADDR in;
				memcpy( &in, hostinfo->h_addr_list[0], 4 );            
				strIP =  inet_ntoa(in) ;
			}
        }        
	}	
}



CString CLogInDlg::GetGhostToday()
{
	CTime	ctToday;

	// 새벽 6시 이전이면 하루 전으로 변환
	ctToday = CTime::GetCurrentTime();
	if( ctToday.GetHour() < 6 )
		ctToday -= CTimeSpan(1, 0, 0, 0);

	return ctToday.Format("%Y%m%d");
}


void CLogInDlg::OnBtnNewLogin() 
{
	// TODO: Add your control notification handler code here
	
}

void CLogInDlg::OnBtnClose() 
{
	// TODO: Add your control notification handler code here
	//CChatSession::DeleteObject();
	m_Rslt = FALSE;
	CDialog::OnOK();
}


BOOL CLogInDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CIDMSBitmapDialog::PreTranslateMessage(pMsg);
}



////////////////////////////////////////////////////////////////////////////////////////
// 사용안함
////////////////////////////////////////////////////////////////////////////////////////

// LoadBMPImage	- Loads a BMP file and creates a bitmap GDI object
//		  also creates logical palette for it.
// Returns	- TRUE for success
// sBMPFile	- Full path of the BMP file
// bitmap	- The bitmap object to initialize
// pPal		- Will hold the logical palette. Can be NULL
/*
BOOL LoadBMPImage( LPCTSTR sBMPFile, CBitmap& bitmap, CPalette *pPal )
{
	CFile file;
	if( !file.Open( sBMPFile, CFile::modeRead) )
		return FALSE;

	BITMAPFILEHEADER bmfHeader;

	// Read file header
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
		return FALSE;

	// File type should be 'BM'
	if (bmfHeader.bfType != ((WORD) ('M' << 8) | 'B'))
		return FALSE;

	// Get length of the remainder of the file and allocate memory
	DWORD nPackedDIBLen = file.GetLength() - sizeof(BITMAPFILEHEADER);
	HGLOBAL hDIB = ::GlobalAlloc(GMEM_FIXED, nPackedDIBLen);
	if (hDIB == 0)
		return FALSE;

	// Read the remainder of the bitmap file.
	if (file.ReadHuge((LPSTR)hDIB, nPackedDIBLen) != nPackedDIBLen )
	{
		::GlobalFree(hDIB);
		return FALSE;
	}


	BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB ;
	BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ;

	// If bmiHeader.biClrUsed is zero we have to infer the number
	// of colors from the number of bits used to specify it.
	int nColors = bmiHeader.biClrUsed ? bmiHeader.biClrUsed : 
						1 << bmiHeader.biBitCount;

	LPVOID lpDIBBits;
	if( bmInfo.bmiHeader.biBitCount > 8 )
		lpDIBBits = (LPVOID)((LPDWORD)(bmInfo.bmiColors + bmInfo.bmiHeader.biClrUsed) + 
			((bmInfo.bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	else
		lpDIBBits = (LPVOID)(bmInfo.bmiColors + nColors);

	// Create the logical palette
	if( pPal != NULL )
	{
		// Create the palette
		if( nColors <= 256 )
		{
			UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
			LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

			pLP->palVersion = 0x300;
			pLP->palNumEntries = nColors;

			for( int i=0; i < nColors; i++)
			{
				pLP->palPalEntry[i].peRed = bmInfo.bmiColors[i].rgbRed;
				pLP->palPalEntry[i].peGreen = bmInfo.bmiColors[i].rgbGreen;
				pLP->palPalEntry[i].peBlue = bmInfo.bmiColors[i].rgbBlue;
				pLP->palPalEntry[i].peFlags = 0;
			}

			pPal->CreatePalette( pLP );

			delete[] pLP;
		}
	}

	CClientDC dc(NULL);
	CPalette* pOldPalette = NULL;
	if( pPal )
	{
		pOldPalette = dc.SelectPalette( pPal, FALSE );
		dc.RealizePalette();
	}

	HBITMAP hBmp = CreateDIBitmap( dc.m_hDC,		// handle to device context 
				&bmiHeader,	// pointer to bitmap size and format data 
				CBM_INIT,	// initialization flag 
				lpDIBBits,	// pointer to initialization data 
				&bmInfo,	// pointer to bitmap color-format data 
				DIB_RGB_COLORS);		// color-data usage 
	bitmap.Attach( hBmp );

	if( pOldPalette )
		dc.SelectPalette( pOldPalette, FALSE );

	::GlobalFree(hDIB);
	return TRUE;
}

LRESULT CLogInDlg::OnLogInAllowed(WPARAM wParam, LPARAM lParam)
{	
	//CString strQuery;
	//GetLoginQuery(m_strID, strQuery);
	//if(CChatSession::Instance().RequestRawSQL(RAWSQL_LOGIN, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	if(CChatSession::Instance().RequestLogIn(m_strID) != TRUE)
	{
		m_StaMsg.SetWindowText("");				
		RedrawWindow();

		m_BtnLogIn.EnableWindow(TRUE);

		//writeLogFile("c:\\ClientLog.log", "OnLogInAllowed Fail!\n");	

		return FALSE;
	}

	//writeLogFile("c:\\ClientLog.log", "OnLogInAllowed 성공!\n");	

	return TRUE;
}

//작업일지 작성 사용자인지..
LRESULT CLogInDlg::OnJobGnRslt(WPARAM wParam, LPARAM lParam)
{
	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_JOBGN* pData = static_cast<S_JOBGN*>(pArray->GetAt(i));
					
		if( pData->strJobGn.Compare("Y") != 0)
		{

			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_JOBGN *pData = static_cast<S_JOBGN*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;
			
			//공통함수로!!
			char tmpStr[255];
			memset(tmpStr, 0x00, sizeof(tmpStr));
			sprintf(tmpStr, "   %s 님 환영합니다!!", (LPCSTR)m_LoginName) ;
			CWnd* pMain = AfxGetMainWnd();
			CMsgBoxThread* pMsgThread = new CMsgBoxThread( pMain,tmpStr , 1 );	
			
			pMsgThread->CreateThread();
			
			
			m_Rslt = TRUE;
			CDialog::OnOK();
			
			return TRUE;
		}
		
		// Clear	
		for(int j = 0; j < pArray->GetSize(); j++)
		{
			S_JOBGN *pData = static_cast<S_JOBGN*>(pArray->GetAt(j));
			if(pData != NULL)
			{
				delete pData;
				pData = NULL;
			}
		}
		delete pArray;	
		pArray = NULL;
		
		CString strDate = GetGhostToday();			
		CString strQuery;
		GetQuerySelectDailyjobMst(m_LoginID, strDate, strQuery);
		
		TRACE("작업일지를 작성하는 사용자임\n");
		CChatSession::Instance().m_bDailyJob = TRUE;

		if(CChatSession::Instance().RequestRawSQL(RAWSQL_DAILYJOB_MST, (LPCSTR)strQuery, strQuery.GetLength() )  != TRUE)
		{
			m_StaMsg.SetWindowText("");				
			RedrawWindow();

			m_BtnLogIn.EnableWindow(TRUE);
			return FALSE;
		}	
		
		return TRUE;			
		
	}
	return TRUE;
}


LRESULT CLogInDlg::OnDailyJobMstRslt(WPARAM wParam, LPARAM lParam)
{
	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		S_DAILYJOB_MST* pData = static_cast<S_DAILYJOB_MST*>(pArray->GetAt(i));
				
		if( pData->strLogOnTime.GetLength() > 0)
		{
			// 로그인시간이 있으면 다시 저장하지 않는다.
			//CString strQuery = GetQueryUpdateLogonTime(m_LoginID, strDate);
			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_DAILYJOB_MST *pData = static_cast<S_DAILYJOB_MST*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;
			
			//공통함수로!!
			char tmpStr[255];
			memset(tmpStr, 0x00, sizeof(tmpStr));
			sprintf(tmpStr, "   %s 님 환영합니다!!", (LPCSTR)m_LoginName) ;
			
			CWnd* pMain = AfxGetMainWnd();
			CMsgBoxThread* pMsgThread = new CMsgBoxThread( pMain,tmpStr , 1 );				
			
			pMsgThread->CreateThread();
			
			UpdateData(FALSE);
			
			m_Rslt = TRUE;
			CDialog::OnOK();
			
			
			return TRUE;
		}
		else if( pData->strJobDay.GetLength() > 0)
		{
			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_DAILYJOB_MST *pData = static_cast<S_DAILYJOB_MST*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;
			
			CString strDate = GetGhostToday();					
			CString strQuery;

			
			//GetQueryInsertLogonTime(m_LoginID, strDate,strQuery);
			GetQueryUpdateLogonTime(m_LoginID, strDate,strQuery); //logon time null이지만 row존재 
			if(CChatSession::Instance().RequestRawSQL(RAWSQL_INSERT_LOGONTIME, (LPCSTR)strQuery, strQuery.GetLength() )  != TRUE)
			{
				m_StaMsg.SetWindowText("");				
				RedrawWindow();
				m_BtnLogIn.EnableWindow(TRUE);
				return FALSE;
			}	
			return TRUE;
		}
		else
		{
			// Clear	
			for(int j = 0; j < pArray->GetSize(); j++)
			{
				S_DAILYJOB_MST *pData = static_cast<S_DAILYJOB_MST*>(pArray->GetAt(j));
				if(pData != NULL)
				{
					delete pData;
					pData = NULL;
				}
			}
			delete pArray;	
			pArray = NULL;
			
			CString strDate = GetGhostToday();					
			CString strQuery;

			
			GetQueryInsertLogonTime(m_LoginID, strDate,strQuery);
			//GetQueryUpdateLogonTime(m_LoginID, strDate,strQuery); //logon time null이지만 row존재 
			if(CChatSession::Instance().RequestRawSQL(RAWSQL_INSERT_LOGONTIME, (LPCSTR)strQuery, strQuery.GetLength() )  != TRUE)
			{
				m_StaMsg.SetWindowText("");				
				RedrawWindow();
				m_BtnLogIn.EnableWindow(TRUE);
				return FALSE;
			}	
			return TRUE;
		}
	}
	
	return TRUE;
}



LRESULT CLogInDlg::OnInsertLogOnTmRslt(WPARAM wParam, LPARAM lParam)
{
	S_COM_RESPONSE * pComRslt = (S_COM_RESPONSE*) lParam;
	CSP_Utility<S_COM_RESPONSE> spTest = pComRslt;

	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		char tmpStr[255];
		memset(tmpStr, 0x00, sizeof(tmpStr));
		sprintf(tmpStr, "   %s 님 환영합니다!!", (LPCSTR)m_LoginName) ;

		CWnd* pMain = AfxGetMainWnd();
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(pMain, tmpStr , 1 );	
		
		pMsgThread->CreateThread();
								
		UpdateData(FALSE);

		m_Rslt = TRUE;
		CDialog::OnOK();

		return TRUE;		
	}
	else
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this, pComRslt->szErrMsg );	
		
		pMsgThread->CreateThread();

		m_StaMsg.SetWindowText("");				
		RedrawWindow();
		m_BtnLogIn.EnableWindow(TRUE);

		return FALSE;
	}			

	return TRUE;
}

LRESULT CLogInDlg::OnSaveLastIPRslt(WPARAM wParam, LPARAM lParam)
{
	S_COM_RESPONSE * pComRslt = (S_COM_RESPONSE*) lParam;
	CSP_Utility<S_COM_RESPONSE> spTest = pComRslt;

	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		TRACEX("작업일지를 작성하는 사용자인지 확인!\n");
		CString strQuery; 
		GetQueryJobWrite(m_LoginID, strQuery);

		if(CChatSession::Instance().RequestRawSQL(RAWSQL_JOB_GN, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
		{
			m_StaMsg.SetWindowText("");				
			RedrawWindow();

			m_BtnLogIn.EnableWindow(TRUE);
			return FALSE;
		}	

		return TRUE;		
	}
	else
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this,pComRslt->szErrMsg );
		
		pMsgThread->CreateThread();

		m_StaMsg.SetWindowText("");				
		RedrawWindow();

		m_BtnLogIn.EnableWindow(TRUE);

		return FALSE;
	}

	return TRUE;
}

void CLogInDlg::GetLoginQuery(const CString& strUserId, CString & strQuery)
{	
	strQuery.Format("SELECT	DECODE(A.COMPANY_POS,'02','KT','NoKT') KT_GN \
,A.USER_NAME USER_NAME,A.PASSWD PASSWD,B.CODE TEAM_GN,B.CODE_NAME TEAM_NAME	\
,A.USER_GRADE USER_GRADE,D.CODE TOP_TEAM_GN,D.CODE_NAME TOP_TEAM_NAME \
,A.STATUS STATUS,A.NICK_NAME ,A.IAMS_AUTHORITY IAMS_AUTH, A.PROG_SELECT PROG_SELECT	\
FROM KM_DOCUSER A,KM_IDMS_CODE B,KM_CODE_REL C ,KM_IDMS_CODE D \
WHERE A.USER_ID='%s' AND A.TEAM_GN=B.CODE(+) AND B.GRP_CODE(+)='1001' \
AND B.GRP_CODE=C.REL_GRP_CODE(+) AND B.CODE=C.REL_CODE(+) \
AND C.GRP_CODE(+)='6008' AND C.GRP_CODE = D.GRP_CODE(+) AND C.CODE = D.CODE(+)", strUserId);	

	//AfxMessageBox(strQuery);
	TRACEX("Length [%d]\n", strQuery.GetLength());	
}

void CLogInDlg::GetQueryUpdateIp(const CString& strLoginId, const CString& strIp, CString& strOut)
{
	strOut.Format("UPDATE KM_DOCUSER SET IP_ADDR='%s' WHERE USER_ID='%s'", strIp, strLoginId) ;	
}


void CLogInDlg::GetQuerySelectDailyjobMst( const CString& strLoginId, const CString& strDate,CString& strQuery)
{
	//strQuery.Format("SELECT JOB_DAY,DECODE(NVL(LOGON_TIME,''),'','',TO_CHAR(LOGON_TIME,'YYYYMMDDHH24MISS')) LOGON_TIME,DECODE(NVL(LOGOFF_TIME,''),'','',TO_CHAR(LOGOFF_TIME,'YYYYMMDDHH24MISS')) LOGOFF_TIME FROM KM_DAILYJOBMST WHERE USER_ID='%s' AND JOB_DAY='%s'", strLoginId, strDate);	

	strQuery.Format("SELECT JOB_DAY,TO_CHAR(LOGON_TIME,'YYYYMMDDHH24MISS'),TO_CHAR(LOGOFF_TIME,'YYYYMMDDHH24MISS') FROM KM_DAILYJOBMST WHERE USER_ID='%s' AND JOB_DAY='%s'", strLoginId, strDate);	
	

	TRACEX("Length [%d]\n", strQuery.GetLength());	
}

void CLogInDlg::GetQueryUpdateLogonTime(const CString& strLoginId, const CString& strDate, CString& strQuery)
{
	strQuery.Format("UPDATE KM_DAILYJOBMST SET LOGON_TIME=SYSDATE WHERE USER_ID='%s' AND JOB_DAY='%s'", strLoginId, strDate) ;
	
}

void CLogInDlg::GetQueryInsertLogonTime(const CString& strLoginId, const CString& strDate, CString& strQuery)
{	
	strQuery.Format("INSERT INTO KM_DAILYJOBMST(USER_ID, JOB_DAY, LOGON_TIME) VALUES('%s','%s',SYSDATE)", strLoginId, strDate);	
}
*/
/*
BOOL CLogInDlg::SaveLogoffTime()
{


	CString			strQuery;
	CString			strDate;
	TCHAR			szJobWriteGn[2];

	pOraObj = COraObjOleDB::Instance();
	// 작업일지를 작성하는 사용자인지 확인한다.
	strQuery = GetQueryJobWrite(m_LoginID);
	if( pOraObj->SelectSQLExecute(strQuery) != OSUCCESS )
	{
		DisplayError(pOraObj);

		return FALSE;
	}
	memset(szJobWriteGn, '\0', sizeof(szJobWriteGn));
	pOraObj->m_Dynaset.GetFieldValue("JOB_WRITE_GN", szJobWriteGn, sizeof(szJobWriteGn));
	if( pOraObj->m_Dynaset.GetRecordCount() <= 0 )
		return FALSE;

	if( strcmp(szJobWriteGn, "Y") != 0 )
		return TRUE;

	strDate = GetGhostToday();
	GetQuerySelectDailyjobMst(m_LoginID, strDate,strQuery);
	if( pOraObj->SelectSQLExecute(strQuery) != OSUCCESS )
	{
		DisplayError(pOraObj);

		return FALSE;
	}
  strDate = GetGhostToday();
  GetQueryUpdateLogoffTime(m_LoginID, strDate,strQuery);
	if( pOraObj->m_Dynaset.GetRecordCount() > 0 )
		GetQueryUpdateLogoffTime(m_LoginID, strDate,strQuery);
	else
		GetQueryInsertLogoffTime(m_LoginID, strDate,strQuery);
	if( pOraObj->SQLExecute(strQuery) != OSUCCESS)
	{
		DisplayError(pOraObj);
		
		return FALSE;
	}

	return TRUE;
}
*/

/*
LRESULT CLogInDlg::OnAlreadyLogIn(WPARAM wParam, LPARAM lParam)
{	
	m_strID = "";
	m_strPwd = "";
	UpdateData(FALSE);

	GetDlgItem(IDC_ED_ID)->SetFocus();

	CMsgBoxThread* pMsgThread = new CMsgBoxThread(this, "이미 로그인한 사용자입니다. 다른 ID 로 로그인 하세요" );
	
	pMsgThread->CreateThread();	
	
	m_StaMsg.SetWindowText("");				
	RedrawWindow();
	UpdateData(FALSE);
	m_BtnLogIn.EnableWindow(TRUE);
	
	return FALSE;
}
*/


//void CLogInDlg::GetQueryInsertLogoffTime(const CString& strLoginId, const CString& strDate, CString& strQuery)
//{	
//	strQuery.Format("INSERT INTO KM_DAILYJOBMST (USER_ID,JOB_DAY,LOGOFF_TIME) VALUES('%s','%s',SYSDATE)",strLoginId,strDate);	
//}

/*
void CLogInDlg::GetQueryJobWrite(const CString& strLoginId, CString& strOut)
{
	strOut.Format("SELECT USER_ID,JOB_WRITE_GN FROM KM_DOCUSER WHERE USER_ID='%s'" , strLoginId ) ;	
}
*/