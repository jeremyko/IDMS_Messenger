// INIManager.cpp: implementation of the CINIManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDMS_Server.h"
#include "INIManager.h"
#include<direct.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CINIManager::CINIManager()
{	
	memset(m_szConfPath, 0x00, sizeof(m_szConfPath));

	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));
	
	GetModuleFileName ( GetModuleHandle("IDMS_Server"), szDirTmp, sizeof(szDirTmp));

	// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ 까지 구한다.
	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
		}
	}
		
	//strFileName.Format("%s\\conf\\",szDirTmp);	
	strFileName.Format("%s\\",szDirTmp);	
	
	/*
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	*/

	sprintf(m_szConfPath,"%s\\server_conf.ini", (LPCSTR)strFileName);

	//TRACE("m_szConfPath [%s]\n", m_szConfPath);
}	

CINIManager::~CINIManager()
{	
	mINIManagerMap.RemoveAll();
	
}

//////////////////////////////////////////////////////////////////////
// CINIManger Functions
/* inline
CINIManager& CINIManager::Instance()
{
	// Meyer 싱글톤 사용 .. 	
	static CINIManager obj;
	return obj;
}
*/

// INI 파일에서 정보를 읽어온다.
void CINIManager::LoadINIFile()
{
	char szBuff[100];
	
	//Server Info
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_PORT, "0", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_PORT, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_IP, "0", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_IP, szBuff);

	//DB SID
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_DB_SID, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_DB_SID, szBuff);

	//DB USER
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_DB_USER, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_DB_USER, szBuff);

	//DB PASS
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_DB_PASS, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_DB_PASS, szBuff);
}


void CINIManager::SaveINIFile()
{
	CString strOutVal;
	//Server Info
	if(mINIManagerMap.Lookup(INI_SERVER_PORT,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_PORT, strOutVal, m_szConfPath); 
	}
	
	if(mINIManagerMap.Lookup(INI_SERVER_IP,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_IP, strOutVal, m_szConfPath); 
	}	

	//DB SID
	if(mINIManagerMap.Lookup(INI_SERVER_DB_SID,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_DB_SID, strOutVal, m_szConfPath); 
	}	

	//DB USER
	if(mINIManagerMap.Lookup(INI_SERVER_DB_USER,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_DB_USER, strOutVal, m_szConfPath); 
	}	
	//DB USER
	if(mINIManagerMap.Lookup(INI_SERVER_DB_PASS,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_DB_PASS, strOutVal, m_szConfPath); 
	}	
}

void CINIManager::GetValue(const char* szFindKey, CString& strOutVal)
{	
	if (mINIManagerMap.Lookup(szFindKey,  strOutVal ))
	{
		//TRACE("strFindKey [%s] strOutVal [%s]" , strFindKey, strOutVal );
	}
	else{
		strOutVal ="";
	}
}

int  CINIManager::GetValueInt(char* szFindKey)
{
	CString strOutTmp;
	if (mINIManagerMap.Lookup(szFindKey,  strOutTmp ))
	{		
		return atoi( (LPCSTR)strOutTmp);
	}

	return CAWAY_DEFAULT;
}


void CINIManager::SetValue( char* szFindKey, char* szVal)
{	
	mINIManagerMap.SetAt(szFindKey,  szVal );
}
