
#include "stdafx.h"
#include <conio.h>
#include <iostream>
#include <time.h>

#include "oracl.h"
#include "SP_Utility.h"
#include "thread_pool.h"
#include "log.h"
using namespace plugware;
#pragma comment(lib, "oraclm32.lib")

class CIDMS_ServerDlg;
class CThreadSockAsyncServer; 
class COracleWorkerMT;


/********************************************************************************
  SQL 작업 결과 CALLBACK 함수 전달
 *******************************************************************************/
typedef struct _S_CALLBACKINFO
{
	_S_CALLBACKINFO()
	{
		m_pThread = NULL;
		m_nRecordCnt = 0;
		m_strData = "";
		memset(m_szUsage		,0x00,  sizeof(m_szUsage));
		memset(m_szIDTo			,0x00,  sizeof(m_szIDTo));
		memset(m_szIDFrom		,0x00,  sizeof(m_szIDFrom));		
		memset(m_szIPFrom		,0x00,  sizeof(m_szIPFrom));
		memset(m_szMultiRoleUsefulInfo		,0x00,  sizeof(m_szMultiRoleUsefulInfo));		
		m_bRslt = FALSE; 
		memset(m_szErrMsg		,0x00,  sizeof(m_szErrMsg));
	}

	HWND 	m_pWnd; 
	LPVOID	m_pThread;
	UINT	m_nRecordCnt	;
	//CString m_strData;
	string m_strData;
	char m_szUsage		[3 +1];
	char m_szIDTo		[20+1];
	char m_szIDFrom		[20+1];		
	char m_szIPFrom		[15+1];	
	//! 클라이언트의 DB요청이 들어오는 시점에서의 보관이 필요한 정보.
	char m_szMultiRoleUsefulInfo[2048];
	
	BOOL m_bRslt; 
	char m_szErrMsg		[100];	
	COracleWorkerMT* m_pCOracleWorkerMT;
									
} S_CALLBACKINFO;

typedef void (CALLBACK* ORANOTIFYPROC)(S_CALLBACKINFO* pSCallBackInfo);



class COracleWorkerMT : public core::CWorkUnit 
{
	class COraTypeLength : public CObject
	{
	public:
		int				m_nIndex;
		short			m_nServerType;
		unsigned short  m_nFieldLength;
	public:
		COraTypeLength() 
		{ 
			m_nIndex = 0;
			m_nServerType = 0; 
			m_nFieldLength = 0;
		}
		
		COraTypeLength( int nIndex, short ntype, unsigned short nLen ) 
		{ 
			m_nIndex = nIndex;
			m_nServerType = ntype; 
			m_nFieldLength = nLen; 
		}
		
		COraTypeLength( const COraTypeLength& a ) // Copy constructor
		{ 
			m_nIndex = a.m_nIndex; 
			m_nServerType = a.m_nServerType; 
			m_nFieldLength = a.m_nFieldLength; 
		} 
		
		const COraTypeLength& operator=( const COraTypeLength& a )
		{
			m_nIndex = a.m_nIndex; 
			m_nServerType = a.m_nServerType; 
			m_nFieldLength = a.m_nFieldLength; 
			return *this;
		}

		BOOL operator==(COraTypeLength a)
		{
			return (
				m_nIndex == a.m_nIndex && 
				m_nServerType == a.m_nServerType && 
				m_nFieldLength == a.m_nFieldLength
				);
		}
	};

	public: 
		COracleWorkerMT()
		{
			memset(m_szDBConn	, 0x00,	sizeof(m_szDBConn	) );		
			memset(m_szUser		, 0x00,	sizeof(m_szUser		) );		
			memset(m_szPassWd	, 0x00,	sizeof(m_szPassWd	) );
			memset(m_szUsage	, 0x00,	sizeof(m_szUsage	) );		
			memset(m_szIDTo	    , 0x00,	sizeof(m_szIDTo		) );		
			memset(m_szIDFrom	, 0x00, sizeof(m_szIDFrom	) );		
			memset(m_szSql		, 0x00, sizeof(m_szSql	) );					
			memset(m_szMultiRoleUsefulInfo		,0x00,  sizeof(m_szMultiRoleUsefulInfo));		
			
			m_nSqlMode = SQL_EXEC ;
			m_nindex = 0;
		}

		COracleWorkerMT(const char* szDBConn, const char* szUser, const char* szPassWd)
		{			
			m_nSqlMode = SQL_EXEC ;

			memset(m_szDBConn	, 0x00,	sizeof(m_szDBConn	) );		
			memset(m_szUser		, 0x00,	sizeof(m_szUser		) );		
			memset(m_szPassWd	, 0x00,	sizeof(m_szPassWd	) );
			memset(m_szUsage	, 0x00,	sizeof(m_szUsage	) );		
			memset(m_szIDTo	    , 0x00,	sizeof(m_szIDTo		) );		
			memset(m_szIDFrom	, 0x00, sizeof(m_szIDFrom	) );		
			memset(m_szSql		, 0x00, sizeof(m_szSql	) );					
			memset(m_szMultiRoleUsefulInfo		,0x00,  sizeof(m_szMultiRoleUsefulInfo));		
			
			strncpy(m_szDBConn , szDBConn , sizeof(m_szDBConn));
			strncpy(m_szUser , szUser, sizeof(m_szUser));
			strncpy(m_szPassWd , szPassWd, sizeof(m_szPassWd));	
			m_nindex = 0;
		}

		void SetLoginInfo(const char* szDBConn, const char* szUser, const char* szPassWd)
		{
			memset(m_szDBConn	, 0x00,	sizeof(m_szDBConn	) );		
			memset(m_szUser		, 0x00,	sizeof(m_szUser		) );		
			memset(m_szPassWd	, 0x00,	sizeof(m_szPassWd	) );
			memset(m_szUsage	, 0x00,	sizeof(m_szUsage	) );		
			memset(m_szIDTo	    , 0x00,	sizeof(m_szIDTo		) );		
			memset(m_szIDFrom	, 0x00, sizeof(m_szIDFrom	) );							
			memset(m_szMultiRoleUsefulInfo		,0x00,  sizeof(m_szMultiRoleUsefulInfo));		
				
			
			strncpy(m_szDBConn , szDBConn , sizeof(m_szDBConn));
			strncpy(m_szUser , szUser, sizeof(m_szUser));
			strncpy(m_szPassWd , szPassWd, sizeof(m_szPassWd));	
		}

		void SetSQL(const char* szSql)
		{
			memset(m_szSql , 0x00, sizeof(m_szSql	) );
			strncpy(m_szSql, szSql, sizeof(m_szSql) );			
		};
						
		/********************************************************************************
		  RollBack
		 *******************************************************************************/
//		void RollBack()
//		{
//			if(m_nSqlMode == SQL_EXEC) //20061129 조회성 아니면 트랜잭션 처리한다.
//			{
//				m_osess.Rollback(false);			
//			}			
//		}

		/********************************************************************************
		  Commit
		 *******************************************************************************/			
//		void Commit()
//		{
//			if(m_nSqlMode == SQL_EXEC) //20061129
//			{
//				m_osess.Commit(false);			
//			}			
//		}

		/********************************************************************************
		  callback 시 요청한 작업을 구별..
		 *******************************************************************************/		
		void SetID	( const char* szUsage, const char* szIDTo, const char* szIDFrom, const char* szIpFrom	) 
		{		
			strncpy(m_szUsage	, szUsage,	sizeof(m_szUsage	) );		
			strncpy(m_szIDTo	, szIDTo,	sizeof(m_szIDTo		) );		
			strncpy(m_szIDFrom	, szIDFrom, sizeof(m_szIDFrom	) );
			strncpy(m_szIPFrom	, szIpFrom, sizeof(m_szIPFrom	) );
		}

		/********************************************************************************
		  callback 함수 불려지는시점에서 요청시점의 정보가 필요한경우
		 *******************************************************************************/
		void SetUsefulInfo	( const char* szData) 
		{
			strncpy(m_szMultiRoleUsefulInfo , szData, sizeof(m_szMultiRoleUsefulInfo));
			
		}		
		
		/********************************************************************************
		  Wnd에서 SQL결과 받을 경우 Noti Proc Set
		 *******************************************************************************/
		void setNotiProcWnd(ORANOTIFYPROC pNotifyProc, HWND pWnd) 
		{
			m_pNotifyProc	= pNotifyProc;
			m_pWnd		=  pWnd; //!!
		}

		/********************************************************************************
		  Wnd 아닌 경우 Noti Proc Set
		 *******************************************************************************/
		void setNotiProcThread(ORANOTIFYPROC pNotifyProc, CThreadSockAsyncServer* pWnd) 
		{
			m_pNotifyProc	= pNotifyProc;
			m_pThread		=  pWnd; //!!
		}
		
		/********************************************************************************
		  처리로직 가상함수 정의..
		 *******************************************************************************/	
		
		
		BOOL process() throw() 
		{
			S_CALLBACKINFO sCallBackInfo;			
			
			OStartup(OSTARTUP_MULTITHREADED); 
			
			ODatabase	odb;
			OSession	osess; 
			ODynaset	odyn;	
			
			/*
			char szSessionNameIndex[10];
			if( strncmp(m_szSql, "SELECT", 6) != 0 )
			{				
				memset(szSessionNameIndex, 0x00, sizeof(szSessionNameIndex));

				long nSesionIndex = COraSessionName::Instance().getOraSessionIndex();
				::ltoa(nSesionIndex, szSessionNameIndex,sizeof(szSessionNameIndex));
				//TRACE ("szSessionNameIndex [%s]\n", szSessionNameIndex);

				OSession named_osess(szSessionNameIndex); // 새로운 Session을 시작해야함!!!
				osess = named_osess ; //!!!
			}
			*/
			
			oresult nOsessRslt = osess.Open();

			if( OSUCCESS != nOsessRslt)
			{
				CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "osess.Open() Error!! ErrNum [%d] [%s]\n", osess.ErrorNumber() ,m_szSql);					 
			}
			
			if( strncmp(m_szSql, "SELECT", 6) == 0 )
			{	
				m_nSqlMode = SQL_SELECT ;
			}
			else
			{
				m_nSqlMode = SQL_EXEC ;

				if( OFAILURE  == osess.BeginTransaction() )
				{
					TRACE("BeginTransaction ERROR!!");					
					
					CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Ln [%d] BeginTransaction : m_nindex [%d] [%s]\n", __LINE__, m_nindex ,m_szSql);
					
					sprintf(m_szErrMsg , "[Ln:%d] BeginTransaction Error!", __LINE__ );
					sCallBackInfo.m_pWnd = m_pWnd;
					sCallBackInfo.m_pThread = m_pThread;
					sCallBackInfo.m_pCOracleWorkerMT = this;
						
					strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
					strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
					strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
					strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
					strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
						
					sCallBackInfo.m_bRslt = FALSE;
					strncpy(sCallBackInfo.m_szErrMsg , m_szErrMsg , sizeof(sCallBackInfo.m_szErrMsg));
					
					osess.Close();
					odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print
					OShutdown();		
					
					m_pNotifyProc (&sCallBackInfo) ;				

					return FALSE;
				}
			}
			
			if ( ! osess.IsOpen() )
			{		
				//sprintf(m_szErrMsg , "[Ln:%d] [%d] %s szSessionNameIndex[%s] [%d]", __LINE__, osess.ServerErrorNumber(), osess.GetServerErrorText(), szSessionNameIndex, m_nindex);
				sprintf(m_szErrMsg , "[Ln:%d] [%d] %s ", __LINE__, osess.ServerErrorNumber(), osess.GetServerErrorText());
				//Log			
				CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "m_szUsage [%s] m_szIDFrom [%s] [%s]\n", m_szUsage, m_szIDFrom ,m_szSql);
				
				sCallBackInfo.m_pWnd = m_pWnd;
				sCallBackInfo.m_pThread = m_pThread;
				sCallBackInfo.m_pCOracleWorkerMT = this;
					
				strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
				strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
				strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
				strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
				strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
					
				sCallBackInfo.m_bRslt = FALSE;
				strncpy(sCallBackInfo.m_szErrMsg , m_szErrMsg , sizeof(sCallBackInfo.m_szErrMsg));
				
				osess.Close();
				odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print
				OShutdown();		
				
				m_pNotifyProc (&sCallBackInfo) ;				

				return FALSE;
			}			
			
			oresult nRslt = odb.Open( osess, m_szDBConn, m_szUser, m_szPassWd);
			
			if( OSUCCESS != nRslt)
			{
				CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Error!! ErrNum [%d] [%s]\n", odb.ErrorNumber() ,m_szSql);					 
			}

			if ( ! odb.IsOpen() )
			{ 
				sprintf(m_szErrMsg , "[Ln:%d] [%d] %s [%d]", __LINE__, odb.ServerErrorNumber(), odb.GetServerErrorText(), m_nindex );
				//Log			
				CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Odb Open Error!! %s [%s]\n", m_szErrMsg ,m_szSql);	

				sCallBackInfo.m_pWnd = m_pWnd;
				sCallBackInfo.m_pThread = m_pThread;
				sCallBackInfo.m_pCOracleWorkerMT = this;

				strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
				strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
				strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
				strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
				strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
				
				sCallBackInfo.m_bRslt = FALSE;
				strncpy(sCallBackInfo.m_szErrMsg , m_szErrMsg , sizeof(sCallBackInfo.m_szErrMsg));
								
				osess.Close(); 
				odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print
				
				OShutdown();		

				m_pNotifyProc (&sCallBackInfo) ;

				return FALSE;
			}
						

			//BOOL bRslt = RunSql();
			BOOL bRslt = TRUE;

			if( SQL_SELECT == m_nSqlMode )
			{
				if ( odyn.Open(odb, m_szSql) != OSUCCESS)
				{
					sprintf(m_szErrMsg , "[Ln: %d] [%d] %s [%d]", __LINE__, odb.ServerErrorNumber(), odb.GetServerErrorText(), m_nindex);
					//Log								
					CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Error!! %s [%s]\n", m_szErrMsg ,m_szSql);	

					bRslt =  FALSE;
				}
			}
			else
			{
				if (odb.ExecuteSQL(m_szSql) != OSUCCESS)
				{
					sprintf(m_szErrMsg , "[Ln: %d] [%d] %s [%d]",__LINE__, odb.ServerErrorNumber(), odb.GetServerErrorText(), m_nindex);
					//Log
					CLog::Instance().WriteFile(FALSE, "ServerSQLError",  "Error!! %s [%s]\n", m_szErrMsg ,m_szSql);
					
					bRslt = FALSE;
				}				
			}

			if (bRslt == FALSE)
			{
				if(m_nSqlMode == SQL_EXEC)
				{
					osess.Rollback(false);					
				}
								
				odyn.Close();				
				osess.Close();				
				odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print
				
				OShutdown();
								
				sCallBackInfo.m_pWnd = m_pWnd;
				sCallBackInfo.m_pThread = m_pThread;
				sCallBackInfo.m_pCOracleWorkerMT = this;
				
				strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
				strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
				strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
				strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
				strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
				
				sCallBackInfo.m_bRslt = FALSE;
				strncpy(sCallBackInfo.m_szErrMsg , m_szErrMsg , sizeof(sCallBackInfo.m_szErrMsg));
				
				m_pNotifyProc (&sCallBackInfo) ;

				return FALSE;
			}
			
			int isopen = odyn.IsOpen();

			if (isopen)
			{			
				int index = 0;
				short nServerDataType = 0 ;
				int nFieldCnt = odyn.GetFieldCount();
				int nRecordCnt = odyn.GetRecordCount();

				sCallBackInfo.m_nRecordCnt = nRecordCnt;

				for(index=0; index < nFieldCnt; index ++)
				{
					nServerDataType = odyn.GetFieldServerType(index);					
					//DisplayGetServerTypeInfo(nServerDataType);					

					OField tmpFirmd = odyn.GetField(index);
					unsigned short nFieldSize =  static_cast<unsigned short>(tmpFirmd.GetSize()); // or long nFieldSize =  m_odyn.GetFieldSize(index);
					
					COraTypeLength* pData = new COraTypeLength( index, nServerDataType, nFieldSize );
					m_FieldTypeLenList.AddTail(pData); 
				}				
				
				// 구분자 Data 생성 
				while( odyn.IsEOF() == FALSE )
				{
					POSITION getPos = m_FieldTypeLenList.GetHeadPosition();
					while( getPos != NULL )
					{					
						//int nBndChk = 0;
						int nLenTotal = 0;
						char * pSzVarchar = NULL;
						COraTypeLength *pSData = static_cast<COraTypeLength*> ( m_FieldTypeLenList.GetNext(getPos) );						
						
						switch(pSData->m_nServerType)
						{
							case OTYPE_VARCHAR:
							case OTYPE_VARCHAR2:
							case OTYPE_STRING:							
							case OTYPE_CHAR:
							case OTYPE_CHARZ:
								{	
									unsigned short nLen = static_cast<unsigned short> (pSData->m_nFieldLength + 1);
									pSzVarchar = new char[nLen];
									memset(pSzVarchar, 0, pSData->m_nFieldLength);
									if (OSUCCESS == odyn.GetFieldValue(pSData->m_nIndex, pSzVarchar, nLen))
									{
										//nLenTotal = sCallBackInfo.m_strData.GetLength();
										nLenTotal = sCallBackInfo.m_strData.length();
									
										sCallBackInfo.m_strData += pSzVarchar;
										sCallBackInfo.m_strData += DELIM1;
									}
									break;
								}
							case OTYPE_DATE:
								{										
									pSzVarchar = new char[20];
									memset(pSzVarchar, 0, pSData->m_nFieldLength);
									if (OSUCCESS == odyn.GetFieldValue(pSData->m_nIndex, pSzVarchar, 20))
									{
										//nLenTotal = sCallBackInfo.m_strData.GetLength();
										nLenTotal = sCallBackInfo.m_strData.length();

										sCallBackInfo.m_strData += pSzVarchar;
										sCallBackInfo.m_strData += DELIM1;
									}
									
									break;
								}
							case OTYPE_NUMBER:
							case OTYPE_SINT:
							case OTYPE_UINT:
								{
									char szTmpNum[20];
									memset(szTmpNum, 0x00, sizeof(szTmpNum));

									int nlTmp = 0;
									if (OSUCCESS == odyn.GetFieldValue(pSData->m_nIndex, &nlTmp) )
									{
										sprintf(szTmpNum, "%d", nlTmp);
									
										//nLenTotal = sCallBackInfo.m_strData.GetLength();
										nLenTotal = sCallBackInfo.m_strData.length();
										sCallBackInfo.m_strData += szTmpNum;
										sCallBackInfo.m_strData += DELIM1;
									}
									
									break;
								}							
							case OTYPE_LONG:
							case OTYPE_FLOAT:
								{
									char szTmpNum[20];
									memset(szTmpNum, 0x00, sizeof(szTmpNum));

									double nlTmp = 0.0;
									if (OSUCCESS == odyn.GetFieldValue(pSData->m_nIndex, &nlTmp))
									{
										sprintf(szTmpNum, "%f", nlTmp);
									
										//nLenTotal = sCallBackInfo.m_strData.GetLength();
										nLenTotal = sCallBackInfo.m_strData.length();
									
										sCallBackInfo.m_strData += szTmpNum;
										sCallBackInfo.m_strData += DELIM1;
									}

									break;
								}								
						
							//case OTYPE_RAW:
							//case OTYPE_LONGRAW:							
							//case OTYPE_ROWID:
							//case OTYPE_CURSOR:
							//case OTYPE_MSLABEL:								
							//	break;
							
						}
						
						if(pSzVarchar)
						{
							delete[] pSzVarchar;						
							pSzVarchar = NULL;
						}
					}					
					
					odyn.MoveNext();								
					
				} //While				
			}
			else // if (isopen)
			{				
				TRACE("Dynaset not opened\n");
			}
			
			//EXIT_HOLE : // goto..~

			if( sCallBackInfo.m_nRecordCnt == 0)	
			{	
				TRACE("데이터 없음\n");
								
				if(m_nSqlMode != SQL_EXEC)
				{						
					//TRACE("ERROR 조회성이면 데이터없음 오류 경우임\n");
					odyn.Close();					
					osess.Close();
					odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print
					OShutdown();
										
					sCallBackInfo.m_pWnd = m_pWnd;
					sCallBackInfo.m_pThread = m_pThread;
					sCallBackInfo.m_pCOracleWorkerMT = this;

					strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
					strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
					strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
					strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
					strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
					sCallBackInfo.m_bRslt = FALSE;
					strncpy(sCallBackInfo.m_szErrMsg , NODATA_FOUND , sizeof(sCallBackInfo.m_szErrMsg));

					m_pNotifyProc (&sCallBackInfo) ;
				
					return FALSE;
				}
			}	
			
			if(m_nSqlMode == SQL_EXEC)
			{
				TRACE("Commit\n");
				
				osess.Commit(false);			
			}

			odyn.Close();			
			osess.Close();			
			odb.Close(); //20090209  http://www.oracle.com/technology/software/tech/windows/ole/htdocs/readme/92048.html?_template=/ocom/print

			OShutdown();						
								
			sCallBackInfo.m_pWnd = m_pWnd;
			sCallBackInfo.m_pThread = m_pThread;
			sCallBackInfo.m_pCOracleWorkerMT = this;
			
			strncpy(sCallBackInfo.m_szUsage , m_szUsage, sizeof(sCallBackInfo.m_szUsage) );
			strncpy(sCallBackInfo.m_szIDTo , m_szIDTo, sizeof(sCallBackInfo.m_szIDTo) );
			strncpy(sCallBackInfo.m_szIDFrom , m_szIDFrom, sizeof(sCallBackInfo.m_szIDFrom) );
			strncpy(sCallBackInfo.m_szIPFrom , m_szIPFrom, sizeof(sCallBackInfo.m_szIPFrom) );
			strncpy(sCallBackInfo.m_szMultiRoleUsefulInfo , m_szMultiRoleUsefulInfo, sizeof(sCallBackInfo.m_szMultiRoleUsefulInfo) );
			sCallBackInfo.m_bRslt = TRUE;
			strncpy(sCallBackInfo.m_szErrMsg , "" , sizeof(sCallBackInfo.m_szErrMsg));

			//Log
			//m_End = GetTickCount();			
			//writeLogFile("c:\\data\\IDMSServerSQLp.log", "SQL 조회종료 m_szUsage[%s] => [%d] \n", m_szUsage,  m_End - m_Start);				

			m_pNotifyProc (&sCallBackInfo) ;

			return TRUE;			
		}
		
				
		/********************************************************************************
		  작업 종료시 처리로직 정의..
		 *******************************************************************************/		
		void ProcWhenDone() throw() 
		{				
			// Heap 메모리 클리어 							
			POSITION clearPos = m_FieldTypeLenList.GetHeadPosition();
			while( clearPos != NULL )
			{
				COraTypeLength *pSData = static_cast<COraTypeLength*> ( m_FieldTypeLenList.GetNext(clearPos) );
				if(pSData)
				{
					delete pSData;
					pSData = NULL;
				}
			}

			m_FieldTypeLenList.RemoveAll();			
		}

		// 데이터 타입 정보 출력 Debug
		/*
		void DisplayGetServerTypeInfo(int nServerDataType)
		{				
			switch (nServerDataType)
			{
			case OTYPE_VARCHAR2:
				std::cout << "DATA TYPE : OTYPE_VARCHAR2 [" << OTYPE_VARCHAR2 << "]" << std::endl;			
				break;
			case OTYPE_NUMBER:
				std::cout << "DATA TYPE : OTYPE_NUMBER [" << OTYPE_NUMBER << "]" << std::endl;			
				break;
			case OTYPE_SINT:
				std::cout << "DATA TYPE : OTYPE_SINT [" << OTYPE_SINT << "]" << std::endl;			
				break;
			case OTYPE_FLOAT:
				std::cout << "DATA TYPE : OTYPE_FLOAT [" << OTYPE_FLOAT << "]" << std::endl;			
				break;
			case OTYPE_STRING:
				std::cout << "DATA TYPE : OTYPE_STRING [" << OTYPE_STRING << "]" << std::endl;			
				break;
			case OTYPE_LONG:
				std::cout << "DATA TYPE : OTYPE_LONG [" << OTYPE_LONG << "]" << std::endl;			
				break;
			case OTYPE_VARCHAR:
				std::cout << "DATA TYPE : OTYPE_VARCHAR [" << OTYPE_VARCHAR << "]" << std::endl;			
				break;
			case OTYPE_ROWID:
				std::cout << "DATA TYPE : OTYPE_ROWID [" << OTYPE_ROWID << "]" << std::endl;			
				break;
			case OTYPE_DATE:
				std::cout << "DATA TYPE : OTYPE_DATE [" << OTYPE_DATE << "]" << std::endl;			
				break;
			case OTYPE_RAW:
				std::cout << "DATA TYPE : OTYPE_RAW [" << OTYPE_RAW << "]" << std::endl;			
				break;
			case OTYPE_LONGRAW:
				std::cout << "DATA TYPE : OTYPE_LONGRAW [" << OTYPE_LONGRAW << "]" << std::endl;			
				break;
			case OTYPE_UINT:
				std::cout << "DATA TYPE : OTYPE_UINT [" << OTYPE_UINT << "]" << std::endl;			
				break;
			case OTYPE_CHAR:
				std::cout << "DATA TYPE : OTYPE_CHAR [" << OTYPE_CHAR << "]" << std::endl;			
				break;
			case OTYPE_CHARZ:
				std::cout << "DATA TYPE : OTYPE_CHARZ [" << OTYPE_CHARZ << "]" << std::endl;			
				break;
			case OTYPE_CURSOR:
				std::cout << "DATA TYPE : OTYPE_CURSOR [" << OTYPE_CURSOR << "]" << std::endl;			
				break;
			case OTYPE_MSLABEL:
				std::cout << "DATA TYPE : OTYPE_MSLABEL [" << OTYPE_MSLABEL << "]" << std::endl;			
				break;
			}
		}
		*/
	public:
		int		m_nSqlMode ; 
		int		m_nindex;
	private: 
		
		//int		m_nSqlMode ; 
		char    m_szDBConn [20] ;
		char	m_szUser   [20];
		char	m_szPassWd [10];
		
		ORANOTIFYPROC		m_pNotifyProc;		
		
		HWND	m_pWnd; 
		LPVOID	m_pThread; 
		
		CObList		m_FieldTypeLenList;
		
		char m_szErrMsg		[100];
		char m_szSql		[MAX_SQL_QUERY_LEN];
		char m_szUsage		[3+1];
		char m_szIDTo		[20+1];
		char m_szIDFrom		[20+1];		
		char m_szIPFrom		[15+1];

		//! 클라이언트의 DB요청이 들어오는 시점에서의 보관이 필요한 정보.
		char m_szMultiRoleUsefulInfo[2048];
		
		//DWORD m_Start, m_End; // TEST 
};





