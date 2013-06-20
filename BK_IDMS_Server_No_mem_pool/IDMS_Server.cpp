// IDMS_Server.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IDMS_Server.h"
#include "IDMS_ServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerApp

BEGIN_MESSAGE_MAP(CIDMS_ServerApp, CWinApp)
	//{{AFX_MSG_MAP(CIDMS_ServerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerApp construction

CIDMS_ServerApp::CIDMS_ServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIDMS_ServerApp object

CIDMS_ServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CIDMS_ServerApp initialization

BOOL CIDMS_ServerApp::InitInstance()
{
	AfxEnableControlContainer();

	AfxInitRichEdit();
	
	//if (!AfxSocketInit())
	//{
	//	AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
	//	return FALSE;
	//}

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
	
	HANDLE hUniKmc = CreateMutex( &mtxAttr, true, AfxGetAppName() );
	if( hUniKmc )
	{
		if( ERROR_ALREADY_EXISTS == GetLastError() ) 
		{
			/*
			HWND hWnd = FindWindow(NULL, "IDMS_Server");
			if(hWnd != NULL)
			{
				CWnd* pWnd = CWnd::FromHandle(hWnd);

				pWnd->ShowWindow(SW_SHOW);
				pWnd->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
				pWnd->SetForegroundWindow();
				pWnd->SetActiveWindow();				
			}
			*/			

			AfxMessageBox("이미 실행중입니다.");				
			CloseHandle( hUniKmc);
			return false;		
		}
	}
	else 
	{
		return false;
	}

	CIDMS_ServerDlg dlg;
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
