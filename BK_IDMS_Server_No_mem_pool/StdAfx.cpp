// stdafx.cpp : source file that includes just the standard includes
//	IDMS_Server.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


void writeLogFileW( const char* filename, char *fmt,...)
{	
	va_list		ap;
    FILE		*fd;		
	CFile	file;
	char szFile[2048+1];
	CString strFileName;

	memset(szFile,0x00,sizeof(szFile));		
	
	strncpy(szFile, filename, 2048);
	
	if((fd = fopen(szFile,"w")) == 0)
	{        
		return;
	}
	
    va_start(ap,fmt);
    CString strRet = Log_fprintf(fd,fmt,ap);
	strRet = strRet +"\n";
    va_end(ap);
    fclose(fd);		
}

void writeLogFile(const char* filename, char *fmt,...)
{	
	va_list		ap;
    FILE		*fd;		
	CFile	file;
	char szFile[2048+1];
	CString strFileName;

	memset(szFile,0x00,sizeof(szFile));		
	
	strncpy(szFile, filename, 2048);
	
	if((fd = fopen(szFile,"a")) == 0)
	{
        if((fd = fopen(szFile,"w")) == 0)
			return;
	}
	
    va_start(ap,fmt);
    CString strRet = Log_fprintf(fd,fmt,ap);
	strRet = strRet +"\n";
    va_end(ap);
    fclose(fd);		
}

CString Log_fprintf(FILE *FD, const char *fmt, va_list ap)
{
    int d;
    char c, p, *s;
	void* x;

	CString strLog;
	CString strTemp;
    while (*fmt)
	{
        if((p=*fmt++)=='%')
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
                    fprintf(FD,"%d", d);
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