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
	CMapStringToString	mINIManagerMap;         
	char m_szConfPath[512];
	char m_szConfPathDB[512];	

// Functions
public:

	virtual ~CINIManager();

	void LoadINIFile();
	void SaveINIFile();

	
	static CINIManager & Instance();
	void GetValue( const char* szFindKey, CString& strOutVal);
	void SetValue( char* szFindKey, char* szVal);

	int  GetValueInt(char* szFindKey);
	//void GetPort(char* szOutPort);
};

#endif // !defined(AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_)
