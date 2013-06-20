// ColorListCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "filetransferdlg.h"
#include "ColorListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl

CColorListCtrl::CColorListCtrl()
{
	m_ColorBgError = RGB(255,45,38);
	m_ColorTextError= RGB(255,239,63);
}

CColorListCtrl::~CColorListCtrl()
{
}


BEGIN_MESSAGE_MAP(CColorListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CColorListCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomdrawList )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl message handlers

void CColorListCtrl::OnCustomdrawList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );    
	
	int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{		
        *pResult = CDRF_NOTIFYITEMDRAW;		
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{		
		if( GetItemText(nItem, 1) == "대기중")
		{
		}
		else if( GetItemText(nItem, 1) == "완료")
		{ 			
			pLVCD->clrText	 = m_ColorText;
			pLVCD->clrTextBk = m_ColorBg ;
			
		}	
		else 
		{
			pLVCD->clrText	 = m_ColorTextError;
			pLVCD->clrTextBk = m_ColorBgError ;			
		}	
		*pResult = CDRF_DODEFAULT;	
	}	
}