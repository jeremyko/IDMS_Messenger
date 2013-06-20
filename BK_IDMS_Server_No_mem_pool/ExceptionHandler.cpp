#include "StdAfx.h"
#include "ExceptionHandler.h"
//#include "SystemSpec.h"

#include <time.h>


#pragma warning(disable : 4311)
#pragma warning(disable : 4312)


//============================== Global Variables =============================

volatile LONG									ExceptionHandler::m_crashCounter			= 0;
LPTOP_LEVEL_EXCEPTION_FILTER					ExceptionHandler::m_previousFilter;
HMODULE											ExceptionHandler::m_hModImagehlp			= NULL;
ExceptionHandler::STACKWALKPROC					ExceptionHandler::m_stackWalk				= NULL;
ExceptionHandler::SYMCLEANUPPROC				ExceptionHandler::m_symCleanup				= NULL;
ExceptionHandler::SYMINITIALIZEPROC				ExceptionHandler::m_symInitialize			= NULL;
ExceptionHandler::SYMGETMODULEBASEPROC			ExceptionHandler::m_symGetModuleBase		= NULL;
ExceptionHandler::SYMGETSYMFROMADDRPROC			ExceptionHandler::m_symGetSymFromAddr		= NULL;
ExceptionHandler::SYMFUNCTIONTABLEACCESSPROC	ExceptionHandler::m_symFunctionTableAccess	= NULL;


ExceptionHandler exceptionHandler;

ExceptionHandler::ExceptionHandler(void)
{
	ZeroMemory(m_logFileName, sizeof(m_logFileName));
	ZeroMemory(m_dumpFolderPath, sizeof(m_dumpFolderPath));
	ZeroMemory(m_moduleName, sizeof(m_moduleName));

	strcpy(m_logFileName, "CrashLog.txt");

	m_version = 0;
	m_dumpType = MiniDumpWithDataSegs;

	// 일단 붙인다
	m_previousFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	
}

ExceptionHandler::~ExceptionHandler(void)
{
	if( m_previousFilter != NULL)
		SetUnhandledExceptionFilter(m_previousFilter);
}


void ExceptionHandler::Init(DWORD version, _MINIDUMP_TYPE dumpType)
{

	// 모듈 이름 얻어옴 sdopfp/BigShot.exe 면. BigShot 만 나옴
	CHAR  buffer[1024];
	LPCSTR pBuffer = NULL;
	GetModuleFileNameA(0, buffer, 1024);
	
	pBuffer = strrchr(buffer, '\\');
	if(pBuffer)
	{
		strcpy(m_moduleName, pBuffer + 1);
		*strrchr(m_moduleName, '.') = '\0';
	}


	// Dump 폴더 PATH 스트링을 구축한다
	strncpy(m_dumpFolderPath, buffer, strrchr(buffer, '\\') - buffer);
	strcat(m_dumpFolderPath, "\\Dump\\");


	// 로그 파일
	strcpy(m_logFileName, buffer);
	LPSTR pszDot = strrchr(m_logFileName, '.');
	if( pszDot )
	{
		pszDot++;											// Advance past the '.'
		if ( strlen(pszDot) >= 3 ) strcpy(pszDot, "rpt");   // "RPT" -> "Report"
	}

	// Dump 폴더가 없으믄 만든다
	::CreateDirectoryA(m_dumpFolderPath, NULL);

	m_version = version;
	m_dumpType = dumpType;
}

_MINIDUMP_TYPE ExceptionHandler::ChangeMiniDumpType(_MINIDUMP_TYPE dumpType)
{
	_MINIDUMP_TYPE lastType = m_dumpType;

	m_dumpType = dumpType;

	return lastType;
}

LONG WINAPI ExceptionHandler::UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	const INT DumpFilenameLen = 1024;
	CHAR dumpFilename[DumpFilenameLen] = {0};
	
	if( InterlockedIncrement(&ExceptionHandler::m_crashCounter) > 1 )
	{
		Sleep(60000);	// 60 초
		goto end;
	}

	// 다음 덤프 파일 이름 생성
	
	GetNextDumpFileName(dumpFilename, DumpFilenameLen);

	// 일단 rpt 파일 쓰면서 스택워킹
	WriteReportFile(pExceptionInfo, dumpFilename);

	// 그리고 덤프
	CreateDumpFile(pExceptionInfo, exceptionHandler.m_dumpType, dumpFilename);
	

end:
	if( m_previousFilter )
		return m_previousFilter(pExceptionInfo);
	else
		return EXCEPTION_CONTINUE_SEARCH;
}



void ExceptionHandler::GetNextDumpFileName(CHAR* buffer, INT size)
{
	// 001 부터 시작
	bool exist = TRUE;
	INT	 fileNum = 1;

	static CHAR dumpFileName[1024];
	ZeroMemory(dumpFileName, sizeof(dumpFileName));

	// Dump 폴더가 없으믄 만든다
	::CreateDirectoryA(exceptionHandler.m_dumpFolderPath, NULL);

	do{
		sprintf( dumpFileName, "%s%sV%04dN%03d.mdmp", exceptionHandler.m_dumpFolderPath, exceptionHandler.m_moduleName, exceptionHandler.m_version, fileNum);

		FILE* fp = fopen(dumpFileName, "r");
		if( fp ) fclose(fp);
		else	 exist = FALSE;

		// 같은 버전의 덤프파일을 1000개까지 만들 거라 생각하지 않아
		if( fileNum >= 1000 )
			return ;

		fileNum++;

	}while(exist);

	// 복사
	strncpy(buffer, dumpFileName, size - 1);
}

bool ExceptionHandler::CreateDumpFile(PEXCEPTION_POINTERS pExceptionInfo, _MINIDUMP_TYPE minidumpType /* = MiniDumpWithDataSegs */, LPCSTR dumpFileName /* = NULL */)
{
	if( dumpFileName == NULL )
	{
		static CHAR tempBuffer[1024] = {0};
		GetNextDumpFileName(tempBuffer, 1023);
		dumpFileName = tempBuffer;
	}

	HANDLE hDumpFile = CreateFileA(dumpFileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0);

	bool success = false;

	if( hDumpFile != INVALID_HANDLE_VALUE )
	{
		MINIDUMP_EXCEPTION_INFORMATION exinfo;
		exinfo.ExceptionPointers = pExceptionInfo;
		exinfo.ClientPointers	 = FALSE;
		exinfo.ThreadId			 = GetCurrentThreadId();

		success = ::MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
										 hDumpFile, minidumpType,&exinfo,NULL,NULL) != 0;

		::CloseHandle(hDumpFile);
	}

	return success;
}


void ExceptionHandler::ClearDumpFolder()
{
}

void ExceptionHandler::WriteReportFile(PEXCEPTION_POINTERS pExceptionInfo, LPCSTR dumpFileName)
{
	FILE* fp = fopen(exceptionHandler.m_logFileName, "a+t");
	setvbuf(fp, NULL, _IONBF, 0);
	
	CHAR filename[1024];
	sprintf(filename, "%sLastCrashReport.txt", exceptionHandler.m_dumpFolderPath);

	FILE* fpLast = fopen(filename, "wt");
	setvbuf(fpLast, NULL, _IONBF, 0);

	

	// print timestamp
	CHAR bufTime[16], bufDate[16];
	_strdate( bufDate ); _strtime( bufTime );
	fprintf(fp, "\r\n\r\n\r\n==================   logged at %s, %s    ==========================\r\n\r\n", bufDate, bufTime);
	fprintf(fpLast, "\r\n\r\n\r\n==================   logged at %s, %s    ==========================\r\n\r\n", bufDate, bufTime);

	fprintf(fp, "Version: %d\r\n", exceptionHandler.m_version);
	fprintf(fpLast, "Version: %d\r\n", exceptionHandler.m_version);

	fprintf(fp, "Dump File Name: %s\r\n", dumpFileName);
	fprintf(fpLast, "Dump File Name: %s\r\n", dumpFileName);


	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

	// First print information about the type of fault
	fprintf(fp, "Exception code: %08X %s\r\n", pExceptionRecord->ExceptionCode, GetExceptionString(pExceptionRecord->ExceptionCode));
	fprintf(fpLast, "Exception code: %08X %s\r\n", pExceptionRecord->ExceptionCode, GetExceptionString(pExceptionRecord->ExceptionCode));

	// Now print information about where the fault occured
	CHAR szFaultingModule[MAX_PATH];
	DWORD section, offset;
	GetLogicalAddress(pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof(szFaultingModule), section, offset);
	fprintf(fp, "Fault address:  %08X %02X:%08X %s\r\n", pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule);
	fprintf(fpLast, "Fault address:  %08X %02X:%08X %s\r\n", pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule);


	PCONTEXT pCtx = pExceptionInfo->ContextRecord;

	// Show the registers
#ifdef _M_IX86  // Intel Only!
	fprintf(fp, "\r\nRegisters:\r\n");

	fprintf(fp, "EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n",
		pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi);

	fprintf(fp, "CS:EIP:%04X:%08X\r\n", pCtx->SegCs, pCtx->Eip);
	fprintf(fp, "SS:ESP:%04X:%08X  EBP:%08X\r\n", pCtx->SegSs, pCtx->Esp, pCtx->Ebp);
	fprintf(fp, "DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs);
	fprintf(fp, "Flags:%08X\r\n", pCtx->EFlags);


	fprintf(fpLast, "\r\nRegisters:\r\n");

	fprintf(fpLast, "EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n",
		pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi);

	fprintf(fpLast, "CS:EIP:%04X:%08X\r\n", pCtx->SegCs, pCtx->Eip);
	fprintf(fpLast, "SS:ESP:%04X:%08X  EBP:%08X\r\n", pCtx->SegSs, pCtx->Esp, pCtx->Ebp);
	fprintf(fpLast, "DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs);
	fprintf(fpLast, "Flags:%08X\r\n", pCtx->EFlags);

#endif

	if ( !InitImagehlpFunctions() )
	{
#ifdef _M_IX86  // Intel Only!
		// Walk the stack using x86 specific code
		IntelStackWalk(pCtx, fp, fpLast);
#endif

		return;
	}

	ImagehlpStackWalk(pCtx, fp, fpLast);

	m_symCleanup(GetCurrentProcess());

	fclose(fp);
	fclose(fpLast);

	// 시스템 사양은 혹시 뻗을지 모르니 뒤에
	/*
	if( systemSpec.IsInited() )
	{
		const SystemSpecData& specData = systemSpec.GetSpecData();

		fpLast = fopen(filename, "at");

		fprintf(fpLast, "\r\nCPU : %s\r\n",		to_ansi(specData.cpu).c_str());
		fprintf(fpLast, "RAM : %s\r\n",			to_ansi(specData.memory).c_str());
		fprintf(fpLast, "Video Card : %s\r\n",	to_ansi(specData.display[0].vgaDesc).c_str());
		fprintf(fpLast, "Video Ram : %s\r\n",	to_ansi(specData.display[0].vram).c_str());
		fprintf(fpLast, "OS : %s\r\n",			to_ansi(specData.os).c_str());

		fclose(fpLast);
	}
	*/
}











//======================================================================
// Given an exception code, returns a pointer to a static string with a 
// description of the exception                                         
//======================================================================
LPSTR ExceptionHandler::GetExceptionString(DWORD dwCode)
{
#define EXCEPTION( x ) case EXCEPTION_##x: return #x;

	switch ( dwCode )
	{
		EXCEPTION( ACCESS_VIOLATION )
		EXCEPTION( DATATYPE_MISALIGNMENT )
		EXCEPTION( BREAKPOINT )
		EXCEPTION( SINGLE_STEP )
		EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
		EXCEPTION( FLT_DENORMAL_OPERAND )
		EXCEPTION( FLT_DIVIDE_BY_ZERO )
		EXCEPTION( FLT_INEXACT_RESULT )
		EXCEPTION( FLT_INVALID_OPERATION )
		EXCEPTION( FLT_OVERFLOW )
		EXCEPTION( FLT_STACK_CHECK )
		EXCEPTION( FLT_UNDERFLOW )
		EXCEPTION( INT_DIVIDE_BY_ZERO )
		EXCEPTION( INT_OVERFLOW )
		EXCEPTION( PRIV_INSTRUCTION )
		EXCEPTION( IN_PAGE_ERROR )
		EXCEPTION( ILLEGAL_INSTRUCTION )
		EXCEPTION( NONCONTINUABLE_EXCEPTION )
		EXCEPTION( STACK_OVERFLOW )
		EXCEPTION( INVALID_DISPOSITION )
		EXCEPTION( GUARD_PAGE )
		EXCEPTION( INVALID_HANDLE )
	}

	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.

	static CHAR szBuffer[512] = { 0 };

	FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA("NTDLL.DLL"),
					dwCode, 0, szBuffer, sizeof(szBuffer), 0);

	return szBuffer;
}

//==============================================================================
// Given a linear address, locates the module, section, and offset containing  
// that address.                                                               
//                                                                             
// Note: the szModule paramater buffer is an output buffer of length specified 
// by the len parameter (in CHARacters!)                                       
//==============================================================================
bool ExceptionHandler::GetLogicalAddress(PVOID addr, LPSTR szModule, DWORD len, DWORD& section, DWORD& offset)
{
	MEMORY_BASIC_INFORMATION mbi;

	if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if ( !GetModuleFileNameA( (HMODULE)hMod, szModule, len ) )
		return FALSE;

	// Point to the DOS header in memory
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

	// From the DOS header, find the NT (PE) header
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (   unsigned i = 0;
		i < pNtHdr->FileHeader.NumberOfSections;
		i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;
			return TRUE;
		}
	}

	return FALSE;   // Should never get here!
}

//============================================================
// Walks the stack, and writes the results to the report file 
//============================================================
void ExceptionHandler::IntelStackWalk(PCONTEXT pContext, FILE* fp /* = NULL */, FILE* fpLast /* = NULL */)
{
	if(fp) fprintf(fp, "\r\nCall stack:\r\n");
	if(fp) fprintf(fp, "Address   Frame     Logical addr  Module\r\n");

	if(fpLast) fprintf(fpLast, "\r\nCall stack:\r\n");
	if(fpLast) fprintf(fpLast, "Address   Frame     Logical addr  Module\r\n");

	DWORD pc = pContext->Eip;
	PDWORD pFrame, pPrevFrame;

	pFrame = (PDWORD)pContext->Ebp;

	do
	{
		CHAR szModule[MAX_PATH] = {0};
		DWORD section = 0, offset = 0;

		GetLogicalAddress((PVOID)pc, szModule, sizeof(szModule), section, offset);

		if(fp)		fprintf(fp, "%08X  %08X  %04X:%08X %s\r\n", pc, pFrame, section, offset, szModule);
		if(fpLast)	fprintf(fpLast, "%08X  %08X  %04X:%08X %s\r\n", pc, pFrame, section, offset, szModule);

		pc = pFrame[1];

		pPrevFrame = pFrame;

		pFrame = (PDWORD)pFrame[0]; // precede to next higher frame on stack

		if ( (DWORD)pFrame & 3 )    // Frame pointer must be aligned on a
			break;                  // DWORD boundary.  Bail if not so.

		if ( pFrame <= pPrevFrame )
			break;

		// Can two DWORDs be read from the supposed frame address?          
		if ( IsBadWritePtr(pFrame, sizeof(PVOID)*2) )
			break;

	} while ( 1 );
}

//============================================================
// Walks the stack, and writes the results to the report file 
//============================================================
void ExceptionHandler::ImagehlpStackWalk(PCONTEXT pContext, FILE* fp /* = NULL */, FILE* fpLast /* = NULL */)
{
	if(fp) fprintf(fp, "\r\nCall stack:\r\n");
	if(fp) fprintf(fp, "Address   Frame\r\n");

	if(fpLast) fprintf(fpLast, "\r\nCall stack:\r\n");
	if(fpLast) fprintf(fpLast, "Address   Frame\r\n");

	// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

	STACKFRAME sf;
	memset( &sf, 0, sizeof(sf) );

	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	sf.AddrPC.Offset       = pContext->Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = pContext->Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = pContext->Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	while(TRUE)
	{
		if ( ! m_stackWalk(  IMAGE_FILE_MACHINE_I386,
			GetCurrentProcess(),
			GetCurrentThread(),
			&sf,
			pContext,
			0,
			m_symFunctionTableAccess,
			m_symGetModuleBase,
			0 ) )
			break;

		if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
			break;                      // the frame is OK.  Bail if not.

		if(fp) fprintf(fp, "%08X  %08X  ", sf.AddrPC.Offset, sf.AddrFrame.Offset);
		if(fpLast) fprintf(fpLast, "%08X  %08X  ", sf.AddrPC.Offset, sf.AddrFrame.Offset);

		// IMAGEHLP is wacky, and requires you to pass in a pointer to a
		// IMAGEHLP_SYMBOL structure.  The problem is that this structure is
		// variable length.  That is, you determine how big the structure is
		// at runtime.  This means that you can't use sizeof(struct).
		// So...make a buffer that's big enough, and make a pointer
		// to the buffer.  We also need to initialize not one, but TWO
		// members of the structure before it can be used.

		BYTE symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
		PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLength = 512;

		DWORD symDisplacement = 0;  // Displacement of the input address,
		// relative to the start of the symbol

		if ( m_symGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &symDisplacement, pSymbol) )
		{
			if(fp) fprintf(fp, "%hs+%X\r\n", pSymbol->Name, symDisplacement );
			if(fpLast) fprintf(fpLast, "%hs+%X\r\n", pSymbol->Name, symDisplacement );
		}
		else    // No symbol found.  Print out the logical address instead.
		{
			CHAR szModule[MAX_PATH] = {0};
			DWORD section = 0, offset = 0;

			GetLogicalAddress(  (PVOID)sf.AddrPC.Offset, szModule, sizeof(szModule), section, offset );

			if(fp) fprintf(fp, "%04X:%08X %s\r\n", section, offset, szModule);
			if(fpLast) fprintf(fpLast, "%04X:%08X %s\r\n", section, offset, szModule);
		}
	}
}


//=========================================================================
// Load IMAGEHLP.DLL and get the address of functions in it that we'll use 
//=========================================================================
bool ExceptionHandler::InitImagehlpFunctions( void )
{
	if (m_hModImagehlp) return TRUE;

	HMODULE hModImagehlp = LoadLibraryA("IMAGEHLP.DLL");
	if ( !hModImagehlp ) return FALSE;

	m_symInitialize = (SYMINITIALIZEPROC)GetProcAddress(hModImagehlp, "SymInitialize");
	if ( !m_symInitialize ) return FALSE;

	m_symCleanup = (SYMCLEANUPPROC)GetProcAddress(hModImagehlp, "SymCleanup");
	if ( !m_symCleanup ) return FALSE;

	m_stackWalk = (STACKWALKPROC)GetProcAddress(hModImagehlp, "StackWalk");
	if ( !m_stackWalk ) return FALSE;

	m_symFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC)GetProcAddress(hModImagehlp, "SymFunctionTableAccess");
	if ( !m_symFunctionTableAccess ) return FALSE;

	m_symGetModuleBase = (SYMGETMODULEBASEPROC)GetProcAddress(hModImagehlp, "SymGetModuleBase");
	if ( !m_symGetModuleBase ) return FALSE;

	m_symGetSymFromAddr = (SYMGETSYMFROMADDRPROC)GetProcAddress(hModImagehlp, "SymGetSymFromAddr" );
	if ( !m_symGetSymFromAddr ) return FALSE;


	bool fInvadeProcess = TRUE;

#ifdef _DEBUG
	if( IsDebuggerPresent() )
	{
		fInvadeProcess = FALSE;
	}
#endif

	if ( !m_symInitialize(GetCurrentProcess(), ".;..", fInvadeProcess) )
		return FALSE;

	m_hModImagehlp = hModImagehlp;
	return TRUE;
}
