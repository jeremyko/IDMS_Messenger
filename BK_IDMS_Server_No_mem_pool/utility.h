/**
 *	Copyright (c) 2003, 2004
 *  Plugware Solutions.com, Ltd.
 *
 *  Permission to use, copy, modify, distribute and sell 
 *  this software and its documentation for any purpose 
 *  is hereby granted without fee, provided that the above 
 *  copyright notice appear in all copies and the both that 
 *  copyright notice and this permission notice appear 
 *  in supporting documentation.  Plugware Solutions.com, Ltd. 
 *  makes no reresentations about the suitability of this 
 *  software for any purpose.  It is provided "as is" 
 *  without express or implied warranty.
*/

#ifndef _plugware_utility_h
#define _plugware_utility_h

#include "stdafx.h"

const int CSPINCOUNT = 4000 ;

namespace plugware
{    
    // Quick and easy CSingleton.
    
    template <typename T> class CSingleton
    {
		public:
			static T& instance();		
    };
		    
    // Quick and easy smart pointer.
    
    template <typename T> class CSmartPointer
    {
		
	public:
        CSmartPointer(T* p) : m_p(0) { *this = p; }
        CSmartPointer(const CSmartPointer& that) : m_p(0) { *this = that.m_p; }
        ~CSmartPointer() { if (m_p) { m_p->release(); m_p = 0; }}
        
        const CSmartPointer& operator=(T* p);        
        
        // data
        mutable T* m_p; 
	public:
		T* get() { return m_p; }
        T*const get() const { return m_p; }
		
    }; 
	
	
    // Quick and easy reference counting.   
	
    class CReferenceCounted
    {
		
	public:
        LONG m_count;
        void release();
        void add_ref();
		
        CReferenceCounted();
        virtual ~CReferenceCounted();
    };
	
    // CSingleton 구현     
	
    template <typename T> T& CSingleton<T>::instance()
    {
        static T t_;
        return t_;
    }	

    // CReferenceCounted 구현     
	
    inline CReferenceCounted::CReferenceCounted() 
		: m_count(0) 
    {
    }
	
    inline CReferenceCounted::~CReferenceCounted() 
    {
    }
    
	
    inline void CReferenceCounted::release() 
    { 		
        if (0 == ::InterlockedDecrement(&m_count))
        {
			// COracleWorkerMT 객체를 
			// new로 생성후 pool에 저장후 사용하는 경우  delete 를 하면 안됨.
			
			// 만약 COracleWorkerMT객체 POOL 을 사용안할 경우는 
			// 여기서 delete 해줘야함

			 delete this; //20060918 			
        } 		
    }
	
    
    inline void CReferenceCounted::add_ref() 
    { 
        ::InterlockedIncrement(&m_count); 
    }
	
        
    // CSmartPointer 구현     
	
    template <class T> const CSmartPointer<T>& CSmartPointer<T>::operator=(T* p)
    {		
        if (p == m_p) 
			return *this;
		
        if (m_p) 
			m_p->release();
		
        m_p = p;
		
        if (m_p) 
			m_p->add_ref();	
		
        return *this;
    }
	
	
    class CCriticalSectionLocalScope
    {
    public:     // interface
        CCriticalSectionLocalScope() throw();
        ~CCriticalSectionLocalScope();        
        // modifiers
        void       unlock();
        const bool acquire_lock();
    private:
        CRITICAL_SECTION m_h_cs;   
		
    }; 
	

    // scoped locking mechanism    	
	class CAutoLock
	{
	public:
		~CAutoLock();
		CAutoLock(CCriticalSectionLocalScope&, char* szUsage );
		const CAutoLock& operator=(const CAutoLock&);
		CCriticalSectionLocalScope& m_c_lock;
		
		char  m_szUsage[100];
	};     
    
	// CCriticalSectionLocalScope 구현     	
    inline CCriticalSectionLocalScope::CCriticalSectionLocalScope()
    {
        //::InitializeCriticalSection(&m_h_cs);
		::InitializeCriticalSectionAndSpinCount( &m_h_cs, CSPINCOUNT ); // spin lock 
    }

    inline CCriticalSectionLocalScope::~CCriticalSectionLocalScope()
    {
        ::DeleteCriticalSection(&m_h_cs);
    }

    inline const bool CCriticalSectionLocalScope::acquire_lock()
    {
		::EnterCriticalSection(&m_h_cs);
		return true;
    }

    inline void CCriticalSectionLocalScope::unlock()
    {
        ::LeaveCriticalSection(&m_h_cs);
    }
    
    // CAutoLock 구현   	
	inline CAutoLock::CAutoLock(CCriticalSectionLocalScope& c_lock, char* szUsage )
	: m_c_lock(c_lock)
	{ 	
		memset(m_szUsage, 0x00, sizeof(m_szUsage));
		strncpy(m_szUsage, szUsage, sizeof(m_szUsage));

		m_c_lock.acquire_lock() ; 
		
		TRACE("CAutoLock 생성 [%s]\n", m_szUsage);
	}        

	inline CAutoLock::~CAutoLock() 
	{ 
		m_c_lock.unlock(); 	
		TRACE("CAutoLock 제거 [%s]\n", m_szUsage );
	}    

} // namespace plugware



#endif //_plugware_utility_h