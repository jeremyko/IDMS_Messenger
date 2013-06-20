// OraObjOleDB.h: interface for the COraObjOleDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORAOBJOLEDB_H__E6B08600_9DBB_4946_B99D_85D883F725FF__INCLUDED_)
#define AFX_ORAOBJOLEDB_H__E6B08600_9DBB_4946_B99D_85D883F725FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "oracl.h"
#pragma comment(lib, "oraclm32.lib")

class COraObjOleDB  : public CObject
{
//	DECLARE_DYNCREATE(COraObjOleDB)
public:
	COraObjOleDB();
	virtual ~COraObjOleDB();

	BOOL	DBConnect(CString strSID, CString strID, CString strPW);
	//static COraObjOleDB* Instance();
	static COraObjOleDB& Instance();	
	long GetOraErrNo();
	const char* GetOraErrMessage();
	
	//! INSERT, UPDATE 에 사용한다
	int SQLExecute(const char *sqlstmt)
	{
		return m_Database.ExecuteSQL(sqlstmt);
	}; 

	//! SELECT 사용
	int SelectSQLExecute( CString sqlstmt);	

	//! ODynaset 을 임시 생성후 인자로 전달 방식 (-> Multi ODynaset)
	//int SelectSQLExecuteMultiSet(ODynaset * pDynaset, CString sqlstmt)
	//{
	//	return pDynaset->Open(m_Database, sqlstmt);
	//}; 

	//! Select 질의에 대한 레코드 갯수 반환
	int GetSlectSQLRecordCount()
	{
		return m_Dynaset.GetRecordCount(); 
	};

	//! start a transaction
	int  BeginTransaction(void)
	{
		return m_Session.BeginTransaction();
	}; 
	
	//! commit (may start new transaction)
    int  Commit(oboolean startnew = FALSE)
	{
		return m_Session.Commit(startnew);
	};   

	//! rolls back transaction (may start new transaction)
    int  Rollback(oboolean startnew = FALSE)
	{
		return m_Session.Rollback(startnew);
	}; 
	
	ODynaset*  GetDynaSet()
	{
		return &m_Dynaset;
	}; 

	OSession* GetSession()
	{
		return & m_Session;
	};

	//! ORACL.H 참조하시오
	ODatabase		m_Database;	
	OSession		m_Session;	
	ODynaset		m_Dynaset;	

};

#endif // !defined(AFX_ORAOBJOLEDB_H__E6B08600_9DBB_4946_B99D_85D883F725FF__INCLUDED_)
