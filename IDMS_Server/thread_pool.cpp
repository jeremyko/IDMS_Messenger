
#include "stdafx.h"

/**
 * \file		thread_pool.cpp
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
 
/*
**  Includes
*/

#define _WIN32_WINNT 0x0500
#include <conio.h>
#include <iostream>
#include <time.h>
//#include "oracl.h"

#include "thread_pool.h"
#include "OraMultithrd.h"
using namespace plugware;

/*
**  Implementation
*/

int gSum =0;

/*
#define WORK( label )                                           \
class work_##label : public core::work_unit                     \
{																\
	public:                                                     \
    void process() throw()                                      \
	{	                    									\
		for(int i=0; i< 100; i++){Sleep(1);}	std::cout << "processing work "<< #label <<"["<< i << "]" <<  std::endl; \
	}															\
};

WORK( 1 ); WORK( 2 ); WORK( 3 );WORK( 4 ); WORK( 5 ); 
WORK( 6 ); WORK( 7 ); WORK( 8 ); 

WORK( 9 );WORK( 10 ); 
WORK( 11 ); WORK( 12 );WORK( 13 ); WORK( 14 ); WORK( 15 );
WORK( 16 ); WORK( 17 );WORK( 18 ); WORK( 19 ); WORK( 20 );
WORK( 21 ); WORK( 22 );WORK( 23 ); WORK( 24 ); WORK( 25 );
*/

int main(int, char**)
{
    // initialize the thread pool
    //global::CThreadPool::instance().initialize();    

	//TEST	 
	// global::CThreadPool::instance().initialize();    
    
    //@@todo: demonstrate asynchronous notification
    //std::cout << "Press any key to die ..." << std::endl;       
	
    // demonstrate chaining	
	/*
    global::thread_pool::instance().queue_request(
        (
			// core::chain 에서 , 와 () 연산자 Overloading 
			// , -> push
			// () -> Get
			core::chain(), \
			new work_1, new work_2, new work_3, new work_4, new work_5, \
			new work_6, new work_7, new work_8, new work_9, new work_10, \
			new work_11,new work_12,new work_13
		)
	);
	*/
	/*
	global::thread_pool::instance().queue_request(
        (
			// core::chain 에서 , 와 () 연산자 Overloading 
			// , -> push
			// () -> Get
			core::chain(), new work_14,new work_15, \
			new work_16,new work_17,new work_18,new work_19,new work_20, \
			new work_21,new work_22,new work_23,new work_24,new work_25
		)
	);
	*/
	
	::getch();

	/*
	COracleWorkerMT * pCls9 = new COracleWorkerMT();
	//pCls9->SetSQL("select doc_id, team_gn,sa_cd_gn,to_char(init_crt_date,'YYYYMMDDHH') from km_doclist  ");
//	pCls9->SetSQL("select *  from km_doclist where rownum < 3 ");
//	pCls9->SetIdentity(10);
//	global::CThreadPool::instance().queue_request(pCls9);
//
//	
//	COracleWorkerMT * pCls1 = new COracleWorkerMT();
//	pCls1->SetSQL("select * from km_module where rownum < 20");
//	pCls1->SetIdentity(1);
//	global::CThreadPool::instance().queue_request(
//        ( core::CChain::instance().SetWork(pCls1))
//	);
	
	*/

     
	/*
//	COracleWorkerMT * pCls10 = new COracleWorkerMT();	
//	pCls10->SetSQL("select *  from km_doclist where rownum < 300 ");
//	pCls10->SetIdentity(10);
//	global::CThreadPool::instance().queue_request(pCls10);	
    */

    /*
//	COracleWorkerMT * pCls99 = new COracleWorkerMT();
//	pCls99->SetSQL("select COL_DATE, COL_LONG,COL_FLOAT,COL_INT,COL_INTEGER,COL_NUMBER,COL_CHAR from test_kojh");
//	pCls99->SetIdentity(2);
//	global::CThreadPool::instance().queue_request(
//        ( core::CChain::instance().SetWork(pCls99))
	);
	*/
	
	//////////////
    /*
//	COracleWorkerMT * pCls2 = new COracleWorkerMT();
//	pCls2->SetSQL("select count(*)-20 from km_module");	
//	pCls2->SetIdentity(3);
//	global::CThreadPool::instance().queue_request(pCls2);
//    
//	
//	COracleWorkerMT * pCls3 = new COracleWorkerMT();
//	pCls3->SetSQL("select count(*)-40 from km_module");	
//	pCls3->SetIdentity(4);
//	global::CThreadPool::instance().queue_request(pCls3);
//
//	COracleWorkerMT * pCls4 = new COracleWorkerMT();
//	pCls4->SetSQL("select count(*)-60 from km_module");	
//	pCls4->SetIdentity(5);
//	global::CThreadPool::instance().queue_request(pCls4);
//
//	COracleWorkerMT * pCls5 = new COracleWorkerMT();
//	pCls5->SetSQL("select count(*)-80 from km_module");	
//	pCls5->SetIdentity(6);
//	global::CThreadPool::instance().queue_request(pCls5);
//    
//	
//	COracleWorkerMT * pCls6 = new COracleWorkerMT();
//	pCls6->SetSQL("select count(*)-100 from km_module");	
//	pCls6->SetIdentity(7);
//	global::CThreadPool::instance().queue_request(pCls6);
//
//	COracleWorkerMT * pCls7 = new COracleWorkerMT();
//	pCls7->SetSQL("select count(*)-120 from km_module");	
//	pCls7->SetIdentity(8);
//	global::CThreadPool::instance().queue_request(pCls7);
//
//
//	COracleWorkerMT * pCls8 = new COracleWorkerMT();
//	pCls8->SetSQL("select count(*)-130 from km_module");	
//	pCls8->SetIdentity(9);
//	global::CThreadPool::instance().queue_request(pCls8);
    */
	
	/*
//	global::thread_pool::instance().queue_request(
//        ( core::chain::instance().SetWork(new COracleWorkerThread))
//	);
//	global::thread_pool::instance().queue_request(
//        ( core::chain::instance().SetWork(new COracleWorkerThread))
	);	
	*/

    ::getch();
    return 0;
}
