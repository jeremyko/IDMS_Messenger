//////////////////////////////////////////////////////////////////////
// CKoreaDate class - 한국형 양력, 음력 일자 클래스
// 
// 기능 : - SetDate(year, month, day, 양음구분, 윤달여부)
//             일자 지정 기능.
//             양력을 지정하면 음력 자동 계산, 음력을 지정하면 양력 자동 계산
//        - GetXXX
//             지정된 일자의 년, 월, 일, 요일, 윤달여부 등 구하기
//        - GetWeekCount()
//             지정된 일자가 속한 월의 주의 수 구하기
//        - GetWeekNumber()
//             지정된 일자가 몇 째 주에 속하는지 구하기
//        - IsDate(year, month, day, 양음구분, 윤달여부)
//             바른 날짜 값 여부 확인 기능. static function
//        - DateAdd(year, month, day, 양음구분)
//		       일자 연산 기능. 년, 월, 일을 동시에 더하거나 빼는 것이 가능하다.
//             음력에서 월을 계산할 때는 윤달도 확인한다. 음력은 1년이 12~14개월이다.
//        - HasLeapMonth(year, month)
//             음력에서 윤달이 있는 달인지 여부 확인 기능(음력만). static function
//        - GetDayOfWeek(year, month, day)
//             요일을 구하는 기능(양력만). static function
//             음력의 요일은 static function이 아닌 멤버함수(GetDayOfWeek)를 사용한다.
//             이름은 같지만 아규먼트가 없는 멤버함수이다.
//        - GetDaysOfMonth(year, month, 양음구분, 윤달여부)
//             지정 년월의 일자수(=마지막날) 구하기. static function
//        - GetPassedDays(year, month, day)
//             1년 1월 1일부터 지정 일자까지 지난 일자수 구하기. static function
//             두 일자의 차이를 일수로 구할 때 유용하다.
//             음력에서 구할 때는 static function이 아닌 멤버함수를 호출하면 구할 수 있다.
//        - IsHoliday()
//             휴일인지 여부 확인 기능
//        - GetDayInfo(TDayInfo diSolar, TDayInfo diLunar)
//             일자 정보 조회 기능(양력, 음력 동시에 같이 구함)
//        - GetDateString(양음구분, 옵션)
//             일자값 문자열 구하기. 상속해서 OnFormatDateString을 재정의하면
//             원하는 스타일로 출력을 구할 수 있다.
//        - 대입, 비교 연산자 지원(=, ==, >=, <=, >, <)
// 
// 양음력 지원 범위 :
//        - 양력은 1년 1월 1일 이후부터 가능
//        - 음력은 음력으로 1841년 ~ 2043년까지
// 
// 사용 예 :
//        CKoreaDate dt1(2002, 12, 25);  // 양력 2002.12.25
// 
//        // 2002.12.25에 해당하는 음력 일자를 디버그 창에 출력
//        TRACE("%s", (LPCTSTR)dt1.GetDateString(TYPE_LUNAR));
// 
//        CKoreanDate dt2;         // 시스템 일자로 초기화
//        dt2 = dt1;               // dt2를 dt1과 같은 날짜로 지정
//        dt1.DateAdd(3, 2, -1);   // 양력으로 3년 2개월을 더하고 1일을 뺀다.
//        dt2.DateAdd(3, 2, -1,    // 음력으로 3년 2개월을 더하고 1일을 뺀다.
//                    TYPE_LUNAR);   
//        if( dt1 == dt2 )         
//        {
//            TRACE0("같은 날입니다");
//        }
//        else
//        {
//            // 양력과 음력은 체계가 다르므로 같은 년월을 빼거나 더하면
//            // 거의 일자가 달라진다.
//            TRACE0("다른 날입니다");
//        }
// 
// Copyright :
//        본 소스코드(KoreanDate.h, KoreanDate.cpp)의 저작권은 저작자인
//        강영호(curealls@chollian.net)에 있습니다.
//        어떠한 용도로 사용해도 무방하나 이로 인해 발생하는 불이익에 대해서는
//        저작자에게 어떤 책임도 물을 수 없습니다.
//        저작자의 허락없이 소스코드(헤더 포함)를 수정하는 것은 안됩니다.
//        
// Bug report :        
//        버그를 발견하시면 아래 e-mail로 알려주시기 바랍니다.
//        curealls@chollian.net
//        강영호
//////////////////////////////////////////////////////////////////////
// KoreanDate.h: interface for the CKoreanDate class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KOREANDATE_H__EEB516D3_9793_4716_B4C6_199E3B6181B2__INCLUDED_)
#define AFX_KOREANDATE_H__EEB516D3_9793_4716_B4C6_199E3B6181B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const int TYPE_SOLAR = 1 << 0;
const int TYPE_LUNAR = 1 << 1;

// 공휴일 등 특정 년월에 대한 정보
typedef struct _tagDayInfo{
	int		nMonth;          // 월
	int		nDay;            // 일
	BOOL	bIsHoliday;      // 휴일 여부
	int		nType;           // TYPE_SOLAR:양력, TYPE_LUNAR:음력
	TCHAR	szDayName[20];   // 설명. 예)어린이날
} TDayInfo;

class CKoreanDate  
{
private:
	static const int BASE_PASSED;         // 양력 1년 1월 1일에서 1841년 1월 22일까지의 일자 수

	static int	 m_narrSolarDays[12];     // 각 월의 기본 날짜 수(양력)
	static TCHAR m_szLunarTable[203][12]; // 음력 변환 데이터
	static int	 m_narrDaysOfYear[203];   // 1841년 부터 매해의 일자수(음력)
	static long	 m_larrDaysAccm[203];     // 1841년 부터 매해 일자수의 누적(음력)
	static TDayInfo	m_arrDayInfoSolar[];  // 양력 공휴일, 기념일
	static TDayInfo	m_arrDayInfoLunar[];  // 음력   "

	BOOL	ConvertSolarToLunar();        // 양력 기준으로 음력 생성
	BOOL	ConvertLunarToSolar();        // 음력 기준으로 양력 생성
	BOOL	DateAddSolar(int nYear, int nMonth, int nDay);
	BOOL	DateAddLunar(int nYear, int nMonth, int nDay);

public:
	// Constructor, Destructor
	CKoreanDate();
	CKoreanDate(int nYear, int nMonth, int nDay,
		        int nType = TYPE_SOLAR,
			    BOOL bIsLeapMonth = FALSE);
	virtual ~CKoreanDate();

	// constants
	static const int BASE_YEAR;                                     // 음력 가능 시작 년도
	static const int MAX_YEAR;                                      // 음력 가능 마지막 년도

	// utilities
	static	BOOL	IsDate(int nYear, int nMonth, int nDay,         // 가능한 일자 값인지 검사
		                   int nType = TYPE_SOLAR,        
					       BOOL bIsLeapMonth = FALSE);
	static	int		GetDaysOfMonth(int nYear, int nMonth,           // 지정 년월의 일자 수를 구한다.
		                           int nType = TYPE_SOLAR,           
								   BOOL bIsLeapMonth = FALSE);          
	static	int		GetDayOfWeek(int nYear, int nMonth, int nDay);  // 지정 일자의 요일을 구한다.
	static	long	GetPassedDays(int nYear, int nMonth, int nDay); // 1년 1월 1일부터 지정 일자까지 흐른 날짜 수를 구한다.
	static	BOOL	HasLeapMonth(int nYear, int nMonth);            // 윤달이 있는 월인지 검사(음력)

	// Attributes
	int		GetYear(int nType = TYPE_SOLAR) const;
	int		GetMonth(int nType = TYPE_SOLAR) const;
	int		GetDay(int nType = TYPE_SOLAR) const;
	BOOL	SetDay(int nDay, int nType = TYPE_SOLAR);
	int		GetDayOfWeek() const;
	BOOL	IsLeapMonth() const;
	int		GetWeekCount() const;                       // 지정 월의 주의 수
	int		GetWeekNumber() const;                      // 지정 일자가 속한 주
	int		GetDaysOfMonth(int nType = TYPE_SOLAR) const;
	long	GetPassedDays() const;                      // 1년 1월 1일부터 지정 일자까지 흐른 날짜 수를 구한다.
	BOOL	IsHoliday() const;                          // 휴일 여부(일요일 포함)
	BOOL	GetDayInfo(TDayInfo* diSolar,               // 지정 일자의 일자 정보를 구한다.
		               TDayInfo* diLunar) const;
	CString GetDateString(int nType = TYPE_SOLAR,       // 날짜 문자열 구하기
		                  DWORD dwOption = 0);                            

	// Operations
	void	SetDateAsCurrent();                         // 시스템 일자로 설정
	BOOL	SetDate(int nYear, int nMonth, int nDay,    // 지정된 년월일로 설정
		            int nType = TYPE_SOLAR,
				    BOOL bIsLeapMonth = FALSE);   
	BOOL	DateAdd(int nYear, int nMonth, int nDay,    // 지정된 년월일 만큼 더하거나 뺀다.
		            int nType = TYPE_SOLAR);   

	// Operators
	const	CKoreanDate& operator=(const CKoreanDate& dateSrc);
	BOOL	operator==(const CKoreanDate& dateSrc) const;
	BOOL	operator>=(const CKoreanDate& dateSrc) const;
	BOOL	operator<=(const CKoreanDate& dateSrc) const;
	BOOL	operator>(const CKoreanDate& dateSrc) const;
	BOOL	operator<(const CKoreanDate& dateSrc) const;

#ifdef _DEBUG
	void	Dump();
#endif
	
protected:
	int		m_nYearSolar;                 // 양력 년
	int		m_nMonthSolar;                // 양력 월
	int		m_nDaySolar;                  // 양력 일
	int		m_nYearLunar;                 // 음력 년
	int		m_nMonthLunar;                // 음력 월
	int		m_nDayLunar;                  // 음력 일
	int		m_nDayOfWeek;                 // 요일(1:일요일, 2:월요일, ...)
	BOOL	m_bIsLeapMonth;               // 윤달 여부(음력일 때)

	// overridable
	virtual CString OnFormatDateString(int nType = TYPE_SOLAR,  // 출력 스트링 생성. GetDateString() 에서 호출
		                               DWORD dwOption = 0);   
};

#endif // !defined(AFX_KOREANDATE_H__EEB516D3_9793_4716_B4C6_199E3B6181B2__INCLUDED_)
