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


// BitmapDialog.h : main header file for the BITMAPDIALOG application
//

#if !defined(AFX_BITMAPDIALOG_H__55BE67E3_17E6_11D2_9AB8_0060B0CDC13E__INCLUDED_)
#define AFX_BITMAPDIALOG_H__55BE67E3_17E6_11D2_9AB8_0060B0CDC13E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "dib256.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CIDMSBitmapDialog dialog

class CIDMSBitmapDialog : public CDialog
{
// Construction
public:
	CIDMSBitmapDialog(UINT uResourceID, CWnd* pParent = NULL);
	CIDMSBitmapDialog(LPCTSTR pszResourceID, CWnd* pParent = NULL);
	CIDMSBitmapDialog();

	enum {
		BITMAP_TOP = 0,
		BITMAP_TILE,		// tile the bitmap to fill the dialog
		BITMAP_STRETCH,			// stretch the bitmap so it fits to the dialog
		BITMAP_CENTER			// center the bitmap inside the dialog
	};


// Attributes
public:
	BOOL	SetBitmap(UINT uResource, int Type = CIDMSBitmapDialog::BITMAP_TOP);

// Dialog Data
protected:
	//{{AFX_DATA(CIDMSBitmapDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDMSBitmapDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CDIBitmap	m_bmpBackground;
	int			m_nType;		// see enum above
	CBrush		m_HollowBrush;

	// Generated message map functions
	//{{AFX_MSG(CIDMSBitmapDialog)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	CommonConstruct();
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPDIALOG_H__55BE67E3_17E6_11D2_9AB8_0060B0CDC13E__INCLUDED_)
