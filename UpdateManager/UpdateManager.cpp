// UpdateManager.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UpdateManager.h"
#include "UpdateManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerApp

BEGIN_MESSAGE_MAP(CUpdateManagerApp, CWinApp)
	//{{AFX_MSG_MAP(CUpdateManagerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerApp construction

CUpdateManagerApp::CUpdateManagerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUpdateManagerApp object

CUpdateManagerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUpdateManagerApp initialization

BOOL CUpdateManagerApp::InitInstance()
{
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

	SetRegistryKey(_T("IDMS"));

	CUpdateManagerDlg dlg;
	// 속성을 추가한다.
	if( m_lpCmdLine[0] != NULL )
	{
		DWORD dwOption;

		dwOption = GetOption(m_lpCmdLine);

		dlg.SetOption(dwOption);
	}
	
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

DWORD CUpdateManagerApp::GetOption(LPTSTR szCmdLine)
{
	DWORD	dwOption;
	char	seps[] = " ";
	char	*token;

	dwOption = _OPTION_NONE;
	token = strtok(szCmdLine, seps);
	while( token != NULL )
	{
		if( token[0] != _T('/') )
		{
			TRACE(_T("Not Valid Format!\r\n"));
			continue;
		}
		switch( token[1] )
		{
		case _T('H') :
		case _T('h') :
			dwOption |= _OPTION_HIDE;
			break;
		default :
			TRACE(_T("Not Valid Option!\r\n"));
			break;
		}
		token = strtok(NULL, seps);
	}

	return dwOption;
}
