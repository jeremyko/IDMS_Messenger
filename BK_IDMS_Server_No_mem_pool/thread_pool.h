

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

#ifndef _plugware_thread_pool_h
#define _plugware_thread_pool_h

#if (_WIN32_WINNT < 0x0500)
#error Requires Windows Server 2003, Windows 2000 Server, or Windows NT Server 3.5 and later.
#endif //_WIN32_WINNT < 0x0500

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include <list>
#include <vector>
#include <crtdbg.h>
#include "utility.h"

#include "oracl.h"
#include "OraObjectArray.h"
#include "log.h"

namespace plugware 
{
	namespace core 
	{		
		/**		
		 *	Context to execute stateful work in a controllable, concurrent fashion.
		 *               
		 *  On win32 platforms implemented using iocp.  Work may be queued against the
		 *  thread pool and will execute when a thread is free.  The thread pool provides
		 *  fined grained concurrency control: number of threads running, total threads and a
		 *  global initialization object available per thread pool made available to worker threads
		 *  and work units.
		 *
		 *  Two related classes interact with the thread pool: work units worker threads.
		 *  Work units are stateful objects encapsulating work to be done; worker thread 
		 *  encapsulate the process by which work units are executed.
		 */
		
		///////////////////////////////////////////////////////////////////////////////////////////
		// ���� 
		///////////////////////////////////////////////////////////////////////////////////////////
		template <class worker>
			class CThreadPool  : public CSingleton<CThreadPool <worker> > // CSingleton ��ӹ���
		{
    public:  
		CThreadPool(); 
		~CThreadPool(); 		
		
        typedef typename worker::request_type request_type; // worker::request_type => CWorkUnit �̴�..
        typedef std::vector<HANDLE> thread_vector;        
		
        enum { _shutdown_flag_ = -1 };    
        void shutdown(const DWORD dw_max_wait = INFINITE);
        void initialize(size_t n_running_threads = 0, size_t n_thread_count = 0, const DWORD dw_stack_size = 0);
        void associate_device(HANDLE h_device);
        void queue_request(request_type* p_request);
        BOOL get_queued_status(LPDWORD pdw_bytes, request_type** pp_request, LPOVERLAPPED* pp_overlapped, DWORD dw_wait);        
        HANDLE get_completion_port() { return m_h_iocp; }		
        const size_t get_size() const;		
		
    private:
		
        void create_iocp(const size_t n_running_threads);
        void create_threads(size_t n_thread_count, const size_t dw_stack_size);
        void flag_shutdown();
        void wait_for_threads_to_die(DWORD dw_max_wait);
		
        thread_vector& get_thread_vector() { return m_c_thread_vector; }
		
        const bool           shutdown_initiated() const { return m_b_shutdown_flag; }
        const thread_vector& get_thread_vector()  const { return m_c_thread_vector; }
        const bool           is_initialized()     const 
		{	return 0     != m_h_iocp &&
			false == get_thread_vector().empty(); 
		}
        // static helpers
        static const size_t get_cpu_count();
        
    private:    
		
        bool			m_b_shutdown_flag;
        thread_vector	m_c_thread_vector;
        HANDLE			m_h_iocp;		
        
		}; // class CThreadPool	
		   
		//class CWorkUnit 
		class CWorkUnit : public CReferenceCounted 
		{		
		public:
			virtual BOOL process() throw() = 0;			//! CWorkUnit �� ��ӹ��� Ŭ�������� ������ �ؼ� ���. 			
			virtual void ProcWhenDone() throw() = 0;	//! �۾��� �Ϸ� �Ǿ����� ȣ��Ǵ� �Լ� 

			BOOL		m_bAllOK ;
			int			m_nIndex;			
		};    	
		
		class CWorkerThread 
		{
		public:
			
			typedef CWorkUnit request_type; // !! request_type == CWorkUnit ���� Define��!!
			typedef CThreadPool<CWorkerThread> CThreadPool;
			void execute(request_type* p_request) const throw();
			
			//static DWORD WINAPI thread_proc(LPVOID pv) throw();
			static unsigned __stdcall thread_proc( void* lpParam ) throw();

			const DWORD thread_proc() const;
			//static void prepare_request(request_type* p_request) { p_request->add_ref(); }
			static void prepare_request(request_type* p_request) { p_request->add_ref(); }
			
		private: // implementation
			CWorkerThread();
		}; 

		// Chain Class ����
		/*
		class CChain 
		{			
			public:			
			class data : public CWorkUnit, public std::list<CSmartPointer<CWorkUnit> > 
			//class data : public CWorkUnit, public std::list<CWorkUnit> //20060918
			{	
				public:				
					data()
					{		
						m_bAllOK = TRUE; // !!! ������� �ݵ�� �ʱ�ȭ �ؾ���...
						m_nIndex = 0;    // !!! ������� �ݵ�� �ʱ�ȭ �ؾ���...
					};

					BOOL process() throw();
					BOOL process(OSession* p_osess, ODatabase*	p_odb) throw();
					void ProcWhenDone() throw();	//kojh
					void Commit() throw();			//kojh
					void RollBack() throw();		//kojh	

					BOOL		m_bAllOK ;
					int			m_nIndex;					
			};  
			
			CChain();
			
			~CChain();
			CChain& operator,( CWorkUnit* p_work );
						
			operator CWorkUnit*() 
			{ 				
				return m_work.get(); 
			}
						
			void SetWork(CWorkUnit* p_work);

			CSmartPointer<data> m_work; // list 		

			int			m_nIndex;			
		}; 
		*/
		
		///////////////////////////////////////////////////////////////////////////////////////////
		// ���� 
		///////////////////////////////////////////////////////////////////////////////////////////
		
		// CThreadPool		
		template <class T>
			CThreadPool<T>::CThreadPool() : m_b_shutdown_flag(false) , m_h_iocp(0)
		{	
			// VC6 ������ ����ȭ �ɼ� �ִ�ӵ��� ��� Singleton �ߺ� Instance ���� ����!!! 
			// ����ȭ Disable�� �ؾ� ���� ������..
			// VC 2005 Pro ������ ���� �ȳ�.
			
			TRACE("CThreadPool ������\n");
		}
		
		template <class worker>
			CThreadPool<worker>::~CThreadPool()
		{			
			shutdown();
		}
		
		template <class worker>
			void CThreadPool<worker>::initialize(size_t n_running_threads, size_t n_thread_count, const DWORD dw_stack_size)
		{
			// validate and assign defaults:
			// n_running_threads: cpu count
			// n_thread_count:    2 * n_running_threads
			if (true == is_initialized()) 
			{
				TRACE("CThreadPool initialize -> is_initialized true return\n");
				return;
			}
			
			// �Ϲ����� ���尪�� ����
			/*
			if (0 >= n_running_threads) 
			n_running_threads = get_cpu_count();
			
			  if (0 >= n_thread_count) 
			  n_thread_count = n_running_threads << 1;
			  
				if (n_running_threads > n_thread_count) 
				n_running_threads = n_thread_count;
			*/
			
			// Blocking �� ���� �۾��� ����ϴٸ� ?? 
			
			n_running_threads = 8;
			n_thread_count = 26; 
			
			// store param, create iocp and threads
			create_iocp(n_running_threads);
			create_threads(n_thread_count, dw_stack_size);
		}
		
		template <class worker>
			void CThreadPool<worker>::create_iocp(const size_t n_running_threads)
		{
			// validate, create and validate
			if (true == is_initialized()) return;
			
			//@@todo: migrate to thread traits
			m_h_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, (DWORD)n_running_threads);
			
			if(m_h_iocp == NULL)
			{
				TRACE("CreateIoCompletionPort NULL Return\n");
			}
			
			if (0 == get_completion_port()) 
			{				
				throw std::exception("CreateIoCompletionPort failed");
			}		
		}
		
		// Thread�� �����ϰ� vector�� ����
		template <class worker>
			void CThreadPool<worker>::create_threads(size_t n_thread_count, const size_t dw_stack_size)
		{
			// validate, reserve, create and store
			if (true == is_initialized()) 
			{				
				return;
			}
			
			get_thread_vector().reserve(n_thread_count);
			
			while (n_thread_count--)
			{
				//@@todo: migrate to thread traits
				//@@review: security, dw_thread_id, etc
				// create thread, validate and store
								
				unsigned dw_thread_id = 0;
				HANDLE h_thread  = (HANDLE) _beginthreadex(NULL, 0, &worker::thread_proc, NULL, 0, &dw_thread_id ) ;			
				
				/*
				DWORD  dw_thread_id = 0;
				HANDLE h_thread = ::CreateThread	(
														0, 
														dw_stack_size, 
														worker::thread_proc, 
														0,
														0,
														&dw_thread_id
													);
				*/

				if (0 == h_thread) 
				{
					throw std::exception("CreateThread failed");
				}
				
				get_thread_vector().push_back(h_thread);
			}
		}
		
		template <class worker>
			void CThreadPool<worker>::associate_device(HANDLE h_device)
		{
			// validate, associate and validate
			if (false == is_initialized()) 
			{
				throw std::exception("uninitialized");
			}

			if (0 == ::CreateIoCompletionPort(h_device, get_completion_port(), 0, 0))
			{
				throw std::exception("CreateIoCompletionPort failed");
			}   
		}
		
		template <class worker>
			void CThreadPool<worker>::queue_request(request_type* p_request)
		{
			// validate, if we are shutting down, bail
			_ASSERT(p_request);
			if (true == shutdown_initiated()) 
			{
				// TRACEX("shutdown_initiated true return"); 
				return;        
			}
			
			worker::prepare_request(p_request);
			
			//@@todo: migrate to thread traits
			
			if (FALSE == ::PostQueuedCompletionStatus(get_completion_port(), 0, (DWORD)p_request, 0))
			{			
				DWORD nErr = GetLastError();				
				TRACE("PostQueuedCompletionStatus Fail [%ld]",nErr);
				
				LPVOID lpMsgBuf;
				FormatMessage	( 
									FORMAT_MESSAGE_ALLOCATE_BUFFER | 
									FORMAT_MESSAGE_FROM_SYSTEM | 
									FORMAT_MESSAGE_IGNORE_INSERTS,
									NULL,
									GetLastError(),
									MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
									(LPTSTR) &lpMsgBuf,
									0,
									NULL 
								);
			  
				TRACE("%s\n" , (LPCTSTR)lpMsgBuf);

				// Free the buffer.
				LocalFree( lpMsgBuf );				
				
				throw std::exception("PostQueuedCompletionStatus failed");
			}
		}
		
		template <class worker>
			BOOL CThreadPool<worker>::get_queued_status(LPDWORD pdw_bytes, request_type** pp_request, LPOVERLAPPED* pp_overlapped, DWORD dw_wait)
		{
			if (false == is_initialized()) 
			{
				throw std::exception("uninitialized");
			}
			
			return ::GetQueuedCompletionStatus(get_completion_port(), pdw_bytes, (LPDWORD)pp_request, pp_overlapped, dw_wait);
		}
		
		template <class worker>
			void CThreadPool<worker>::shutdown(const DWORD dw_max_wait)
		{		
			// validate
			if (false == is_initialized())
			{
				return;
			}

			if (true == shutdown_initiated())
			{
				return;
			}
			
			// flag, wait
			flag_shutdown();
			wait_for_threads_to_die(dw_max_wait);
			
			// close handles and clear
			for (thread_vector::iterator handle(get_thread_vector().begin());
			handle != get_thread_vector().end();
			*handle && ::CloseHandle(*handle), ++handle) {}
			
			get_thread_vector().clear();
			::CloseHandle(m_h_iocp);
			m_h_iocp = 0;
		}
		
		template <class worker>
			void CThreadPool<worker>::flag_shutdown()
		{
			// flag and queue
			m_b_shutdown_flag = true;
			size_t n_thread_count = get_thread_vector().size();
			
			//@@todo: migrate thread traits
			while (n_thread_count--) 
			{
				::PostQueuedCompletionStatus(	get_completion_port(),
												0,
												0,
												(LPOVERLAPPED)_shutdown_flag_
											);
			}
		}
		
		template <class worker>
			void CThreadPool<worker>::wait_for_threads_to_die(const DWORD dw_max_wait)
		{
			::WaitForMultipleObjects(	(DWORD)get_thread_vector().size(), 
										&get_thread_vector().at(0), 
										TRUE, 
										dw_max_wait
									);
		}
		
		template <class worker>
			const size_t CThreadPool<worker>::get_cpu_count()
		{
			SYSTEM_INFO c_si = {0};
			::GetSystemInfo(&c_si);
			return c_si.dwNumberOfProcessors;
		}
		
		
		// CWorkerThread ����		
		inline CWorkerThread::CWorkerThread()
		{
		}
		
		inline unsigned __stdcall CWorkerThread::thread_proc( void* lpParam ) throw()
		//inline DWORD WINAPI CWorkerThread::thread_proc(LPVOID) throw()
		{
			try
			{
				// return values:
				//  0: success, -1: failure
				return CWorkerThread().thread_proc();
			}
			catch (...) 						
			{
				CLog::Instance().WriteFile(FALSE, "ErrorTerminate", "Ln [%d] �� CWorkerThread::thread_proc �׾��� [%d]!!\n", __LINE__, (int) GetLastError()  );				
			}

			return DWORD(-1);
		}
		
		inline const DWORD CWorkerThread::thread_proc() const
		{
			// declare
			DWORD         dw_bytes_transferred = 0;
			OVERLAPPED*   p_overlapped         = 0;
			request_type* p_request            = 0;
			
			// main loop
			for (;;)
			{ 			
				// get next packet, on failure assert and and continue
				if (FALSE == 
					//CThreadPool::instance().get_queued_status( // .Net OK kojh 
					core::CThreadPool<core::CWorkerThread>::instance().get_queued_status(  //VC6 kojh
																			&dw_bytes_transferred,
																			&p_request,
																			&p_overlapped,
																			INFINITE))
				{
					// nothing to see, move along ...
					continue;
				} 			
				
				// shutdown or process
				//if (CThreadPool::_shutdown_flag_ == (UINT_PTR)p_overlapped) break; // .Net OK kojh 
				if (core::CThreadPool<core::CWorkerThread>::_shutdown_flag_ == (UINT_PTR)p_overlapped) 
				{					
					//kojh
					break; //VC6 OK kojh
				}
				
				// no io requests, please
				_ASSERT(NULL == p_overlapped);

				execute(p_request);
			}
			
#ifdef _DEBUG    
			::OutputDebugStringA("CWorkerThread: terminated\n");
#endif //_DEBUG
			
			return 0;
		}
		
		inline void CWorkerThread::execute(request_type* p_request) const throw()
		{
			// validate and process (io or logic)
			if (0 == p_request) 
			{
				return;
			}			

			p_request->process();
			p_request->ProcWhenDone();		
			p_request->release();  

			TRACE("execute Done\n");
		}
		 
		
		//  CChain Class ����
		/*
		inline CChain::CChain() : m_work(new(data)) 
		{	
			m_nIndex = -1;
			m_nIndex = COraObjectArray::Instance().getOraObject().m_nIndex;

			((data*)m_work.m_p)->m_nIndex = m_nIndex ;									
			
			//TRACEX("CChain::CChain() Call..\n");			
		}
				
		inline CChain& CChain::operator,(CWorkUnit* p_work)
		{				
			p_work->m_nIndex = m_nIndex;
			
			m_work.get()->push_back(p_work);

			return *this;
		}		

		inline CChain::~CChain()
		{
			//TRACEX("�� CChain::~CChain()!!!\n");			
		}
		
		
		inline void CChain::SetWork(CWorkUnit* p_work)
		{
			_ASSERT( 0 != p_work );
			_ASSERT( 0 != m_work.get());
			m_work.get()->push_back(p_work);			
		}
		
		
					
		///////////////////////////////////////////////////////////////////////////////
		//  CChain::data
		///////////////////////////////////////////////////////////////////////////////
		// CChain �� ����ϴ� ���: �����忡�� process() -> ����  process() call..
		
		inline BOOL CChain::data::process(OSession* p_osess, ODatabase*	p_odb)		
		{
			return FALSE;
		}

		inline BOOL CChain::data::process()		
		{
			_ASSERT(size());
			_ASSERT(front().get());
			
			TRACEX("��� process Index[%d] \n",m_nIndex);

			if(front().get()->process	( 
											& (COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_OSession),
											& (COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_ODataBase)											
										) != TRUE)
			{				
				TRACEX("m_bAllOK FALSE!!\n");
				m_bAllOK = FALSE;
			}			
			
			front().get()->ProcWhenDone();			
		
			pop_front(); //kojh
								
			// if not empty, requeue
			if (true == empty()) 			
			{				
				if( m_bAllOK == FALSE)
				{					
					// All or Nothing..

					RollBack();
					
					return FALSE;
				}
				
				Commit();
				
				return TRUE;
			}
			
			CThreadPool<CWorkerThread>::instance().queue_request(this);

			// !!!!!!! ���⼭ return �ϸ� �ȵ�!! �����Ϸ� ���� ����.. !!!!!!
		}   // !!!!!!! ���⼭ return �ϸ� �ȵ�!! �����Ϸ� ���� ����.. !!!!!!
						  
		// CChain �� ����ϴ� ���: 	
		inline void CChain::data::ProcWhenDone()		
		{
			TRACEX("CChain::data::ProcWhenDone => OShutdown\n");			
			
			OShutdown();
		}	

		inline void CChain::data::Commit()		
		{		
			TRACEX("����� Commit Index[%d] \n",m_nIndex);

			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_OSession.Commit();
			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_OSession.Close();
			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_ODataBase.Close();

			COraObjectArray::Instance().returnObject(m_nIndex);
			
			//testOra * pOra = new testOra();
			//delete pOra; => Error!!

			//OSession* pSe = new OSession();
			//delete pSe; => Error!!			

			OShutdown();			
		}	

		inline void CChain::data::RollBack()		
		{			
			TRACEX("����� RollBack Index[%d] \n",m_nIndex);

			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_OSession.Rollback();
			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_OSession.Close();
			COraObjectArray::Instance().getOraObjectByIndex(m_nIndex).m_ODataBase.Close();
			
			COraObjectArray::Instance().returnObject(m_nIndex);			

			OShutdown();			
		}		
		*/

	} // namespace core
	

	// kojh : CWorkerThread �� ��üȭ.
	namespace global 
	{
		typedef core::CThreadPool<core::CWorkerThread> CThreadPool;
	} // namespace global

} // namespace plugware
#endif //_plugware_thread_pool_h
