// IDMSComboBox.cpp : implementation file
//

#include "stdafx.h"

#include "IDMSComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIDMSComboBox

CIDMSComboBox::CIDMSComboBox()
{
}

CIDMSComboBox::~CIDMSComboBox()
{
}


BEGIN_MESSAGE_MAP(CIDMSComboBox, CComboBox)
	//{{AFX_MSG_MAP(CIDMSComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/******************************************************************************
	함수명		: GetCode(int nCol)
	함수설명	: 현재선택된 콤보아이템의 코드중 nCol번째 코드를 알아낸다
	인자설명	: [IN] nCol	: 원하는 코드의 컬럼인덱스
				  
	리턴값		: 코드스트링
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
CString CIDMSComboBox::GetCode(int nCol/*=1*/)
{
	// 코드 인덱스값 검사
	if( nCol>m_nCodeCount ) 
		return _T("");

	// 선택된아이템이 없으면 NULL값 리턴
	if( GetCurSel()<0 )
		return _T("");
	
	return GetStringByDelim(m_astrCode.GetAt(GetCurSel()), nCol);
}

/******************************************************************************
	함수명		: SetCode(CString strCode, int nCol)
	함수설명	: 코드로 콤보아이템 찾아 선택한다
	인자설명	: [IN] strCode	: 코드
				  [IN] nCol		: 코드가 있는 컬럼위치(default:1)
	
	리턴값		: 없음
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
void CIDMSComboBox::SetCode(CString strCode, int nCol/*=1*/)
{
	// 코드 인덱스값 검사 
	if( nCol>m_nCodeCount )
		return;

	// 코드값이 있는 아이템을 찾아내서 선택해준다
	for( int i=0 ; i<GetCount() ; i++ )
	{
		if( strCode.IsEmpty() ) 
			break;
		
		if( strCode == GetStringByDelim(m_astrCode.GetAt(i), nCol))
		{
			SetCurSel(i);
			return;
		}
	}

	// 코드를 찾아내지 못하면 첫번째 코드 선택
	SetCurSel(0);
}

/******************************************************************************
	함수명		: AddItem(CString strName, CString strCode)
	함수설명	: 콤보박스에 아이템을 추가한다
	인자설명	: [IN] strName	: 콤보박스에 보여질 텍스트
				  [IN] strCode	: 콤보박스 아이템이 가질 코드값
								  ("`"구분자로 여러개의 코드 입력가능)
	
	리턴값		: 에러발생시 CB_ERR 리턴, 성공시 아이템갯수 리턴 
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
int CIDMSComboBox::AddItem(CString strName, CString strCode)
{
	if( strCode.IsEmpty() == TRUE )
		return CB_ERR;

	// IDMSCOMBOBOX_DELIM 구분자 붙여준다
	if( strCode.Right(1) != IDMSCOMBOBOX_DELIM )
		strCode += IDMSCOMBOBOX_DELIM;

	// 처음입력일 경우 코드 컬럼갯수 설정
	if( GetCount() == 0 )
		m_nCodeCount = GetColCnt(strCode);
	else
	{
		// 코드갯수 체크
		if( m_nCodeCount != GetColCnt(strCode) )
			return CB_ERR;
	}
	
	// 이미 추가된 아이템이 아닐경우만 추가
	if( FindStringExact(0, strName) == CB_ERR  )
	{
		CComboBox::AddString(strName);
		m_astrName.Add(strName);
		m_astrCode.Add(strCode);
	}

	return GetCount();
}

/******************************************************************************
	함수명		: InsertItem(long nIndex, CString strName, CString strCode)
	함수설명	: 콤보박스에 아이템을 원하는 위치에 삽입한다
	인자설명	: [IN] nIndex	: 삽입될 위치
				  [IN] strName	: 콤보박스에 보여질 텍스트
				  [IN] strCode	: 콤보박스 아이템이 가질 코드값
								  ("`"구분자로 여러개의 코드 입력가능)
	
	리턴값		: 에러발생시 CB_ERR 리턴, 성공시 아이템갯수 리턴 
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
int CIDMSComboBox::InsertItem(long nIndex, CString strName, CString strCode)
{
	if( strCode.IsEmpty() )
		return CB_ERR;

	// IDMSCOMBOBOX_DELIM 구분자 붙여준다
	if( strCode.Right(1) != IDMSCOMBOBOX_DELIM )
		strCode += IDMSCOMBOBOX_DELIM;

	if( GetCount() == 0 )
		return AddItem(strName, strCode);

	// 삽입범위 체크
	if( nIndex<0 || nIndex>=GetCount() )
		return CB_ERR;

	// 코드갯수 체크
	if( m_nCodeCount != GetColCnt(strCode) )
		return CB_ERR;
	
	// 이미 추가된 아이템이 아닐경우만 추가
	if( FindStringExact(0, strName) == CB_ERR )
	{
		CComboBox::InsertString(nIndex, strName);
		m_astrName.InsertAt(nIndex, strName);
		m_astrCode.InsertAt(nIndex, strCode);
	}

	return GetCount();
}

/******************************************************************************
	함수명		: GetCount()
	함수설명	: 아이템 갯수를 알아낸다
	인자설명	: 없음
	리턴값		: 아이템 갯수
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
inline long CIDMSComboBox::GetCount()
{
	return m_astrName.GetSize();
}

/******************************************************************************
	함수명		: DeleteAllItem()
	함수설명	: 아이템 모두 삭제
	인자설명	: 없음
	리턴값		: 없음
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
void CIDMSComboBox::DeleteAllItem()
{
	m_astrCode.RemoveAll();
	m_astrName.RemoveAll();
	
	CComboBox::ResetContent();
}

/******************************************************************************
	함수명		: GetColCnt(CString strString)
	함수설명	: 코드문다열의 코드갯수를 샌다
	인자설명	: [IN] strString : 코드문자열
				  
	리턴값		: 코드갯수
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
int CIDMSComboBox::GetColCnt(CString strString)
{
	int nColCnt = 0;

	for( int i=0 ; i<strString.GetLength() ; i++)
		if( CString(strString.GetAt(i)) == IDMSCOMBOBOX_DELIM ) nColCnt++;
	
	return nColCnt;
}

/******************************************************************************
	함수명		: GetStringByDelim(CString strValue, int nCol)
	함수설명	: 코드문자열에서 원하는 위치의 코드값을 찾아낸다
	인자설명	: [IN] strValue : 코드문자열
				  [IN] nCol		: 찾을 코드 인덱스(1베이스 인덱스)
				  
	리턴값		: 코드갯수
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
CString CIDMSComboBox::GetStringByDelim(CString strValue, int nCol)
{
	if( strValue.IsEmpty() ) 
		return _T("");

	if( nCol<1 )
		return _T("");

	CString strCode;
	int		nIndex;

	// 인자로 받은 nCol번째의 코드값을 찾아낸다
	for( int i=0 ; i<m_nCodeCount ; i++ )
	{
		nIndex  = strValue.Find(IDMSCOMBOBOX_DELIM);
		strCode = strValue.Left(nIndex);
		
		if( i == (nCol-1) )
			break;
		
		strValue = strValue.Mid(nIndex+1);
	}

	return strCode;
}

/******************************************************************************
	함수명		: GetName(UINT nIndex)
	함수설명	: 임의의 인덱스의 아이템 이름을 얻어낸다
	인자설명	: [IN] nIndex : 원하는 아이템의 인덱스				  
	리턴값		: 성공시 아이템의 이름, 실패시 NULL값
	최종수정일	: 2003년 10월 14일
	참고사항	: 
 ******************************************************************************/
CString CIDMSComboBox::GetName(int nIndex/*=-1*/)
{
	if( nIndex == -1 ) nIndex = GetCurSel();

	// 인덱스 범위 체크
	if( nIndex<0 || nIndex>=GetCount() )
		return _T("");

	return m_astrName.GetAt(nIndex);
}

/******************************************************************************
	오버라이딩한 함수들
 ******************************************************************************/
int CIDMSComboBox::AddString(LPCTSTR lpszString)
{
	return CB_ERR;
}

int CIDMSComboBox::DeleteString(UINT nIndex)
{
	// 삭제범위 체크
	if( (UINT)GetCount() <= nIndex )
		return CB_ERR;

	CComboBox::DeleteString(nIndex);
	m_astrCode.RemoveAt(nIndex);
	m_astrName.RemoveAt(nIndex);

	return GetCount();
}

int CIDMSComboBox::InsertString(int nIndex, LPCTSTR lpszString)
{
	return CB_ERR; 
}

void CIDMSComboBox::ResetContent( )
{
	DeleteAllItem();
}
/////////////////////////////////////////////////////////////////////////////
// CIDMSComboBox message handlers
