// INIManager.h: interface for the CINIManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_)
#define AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

class CINIManager  
{
protected:
	CINIManager();
	

protected:	
	CMapStringToString	mINIManagerMap;         
	char m_szConfPath[512];

// Functions
public:

	virtual ~CINIManager();

	void LoadINIFile();
	void SaveINIFile();

	
	static CINIManager & Instance()
	{
		static CINIManager obj;
		return obj;
	};
	void GetValue( const char* szFindKey, CString& strOutVal);
	void SetValue( char* szFindKey, char* szVal);

	int  GetValueInt(char* szFindKey);
	//void GetPort(char* szOutPort);
};

#endif // !defined(AFX_INIMANAGER_H__0252E729_FA53_42CA_804F_30CEC5EC240C__INCLUDED_)
