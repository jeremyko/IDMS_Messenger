// DBWrapper.cpp: implementation of the CDBWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBWrapper.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBWrapper::CDBWrapper()
{
	// init member variable
	//m_pDatabase		= NULL;
	m_pRecordSet	= NULL;
	m_bUseTrans		= FALSE;
}

CDBWrapper::~CDBWrapper()
{
	// close db
	//Close();
	TRACE("CDBWrapper::~CDBWrapper()\r\n");
}

/************************************************************
 *	function desc	: db open
 *  return value	: 0 and below = fail
 *					  above 0	  = success	 
 *  written by		: Jerry(011-9261-3979) 
 *  last edit		: 2004-02-11
 ************************************************************/
/*
BOOL CDBWrapper::Open()
{
	m_pDatabase =  COraObjOleDB::Instance();

	
	if (m_pDatabase)
	{		
		return TRUE;
	}else
		return FALSE;
}
*/

/************************************************************
 *	function desc	: db close
 *  return value	: 0 and below = fail
 *					  above 0	  = success	 
 *  written by		: kojh
 *  last edit		: 2004-11-08
 ************************************************************/
BOOL CDBWrapper::Close()
{
	// trans check
	if( m_bUseTrans )
	{
		CommitTrans();
	}

	COraObjOleDB::Instance().m_Session.Close();
	COraObjOleDB::Instance().m_Dynaset.Close();
	COraObjOleDB::Instance().m_Database.Close();

	OShutdown();	

	return TRUE;
}

/************************************************************
 *	function desc	: run sql query
 *  return value	: 0 and below = fail
 *					  above 0	  = success	 
 *  written by		: Jerry(011-9261-3979) 
 *  last edit		: 2004-02-11
 ************************************************************/
BOOL CDBWrapper::RunSql(CString strQuery)
{
	strQuery.TrimLeft(' ');
	strQuery.TrimLeft('\t');

	strQuery.TrimLeft();
	CString strQueryType = strQuery.Mid(0, 6);
	
	strQueryType.MakeUpper();
	
	// select case 
	if( stricmp(strQueryType, "SELECT") == 0 )
	{		
		// 대소문자구분 없이 비교한다.	
		
		// 현재 RecordSet 이 1개이다 -> 추후 고려해야함!!
		if (COraObjOleDB::Instance().SelectSQLExecute(strQuery) != OSUCCESS)
		{
			// Oracle error
			COraObjOleDB::Instance().m_Dynaset.Close();	
			
			long oraerr = COraObjOleDB::Instance().GetOraErrNo();
			const char *dberrs = COraObjOleDB::Instance().GetOraErrMessage();

			AfxMessageBox(dberrs);  
			return FALSE;
		}		
	// other case(update,insert,delete..)
	}else{
		if (COraObjOleDB::Instance().SQLExecute(strQuery) != OSUCCESS){
			// Oracle error
			COraObjOleDB::Instance().m_Dynaset.Close();
			long oraerr = COraObjOleDB::Instance().GetOraErrNo();
			const char *dberrs = COraObjOleDB::Instance().GetOraErrMessage();

			AfxMessageBox(dberrs);  
			return FALSE;
		}		
	}
	
	m_pRecordSet = COraObjOleDB::Instance().GetDynaSet() ;
	//m_pSession = m_pDatabase->GetSession();
	return TRUE;
}



/************************************************************
 *	function desc	: get field data by field name
 *  return value	: data is null = null
 *					  data is no null = field string value
 *  written by		: Jerry(011-9261-3979, lys0913@hotmail.com)
 *  last edit		: 2004-02-11
 ************************************************************/
CString CDBWrapper::GetData(CString strFieldName, int nByte)
//CString CDBWrapper::GetData(CString strFieldName)
{
	char*	pBuf;
	int		nLen;
	CString	strData;

	nLen = this->m_pRecordSet->GetFieldSize(strFieldName) + 1;
	pBuf = new char[nLen];
	memset(pBuf, 0, nLen);

	COraObjOleDB::Instance().m_Dynaset.GetFieldValue(strFieldName, pBuf, nLen);
	strData.Format("%s", pBuf);

	delete[] pBuf;

	return strData;
//	CString tmpStr;
//	m_pDatabase->m_Dynaset.GetFieldValue(strFieldName, tmpStr.GetBuffer(nByte), nByte);
//	tmpStr.ReleaseBuffer();
//	return 	tmpStr;
}

/************************************************************
 *	function desc	: get field data by field index
 *  return value	: data is null = null
 *					  data is no null = field string value
 *  written by		: Jerry(011-9261-3979, lys0913@hotmail.com)
 *  last edit		: 2004-02-11
 ************************************************************/
CString CDBWrapper::GetData(int nFieldIndex, int nByte)
//CString CDBWrapper::GetData(int nFieldIndex)
{
	char*	pBuf;
	int		nLen;
	CString	strData;

	nLen = this->m_pRecordSet->GetFieldSize(nFieldIndex) + 1;
	pBuf = new char[nLen];
	memset(pBuf, 0, nLen);

	COraObjOleDB::Instance().m_Dynaset.GetFieldValue(nFieldIndex, pBuf, nLen);
	strData.Format("%s", pBuf);

	delete[] pBuf;

	return strData;
//	CString tmpStr;
//	m_pDatabase->m_Dynaset.GetFieldValue(nFieldIndex, tmpStr.GetBuffer(nByte), nByte);
//	tmpStr.ReleaseBuffer();
//	return 	tmpStr;	
}

int CDBWrapper::GetFieldCount()
{
	return COraObjOleDB::Instance().m_Dynaset.GetFieldCount();
}

long CDBWrapper::GetRecordCount()
{
	long nReturn = COraObjOleDB::Instance().m_Dynaset.GetRecordCount();

	return nReturn;
}

BOOL CDBWrapper::BeginTrans()
{
	m_bUseTrans = FALSE;

	if( COraObjOleDB::Instance().BeginTransaction() == OFAILURE )
	{		
		const char *dberrs = COraObjOleDB::Instance().m_Session.GetServerErrorText() ;
		AfxMessageBox(dberrs);  
	}else{
		m_bUseTrans = TRUE;
	}

	return m_bUseTrans;
}
	
BOOL CDBWrapper::CommitTrans()
{
	m_bUseTrans;

	m_bUseTrans = FALSE;

	if( COraObjOleDB::Instance().Commit() != OSUCCESS )
	{
		const char *dberrs = COraObjOleDB::Instance().m_Session.GetServerErrorText() ;
		AfxMessageBox(dberrs);  

		return FALSE;
	}
	
	return TRUE;
}

BOOL CDBWrapper::Rollback()
{
	m_bUseTrans = FALSE;

	if( COraObjOleDB::Instance().Rollback() != OSUCCESS )
	{
		const char *dberrs = COraObjOleDB::Instance().m_Session.GetServerErrorText() ;
		AfxMessageBox(dberrs);  

		return FALSE;
	}

	return TRUE;
}

BOOL CDBWrapper::IsTransaction()
{
	return m_bUseTrans;
}