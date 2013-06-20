 #ifndef __ALL_COMMON_DEFINES_H__
#define __ALL_COMMON_DEFINES_H__
#define MSET(msg) memset(msg, 0x00, sizeof(memset))

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

#include <ws2tcpip.h>
#include "..\\common\\tpipv6.h"
#include <string>

const int	C_FILE_SVR_PORT = 40440 ;

#define		_USER_GRADE				_T("G")
#define		_END_DATE				_T("99991231")
#define		NEW_LOG_IN   "00"   //이전 사용자 최초 LogIn
#define		USE_LOG_IN   "01"   //사용중인 사용자 LogIn
#define		DEL_LOG_IN   "02"   //삭제된 사용자 LogIn

const int FILETRANSLOG = 1 ;

const int ROLE_SEND_INPUT = 1;
const int ROLE_DISPLAY = 2;

const int   MAX_WAIT_TIME		=	5000 ; // MiliSec
const int   TRY_COUNT			=	3 ;

//const int   MAX_CS_CNT_SOCK		=	10 ; // 20070209 
//const int   MAX_CS_CNT			=	10 ; // 20070209 
const int	MAX_PATH_LEN		=	1024 ;
const int	MAX_BUFF_SIZE		=	4096 ;
const int	MAX_SQL_QUERY_LEN	=	2048 ;
const char  DELIM1				=	0x1F;	///< 구분자 정의
const char  DELIM2				=	0x1E;	///< 구분자 정의
const int   C_NICK_NAME_LEN     =	100;
const int   C_IT_COMP_LEN       =	300;
const int   C_GRP_NAME_LEN      =	50 ;
const int	SQL_EXEC_TRAN		=	1;
const int	SQL_EXEC			=	0;
const int	SQL_SELECT			=	1;
const int	C_ROLE_CHG_COMP_GRP	=	0;
const int	C_ROLE_CHG_GRP_NAME	=	1;
const int	C_ROLE_ADD_GRP		=	2;
const int	C_ROLE_CHG_IT_COMP	=	0;
const int	C_ROLE_ADD_IT_COMP	=	1;
const int	C_CHATDLG			=	1;
const int	C_MULTICHATDLG		=	2;
const int	C_ALLMSGDLG			=	3;
const int	CAWAY_DEFAULT		= 600 ;
const int	MAX_DESCRIPTION_BUF_LEN = 600;
const int   MAX_SEND_FILE_NAME = 2048;
const int   MAX_MULTI_CHAT_SESSION = 3;
const int   MAX_TRY_CONNECT        = 10; 
const int   CHK_TIMER_SEC = 1000*60; // 1 Min.
const int	MAX_CHAT_LEN = 10240;  

#define CFL_AWAY		0x01
#define CFL_BUSY		0x02
#define CFL_LOOK_AWAY	0x04

const char* const NODATA_FOUND	= 	"1403";
const char* const IDMS_MAIN		= "IdmsMain.exe" ;
const char* const IDMS_MSN_NAME	= "IdmsMsn" ; 
const char* const IDMS_MSN_SVR_NAME	= "IDMS_Server"  ;


// Usage (송/수신 동시사용)
const char* const STATUS_AWAY				=   "000" ; 
const char* const STATUS_ONLINE				=   "001" ; 
const char* const CHAT_MSG		  			=   "002" ; 
const char* const GET_BUDDY_GRP  			=   "003" ; 
const char* const GET_BUDDY_LIST			=   "004" ; 
const char* const SEARCH_ID 				=   "005" ; 
const char* const RAWSQL_JOB_GN     		=   "006" ; 
const char* const RAWSQL_DAILYJOB_MST  		=   "007" ; 
const char* const RAWSQL_GETUSER_INFO		=   "009" ; 
const char* const RAWSQL_TEAM        		=   "010" ; //set1
const char* const RAWSQL_GETCOMPANY_INFO	=   "011" ; //set1
const char* const RAWSQL_GETCLASS_INFO 		=   "012" ; //set1
const char* const RAWSQL_GETBUDDY_INFO		=   "013" ; ///set2
const char* const SOMEONE_ADD_ME			=   "014" ; ///set2
const char* const DEL_BUDDY_GRP				=   "015" ; 
const char* const DEL_BUDDY 				=   "016" ; ///< 대화상대삭제
const char* const CHG_BUDDY_GRP  			=   "017" ; ///< 대화상대 그룹변경
const char* const ADD_BUDDY_GRP				=   "018" ; 
const char* const CHG_GRP_NAME  			=   "019" ; ///< 그룹 이름 변경
const char* const YOUALLOWEDASCOMP 			=   "020" ; ///< 삭제 대화상대를 다시 대화상대로 추가함. (A=>B 경우 A에게 응답)
const char* const SOMEONE_DEL_ME			=   "021" ; ///< 누군가가 나를 대화상대에서 삭제함.
const char* const ACK_NICK_CHG				=   "022" ; 
const char* const SOMEONE_WANTS_SEND_FILES 	=   "023" ; ///< 파일을 보내길 원함
const char* const MULTICHAT_SERVER_ROOMNO	=   "024" ;
const char* const CANCEL_FILE_TRANSFER	 	=   "025" ; ///< 파일 전송 취소
const char* const ALL_MSG		  			=   "026" ; ///< 공지사항
const char* const ALL_MSG_ACK	  			=   "027" ; ///< 공지사항 확인
const char* const FOLDER_SELECTING  	 	=   "028" ; 
const char* const NO_THANKS_FILE_TRANSFER 	=   "029" ; 
const char* const RAWSQL_REG_NEW_USER		=   "030" ; ///< 사용자 등록
const char* const RAWSQL_SAVE_LAST_IP  		=   "031" ; 
const char* const ALREADY_LOGIN  			=   "032" ; ///< 이미 로그인 했음!
const char* const LOGIN_ALLOWED  			=   "033" ; ///< 로그인 해라.
const char* const RAWSQL_INSERT_LOGONTIME  	=   "034" ; 
const char* const RAWSQL_CHK_DUP_ID			=   "035" ; ///< 사용자 중복CHK
const char* const RAWSQL_CHK_USERID			=   "036" ; ///< 비밀번로 변경시 사용자 CHK
const char* const RAWSQL_UPDATE_PASSWD 		=   "037" ; 
const char* const ADD_BUDDY_LIST_ERR		=   "038" ; 
const char* const ACK_ONLINE	  			=   "039" ; ///< 온라인임을 알림
const char* const ACK_OFFLINE	  			=   "040" ; ///< OFF라인임을 알림
const char* const YOUALLOWEDASCOMP_INFORM	=   "041" ; ///< 삭제 대화상대를 다시 대화상대로 추가함. (A=>B 경우 B에게 알림)
const char* const RAWSQL_UPDATE_LOGOFFTIME  =   "042" ; 
const char* const ADD_BUDDY_LIST			=   "043" ; 
const char* const CHK_ALREADY_LOGIN  		=   "044" ; ///< 이미 로그인 확인
const char* const CHNG_NICK					=   "045" ; 
const char* const MULTICHAT_GET_ROOMNO		=   "046" ; 
const char* const MULTI_CHAT_MSG     		=   "047" ; 
const char* const SET_MULTICHAT_SERVER_ROOMNO = "048" ; 
const char* const EXIT_MULTI_CHAT			=   "049" ;
const char* const SLIP_MSG                  =   "050" ;
const char* const GET_SLIP_MSG              =   "051" ;
const char* const RAWSQL_ACK_SLIP_MSG       =   "052" ;
const char* const ADD_IT_COMP               =   "053" ;
const char* const DEL_IT_COMP               =   "054" ;
const char* const CHG_IT_COMP               =   "055" ;
const char* const GET_COMP_LIST_INIT		=   "056"  ; /// server inner use
const char* const STATUS_BUSY		        =   "057"  ; /// 다른용무중
const char* const LOGIN		                =   "058"  ; /// LOG 요청
const char* const FOOD_MENU                 =   "059"  ; 
const char* const ALL_USER_ID_TEAM          =   "060"  ;
const char* const GET_ALL_USER_INIT         =   "061"  ; /// server inner use
const char* const ALL_USER_ID_TEAM_FORADD   =   "062"  ;
const char* const UPDATE_ALL_USER_INIT      =   "063"  ; /// server inner use
const char* const ALERT_PENDING_WORK        =   "064"  ; /// server inner use
const char* const IM_ALIVE                  =   "065"  ; 

const char* const LOAD_TEST				    =   "999"  ; /// server test use

const char* const USAGE_CHK_DB				= 	"100"  ;
const char* const ID_MEANINGLESS			= 	"200"  ;

static char* m_szUsageAry[] =
{
	"STATUS_AWAY"             ,    /*01*/
	"STATUS_ONLINE"           ,    /*02*/
	"CHAT_MSG"                ,    /*03*/
	"GET_BUDDY_GRP"           ,    /*04*/
	"GET_BUDDY_LIST"          ,    /*05*/
	"SEARCH_ID"               ,    /*06*/
	"RAWSQL_JOB_GN"           ,    /*07*/
	"RAWSQL_DAILYJOB_MST"     ,    /*08*/
	"008"                     ,    /*09*/
	"RAWSQL_GETUSER_INFO"     ,    /*10*/
	"RAWSQL_TEAM"             ,    /*11*/
	"RAWSQL_GETCOMPANY_INFO"  ,    /*12*/
	"RAWSQL_GETCLASS_INFO"    ,    /*13*/
	"RAWSQL_GETBUDDY_INFO"    ,    /*14*/
	"SOMEONE_ADD_ME"          ,    /*15*/
	"DEL_BUDDY_GRP"           ,    /*16*/
	"DEL_BUDDY"               ,    /*17*/
	"CHG_BUDDY_GRP"           ,    /*18*/
	"ADD_BUDDY_GRP"           ,    /*19*/
	"CHG_GRP_NAME"            ,    /*20*/
	"YOUALLOWEDASCOMP"        ,    /*21*/
	"SOMEONE_DEL_ME"          ,    /*22*/
	"ACK_NICK_CHG"            ,    /*23*/
	"SOMEONE_WANTS_SEND_FILES",    /*24*/
	"MULTICHAT_SERVER_ROOMNO" ,    /*25*/
	"CANCEL_FILE_TRANSFER"    ,    /*26*/
	"ALL_MSG"                 ,    /*27*/
	"ALL_MSG_ACK"             ,    /*28*/
	"FOLDER_SELECTING"        ,    /*29*/
	"NO_THANKS_FILE_TRANSFER" ,    /*30*/
	"RAWSQL_REG_NEW_USER"     ,    /*31*/
	"RAWSQL_SAVE_LAST_IP"     ,    /*32*/
	"ALREADY_LOGIN"           ,    /*33*/
	"LOGIN_ALLOWED"           ,    /*34*/
	"RAWSQL_INSERT_LOGONTIME" ,    /*35*/
	"RAWSQL_CHK_DUP_ID"       ,    /*36*/
	"RAWSQL_CHK_USERID"       ,    /*37*/
	"RAWSQL_UPDATE_PASSWD"    ,    /*38*/
	"ADD_BUDDY_LIST_ERR"      ,    /*39*/
	"ACK_ONLINE"              ,    /*40*/
	"ACK_OFFLINE"             ,    /*41*/
	"YOUALLOWEDASCOMP_INFORM" ,    /*42*/
	"RAWSQL_UPDATE_LOGOFFTIME",    /*43*/
	"ADD_BUDDY_LIST"          ,    /*44*/
	"CHK_ALREADY_LOGIN"       ,    /*45*/
	"CHNG_NICK"               ,    /*46*/
	"MULTICHAT_GET_ROOMNO"    ,    /*47*/
	"MULTI_CHAT_MSG"          ,    /*48*/
	"SET_MULTICHAT_SERVER_ROOMNO" ,/*49*/
	"EXIT_MULTI_CHAT"         ,    /*50*/
	"SLIP_MSG"                ,    /*51*/
	"GET_SLIP_MSG"            ,    /*52*/
	"RAWSQL_ACK_SLIP_MSG"     ,    /*53*/
	"ADD_IT_COMP"             ,    /*54*/
	"DEL_IT_COMP"             ,    /*55*/
	"CHG_IT_COMP"             ,    /*56*/
	"GET_COMP_LIST_INIT"      ,    /*57*/
	"STATUS_BUSY"             ,    /*58*/
	"LOGIN"                   ,    /*59*/
	"FOOD_MENU"               ,    /*60*/
	"ALL_USER_ID_TEAM"        ,    /*61*/
	"GET_ALL_USER_INIT"       ,    /*62*/
	"ALL_USER_ID_TEAM_FORADD" ,    /*63*/
	"UPDATE_ALL_USER_INIT"    ,    /*64*/
	"ALERT_PENDING_WORK"      ,    /*65*/
	"IM_ALIVE"                     /*66*/
};

#define INI_LOGIN_SEC    		_T("LOGIN_SEC")
#define INI_LOGIN_ID 			_T("LOGIN_ID")
#define INI_AWAY_SEC    		_T("AWAY_SEC")
#define INI_SEC_CONVER_COLOR 	_T("CONVERSATION COLOR")
#define INI_COLOR_ME_R 			_T("COLOR_ME_R_VAL")
#define INI_COLOR_ME_G 			_T("COLOR_ME_G_VAL")
#define INI_COLOR_ME_B 			_T("COLOR_ME_B_VAL")
#define INI_COLOR_COMP_R 		_T("COLOR_COMP_R_VAL")
#define INI_COLOR_COMP_G 		_T("COLOR_COMP_G_VAL")
#define INI_COLOR_COMP_B 		_T("COLOR_COMP_B_VAL")
#define INI_COLOR_BG_R 			_T("COLOR_BG_R_VAL")
#define INI_COLOR_BG_G 			_T("COLOR_BG_G_VAL")
#define INI_COLOR_BG_B 			_T("COLOR_BG_B_VAL")

#define INI_SEC_TIME_CONF      	_T("TIME_CONF")
#define INI_SEC_CONVER_FONT 	_T("CONVERSATION FONT")
#define INI_FONT 				_T("FONT")

#define INI_SEC_SERVER_INFO 	_T("SERVER_INFO")
#define INI_SERVER_PORT			_T("SERVER_PORT")
#define INI_SERVER_IP			_T("SERVER_IP")
#define INI_SERVER_DB_SID		_T("SERVER_DB_SID")
#define INI_SERVER_DB_USER		_T("SERVER_DB_USER")
#define INI_SERVER_DB_PASS		_T("SERVER_DB_PASS")

#define INI_SEC_AUTO_LOGIN  	_T("AUTO_LOGIN")
#define INI_USE_AUTO_LOGIN  	_T("USE_AUTO_LOGIN")
#define INI_PASSWD          	_T("PASSWD")

#define INI_SEC_OPTION  	    _T("OPTION")
#define INI_ALWAYS_ON_TOP		_T("ALWAYS_ON_TOP")
#define INI_MY_IMAGE_PATH		_T("MY_IMAGE_PATH")

#define INI_USE_DOWNFOLDER		_T("USE_DOWNFOLDER")
#define INI_DOWNFOLDER_PATH		_T("DOWNFOLDER_PATH")

#define INI_USE_LOGFOLDER		_T("USE_LOGFOLDER")
#define INI_LOGFOLDER_PATH		_T("LOGFOLDER_PATH")


#define INI_SEC_SOUND  	        _T("SOUND")
#define INI_USE_SOUND	    	_T("INI_USE_SOUND")
#define INI_SOUND_PATH	    	_T("INI_SOUND_PATH")


#define INI_GRP_FOLDED			_T("FOLDED_GRP")

#define INI_SEC_WINPOS  	    _T("WINDOW_POSITION")
#define INI_PREV_POS_SIZE_MAIN	_T("MAIN_POS")
#define INI_PREV_POS_SIZE_CHAT		_T("CHAT_DLG_POS")
#define INI_PREV_POS_CHAT_SPLITTER	_T("CHAT_SPLITTER_POS")

#define INI_SEC_DB  			_T("DB_LOGIN")
#define INI_DB_KIND				_T("DB_KIND")


//상용 응답 목록
#define INI_SEC_REPLY  	    _T("REPLY_LIST")
#define INI_REPLY_LIST0     _T("REPLY_LIST0")
#define INI_REPLY_LIST1     _T("REPLY_LIST1")
#define INI_REPLY_LIST2     _T("REPLY_LIST2")
#define INI_REPLY_LIST3     _T("REPLY_LIST3")
#define INI_REPLY_LIST4     _T("REPLY_LIST4")
#define INI_REPLY_LIST5     _T("REPLY_LIST5")
#define INI_REPLY_LIST6     _T("REPLY_LIST6")
#define INI_REPLY_LIST7     _T("REPLY_LIST7")
#define INI_REPLY_LIST8     _T("REPLY_LIST8")
#define INI_REPLY_LIST9     _T("REPLY_LIST9")


#define INI_SEC_SELECT_SEND		_T("SELECT_SEND")
#define INI_USE_SELECT_SEND		_T("USE_SELECT_SEND") 

// 자동부착글
#define INI_SEC_AUTO_APPEND		_T("AUTO_APPEND")
#define INI_USE_AUTO_APPEND		_T("USE_AUTO_APPEND") 
#define INI_AUTO_APPEND_MSG		_T("AUTO_APPEND_MSG") 

// User Message
#define WM_DB_VALID						(WM_USER + 10)
#define WM_SQL_ERR      				(WM_USER + 20)
 
#define WM_CHK_DUP_ID					(WM_USER + 100)
#define WM_INIT_WORK					(WM_USER + 101)
#define WM_REGUSER_INFO					(WM_USER + 102)
#define WM_TEAMCOMBO_INFO				(WM_USER + 103)
#define WM_COMPANYCOMBO_INFO			(WM_USER + 104)
#define WM_CLASSCOMBO_INFO				(WM_USER + 105)
#define WM_REG_NEW_USER 				(WM_USER + 106)
#define WM_CHK_USERID					(WM_USER + 107) ///< 비밀번호 변경시 사용자 CHK
#define WM_UPDATE_PASSWD				(WM_USER + 108) ///< 비밀번호 변경
#define WM_LOGIN						(WM_USER + 109) ///< 로그인
#define WM_SAVE_LAST_IP					(WM_USER + 110) ///< 
#define WM_JOBGN     					(WM_USER + 111) 
#define WM_DAILYJOB_MST					(WM_USER + 112) 
#define WM_INSERT_LOGONTIME				(WM_USER + 113) 
#define WM_BUDDY_LIST   				(WM_USER + 114) 
#define WM_CHATMSG						(WM_USER + 116)
#define WM_CHGCOMPINFO					(WM_USER + 119)
#define WM_BUDDY_STSTUS					(WM_USER + 120)
#define WM_ADD_BUDDY					(WM_USER + 123)
#define WM_BUDDY_GRP    				(WM_USER + 125)
#define WM_ADD_BUDDY_GRP   				(WM_USER + 126)
#define WM_DEL_BUDDY_GRP   				(WM_USER + 127)
#define WM_SOMEONE_ADD_ME  				(WM_USER + 128)
#define WM_DEL_BUDDY_ID  				(WM_USER + 129)
#define WM_SOMEONE_DEL_ME  				(WM_USER + 130)
#define WM_CHG_BUDDY_GRP  				(WM_USER + 131)
#define WM_CHG_GRP_NAME  				(WM_USER + 132)
#define WM_BUDDY_ONLINE  				(WM_USER + 133)
#define WM_BUDDY_OFFLINE  				(WM_USER + 134)
#define WM_CHATDLG_CLOSE				(WM_USER + 135)
#define WM_ALREADY_LOGIN				(WM_USER + 136)
#define WM_LOGIN_ALLOWED				(WM_USER + 137)
#define WM_ALLMSG_ACK					(WM_USER + 138)
#define WM_TO_ALLMSG					(WM_USER + 139)
#define WM_ALLMSG_CLOSE					(WM_USER + 140)
#define WM_ALLMSG						(WM_USER + 141)
#define WM_VIEWALLMSG_CLOSE				(WM_USER + 142)
#define WM_ALLOWED_COMP					(WM_USER + 143)
#define WM_YOU_ALLOWED_BY				(WM_USER + 144)
#define WM_CHNG_NICK    				(WM_USER + 145)
#define WM_AWAY_TRUE    				(WM_USER + 146)
#define WM_AWAY_FALSE    				(WM_USER + 147)
#define WM_SEARCH_ID    				(WM_USER + 148)
#define WM_SERVER_CLOSED   				(WM_USER + 149)
#define WM_SERVER_RE_CONNECTED			(WM_USER + 150)
#define WM_SLIP_MSG_SUCCESS  			(WM_USER + 151)
#define WM_SLIP_MSG          			(WM_USER + 152)
#define WM_SLIPMSG_DLG_CLOSE   			(WM_USER + 153)
#define WM_ADD_IT_COMP       			(WM_USER + 154)
#define WM_DEL_IT_COMP      			(WM_USER + 155)
#define WM_CHG_IT_COMP      			(WM_USER + 156)
#define WM_CONCNT           			(WM_USER + 157)
#define WM_STATUS_BUSY         			(WM_USER + 158)
#define WM_FOOD_MENU         			(WM_USER + 159)
#define WM_ALL_USER_ID_TEAM    			(WM_USER + 160)
#define WM_ALL_USER_ID_TEAM_FORADD		(WM_USER + 161)


#define WM_MULTI_CHATMSG				(WM_USER + 180)
#define WM_EXIT_MULTI_CHAT				(WM_USER + 181)
#define WM_MULTI_CHAT_GUEST_CLOSE		(WM_USER + 182)
#define WM_MULTICHAT_ACCEPT				(WM_USER + 184)
#define WM_MULTI_CHATDLG_CLOSE			(WM_USER + 185)
#define WM_MCHAT_SERVER_ROOMNO			(WM_USER + 186)
#define WM_ALIVE            			(WM_USER + 187)
#define WM_SETREPLY            			(WM_USER + 188)
#define WM_EMOTICON_MAP					(WM_USER + 189)
#define WM_FOCUS_INPUT					(WM_USER + 190)
#define WM_CHG_CONFIG					(WM_USER + 191)


#define WM_IMADEUSERLIST         		(WM_USER + 400)
#define WM_IMADE_ALL_USERLIST      		(WM_USER + 401)
#define WM_SENDALLMSGACKED      		(WM_USER + 402)

// 좌측 Section Data  
#define WM_SECTION_REQUEST				(WM_USER+500)
#define WM_SECTION_RESPONSE				(WM_USER+510)



struct FileTransInfo
{
	int nPercent,nSpeed, nRemnantTime;
	__int64 nReceiveSize, nFileSize;
};

struct st_FileList
{
	long nSize[20];
	char file_list[20][256];
	st_FileList()
	{
		ZeroMemory(nSize, sizeof(long)*20);
		ZeroMemory(file_list, 20*256);
	};
};

const int REQUEST_FILE				= 1001;

//! 공통 구조체 헤더
typedef struct _S_COMMON_HEADER
{
	// 4+9+5 = 18 
	char m_szUsage  [3+1];	///< 데이터 구분
	char m_szLength [8+1];	///< 전체 길이
	char m_szCount  [4+1];  ///< 데이터 갯수	
	                 
} S_COMMON_HEADER;
const int COMMON_HEADER_LENGTH	= 18; 

//! 공통 응답
typedef struct _S_COM_RESPONSE
{
	char szRsltCd [1+1];	
	char szErrMsg [100+1];
	                 
} S_COM_RESPONSE;
//#define COMMON_ERRORINFO_LENGTH	sizeof(S_COM_RESPONSE) ///< S_COM_RESPONSE 의 size
const int COMMON_ERRORINFO_LENGTH	= 103;

typedef struct _S_FROMIP_HEADER
{	
	// 사용자의 구분은 IP로 한다..
	char m_szIP [15 + 1];	
	                 
} S_FROMIP_HEADER;
//#define FROMIP_HEADER_LENGTH	sizeof(S_FROMIP_HEADER) ///< S_FROMIP_HEADER 의 size
const int FROMIP_HEADER_LENGTH	= 16;

typedef struct _S_FROMTOID_HEADER
{	
	// 사용자의 구분 ID로 
	char m_szIP [15 + 1];	
	char m_szFromID [20 + 1];
	char m_szToID [20 + 1];	
	                 
} S_FROMTOID_HEADER;
//#define FROMTOID_HEADER_LENGTH	sizeof(S_FROMTOID_HEADER) ///< S_FROMID_HEADER 의 size
const int FROMTOID_HEADER_LENGTH = 58;

//RawSQL 응답은 처리결과와 에러메시지를 보낸다..
typedef struct _S_SQL_RSLTDATA
{	
	char szRsltCd	[1+1];	
	char szErrMsg	[100+1];
	                 
} S_SQL_RSLTDATA;
//#define SQL_RSLTDATA_LENGTH	sizeof(S_SQL_RSLTDATA) ///< S_SQL_RSLTDATA 의 size
const int SQL_RSLTDATA_LENGTH = 103;


typedef struct _SCOMBO_INFO
{
	BOOL    bIsSuccess;
	CString strErrorMSg;
	CString	m_strCodeName    	;
	CString m_strCode           ;	
} SCOMBO_INFO ;

//!
typedef struct _S_CHK_USERID
{
	char szExists [1+1];
	char szPasswd [16+1]; 
	                 
} S_CHK_USERID;

//! 사용자 등록 화면 초기화시 사용자 정보 조회 결과
typedef struct _SUSER_INFO
{
	BOOL    bIsSuccess;
	CString strErrorMSg;
	CString	m_strJUMIN_NO    	;
	CString m_strUSER_NAME       ;
	CString m_strUSER_ID         ;
	CString m_strPASSWD          ;
	CString m_strTEAM_GN         ;
	CString m_strOFF_TEL_NO      ;
	CString m_strOFF_IN_NO       ;
	CString m_strHP_TEL_NO       ;
	CString m_strJOB_WRITE_GN    ;
	CString m_strJOB_CLASS       ;
	CString m_strCOMPANY_POS     ;
} SUSER_INFO ;

typedef struct _S_LOGIN
{
	BOOL    bIsSuccess;
	CString strErrorMSg;
	CString strKT_GN		  ;
	CString strUSER_NAME      ;
	CString strPASSWD         ;
	CString strTEAM_GN        ;
	CString strTEAM_NAME      ;
	CString strUSER_GRADE     ;
	CString strTOP_TEAM_GN    ;
	CString strTOP_TEAM_NAME  ;
	CString strSTATUS         ;	                 
	CString strNick           ;	  
	CString strIAMS_AUTHORITY ;	  
	CString strPROG_SELECT ;	  
	
} S_LOGIN;

typedef struct _S_JOBGN
{	
	CString strUserID		  ;
	CString strJobGn      ;	                 
} S_JOBGN;

typedef struct _S_DAILYJOB_MST
{	
	CString strJobDay	;
	CString strLogOnTime    ;	                 
	CString strLogOffTime    ;	                 
} S_DAILYJOB_MST ;

//Client 사용
class CBuddyInfo: public CObject
{
	public:
	
	char	m_szIpAddr		[15 + 1];	
	char	m_szUserName	[50 + 1];	
	char	m_szNickName	[C_NICK_NAME_LEN + 1];	
	char	m_szBuddyID		[20 + 1];	
	char	m_szBuddyGrp	[50 + 1];	
	BOOL	m_bConnStatus;
	int		m_nYouBlocked;	///< 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함) 9(정보친구)
	int     m_nAway;
	public:
    CBuddyInfo() 
	{	
		memset(m_szIpAddr	,0x00, sizeof(m_szIpAddr));
		memset(m_szUserName	,0x00, sizeof(m_szUserName));
		memset(m_szNickName	,0x00, sizeof(m_szNickName));
		memset(m_szBuddyID	,0x00, sizeof(m_szBuddyID));
		memset(m_szBuddyGrp	,0x00, sizeof(m_szBuddyGrp));
		
		m_bConnStatus = FALSE;
		m_nYouBlocked = 0;
		m_nAway = 0;
	}

	CBuddyInfo(const CBuddyInfo& obj)
	{		
		memset(m_szIpAddr	,0x00, sizeof(m_szIpAddr));
		memset(m_szUserName	,0x00, sizeof(m_szUserName));
		memset(m_szNickName	,0x00, sizeof(m_szNickName));
		memset(m_szBuddyID	,0x00, sizeof(m_szBuddyID));
		memset(m_szBuddyGrp	,0x00, sizeof(m_szBuddyGrp));
		m_bConnStatus = FALSE;
		m_nYouBlocked = 0;
		m_nAway = 0;
		
		strncpy(m_szIpAddr, obj.m_szIpAddr, sizeof(m_szIpAddr));
		strncpy(m_szUserName, obj.m_szUserName, sizeof(m_szUserName));
		strncpy(m_szNickName, obj.m_szNickName, sizeof(m_szNickName));
		strncpy(m_szBuddyID, obj.m_szBuddyID, sizeof(m_szBuddyID));
		strncpy(m_szBuddyGrp, obj.m_szBuddyGrp, sizeof(m_szBuddyGrp));
		
		m_bConnStatus = obj.m_bConnStatus;
		m_nYouBlocked = obj.m_nYouBlocked;	
		m_nAway =  obj.m_nAway;	
	}

} ;

typedef struct _S_BUDDY_STATUS
{
	char m_szUserID		[20 + 1];	
	char m_szIpAddr		[15 + 1];
	char m_szConStatus  [ 1 + 1];  
	char m_szAwayInfo   [ 1 + 1];  
} S_BUDDY_STATUS ;

// GROUP 추가 (서버로 전송)
typedef struct _S_ADD_BUDDY_GRP
{
	char	m_szUserID		[20 + 1];
	char	m_szBuddyGrp	[50 + 1];
} S_ADD_BUDDY_GRP ;


// 대화상대 추가 (Client->서버로 전송)
typedef struct _S_ADD_BUDDY
{
	char	m_szUserID		[20 + 1];	
	char	m_szBuddyID		[20 + 1];	
	char	m_szBuddyGrp	[50 + 1];
} S_ADD_BUDDY ;

// 대화상대 추가 (서버->Client로 전송)
typedef struct _S_ADD_BUDDY_CLISVR
{	
	char	m_szBuddyGrp	[50 + 1];
} S_ADD_BUDDY_CLISVR ;

typedef struct _S_ADD_BUDDY_RCV
{	
	CString m_strBuddyID	;	
	CString	m_strBuddyName	;
	CString	m_strBuddyNick	;
	CString	m_strBuddyIP	;
	CString	m_strBuddyGrp	;
	int     m_nBlock;
	BOOL	m_bConnStatus;
} S_ADD_BUDDY_RCV ;

typedef struct _S_BUDDY_GRP
{
	_S_BUDDY_GRP()
	{
		m_strBuddyGrp = "";
	};

	CString	m_strBuddyGrp;
} S_BUDDY_GRP ;

typedef struct _S_BUDDY_DEL
{		
	char	m_szBuddyID[20+1];
} S_BUDDY_DEL ;

typedef struct _S_BUDDY_CHG
{		
	char	m_szBuddyID	[20+1];
	char	m_szBuddyGrp[50 + 1];
} S_BUDDY_CHG ;

typedef struct _S_GRP_CHG_NAME
{		
	char	m_szBefGrp[50 + 1];
	char	m_szAftGrp[50 + 1];
} S_GRP_CHG_NAME ;

typedef struct _S_SOMEONE_DEL_ME
{		
	_S_SOMEONE_DEL_ME()
	{
		memset(m_szWhoDelMeID, 0x00, sizeof(m_szWhoDelMeID));
	}
	char	m_szWhoDelMeID[20+1];
} S_SOMEONE_DEL_ME ;

typedef struct _S_CHAT_CLIENT // Client 데이터 처리용
{
	CString strIDTo		;	
	CString strIDFrom	;	
	CString strMsg		;	                 
} S_CHAT_CLIENT;

//! chat 전송시에는  S_HEADER + S_CHAT_HEADER + 메시지.. 
typedef struct _S_CHAT_HEADER
{
	char szIDTo   [20+1];	///< chat 상대방 ID
	char szIDFrom [20+1];	///< chat My ID
	                 
} S_CHAT_HEADER;
//#define S_CHAT_HEADER_LENGTH	sizeof(S_CHAT_HEADER) ///< S_CHAT_HEADER 의 size
const int S_CHAT_HEADER_LENGTH = 42; 

typedef struct _S_BUDDY_ONLINE
{		
	char	m_szBuddyID[20+1];
	char	m_szBuddyIP[15+1];
} S_BUDDY_ONLINE ;

typedef struct _S_BUDDY_OFFLINE
{		
	char	m_szBuddyID[20+1];
} S_BUDDY_OFFLINE ;


//! 상대방이 파일 전송하겠다는 알림을 내가 받는경우 ->클라이언트에서 사용
typedef struct _S_SOMEONE_WANT_SEND_FILES
{	
	_S_SOMEONE_WANT_SEND_FILES()
	{
		memset(szFromID, 0x00, sizeof(szFromID));
		memset(szFromIP, 0x00, sizeof(szFromIP));
	}	

	char szFromID [20 + 1];
	char szFromIP [15+1]   ;
	std::string strFilePath   ;  
	std::string strRootPath   ;  
      
} S_SOMEONE_WANT_SEND_FILES;

typedef struct _S_ALLMSG_CLIENT // Client 
{
	_S_ALLMSG_CLIENT()
	{
		strIDTo = "";
		strIDFrom = "";
		strNameFrom = "";
		strMsg = "";
	}	

	CString strIDTo		;	
	CString strIDFrom	;	
	CString strNameFrom	;	
	CString strMsg		;	
	                 
} S_ALLMSG_CLIENT;


typedef struct _S_MULTI_CHAT_CLIENT // Client 데이터 처리용
{
	_S_MULTI_CHAT_CLIENT()
	{
		strIDTo = "";
		strIDFrom = "";
		strMsg = "";
		memset(szFromDlgID, 0x00, sizeof(szFromDlgID));
	}	

	CString strIDTo		;	
	CString strIDFrom	;	
	CString strMsg		;
	char	szFromDlgID [15+1]; // 각 화면당 고유 아이디	
	                 
} S_MULTI_CHAT_CLIENT;

typedef struct _S_MULTICHAT_CLOSEDLG
{
	_S_MULTICHAT_CLOSEDLG()
	{	
		memset(szFromIP, 0x00, sizeof(szFromIP));
		memset(szDlgId, 0x00, sizeof(szDlgId));
	}	
	char szFromIP			[15+1];		
	char szDlgId			[15+1];	
	
} S_MULTICHAT_CLOSEDLG;


//! File전송 CANCEL
typedef struct _S_CANCEL_FILE_TRANSFER
{
	_S_CANCEL_FILE_TRANSFER()
	{
		memset(szFromID, 0x00, sizeof(szFromID));
		memset(szFromIP, 0x00, sizeof(szFromIP));
	}	

	char szFromID			[20+1];		
	char szFromIP			[15+1];	
	
} S_CANCEL_FILE_TRANSFER;


typedef struct _S_YOU_ALLOWED_BY
{		
	char	m_szYouAllowedByID[20+1];
} S_YOU_ALLOWED_BY ;

typedef struct _S_YOU_ALLOWED
{		
	char	m_szYouAllowedID[20+1];
	char	m_szGrpName[C_GRP_NAME_LEN+1];
} S_YOU_ALLOWED ;


typedef struct _S_CHG_NICK // 대화명 변경
{
	char szIDFrom	[20+1];
	char szNickBef  [C_NICK_NAME_LEN+1];	
	char szNickAft  [C_NICK_NAME_LEN+1];	
} S_CHG_NICK;

typedef struct _S_AWAY_INFO // 자리비움
{
	char szIDFrom	[20+1];
} S_AWAY_INFO;

typedef struct _S_SEARCH_ID // 로그인 아이디 검색
{
	char szIDFrom	[20+1];
	char szID	[20 + 1];
	char szName	[50 + 1];	
	
} S_SEARCH_ID ;


typedef struct _S_SEARCHID_CLI // 로그인 아이디 검색 결과 => client 사용
{
	char szUserName		[50 + 1];
	char szUserId		[20 + 1];
	char szCompany		[50 + 1]; 
	char szTopTeamName	[50 + 1]; 
	char szTeamName		[50 + 1];  
	char szTel1			[14 + 1];  	
	char szTel2			[19 + 1];  	
	char szJobClassName [50 + 1];  
	char szCompanyuCode [4  + 1];  
	char szTopTeamCode	[4  + 1];   
	char szTeamCode		[4  + 1];  
	char szJobClassCode	[4  + 1]; 
	
} S_SEARCHID_CLI ;


//! 로그인 요청
typedef struct _S_LOGIN_S
{
	char szUerID  [20+1];		
	char szLogInIp[20+1];
	                 
} S_LOGIN_S;

//! 로그인 요청 응답 서버 -> 클라이언트
typedef struct _S_LOGIN_R
{
	char szPasswd [8+1]; 
	                 
} S_LOGIN_R;

//! 로그인 요청 응답 클라이언트 사용
typedef struct _S_LOGIN_R_CLI
{
	CString strRsltCd;	
	CString strErrMsg;
	CString strNicName ;	
	CString strRealName ;	
	                 
} S_LOGIN_R_CLI;

//! 대화상대 추가 요청
typedef struct _S_REG_NEW_COMP_S
{	
	char szMyUerID		[20+1];
	char szMyUerIP		[20+1];	
	char szCompanionID  [20+1];	
	char szMyNickName	[C_NICK_NAME_LEN+1];
	char szRealName		[20+1];
	char szMyGroup		[50+1]; // 추가되는 대화 상대가 소속되는 그룹
	char cKind   ;
	
} S_REG_NEW_COMP_S;


//! 대화상대 추가 응답
typedef struct _S_REG_NEW_COMP_R
{
	// A 가 B 추가시 A가 받는 응답 -> Client가 사용			
	char szCompanionID		[20+1];	  // 추가되는 상대방 ID
	char szCompUerIP		[20+1];	  // 추가되는 상대방 IP
	char szCompNickName		[C_NICK_NAME_LEN+1];   // 추가되는 상대방 가병
	char cConnStatus		;	      // 추가되는 상대방 접속상태
	char szCompRealName		[20+1];   // 추가되는 상대방 실명	
	
} S_REG_NEW_COMP_R;

//! 나를 대화상대로 추가한것을 알림
//! A가B를 대화상대로 추가시 A에게 추가 성공여부를 알리고 B가 연결된 사용자인 경우 서버에서 대화상대로 추가한것을 알림 

typedef struct _S_NOTI_SOMEONE_REG_ME_COMP
{
	char szSomeOneID		[20+1];	// 추가되는 상대방 ID
	char szSomeOneIP		[20+1];	// 추가되는 상대방 IP
	char szSomeOneNickName	[100+1];   // 추가되는 상대방 가명	
	char szSomeOneRealName	[20+1];   // 추가되는 상대방 실명	

} S_NOTI_SOMEONE_REG_ME_COMP;


//! 대화상대 목록 요청
typedef struct _S_COMP_LIST_S
{
	char szUerID  [20+1];	
	                 
} S_COMP_LIST_S;

//! 대화상대 목록 응답->클라이언트에서 사용
typedef struct _S_COMP_LIST_R
{	
	
	CString strCompanionID  ;	
	CString strCompanionIP  ;	
	CString strCompNickName ;
	BOOL	bConn_Status ;
	CString strRealName ;
	                 
} S_COMP_LIST_R;

// Client 가 접속한 경우 다른 Client 에게 알림
typedef struct _S_NOTI_SOMEONE_CONNECTED
{
	char szSomeOneIP	[15+1];	
	char szSomeOneID	[20+1];	
	
} S_NOTI_SOMEONE_CONNECTED;

// Client 가 접속 종료한 경우 다른 Client 에게 알림
typedef struct _S_NOTI_SOMEONE_DISCONNECTED
{
	char szSomeOneIP	[15+1];	
	char szSomeOneID	[20+1];	
	
} S_NOTI_SOMEONE_DISCONNECTED;

//! 파일 전송하겠음을 알림  ->동적으로 메모리 할당 전송됨, 클라이언트에서 사용
typedef struct _S_WANT_FILE_SEND_S
{
	CString szMyUerID    ;
	CString szMyIP       ;	
	CString szToID       ;	
	CString szToIP       ;	
	CString szFilePath   ;
	
} S_WANT_FILE_SEND_S;


typedef struct _S_SERVER_ROOMNO
{
	char m_szClientDlgID  [15+1];
	char m_szServerRoomNo [5+1];
	
} S_SERVER_ROOMNO;

typedef struct _S_SLIP_MSG //Client 사용
{		
	char m_szFromID [20 + 1];
	char m_szFromName [50 + 1];
	char m_szMsg    [1024 + 1];	
	char m_szRegDate[14 + 1];	
	                 
} S_SLIP_MSG;

typedef struct _S_ADD_IT_COMP
{		
	char	m_szDesc[100 + 1];
	char	m_szUrl [300 + 1];
} S_ADD_IT_COMP ;

typedef struct _S_IT_COMP_DEL
{		
	char	m_szName[100+1];
} S_IT_COMP_DEL ;

typedef struct _S_CHG_IT_COMP
{		
	char	m_szDescOld[100 + 1];
	char	m_szDesc[100 + 1];
	char	m_szUrl [300 + 1];
} S_CHG_IT_COMP ;


typedef struct _S_FOOD_MENU_PERIOD
{	
	char	m_szFromDate	[8 + 1];	
	char	m_szToDate	    [8 + 1];	
} S_FOOD_MENU_PERIOD ;

typedef struct _S_FOOD_MENU_CLI
{	
	char	m_szDate	[10  +1];
	char    m_szLD_FLAG [1  +1];
	char    m_szMenu    [200+1];
	char    m_szCal     [10 +1];
} S_FOOD_MENU_CLI ;


typedef struct _S_ALLID_TREEINFO
{	
	char   m_szID [20+1];
	int    m_nConStatus;
} S_ALLID_TREEINFO ;


//File 전송관련
// USAGE
#define USAGE_TAKE_FILE					"600"
#define USAGE_NO_THANKS_FILE_TRANSFER	"601"  
#define USAGE_FOLDER_SELECTING			"602"
#define USAGE_SEND_THIS_INFORMED_FILE	"603"
#define USAGE_MAKE_THIS_FILE        	"604"
#define USAGE_CANCEL_FILE_TRANSFER     	"605"
//window mwssage
#define WM_FOLDER_SELECTING			(WM_USER + 600)	
#define WM_SOMEONE_WANTS_SEND_FILES	(WM_USER + 601)
#define WM_SEND_THIS_INFORMED_FILE	(WM_USER + 602)
#define WM_MAKE_THIS_FILE        	(WM_USER + 603)
#define WM_CANCEL_FILE_TRANSFER    	(WM_USER + 604)
#define WM_NO_THANKS_FILE_TRANSFER 	(WM_USER + 605)
#define WM_SEND_BYTES            	(WM_USER + 606)
#define WM_RECV_BYTES            	(WM_USER + 607)
#define WM_DISCONNECTED            	(WM_USER + 608)
#define WM_FILE_SENDDLG_CLOSE      	(WM_USER + 609)
#define WM_FILE_RECVDLG_CLOSE      	(WM_USER + 610)


typedef struct _BUFFER_OBJ_FILE 
{
	BYTE				*buf;      // Data buffer for data
	int					buflen;    // Length of buffer or number of bytes contained in buffer	
	struct _BUFFER_OBJ_FILE	*next;     // Used to maintain a linked list of buffers
	BOOL	bIsFileData;		
	__int64	dwTotalFileLength;  
	__int64	dwReadFilePosition; // 읽을 위치 
	DWORD	dwReadLength;		// 읽을 길이 	

} BUFFER_OBJ_FILE;

//! 구조체 헤더
typedef struct _FILE_Header
{
	_FILE_Header()
	{
		memset(szUsage, 0x00, sizeof(szUsage));
		memset(szFilePath, 0x00, sizeof(szFilePath));
		memset(szLength, 0x00, sizeof(szLength));
		memset(szFileLength, 0x00, sizeof(szFileLength));		
		memset(szFromID, 0x00, sizeof(szFromID));		
		memset(szToIP, 0x00, sizeof(szToIP));		
	}

	char szUsage		[3  +1];		///< 데이터 구분(Function별)
	char szFilePath		[360+1];		///< 파일경로	
	char szLength		[16 +1];		///< 전체 길이	
	char szFileLength	[20 +1];		///< File 길이	
	char szFromID		[20 +1];	
	char szToIP			[15 +1];
	char szFromName		[20 +1];	//2000710
	                 
} FILE_HEADER;

#define FILE_HEADER_LENGTH 460   //2000710

typedef struct _SOCKET_OBJ_FILE
{
	BYTE*				m_pBuff;
	int					m_iBuffLength;	///< 모아놓은(m_pBuff) 데이터의 크기	
	char                m_szIP[15+1];
	FILE_HEADER			sPacketHeader;
	int					nTotalOnePacketLen;  // 전체 데이터 길이	
	SOCKET				s;              
	HANDLE				event;          
	int					listening;      // Socket is a listening socket (TCP)
	int					closing;        // Indicates whether the connection is closing
	SOCKADDR_STORAGE	addr;			// Used for client's remote address	
	int					addrlen;		// Length of the address
	
	//전송할 데이터 큐.
	BUFFER_OBJ_FILE		*pending,       // List of pending buffers to be sent
						*pendingtail;   // Last entry in buffer list
	
	struct _SOCKET_OBJ_FILE	*next,			// Used to link socket objects together
	                		*prev;		
	HWND                m_HwndFileRecv;
	HWND                m_HwndFileSend;
	char                m_szSockObjID[5+1];
	BOOL                m_bIsRealFileByteData ;
	char				m_szLocalFilePath [360+1];	///< 모든 문자포함 파일경로
} SOCKET_OBJ_FILE;

typedef struct _THREAD_OBJ_FILE 
{
	SOCKET_OBJ_FILE		*SocketList,          // Linked list of all sockets allocated
						*SocketListEnd;       // End of socket list
	int					SocketCount;          // Number of socket objects in list
	HANDLE				Event;                // Used to signal new clients assigned  to this thread
	HANDLE				Thread;
	HANDLE				Handles[MAXIMUM_WAIT_OBJECTS]; // Array of socket's event handles
	CRITICAL_SECTION	ThreadCritSec;		// Protect access to SOCKET_OBJ lists
	struct _THREAD_OBJ_FILE	*next;				// Next thread object in list
} THREAD_OBJ_FILE;

typedef struct _RCV_SOMEONE_WANT_SEND_FILE_S
{		
	char szFromIP [15+1]   ;
	char szFromID [20+1]   ;
	std::string        strFile      ;  
      
} ST_RCV_SOMEONE_WANT_SEND_FILE_S;

typedef struct _RCVED_FILE_INFO
{	
	char* szFilePath   ;
	__int64 nFileLen ;  
      
} ST_RCVED_FILE_INFO;

typedef struct _RCVED_BYTES_INFO
{		
	BYTE*				pBuff ;
	SOCKET_OBJ_FILE*	sock ;
	int					receive_length ;
	FILE_HEADER*		sFileHeader;
      
} ST_RCVED_BYTES_INFO;

class CSockObjMapInfoFile:public CObject
{
public:	
	CSockObjMapInfoFile();
	~CSockObjMapInfoFile();

	SOCKET_OBJ_FILE* m_pSockObj;
	THREAD_OBJ_FILE* m_pThreadObj;
};

#endif

