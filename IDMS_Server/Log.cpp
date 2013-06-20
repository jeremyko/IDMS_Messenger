// Log.cpp : implementation file
//

#include "stdafx.h"
#include "idms_server.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLog

CLog::CLog()
{
	memset(m_szLogPath,0x00,sizeof(m_szLogPath));
}

CLog::CLog(HWND hOwnerWnd)
{
	memset(m_szLogPath,0x00,sizeof(m_szLogPath));
}


CLog::~CLog()
{
}

CLog& CLog::Instance()
{
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	static CLog obj(pMainWnd->GetSafeHwnd());
	
	return obj;
} 


void CLog::WriteFile( BOOL bOverWrite, const char* filename, char *fmt,...)
{	
	va_list		ap;
    FILE		*fd;		
	CFile	file;
	char szFile[1024+1];
	CString strFileName;

	CTime iDT =  CTime::GetCurrentTime();	
	char szTmpPath[100];
	
	memset(szTmpPath, 0x00, sizeof(szTmpPath));		
	_snprintf(szTmpPath ,sizeof(szTmpPath)-1, "%s%s%s.log", GetLogPath(), filename, iDT.Format("%Y%m%d")  );

	memset(szFile,0x00,sizeof(szFile));		
	
	strncpy(szFile, szTmpPath, 1024);
	
	if(bOverWrite)
	{
		if((fd = fopen(szFile,"w")) == 0)
		{
			return;
		}
	}
	else
	{
		if((fd = fopen(szFile,"a")) == 0)
		{
			return;
        }
	}
	
    va_start(ap,fmt);
    CString strRet = Log_fprintf(fd,fmt,ap);
	strRet = strRet +"\n";
    va_end(ap);
    fclose(fd);		
}

CString CLog::Log_fprintf(FILE *FD, const char *fmt, va_list ap)
{
    int d;
    char c, p, *s;
	void* x;

	CString strLog;

	CTime iDT =  CTime::GetCurrentTime();	
	fprintf(FD,"[%s]", iDT.Format("%Y%m%d%H%M%S"));
		
	
	CString strTemp;
    while (*fmt)
	{		
        if( (p=*fmt++)=='%' )
		{
            switch(*fmt++) 
			{
                case 's':           /* string */
                    s = va_arg(ap, char *);
                    fprintf(FD,"%s", s);
					strTemp.Format("%s", s);
                    break;
                case 'd':           /* int */ 
					d = va_arg(ap, int);
					fprintf(FD,"%ld", d);
					strTemp.Format("%d", d);					
                    break;				
				
                case 'c':           /* char */
                    c = va_arg(ap, char);
                    fprintf(FD,"%c", c);
					strTemp.Format("%c", c);
        			break;
				case 'x':           /* point */
                    x = va_arg(ap, void *);
                    fprintf(FD,"%x", x);
					strTemp.Format("%x", x);
        			break;
            }
			strLog += strTemp;
	    }
		else
		{
            fprintf(FD,"%c",p);
			strTemp.Format("%c", p);
			strLog += strTemp;
		}
	}

	fflush(FD);

    return strLog;
}



BEGIN_MESSAGE_MAP(CLog, CWnd)
	//{{AFX_MSG_MAP(CLog)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLog message handlers
