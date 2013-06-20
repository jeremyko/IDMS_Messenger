// OraObjectArray.h: interface for the COraObjectArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORAOBJECTARRAY_H__3A3BC48A_4E78_432C_A1A5_28847D2D01F5__INCLUDED_)
#define AFX_ORAOBJECTARRAY_H__3A3BC48A_4E78_432C_A1A5_28847D2D01F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include "oracl.h"
#include "utility.h"

// Oracle SessionÀ» °ü¸®.



// 20061104 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class COraSessionName
{
public:
	COraSessionName();
	virtual ~COraSessionName();
	
	static COraSessionName& Instance()
	{
		static COraSessionName obj;
		return obj;
	};	
	
	long getOraSessionIndex();	
		
	static long m_nCurIndex;	
	
};

#endif // !defined(AFX_ORAOBJECTARRAY_H__3A3BC48A_4E78_432C_A1A5_28847D2D01F5__INCLUDED_)
