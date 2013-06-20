// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 


#if !defined(AFX_STDAFX_H__71BF88DD_141D_4143_92D2_FC58D2B71F02__INCLUDED_)
#define AFX_STDAFX_H__71BF88DD_141D_4143_92D2_FC58D2B71F02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500 //kojh
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define TRACEX  TRACE("%s(%d) : ", __FILE__, __LINE__); TRACE

#include <winsock2.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")

//kojh
#include <afx.h>
#include <afxcoll.h>
#include "..\\common\\AllCommonDefines.h"
#include "AllSockDefines.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <set>


using namespace std ;

void writeLogFileW( const char* filename, char *fmt,...);
void writeLogFile(const char* filename, char *fmt,...);
CString Log_fprintf(FILE *FD, const char *fmt, va_list ap);
void getDelimitData(const string strSrc, char delimit, string& strOut, int index) ;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__71BF88DD_141D_4143_92D2_FC58D2B71F02__INCLUDED_)
