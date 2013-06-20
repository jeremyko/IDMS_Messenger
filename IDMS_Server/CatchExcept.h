#include "stdafx.h"

#include "log.h"

#include <windows.h>
#include <vector>
#include <iostream>

#ifndef _CATCH_EXCEPT_H
#define _CATCH_EXCEPT_H

const DWORD CPP_EXCEPTION    = 0xE06D7363; 
const DWORD MS_MAGIC         = 0x19930520;


// http://www.ddj.com/windows/184416600 ÂüÁ¶ 

// single thread 
/*
void PreviewException(void * pObject,
                      void * pThrowAddress, 
                      const type_info * pObjectInfo)
{
    pObject; //reference parameter
    if (pObjectInfo)
	{
		CLog::Instance().WriteFile(FALSE, "Error",  "Exception of type [%s] occured at [%x]\n", 
										pObjectInfo->name() , pThrowAddress );			

        std::cout << "Exception of type \"" 
                  << pObjectInfo->name() 
                  << "\" occured at " 
                  << pThrowAddress 
                  << "\n";
	}    
}

// this processor-specific function returns
// the return address of the caller
// assuming that callers frame was not optimized

__declspec(naked)
void * GetReturnAddress()
{
    __asm
    {
        mov eax, [ebp + 4]
        ret
    }
}

// this processor-specific function returns
// the stack frame of the caller's caller
// assuming that frames were not optimized

__declspec(naked)
void * GetCallerFrame()
{
    __asm
    {
        mov eax, [ebp]
        ret
    }
}

// imhibit optimization of frames
#pragma optimize("y",off)

extern "C"
void __stdcall _CxxThrowException(void * pObject,
                                  _s__ThrowInfo const * pObjectInfo)
{    
    void * paddr = GetReturnAddress();
    // call __CxxThrowException@8 is 5 bytes
    // long on i386. Subtracting 5 from return address
    // gives us the address of call instruction
paddr = (void*)((DWORD_PTR)paddr - 5);
    void * pframe = GetCallerFrame();

    type_info * pti = 0;
    // since we can get here when an exception is rethrown
    // pObjectInfo could be 0. Also it is a good idea to
    // make a sanity check
        if (pObjectInfo && 
            pObjectInfo->pCatchableTypeArray &&
            pObjectInfo->pCatchableTypeArray->nCatchableTypes > 0)
    {
        const _s__CatchableTypeArray * pTypes = 
            pObjectInfo->pCatchableTypeArray;
        pti = (type_info*) 
            ((*(pTypes->arrayOfCatchableTypes))[0].pType);
    }

    //call the hook function
    PreviewException(pObject,paddr,pti);

    const ULONG_PTR args[] = {MS_MAGIC,
                              (ULONG_PTR)pObject,
                              (ULONG_PTR)pObjectInfo};
    RaiseException(CPP_EXCEPTION,
                   EXCEPTION_NONCONTINUABLE,
                   sizeof(args)/sizeof(args[0]),
                   args);
}

//restore optimization settings
#pragma optimize("",on)
*/


// Multi Thread

#if defined(_MT) && !defined(_DLL)

extern "C"
{
    // merged from VC 6 and .NET internal headers in CRT source code

    struct _tiddata 
    {
        unsigned long   _tid;       /* thread ID */

        unsigned long   _thandle;   /* thread handle */

        int     _terrno;            /* errno value */
        unsigned long   _tdoserrno; /* _doserrno value */
        unsigned int    _fpds;      /* Floating Point data segment */
        unsigned long   _holdrand;  /* rand() seed value */
        char *      _token;         /* ptr to strtok() token */
        wchar_t *   _wtoken;        /* ptr to wcstok() token */
        unsigned char * _mtoken;    /* ptr to _mbstok() token */

        /* following pointers get malloc'd at runtime */
        char *      _errmsg;        /* ptr to strerror()/_strerror()
                                       buff */
#if _MSC_VER >= 1300
        wchar_t *   _werrmsg;       /* ptr to _wcserror()/__wcserror()
                                       buff */
#endif
        char *      _namebuf0;      /* ptr to tmpnam() buffer */
        wchar_t *   _wnamebuf0;     /* ptr to _wtmpnam() buffer */
        char *      _namebuf1;      /* ptr to tmpfile() buffer */
        wchar_t *   _wnamebuf1;     /* ptr to _wtmpfile() buffer */
        char *      _asctimebuf;    /* ptr to asctime() buffer */
        wchar_t *   _wasctimebuf;   /* ptr to _wasctime() buffer */
        void *      _gmtimebuf;     /* ptr to gmtime() structure */
        char *      _cvtbuf;        /* ptr to ecvt()/fcvt buffer */

        /* following fields are needed by _beginthread code */
        void *      _initaddr;      /* initial user thread address */
        void *      _initarg;       /* initial user thread argument */

        /* following three fields are needed to support 
         * signal handling and
         * runtime errors */
        void *      _pxcptacttab;   /* ptr to exception-action table */
        void *      _tpxcptinfoptrs; /* ptr to exception info pointers*/
        int         _tfpecode;      /* float point exception code */
#if _MSC_VER >= 1300
        /* pointer to the copy of the multibyte character 
         * information used by the thread */
        /*pthreadmbcinfo*/ void *  ptmbcinfo;

        /* pointer to the copy of the locale informaton 
         * used by the thead */
        /*pthreadlocinfo*/ void *  ptlocinfo;
#endif


        /* following field is needed by NLG routines */
        unsigned long   _NLG_dwCode;

        /*
         * Per-Thread data needed by C++ Exception Handling
         */
        void *      _terminate;     /* terminate() routine */
        void *      _unexpected;    /* unexpected() routine */
        void *      _translator;    /* S.E. translator */
        void *      _curexception;  /* current exception */
        void *      _curcontext;    /* current exception context */
#if _MSC_VER >= 1300
        int         _ProcessingThrow; /* for uncaught_exception */
#endif
#if defined (_M_IA64)
        void *      _pExitContext;
        void *      _pUnwindContext;
        void *      _pFrameInfoChain;
        unsigned __int64     _ImageBase;
        unsigned __int64     _TargetGp;
        unsigned __int64     _ThrowImageBase;
#elif defined (_M_IX86)
        void *      _pFrameInfoChain;
#endif  /* defined (_M_IX86) */
    };
    typedef struct _tiddata * _ptiddata;

    _ptiddata __cdecl _getptd();
}

const EXCEPTION_RECORD * GetCurrentExceptionRecord()
{
    _ptiddata p = _getptd();

	return (EXCEPTION_RECORD *)p->_curexception;
}

#elif !defined(_MT)

extern struct EHExceptionRecord * _pCurrentException;

const EXCEPTION_RECORD * GetCurrentExceptionRecord()
{
    return (EXCEPTION_RECORD *)_pCurrentException;
}

#endif //_MT


#endif 


