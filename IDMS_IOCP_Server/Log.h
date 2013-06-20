#if !defined(AFX_LOG_H__BF418A40_8459_4734_96F7_5513A918FFBD__INCLUDED_)
#define AFX_LOG_H__BF418A40_8459_4734_96F7_5513A918FFBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Log.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLog window

class CLog : public CWnd
{
// Construction
private:		
	CLog();	
	CLog(HWND hOwnerWnd);	

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLog)
	//}}AFX_VIRTUAL

// Implementation
public:
	
	virtual ~CLog();
	static CLog& Instance() ;
	
	void SetLogPath( const char* szPath)
	{
		strncpy(m_szLogPath, szPath, sizeof(m_szLogPath));
	};

	char* GetLogPath()
	{
		return m_szLogPath ;
	};

	void WriteFile( BOOL bOverWrite, const char* filename, char *fmt,...);
		
	
	// Generated message map functions
protected:
	char m_szLogPath [1024];
	CString Log_fprintf(FILE *FD, const char *fmt, va_list ap);

	//{{AFX_MSG(CLog)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOG_H__BF418A40_8459_4734_96F7_5513A918FFBD__INCLUDED_)
