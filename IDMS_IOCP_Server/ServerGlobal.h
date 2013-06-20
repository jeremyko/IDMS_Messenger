#if !defined(AFX_SERVERGLOBAL_H__9C159D7C_6501_451B_BF1C_4331EEF8C8F2__INCLUDED_)
#define AFX_SERVERGLOBAL_H__9C159D7C_6501_451B_BF1C_4331EEF8C8F2__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// CServerGlobal window

class CServerGlobal
{
// Construction
protected:
	CServerGlobal();

public:
	CMapStringToString	mGlobalMap; 


public:
	virtual ~CServerGlobal();

	static CServerGlobal & Instance()
	{
		static CServerGlobal obj;
		return obj;
	};
	
	//void GetStrValue( const char* szFindKey, CString& strOutVal);
	//void SetStrValue( char* szFindKey, char* szVal);
	CString	m_strDBConn, m_strUser, m_strPassWd ;	
	// Generated message map functions
};


#endif // !defined(AFX_SERVERGLOBAL_H__9C159D7C_6501_451B_BF1C_4331EEF8C8F2__INCLUDED_)
