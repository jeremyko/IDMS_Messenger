// INIManager.h: interface for the CINIManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_)
#define AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CINIManager  
{
protected:
	CINIManager();
	

protected:	
	char				m_szModule[255];
	CStringArray		m_StrKeyAry;	
	CMapStringToString	mINIManagerMap;         
	CMapStringToString	mSectionMap; 
	char m_szConfPath[512];	
	char m_szIniFile [255];	

// Functions
public:

	virtual ~CINIManager();

	void LoadINIFile();
	void SaveINIFile();				
	void SetKeySection(CString& strKey, CString& strSec);
	void SetIniFile( const char* szModule, const char* szIniFile);
	
	static CINIManager & Instance();
	void GetValue( const char* szFindKey, CString& strOutVal);
	void SetValue( const char* szFindKey, const char* szVal);	
};

#endif // !defined(AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_)
