// MemoryPool.cpp: implementation of the CMemoryPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDMS_server.h"
#include "MemoryPool.h"
#include "OraMultithrd.h"

#include "log.h"

//const int   DEFAULT_CAPACITY       = 5;
const int   DEFAULT_CAPACITY       = 200; 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPtrList			CMemoryPool::m_COracleWorkerMTPtrList;
CRITICAL_SECTION	CMemoryPool::mx_CS_COracleWorkerMT;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryPool::CMemoryPool()
{
	
}

CMemoryPool::~CMemoryPool()
{

}

void CMemoryPool::SetupMemoryPool ()
{
	::InitializeCriticalSection(&mx_CS_COracleWorkerMT);
		
	for(int i=0; i < DEFAULT_CAPACITY; i++)
	{
		COracleWorkerMT* pData = new COracleWorkerMT;
		pData->m_nindex = i ;
		m_COracleWorkerMTPtrList.AddTail(pData);		
	}
}

void CMemoryPool::AllDeleteMemory()
{	
	DeleteAll_COracleWorkerMT();
	::DeleteCriticalSection(&mx_CS_COracleWorkerMT);
}

COracleWorkerMT* CMemoryPool::New_COracleWorkerMT()
{		
	EnterCriticalSection(&mx_CS_COracleWorkerMT);
	
	int nCnt = m_COracleWorkerMTPtrList.GetCount();

	if(nCnt == 0)
	{		
		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT �� �����. �ٽ��Ҵ� ����! \n" );		

		for(int i=0; i < DEFAULT_CAPACITY; i++)
		{
			COracleWorkerMT* pData = new COracleWorkerMT;
			pData->m_nindex = i ;
			m_COracleWorkerMTPtrList.AddTail(pData);			
		}
		//Log					
		
		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT �ٽ��Ҵ� ����! \n" );		

		COracleWorkerMT* pOraData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.RemoveHead();
		
		LeaveCriticalSection(&mx_CS_COracleWorkerMT);
		
		//Log					
		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT �ٽ��Ҵ�� �޸𸮸� �����ش�! �� \n" );				

		
		return pOraData; // ����ȭ ������ ���� => list���� �����Ǵ� ��찡 ���� ������ �����ϴ�.
	}
	else
	{		
		COracleWorkerMT* pOraData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.RemoveHead();
		
		//CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT cnt [%d] index [%d] [%d]\n" , nCnt , pOraData->m_nindex , (DWORD)pOraData);	

		LeaveCriticalSection(&mx_CS_COracleWorkerMT); // �̰��� �ٸ� ��������� ��Ҹ� �����ϰų� ���� �����ϴ� ��찡 ���� ������ �����ϴ�..

		return pOraData;
	}	
	
	LeaveCriticalSection(&mx_CS_COracleWorkerMT);
}


BOOL CMemoryPool::Delete_COracleWorkerMT(COracleWorkerMT* pMemory)
{
	EnterCriticalSection(&mx_CS_COracleWorkerMT); //20061106
		
	if(pMemory == NULL)
	{
		LeaveCriticalSection(&mx_CS_COracleWorkerMT);
		return FALSE;
	}
	
	//int nCnt = m_COracleWorkerMTPtrList.GetCount();	
	//CLog::Instance().WriteFile(FALSE, "MemoryPool",  "Delete_COracleWorkerMT cnt [%d] index [%d] [%d]\n", nCnt, pMemory ->m_nindex ,(DWORD)pMemory );	

	m_COracleWorkerMTPtrList.AddTail((COracleWorkerMT*)pMemory);

	LeaveCriticalSection(&mx_CS_COracleWorkerMT);
	
	return TRUE;	
}


void CMemoryPool::DeleteAll_COracleWorkerMT()
{	
	EnterCriticalSection(&mx_CS_COracleWorkerMT);
			
	COracleWorkerMT *pSData;
	POSITION clearPos ;

	clearPos = m_COracleWorkerMTPtrList.GetTailPosition();

	while( clearPos != NULL )
	{
		pSData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.GetAt( clearPos ); 
		if(pSData && (DWORD)pSData != 0xfeeefeee )
		{
			delete pSData;
			pSData = NULL;
			m_COracleWorkerMTPtrList.RemoveTail();

			clearPos = m_COracleWorkerMTPtrList.GetTailPosition();
		}
	}

	m_COracleWorkerMTPtrList.RemoveAll();

	LeaveCriticalSection(&mx_CS_COracleWorkerMT);
}

