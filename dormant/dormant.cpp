// dormant.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "dormant.h"

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
// CDormantApp

BEGIN_MESSAGE_MAP(CDormantApp, CWinApp)
	//{{AFX_MSG_MAP(CDormantApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDormantApp construction

CDormantApp::CDormantApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDormantApp object

CDormantApp theApp;


#pragma comment(linker, "/section:SharedData,rws")
#pragma data_seg("SharedData")

static HINSTANCE    hInstance=NULL;
static HHOOK kbHook=0;
static HHOOK msHook=0;
static CTime lastTime=0;
static POINT pos={0,0};

#pragma data_seg()


LRESULT CALLBACK KeyboardHook (int nCode, WORD wParam, DWORD lParam )
{
	if(nCode==HC_ACTION || nCode==HC_NOREMOVE)
		lastTime = CTime::GetCurrentTime();

	return (int)CallNextHookEx(kbHook, nCode, wParam, lParam);
}


LRESULT CALLBACK MouseHook (int nCode, WORD wParam, DWORD lParam )
{
	// If necessary Save or Load variables...
	// wHitTestCode 
	// HTNOWHERE, HTCLIENT, HTCAPTION, 

	if(nCode>=0)
	{
		UINT a=((MOUSEHOOKSTRUCT*)lParam)->wHitTestCode;
		POINT p=((MOUSEHOOKSTRUCT*)lParam)->pt;
		if(a!=HTCLIENT || p.x!=pos.x || p.y!=pos.y)
		{
			lastTime = CTime::GetCurrentTime();
			pos = p;
		}
	}

	return (int)CallNextHookEx(msHook, nCode, wParam, lParam); 
}


extern "C" __declspec(dllexport) BOOL Initiate()
{
	kbHook = 0;
	msHook = 0;
	lastTime = CTime::GetCurrentTime();
	hInstance = theApp.m_hInstance;

	
	kbHook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardHook, hInstance, 0);
	msHook = (HHOOK)SetWindowsHookEx(WH_MOUSE,(HOOKPROC)MouseHook, hInstance, 0);

	if(!msHook || !kbHook)
	{	if(kbHook)	UnhookWindowsHookEx(kbHook);
		if(msHook)	UnhookWindowsHookEx(msHook);
		return FALSE;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) BOOL EndUp()
{
	if(UnhookWindowsHookEx(kbHook) && UnhookWindowsHookEx(msHook))
		return TRUE;
	else
		return FALSE;
}

extern "C" __declspec(dllexport) time_t GetDormantTime()
{
	CTime curTime = CTime::GetCurrentTime();
	return curTime.GetTime() -  lastTime.GetTime();
}

