// CalNumberEdit.cpp : implementation file
//

#include "stdafx.h"
//#include "calendar.h"
#include "CalNumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalNumberEdit

CCalNumberEdit::CCalNumberEdit()
{
	m_nMinValue = -1;
	m_nMaxValue = -1;
}

CCalNumberEdit::~CCalNumberEdit()
{
}


BEGIN_MESSAGE_MAP(CCalNumberEdit, CEdit)
	//{{AFX_MSG_MAP(CCalNumberEdit)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalNumberEdit message handlers

BOOL CCalNumberEdit::PreTranslateMessage(MSG* pMsg) 
{
	CWnd* pWnd;
	CString strText;
	int nValue;
	BOOL bOK;

	// 키보드 입력을 검사한다.
	// 리턴키이면 부모 윈도우에 엔터 눌렀음을 알리고 숨긴다
	// ESC 키이면 숨긴다.
	// 방향키, DEL, BACKSPACE, 0-9 키가 아니면 키 입력 무시한다.
	if( pMsg->message == WM_KEYDOWN )
	{
		switch( pMsg->wParam )
		{
		case VK_RETURN:
			GetWindowText(strText);
			nValue = _ttoi( (LPTSTR)(LPCTSTR)strText );
			bOK = TRUE;
			if( m_nMinValue != -1 && nValue < m_nMinValue )
			{
				bOK = FALSE;
				strText.Format(_T("%d"), m_nMinValue);
				SetWindowText((LPCTSTR)strText);
				SetSel(0, strText.GetLength());
			}
			if( m_nMaxValue != -1 && nValue > m_nMaxValue )
			{
				bOK = FALSE;
				strText.Format(_T("%d"), m_nMaxValue);
				SetWindowText((LPCTSTR)strText);
				SetSel(0, strText.GetLength());
			}

			if(bOK)
			{
				pWnd = GetParent();
				pWnd->SendMessage(CNEN_RETURN, (WPARAM)nValue, (LPARAM)m_nWhatDoing);
				pWnd->SetFocus();
				ShowWindow(SW_HIDE);
			}

			return TRUE;
		case VK_ESCAPE:
			pWnd = GetParent();
			pWnd->SetFocus();
			ShowWindow(SW_HIDE);
			return TRUE;
		case VK_UP:
			GetWindowText(strText);
			nValue = _ttoi( (LPTSTR)(LPCTSTR)strText );
			if(nValue != 0)
			{
				nValue++;
				if( m_nMaxValue != -1 && nValue > m_nMaxValue ) nValue = m_nMaxValue;
				strText.Format(_T("%d"), nValue);
				SetWindowText((LPCTSTR)strText);
				SetSel(0, strText.GetLength());
			}
			return TRUE;
		case VK_DOWN:
			GetWindowText(strText);
			nValue = _ttoi( (LPTSTR)(LPCTSTR)strText );
			if(nValue != 0)
			{
				nValue--;
				if( m_nMinValue != -1 && nValue < m_nMinValue ) nValue = m_nMinValue;
				strText.Format(_T("%d"), nValue);
				SetWindowText((LPCTSTR)strText);
				SetSel(0, strText.GetLength());
			}
			return TRUE;
		case VK_LEFT:
		case VK_RIGHT:
		case VK_DELETE:
		case VK_BACK:
		case VK_HOME:
		case VK_END:
			return FALSE;
		}

		if( pMsg->wParam < '0' || pMsg->wParam > '9' ) return TRUE;
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

void CCalNumberEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	ShowWindow(SW_HIDE)	;
}


BOOL CCalNumberEdit::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= ES_RIGHT|ES_AUTOHSCROLL;

	return CEdit::PreCreateWindow(cs);
}
