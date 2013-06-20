//////////////////////////////////////////////////////////////////////
// CKoreaDate class - �ѱ��� ���, ���� ���� Ŭ����
// 
// ��� : - SetDate(year, month, day, ��������, ���޿���)
//             ���� ���� ���.
//             ����� �����ϸ� ���� �ڵ� ���, ������ �����ϸ� ��� �ڵ� ���
//        - GetXXX
//             ������ ������ ��, ��, ��, ����, ���޿��� �� ���ϱ�
//        - GetWeekCount()
//             ������ ���ڰ� ���� ���� ���� �� ���ϱ�
//        - GetWeekNumber()
//             ������ ���ڰ� �� ° �ֿ� ���ϴ��� ���ϱ�
//        - IsDate(year, month, day, ��������, ���޿���)
//             �ٸ� ��¥ �� ���� Ȯ�� ���. static function
//        - DateAdd(year, month, day, ��������)
//		       ���� ���� ���. ��, ��, ���� ���ÿ� ���ϰų� ���� ���� �����ϴ�.
//             ���¿��� ���� ����� ���� ���޵� Ȯ���Ѵ�. ������ 1���� 12~14�����̴�.
//        - HasLeapMonth(year, month)
//             ���¿��� ������ �ִ� ������ ���� Ȯ�� ���(���¸�). static function
//        - GetDayOfWeek(year, month, day)
//             ������ ���ϴ� ���(��¸�). static function
//             ������ ������ static function�� �ƴ� ����Լ�(GetDayOfWeek)�� ����Ѵ�.
//             �̸��� ������ �ƱԸ�Ʈ�� ���� ����Լ��̴�.
//        - GetDaysOfMonth(year, month, ��������, ���޿���)
//             ���� ����� ���ڼ�(=��������) ���ϱ�. static function
//        - GetPassedDays(year, month, day)
//             1�� 1�� 1�Ϻ��� ���� ���ڱ��� ���� ���ڼ� ���ϱ�. static function
//             �� ������ ���̸� �ϼ��� ���� �� �����ϴ�.
//             ���¿��� ���� ���� static function�� �ƴ� ����Լ��� ȣ���ϸ� ���� �� �ִ�.
//        - IsHoliday()
//             �������� ���� Ȯ�� ���
//        - GetDayInfo(TDayInfo diSolar, TDayInfo diLunar)
//             ���� ���� ��ȸ ���(���, ���� ���ÿ� ���� ����)
//        - GetDateString(��������, �ɼ�)
//             ���ڰ� ���ڿ� ���ϱ�. ����ؼ� OnFormatDateString�� �������ϸ�
//             ���ϴ� ��Ÿ�Ϸ� ����� ���� �� �ִ�.
//        - ����, �� ������ ����(=, ==, >=, <=, >, <)
// 
// ������ ���� ���� :
//        - ����� 1�� 1�� 1�� ���ĺ��� ����
//        - ������ �������� 1841�� ~ 2043�����
// 
// ��� �� :
//        CKoreaDate dt1(2002, 12, 25);  // ��� 2002.12.25
// 
//        // 2002.12.25�� �ش��ϴ� ���� ���ڸ� ����� â�� ���
//        TRACE("%s", (LPCTSTR)dt1.GetDateString(TYPE_LUNAR));
// 
//        CKoreanDate dt2;         // �ý��� ���ڷ� �ʱ�ȭ
//        dt2 = dt1;               // dt2�� dt1�� ���� ��¥�� ����
//        dt1.DateAdd(3, 2, -1);   // ������� 3�� 2������ ���ϰ� 1���� ����.
//        dt2.DateAdd(3, 2, -1,    // �������� 3�� 2������ ���ϰ� 1���� ����.
//                    TYPE_LUNAR);   
//        if( dt1 == dt2 )         
//        {
//            TRACE0("���� ���Դϴ�");
//        }
//        else
//        {
//            // ��°� ������ ü�谡 �ٸ��Ƿ� ���� ����� ���ų� ���ϸ�
//            // ���� ���ڰ� �޶�����.
//            TRACE0("�ٸ� ���Դϴ�");
//        }
// 
// Copyright :
//        �� �ҽ��ڵ�(KoreanDate.h, KoreanDate.cpp)�� ���۱��� ��������
//        ����ȣ(curealls@chollian.net)�� �ֽ��ϴ�.
//        ��� �뵵�� ����ص� �����ϳ� �̷� ���� �߻��ϴ� �����Ϳ� ���ؼ���
//        �����ڿ��� � å�ӵ� ���� �� �����ϴ�.
//        �������� ������� �ҽ��ڵ�(��� ����)�� �����ϴ� ���� �ȵ˴ϴ�.
//        
// Bug report :        
//        ���׸� �߰��Ͻø� �Ʒ� e-mail�� �˷��ֽñ� �ٶ��ϴ�.
//        curealls@chollian.net
//        ����ȣ
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

// ������ �� Ư�� ����� ���� ����
typedef struct _tagDayInfo{
	int		nMonth;          // ��
	int		nDay;            // ��
	BOOL	bIsHoliday;      // ���� ����
	int		nType;           // TYPE_SOLAR:���, TYPE_LUNAR:����
	TCHAR	szDayName[20];   // ����. ��)��̳�
} TDayInfo;

class CKoreanDate  
{
private:
	static const int BASE_PASSED;         // ��� 1�� 1�� 1�Ͽ��� 1841�� 1�� 22�ϱ����� ���� ��

	static int	 m_narrSolarDays[12];     // �� ���� �⺻ ��¥ ��(���)
	static TCHAR m_szLunarTable[203][12]; // ���� ��ȯ ������
	static int	 m_narrDaysOfYear[203];   // 1841�� ���� ������ ���ڼ�(����)
	static long	 m_larrDaysAccm[203];     // 1841�� ���� ���� ���ڼ��� ����(����)
	static TDayInfo	m_arrDayInfoSolar[];  // ��� ������, �����
	static TDayInfo	m_arrDayInfoLunar[];  // ����   "

	BOOL	ConvertSolarToLunar();        // ��� �������� ���� ����
	BOOL	ConvertLunarToSolar();        // ���� �������� ��� ����
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
	static const int BASE_YEAR;                                     // ���� ���� ���� �⵵
	static const int MAX_YEAR;                                      // ���� ���� ������ �⵵

	// utilities
	static	BOOL	IsDate(int nYear, int nMonth, int nDay,         // ������ ���� ������ �˻�
		                   int nType = TYPE_SOLAR,        
					       BOOL bIsLeapMonth = FALSE);
	static	int		GetDaysOfMonth(int nYear, int nMonth,           // ���� ����� ���� ���� ���Ѵ�.
		                           int nType = TYPE_SOLAR,           
								   BOOL bIsLeapMonth = FALSE);          
	static	int		GetDayOfWeek(int nYear, int nMonth, int nDay);  // ���� ������ ������ ���Ѵ�.
	static	long	GetPassedDays(int nYear, int nMonth, int nDay); // 1�� 1�� 1�Ϻ��� ���� ���ڱ��� �帥 ��¥ ���� ���Ѵ�.
	static	BOOL	HasLeapMonth(int nYear, int nMonth);            // ������ �ִ� ������ �˻�(����)

	// Attributes
	int		GetYear(int nType = TYPE_SOLAR) const;
	int		GetMonth(int nType = TYPE_SOLAR) const;
	int		GetDay(int nType = TYPE_SOLAR) const;
	BOOL	SetDay(int nDay, int nType = TYPE_SOLAR);
	int		GetDayOfWeek() const;
	BOOL	IsLeapMonth() const;
	int		GetWeekCount() const;                       // ���� ���� ���� ��
	int		GetWeekNumber() const;                      // ���� ���ڰ� ���� ��
	int		GetDaysOfMonth(int nType = TYPE_SOLAR) const;
	long	GetPassedDays() const;                      // 1�� 1�� 1�Ϻ��� ���� ���ڱ��� �帥 ��¥ ���� ���Ѵ�.
	BOOL	IsHoliday() const;                          // ���� ����(�Ͽ��� ����)
	BOOL	GetDayInfo(TDayInfo* diSolar,               // ���� ������ ���� ������ ���Ѵ�.
		               TDayInfo* diLunar) const;
	CString GetDateString(int nType = TYPE_SOLAR,       // ��¥ ���ڿ� ���ϱ�
		                  DWORD dwOption = 0);                            

	// Operations
	void	SetDateAsCurrent();                         // �ý��� ���ڷ� ����
	BOOL	SetDate(int nYear, int nMonth, int nDay,    // ������ ����Ϸ� ����
		            int nType = TYPE_SOLAR,
				    BOOL bIsLeapMonth = FALSE);   
	BOOL	DateAdd(int nYear, int nMonth, int nDay,    // ������ ����� ��ŭ ���ϰų� ����.
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
	int		m_nYearSolar;                 // ��� ��
	int		m_nMonthSolar;                // ��� ��
	int		m_nDaySolar;                  // ��� ��
	int		m_nYearLunar;                 // ���� ��
	int		m_nMonthLunar;                // ���� ��
	int		m_nDayLunar;                  // ���� ��
	int		m_nDayOfWeek;                 // ����(1:�Ͽ���, 2:������, ...)
	BOOL	m_bIsLeapMonth;               // ���� ����(������ ��)

	// overridable
	virtual CString OnFormatDateString(int nType = TYPE_SOLAR,  // ��� ��Ʈ�� ����. GetDateString() ���� ȣ��
		                               DWORD dwOption = 0);   
};

#endif // !defined(AFX_KOREANDATE_H__EEB516D3_9793_4716_B4C6_199E3B6181B2__INCLUDED_)
