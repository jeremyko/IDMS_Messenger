// DBWrapper.h: interface for the CDBWrapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBWRAPPER_H__A400F246_8B5B_4B52_927A_285025B038F6__INCLUDED_)
#define AFX_DBWRAPPER_H__A400F246_8B5B_4B52_927A_285025B038F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxdb.h>

#include "OraObjOleDB.h"

class CDBWrapper  : CObject
{
public:
	CDBWrapper();
	virtual ~CDBWrapper();

//	DECLARE_DYNAMIC(CDBWrapper)
public:
			
	BOOL		Close			();
	long		GetRecordCount	();
	int			GetFieldCount	();
	BOOL		RunSql			(CString strQuery	 );
	CString		GetData			(CString strFieldName, int nByte = 255);
	CString		GetData			(int	 nFieldIndex , int nByte = 255);			

	// 트랜잭션관련
	BOOL		BeginTrans		();
	BOOL		CommitTrans		();
	BOOL		Rollback		();

public:
	BOOL IsTransaction();
	BOOL		m_bUseTrans;		
	ODynaset * m_pRecordSet;

protected:
	//CString	VariantToString(CDBVariant& rvarValue);

};

#endif // !defined(AFX_DBWRAPPER_H__A400F246_8B5B_4B52_927A_285025B038F6__INCLUDED_)
