// INIManager.cpp: implementation of the CINIManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "INIManager.h"
#include "ChatSession.h"
#include "..\\common\\AllCommonDefines.h"
extern "C"
{
    #include <direct.h>
}
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CINIManager::CINIManager()
{	
	memset(m_szConfPath, 0x00, sizeof(m_szConfPath));
	memset(m_szConfPathDB, 0x00, sizeof(m_szConfPathDB));
	
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));
	
	GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

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
	//strFileName.Format("%s",szDirTmp);	
	
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}	
	
	strFileName.Format("%s\\ini",szDirTmp);	
	sprintf(m_szConfPath,"%s\\conf.ini", (LPCSTR)strFileName);
	
	sprintf(m_szConfPathDB,"%s\\InitInfo.ini", (LPCSTR)strFileName);
    
	//sprintf(m_szConfPath,"%s\\conf.ini", szDirTmp);
	TRACE("m_szConfPath [%s] m_szConfPathDB [%s]\n", m_szConfPath, m_szConfPathDB);
}	

CINIManager::~CINIManager()
{	
	mINIManagerMap.RemoveAll();
	
}

//////////////////////////////////////////////////////////////////////
// CINIManger Functions

CINIManager& CINIManager::Instance()
{
	static CINIManager obj;
	return obj;
}

// INI 파일에서 정보를 읽어온다.
void CINIManager::LoadINIFile()
{
	char szBuff[1024];
	
	//LogIn
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_LOGIN_SEC, INI_LOGIN_ID, "0", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_LOGIN_ID, szBuff);		

	//Server Info
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_PORT, "0", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_PORT, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SERVER_INFO, INI_SERVER_IP, "0", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SERVER_IP, szBuff);	

	//
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_ME_R, "7", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_ME_R, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_ME_G, "190", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_ME_G, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_ME_B, "67", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_ME_B, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_COMP_R, "243", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_COMP_R, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_COMP_G, "146", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_COMP_G, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_COMP_B, "73", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_COMP_B, szBuff);	

	//BackGround
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_BG_R, "26", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_BG_R, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_BG_G, "4", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_BG_G, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_CONVER_COLOR, INI_COLOR_BG_B, "74", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_COLOR_BG_B, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_TIME_CONF, INI_AWAY_SEC, "180", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_AWAY_SEC, szBuff);	

	//자동 로그인 
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_AUTO_LOGIN, INI_USE_AUTO_LOGIN, "N", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_AUTO_LOGIN, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_AUTO_LOGIN, INI_PASSWD, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_PASSWD, szBuff);	

	// 항상위
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_ALWAYS_ON_TOP, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_ALWAYS_ON_TOP, szBuff);	

	// 이전 실행크기, 위치
	memset(szBuff, 0x00, sizeof(szBuff));
		// 메인
	::GetPrivateProfileString(INI_SEC_WINPOS, INI_PREV_POS_SIZE_MAIN, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_PREV_POS_SIZE_MAIN, szBuff);	
		// chat 
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_WINPOS, INI_PREV_POS_SIZE_CHAT, "762|118|416|351|", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_PREV_POS_SIZE_CHAT, szBuff);	
		// chat splitter
	//memset(szBuff, 0x00, sizeof(szBuff));
	//::GetPrivateProfileString(INI_SEC_WINPOS, INI_PREV_POS_CHAT_SPLITTER, "" /*"3|149|410|159|0|"*/, szBuff, sizeof(szBuff), m_szConfPath); 
	//mINIManagerMap.SetAt(INI_PREV_POS_CHAT_SPLITTER, szBuff);	

	// 나의 이미지 파일 경로 
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_MY_IMAGE_PATH, "DefaultMe.jpg", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_MY_IMAGE_PATH, szBuff);	

	// DB 로그인 구분
	memset(szBuff, 0x00, sizeof(szBuff));
	//::GetPrivateProfileString(INI_SEC_DB, INI_DB_KIND, "", szBuff, sizeof(szBuff), m_szConfPath  ); 
	//mINIManagerMap.SetAt(INI_DB_KIND, szBuff);	
	
	//20070117
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString("SETTING", "SERVER", "", szBuff, sizeof(szBuff), m_szConfPathDB  ); 
	mINIManagerMap.SetAt("SERVER", szBuff);	

	//20070317 파일 저장 위치
	//INI_USE_DOWNFOLDER , INI_DOWNFOLDER_PATH
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_USE_DOWNFOLDER, "N", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_DOWNFOLDER, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_DOWNFOLDER_PATH, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_DOWNFOLDER_PATH, szBuff);

	//	
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_USE_LOGFOLDER, "N", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_LOGFOLDER, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_LOGFOLDER_PATH, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_LOGFOLDER_PATH, szBuff);

	//Sound
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SOUND, INI_USE_SOUND, "N", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_SOUND, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SOUND, INI_SOUND_PATH, "notify.wav", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_SOUND_PATH, szBuff);	
		
	// 그룹 Folding 상태 (Floded 된 그룹만)
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_OPTION, INI_GRP_FOLDED, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_GRP_FOLDED, szBuff);

	////////////////////////////////////////////////////////////////////////////
	//Reply List
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST0, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST0, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST1, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST1, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST2, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST2, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST3, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST3, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST4, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST4, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST5, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST5, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST6, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST6, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST7, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST7, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST8, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST8, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_REPLY, INI_REPLY_LIST9, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_REPLY_LIST9, szBuff);	

	// 자동부착글 
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_AUTO_APPEND, INI_USE_AUTO_APPEND, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_AUTO_APPEND, szBuff);	

	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_AUTO_APPEND, INI_AUTO_APPEND_MSG, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_AUTO_APPEND_MSG, szBuff);	

	// Select Send
	memset(szBuff, 0x00, sizeof(szBuff));
	::GetPrivateProfileString(INI_SEC_SELECT_SEND, INI_USE_SELECT_SEND, "", szBuff, sizeof(szBuff), m_szConfPath); 
	mINIManagerMap.SetAt(INI_USE_SELECT_SEND, szBuff);	
	
}


void CINIManager::SaveINIFile()
{
	CString strOutVal;

	//LogIn
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_LOGIN_ID,  strOutVal ))
	{
		if( !strOutVal.IsEmpty() )
		{
			::WritePrivateProfileString (INI_LOGIN_SEC, INI_LOGIN_ID, strOutVal, m_szConfPath); 
		}
	}

	//Server Info
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_SERVER_PORT,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_PORT, strOutVal, m_szConfPath); 
	}
	
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_SERVER_IP,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SERVER_INFO, INI_SERVER_IP, strOutVal, m_szConfPath); 
	}

	//Me
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_ME_R,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_ME_R, strOutVal, m_szConfPath); 
	}
	
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_ME_G,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_ME_G, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_ME_B,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_ME_B, strOutVal, m_szConfPath); 
	}
	
	//Comp
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_COMP_R,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_COMP_R, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_COMP_G,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_COMP_G, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_COMP_B,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_COMP_B, strOutVal, m_szConfPath); 
	}
	
	//BG
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_BG_R,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_BG_R, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_BG_G,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_BG_G, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_COLOR_BG_B,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_CONVER_COLOR, INI_COLOR_BG_B, strOutVal, m_szConfPath); 
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_AWAY_SEC,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_TIME_CONF, INI_AWAY_SEC, strOutVal, m_szConfPath); 
	}

	// 암호화된 비밀번호
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_USE_AUTO_LOGIN,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_AUTO_LOGIN, INI_USE_AUTO_LOGIN, strOutVal, m_szConfPath); 
		
		if( !CChatSession::Instance().m_decrpPassWd.IsEmpty() )
		{
			::WritePrivateProfileString (INI_SEC_AUTO_LOGIN, INI_PASSWD, CChatSession::Instance().m_decrpPassWd, m_szConfPath); 
		}
	}

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_ALWAYS_ON_TOP,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_OPTION, INI_ALWAYS_ON_TOP, strOutVal, m_szConfPath); 
	}

	// Window position
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_PREV_POS_SIZE_MAIN,  strOutVal ))
	{
		//100|200|300|400 
		::WritePrivateProfileString (INI_SEC_WINPOS, INI_PREV_POS_SIZE_MAIN, strOutVal, m_szConfPath); 
	}	
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_PREV_POS_SIZE_CHAT,  strOutVal ))
	{
		//100|200|300|400 
		::WritePrivateProfileString (INI_SEC_WINPOS, INI_PREV_POS_SIZE_CHAT, strOutVal, m_szConfPath); 
	}	

	/*
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_PREV_POS_CHAT_SPLITTER,  strOutVal ))
	{
		//100|200|300|400 
		::WritePrivateProfileString (INI_SEC_WINPOS, INI_PREV_POS_CHAT_SPLITTER, strOutVal, m_szConfPath); 
	}	
	*/

	// 나의 이미지 파일
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_MY_IMAGE_PATH,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_OPTION, INI_MY_IMAGE_PATH, strOutVal, m_szConfPath); 
	}	
	
	//20070317 파일 저장 위치
	//INI_USE_DOWNFOLDER , INI_DOWNFOLDER_PATH
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_USE_DOWNFOLDER,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_OPTION, INI_USE_DOWNFOLDER, strOutVal, m_szConfPath); 
	}	

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_DOWNFOLDER_PATH,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_OPTION, INI_DOWNFOLDER_PATH, strOutVal, m_szConfPath); 
	}	

	//
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_USE_LOGFOLDER,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_OPTION, INI_USE_LOGFOLDER, strOutVal, m_szConfPath); 
	}	

	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_LOGFOLDER_PATH,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_OPTION, INI_LOGFOLDER_PATH, strOutVal, m_szConfPath); 
	}	
	
	//Sound
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_USE_SOUND,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_SOUND, INI_USE_SOUND, strOutVal, m_szConfPath); 
	}	
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_SOUND_PATH,  strOutVal ))
	{	
		::WritePrivateProfileString (INI_SEC_SOUND, INI_SOUND_PATH, strOutVal, m_szConfPath); 
	}	

	//DB
	/*
	if(mINIManagerMap.Lookup(INI_SEC_DB,  strOutVal ))
	{		
		::WritePrivateProfileString (INI_SEC_DB, INI_DB_KIND, strOutVal, m_szConfPath); 
	}
	*/
	
	strOutVal.Empty();
	if(mINIManagerMap.Lookup("SETTING",  strOutVal ))
	{		
		::WritePrivateProfileString ("SETTING", "SERVER", strOutVal, m_szConfPathDB); 
	}

	// Floded grp
	strOutVal.Empty();
	if(mINIManagerMap.Lookup(INI_GRP_FOLDED,  strOutVal ))
	{		
		::WritePrivateProfileString (INI_SEC_OPTION, INI_GRP_FOLDED, strOutVal, m_szConfPath); 
	}

	//////////////////////////////////////////////////////////////////////////////////
	//Reply List
	if(mINIManagerMap.Lookup(INI_REPLY_LIST0,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST0, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST1,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST1, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST2,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST2, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST3,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST3, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST4,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST4, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST5,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST5, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST6,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST6, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST7,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST7, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST8,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST8, strOutVal, m_szConfPath); 
	}

	if(mINIManagerMap.Lookup(INI_REPLY_LIST9,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_REPLY, INI_REPLY_LIST9, strOutVal, m_szConfPath); 
	}

	// 자동부착글 
	if(mINIManagerMap.Lookup(INI_USE_AUTO_APPEND,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_AUTO_APPEND, INI_USE_AUTO_APPEND, strOutVal, m_szConfPath); 
	}
	if(mINIManagerMap.Lookup(INI_AUTO_APPEND_MSG,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_AUTO_APPEND, INI_AUTO_APPEND_MSG, strOutVal, m_szConfPath); 
	}	

	//Select Send
	if(mINIManagerMap.Lookup(INI_USE_SELECT_SEND,  strOutVal ))
	{
		::WritePrivateProfileString (INI_SEC_SELECT_SEND, INI_USE_SELECT_SEND, strOutVal, m_szConfPath); 
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
