#include "stdafx.h"
#include <conio.h>
#include <iostream>
#include <time.h>
#include <mysql.h>
#include "SP_Utility.h"
#include "thread_pool.h"
using namespace plugware;
#pragma comment(lib, "libmysql.lib")

#define MAX_DATA_LEN	80000
//#define DELIM			0x1F		// 구분자 정의

// 인자수가 너무 많은 문제.. -> 추후 고려대상...
typedef void (CALLBACK* MYSQLNOTIFYPROC)(LPVOID, char*, char*, char*, char*, char*, char*, char*, char*, BOOL bRslt, char* szErrMsg, UINT nCnt);

class CServerDlg;
class CIocpServer; //TEST

// 전송데이터 양식...
typedef struct { 
	int	    m_nDataLen;             // 총 데이터 길이
	int	    m_nRecordCnt;           // 총 데이터 건수
	//int	m_nDataConsecutive;		// 데이터 연속여부
	char	m_szData[MAX_DATA_LEN];   
} SMYSQLDATA;
	

class CMySqlWorkerMT : public core::CWorkUnit 
{
	
	public: 		
		CMySqlWorkerMT(CString strDBConn, CString strUser, CString strPassWd)
		{
			m_strDBConn = strDBConn;
			m_strUser = strUser;
			m_strPassWd = strPassWd;

			memset(m_szUsage	, 0x00,	sizeof(m_szUsage	) );		
			memset(m_szIDTo	, 0x00,	sizeof(m_szIDTo		) );		
			memset(m_szIDFrom	, 0x00, sizeof(m_szIDFrom	) );		
			memset(m_szGroup	, 0x00,	sizeof(m_szGroup	) );		
		}

		// Get..Set..
		void SetSQL(char* szSql)
		{
			//strcpy(m_szSql, szSql);
			m_strSql = szSql;
		};
		
		CString GetSQL()
		{
			//return m_szSql;
			return m_strSql;
		};
		
		/*
		void GetIdentity	(
								char* szUsage, 
								char* szIDTo, 
								char* szIDFrom, 
								char* szGroup
							)
		{
			strncpy(szUsage	, m_szUsage,	sizeof(szUsage	) );		
			strncpy(szIDTo	, m_szIDTo,		sizeof(szIDTo	) );		
			strncpy(szIDFrom, m_szIDFrom,	sizeof(szIDFrom	) );		
			strncpy(szGroup	, m_szGroup,	sizeof(szGroup	) );	
		}
		*/

		//callback 시 요청한 작업을 구별..
		void SetIdentity	(
								char* szUsage, 
								char* szIDTo, 
								char* szIDFrom, 								
								char* szIpFrom,
								char* szNickNmFrom,
								char* szRealNmFrom,
								char* szGroup
							) 
		{		
			strncpy(m_szUsage	, szUsage,	sizeof(m_szUsage	) );		
			strncpy(m_szIDTo	, szIDTo,	sizeof(m_szIDTo		) );		
			strncpy(m_szIDFrom	, szIDFrom, sizeof(m_szIDFrom	) );		

			strncpy(m_szIPFrom	, szIpFrom, sizeof(m_szIPFrom	) );		
			strncpy(m_szNickNmFrom	, szNickNmFrom, sizeof(m_szNickNmFrom	) );		
			strncpy(m_szRealNmFrom	, szRealNmFrom, sizeof(m_szRealNmFrom	) );		
			
			strncpy(m_szGroup	, szGroup,	sizeof(m_szGroup	) );		
		}
		
		void setNotiProc(MYSQLNOTIFYPROC pNotifyProc, CIocpServer* pWnd) 
		{
			m_pNotifyProc	= pNotifyProc;
			m_pWnd		=  pWnd;
		}

		// 선택적 SQL 실행
		BOOL RunSql()
		{	
			//AfxMessageBox("TEST 01");
			int		i=0, j=0, k=0,l=0 ;
			CString strQuery = GetSQL();
			strQuery.TrimLeft(' ');
			strQuery.TrimLeft('\t');
			strQuery.TrimLeft();
			
			{				
				if ( ! mysql_query( m_MyData, strQuery ) ) 
				{
					//AfxMessageBox("TEST 02");
					// Success
				}
				else
				{	
					if(strcmp(m_szUsage, static_cast<char*>(USAGE_ADD_COMP_LIST_STEP2)) == 0 && mysql_errno(m_MyData) == 1062)
					{
						// 대화상대 추가시의 데이터 중복 에러는 성공으로 간주.. :-P
						return TRUE;
					}				
					
					TRACE("err_no [%d] err_msg [%s]\n",mysql_errno(m_MyData), mysql_error(m_MyData));
					//m_strErr.Format(_T("MySql Error [%d] : %s "), mysql_errno(m_MyData), mysql_error(m_MyData));						
					return FALSE;
				}			
			}
			//AfxMessageBox("TEST 03");
			return TRUE;
		}		
		
	
		// 처리로직 가상함수 정의..
		void process() throw() 
		{	
			//AfxMessageBox("process 01");
			
			int nDataExists = 0;
			SMYSQLDATA sData;
			memset(&sData, 0x00, sizeof(sData));
							 
			int		i=0, j=0, k=0,l=0,x=0 ;
			
			if	( 
					(m_MyData = mysql_init((MYSQL*) 0)) && 
					//mysql_real_connect( m_MyData, NULL, "root", "rhdiddl1", NULL, MYSQL_PORT,	NULL, 0 ) 
					mysql_real_connect( m_MyData, NULL, (LPCSTR)m_strUser, (LPCSTR)m_strPassWd, NULL, MYSQL_PORT,	NULL, 0 ) 					
				)
			{
				m_MyData->reconnect= 1;
				//if ( mysql_select_db( m_MyData, "messenger" ) < 0 ) 
				if ( mysql_select_db( m_MyData, (LPCSTR)m_strDBConn ) < 0 ) 
				{
					TRACE( "Can't select the database !\n") ;
					//m_strErr.Format(_T("Can't select the database! error [%d] : %s\n"), MYSQL_PORT, mysql_errno(m_MyData), mysql_error(m_MyData));
					//AfxMessageBox(m_strErr);

					mysql_close( m_MyData ) ;
					m_pNotifyProc	(	(LPVOID) m_pWnd, sData.m_szData, 
										m_szUsage, 
										m_szIDTo, 
										m_szIDFrom, 
										m_szIPFrom	, 
										m_szNickNmFrom	, 
										m_szRealNmFrom	, 
										m_szGroup, 
										FALSE, 
										(LPTSTR)(LPCSTR)m_strErr, 
										i
									);
					return ;
				}
			}
			else 
			{
				//m_strErr.Format(_T("Can't connect to the mysql server on port %d ! error [%d] : %s\n"), MYSQL_PORT, mysql_errno(m_MyData), mysql_error(m_MyData));	
				mysql_close( m_MyData ) ;

				m_pNotifyProc	(
									(LPVOID) m_pWnd, 
									sData.m_szData, 
									m_szUsage, 
									m_szIDTo, 
									m_szIDFrom, 
									m_szIPFrom	, 
									m_szNickNmFrom	, 
									m_szRealNmFrom	, 
									m_szGroup, 
									FALSE, 
									(LPTSTR)(LPCSTR)m_strErr, 
									i
								);
				
				return ;
			}
			
			BOOL bRslt = RunSql();

			if (bRslt == FALSE)
			{
				mysql_close( m_MyData ) ;
				
				m_pNotifyProc	(
									(LPVOID) m_pWnd, 
									sData.m_szData, 
									m_szUsage, 
									m_szIDTo, 
									m_szIDFrom, 
									m_szIPFrom	, 
									m_szNickNmFrom	, 
									m_szRealNmFrom	, 
									m_szGroup, 
									FALSE, 
									(LPTSTR)(LPCSTR)m_strErr, 
									i
								);

				return ;
			}
			
			m_Res = mysql_store_result( m_MyData ) ;

			if(m_Res)
			{
				i = (int) mysql_num_rows( m_Res ) ; 
				l = 1 ;
				
				TRACE( "Query:  %s\nNumber of records found:  %ld\n", GetSQL(), i ) ;
								
				for ( x = 0 ; m_fld = mysql_fetch_field( m_Res ) ; x++ )
					strcpy( m_aSzFlds[ x ], m_fld->name ) ;
								
				while ( m_Row = mysql_fetch_row( m_Res ) ) 
				{
					j = mysql_num_fields( m_Res ) ;
					//printf( "Record #%ld:-\n", l++ ) ;
					
					nDataExists = 1;
					// 구분자 Data 생성 
					for ( k = 0 ; k < j ; k++ )
					{
						//printf( "  Fld #%d (%s): %s\n", k + 1, m_aSzFlds[ k ], (((m_Row[k]==NULL)||(!strlen(m_Row[k])))?"NULL":m_Row[k])) ;
						
						if( strcmp(m_Row[k] , "" ) == 0)
							strcat(sData.m_szData, " ");
						else
							strcat(sData.m_szData, m_Row[k]);

						sData.m_szData[strlen(sData.m_szData)] = DELIM1;
					}					
				}
				
				mysql_free_result( m_Res ) ;
			}

			if(nDataExists == 0)	
			{
				if	(
						strcmp(m_szUsage, static_cast<char*>(USAGE_REG_NEW_USER)) == 0 ||						
						strcmp(m_szUsage, static_cast<char*>(USAGE_AFT_LOGIN_UPDATE)) == 0 || 
						strcmp(m_szUsage, static_cast<char*>(USAGE_ADD_COMP_LIST_STEP1)) == 0  || 
						strcmp(m_szUsage, static_cast<char*>(USAGE_ADD_COMP_LIST_STEP2)) == 0  || 
						strcmp(m_szUsage, static_cast<char*>(USAGE_GET_COMP_LIST)) == 0  || 
						strcmp(m_szUsage, static_cast<char*>(USAGE_DEL_COMP_LIST)) == 0  						
					)
				{
					TRACE("데이터 없음 에러로 보지 않는 경우");
				}
				else
				{
					mysql_close( m_MyData ) ;
					m_pNotifyProc	(
										(LPVOID) m_pWnd, 
										sData.m_szData, 
										m_szUsage, 
										m_szIDTo, 
										m_szIDFrom, 
										m_szIPFrom	, 
										m_szNickNmFrom	, 
										m_szRealNmFrom	, 
										m_szGroup, 
										FALSE, 
										(LPSTR)(LPCSTR)m_strErr, 
										i
									);	
				
					return ;
				}
			}			
			
			mysql_close( m_MyData ) ;
						
			m_pNotifyProc	(
								(LPVOID) m_pWnd, 
								sData.m_szData, 
								m_szUsage, 
								m_szIDTo, 
								m_szIDFrom,
								m_szIPFrom	, 
								m_szNickNmFrom	, 
								m_szRealNmFrom	, 
								m_szGroup, 
								TRUE, 
								"", 
								i
							);	

			return ;			
		}
		
		// 작업 종료시 처리로직 재정의..
		void ProcWhenDone() throw() 
		{		
			std::cout << "ProcWhenDone() Call / Usage[%s] IDTo[%s] IDFrom[%s]" << m_szUsage << m_szIDTo << m_szIDFrom << std::endl;				
		}

	public:		
		CString m_strDBConn ;
		CString m_strUser ;
		CString m_strPassWd ;
		CString		m_strErr;		
		MYSQLNOTIFYPROC		m_pNotifyProc;
		//CIDMS_ServerDlg*	m_pWnd;
		CIocpServer *	m_pWnd; //TEST
		
		char		m_aSzFlds[ 100 ][ 50 ], m_SzDB[ 50 ] ;
		const  char   * m_pSzT;
		MYSQL		* m_MyData ;
		MYSQL_RES	* m_Res ;
		MYSQL_FIELD	* m_fld ;
		MYSQL_ROW	  m_Row ;
	protected:
		CString m_strSql;
	public: 
				
		char m_szUsage		[5];
		char m_szIDTo		[20];
		char m_szIDFrom		[20];		
		char m_szIPFrom		[20];			
		char m_szNickNmFrom	[255];
		char m_szRealNmFrom	[20];	
	
		char m_szGroup[50];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



