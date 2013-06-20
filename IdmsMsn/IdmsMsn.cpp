// IdmsMsn.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "IdmsMsnDlg.h"
#include <Afxsock.h>

#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnApp

BEGIN_MESSAGE_MAP(CIdmsMsnApp, CWinApp)
	//{{AFX_MSG_MAP(CIdmsMsnApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnApp construction

CIdmsMsnApp::CIdmsMsnApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIdmsMsnApp object

CIdmsMsnApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CIdmsMsnApp initialization

BOOL CIdmsMsnApp::InitInstance()
{
	if (!AfxOleInit())
	{		
		return FALSE;
	}

	AfxInitRichEdit();

	if (!AfxSocketInit())
	{
		AfxMessageBox("Windows 소켓 초기화에 실패하였습니다");
		return FALSE;
	}

	AfxEnableControlContainer();
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Single Instance	
	SECURITY_ATTRIBUTES mtxAttr;
	
	mtxAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	mtxAttr.lpSecurityDescriptor = NULL;
	mtxAttr.bInheritHandle = TRUE;

	TRACE("%s\n",AfxGetAppName());	
	
	HANDLE hUniKmc = CreateMutex( &mtxAttr, true, AfxGetAppName() );
	if( hUniKmc )
	{
		TRACE("[%d]\n",GetLastError());
		if( ERROR_ALREADY_EXISTS == GetLastError() ) 
		{						
			
	#ifndef _DEBUG
			CloseHandle( hUniKmc);
			AfxMessageBox("이미 실행중입니다."); 
			return false;		
	#endif
		}
	}

	// updatemanager 를 Update한다: 시작
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));
	
	GetModuleFileName ( GetModuleHandle(AfxGetAppName()), szDirTmp, sizeof(szDirTmp));

	// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ 까지 구한다.
	
	int nPos = 0;
	int nCnt = 0;
	int nLen = strlen(szDirTmp);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			nCnt++;

			if(nCnt ==1)
			{
				break;
			}
		}
	}

	TRACE("%s\n" , szDirTmp);
	
	strFileName.Format("%s\\UpdateManager_.exe",szDirTmp);	
	CString strDestFileName;
	strDestFileName.Format("%s\\UpdateManager.exe",szDirTmp);	
	//////////////////////////

	int nRtn = _access(strFileName, 0) ;

	if(nRtn != -1)
	{
		TRACE("패치할 파일이 존재.\n");

		BOOL rtn = CopyFile(strFileName,strDestFileName,FALSE);
		
		if(!rtn)
		{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
			// Free the buffer.
			LocalFree( lpMsgBuf );

			return FALSE;
		}

		DeleteFile( strFileName );

		ShellExecute(NULL, "open", strDestFileName, "", "", SW_SHOWNOACTIVATE);
				
		return TRUE;
	}
	
	TRACE("패치할 파일이 존재하지 않습니다.\n");		
	// updatemanager 를 Update한다: 종료

	CIdmsMsnDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CIdmsMsnApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	
	return CWinApp::ExitInstance();
}
