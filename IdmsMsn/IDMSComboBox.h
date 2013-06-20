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
	CStringArray m_astrName;	// 아이템 이름 배열
	CStringArray m_astrCode;	// 아이템 코드 배열
	int			 m_nCodeCount;	// 콤보아이템 코드 컬럼수
	
// member function
public:
	void		DeleteAllItem();											// 모든 아이템 삭제
	void		SetCode(CString strCode, int nCol=1);						// 코드로 아이템을 찾아서 선택
	CString		GetCode(int nCol=1);										// 선택된 아이템의 nCol번째 컬럼의 코드 구함
	CString		GetName(int nIndex=-1);										// nIndex번째 아이템의 이름을 구함
	int			AddItem(CString strName, CString strCode);					// 콤보에 아이템 추가
	int			InsertItem(long nIndex, CString strName, CString strCode);	// 콤보의 특정위치에 아이템 추가
	
	// override
	int			AddString(LPCTSTR lpszString);
	int			DeleteString(UINT nIndex);
	int			InsertString(int nIndex, LPCTSTR lpszString);
	void		ResetContent();
	long		GetCount();													// 콤보아이템 갯수 구함

// member function
protected:
	CString		GetStringByDelim(CString strValue, int nCol);	// 코드 레코드의 nCol컬럼의 코드 구함
	int			GetColCnt(CString strString);					// 코드 레코드의 컬럼갯수를 구함
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDMSCOMBOBOX_H__BBCD5427_D9E7_4996_BB80_EA28AA4F98D6__INCLUDED_)
