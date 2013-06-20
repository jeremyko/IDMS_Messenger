#pragma once

#include <imagehlp.h>
//#include <DbgHelp.h>

#ifndef _BASELIB
# pragma  comment(lib, "dbghelp.lib")
#endif	_BASELIB

// from msjexhnd
// modified by kue [9/4/2005] 

class ExceptionHandler
{
public:
	ExceptionHandler();
	~ExceptionHandler();


public:
	void			Init(DWORD	version, _MINIDUMP_TYPE dumpType = MiniDumpWithDataSegs);
	_MINIDUMP_TYPE	ChangeMiniDumpType(_MINIDUMP_TYPE dumpType);

	static void		GetNextDumpFileName(CHAR* buffer, INT size);
	static bool		CreateDumpFile(PEXCEPTION_POINTERS pExceptionInfo, _MINIDUMP_TYPE minidumpType = MiniDumpWithDataSegs, LPCSTR dumpFileName = NULL);
	static void		WriteReportFile(PEXCEPTION_POINTERS pExceptionInfo, LPCSTR dumpFileName);
	static void		ClearDumpFolder();


	// entry point where control comes on an unhandled exception
	static LONG WINAPI	UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

private:
	DWORD			m_version;
	_MINIDUMP_TYPE	m_dumpType;
	CHAR			m_logFileName[1024];
	CHAR			m_dumpFolderPath[1024];
	CHAR			m_moduleName[1024];
	static volatile LONG m_crashCounter;


public:
	// Helper functions
	static bool			InitImagehlpFunctions();
	static void			ImagehlpStackWalk(PCONTEXT pContext, FILE* fp = NULL, FILE* fpLast = NULL);
	static void			IntelStackWalk(PCONTEXT pContext, FILE* fp = NULL, FILE* fpLast = NULL);
	static LPSTR		GetExceptionString(DWORD dwCode);
	static bool			GetLogicalAddress(PVOID addr, LPSTR szModule, DWORD len, DWORD& section, DWORD& offset);


private:
	static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;

	// for some IMAGEHLP.DLL functions so that we can use them with GetProcAddress
	typedef bool (__stdcall *SYMINITIALIZEPROC)(HANDLE, LPSTR, bool);
	typedef bool (__stdcall *SYMCLEANUPPROC)(HANDLE);
	typedef bool (__stdcall *SYMGETSYMFROMADDRPROC)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
	typedef bool (__stdcall *STACKWALKPROC)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE, 
											PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
	typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)(HANDLE, DWORD);
	typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)(HANDLE, DWORD);

	static HMODULE						m_hModImagehlp;
	static STACKWALKPROC				m_stackWalk;
	static SYMCLEANUPPROC				m_symCleanup;
	static SYMINITIALIZEPROC			m_symInitialize;
	static SYMGETMODULEBASEPROC			m_symGetModuleBase;
	static SYMGETSYMFROMADDRPROC		m_symGetSymFromAddr;
	static SYMFUNCTIONTABLEACCESSPROC	m_symFunctionTableAccess;
};



extern ExceptionHandler exceptionHandler;