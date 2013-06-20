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

// VC6 ������ ����ȭ max speed ����ϸ� singleton �ȵ�
COraObjOleDB& COraObjOleDB::Instance()
{	
	static COraObjOleDB obj;
	
	return obj;
}


BOOL COraObjOleDB::DBConnect(CString strSID, CString strID, CString strPW)
{
	// ODataBase, OSession �� Oracle Calss Lib �� �ʱ�ȭ...
	if( OStartup() == OFAILURE )
	{
		OShutdown();
		TRACE("DB �ʱ�ȭ�� �����Ͽ����ϴ�");
		return FALSE;
	}

	//DB open
	if(m_Database.Open(strSID, strID, strPW ) == OFAILURE)	
	{
		OShutdown();
		TRACE("DB ���ῡ ���� �Ͽ����ϴ�");
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
