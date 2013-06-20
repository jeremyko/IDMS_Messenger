/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Jörg König
// All rights reserved
//
// This file is part of the completely free tetris clone "CGTetris".
//
// This is free software.
// You may redistribute it by any means providing it is not sold for profit
// without the authors written consent.
//
// No warrantee of any kind, expressed or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    J.Koenig@adg.de                 (company site)
//    Joerg.Koenig@rhein-neckar.de    (private site)
/////////////////////////////////////////////////////////////////////////////


// BitmapDialog.cpp
//

#include "stdafx.h"
#include "IDMSBitmapDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIDMSBitmapDialog dialog

CIDMSBitmapDialog::CIDMSBitmapDialog()
{
	CommonConstruct();
}


CIDMSBitmapDialog::CIDMSBitmapDialog(UINT uResource, CWnd* pParent /*=NULL*/)
	: CDialog(uResource, pParent)
{
	CommonConstruct();
}


CIDMSBitmapDialog::CIDMSBitmapDialog(LPCTSTR pszResource, CWnd* pParent /*=NULL*/)
	: CDialog(pszResource, pParent)
{
	CommonConstruct();
}


void CIDMSBitmapDialog::CommonConstruct()
{
	m_nType = BITMAP_TOP;
	VERIFY(m_HollowBrush.CreateStockObject(HOLLOW_BRUSH));

	//{{AFX_DATA_INIT(CIDMSBitmapDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BOOL CIDMSBitmapDialog::SetBitmap(UINT uResource, int nType /*BITMAP_TILE*/) {
	m_nType = nType;
	//ASSERT(m_nType == BITMAP_TILE || m_nType == BITMAP_STRETCH || m_nType == BITMAP_CENTER ||  m_nType == BITMAP_TOP);
	return m_bmpBackground.LoadResource(uResource);
}


void CIDMSBitmapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIDMSBitmapDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIDMSBitmapDialog, CDialog)
	//{{AFX_MSG_MAP(CIDMSBitmapDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDMSBitmapDialog message handlers

BOOL CIDMSBitmapDialog::OnEraseBkgnd(CDC* pDC) 
{
	if(m_bmpBackground.GetPixelPtr() != 0) {
		//ASSERT(m_nType == BITMAP_TILE || m_nType == BITMAP_STRETCH || m_nType == BITMAP_CENTER || m_nType == BITMAP_TOP);
		
		CRect rc;
		GetClientRect(rc);
		int x = 0, y = 0;

		switch(m_nType) {
			case BITMAP_TOP:
				CDialog::OnEraseBkgnd(pDC);
				x = 0;
				y = 0;
				m_bmpBackground.DrawDIB(pDC, x, y);
				break;
			case BITMAP_CENTER:
				// center the bitmap
				CDialog::OnEraseBkgnd(pDC);
				x = (rc.Width() - m_bmpBackground.GetWidth()) / 2;
				y = (rc.Height() - m_bmpBackground.GetHeight()) / 2;
				m_bmpBackground.DrawDIB(pDC, x, y);
				break;

			case BITMAP_STRETCH:
				// stretch bitmap so it will best fit to the dialog
				m_bmpBackground.DrawDIB(pDC, 0, 0, rc.Width(), rc.Height());
				break;

			default:
				// tile the bitmap
				while(y < rc.Height()) {
					while(x < rc.Width()) {
						m_bmpBackground.DrawDIB(pDC, x, y);
						x += m_bmpBackground.GetWidth();
					}
					x = 0;
					y += m_bmpBackground.GetHeight();
				}
				break;
		}
	} else
		// no bitmap set. behave like a normal dialog
		return CDialog::OnEraseBkgnd(pDC);

	return TRUE;
}

HBRUSH CIDMSBitmapDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if(m_bmpBackground.GetPixelPtr() != 0) 
	{
		switch(nCtlColor) 
		{
			case CTLCOLOR_STATIC:
				// The Slider Control has CTLCOLOR_STATIC, but doesn't let
				// the background shine through,
				TCHAR lpszClassName[255];
				GetClassName(pWnd->m_hWnd, lpszClassName, 255);
				if(_tcscmp(lpszClassName, TRACKBAR_CLASS) == 0)
					return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

			case CTLCOLOR_BTN:
				// let static controls shine through
				pDC->SetBkMode(TRANSPARENT);
				return HBRUSH(m_HollowBrush);

			default:
				break;
		}
	}
	
	// if we reach this line, we haven't set a brush so far
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CIDMSBitmapDialog::OnQueryNewPalette() 
{
	CPalette * pPal = m_bmpBackground.GetPalette();
	if( pPal != 0 && GetSafeHwnd() != 0 ) {
		CClientDC dc(this);
		CPalette * pOldPalette = dc.SelectPalette(pPal, FALSE);
		UINT nChanged = dc.RealizePalette();
		dc.SelectPalette(pOldPalette, TRUE);

		if (nChanged == 0)
			return FALSE;

		Invalidate();
		return TRUE;
	}
	return CDialog::OnQueryNewPalette();
}

void CIDMSBitmapDialog::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CPalette * pPal = m_bmpBackground.GetPalette();
	if( pPal != 0 && GetSafeHwnd() != 0 && pFocusWnd != this && ! IsChild(pFocusWnd) ) {
		CClientDC dc(this);
		CPalette * pOldPalette = dc.SelectPalette(pPal, TRUE);
		UINT nChanged = dc.RealizePalette();
		dc.SelectPalette(pOldPalette, TRUE);

		if( nChanged )
			Invalidate();
	} else
		CDialog::OnPaletteChanged(pFocusWnd);
}
