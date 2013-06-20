/**
 * \file		utility.h
 * \author		Plugware Solutions.com, Ltd.
 * \date		
 * \brief		[brief here]
 *
 *              Copyright (c) 2003, 2004
 *              Plugware Solutions.com, Ltd.
 *
 *              Permission to use, copy, modify, distribute and sell 
 *              this software and its documentation for any purpose 
 *              is hereby granted without fee, provided that the above 
 *              copyright notice appear in all copies and the both that 
 *              copyright notice and this permission notice appear 
 *              in supporting documentation.  Plugware Solutions.com, Ltd. 
 *              makes no reresentations about the suitability of this 
 *              software for any purpose.  It is provided "as is" 
 *              without express or implied warranty.
 */

#ifndef _plugware_utility_h
#define _plugware_utility_h

#include "stdafx.h"

namespace plugware
{    
    // Quick and easy CSingleton.
    
    template <typename T>
		class CSingleton
    {
		public:
			static T& instance();
		
//		private:	
//			CSingleton(){};
//			CSingleton(const CSingleton&);
//			CSingleton&	operator=(const CSingleton& rhs);
		
    };
		
    
    // Quick and easy smart pointer.
    
    template <typename T>
		class CSmartPointer
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
    
	
	
    // CSingleton 备泅     
	
    template <typename T>
		T& CSingleton<T>::instance()
    {
        static T t_;
        return t_;
    }
	

    // CReferenceCounted 备泅     
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
            delete this; 
        }            
    }
    
    inline void CReferenceCounted::add_ref() 
    { 
        ::InterlockedIncrement(&m_count); 
    }
    
	
    
    // CSmartPointer 备泅     
    template <class T>
		const CSmartPointer<T>& CSmartPointer<T>::operator=(T* p)
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
	
	//
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
		CString  m_strUsage;
	};     
    
	// CCriticalSectionLocalScope 备泅     	
    inline CCriticalSectionLocalScope::CCriticalSectionLocalScope()
    {
        ::InitializeCriticalSection(&m_h_cs);
    }

    inline CCriticalSectionLocalScope::~CCriticalSectionLocalScope()
    {
        ::DeleteCriticalSection(&m_h_cs);
    }

    inline const bool CCriticalSectionLocalScope::acquire_lock()
    {
    #if(_WIN32_WINNT >= 0x0400)
        return 1L & TryEnterCriticalSection(&m_h_cs);
    #else
        EnterCriticalSection(&m_h_cs);
        return true;
    #endif //_WIN32_WINNT >= 0x0400
    }

    inline void CCriticalSectionLocalScope::unlock()
    {
        ::LeaveCriticalSection(&m_h_cs);
    }
    
    // CAutoLock 备泅   	
	inline CAutoLock::CAutoLock(CCriticalSectionLocalScope& c_lock, char* szUsage )
	: m_c_lock(c_lock) , m_strUsage(szUsage)
	{ 	
		while (false == m_c_lock.acquire_lock()) {}
		
		//TRACE("CAutoLock 积己 [%s]\n", m_strUsage);
	}        

	inline CAutoLock::~CAutoLock() 
	{ 
		m_c_lock.unlock(); 	
		//TRACE("CAutoLock 力芭 [%s]\n",m_strUsage);
		m_strUsage.Empty();
	}    

} // namespace plugware
#endif //_plugware_utility_h