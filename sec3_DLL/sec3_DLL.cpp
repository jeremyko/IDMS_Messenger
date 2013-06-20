// sec3_DLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "sec3_DLL.h"
#include "Sec3Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CSec3_DLLApp

BEGIN_MESSAGE_MAP(CSec3_DLLApp, CWinApp)
	//{{AFX_MSG_MAP(CSec3_DLLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSec3_DLLApp construction

CSec3_DLLApp::CSec3_DLLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

extern "C" _declspec(dllexport) CDialog * CreateDlg()
{
	HINSTANCE hInstResource = AfxGetResourceHandle();

	HMODULE hModule = ::GetModuleHandle("sec3_DLL.DLL");
	AfxSetResourceHandle(hModule);
	//CWinApp *pApp = AfxGetApp();
	//CWnd *pWnd = pApp->GetMainWnd();

	/*
	HMODULE hM = GetModuleHandle(NULL);

	FnPtrT FnPtr = (FnPtrT)::GetProcAddress(hM, "__callback_print");
	if (FnPtr) 
	{
		(*FnPtr)("Message From The DLL");
	}
	*/
	
	CSec3Dlg *pDlg = new CSec3Dlg;	
		
	if(pDlg->Create(CSec3Dlg::IDD) == TRUE)
	{		
		//pDlg->SetWindowSizeInMain(pDlg->GetSafeHwnd()); //size
		
		AfxSetResourceHandle(hInstResource);
		return (CDialog*)pDlg;
	}
	else
		delete pDlg;	

	AfxSetResourceHandle(hInstResource);

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSec3_DLLApp object

CSec3_DLLApp theApp;
