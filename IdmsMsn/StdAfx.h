// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#  pragma warning ( disable : 4355 4284  4231 4503 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#if !defined(AFX_STDAFX_H__39135384_70DA_4688_998C_C066CA025260__INCLUDED_)
#define AFX_STDAFX_H__39135384_70DA_4688_998C_C066CA025260__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define WINVER 0x0500		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.

/*
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0500		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT 		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0500		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif
*/

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#define TRACEX  TRACE("%s(%d) : ", __FILE__, __LINE__); TRACE

//#define UNICODE
//#define _UNICODE

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxtempl.h>

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <windowsx.h>
#include <Mmsystem.h> //sound

//GDI+
/*
#include <comdef.h> //!!
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus") 
*/ 
//

#include <winsock2.h>

#include "GSEncryptX.h"

#include <string>

#include <afxrich.h>		// MFC rich edit classes
//#include <olestd.h>

#pragma warning(disable : 4996)
#pragma warning(disable : 4819)

using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "..\\LibDebug\\GSEncryptX.lib")
#else
#pragma comment(lib, "..\\LibRelease\\GSEncryptX.lib")
#endif

#import <msxml4.dll>

void writeLogFile(const char* filename, char *fmt,...);
CString Log_fprintf(FILE *FD, const char *fmt, va_list ap);
void		getDelimitData(const string strSrc, char delimit, string& strOut, int index);
int GetStrByDelim(char* szSrc, char* szOut, const char cDeli);
void getFileNamePathExceptRoot(const char* szFileWithPath, const char* szRootPath, char* szOutFilePath );

/*
//GDI+
class CMemDC : public CDC
{
protected:
   CBitmap  m_bitmap;       
   CBitmap* m_oldBitmap;    
   CDC*     m_pDC;          
   CRect    m_rect;         
   BOOL     m_bMemDC;       
    
   void Build( CDC* pDC )
   {
        ASSERT(pDC != NULL); 

        m_pDC		= pDC;
        m_oldBitmap = NULL;
        m_bMemDC	= !pDC->IsPrinting();

        if( m_bMemDC )
		{
            CreateCompatibleDC(pDC);
            pDC->LPtoDP(&m_rect);

            m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
            m_oldBitmap = SelectObject(&m_bitmap);
            
            SetMapMode(pDC->GetMapMode());
            pDC->DPtoLP(&m_rect);
            SetWindowOrg(m_rect.left, m_rect.top);
        }
		else
		{
            m_bPrinting = pDC->m_bPrinting;
            m_hDC       = pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        }

        FillSolidRect( m_rect, pDC->GetBkColor() );
    }

public:
   CMemDC( CDC *pDC )
	   : CDC()
   {
	   pDC->GetClipBox( &m_rect );
	   Build( pDC );
   }

   CMemDC( HDC hDC )
	   : CDC()
   {
	   CDC::FromHandle( hDC )->GetClipBox( &m_rect );
	   Build( CDC::FromHandle( hDC ) );
   }

   CMemDC( CDC *pDC, const RECT& rect ) 
	   : CDC()
   {
	   m_rect = rect;
	   Build( pDC ); 
   }
    
   virtual ~CMemDC()
   {        
        if( m_bMemDC )
		{
            m_pDC->BitBlt( m_rect.left,
				           m_rect.top,
						   m_rect.Width(),
						   m_rect.Height(),
						   this,
						   m_rect.left,
						   m_rect.top,
						   SRCCOPY );            
            
            SelectObject(m_oldBitmap);        
        }
		else
		{
            m_hDC = m_hAttribDC = NULL;
        }    
    }
    
    CMemDC* operator->() 
    {
        return this;
    }    

    operator CMemDC*() 
    {
        return this;
    }
};
//
*/

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_STDAFX_H__39135384_70DA_4688_998C_C066CA025260__INCLUDED_)
