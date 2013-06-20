// MemoryPool.cpp: implementation of the CMemoryPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDMS_server.h"
#include "MemoryPool.h"
#include "OraMultithrd.h"

#include "log.h"

//const int   DEFAULT_CAPACITY       = 5;
const int   DEFAULT_CAPACITY       = 1000; 

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
	
// 	::InitializeCriticalSection(&mx_CS_COracleWorkerMT);
// 		
// 	for(int i=0; i < DEFAULT_CAPACITY; i++)
// 	{
// 		COracleWorkerMT* pData = new COracleWorkerMT;
// 		pData->m_nindex = i ;
// 		m_COracleWorkerMTPtrList.AddTail(pData);		
// 	}
	
}

void CMemoryPool::AllDeleteMemory()
{	
	
// 	DeleteAll_COracleWorkerMT();
// 	::DeleteCriticalSection(&mx_CS_COracleWorkerMT);
	
}

COracleWorkerMT* CMemoryPool::New_COracleWorkerMT()
{		
	COracleWorkerMT* pData = new COracleWorkerMT;
	return pData ;
	
// 	EnterCriticalSection(&mx_CS_COracleWorkerMT);
// 	
// 	int nCnt = m_COracleWorkerMTPtrList.GetCount();
// 
// 	if(nCnt == 0)
// 	{		
// 		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT 에 비었다. 다시할당 시작! \n" );		
// 
// 		for(int i=0; i < DEFAULT_CAPACITY; i++)
// 		{
// 			COracleWorkerMT* pData = new COracleWorkerMT;
// 			pData->m_nindex = i ;
// 			m_COracleWorkerMTPtrList.AddTail(pData);			
// 		}
// 		//Log					
// 		
// 		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT 다시할당 종료! \n" );		
// 
// 		COracleWorkerMT* pOraData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.RemoveHead();
// 		
// 		LeaveCriticalSection(&mx_CS_COracleWorkerMT);
// 		
// 		//Log					
// 		CLog::Instance().WriteFile(FALSE, "MemoryPool",  "New_COracleWorkerMT 다시할당된 메모리를 돌려준다! ★ \n" );
// 		
// 		return pOraData; // 동기화 문제에 대해 => list에서 삭제되는 경우가 없기 때문에 무방하다.
// 	}
// 	else
// 	{		
// 		COracleWorkerMT* pOraData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.RemoveHead();
// 				
// 		LeaveCriticalSection(&mx_CS_COracleWorkerMT); // 이것은 다른 쓰레드들이 요소를 삭제하거나 직접 변경하는 경우가 없기 때문에 가능하다..
// 
// 		return pOraData;
// 	}	
// 	
// 	LeaveCriticalSection(&mx_CS_COracleWorkerMT);	

}


BOOL CMemoryPool::Delete_COracleWorkerMT(COracleWorkerMT* pMemory)
{
	
// 	EnterCriticalSection(&mx_CS_COracleWorkerMT); //20061106
// 		
// 	if(pMemory == NULL)
// 	{
// 		LeaveCriticalSection(&mx_CS_COracleWorkerMT);
// 		return FALSE;
// 	}	
// 	
// 	m_COracleWorkerMTPtrList.AddTail((COracleWorkerMT*)pMemory);
// 
// 	LeaveCriticalSection(&mx_CS_COracleWorkerMT);
	

	//delete pMemory;
	//pMemory = NULL;
	
	return TRUE;	
}


void CMemoryPool::DeleteAll_COracleWorkerMT()
{	
	
// 	EnterCriticalSection(&mx_CS_COracleWorkerMT);
// 			
// 	COracleWorkerMT *pSData;
// 	POSITION clearPos ;
// 
// 	clearPos = m_COracleWorkerMTPtrList.GetTailPosition();
// 
// 	while( clearPos != NULL )
// 	{
// 		pSData = (COracleWorkerMT*) m_COracleWorkerMTPtrList.GetAt( clearPos ); 
// 		if(pSData && (DWORD)pSData != 0xfeeefeee )
// 		{
// 			delete pSData;
// 			pSData = NULL;
// 			m_COracleWorkerMTPtrList.RemoveTail();
// 
// 			clearPos = m_COracleWorkerMTPtrList.GetTailPosition();
// 		}
// 	}
// 
// 	m_COracleWorkerMTPtrList.RemoveAll();
// 
// 	LeaveCriticalSection(&mx_CS_COracleWorkerMT);
	
}

