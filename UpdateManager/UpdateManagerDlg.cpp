// DocManagerUpdateDlg.cpp : implementation file
//


//-----------------------------------------------------------------------//
// Update_Date : 2005.10.19 BY LSH   (KEY WORD : FILE_ATTRIBUTE)
//  1.Update ��� ������  ReadOnly ||| Hidden �Ӽ� ������� �Ӽ� ���� 
// Update_Date : 2006.04.27 BY LSH   (KEY WORD : SET_CONNECT_TIMEOUT)
//  1.FTP ����� Time Out (10��) ����   
// Update_Date : 2006.08.28 BY LSH   (KEY WORD : GET_TNSFILE)
//  1. Tnsnames.ora ���� path ���ϴ� ���� ���� 
// Update_Date : 2006.08.29 BY LSH   (KEY WORD : IDMS_BILL)
//  1. Server ���� ������ �ִ� INI File �߰� 
// Update_Date : 2006.10.27 BY LSH   (KEY WORD : META_DATA)
//  1. IMSS DB Connect String Set Write(tnsnames.ora)
// Update_Date	: 2007.09.12 BY KSY	 (KEY WORD : NEW_IDMS)
//	1. 147.6.184.87(SICC) -> 147.6.184.89(IDMS00) ����

// 20090531 �������� �� ������ �����ϰ��Ѵ�. ���� ���ӽ����ϸ� ����ó�� DB�� �����´�.
// => 20090531 �� �˻�.. 
//-----------------------------------------------------------------------// 


#include "stdafx.h"
#include "UpdateManager.h"
#include "UpdateManagerDlg.h"
#include "DBWrapper.h"
#include "SelectServer.h"								// IDMS_BILL
#include "INIManager.h"    // 20090531 
#include  <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDMS_GLOBAL_REG_SETTINGS		_T("All Settings")
#define IDMS_GLOBAL_REG_INSTALLDIR		_T("INSTALL_DIR")
#define IDMS_GLOBAL_REG_FTPIP			_T("FTP_IP")
#define IDMS_GLOBAL_REG_UPDATEDATE		_T("UPDATE_DATE")
#define IDME_CONNECT_TIME_OUT           10*1000
////////////////////////////////////////////////////////////////////////////////////////
// IDMS_BILL
#define CUSTOM_ALIAS				_T("IDMS00"			)
#define CUSTOM_ALIAS_WORLD			_T("IDMS00.WORLD"	)

#define BILL_ALIAS					_T("BILCSS40"      )
#define BILL_ALIAS_WORLD			_T("BILCSS40.WORLD")
// IDMS_BILL
////////////////////////////////////////////////////////////////////////////////////////
#define OPERA_ALIAS					_T("OPERA10"      )				// META_DATA
#define OPERA_ALIAS_WORLD			_T("OPERA10.WORLD")				// META_DATA	
/////////////////////////////////////////////////////////////////////////////
#define INI_FTP_SEC			_T("FTP SECTION") 
#define INI_FTP_IP			_T("UPDATE FTP_IP") 
#define INI_FTP_ID			_T("UPDATE FTP_ID") 
#define INI_FTP_PW			_T("UPDATE FTP_PW") 
#define INI_FTP_DIR			_T("UPDATE FTP_DIR") 
#define INI_MSNSVR_SEC		_T("MSNSVR SECTION") 
#define INI_MSNSVR_IP		_T("MSN SERVER IP") 
#define INI_MSNSVR_PORT		_T("MSN SERVER PORT") 

// CUpdateManagerDlg dialog

CUpdateManagerDlg::CUpdateManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateManagerDlg::IDD, pParent)
{
Trace("CUpdateManagerDlg");
	//{{AFX_DATA_INIT(CUpdateManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pSession	= NULL;
	m_pFtpCon	= NULL;
	m_strFtpIp	= _T("");
	m_strFtpId	= _T("");
	m_strFtpPw	= _T("");
	m_dwOption	= _OPTION_NONE;
	m_mapFileList.RemoveAll();
}

CUpdateManagerDlg::~CUpdateManagerDlg()
{
Trace("~CUpdateManagerDlg");
}

void CUpdateManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateManagerDlg)
	DDX_Control(pDX, ID_ANIMATE, m_Animate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CUpdateManagerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerDlg message handlers

BOOL CUpdateManagerDlg::OnInitDialog()
{
Trace("OnInitDialog");
	CDialog::OnInitDialog();

	m_Animate.Open(IDR_AVI);
	m_Animate.Play(0, -1, -1);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// ����Ӽ� Ȯ��
	if( m_dwOption & _OPTION_HIDE )
		ShowWindow(SW_HIDE);
	else
		ShowWindow(SW_SHOWNORMAL);

	UpdateWindow();

	////////////////////////////////////////////////////////////////////////////////////////
	//// IDMS_BILL
	m_strSetServer = GetServerFlag();	
//m_strSetServer = IDMS_CUSTOM;

	if(m_strSetServer.IsEmpty())
	{
		AfxMessageBox("������ ������ ��Ȯġ �ʽ��ϴ�. �����ڿ��� ���� �ϼ���.");
		return FALSE;
	}

	SetConnectInfo(m_strSetServer);
	////////////////////////////////////////////////////////////////////////////////////////
	// 20090531 
	CINIManager::Instance().SetIniFile( "UpdateManager" /* module*/, "idmsips.ini" /* ini file*/);
	CINIManager::Instance().SetKeySection( CString(INI_FTP_IP), CString(INI_FTP_SEC) );
	CINIManager::Instance().SetKeySection( CString(INI_FTP_ID), CString(INI_FTP_SEC) );
	CINIManager::Instance().SetKeySection( CString(INI_FTP_PW), CString(INI_FTP_SEC) );
	CINIManager::Instance().SetKeySection( CString(INI_FTP_DIR), CString(INI_FTP_SEC) );
	//CINIManager::Instance().SetKeySection( CString(INI_MSNSVR_IP), CString(INI_MSNSVR_SEC) );
	//CINIManager::Instance().SetKeySection( CString(INI_MSNSVR_PORT), CString(INI_MSNSVR_SEC) );
	
	CINIManager::Instance().LoadINIFile();
		
	CString strval;
	CINIManager::Instance().GetValue(INI_FTP_IP, m_strFtpIp);
	CINIManager::Instance().GetValue(INI_FTP_ID, m_strFtpId);
	CINIManager::Instance().GetValue(INI_FTP_PW, m_strFtpPw);
	CINIManager::Instance().GetValue(INI_FTP_DIR, m_strFtpDir);
	// 20090531 
						
	if( DoUpdate() == FALSE )
	{		
		// 20090531  �������Ͽ��� ���� => DB ���� �����´�
		SetDlgItemText(ID_ST_COMMENT, _T("DB���� ������Ʈ ���� ������ �����ɴϴ�...."));
		if( GetServerInfo() == FALSE )
		{
			SetDlgItemText(ID_ST_COMMENT, _T("DB ��ȸ ����...."));

			m_Animate.Close();		
			return FALSE;
		}
		
		// 20090531  : DB���� ���� ������ �ٽ� �õ� 
		SetDlgItemText(ID_ST_COMMENT, _T("DB ��ȸ ���� ������ �ٽ� ������Ʈ �õ��մϴ�...."));
		if( DoUpdate() == FALSE )
		{
			SetDlgItemText(ID_ST_COMMENT, _T("DB ��ȸ ���� ������ �ٽ� ������Ʈ �����Ͽ����ϴ�."));

			CDialog::OnCancel();
			return FALSE;
		}		
	}

	// â ����
	EndDialog(0);
	// DocManager ����
	CString strInstallDir;			// ��ġ���丮

	strInstallDir = AfxGetApp()->GetProfileString(IDMS_GLOBAL_REG_SETTINGS, IDMS_GLOBAL_REG_INSTALLDIR);
	if( strInstallDir.IsEmpty() )
		strInstallDir = _T("C:\\Program Files\\IDMS\\");
	else if( strInstallDir.Right(1) != _T("\\") )
		strInstallDir += _T("\\");
	strInstallDir += "Bin\\IdmsMsn.exe";
	ShellExecute(NULL, "open", strInstallDir, NULL, NULL, SW_SHOWDEFAULT);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CUpdateManagerDlg::FileUpdate(CString& strInstallDir)
{
Trace("FileUpdate");
	POSITION	pos;
	CString		strFileName;
	CString		strFilePath;
	CString		strFile;
	CString		strFileExp;
	BOOL		bResult;
	
	CString	    strTargetFile =_T("");
	DWORD		dFileAttribute;						// FILE_ATTRIBUTE

	
	pos = m_mapFileList.GetStartPosition();
	while( pos != NULL )
	{
		m_mapFileList.GetNextAssoc(pos, strFileName, strFilePath);

		SetDlgItemText(ID_ST_COMMENT, "ȭ�ϸ� : " + strFileName);

		if( strFileName.GetLength() > 3 )
			strFileExp = strFileName.Right(3);
		else
			strFileExp = _T("");

		if( strFilePath.Right(1) != _T("/") )
			strFilePath += _T("/");

		strFile.Format(_T("%s%s"), strFilePath, strFileName);

		///////////////////////////////////////////////////////////////////////////////////////
		//// FILE_ATTRIBUTE
		
		strTargetFile=strInstallDir;
		if     ( stricmp(strFileExp, _T("rpt")) == 0 )		strTargetFile+="Report\\";
		else if( stricmp(strFileExp, _T("mdb")) == 0 )		strTargetFile+="Dat\\"   ; 
		else if( stricmp(strFileExp, _T("txt")) == 0 )		strTargetFile+="Update\\"; 
		else if( stricmp(strFileExp, _T("ini")) == 0 )		strTargetFile+="Ini\\"	 ;			// IDMS_BILL
		else												strTargetFile+="Bin\\"   ; 

		strTargetFile+=	strFileName;	
		
		dFileAttribute = GetFileAttributes(strTargetFile);
		if(dFileAttribute!=-1)									// ���� ����. 
		{
			if((dFileAttribute & FILE_ATTRIBUTE_READONLY) || (dFileAttribute & FILE_ATTRIBUTE_HIDDEN) )
			{
				SetFileAttributes(strTargetFile,FILE_ATTRIBUTE_ARCHIVE);
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////
		
		bResult = m_pFtpCon->GetFile(strFile, strTargetFile, FALSE);
		if( bResult == FALSE )
		{
			LPVOID lpMsgBuf;
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						   FORMAT_MESSAGE_FROM_SYSTEM | 
						   FORMAT_MESSAGE_IGNORE_INSERTS,
						   NULL,
						   GetLastError(),
						   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						   (LPTSTR) &lpMsgBuf,
						   0,
						   NULL 
						 );

			// Display the string.
			AfxMessageBox((LPCTSTR)lpMsgBuf, MB_OK | MB_ICONINFORMATION);

			// Free the buffer.
			LocalFree( lpMsgBuf );

			return FALSE;
		}
	}

	return TRUE;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdateManagerDlg::OnPaint() 
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
HCURSOR CUpdateManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CUpdateManagerDlg::OpenFtpSession(CString& strUrl, CString& strId, CString& strPw)
{
Trace("OpenFtpSession");
	if( m_pSession == NULL )
		m_pSession = new CInternetSession();
	
	///////////////////////////////////////////////////////////////
	// SET_CONNECT_TIMEOUT
 	m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, IDME_CONNECT_TIME_OUT);

	TRY
	{
		m_pFtpCon = m_pSession->GetFtpConnection(strUrl, strId, strPw);
	}
	CATCH(CInternetException , pEx)
	{
		//pEx->ReportError();
		if( m_pFtpCon != NULL ) delete m_pFtpCon;
		m_pFtpCon = NULL;
		return FALSE;
	}
	END_CATCH

	if( m_pFtpCon == NULL )
		return FALSE;
	
	return TRUE;
}

void CUpdateManagerDlg::CloseFtpSession(BOOL bExitSession/*=FALSE*/)
{
Trace("CloseFtpSession");
	if( m_pFtpCon != NULL ){
		m_pFtpCon->Close();
		delete m_pFtpCon;
		m_pFtpCon = NULL;
	}	

	if( bExitSession == TRUE )
	{
		if( m_pSession != NULL ){
			m_pSession->Close();
			delete m_pSession;
			m_pSession = NULL;
		}
	}
}

BOOL CUpdateManagerDlg::FindUpdateFile(CString& strUpdateDate)
{
	if( m_strFtpDir.Right(1) != _T("/") )
	{
		m_strFtpDir += _T("/");
	}

	if( m_pFtpCon->SetCurrentDirectory(m_strFtpDir) == FALSE )
	{
		return FALSE;
	}

	CFtpFileFind*	pff;
	BOOL			bFlag = FALSE;
	CString			strPath;


	pff = new CFtpFileFind(m_pFtpCon);

	bFlag = pff->FindFile(_T("20*"));

	while( bFlag )
	{
		bFlag = pff->FindNextFile();

		if( pff->IsDots() || pff->IsDirectory() == FALSE )	continue;
		strPath = pff->GetFileName();

		if( strUpdateDate < strPath )
		{
			// ������Ʈ�� ������ ������� �ȿ� �ִ� ������ �˻�
			if( CheckUpdateFile(pff->GetFilePath()) == FALSE )
			{
				// �˻��� �����ϸ� ������Ʈ ����
				pff->Close();
				delete pff;
				return FALSE;
			}
			m_strUpPath = strPath;						// IDMS_BILL
		}
	}

	pff->Close();
	delete pff;	

	return TRUE;
}

BOOL CUpdateManagerDlg::CheckUpdateFile(CString strPath)
{
Trace("CheckUpdateFile");
	CInternetSession*	pSession	= NULL;
	CFtpConnection*		pFtpCon		= NULL;
	CFtpFileFind*		pff			= NULL;

	CString				strFilter;
	CString				strFile;
	CString				strData;
	BOOL				bFlag;

	TRY
	{
		pSession = new CInternetSession();
		pFtpCon = m_pSession->GetFtpConnection(m_strFtpIp, m_strFtpId, m_strFtpPw);
	}
	CATCH(CInternetException , pEx)
	{
		//pEx->ReportError();
		if( pFtpCon != NULL )	delete pFtpCon;
		if( pSession != NULL)	delete pSession;

		return FALSE;
	}
	END_CATCH

	if( pFtpCon == NULL )
		return FALSE;

	if( pFtpCon->SetCurrentDirectory(strPath) == FALSE )
	{
		if( pFtpCon != NULL )	delete pFtpCon;
		if( pSession != NULL)	delete pSession;
		return FALSE;
	}
	
	pff = new CFtpFileFind(pFtpCon);

	strFilter.Format(_T("%s/*.*"), strPath);

	bFlag = pff->FindFile(strFilter);

	if( bFlag == FALSE )
	{
		if( pff != NULL)		delete pff;
		if( pFtpCon != NULL )	delete pFtpCon;
		if( pSession != NULL)	delete pSession;

		return FALSE;
	}

	while( bFlag )
	{
		bFlag = pff->FindNextFile();
		// ���������� �˻����� �ʴ´�.
		if( pff->IsDots() || pff->IsDirectory() )
			continue;

		strFile = pff->GetFileName();
		// ������ ���� �����̰ų� ������������ �űԸ� ����
		if( m_mapFileList.Lookup(strFile, strData) == FALSE )
			m_mapFileList.SetAt(strFile, strPath);
		else if( strData < strPath )
			m_mapFileList.SetAt(strFile, strPath);
	}

	pff->Close();
	delete pff;
	if( pFtpCon != NULL )	delete pFtpCon;
	if( pSession != NULL)	delete pSession;

	return TRUE;
}

BOOL CUpdateManagerDlg::CheckInstDir(CString strDir)
{
Trace("CheckInstDir");
	CFileFind	ff;
	CString		strFilter;
	CString		strName;
	BOOL		bResult;
	BOOL		bReport;			// ����Ʈ����
	BOOL		bDat;				// ����������
	BOOL		bUpdate;			// ������Ʈ ���
	BOOL		bBin;				// �������� �� DLL ��
	BOOL		bIni;				// Ini ����				//IDMS_BILL

	if( strDir.Right(1) != _T("\\") )
		strDir += _T("\\");
	strFilter.Format(_T("%s*.*"), strDir);

	bResult = ff.FindFile(strFilter);
	if( bResult == FALSE )
		return FALSE;

	bReport = bDat = bUpdate = bBin = FALSE;
	bIni=FALSE;														//IDMS_BILL
	while( bResult )
	{
		bResult = ff.FindNextFile();

		if( ff.IsDots() || ff.IsDirectory() == FALSE )
			continue;

		strName = ff.GetFileName();
		if     ( stricmp(strName, _T("Report")) == 0 )		bReport = TRUE;
		else if( stricmp(strName, _T("Dat")   ) == 0 )		bDat    = TRUE;
		else if( stricmp(strName, _T("Update")) == 0 )		bUpdate = TRUE;
		else if( stricmp(strName, _T("Bin")   ) == 0 )		bBin    = TRUE;
		else if( stricmp(strName, _T("Ini")   ) == 0 )		bIni    = TRUE;				 //IDMS_BILL
	}
	// Report ���� ����
	if( bReport == FALSE )
	{
		if( CreateDirectory(strDir + _T("Report"), NULL) == FALSE )
			return FALSE;
	}
	// Dat ���� ����
	if( bDat == FALSE )
	{
		if( CreateDirectory(strDir + _T("Dat"),    NULL) == FALSE )
			return FALSE;
	}
	// Update ���� ����
	if( bUpdate == FALSE )
	{
		if( CreateDirectory(strDir + _T("Update"), NULL) == FALSE )
			return FALSE;
	}
	// Bin ���� ����
	if( bBin == FALSE )
	{
		if( CreateDirectory(strDir + _T("Bin"),    NULL) == FALSE )
			return FALSE;
	}
	// Ini ���� ����													//IDMS_BILL
	if( bIni == FALSE )													//IDMS_BILL
	{																	//IDMS_BILL
		if( CreateDirectory(strDir + _T("Ini"),    NULL) == FALSE )		//IDMS_BILL
			return FALSE;												//IDMS_BILL
	}																	//IDMS_BILL

	return TRUE;
}

void GetLocalIP(CString& strIP)
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

BOOL CUpdateManagerDlg::GetServerInfo()
{	
	
	CWaitCursor Wait;	
	CDBWrapper	db;
	CString		strQuery;
	CString		strData;
			
	BOOL bRtn = COraObjOleDB::Instance(). DBConnect(m_strSID, m_strUser, m_strPW);		// IDMS_BILL
	
	if(!bRtn)
	{
		CString strMsg=_T("");
		const char* pErrMsg;

		pErrMsg = COraObjOleDB::Instance().m_Database.GetErrorText();
		strMsg.Format("%s",pErrMsg);
		strMsg+="\r\n\r\n";
		strMsg+="�����ڿ��� ���� �Ͻʽÿ�.";
		
		AfxMessageBox(strMsg);

		return FALSE;
	}
	
	// run query
	strQuery =	"SELECT CODE, CODE_NAME	 FROM KM_IDMS_CODE	WHERE GRP_CODE = '6006'	 AND END_DATE > SYSDATE ";

	if( db.RunSql(strQuery) == FALSE )
	{
		m_Animate.Close();
		CDialog::OnCancel();

		CString strMsg=_T("");
		const char* pErrMsg;

		pErrMsg = COraObjOleDB::Instance().m_Database.GetErrorText();
		strMsg.Format("%s",pErrMsg);
		strMsg+="\r\n\r\n";
		strMsg+="�����ڿ��� ���� �Ͻʽÿ�.";
		
		AfxMessageBox(strMsg);

		return FALSE;
	}
	
	while( db.m_pRecordSet->IsEOF() == FALSE )
	{
		strData = db.GetData(0);

		if     ( strData == _T("01") )	m_strFtpIp  = db.GetData(1);
		else if( strData == _T("02") )	m_strFtpId  = db.GetData(1);
		else if( strData == _T("03") )	m_strFtpPw  = db.GetData(1);
		else if( strData == _T("04") )	m_strFtpDir = db.GetData(1);

		db.m_pRecordSet->MoveNext();
	}

			
	CINIManager::Instance().SetValue(INI_FTP_IP		, m_strFtpIp	);
	CINIManager::Instance().SetValue(INI_FTP_ID		, m_strFtpId	);
	CINIManager::Instance().SetValue(INI_FTP_PW		, m_strFtpPw	);
	CINIManager::Instance().SetValue(INI_FTP_DIR	, m_strFtpDir	);
	
	CINIManager::Instance().SaveINIFile();	

	
	// ������Ʈ ������ ��������� ǥ��!!
	
// 	CString strIP ;
// 	GetLocalIP(strIP) ;
// 	strQuery.Format ("UPDATE not_updated SET UPDATED ='Y' WHERE IP_ADDR = '%s' ", strIP);
// 
// 	if( db.RunSql(strQuery) == FALSE )
// 	{
// 		m_Animate.Close();
// 		CDialog::OnCancel();
// 
// 		return FALSE;
// 	}
	
	
	db.Close();	
	
		
	return TRUE;
}

BOOL CUpdateManagerDlg::DoUpdate()
{
Trace("DoUpdate");
	
	int			nResult = 0;			    // IDMS_BILL		
	int			nRCnt   = 0;
	CTime		ctDate;

	CString		strDate			=_T("");
	CString		strInstallDir	=_T("");	// ��ġ���丮
	CString		strUpdateDate	=_T("");	// �ֱپ�����Ʈ��¥

	CheckServer();
	
	strInstallDir = AfxGetApp()->GetProfileString(IDMS_GLOBAL_REG_SETTINGS,
												  IDMS_GLOBAL_REG_INSTALLDIR);
	if( strInstallDir.IsEmpty() )
	{
		char szPath[MAX_PATH];

		memset(szPath, 0, MAX_PATH);
		GetCurrentDirectory(MAX_PATH, szPath);
		strInstallDir = szPath;
		strInstallDir.Delete(strInstallDir.ReverseFind(_T('\\')),
							 strInstallDir.GetLength() - strInstallDir.ReverseFind(_T('\\')));
	}

	if( strInstallDir.Right(1) != _T("\\") )		
		strInstallDir += _T("\\");
	
	// ��ġ�� ���丮�� ������ üũ
	if( CheckInstDir(strInstallDir) == FALSE )
	{
		SetDlgItemText(ID_ST_COMMENT, _T("��ġ ��� ���� ����"));
		Sleep(500);
		nResult = IDCANCEL;
		
		return TRUE;
	}

	// FTP ����		
	SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ������ ������...."));

	if( OpenFtpSession(m_strFtpIp, m_strFtpId, m_strFtpPw) == TRUE )
	{
		SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ������ ����Ǿ����ϴ�...."));

		strUpdateDate    =_T("");			
		m_strUpPath      =_T("");									// IDMS_BILL

		SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ȭ�ϰ˻���...."));
		strUpdateDate = AfxGetApp()->GetProfileString(IDMS_GLOBAL_REG_SETTINGS,
													  IDMS_GLOBAL_REG_UPDATEDATE);

		// �¾��ѳ�¥�ϰ��
		if( strUpdateDate.Find('-') != -1 )
		{
			int nIndex = strUpdateDate.Find('-');
			int nMon   = atoi(strUpdateDate.Left(nIndex));

			strUpdateDate = strUpdateDate.Mid(nIndex+1);
			nIndex		  = strUpdateDate.Find('-');

			int nDay      = atoi(strUpdateDate.Left(nIndex));
			int nYear     = atoi(strUpdateDate.Mid(nIndex+1));

			strUpdateDate.Format("%4d%02d%02d00", nYear, nMon, nDay);
		}
		
		// �ֽž�����Ʈ ����ã��
		
		if( FindUpdateFile(strUpdateDate) == FALSE )
		{
			SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ����Ʈ �ۼ� ����"));
			Sleep(500);	
			
			nResult = IDCANCEL; //20090531
			return FALSE;       //20090531
		}
		else if( m_mapFileList.GetCount() > 0 )
		{
			if( FileUpdate(strInstallDir) == FALSE )	
			{
				SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ����!!"));
			}
			else
			{				
				ctDate = CTime::GetCurrentTime();
				
				if(!m_strUpPath.IsEmpty())	strDate = m_strUpPath+"00";	
				else						strDate = ctDate.Format(_T("%Y%m%d%H%M%S"));								
								
				AfxGetApp()->WriteProfileString(IDMS_GLOBAL_REG_SETTINGS,
												IDMS_GLOBAL_REG_UPDATEDATE,
												strDate);

				SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ �Ϸ�!!\n��ø� ��ٸ�����..."));						
			} 
		}
		else
		{
			SetDlgItemText(ID_ST_COMMENT, _T("������Ʈ ������"));			
		}
		
		nResult = IDOK;

		// FTP ����
		CloseFtpSession(TRUE);		
		//m_Animate.Close();

		return TRUE;
	}
	else
	{
		SetDlgItemText(ID_ST_COMMENT, _T("FTP���� �������"));
		Sleep(500);
		nResult = IDCANCEL;

		return FALSE;
	}
	
}


BOOL CUpdateManagerDlg::CheckServer()
{
Trace("CheckServer");
	CFile	file;
	DWORD	dwLen;
	int		nPos;
	TCHAR*	pBuf;

	CString	strFile	   =_T("");

	CString	strOraHome =_T("");
	CString	strData	   =_T("");
	CString strChkSid  =_T("");										// IDMS_BILL

	BOOL    bRealWorld = FALSE;										// IDMS_BILL		
	BOOL	bReal      = FALSE;										// IDMS_BILL	
	BOOL	bCcrcss10  = FALSE;										// IDMS_BILL
	
	BOOL    bOpera     = FALSE;										// META_DATA
	BOOL    bOperaWorld= FALSE;										// META_DATA	
	
	strFile = GetDefaultTNSNameFilePath();							// GET_TNSFILE

	if( file.Open(strFile, CFile::modeReadWrite) == FALSE )		return FALSE;

	dwLen = file.GetLength();
	if( dwLen > 0 )
	{
		pBuf = new TCHAR[dwLen+1];
		memset(pBuf, NULL, dwLen);

		file.SeekToBegin();
		file.Read((BYTE*)pBuf, dwLen);
		pBuf[dwLen] = _T('\0');
		strData.Format(_T("%s"), pBuf);
		delete[] pBuf;
		strData.MakeUpper();			// �񱳸� ���Ͽ� �빮�ڷ� ��ȯ
		// TNS Name �˻�

		////////////////////////////////////////////////////////////////////////////////
		// IDMS_BILL
		for(int i=0;i<5;i++)
		{			
			if     (i==0)
			{
				if(m_strSetServer==IDMS_BILL)		 strChkSid = BILL_ALIAS;	
				else								 strChkSid = CUSTOM_ALIAS;	
			}
			else if(i==1)		
			{
				if(m_strSetServer==IDMS_BILL)		 strChkSid = BILL_ALIAS_WORLD;	
				else								 strChkSid = CUSTOM_ALIAS_WORLD;
			}
			else if(i==2)							 strChkSid = OPERA_ALIAS;				// META_DATA
			else if(i==3)							 strChkSid = OPERA_ALIAS_WORLD;			// META_DATA
			else /*if(i==2)*/						 strChkSid = "CCRCSS10";
			
			nPos = 0;
			while( nPos >= 0 )
			{
				nPos = strData.Find(_T(strChkSid), nPos);					
				if( nPos < 0 )		break;
	
				char szAfterChar =_T(' ');
				char szTemp      =_T(' ');

				// Key Word ���� ù Char �� '=' �ΰ�� �ش� ������ �ִ� ������ �Ǵ�. 
				int j=0;
				while(true)
				{
					szTemp = strData.GetAt(nPos + strlen(strChkSid)+j);
					if(szTemp!=_T(' '))
					{
						szAfterChar=szTemp;
						break;
					}					
					j++;					
					if(strData.GetLength() <= (int)(nPos + strlen(strChkSid)+j)) break;					
				}

				if(szAfterChar==_T('='))	
				{
					if     (i==0) bReal      = TRUE;
					else if(i==1) bRealWorld = TRUE;
					else if(i==2) bOpera     = TRUE;						// META_DATA
					else if(i==3) bOperaWorld= TRUE;						// META_DATA
					else		  bCcrcss10  = TRUE;					
					break;
				}

//				// �������� Ȯ��
//				if( nPos > 0                         && 
//					strData.GetAt(nPos-1) >= _T('A') &&
//					strData.GetAt(nPos-1) <= _T('Z') 
//				  )					continue;
//			
//				// �������� Ȯ��
//				if( strData.GetAt(nPos + strlen(strChkSid)) >= _T('A') &&	
//					strData.GetAt(nPos + strlen(strChkSid)) <= _T('Z') )		continue;
//
//				if( strData.Find(_T('('), nPos) < strData.Find(_T(')'), nPos) )
//				{
//					if     (i==0) bReal      = TRUE;
//					else if(i==1) bRealWorld = TRUE;
//					else		  bCcrcss10  = TRUE;	
//					
//					break;
//				}
				nPos++;
			}
		}
		// IDMS_BILL
		////////////////////////////////////////////////////////////////////////////////
	}

	file.SeekToEnd();
	if( bReal == FALSE )
	{
		if(m_strSetServer==IDMS_BILL) strData = GetServerInfoBill(BILL_ALIAS);		  // IDMS_BILL
		else						  strData = GetServerInfoCt40(CUSTOM_ALIAS);	  // IDMS_BILL
		
		file.Write(strData, strData.GetLength());
	}
	
	if( bRealWorld == FALSE )														   // IDMS_BILL	
	{																				   // IDMS_BILL
		if(m_strSetServer==IDMS_BILL) strData = GetServerInfoBill(BILL_ALIAS_WORLD);   // IDMS_BILL
		else						  strData = GetServerInfoCt40(CUSTOM_ALIAS_WORLD); // IDMS_BILL
																					   // IDMS_BILL
		file.Write(strData, strData.GetLength());									   // IDMS_BILL
	}																				   // IDMS_BILL

	if( bOpera == FALSE )																// META_DATA
	{																					// META_DATA
		strData = GetServerInfoOpera(OPERA_ALIAS);										// META_DATA
		file.Write(strData, strData.GetLength());										// META_DATA
	}																					// META_DATA

	if( bOperaWorld == FALSE )															// META_DATA
	{																					// META_DATA
		strData = GetServerInfoOpera(OPERA_ALIAS_WORLD);								// META_DATA
		file.Write(strData, strData.GetLength());										// META_DATA	
	}																					// META_DATA
	

	if( bCcrcss10 == FALSE )
	{
		strData = GetServerInfoCcrcss10();
		file.Write(strData, strData.GetLength());
	}

	file.Close();

	return TRUE;
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.09.12 BY LSH		IDMS_BILL
// DESCRIPT    : ��� IDMS ���� ����
// PARAMETER   : strKeyWord - Alias ����
// RETURN      : TRUE(Non Use) 
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetServerInfoBill(CString strKeyWord)						//IDMS_BILL
{
	CString strServer =_T("");

	strServer  = "\r\n\r\n";
	strServer += strKeyWord + " = \r\n";
	strServer += "  (DESCRIPTION =\r\n";
	strServer += "    (ADDRESS_LIST =\r\n";
 	strServer += "      (ADDRESS = (PROTOCOL = TCP)(HOST = "ORA_BILL_IP")(PORT = 1521))\r\n";
	strServer += "    )\r\n";
	strServer += "    (CONNECT_DATA =\r\n";
	strServer += "      (SID = "ORA_BILL_SID")\r\n";
	strServer += "    )\r\n";
	strServer += "  )";

	return strServer;	
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.10.27 BY LSH					 META_DATA
// DESCRIPT    : IMSS DB(OPERA10) Connect String Set
// PARAMETER   : strKeyWord - Connection Alias
// RETURN      : String Set
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetServerInfoOpera(CString strKeyWord)
{
	CString strServer =_T("");

	strServer  = "\r\n\r\n";
	strServer += strKeyWord + " = \r\n";
	strServer += "  (DESCRIPTION =\r\n";
	strServer += "    (ADDRESS_LIST =\r\n";
 	strServer += "      (ADDRESS = (PROTOCOL = TCP)(HOST = 147.6.35.228)(PORT = 1521))\r\n";
	strServer += "    )\r\n";
	strServer += "    (CONNECT_DATA =\r\n";
	strServer += "      (SID = OPERA10)\r\n";
	strServer += "    )\r\n";
	strServer += "  )";

	return strServer;	
}

CString CUpdateManagerDlg::GetServerInfoCt40(CString strKeyWord)
{
	CString strServer =_T("");
	
	strServer  = "\r\n\r\n";
	strServer += strKeyWord + " = \r\n";
	strServer += "  (DESCRIPTION =\r\n";
	strServer += "    (ADDRESS_LIST =\r\n";
	strServer += "      (ADDRESS = (PROTOCOL = TCP)(HOST = "ORA_CUSTOM_IP")(PORT = 1522))\r\n";
	strServer += "    )\r\n";
	strServer += "    (CONNECT_DATA =\r\n";
	strServer += "      (SID = "ORA_CUSTOM_SID")\r\n";
	strServer += "    )\r\n";
	strServer += "  )";

	return strServer;
}

CString CUpdateManagerDlg::GetServerInfoCcrcss10()
{
	CString strServer =_T("");

// Update_Date	: 2007.09.17 BY KSY
// ReauireDesc	: ��������� ���̻� ������� ����
//	strServer =	"\r\n\r\n"
//				"CCRCSS10 =\r\n"
//				"  (DESCRIPTION =\r\n"
//				"    (ADDRESS_LIST =\r\n"
//				"      (ADDRESS = (PROTOCOL = TCP)(HOST = 147.6.119.15)(PORT = 1521))\r\n"
//				"    )\r\n"
//				"    (CONNECT_DATA =\r\n"
//				"      (SID = ccrcss10)\r\n"
//				"    )\r\n"
//				"  )";

	return strServer;
}

void CUpdateManagerDlg::SetOption(DWORD dwOption)
{
	m_dwOption = dwOption;
}

void CUpdateManagerDlg::Trace(CString strData)
{
	CFile	file;
	CString	strFile;
	CTime	dtTime;
	CString	strBuf;

	strFile = AfxGetApp()->GetProfileString(IDMS_GLOBAL_REG_SETTINGS,
											IDMS_GLOBAL_REG_INSTALLDIR);
	if( strFile.Right(1) != _T("\\") )
		strFile += _T("\\Bin\\UpdateTrace.log");
	else
		strFile += _T("Bin\\UpdateTrace.log");

	if( file.Open(strFile, CFile::modeReadWrite) == FALSE )
		return;

	if( file.GetLength() > 0 )
	{
		file.SeekToEnd();
		file.Write(_T("\r\n\r\n"), sizeof(TCHAR) * 4);
	}
	dtTime = CTime::GetCurrentTime();
	strBuf.Format(_T("%s\r\n%s\r\n"), dtTime.Format(_T("[%Y.%m.%d %H:%M:%S]")), strData);
	file.Write(strBuf, strBuf.GetLength());

	file.Close();
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.28 BY LSH		(GET_TNSFILE : From Devpia)
// DESCRIPT    : Ora Home path Get
// PARAMETER   : 
// RETURN      : String - Ora Home path
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetDefaultOracleHomePath()
{

	BOOL			ret = FALSE;
	BOOL			isDefaultHomeExist = TRUE;

	CString			strPath         =_T("");
	CString			strDefaultHome  =_T("");
	
	CStringArray	oracleHomePathList;
	
	// Step 1. Get Oracle Home Path
	ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ORACLE\\ALL_HOMES"), _T("DEFAULT_HOME"), strDefaultHome );
	if (ret == FALSE || strDefaultHome.IsEmpty() == TRUE) isDefaultHomeExist = FALSE;
	
	if (isDefaultHomeExist == TRUE)
	{
		// DEFAULT_HOME ���� ����(10g�� �� ���� �ƴ� ���, �Ǵ� 10g�� ��� ���� ���� ���)
		CString strHomeCounterValue;
		
		ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ORACLE\\ALL_HOMES"), _T("HOME_COUNTER"), strHomeCounterValue );

		// HOME_COUNTER ���� �������� �ʴ� ��쿡�� ID0 �� ��츦 �ѹ� �׽�Ʈ �غ� 
		if (ret == FALSE || strHomeCounterValue.IsEmpty() == TRUE)		strHomeCounterValue = "1";
			
		const int homeCount = atoi(strHomeCounterValue);
		
		for (int i = 0; i < homeCount; ++i)
		{
			CString strIds, strName;
			
			strIds.Format(_T("SOFTWARE\\ORACLE\\ALL_HOMES\\ID%d"), i);
			ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, strIds, _T("NAME"),  strName);


			// ��ȿ���� ���� ORACLE HOME ����, �׷��� �׳� �������� �Ѿ��.
			if ( ret == FALSE || strName.IsEmpty() == TRUE) continue;
			
			CString tempPath=_T("");
			
			ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, strIds, _T("PATH"),  tempPath);

			if ( ret == FALSE || tempPath.IsEmpty() == TRUE) 		continue;
			
			oracleHomePathList.Add(tempPath);
			
			if (strName == strDefaultHome) 				
			{        				
				if ( tempPath.IsEmpty() == FALSE)  strPath = tempPath;				
				break;				
			}
			
		} // end for		
		
		// ��� HOME ������ ã�Ҵµ��� �ұ��ϰ� ��ȿ�� HOME ����������, ��ȿ�� Default Ȩ ���� ���� ������ �Ǵ���
		if (strPath.IsEmpty() == TRUE) isDefaultHomeExist = FALSE;
		
	} // if (isDefaultHomeExist == TRUE)
	
	// System Path ���� ���� ���� ���� �ִ� Oracle Home Path�� ã�´�.
	// 10g ���ʹ� Default Oracle Home �� System Path�� ���� �տ� ���´�.
	
	BOOL isOra10GExist = FALSE;
	Get10GOracleHome(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ORACLE"), oracleHomePathList, isOra10GExist);
		
	if (isOra10GExist)
	{
		CString systemPath;
		ret = GetRegKeyValue(HKEY_LOCAL_MACHINE, 
			   				_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), 
							_T("Path"),
							systemPath);
		
		if( ret == TRUE && systemPath.IsEmpty() == FALSE)			
		{			
			int firstOracleHomeIndex = -1;			
			const int listSize = oracleHomePathList.GetSize();			
			char* tempOracleBin = (char*)0xFFFFFFFF;			
			for (int i = 0; i<listSize; ++i)				
			{				
				char* temp = strstr(systemPath, oracleHomePathList[i]);				
				if(temp)					
				{					
					if (temp < tempOracleBin)						
					{						
						tempOracleBin = temp;						
						firstOracleHomeIndex = i;
					}					
				}				
			}
			
			// ȯ�� ������ ���� ���			
			if (firstOracleHomeIndex == -1 ) 		isDefaultHomeExist = FALSE;			
			else				
			{				
				// ���� ����Ʈ Ȩ�̶� �ٸ� ���, �̷��� ���� ���� �켱�̴�. path�� �� ó���� �����Ƿ�...				
				if (strPath != oracleHomePathList[firstOracleHomeIndex])					
				{					
					strPath = oracleHomePathList[firstOracleHomeIndex];					
					isDefaultHomeExist = TRUE;					
				}
			}			
		}    		
	}	
	
	if (isDefaultHomeExist == FALSE)		
	{

		// DEFAULT_HOME ���� ���� , 10g �� �� ���� ���� ��� üũ�� �������� �� 
		ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ORACLE"), _T("ORA_CRS_HOME"), strPath );
		if ( ret == FALSE || strPath.IsEmpty() == TRUE) 
		{
			// ������ �õ�	
			ret = GetRegKeyValue( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ORACLE"), _T("ORACLE_HOME"), strPath );	

			if ( ret == FALSE || strPath.IsEmpty() == TRUE) 				
			{				
				// ���� ��Ȳ 2				
				// �� �̻� ORACLE HOME�� ã�� ����� ����				
				return "";				
			}			
		}		
	}	
	return strPath;	
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.28 BY LSH		(GET_TNSFILE : From Devpia)
// DESCRIPT    : 
// PARAMETER   : 
// RETURN      : TRUE(Non Use) 
// REMARK      : 
//-----------------------------------------------------------------------//
BOOL CUpdateManagerDlg::Get10GOracleHome(HKEY hKey, 
										 CString strSection, 
										 CStringArray& oraclePathList, 
										 BOOL& isOra10GExist)
{
	HKEY hSecKey=NULL;	
	DWORD retCode;	
	int i = 0;

	if ( ::RegOpenKeyEx( hKey, strSection, 0, KEY_ALL_ACCESS, &hSecKey ) != ERROR_SUCCESS )		return FALSE;
	
	for ( retCode = (DWORD)ERROR_SUCCESS; retCode == ERROR_SUCCESS; ++i) 		
	{     		
		char bufPath[MAX_PATH];		
		DWORD bufSize = sizeof(bufPath);		
		retCode = RegEnumKeyEx(hSecKey, i, bufPath, &bufSize, NULL, NULL, NULL, NULL); 
		
		if (retCode == (DWORD)ERROR_SUCCESS) 			
		{			
			if (strstr(bufPath, "KEY_"))				
			{				
				CString oracleHomePath;				
				BOOL ret = GetRegKeyValue(hKey, strSection+"\\"+bufPath, _T("ORACLE_HOME"), oracleHomePath);				
				if (ret == FALSE || oracleHomePath.IsEmpty() == TRUE )			continue;			
				
				oraclePathList.Add(oracleHomePath);				
				isOra10GExist = TRUE;				
			}			
		}		
	}	
	RegCloseKey( hSecKey );	
	return TRUE;	
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.28 BY LSH		(GET_TNSFILE : From Devpia)
// DESCRIPT    : Get TnaNames.ora File Path
// PARAMETER   : 
// RETURN      : File Path
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetDefaultTNSNameFilePath()
{	
	CString     strFileName;
	CString     strPath = GetDefaultOracleHomePath();

	strFileName.Format( _T("%s\\network\\admin\\tnsnames.ora"), strPath );
	
	if( (_taccess( strFileName, 0 )) == -1 )		
	{
		// File does not exists in this case		
		strFileName.Format( _T("%s\\net80\\admin\\tnsnames.ora"), strPath );
		
		if( (_taccess( strFileName, 0 )) == -1 )			
		{			
			// ���� ��Ȳ 3			
			// ����� ���� ���� ã�ư��� tnsname.ora �� ����			
			return "";			
		}		
	}	
	return strFileName;	
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.28 BY LSH		(GET_TNSFILE : From Devpia)
// DESCRIPT    : Registry Value Get
// PARAMETER   : hKkey,strSection,szKey - Registry path Info
//               strValue - Registry Key Value 
// RETURN      : TRUE, FALSE
// REMARK      : 
//-----------------------------------------------------------------------//
BOOL CUpdateManagerDlg::GetRegKeyValue(HKEY hKey, LPCSTR strSection, LPCTSTR szKey, CString &strValue)
{
	DWORD size = 2048;	
	char szBuff[2048];	
	HKEY openKey;	
	LONG nRetVal = ::RegOpenKey(HKEY_LOCAL_MACHINE, strSection, &openKey);
	
	if(ERROR_SUCCESS != nRetVal)
	{		
		LPVOID lpMsgBuf;		
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,  NULL, GetLastError(),			
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language			
		               (LPTSTR) &lpMsgBuf, 0,  NULL ); 

		strValue = "";
		
		return FALSE;		
	}	

	nRetVal = ::RegQueryValueEx(openKey, szKey, 0, NULL, (unsigned char *) szBuff, &size);	
	
	::RegCloseKey(openKey);	

	if(nRetVal == ERROR_SUCCESS)
	{		
		strValue = szBuff;		
		return TRUE;		
	}	
	else
	{		
		strValue = "";		
		return FALSE;		
	}	
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.29 BY LSH						IDMS_BILL
// DESCRIPT    : Ini ������ ���Ե� Server ���� Get
// PARAMETER   : 
// RETURN      : Server ����(��, ���, Null) 
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetServerFlag()
{
	CHAR buf[16];
	CString strInfo=_T("");
	
	ZeroMemory(buf,sizeof(buf));
	
	::GetPrivateProfileString(IDMS_INI_SECTION,
		                      IDMS_INI_KEY,
							  "",
							  buf,
							  sizeof(buf),
							  IDMS_INI_PATH);
	strInfo.Format("%s",buf);

	if(strInfo.IsEmpty())
	{
		CSelectServer	dlg;
		if(dlg.DoModal()==IDOK)
		{			
			CString strPath=_T("");
			
			if(!dlg.m_strServer.IsEmpty())	strInfo=dlg.m_strServer;

			strPath = GetCurPath();
			CreateDirectory(strPath + _T("Ini"), NULL);				// ini ���� ���� 

			WritePrivateProfileString(IDMS_INI_SECTION, 
				                      IDMS_INI_KEY, 
									  strInfo,
									  IDMS_INI_PATH);    
		}
		else return _T("");		
	}

	return strInfo;
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.08.29 BY LSH			IDMS_BILL
// DESCRIPT    : strSvrInfo - ���� ����
// PARAMETER   : 
// RETURN      : TRUE(Non Use) 
// REMARK      : 
//-----------------------------------------------------------------------//
int CUpdateManagerDlg::SetConnectInfo(CString& strSvrInfo)
{
	if(strSvrInfo==IDMS_BILL)		// ��� 
	{		
		m_strSID  = BILL_ALIAS;
		m_strUser = ORA_BILL_ID;
		m_strPW   = ORA_BILL_PW;
	}
	else
	{
		m_strSID  = CUSTOM_ALIAS;
		m_strUser = ORA_CUSTOM_ID;
		m_strPW   = ORA_CUSTOM_PW;
	}

	return TRUE;
}

//-----------------------------------------------------------------------//
// MAKE_DATE   : 2006.09.01 BY LSH			IDMS_BILL
// DESCRIPT    : ��ġ�� Path Get
// PARAMETER   : 
// RETURN      : String - ��ġ�� Path
// REMARK      : 
//-----------------------------------------------------------------------//
CString CUpdateManagerDlg::GetCurPath()
{
	CString strPath=_T("");
	
	strPath = AfxGetApp()->GetProfileString(IDMS_GLOBAL_REG_SETTINGS,IDMS_GLOBAL_REG_INSTALLDIR);

	if( strPath.IsEmpty() )
	{
		char szPath[MAX_PATH];
		memset(szPath, 0, MAX_PATH);		
		GetCurrentDirectory(MAX_PATH, szPath);

		strPath = szPath;
		strPath.Delete(strPath.ReverseFind(_T('\\')),
					   strPath.GetLength() - strPath.ReverseFind(_T('\\')));
	}

	if( strPath.Right(1) != _T("\\") )		strPath += _T("\\");

	return strPath;
}









