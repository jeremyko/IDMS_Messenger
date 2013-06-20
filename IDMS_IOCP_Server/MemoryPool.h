// MemoryPool.h: interface for the CMemoryPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYPOOL_H__67A96FC4_524D_44AE_B38F_48BC4FD90248__INCLUDED_)
#define AFX_MEMORYPOOL_H__67A96FC4_524D_44AE_B38F_48BC4FD90248__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class COracleWorkerMT;
class COracleWorkerMTTran;

class CMemoryPool  
{
public:
	CMemoryPool();
	virtual ~CMemoryPool();

private:
	static CRITICAL_SECTION	mx_CS_COracleWorkerMT;		
	static CPtrList m_COracleWorkerMTPtrList;			
	static void DeleteAll_COracleWorkerMT();
	
public:
	static void AllDeleteMemory();		///< 모든 메모리 해제
	static void SetupMemoryPool ();
	static COracleWorkerMT* New_COracleWorkerMT();
	static BOOL Delete_COracleWorkerMT(COracleWorkerMT* pMemory);
};

#endif // !defined(AFX_MEMORYPOOL_H__67A96FC4_524D_44AE_B38F_48BC4FD90248__INCLUDED_)
