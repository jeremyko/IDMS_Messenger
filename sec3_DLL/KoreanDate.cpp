// KoreanDate.cpp: implementation of the CKoreanDate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KoreanDate.h"
#include <time.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int CKoreanDate::BASE_YEAR = 1841;      // ���� ��ȯ ���� ���� �⵵
const int CKoreanDate::MAX_YEAR  = 2043;      // ���� ��ȯ ���� ������ �⵵

// ���� 1841�� 1�� 1���� ������� 1841�� 1�� 23���̴�
// BASE_PASSED�� 1�� 1�� 1�Ϻ��� 1841�� 1�� 22�� ������ �ϼ��� �����Ѵ�.
// �׷��Ƿ� 1�� 1�� 1�Ͽ� 672068���� ���ϸ� 1841�� 1�� 23���� �ȴ�.
const int CKoreanDate::BASE_PASSED = 672068;  // ���, ���� ��ȯ �ÿ� ���


int CKoreanDate::m_narrSolarDays[]        // �� ���� �⺻ ��¥ ��
		= {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// ��� ������ �� �����
TDayInfo CKoreanDate::m_arrDayInfoSolar[] = {
	{ 1,  1, TRUE,  TYPE_SOLAR, _T("����\0")},
	{ 3,  1, TRUE,  TYPE_SOLAR, _T("3.1��\0")},
	{ 3,  3, FALSE, TYPE_SOLAR, _T("�������� ��\0")},
	{ 3, 19, FALSE, TYPE_SOLAR, _T("����� ��\0")},
	{ 3, 22, FALSE, TYPE_SOLAR, _T("���� ��\0")},
	{ 4,  5, TRUE,  TYPE_SOLAR, _T("�ĸ���\0")},
	{ 4,  7, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{ 4, 13, FALSE, TYPE_SOLAR, _T("�ӽ����μ���\0")},
	{ 4, 19, FALSE, TYPE_SOLAR, _T("4.19����\0")},
	{ 4, 20, FALSE, TYPE_SOLAR, _T("������� ��\0")},
	{ 4, 21, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{ 4, 22, FALSE, TYPE_SOLAR, _T("��������� ��\0")},
	{ 5,  1, FALSE, TYPE_SOLAR, _T("�ٷ����� ��\0")},
	{ 5,  5, TRUE,  TYPE_SOLAR, _T("��̳�\0")},
	{ 5,  8, FALSE, TYPE_SOLAR, _T("����̳�\0")},
	{ 5, 15, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{ 5, 18, FALSE, TYPE_SOLAR, _T("5.18����ȭ�\0")},
	{ 5, 19, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{ 5, 25, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{ 5, 31, FALSE, TYPE_SOLAR, _T("�ٴ��� ��\0")},
	{ 6,  5, FALSE, TYPE_SOLAR, _T("ȯ���� ��\0")},
	{ 6,  6, TRUE,  TYPE_SOLAR, _T("������\0")},
	{ 6, 25, FALSE, TYPE_SOLAR, _T("6.25�纯��\0")},
	{ 7, 17, TRUE,  TYPE_SOLAR, _T("������\0")},
	{ 8, 15, TRUE,  TYPE_SOLAR, _T("������\0")},
	{ 9, 18, FALSE, TYPE_SOLAR, _T("ö���� ��\0")},
	{10,  1, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{10,  2, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{10,  3, TRUE,  TYPE_SOLAR, _T("��õ��\0")},
	{10,  8, FALSE, TYPE_SOLAR, _T("���ⱺ���� ��\0")},
	{10,  9, FALSE, TYPE_SOLAR, _T("�ѱ۳�\0")},
	{10, 15, FALSE, TYPE_SOLAR, _T("ü���� ��\0")},
	{10, 20, FALSE, TYPE_SOLAR, _T("��ȭ�� ��\0")},
	{10, 21, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{10, 24, FALSE, TYPE_SOLAR, _T("����������\0")},
	{10, 28, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{11,  3, FALSE, TYPE_SOLAR, _T("�л��� ��\0")},
	{11,  9, FALSE, TYPE_SOLAR, _T("�ҹ��� ��\0")},
	{11, 11, FALSE, TYPE_SOLAR, _T("������� ��\0")},
	{11, 17, FALSE, TYPE_SOLAR, _T("���������� ��\0")},
	{11, 30, FALSE, TYPE_SOLAR, _T("������ ��\0")},
	{12,  3, FALSE, TYPE_SOLAR, _T("�Һ����� ��\0")},
	{12,  5, FALSE, TYPE_SOLAR, _T("���α������弱����\0")},
	{12, 10, FALSE, TYPE_SOLAR, _T("�����αǼ�������\0")},
	{12, 25, TRUE,  TYPE_SOLAR, _T("��ź��\0")},
};

// ���� ������ �� ����ǳ����
// 24����� ��¥�� ������ ���� ���� �� ����.
TDayInfo CKoreanDate::m_arrDayInfoLunar[] = {
	{ 1,  1, TRUE,  TYPE_LUNAR, _T("����\0")},
	{ 1,  2, TRUE,  TYPE_LUNAR, _T("\0")},               // ���� ������
	{ 1, 15, FALSE, TYPE_LUNAR, _T("�����뺸��\0")},
	{ 3, 27, FALSE, TYPE_LUNAR, _T("�湫��ź����\0")},
	{ 4,  8, TRUE,  TYPE_LUNAR, _T("����ź����\0")},
	{ 5,  5, FALSE, TYPE_LUNAR, _T("�ܿ�\0")},
	{ 7,  7, FALSE, TYPE_LUNAR, _T("ĥ��\0")},
	{ 8, 14, TRUE,  TYPE_LUNAR, _T("\0")},               // �߼� ����
	{ 8, 15, TRUE,  TYPE_LUNAR, _T("�߼�\0")},
	{ 8, 16, TRUE,  TYPE_LUNAR, _T("\0")},               // �߼� ������
	{12,  0, TRUE,  TYPE_LUNAR, _T("\0")},               // ���� ����. �����̹Ƿ� ��¥�� �˼� ����.
};

/*  �Ʒ��� ���� �����ʹ�
    
	1 -> 29��¥�� ���� ����
	2 -> 30��¥�� ���� ����
	3 -> 29��¥�� ���̰� �� �� 29��¥�� ������ ������ ����
	4 -> 29��¥�� ���̰� �� �� 30��¥�� ������ ������ ����
	5 -> 30��¥�� ���̰� �� �� 29��¥�� ������ ������ ����
	6 -> 30��¥�� ���̰� �� �� 30��¥�� ������ ������ ����

	�̷��� �ϸ� 1���� �����Ͱ� ��� 12���̸� �ȴ�.
	���� ��� ���� ���������� ���� �� �� �ִ�.  */		
TCHAR CKoreanDate::m_szLunarTable[203][12] = {
	/* 1841 */      1, 2, 4, 1, 1, 2,    1, 2, 1, 2, 2, 1,
					2, 2, 1, 2, 1, 1,    2, 1, 2, 1, 2, 1,
					2, 2, 2, 1, 2, 1,    4, 1, 2, 1, 2, 1,
					2, 2, 1, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 2, 1,    2, 1, 2, 1, 2, 1,
					2, 1, 2, 1, 5, 2,    1, 2, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 3, 2, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 2, 1, 2,
	/* 1851 */      2, 2, 1, 2, 1, 1,    2, 1, 2, 1, 5, 2,
					2, 1, 2, 2, 1, 1,    2, 1, 2, 1, 1, 2,
					2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 1, 2,    5, 2, 1, 2, 1, 2,
					1, 1, 2, 1, 2, 2,    1, 2, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 5, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 1, 2, 1,    1, 2, 2, 1, 2, 2,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 1, 2, 2,
					2, 1, 6, 1, 1, 2,    1, 1, 2, 1, 2, 2,
	/* 1861 */      1, 2, 2, 1, 2, 1,    2, 1, 2, 1, 1, 2,
					2, 1, 2, 1, 2, 2,    1, 2, 2, 3, 1, 2,
					1, 2, 2, 1, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 1, 2, 4, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 1, 2, 1, 1,    2, 2, 1, 2, 2, 2,
					1, 2, 1, 1, 2, 1,    1, 2, 1, 2, 2, 2,
					1, 2, 2, 3, 2, 1,    1, 2, 1, 2, 2, 1,
					2, 2, 2, 1, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 2, 2, 1, 2, 1,    2, 1, 1, 5, 2, 1,
	/* 1871 */      2, 2, 1, 2, 2, 1,    2, 1, 2, 1, 1, 2,
					1, 2, 1, 2, 2, 1,    2, 1, 2, 2, 1, 2,
					1, 1, 2, 1, 2, 4,    2, 1, 2, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 1, 2, 1, 1,    2, 1, 2, 2, 2, 1,
					2, 2, 1, 1, 5, 1,    2, 1, 2, 2, 1, 2,
					2, 2, 1, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 2, 4, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 1, 2, 2, 1, 2,    2, 1, 2, 1, 1, 2,
	/* 1881 */      1, 2, 1, 2, 1, 2,    5, 2, 2, 1, 2, 1,
					1, 2, 1, 2, 1, 2,    1, 2, 2, 1, 2, 2,
					1, 1, 2, 1, 1, 2,    1, 2, 2, 2, 1, 2,
					2, 1, 1, 2, 3, 2,    1, 2, 2, 1, 2, 2,
					2, 1, 1, 2, 1, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 2, 1, 5, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 1, 2, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 5, 2, 1, 2, 2,    1, 2, 1, 2, 1, 2,
	/* 1891 */      1, 2, 1, 2, 1, 2,    1, 2, 2, 1, 2, 2,
					1, 1, 2, 1, 1, 5,    2, 2, 1, 2, 2, 2,
					1, 1, 2, 1, 1, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 1, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 5, 1,    2, 1, 2, 1, 2, 1,
					2, 2, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					1, 2, 2, 1, 2, 1,    2, 1, 2, 1, 2, 1,
					2, 1, 5, 2, 2, 1,    2, 1, 2, 1, 2, 1,
					2, 1, 2, 1, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 5, 2, 2, 1, 2,
	/* 1901 */      1, 2, 1, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 1, 1, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 3, 2,    1, 1, 2, 2, 1, 2,
					2, 2, 1, 2, 1, 1,    2, 1, 1, 2, 2, 1,
					2, 2, 1, 2, 2, 1,    1, 2, 1, 2, 1, 2,
					1, 2, 2, 4, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 1, 2,    2, 1, 2, 1, 2, 1,
					2, 1, 1, 2, 2, 1,    2, 1, 2, 2, 1, 2,
					1, 5, 1, 2, 1, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 1, 2, 2, 2, 1,
	/* 1911 */      2, 1, 2, 1, 1, 5,    1, 2, 2, 1, 2, 2,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 2, 1, 2,
					2, 2, 1, 2, 1, 1,    2, 1, 1, 2, 1, 2,
					2, 2, 1, 2, 5, 1,    2, 1, 2, 1, 1, 2,
					2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 1, 2,    2, 1, 2, 1, 2, 1,
					2, 3, 2, 1, 2, 2,    1, 2, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    5, 2, 2, 1, 2, 2,
					1, 2, 1, 1, 2, 1,    1, 2, 2, 1, 2, 2,
	/* 1921 */      2, 1, 2, 1, 1, 2,    1, 1, 2, 1, 2, 2,
					2, 1, 2, 2, 3, 2,    1, 1, 2, 1, 2, 2,
					1, 2, 2, 1, 2, 1,    2, 1, 2, 1, 1, 2,
					2, 1, 2, 1, 2, 2,    1, 2, 1, 2, 1, 1,
					2, 1, 2, 5, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 1, 2, 2,
					1, 5, 1, 2, 1, 1,    2, 2, 1, 2, 2, 2,
					1, 2, 1, 1, 2, 1,    1, 2, 1, 2, 2, 2,
					1, 2, 2, 1, 1, 5,    1, 2, 1, 2, 2, 1, 
	/* 1931 */      2, 2, 2, 1, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 2, 2, 1, 2, 1,    2, 1, 1, 2, 1, 2,
					1, 2, 2, 1, 6, 1,    2, 1, 2, 1, 1, 2,
					1, 2, 1, 2, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 2, 1, 2, 2, 1, 
					2, 1, 4, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 1, 2, 1, 1,    2, 1, 2, 2, 2, 1,
					2, 2, 1, 1, 2, 1,    4, 1, 2, 2, 1, 2,
					2, 2, 1, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    1, 1, 2, 1, 2, 1,
	/* 1941 */      2, 2, 1, 2, 2, 4,    1, 1, 2, 1, 2, 1,
					2, 1, 2, 2, 1, 2,    2, 1, 2, 1, 1, 2,
					1, 2, 1, 2, 1, 2,    2, 1, 2, 2, 1, 2,
					1, 1, 2, 4, 1, 2,    1, 2, 2, 1, 2, 2,
					1, 1, 2, 1, 1, 2,    1, 2, 2, 2, 1, 2, 
					2, 1, 1, 2, 1, 1,    2, 1, 2, 2, 1, 2,
					2, 5, 1, 2, 1, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    3, 2, 1, 2, 1, 2,
					2, 1, 2, 2, 1, 2,    1, 1, 2, 1, 2, 1, 
	/* 1951 */      2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 4, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 1, 2, 2,    1, 2, 2, 1, 2, 2,
					1, 1, 2, 1, 1, 2,    1, 2, 2, 1, 2, 2,
					2, 1, 4, 1, 1, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 1, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 2, 1,    1, 5, 2, 1, 2, 2,
					1, 2, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					1, 2, 2, 1, 2, 1,    2, 1, 2, 1, 2, 1,
					2, 1, 2, 1, 2, 5,    2, 1, 2, 1, 2, 1, 
	/* 1961 */      2, 1, 2, 1, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 2, 3, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 1, 1, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 1, 1,    2, 1, 1, 2, 2, 1, 
					2, 2, 5, 2, 1, 1,    2, 1, 1, 2, 2, 1,
					2, 2, 1, 2, 2, 1,    1, 2, 1, 2, 1, 2,
					1, 2, 2, 1, 2, 1,    5, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 1, 2,    2, 1, 2, 1, 2, 1,
					2, 1, 1, 2, 2, 1,    2, 1, 2, 2, 1, 2,
	/* 1971 */      1, 2, 1, 1, 5, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 2, 2, 1,
					2, 2, 1, 5, 1, 2,    1, 1, 2, 2, 1, 2,
					2, 2, 1, 2, 1, 1,    2, 1, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    1, 5, 2, 1, 1, 2,
					2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 1,
					2, 2, 1, 2, 1, 2,    2, 1, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 6,    1, 2, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 1, 2, 2,
	/* 1981 */      1, 2, 1, 1, 2, 1,    1, 2, 2, 1, 2, 2,
					2, 1, 2, 3, 2, 1,    1, 2, 2, 1, 2, 2,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 1, 2, 2,
					2, 1, 2, 2, 1, 1,    2, 1, 1, 5, 2, 2,
					1, 2, 2, 1, 2, 1,    2, 1, 1, 2, 1, 2, 
					1, 2, 2, 1, 2, 2,    1, 2, 1, 2, 1, 1,
					2, 1, 2, 2, 1, 5,    2, 2, 1, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 1, 2, 1, 2,    1, 2, 2, 1, 2, 2,
					1, 2, 1, 1, 5, 1,    2, 1, 2, 2, 2, 2, 
	/* 1991 */      1, 2, 1, 1, 2, 1,    1, 2, 1, 2, 2, 2,
					1, 2, 2, 1, 1, 2,    1, 1, 2, 1, 2, 2,
					1, 2, 5, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 2, 2, 1, 2, 1,    2, 1, 1, 2, 1, 2,
					1, 2, 2, 1, 2, 2,    1, 5, 2, 1, 1, 2,
					1, 2, 1, 2, 2, 1,    2, 1, 2, 2, 1, 2,
					1, 1, 2, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 1, 2, 3, 2,    2, 1, 2, 2, 2, 1,
					2, 1, 1, 2, 1, 1,    2, 1, 2, 2, 2, 1,
					2, 2, 1, 1, 2, 1,    1, 2, 1, 2, 2, 1, 
	/* 2001 */      2, 2, 2, 3, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 2, 1, 2, 2, 1,    2, 1, 1, 2, 1, 2,
					1, 5, 2, 2, 1, 2,    1, 2, 2, 1, 1, 2,
					1, 2, 1, 2, 1, 2,    2, 1, 2, 2, 1, 2, 
					1, 1, 2, 1, 2, 1,    5, 2, 2, 1, 2, 2,
					1, 1, 2, 1, 1, 2,    1, 2, 2, 2, 1, 2,
					2, 1, 1, 2, 1, 1,    2, 1, 2, 2, 1, 2,
					2, 2, 1, 1, 5, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
	/* 2011 */      2, 1, 2, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 1, 6, 2, 1, 2,    1, 1, 2, 1, 2, 1,
					2, 1, 2, 2, 1, 2,    1, 2, 1, 2, 1, 2,
					1, 2, 1, 2, 1, 2,    1, 2, 5, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 2, 2, 1, 2, 2, 
					1, 1, 2, 1, 1, 2,    1, 2, 2, 1, 2, 2,
					2, 1, 1, 2, 3, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 1, 1,    2, 1, 2, 1, 2, 2,
					2, 1, 2, 1, 2, 1,    1, 2, 1, 2, 1, 2,
					2, 1, 2, 5, 2, 1,    1, 2, 1, 2, 1, 2, 
	/* 2021 */      1, 2, 2, 1, 2, 1,    2, 1, 2, 1, 2, 1,
					2, 1, 2, 1, 2, 2,    1, 2, 1, 2, 1, 2,
					1, 5, 2, 1, 2, 1,    2, 2, 1, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 2, 1, 2, 2, 1,
					2, 1, 2, 1, 1, 5,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 1, 1, 2,    1, 2, 1, 2, 2, 2,
					1, 2, 1, 2, 1, 1,    2, 1, 1, 2, 2, 2,
					1, 2, 2, 1, 5, 1,    2, 1, 1, 2, 2, 1,
					2, 2, 1, 2, 2, 1,    1, 2, 1, 1, 2, 2,
					1, 2, 1, 2, 2, 1,    2, 1, 2, 1, 2, 1,
	/* 2031 */      2, 1, 5, 2, 1, 2,    2, 1, 2, 1, 2, 1,
					2, 1, 1, 2, 1, 2,    2, 1, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    5, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    2, 1, 2, 2, 2, 1,
					2, 1, 2, 1, 1, 2,    1, 1, 2, 2, 1, 2,
					2, 2, 1, 2, 1, 4,    1, 1, 2, 1, 2, 2,
					2, 2, 1, 2, 1, 1,    2, 1, 1, 2, 1, 2,
					2, 2, 1, 2, 1, 2,    1, 2, 1, 1, 2, 1,
					2, 2, 1, 2, 5, 2,    1, 2, 1, 2, 1, 1,
					2, 1, 2, 2, 1, 2,    2, 1, 2, 1, 2, 1,  
	/* 2041 */      2, 1, 1, 2, 1, 2,    2, 1, 2, 2, 1, 2,
					1, 5, 1, 2, 1, 2,    1, 2, 2, 2, 1, 2,
					1, 2, 1, 1, 2, 1,    1, 2, 2, 1, 2, 2,
};  

// ������ ���� ��(����)  - LunarTable�� ������ ��
int CKoreanDate::m_narrDaysOfYear[203] = {
/* 1841 */	383, 354,  384,  355,  354,  384,  355,  354,  384,  354, 
/* 1851 */	384, 354,  355,  384,  354,  355,  384,  354,  354,  384, 
/* 1861 */	354, 384,  355,  354,  384,  355,  354,  383,  354,  384, 
/* 1871 */	355, 355,  384,  354,  354,  384,  354,  354,  384,  355, 
/* 1881 */	384, 355,  354,  384,  354,  354,  384,  354,  355,  384, 
/* 1891 */	355, 384,  354,  354,  383,  355,  354,  384,  355,  384, 
/* 1901 */	354, 355,  383,  354,  355,  384,  354,  355,  384,  354, 
/* 1911 */	384, 354,  354,  384,  355,  354,  384,  355,  384,  354, 
/* 1921 */	354, 384,  354,  354,  385,  354,  355,  384,  354,  383, 
/* 1931 */	354, 355,  384,  355,  354,  384,  354,  384,  354,  354, 
/* 1941 */	384, 355,  355,  384,  354,  354,  384,  354,  384,  354, 
/* 1951 */	355, 384,  355,  354,  384,  354,  384,  354,  354,  384, 
/* 1961 */	355, 354,  384,  355,  353,  384,  355,  384,  354,  355, 
/* 1971 */	384, 354,  354,  384,  354,  384,  354,  355,  384,  355, 
/* 1981 */	354, 384,  354,  384,  354,  354,  385,  354,  355,  384, 
/* 1991 */	354, 354,  383,  355,  384,  355,  354,  384,  354,  354, 
/* 2001 */	384, 354,  355,  384,  355,  384,  354,  354,  384,  354, 
/* 2011 */	354, 384,  355,  384,  355,  354,  384,  354,  354,  384, 
/* 2021 */	354, 355,  384,  354,  384,  355,  354,  383,  355,  354, 
/* 2031 */	384, 355,  384,  354,  354,  384,  354,  354,  384,  355, 
/* 2041 */	355, 384,  354,
};

// 1841���� �������� ������ ���� ���� �����ؼ� ����(����) - LunarTable�� ������ ��
long CKoreanDate::m_larrDaysAccm[203] = {
/* 1841 */	  383,   737,  1121,  1476,  1830,  2214,  2569,  2923,  3307,  3661,
/* 1851 */	 4045,  4399,  4754,  5138,  5492,  5847,  6231,  6585,  6939,  7323,
/* 1861 */	 7677,  8061,  8416,  8770,  9154,  9509,  9863, 10246, 10600, 10984,
/* 1871 */	11339, 11694, 12078, 12432, 12786, 13170, 13524, 13878, 14262, 14617,
/* 1881 */	15001, 15356, 15710, 16094, 16448, 16802, 17186, 17540, 17895, 18279,
/* 1891 */	18634, 19018, 19372, 19726, 20109, 20464, 20818, 21202, 21557, 21941,
/* 1901 */	22295, 22650, 23033, 23387, 23742, 24126, 24480, 24835, 25219, 25573,
/* 1911 */	25957, 26311, 26665, 27049, 27404, 27758, 28142, 28497, 28881, 29235,
/* 1921 */	29589, 29973, 30327, 30681, 31066, 31420, 31775, 32159, 32513, 32896,
/* 1931 */	33250, 33605, 33989, 34344, 34698, 35082, 35436, 35820, 36174, 36528,
/* 1941 */	36912, 37267, 37622, 38006, 38360, 38714, 39098, 39452, 39836, 40190,
/* 1951 */	40545, 40929, 41284, 41638, 42022, 42376, 42760, 43114, 43468, 43852,
/* 1961 */	44207, 44561, 44945, 45300, 45653, 46037, 46392, 46776, 47130, 47485,
/* 1971 */	47869, 48223, 48577, 48961, 49315, 49699, 50053, 50408, 50792, 51147,
/* 1981 */	51501, 51885, 52239, 52623, 52977, 53331, 53716, 54070, 54425, 54809,
/* 1991 */	55163, 55517, 55900, 56255, 56639, 56994, 57348, 57732, 58086, 58440,
/* 2001 */	58824, 59178, 59533, 59917, 60272, 60656, 61010, 61364, 61748, 62102,
/* 2011 */	62456, 62840, 63195, 63579, 63934, 64288, 64672, 65026, 65380, 65764,
/* 2021 */	66118, 66473, 66857, 67211, 67595, 67950, 68304, 68687, 69042, 69396,
/* 2031 */	69780, 70135, 70519, 70873, 71227, 71611, 71965, 72319, 72703, 73058,
/* 2041 */	73413, 73797, 74151,
};


CKoreanDate::CKoreanDate()
{
	SetDateAsCurrent();
}

// ������ ���ڰ� �ùٸ� ���ڰ��� �ƴϸ� �ý��� ���ڷ� �����Ѵ�.
CKoreanDate::CKoreanDate(int nYear, int nMonth, int nDay, int nType, BOOL bIsLeapMonth)
{
	if( !SetDate(nYear, nMonth, nDay, nType, bIsLeapMonth) )
	{
		SetDateAsCurrent();
	}
}

CKoreanDate::~CKoreanDate()
{

}

#ifdef _DEBUG
// ����� ��忡�� ���� Ȯ��
void CKoreanDate::Dump()
{
	TCHAR* szWeekdayName[] = {
		_T("��"), _T("��"), _T("ȭ"), _T("��"), _T("��"), _T("��"), _T("��")
	};

	TRACE(_T("��� %4d�� %02d�� %02d�� %s���� ")  , m_nYearSolar
		                                     , m_nMonthSolar
						        		     , m_nDaySolar
								        	 , szWeekdayName[m_nDayOfWeek-1]);
	TRACE(_T("(%d / %d)��\r\n"), GetWeekNumber(), GetWeekCount());

	TRACE(_T("���� %4d�� %02d�� %02d�� ")  , m_nYearLunar
		                                 , m_nMonthLunar
						        		 , m_nDayLunar);
	if( m_bIsLeapMonth )
		TRACE(_T("����\r\n"));
	else
		TRACE(_T("���\r\n"));

}
#endif

// static public function
// �ش� ������� ���ڰ����� �˻�
BOOL CKoreanDate::IsDate(int nYear, int nMonth, int nDay, int nType, BOOL bIsLeapMonth)
{
	int nDaysOfMonth;

	if(nYear <= 0) return FALSE;
	if(nMonth < 1 || nMonth > 12) return FALSE;
	if(nDay < 1 || nDay > 31) return FALSE;

	nDaysOfMonth = GetDaysOfMonth(nYear, nMonth, nType, bIsLeapMonth);
	if( nDaysOfMonth == 0 ) return FALSE;
	return ( nDay <= nDaysOfMonth );
}

// static public function
// �ش� ����� ��¥ ���� ���Ѵ�.
int CKoreanDate::GetDaysOfMonth(int nYear, int nMonth, int nType, BOOL bIsLeapMonth)
{
	int nDaysOfMonth = 0;
	int nLunarData;

	if( nMonth < 1 || nMonth > 12 ) return 0;

	if( nType == TYPE_SOLAR )
	{
		//if( nYear < 1 ) return 0;

		nDaysOfMonth = m_narrSolarDays[nMonth-1];        // �ش���� �⺻ ��¥ ��
		if( nMonth == 2 )  // 2���̸� �������� �˻� �ʿ�
		{
			if( !(nYear%4) && nYear%100 || !(nYear%400) )  // �����̸�
				nDaysOfMonth++;
		}
	}
	else
	{
		if( nYear < BASE_YEAR || nYear > MAX_YEAR ) return 0;
		
		nLunarData = m_szLunarTable[nYear-BASE_YEAR][nMonth-1];
		if( bIsLeapMonth && nLunarData <= 2 ) return 0;  //������ ���µ� ������ ã����

		if( !bIsLeapMonth )  // ������ �ƴ� ��� ���� �˻�
		{
			switch( nLunarData )
			{
			case 1:
			case 3:
			case 4:
				nDaysOfMonth = 29;
				break;
			default:
				nDaysOfMonth = 30;
				break;
			}
		}
		else                 // ������ ��� ���� �˻�
		{
			switch( nLunarData )
			{
			case 3:
			case 5:
				nDaysOfMonth = 29;
				break;
			case 4:
			case 6:
				nDaysOfMonth = 30;
				break;
			}
		}  // end of if( !bIsLeapMonth )
	}

	return nDaysOfMonth;
}

int CKoreanDate::GetDaysOfMonth(int nType) const
{
	if( nType == TYPE_SOLAR )
		return GetDaysOfMonth(m_nYearSolar, m_nMonthSolar);
	else
		return GetDaysOfMonth(m_nYearLunar, m_nMonthLunar, TYPE_LUNAR, m_bIsLeapMonth);
}

// static public function
// 1�� 1�� 1�Ϻ��� ���� ���ڱ��� �帥 ��¥ ���� ���Ѵ�.
// ��Ȯ�� ���� ���� �������� ������ -1�� �����Ѵ�.
long CKoreanDate::GetPassedDays(int nYear, int nMonth, int nDay)
{
	long	lTotal;   // 1�� 1�� 1�Ϻ��� ���� ���ڱ��� �帥 �� ��¥��
	int		i;
	
	if( !IsDate(nYear, nMonth, nDay) ) return -1;

	lTotal = (nYear - 1) * 365              // X�� * 365��
		   + (long)( (nYear - 1) / 4 )      // 4�⸶�� 1�� �߰�
		   - (long)( (nYear - 1) / 100 )    // 100�� ���� 1�� ����
		   + (long)( (nYear - 1) / 400 );   // 400�⸶�� 1�� �߰�
	for(i = 1; i < nMonth; i++)
	{
		lTotal += GetDaysOfMonth(nYear, i);
	}
	lTotal += nDay;

	return lTotal;
}

// static public function
// �ش� ����� ������ ���Ѵ�.
// ��Ȯ�� ���ڰ��� �������� ������ 0�� �����Ѵ�.
int CKoreanDate::GetDayOfWeek(int nYear, int nMonth, int nDay)
{
	return (int)(GetPassedDays(nYear, nMonth, nDay) % 7) + 1; // 1 base
}

// static public function
// ������ �ִ� ������ �˻�(����)
BOOL CKoreanDate::HasLeapMonth(int nYear, int nMonth)
{
	if( !IsDate(nYear, nMonth, 1, TYPE_LUNAR, FALSE) ) return FALSE;
	return ( m_szLunarTable[nYear-BASE_YEAR][nMonth-1] > 2 );
}

// private function
// ��� ���ڸ� �������� ���� ���ڸ� ���Ѵ�.
BOOL CKoreanDate::ConvertSolarToLunar()
{
	long lTotal;               
	int nYear, nMonth, nDay;   // ���� ���� ����(���� ���� ���� ���̽� ���� ������� ��. ���� ����)
	BOOL bIsLeapMonth;         // ���� ���� ���� ���ڰ� �������� ����
	BOOL bNewMonth;            // ������ ��� ���� ���� ��Ű�� �ʰ� bNewMonth = FALSE�� ����
	int nLunarData;            // m_szLunarTable�� ��

	m_nYearLunar = 0;
	m_nMonthLunar = 0;
	m_nDayLunar = 0;
	m_bIsLeapMonth = FALSE;

	if( m_nYearSolar < BASE_YEAR || m_nYearSolar > MAX_YEAR )
		return FALSE;

	lTotal = GetPassedDays(m_nYearSolar, m_nMonthSolar, m_nDaySolar) - BASE_PASSED;

	// 2043����� �Ʒ��� �⵵ ����
	// for ���� ��� �� nYear + BASE_YEAR �� ���� �⵵�̴�.
	for(nYear = MAX_YEAR - BASE_YEAR + 1; nYear > 0 ; nYear--) 
	{
		if( lTotal > m_larrDaysAccm[nYear-1] )
		{
			lTotal -= m_larrDaysAccm[nYear-1];  // �⵵�� ���� �����Ǵ� �ϼ� ���ֱ�
			break;
		}
	}

	// ��, ��, ���� ���� ����
	nMonth = 0;
	bNewMonth = TRUE;
	while( true )
	{
		nLunarData = m_szLunarTable[nYear][nMonth];
		nDay = GetDaysOfMonth(nYear+BASE_YEAR, nMonth+1, TYPE_LUNAR, !bNewMonth);
		bIsLeapMonth = ( !bNewMonth && nLunarData > 2 ) ? TRUE : FALSE;

		if( lTotal <= nDay )
		{
			nDay = lTotal;
			break;
		}
		else
		{
			lTotal -= nDay;
		}

		if( nLunarData > 2 && bNewMonth )  // �����̸�
		{
			bNewMonth = FALSE;
		}
		else
		{
			nMonth++;
			bNewMonth = TRUE;
		}
	} // end of while

	m_nYearLunar = nYear + BASE_YEAR;
	m_nMonthLunar = nMonth + 1;
	m_nDayLunar = nDay;
	m_bIsLeapMonth = bIsLeapMonth;

	return TRUE;
}

// private function
// ���� ���ڸ� �������� ��� ���ڸ� ���Ѵ�.
BOOL CKoreanDate::ConvertLunarToSolar()
{
	long lTotal, lTemp;               
	int nYear, nMonth, nDay;
	int nLunarData;            // m_szLunarTable�� ��

	nYear = m_nYearLunar-BASE_YEAR;

	// �⵵���� �ϼ� ���ϱ�
	lTotal = BASE_PASSED;
	lTotal += (nYear == 0) ? 0 : m_larrDaysAccm[nYear-1];
	
	// (m_nMonthLunar - 1) �������� �ϼ� ���ϱ�
	for(nMonth = 0; nMonth < (m_nMonthLunar - 1); nMonth++)
	{
		nLunarData = m_szLunarTable[nYear][nMonth];
		switch(nLunarData)  // 3 �̻��� ������ �ִ� ���̹Ƿ� ���+����
		{
		case 1:
			lTotal += 29;
			break;
		case 2:
			lTotal += 30;
			break;
		case 3:
			lTotal += 58;
			break;
		case 4:
		case 5:
			lTotal += 59;
			break;
		case 6:
			lTotal += 60;
			break;
		}
	}

	// ���� ���� �����̸� ���� ������ �����ش�
	if( m_bIsLeapMonth )
	{
		switch( m_szLunarTable[nYear][m_nMonthLunar - 1] )
		{
		case 3:
		case 4:
			lTotal += 29;
			break;
		case 5:
		case 6:
			lTotal += 30;
			break;
		}
	}

	// �ϼ� ���ϱ�
	// m_nDayLunar�� ���ϸ� lTotal�� ��� 1�� 1�� 1�� ���� ������� �ϼ��� ������
	lTotal += m_nDayLunar;
	                        
	// �ϼ��� �������� ��� ���ڸ� ���Ѵ�.
	nYear = (int)(lTotal / 365) + 1;
	while( true )
	{
		lTemp = GetPassedDays(nYear, 12, 31);
		
		if( lTotal == lTemp )
		{
			lTotal -= GetPassedDays(nYear-1, 12, 31);
			break;
		}
		else if( lTotal > lTemp )
		{
			nYear++;
			// �����̸� nYear�ش� 366��, �׷��� ������ 365���̴�
			nDay = ( !(nYear%4) && nYear%100 || !(nYear%400) ? 366 : 365 );

			if( (lTotal - lTemp) <= nDay )
			{
				lTotal -= GetPassedDays(nYear-1, 12, 31);
				break;
			}
		}
		else
		{
			lTemp = GetPassedDays(nYear, 1, 1);
			if( lTemp <= lTotal )
			{
				lTotal -= GetPassedDays(nYear-1, 12, 31);
				break;
			}
			else
			{
				nYear--;
			}
		}  // end of if
	}  // end of while( true )

	// ��, �� ����
	for(nMonth = 1; nMonth <= 12; nMonth++)
	{
		nDay = GetDaysOfMonth(nYear, nMonth);
		if( lTotal <= nDay )
		{
			nDay = lTotal;
			break;
		}
		else
		{
			lTotal -= nDay;
		}
	}  // end of for

	if( IsDate(nYear, nMonth, nDay) )
	{
		m_nYearSolar = nYear;
		m_nMonthSolar = nMonth;
		m_nDaySolar = nDay;
		return TRUE;
	}

	return FALSE;
}

// public function
int CKoreanDate::GetYear(int nType) const
{
	return (nType == TYPE_SOLAR ? m_nYearSolar : m_nYearLunar);
}

// public function
int CKoreanDate::GetMonth(int nType) const
{
	return (nType == TYPE_SOLAR ? m_nMonthSolar : m_nMonthLunar);
}

// public function
int CKoreanDate::GetDay(int nType) const
{
	return (nType == TYPE_SOLAR ? m_nDaySolar : m_nDayLunar);
}

// public function
BOOL CKoreanDate::SetDay(int nDay, int nType)
{
	if( nType == TYPE_SOLAR )
	{
		if( IsDate(m_nYearSolar, m_nMonthSolar, nDay) )
		{
			SetDate(m_nYearSolar, m_nMonthSolar, nDay);
			return TRUE;
		}
	}
	else
	{
		if( IsDate(m_nYearLunar, m_nMonthLunar, nDay, TYPE_LUNAR, m_bIsLeapMonth) )
		{
			SetDate(m_nYearLunar, m_nMonthLunar, nDay, TYPE_LUNAR, m_bIsLeapMonth);
			return TRUE;
		}
	}

	return FALSE;
}

// public function
// ����(1:�Ͽ���, 2:������, ...)
int CKoreanDate::GetDayOfWeek() const
{
	return m_nDayOfWeek;
}

// public function
// ���� ����(����)
BOOL CKoreanDate::IsLeapMonth() const
{
	return m_bIsLeapMonth;
}

// public function
// �ش� ���� �� ���� ���� ���Ѵ�.
int CKoreanDate::GetWeekCount() const
{
	int nDays;

	nDays = GetDaysOfMonth(m_nYearSolar, m_nMonthSolar) - 1;
	nDays += ( GetDayOfWeek(m_nYearSolar, m_nMonthSolar, 1) - 1 );
	return ( (int)(nDays / 7) + 1 );
}

// public function
// �ش� ���� �� ��° �ֿ� ���ϴ��� ���Ѵ�.
int CKoreanDate::GetWeekNumber() const
{
	int nDays;

	nDays = m_nDaySolar + GetDayOfWeek(m_nYearSolar, m_nMonthSolar, 1) - 2;
	return ( (int)(nDays / 7) + 1 );
}

// public function
// 1�� 1�� 1�Ϻ��� ���� ���ڱ��� �帥 ��¥ ���� ���Ѵ�.
// ��Ȯ�� ���� ���� �������� ������ -1�� �����Ѵ�.
long CKoreanDate::GetPassedDays() const
{
	return GetPassedDays(m_nYearSolar, m_nMonthSolar, m_nDaySolar);
}

// public function
// �ý��� ���ڷ� ����
void CKoreanDate::SetDateAsCurrent()
{
	struct tm atm;
	time_t now;
	
	::time(&now);
	atm = *(::localtime(&now));

	m_nYearSolar = atm.tm_year + 1900;  // 1900 base
	m_nMonthSolar = atm.tm_mon + 1;     // 0 base
	m_nDaySolar = atm.tm_mday;
	m_nDayOfWeek = GetDayOfWeek(m_nYearSolar, m_nMonthSolar, m_nDaySolar);

	ConvertSolarToLunar();
}

// public function
// �Ѱ��� ���ڷ� ����
BOOL CKoreanDate::SetDate(int nYear, int nMonth, int nDay, int nType, BOOL bIsLeapMonth)
{
	if( nYear <= 1 ) return FALSE;
	if( !IsDate(nYear, nMonth, nDay, nType, bIsLeapMonth) ) return FALSE;

	if( nType == TYPE_SOLAR )
	{
		m_nYearSolar = nYear;
		m_nMonthSolar = nMonth;
		m_nDaySolar = nDay;
		m_nDayOfWeek = GetDayOfWeek(m_nYearSolar, m_nMonthSolar, m_nDaySolar);

		ConvertSolarToLunar();
	}
	else
	{
		m_nYearLunar = nYear;
		m_nMonthLunar = nMonth;
		m_nDayLunar = nDay;
		m_bIsLeapMonth = bIsLeapMonth;

		if( ConvertLunarToSolar() )
		{
			m_nDayOfWeek = GetDayOfWeek(m_nYearSolar, m_nMonthSolar, m_nDaySolar);
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

// private function
// �Ѱ��� ����ŭ ��¥�� ���ϰų� ����. �� ���� ���� �� �Ѱ���
// �⵵ ���� ����ϰ� ��, �� ������ ����Ѵ�.
// �� ���� 12�� ���� �������� ���ϰ� ���� �⵵�� ���Ѵ�.
// ���� �� ������ ȯ������ �ʴ´�.
// �� �Ǵ� ���� ���ϰų� �� �� ���ڰ� ������ �� ���� ���Ϸ� �ȴ�.
// ��) 2000.3.31 - 1�� = 2000.2.29  (2002�� 2���� �����̴�.)
// �׷��Ƿ� ��¥ ���� ��Ȯ�� �Ͽ� ����ϰ��� �� ���� ����� 0���� �ϰ� ���ڸ� �Ѱ��ش�.
// ��) ���� ���ڿ��� 50���� ���� ��       : DateAdd(0, 0, 50);
//                   1�� 2������ ���� ��  : DateAdd(1, 2, 0);
//                   3�� ���ϰ� 2�� �� �� : DateAdd(0, 3, -2);
BOOL CKoreanDate::DateAddSolar(int nYear, int nMonth, int nDay)
{
	int nTempYear, nTempMonth, nTempDay;
	long lTotal, lTemp;

	nTempYear = m_nYearSolar;
	nTempMonth = m_nMonthSolar;
	nTempDay = m_nDaySolar;

	nYear += nMonth / 12;
	nMonth %= 12;

	if( nYear != 0 )
	{
		nTempYear += nYear;
		if( nTempYear < 1 )	return FALSE;
	}

	if( nMonth != 0 )
	{
		nTempMonth += nMonth;
		if( nTempMonth > 12 )
		{
			nTempYear++;
			nTempMonth -= 12;
		}
		else if( nTempMonth <= 0 )
		{
			nTempYear--;
			nTempMonth += 12;
			if( nTempYear < 1 )	return FALSE;
		}
	}

	// ����� ����Ǿ� ��¥�� ��Ȯ���� ���� �� ���Ϸ� ����
	if( (nYear != 0 || nMonth != 0) && !IsDate(nTempYear, nTempMonth, nTempDay) )
	{
		nTempDay = GetDaysOfMonth(nTempYear, nTempMonth);
	}

	if( nDay != 0 )
	{
		lTotal = GetPassedDays(nTempYear, nTempMonth, nTempDay) + nDay;

		// �⵵ ����
		// ��Ȯ�� ����� �� �����Ƿ� �ϴ� 1���� 365�Ϸ� �����ϰ� �ٻ� �⵵�� ���Ѵ�.
		// �ٻ� �⵵�� 12�� 31�ϱ��� �ϼ��� ����� �ϼ�(lTotal)�� ���̰�����
		// ��Ȯ�� �⵵�� ���Ѵ�.
		// �⵵�� ���� �Ŀ� lTotal ���� (�⵵ - 1)�� 12�� 31�� ���� �ϼ��� �� ����
		// �־��־� ��, ���� ���ϴµ� ����Ѵ�.
		nTempYear = (int)(lTotal / 365) + 1;
		while( true )
		{
			lTemp = GetPassedDays(nTempYear, 12, 31);
			
			if( lTotal == lTemp )
			{
				lTotal -= GetPassedDays(nTempYear-1, 12, 31);
				break;
			}
			else if( lTotal > lTemp )
			{
				nTempYear++;
				// �����̸� nTempYear�ش� 366��, �׷��� ������ 365���̴�
				nTempDay = ( !(nTempYear%4) && nTempYear%100 || !(nTempYear%400) ? 366 : 365 );

				if( (lTotal - lTemp) <= nTempDay )
				{
					lTotal -= GetPassedDays(nTempYear-1, 12, 31);
					break;
				}
			}
			else
			{
				lTemp = GetPassedDays(nTempYear, 1, 1);
				if( lTemp <= lTotal )
				{
					lTotal -= GetPassedDays(nTempYear-1, 12, 31);
					break;
				}
				else
				{
					nTempYear--;
				}
			}  // end of if
		}  // end of while( true )

		// ��, �� ����
		for(nTempMonth = 1; nTempMonth <= 12; nTempMonth++)
		{
			nTempDay = GetDaysOfMonth(nTempYear, nTempMonth);
			if( lTotal <= nTempDay )
			{
				nTempDay = lTotal;
				break;
			}
			else
			{
				lTotal -= nTempDay;
			}
		}  // end of for
	}  // end of if( nDay != 0 )

	return SetDate(nTempYear, nTempMonth, nTempDay);
}

// private function
// ���� �������� ���ڸ� ���ϰų� ����
// ���¿��� 1���� �ݵ�� 12������ �ƴϴ� (12, 13, 14 ����)
// �׷��Ƿ� �⵵���� ���� ����ϴ� ���� �Ұ����ϴ�.
BOOL CKoreanDate::DateAddLunar(int nYear, int nMonth, int nDay)
{
	int nTempYear, nTempMonth, nTempDay;
	BOOL bIsLeapMonth;
	CKoreanDate ld(m_nYearLunar, m_nMonthLunar, m_nDayLunar, TYPE_LUNAR, m_bIsLeapMonth);

	nTempYear = m_nYearLunar + nYear;
	nTempMonth = m_nMonthLunar;
	nTempDay = m_nDayLunar;

	//--- �⵵ ���� --------------------------------------------------------------
	// �ܼ��� ���� ���ؼ� ������ �����ϸ� �⵵ ���� �Ϸ�ȴ�.(������ ��� ���޷�)
	// �׷��� ������(���� �̾��µ� �⵵�� �ٲ�� ������ ���ų�
	//               30���̾��µ� �⵵�� �ٲ�� 29���� ������ ���)
	// ������ �����غ��� �׷��� �ٸ� ���ڰ� �ƴϸ� ��-1 �� �Ѵ�.
	// ���� 30���� 29�� �ٲ� 30�� nTempDay�� �����ؼ� �� ���濡�� �ٽ� �����ϹǷ� �������.
	if( !IsDate(nTempYear, nTempMonth, nTempDay, TYPE_LUNAR, m_bIsLeapMonth) )
	{
		if( !IsDate(nTempYear, nTempMonth, nTempDay, TYPE_LUNAR, FALSE) )
		{
			if( !IsDate(nTempYear, nTempMonth, nTempDay-1, TYPE_LUNAR, FALSE) )
				return FALSE;
			if( !ld.SetDate(nTempYear, nTempMonth, nTempDay-1, TYPE_LUNAR, FALSE) )
				return FALSE;
		}
		else
		{
			ld.SetDate(nTempYear, nTempMonth, nTempDay, TYPE_LUNAR, FALSE);
		}
	}
	else
	{
		ld.SetDate(nTempYear, nTempMonth, nTempDay, TYPE_LUNAR, m_bIsLeapMonth);
	}
	//----------------------------------------------------------------------------

	//--- �� ���� ----------------------------------------------------------------
	// 1�� ���ϱ� : ������� ���� �����̸� ���� �÷��׸� TRUE�� �����Ѵ�.
	//                       �����̰ų� ����̸� 1�� ���Ѵ�. 13���̸� 1�� �����ϰ� 1���� �����Ѵ�.
	// 1�� ����   : ������� �����̸� ���� �÷��׸� FALSE�� �����Ѵ�.
	//                       ���� �����̰ų� ����̸� 1�� ���� 0���̸� 1�� �����ϰ� 12���� �����Ѵ�.
	// ���ϰų� ���� �� �� ��ŭ ���� �� ���� ���� ����Ϸ� SetDate�Ѵ�.
	if( nMonth != 0 )
	{
		bIsLeapMonth = ld.IsLeapMonth();

		while( nMonth != 0 )
		{
			if( nMonth > 0 )
			{
				if( HasLeapMonth(nTempYear, nTempMonth) && !bIsLeapMonth )  // ���� �����̸�
				{
					bIsLeapMonth = TRUE;
				}
				else
				{
					nTempMonth++;
					if( nTempMonth > 12 )
					{
						nTempYear++;
						nTempMonth = 1;
					}

					bIsLeapMonth = FALSE;
				}
				nMonth--;
			}
			else
			{
				if( HasLeapMonth(nTempYear, nTempMonth) && bIsLeapMonth )   // �����̸�
				{
					bIsLeapMonth = FALSE;
				}
				else
				{
					nTempMonth--;
					if( nTempMonth < 1 )
					{
						nTempYear--;
						nTempMonth = 12;
					}

					bIsLeapMonth = HasLeapMonth(nTempYear, nTempMonth);
				}
				nMonth++;
			}
		} // end of while

		if( !ld.SetDate(nTempYear, nTempMonth, nTempDay, TYPE_LUNAR, bIsLeapMonth) )
		{
			if( !ld.SetDate(nTempYear, nTempMonth, nTempDay-1, TYPE_LUNAR, bIsLeapMonth) )
				return FALSE;
		}
	} // end of if
	//----------------------------------------------------------------------------

	ld.DateAdd(0, 0, nDay);   // �� ���� - ������� ����

	return SetDate(ld.GetYear(), ld.GetMonth(), ld.GetDay());
}

// public function
BOOL CKoreanDate::DateAdd(int nYear, int nMonth, int nDay, int nType)
{
	if( nType == TYPE_SOLAR )
		return DateAddSolar(nYear, nMonth, nDay);  // ���
	else
		return DateAddLunar(nYear, nMonth, nDay);  // ����
}

// virtual protected function
// ��� ������ ���� ���� ������ ��� �޾Ƽ�
// ��� ������ ���ϴ� ��Ÿ�Ϸ� ������ �ϸ� �ȴ�.
// dwOption�� ������ �ÿ� ���� �뵵�� ����� �� �ִ�.
CString CKoreanDate::OnFormatDateString(int nType, DWORD dwOption)
{
	CString sDate;

	TCHAR* szWeekdayName[] = {
		_T("��"), _T("��"), _T("ȭ"), _T("��"), _T("��"), _T("��"), _T("��")
	};

	if( nType == TYPE_SOLAR )
	{
		sDate.Format(_T("%d�� %02d�� %02d�� %s����") , m_nYearSolar
										, m_nMonthSolar
						           		, m_nDaySolar
								        , szWeekdayName[m_nDayOfWeek-1]);
	}
	else
	{
		/*
		sDate.Format(_T("%d�� %02d�� %02d�� %s����")  , m_nYearLunar
										, m_nMonthLunar
						           		, m_nDayLunar
								        , szWeekdayName[m_nDayOfWeek-1]);
		*/
		sDate.Format(_T("%d�� %02d�� %02d��")  , m_nYearLunar
										, m_nMonthLunar
						           		, m_nDayLunar);
		if( m_bIsLeapMonth )
			sDate += (_T("(����)"));
	}

	return CString((LPCTSTR)sDate);
}

// ���� ����. �Ͽ��� ����
BOOL CKoreanDate::IsHoliday() const
{
	int i;
	int bIsLeapMonth;

	if( m_nDayOfWeek == 1 ) return TRUE;   // �Ͽ����̸�

	//--- ��� ���� �˻� ------------------------------------------
	for(i = 0; i < sizeof(m_arrDayInfoSolar)/sizeof(TDayInfo); i++)
	{
		if( m_arrDayInfoSolar[i].nMonth > m_nMonthSolar ) break;

		if( m_arrDayInfoSolar[i].nMonth == m_nMonthSolar &&
			m_arrDayInfoSolar[i].nDay   == m_nDaySolar
		)
		{
			if( m_arrDayInfoSolar[i].bIsHoliday )
				return TRUE;
			else
				break;
		}
	}
	//-------------------------------------------------------------

	//--- ��� ���� �˻� ------------------------------------------
	if( m_nMonthLunar == 12 )  // ���� ���� �˻�. ���� 12���� ������ ������ Ȯ��
	{
		bIsLeapMonth = HasLeapMonth(m_nYearLunar, m_nMonthLunar);
		if( m_bIsLeapMonth == bIsLeapMonth  && 
			m_nDayLunar == GetDaysOfMonth(m_nYearLunar, m_nMonthLunar, TYPE_LUNAR, bIsLeapMonth) )
			return TRUE;
	}

	if( !m_bIsLeapMonth )  // ������ �ƴ� ��츸
	{
		for(i = 0; i < sizeof(m_arrDayInfoLunar)/sizeof(TDayInfo); i++)
		{
			if( m_arrDayInfoLunar[i].nMonth > m_nMonthLunar ) break;

			if( m_arrDayInfoLunar[i].nMonth == m_nMonthLunar &&
				m_arrDayInfoLunar[i].nDay   == m_nDayLunar
			)
			{
				if( m_arrDayInfoLunar[i].bIsHoliday )
					return TRUE;
				else
					break;
			}
		}  // end of for
	}  // end of if( !m_bIsLeapMonth )
	//-------------------------------------------------------------

	return FALSE;
}

// public function
// diSolar�� ��� ���� ������, diLunar�� ���� ���� ������ �־��ش�.
// Return value:
//    ��� ���� ������ ������ TYPE_SOLAR
//    ���� ���� ������ ������ TYPE_LUNAR
//    �Ѵ� ������             TYPE_SOLAR | TYPE_LUNAR
//    �Ѵ� ������             0
//
// Example:
//  CKoreanDate dt(2003, 5, 8);
//	TDayInfo diSolar, diLunar;
//	int nRet = dt.GetDayInfo(&diSolar, &diLunar);
//	if( nRet & TYPE_SOLAR )    // ��� ���� ������ ������
//		TRACE("������ ������� %s�Դϴ�.\r\n", diSolar.szDayName);
//	if( nRet & TYPE_LUNAR )    // ���� ���� ������ ������
//		TRACE("������ �������� %s�Դϴ�.\r\n", diLunar.szDayName);
int CKoreanDate::GetDayInfo(TDayInfo *diSolar, TDayInfo *diLunar) const
{
	int i;
	int bIsLeapMonth;
	int nReturn = 0;

	if( diSolar == NULL || diLunar == NULL ) return 0;

	memset(diSolar, 0, sizeof(TDayInfo));
	memset(diLunar, 0, sizeof(TDayInfo));
	diSolar->nType = TYPE_SOLAR;
	diLunar->nType = TYPE_LUNAR;

	//--- ��� ���� �˻� ------------------------------------------
	for(i = 0; i < sizeof(m_arrDayInfoSolar)/sizeof(TDayInfo); i++)
	{
		if( m_arrDayInfoSolar[i].nMonth > m_nMonthSolar ) break;

		if( m_arrDayInfoSolar[i].nMonth == m_nMonthSolar &&
			m_arrDayInfoSolar[i].nDay   == m_nDaySolar
		)
		{
			memcpy(diSolar, &m_arrDayInfoSolar[i], sizeof(TDayInfo));
			nReturn |= TYPE_SOLAR;
			break;
		}
	}
	//-------------------------------------------------------------

	//--- ���� ���� �˻� ------------------------------------------
	if( m_nMonthLunar == 12 )  // ���� ���� �˻�. ���� 12���� ������ ������ Ȯ��
	{
		bIsLeapMonth = HasLeapMonth(m_nYearLunar, m_nMonthLunar);
		if( m_bIsLeapMonth == bIsLeapMonth  && 
			m_nDayLunar == GetDaysOfMonth(m_nYearLunar, m_nMonthLunar, TYPE_LUNAR, bIsLeapMonth) )
		{
			diLunar->nMonth = m_nMonthLunar;
			diLunar->nDay = m_nDayLunar;
			diLunar->bIsHoliday = TRUE;
			diLunar->nType = TYPE_LUNAR;

			nReturn |= TYPE_LUNAR;
			return nReturn;
		}
	}

	if( !m_bIsLeapMonth )
	{
		for(i = 0; i < sizeof(m_arrDayInfoLunar)/sizeof(TDayInfo); i++)
		{
			if( m_arrDayInfoLunar[i].nMonth > m_nMonthLunar ) break;

			if( m_arrDayInfoLunar[i].nMonth == m_nMonthLunar &&
				m_arrDayInfoLunar[i].nDay   == m_nDayLunar
			)
			{
				memcpy(diLunar, &m_arrDayInfoLunar[i], sizeof(TDayInfo));
				nReturn |= TYPE_LUNAR;
				break;
			}
		} // end of for
	}
	//-------------------------------------------------------------

	return nReturn;
}

// public function
// ��¥ ���ڿ� ���ϱ�
CString CKoreanDate::GetDateString(int nType, DWORD dwOption)
{
	return OnFormatDateString(nType, dwOption);
}

// public function
// = ������ ����
const CKoreanDate& CKoreanDate::operator=(const CKoreanDate& dateSrc)
{
	m_nYearSolar = dateSrc.m_nYearSolar;
	m_nMonthSolar = dateSrc.m_nMonthSolar;
	m_nDaySolar = dateSrc.m_nDaySolar;
	m_nYearLunar = dateSrc.m_nYearLunar;
	m_nMonthLunar = dateSrc.m_nMonthLunar;
	m_nDayLunar = dateSrc.m_nDayLunar;
	m_bIsLeapMonth = dateSrc.m_bIsLeapMonth;
	m_nDayOfWeek = dateSrc.m_nDayOfWeek;

	return *this;
}

// public function
// == ������ ����
BOOL CKoreanDate::operator==(const CKoreanDate& dateSrc) const
{
	return ( m_nYearSolar == dateSrc.m_nYearSolar &&
	         m_nMonthSolar == dateSrc.m_nMonthSolar &&
	         m_nDaySolar == dateSrc.m_nDaySolar);
}

// public function
// >= ������ ����
BOOL CKoreanDate::operator>=(const CKoreanDate& dateSrc) const
{
	return ( GetPassedDays() >= dateSrc.GetPassedDays() );
}

// public function
// <= ������ ����
BOOL CKoreanDate::operator<=(const CKoreanDate& dateSrc) const
{
	return ( GetPassedDays() <= dateSrc.GetPassedDays() );
}

// public function
// < ������ ����
BOOL CKoreanDate::operator<(const CKoreanDate& dateSrc) const
{
	return ( GetPassedDays() < dateSrc.GetPassedDays() );
}

// public function
// > ������ ����
BOOL CKoreanDate::operator>(const CKoreanDate& dateSrc) const
{
	return ( GetPassedDays() > dateSrc.GetPassedDays() );
}



