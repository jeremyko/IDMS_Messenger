// OraObjOleDB.cpp: implementation of the COraObjOleDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OraObjOleDB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COraObjOleDB::COraObjOleDB()
{

}

COraObjOleDB::~COraObjOleDB()
{
	m_Session.Close();
	m_Dynaset.Close();
	m_Database.Close();

	OShutdown();	
}

int COraObjOleDB::SelectSQLExecute( CString sqlstmt)
{
	return m_Dynaset.Open(m_Database, sqlstmt);	
}

// VC6 에서는 최적화 max speed 사용하면 singleton 안됨
COraObjOleDB& COraObjOleDB::Instance()
{	
	static COraObjOleDB obj;
	
	return obj;
}


BOOL COraObjOleDB::DBConnect(CString strSID, CString strID, CString strPW)
{
	// ODataBase, OSession 등 Oracle Calss Lib 의 초기화...
	if( OStartup() == OFAILURE )
	{
		OShutdown();
		TRACE("DB 초기화에 실패하였습니다");
		return FALSE;
	}

	//DB open
	if(m_Database.Open(strSID, strID, strPW ) == OFAILURE)	
	{
		OShutdown();
		TRACE("DB 연결에 실패 하였습니다");
		return FALSE;
	}

	m_Session = m_Database.GetSession();

	return TRUE;
}

long COraObjOleDB::GetOraErrNo()
{
	return (long) m_Database.ServerErrorNumber();
}

const char* COraObjOleDB::GetOraErrMessage()
{
	return m_Database.GetServerErrorText();
}
