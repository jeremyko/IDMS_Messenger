// MyButtonUtil.h: interface for the CMyButtonUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_)
#define AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



// Define
#define	BUTTON_SETUP				0  //환경설정
#define BUTTON_CLOSE				1	//닫기  
#define BUTTON_CLOSE_SMALL			2	//닫기 (Small Size)
#define BUTTON_REFRESH				3	//화면 갱신 
#define BUTTON_SAVE					4	//화면 저장
#define BUTTON_PRINT				5	//인쇄 
#define BUTTON_PRINT_SMALL			6	//인쇄 (Small Size)
#define BUTTON_EXCEL				7	//엑셀 변환
#define BUTTON_SETTING				8	//환경 설정
#define BUTTON_SEARCH				9	//조회
#define BUTTON_SEARCH_SMALL			10	//조회 (Small Size)



//구성 조회
#define BUTTON_TAB_NETWORK			12   //탭 - 계통
#define BUTTON_TAB_NODE				13   //탭 - 단국


//MX2800, ETC, N-DCS
#define BUTTON_TAB_MX2800			14	 //탭 - MX2800 
#define BUTTON_TAB_ETC				15   //탭 - 기타장비
#define BUTTON_TAB_NDCS				16   //탭 - N-DCS

//구성변경이력 
#define BUTTON_TAB_NODE_MUX			17   //탭 - 단국 MUX

// 상위 계통단국,대국장비 조회 화면 
#define BUTTON_TAB_MX2800_EX		20   //탭 - MX2800
#define BUTTON_TAB_ETC_EX			21   //탭 - 기타장비


//구성 관리
#define BUTTON_MNG_ADD				101	 //추가 
#define BUTTON_MNG_MODI				102  //변경 
#define BUTTON_MNG_DEL				103  //삭제
#define BUTTON_MNG_ADD_SMALL		104	 //추가(Small) 
#define BUTTON_MNG_MODI_SMALL		105  //변경(Small)  
#define BUTTON_MNG_DEL_SMALL		106  //삭제(Small) 
#define BUTTON_MNG_CLOSE_SMALL		107  //닫기(Small) 
#define BUTTON_SEARCH_VERYSMALL		108  //검색(Small) 
#define	BUTTON_SEARCH_DLG			109	 //검색창 뛰우는 작은 조회버튼 (단국/더미장비 관리 ..)
#define	BUTTON_RELATION_CANCEL		110	 //대국 취소버튼


//더미 접속 / 관리 화면
#define BUTTON_DUMMY_CONN			200  //더미장비접속
#define BUTTON_RECONNECT			201  //재연결
#define BUTTON_SCREENSAVE			202	 //화면저장
#define BUTTON_GALMURI				203  //갈무리 시작
#define BUTTON_GALMURIEND			204  //갈무리 끝 
#define BUTTON_DUMMY_ADD			205	 //등록 (더미장비 접속 화면)  
#define BUTTON_DUMMY_MODI			206  //변경 (더미장비 접속 화면) 
#define BUTTON_DUMMY_DEL			207  //삭제 (더미장비 접속 화면)  



//장애 현황
#define BUTTON_ALERTLOG				301	 //장애로그 
#define BUTTON_ALERTSTATUS			303  //장애현황
#define BUTTON_ALERTSTATUSDB		304  //장애이력 DB

//가청정보설정
#define BUTTON_ALERTSOUND_PATH		302  //장애사운드 
#define BUTTON_ALERTSOUND_PLAY		305  //장애사운드 듣기	 


//세부 필터 
#define BUTTON_DFILTER_NETWORK_ALL  401  //모두선택
#define BUTTON_DFILTERCFG			402	 //세부필터 설정
#define BUTTON_DFILTERCFGBRIEF		403	 //세부필터


//원격제어
#define BUTTON_ADD_DIALOG			501	 //등록화면
#define BUTTON_RUN					502	 //실행
#define BUTTON_REMOTE_CANCEL		503	 //취소


//단국순서 변경
#define BUTTON_DOWN					601	 //▽
#define BUTTON_UP					602	 //△

//CRS
#define BUTTON_REFRESH_DB			701	 //DB갱신 


//작업중/Inhibit 조회
#define BUTTON_TAB_WORKING			801	 //탭 - 작업중 
#define BUTTON_TAB_INHIBIT			802  //탭 - Inhibit


//선번장
#define BUTTON_SUNBUNJANG			901	 //선번장

//원격접속 보기 버튼 
#define BUTTON_DUMMYTERMINAL_SHOW	902

//기타
#define BUTTON_OK					1000
#define BUTTON_CANCEL				1001
#define BUTTON_APPLY				1002  //적용
#define BUTTON_APPLY2				1003  //적용
#define BUTTON_COLOR_TEXT			1004
#define BUTTON_COLOR_BG				1005

//가청경보 닫기
#define BUTTON_CLOSE_AUDIBLE		1006

//응답창
#define BUTTON_RESPONSEWND			1007


// MX-2800 관련
#define BUTTON_MX2800_ALARM_SEARCH	1050
#define BUTTON_MX2800_ALARM_SYNC	1051
#define BUTTON_MX2800_STATUS_SEARCH	1052
#define BUTTON_MX2800_RESPONSE		1053



class CMyButtonUtil  
{
public:
	CMyButtonUtil();
	virtual ~CMyButtonUtil();

	void  AutoSetting(UINT nType, CWnd *pButton, CWnd* wndParent);
};

#endif // !defined(AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_)
