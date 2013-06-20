// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__AF97388A_5CCC_4837_BD34_081CD7F6D804__INCLUDED_)
#define AFX_STDAFX_H__AF97388A_5CCC_4837_BD34_081CD7F6D804__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winsock2.h>

//////////////////////////////////////////////////////////////////////////////////////////
// IDMS_BILL
#define ORA_CUSTOM_SID				"TCCRCS20"					// 운용서버(고객)
#define ORA_CUSTOM_ID				"IDMS00"					// 운용ID(고객) 07.09.12 변경
#define ORA_CUSTOM_PW				"TWINS!"					// 운용PW(고객) 07.09.12 변경
#define ORA_CUSTOM_IP				"147.6.184.89"				// 운용IP(고객)


#define ORA_BILL_SID				"BILCSS40"					// 운용서버(요금)
#define ORA_BILL_ID					"IDMS"						// 운용ID(요금)
#define ORA_BILL_PW					"IDMS"						// 운용PW(요금)
#define ORA_BILL_IP					"147.6.119.157"				// 운용IP(요금)
	
// 접속 서버 
#define IDMS_CUSTOM					_T("1")						// IDMS 고객
#define IDMS_BILL					_T("2")						// IDMS 요금 

// ini File Info
#define IDMS_INI_SECTION			_T("SETTING")				// ini File Section
#define IDMS_INI_KEY				_T("SERVER")				// ini File Key
#define IDMS_INI_PATH				_T("..\\Ini\\InitInfo.ini")	// Ini File Path
// IDMS_BILL
//////////////////////////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AF97388A_5CCC_4837_BD34_081CD7F6D804__INCLUDED_)
