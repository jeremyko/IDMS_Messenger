// INIManager.cpp: implementation of the CINIManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "INIManager.h"

extern "C"
{
    #include <direct.h>
}
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#define INI_SEC	_T("SECTION") 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CINIManager::CINIManager()
{		
	
}	

CINIManager::~CINIManager()
{	
	mINIManagerMap.RemoveAll();
	
}

//////////////////////////////////////////////////////////////////////
// CINIManger Functions

void CINIManager::SetIniFile( const char* szModule, const char* szIniFile )
{
	memset(m_szModule,0x00, sizeof(m_szModule));
	memset( m_szIniFile, 0x00, sizeof(m_szIniFile));
	memset(m_szConfPath, 0x00, sizeof(m_szConfPath));
	
	strncpy( m_szModule ,  szModule, sizeof(m_szModule)) ;	
	strncpy( m_szIniFile , szIniFile, sizeof(m_szIniFile) ) ;	
		
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));
	
	GetModuleFileName ( GetModuleHandle(m_szModule), szDirTmp, sizeof(szDirTmp));

	// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ 까지 구한다.
	
	int nPos = 0;
	int nCnt = 0;
	int nLen = strlen(szDirTmp);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <2 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			nCnt++;

			if(nCnt ==2)
			{
				break;
			}
		}
	}

	TRACE("%s\n" , szDirTmp);	
	
	strFileName.Format("%s\\ini\\",szDirTmp);	
		
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}	
		
	sprintf(m_szConfPath,"%s%s", (LPCSTR)strFileName, m_szIniFile);
		    	
	TRACE("m_szConfPath [%s] \n", m_szConfPath);	
}

CINIManager& CINIManager::Instance()
{
	static CINIManager obj;
	return obj;
}


void CINIManager::SetKeySection(CString& strKey, CString& strSec)
{
	mSectionMap.SetAt(strKey,  strSec );
	m_StrKeyAry.Add(strKey) ;	
}


// INI 파일에서 정보를 읽어온다.
void CINIManager::LoadINIFile()
{	
	CString strOutVal;
	int nSize = m_StrKeyAry.GetSize();

	for(int n=0; n < nSize; n++)
	{
		CString strKey = m_StrKeyAry.GetAt(n);

		char szBuff[1024];
		
		if( mSectionMap.Lookup (strKey, strOutVal) )
		{
			memset(szBuff, 0x00, sizeof(szBuff));
			::GetPrivateProfileString(strOutVal, strKey, "0", szBuff, sizeof(szBuff), m_szConfPath); 
			mINIManagerMap.SetAt( (LPCTSTR) strKey, szBuff);
		}		
	}	
}

//INI_FTP_IP

void CINIManager::SaveINIFile()
{
	CString strOutVal, strOutValSec;

	int nSize = m_StrKeyAry.GetSize();
	
	for(int n=0; n < nSize; n++)
	{
		CString strKey = m_StrKeyAry.GetAt(n);

		strOutVal.Empty();
		
		if(mINIManagerMap.Lookup(strKey,  strOutVal ))
		{
			if( mSectionMap.Lookup (strKey, strOutValSec) )
			{
				
				::WritePrivateProfileString (strOutValSec, strKey, strOutVal, m_szConfPath); 
			}
		}
	}	
}

void CINIManager::GetValue(const char* szFindKey, CString& strOutVal)
{	
	if (mINIManagerMap.Lookup(szFindKey,  strOutVal ))
	{
		//TRACE("strFindKey [%s] strOutVal [%s]" , strFindKey, strOutVal );
	}
	else
	{
		strOutVal ="";
	}
}



void CINIManager::SetValue( const char* szFindKey, const char* szVal)
{	
	CString strKey (szFindKey);
	m_StrKeyAry.Add(strKey);

	mINIManagerMap.SetAt(szFindKey,  szVal );
}
