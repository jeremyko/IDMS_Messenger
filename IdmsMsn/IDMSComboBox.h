#if !defined(AFX_IDMSCOMBOBOX_H__BBCD5427_D9E7_4996_BB80_EA28AA4F98D6__INCLUDED_)
#define AFX_IDMSCOMBOBOX_H__BBCD5427_D9E7_4996_BB80_EA28AA4F98D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IDMSComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIDMSComboBox window

static const char* IDMSCOMBOBOX_DELIM = "`";


class CIDMSComboBox : public CComboBox
{
// Construction
public:
	CIDMSComboBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDMSComboBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIDMSComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIDMSComboBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	// member variable
private:
	CStringArray m_astrName;	// ������ �̸� �迭
	CStringArray m_astrCode;	// ������ �ڵ� �迭
	int			 m_nCodeCount;	// �޺������� �ڵ� �÷���
	
// member function
public:
	void		DeleteAllItem();											// ��� ������ ����
	void		SetCode(CString strCode, int nCol=1);						// �ڵ�� �������� ã�Ƽ� ����
	CString		GetCode(int nCol=1);										// ���õ� �������� nCol��° �÷��� �ڵ� ����
	CString		GetName(int nIndex=-1);										// nIndex��° �������� �̸��� ����
	int			AddItem(CString strName, CString strCode);					// �޺��� ������ �߰�
	int			InsertItem(long nIndex, CString strName, CString strCode);	// �޺��� Ư����ġ�� ������ �߰�
	
	// override
	int			AddString(LPCTSTR lpszString);
	int			DeleteString(UINT nIndex);
	int			InsertString(int nIndex, LPCTSTR lpszString);
	void		ResetContent();
	long		GetCount();													// �޺������� ���� ����

// member function
protected:
	CString		GetStringByDelim(CString strValue, int nCol);	// �ڵ� ���ڵ��� nCol�÷��� �ڵ� ����
	int			GetColCnt(CString strString);					// �ڵ� ���ڵ��� �÷������� ����
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDMSCOMBOBOX_H__BBCD5427_D9E7_4996_BB80_EA28AA4F98D6__INCLUDED_)
