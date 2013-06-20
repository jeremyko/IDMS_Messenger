// stdafx.cpp : source file that includes just the standard includes
//	IdmsMsn.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information
//
#include "stdafx.h"

using namespace std;

void writeLogFile(const char* filename, char *fmt,...)
{	
	va_list		ap;
    FILE		*fd;		
	CFile	file;
	char szFile[2048];
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
	//strRet = strRet +"\n";
    va_end(ap);
    fclose(fd);		
}

CString Log_fprintf(FILE *FD, const char *fmt, va_list ap)
{
    int d;
    char c, p, *s;

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
					strTemp.Format( _T("%s"), s);
                    break;
                case 'd':           /* int */
                    d = va_arg(ap, int);
                    fprintf(FD,"%d", d);
					strTemp.Format( _T("%d"), d);
                    break;
                case 'c':           /* char */
                    c = va_arg(ap, char);
                    fprintf(FD,"%c", c);
					strTemp.Format( _T("%c"), c);
        			break;
            }
			strLog += strTemp;
	    }
		else
		{
            fprintf(FD,"%c",p);
			strTemp.Format(_T("%c"), p);
			strLog += strTemp;
		}
	}

	fflush(FD);

    return strLog;
}

void		getDelimitData(const string strSrc, char delimit, string& strOut, int index)
{
	string	data("\0",10240);
	int		size = strSrc.length();
	int		count, point;
	int		i;	

	for ( i=0, count=0, point=0; i<size; i++ ) 
	{
		char ch = strSrc[i];
		if ( ch == delimit ) 
		{
			count++;
			continue;
		}
		
		if ( index != count ) continue;

		data[point++] = ch;	
	}
		
	data.resize(point);
	strOut = data;
	//return sprintf(pdat, "%s", data);	
}

int GetStrByDelim(char* szSrc, char* szOut, const char cDeli)
{	
	int nPos = 0;
	char	ch = 0;	

	int	n_size = strlen(szSrc);
	for ( int i=0; i< n_size; i++ ) 
	{
		if ( szSrc[i] == cDeli ) 
		{
			strncpy(szOut , szSrc , nPos) ; // 출력문자열
			strncpy(szSrc , szSrc + nPos + 1, n_size - nPos) ; // Src를 변경~
			return nPos;
		}
		else
		{
			nPos++;
		}		
	}

	strcpy(szOut , "NotFound" ) ; 
	return  -1;	
}

// Full Path 에서 Root Path 를 제거한다 
void getFileNamePathExceptRoot(const char* szFileWithPath, const char* szRootPath, char* szOutFilePath )
{
	char szFileName [360 + 1];
	
	memset(szFileName, 0x00, sizeof(szFileName));
		
	int nPos = 0;
	int nLen = 0;
	int nRootLen = 0;	
	
	strncpy( szFileName , szFileWithPath , sizeof(szFileName));
	nLen = strlen(szFileName);
	nRootLen = strlen(szRootPath);
	
	TRACE("nLen [%d] nRootLen [%d]\n", nLen, nRootLen);

	//szFileName = szFileName.Mid(nRootLen+1, nLen-nRootLen);	
	strncpy(szOutFilePath, szFileName + nRootLen+1,  nLen-nRootLen);	

	TRACE("szOutFilePath [%s]\n",szOutFilePath);
	
}