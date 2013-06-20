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
	�Լ���		: GetCode(int nCol)
	�Լ�����	: ���缱�õ� �޺��������� �ڵ��� nCol��° �ڵ带 �˾Ƴ���
	���ڼ���	: [IN] nCol	: ���ϴ� �ڵ��� �÷��ε���
				  
	���ϰ�		: �ڵ彺Ʈ��
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
CString CIDMSComboBox::GetCode(int nCol/*=1*/)
{
	// �ڵ� �ε����� �˻�
	if( nCol>m_nCodeCount ) 
		return _T("");

	// ���õȾ������� ������ NULL�� ����
	if( GetCurSel()<0 )
		return _T("");
	
	return GetStringByDelim(m_astrCode.GetAt(GetCurSel()), nCol);
}

/******************************************************************************
	�Լ���		: SetCode(CString strCode, int nCol)
	�Լ�����	: �ڵ�� �޺������� ã�� �����Ѵ�
	���ڼ���	: [IN] strCode	: �ڵ�
				  [IN] nCol		: �ڵ尡 �ִ� �÷���ġ(default:1)
	
	���ϰ�		: ����
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
void CIDMSComboBox::SetCode(CString strCode, int nCol/*=1*/)
{
	// �ڵ� �ε����� �˻� 
	if( nCol>m_nCodeCount )
		return;

	// �ڵ尪�� �ִ� �������� ã�Ƴ��� �������ش�
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

	// �ڵ带 ã�Ƴ��� ���ϸ� ù��° �ڵ� ����
	SetCurSel(0);
}

/******************************************************************************
	�Լ���		: AddItem(CString strName, CString strCode)
	�Լ�����	: �޺��ڽ��� �������� �߰��Ѵ�
	���ڼ���	: [IN] strName	: �޺��ڽ��� ������ �ؽ�Ʈ
				  [IN] strCode	: �޺��ڽ� �������� ���� �ڵ尪
								  ("`"�����ڷ� �������� �ڵ� �Է°���)
	
	���ϰ�		: �����߻��� CB_ERR ����, ������ �����۰��� ���� 
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
int CIDMSComboBox::AddItem(CString strName, CString strCode)
{
	if( strCode.IsEmpty() == TRUE )
		return CB_ERR;

	// IDMSCOMBOBOX_DELIM ������ �ٿ��ش�
	if( strCode.Right(1) != IDMSCOMBOBOX_DELIM )
		strCode += IDMSCOMBOBOX_DELIM;

	// ó���Է��� ��� �ڵ� �÷����� ����
	if( GetCount() == 0 )
		m_nCodeCount = GetColCnt(strCode);
	else
	{
		// �ڵ尹�� üũ
		if( m_nCodeCount != GetColCnt(strCode) )
			return CB_ERR;
	}
	
	// �̹� �߰��� �������� �ƴҰ�츸 �߰�
	if( FindStringExact(0, strName) == CB_ERR  )
	{
		CComboBox::AddString(strName);
		m_astrName.Add(strName);
		m_astrCode.Add(strCode);
	}

	return GetCount();
}

/******************************************************************************
	�Լ���		: InsertItem(long nIndex, CString strName, CString strCode)
	�Լ�����	: �޺��ڽ��� �������� ���ϴ� ��ġ�� �����Ѵ�
	���ڼ���	: [IN] nIndex	: ���Ե� ��ġ
				  [IN] strName	: �޺��ڽ��� ������ �ؽ�Ʈ
				  [IN] strCode	: �޺��ڽ� �������� ���� �ڵ尪
								  ("`"�����ڷ� �������� �ڵ� �Է°���)
	
	���ϰ�		: �����߻��� CB_ERR ����, ������ �����۰��� ���� 
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
int CIDMSComboBox::InsertItem(long nIndex, CString strName, CString strCode)
{
	if( strCode.IsEmpty() )
		return CB_ERR;

	// IDMSCOMBOBOX_DELIM ������ �ٿ��ش�
	if( strCode.Right(1) != IDMSCOMBOBOX_DELIM )
		strCode += IDMSCOMBOBOX_DELIM;

	if( GetCount() == 0 )
		return AddItem(strName, strCode);

	// ���Թ��� üũ
	if( nIndex<0 || nIndex>=GetCount() )
		return CB_ERR;

	// �ڵ尹�� üũ
	if( m_nCodeCount != GetColCnt(strCode) )
		return CB_ERR;
	
	// �̹� �߰��� �������� �ƴҰ�츸 �߰�
	if( FindStringExact(0, strName) == CB_ERR )
	{
		CComboBox::InsertString(nIndex, strName);
		m_astrName.InsertAt(nIndex, strName);
		m_astrCode.InsertAt(nIndex, strCode);
	}

	return GetCount();
}

/******************************************************************************
	�Լ���		: GetCount()
	�Լ�����	: ������ ������ �˾Ƴ���
	���ڼ���	: ����
	���ϰ�		: ������ ����
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
inline long CIDMSComboBox::GetCount()
{
	return m_astrName.GetSize();
}

/******************************************************************************
	�Լ���		: DeleteAllItem()
	�Լ�����	: ������ ��� ����
	���ڼ���	: ����
	���ϰ�		: ����
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
void CIDMSComboBox::DeleteAllItem()
{
	m_astrCode.RemoveAll();
	m_astrName.RemoveAll();
	
	CComboBox::ResetContent();
}

/******************************************************************************
	�Լ���		: GetColCnt(CString strString)
	�Լ�����	: �ڵ幮�ٿ��� �ڵ尹���� ����
	���ڼ���	: [IN] strString : �ڵ幮�ڿ�
				  
	���ϰ�		: �ڵ尹��
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
int CIDMSComboBox::GetColCnt(CString strString)
{
	int nColCnt = 0;

	for( int i=0 ; i<strString.GetLength() ; i++)
		if( CString(strString.GetAt(i)) == IDMSCOMBOBOX_DELIM ) nColCnt++;
	
	return nColCnt;
}

/******************************************************************************
	�Լ���		: GetStringByDelim(CString strValue, int nCol)
	�Լ�����	: �ڵ幮�ڿ����� ���ϴ� ��ġ�� �ڵ尪�� ã�Ƴ���
	���ڼ���	: [IN] strValue : �ڵ幮�ڿ�
				  [IN] nCol		: ã�� �ڵ� �ε���(1���̽� �ε���)
				  
	���ϰ�		: �ڵ尹��
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
CString CIDMSComboBox::GetStringByDelim(CString strValue, int nCol)
{
	if( strValue.IsEmpty() ) 
		return _T("");

	if( nCol<1 )
		return _T("");

	CString strCode;
	int		nIndex;

	// ���ڷ� ���� nCol��°�� �ڵ尪�� ã�Ƴ���
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
	�Լ���		: GetName(UINT nIndex)
	�Լ�����	: ������ �ε����� ������ �̸��� ����
	���ڼ���	: [IN] nIndex : ���ϴ� �������� �ε���				  
	���ϰ�		: ������ �������� �̸�, ���н� NULL��
	����������	: 2003�� 10�� 14��
	�������	: 
 ******************************************************************************/
CString CIDMSComboBox::GetName(int nIndex/*=-1*/)
{
	if( nIndex == -1 ) nIndex = GetCurSel();

	// �ε��� ���� üũ
	if( nIndex<0 || nIndex>=GetCount() )
		return _T("");

	return m_astrName.GetAt(nIndex);
}

/******************************************************************************
	�������̵��� �Լ���
 ******************************************************************************/
int CIDMSComboBox::AddString(LPCTSTR lpszString)
{
	return CB_ERR;
}

int CIDMSComboBox::DeleteString(UINT nIndex)
{
	// �������� üũ
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
