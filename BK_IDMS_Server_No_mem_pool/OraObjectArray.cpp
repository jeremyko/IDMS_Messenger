// OraObjectArray.cpp: implementation of the COraObjectArray class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDMS_server.h"
#include "OraObjectArray.h"
#include "log.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


long COraSessionName::m_nCurIndex = 0; 

COraSessionName::COraSessionName()
{	
	 
}

COraSessionName::~COraSessionName()
{

}

long COraSessionName::getOraSessionIndex()
{	

	InterlockedIncrement(& m_nCurIndex);		

	if(m_nCurIndex > 100000)
	{
		CLog::Instance().WriteFile(FALSE, "COraObjectArray",  "getOraSessionIndex m_nCurIndex 100000 ÃÊ°ú => 0 ReSet! \n" );

		m_nCurIndex = 0;
	}

	return m_nCurIndex;
}