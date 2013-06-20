#if !defined(AFX_EXTTREECTRL_H__CD455D84_6BDF_4A42_BBA6_6B58B0728393__INCLUDED_)
#define AFX_EXTTREECTRL_H__CD455D84_6BDF_4A42_BBA6_6B58B0728393__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtTreeCtrl.h : header file
//

#include "..\\common\\AllCommonDefines.h"
/////////////////////////////////////////////////////////////////////////////
// CExtTreeCtrl window

/********************************************************************
    filename:   CherryTree.h
    author:     (c) 2005 Alex Zakharenko
    
    purpose:    Interface of advanced tree control (MFC edition)
*********************************************************************/

#pragma once

// CBitmapT class was taken from WTL 7.1 library
// Windows Template Library - WTL version 7.1
// Copyright (C) 1997-2003 Microsoft Corporation
// All rights reserved.
class U_STRINGorID
{
public:
    U_STRINGorID( LPCTSTR lpString ) : m_lpstr( lpString )
    { }
    U_STRINGorID( UINT nID ) : m_lpstr( MAKEINTRESOURCE( nID ) )
    { }
    LPCTSTR m_lpstr;
};

template <bool t_bManaged>
class CBitmapT
{
public:
// Data members
    HBITMAP m_hBitmap;

// Constructor/destructor/operators
    CBitmapT(HBITMAP hBitmap = NULL) : m_hBitmap(hBitmap)
    {
	}

    ~CBitmapT()
    {
        if(t_bManaged && m_hBitmap != NULL)
            DeleteObject();
    }

    CBitmapT< t_bManaged >& operator=( HBITMAP hBitmap )
    {
        Attach( hBitmap );
        return *this;
    }

    void Attach( HBITMAP hBitmap )
    {
        if( t_bManaged && m_hBitmap != NULL && m_hBitmap != hBitmap )
            ::DeleteObject( m_hBitmap );
        m_hBitmap = hBitmap;
    }

    HBITMAP Detach()
    {
        HBITMAP hBitmap = m_hBitmap;
        m_hBitmap = NULL;
        return hBitmap;
    }

    operator HBITMAP() const { return m_hBitmap; }

    bool IsNull() const { return ( m_hBitmap == NULL ); }

	// Create and load methods
    HBITMAP LoadBitmap(U_STRINGorID bitmap)
    {
        ASSERT(m_hBitmap == NULL);
#if (_ATL_VER >= 0x0700)
        m_hBitmap = ::LoadBitmap(ATL::_AtlBaseModule.GetResourceInstance(), bitmap.m_lpstr);
#else //!(_ATL_VER >= 0x0700)
        m_hBitmap = ::LoadBitmap(_Module.GetResourceInstance(), bitmap.m_lpstr);
#endif //!(_ATL_VER >= 0x0700)
        return m_hBitmap;
    }

    HBITMAP LoadOEMBitmap(UINT nIDBitmap) // for OBM_/OCR_/OIC_
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap));
        return m_hBitmap;
    }

    HBITMAP LoadMappedBitmap(UINT nIDBitmap, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
    {
        ASSERT(m_hBitmap == NULL);
#if (_ATL_VER >= 0x0700)
        m_hBitmap = ::CreateMappedBitmap(ATL::_AtlBaseModule.GetResourceInstance(), nIDBitmap, (WORD)nFlags, lpColorMap, nMapSize);
#else //!(_ATL_VER >= 0x0700)
        m_hBitmap = ::CreateMappedBitmap(_Module.GetResourceInstance(), nIDBitmap, (WORD)nFlags, lpColorMap, nMapSize);
#endif //!(_ATL_VER >= 0x0700)
        return m_hBitmap;
    }

    HBITMAP CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitcount, const void* lpBits)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateBitmap(nWidth, nHeight, nPlanes, nBitcount, lpBits);
        return m_hBitmap;
    }

    HBITMAP CreateBitmapIndirect(LPBITMAP lpBitmap)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateBitmapIndirect(lpBitmap);
        return m_hBitmap;
    }

    HBITMAP CreateCompatibleBitmap(HDC hDC, int nWidth, int nHeight)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
        return m_hBitmap;
    }

    HBITMAP CreateDiscardableBitmap(HDC hDC, int nWidth, int nHeight)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDiscardableBitmap(hDC, nWidth, nHeight);
        return m_hBitmap;
    }

    BOOL DeleteObject()
    {
        ASSERT(m_hBitmap != NULL);
        BOOL bRet = ::DeleteObject(m_hBitmap);
        if(bRet)
            m_hBitmap = NULL;
        return bRet;
    }

// Attributes
    int GetBitmap(BITMAP* pBitMap) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetObject(m_hBitmap, sizeof(BITMAP), pBitMap);
    }

    bool GetBitmap(BITMAP& bm) const
    {
        ASSERT(m_hBitmap != NULL);
        return (::GetObject(m_hBitmap, sizeof(BITMAP), &bm) == sizeof(BITMAP));
    }

    bool GetSize(SIZE& size) const
    {
        ASSERT(m_hBitmap != NULL);
        BITMAP bm = { 0 };
        if(!GetBitmap(&bm))
            return false;
        size.cx = bm.bmWidth;
        size.cy = bm.bmHeight;
        return true;
    }

    DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetBitmapBits(m_hBitmap, dwCount, lpBits);
    }

    DWORD SetBitmapBits(DWORD dwCount, const void* lpBits)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetBitmapBits(m_hBitmap, dwCount, lpBits);
    }

    BOOL GetBitmapDimension(LPSIZE lpSize) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetBitmapDimensionEx(m_hBitmap, lpSize);
    }

    BOOL SetBitmapDimension(int nWidth, int nHeight, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetBitmapDimensionEx(m_hBitmap, nWidth, nHeight, lpSize);
    }

// DIB support
    HBITMAP CreateDIBitmap(HDC hDC, CONST BITMAPINFOHEADER* lpbmih, DWORD dwInit, CONST VOID* lpbInit, CONST BITMAPINFO* lpbmi, UINT uColorUse)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDIBitmap(hDC, lpbmih, dwInit, lpbInit, lpbmi, uColorUse);
        return m_hBitmap;
    }

    HBITMAP CreateDIBSection(HDC hDC, CONST BITMAPINFO* lpbmi, UINT uColorUse, VOID** ppvBits, HANDLE hSection, DWORD dwOffset)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDIBSection(hDC, lpbmi, uColorUse, ppvBits, hSection, dwOffset);
        return m_hBitmap;
    }

    int GetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines,  LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT uColorUse) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetDIBits(hDC, m_hBitmap, uStartScan, cScanLines,  lpvBits, lpbmi, uColorUse);
    }

    int SetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetDIBits(hDC, m_hBitmap, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
    }
};

typedef CBitmapT< false >   CBitmapHandle;


//// End of CBitmapT implementation

#ifndef TVIS_FOCUSED
#define TVIS_FOCUSED    1
#else
#if TVIS_FOCUSED != 1
#error TVIS_FOCUSED was assumed to be 1
#endif
#endif

//define for custom notification
// sent on check-box change
// (Care: may clash in future)
#ifdef _MST_TVN_CHKCHANGE
#ifndef _MST_MULTI_CHECK
#define _MST_MULTI_CHECK    //implied
#endif
#ifndef TVN_CHKCHANGE
#define TVN_CHKCHANGEA  (TVN_FIRST-49)
#define TVN_CHKCHANGEW  (TVN_FIRST-(49+49))
#ifdef UNICODE
#define TVN_CHKCHANGE   TVN_CHKCHANGEW
#else
#define TVN_CHKCHANGE   TVN_CHKCHANGEA
#endif
#endif //!TVN_CHKCHANGE
#endif //_MST_TVN_CHKCHANGE

typedef CList< HTREEITEM, HTREEITEM > CTreeItemList;

//const DWORD DefaultColor = 0xffffffff;
const COLORREF DefaultColor = RGB( 23,117,128 );

const COLORREF DefaultTransparentColor = RGB( 255, 255, 255 );
const COLORREF DefaultSelectedItemColor = RGB( 255, 255, 255 );
const COLORREF DefaultSelectedNonFocusedItemColor = RGB( 0, 0, 0 );

#ifndef MST_TIMER_PERIOD
#define MST_TIMER_PERIOD    75      //ms
#endif

#define _bShift ( nFlags & MK_SHIFT )
#define _bCtrl  ( nFlags & MK_CONTROL )

enum ItemState
{
    isLeaf = 0,
    isExpanded = 1,
    isCollapsed = 2
};

// Picture position constants
const DWORD ppReservedPositions = 0xf0000000;
const DWORD ppFirstLeft     = 0xffffffff;
const DWORD ppLastLeft      = 0xfffffffe;
const DWORD ppFirstRight    = 0xfffffffd;
const DWORD ppLastRight     = 0xfffffffc;

const int g_nDefaultIndent = 8;
const int g_nDefaultElementGap = 2;
const int g_nLabelLeadIn = 2;
const int g_nLabelLeadOut = 4;

const UINT g_uAnimationTimerId = 37692;
const int g_nDefaultAnimationDelay = 100;

// More tree styles
#define TVS_TRACKGROUPSELECT    0x10000
#define TVS_SINGLECLICKEXPAND   0x20000

class CExtTreeCtrl;

class TreeItemImage
{
public:
    TreeItemImage() : TransparentColor( DefaultTransparentColor ) {};
    virtual ~TreeItemImage() {};

    virtual bool IsAnimation() = 0;
    virtual void RenderItem( HDC dc, POINT upper_left, bool bNextFrame = false ) = 0;
    virtual CSize GetSize() = 0;

    DWORD dwItemId;

    COLORREF GetTransparentColor();
    COLORREF SetTransparentColor( COLORREF color );

private:
    COLORREF TransparentColor;
};

class TreeItemBitmap : public TreeItemImage
{
public:
    TreeItemBitmap( HBITMAP hBmp );

    virtual bool IsAnimation();
    virtual void RenderItem( HDC dc, POINT upper_left, bool bNextFrame = false );
    virtual CSize GetSize();

protected:
    CSize sizeBitmap;
    CBitmapHandle bmp;
};

class TreeItemIcon : public TreeItemImage
{
public:
    TreeItemIcon( HICON _hIcon ) : TreeItemImage(), hIcon( _hIcon ) {};
    virtual bool IsAnimation();
    virtual void RenderItem( HDC dc, POINT upper_left, bool bNextFrame = false );
    virtual CSize GetSize();

protected:
    HICON hIcon;
};

class TreeItemAnimation : public TreeItemImage
{
public:
    TreeItemAnimation( const CExtTreeCtrl& refParentTree, int nFrames, HBITMAP hFirstFrame, ... );
    TreeItemAnimation( const CExtTreeCtrl& refParentTree, int nFrames, const HBITMAP aFrames[] );

    virtual ~TreeItemAnimation();

    virtual bool IsAnimation();
    virtual void RenderItem( HDC dc, POINT upper_left, bool bNextFrame = false );
    virtual CSize GetSize();
    void AddFrame( HBITMAP hBmp );

protected:
    CArray< CBitmapHandle, CBitmapHandle > aFrames;
    CSize sizeAnimation;
    INT_PTR nCurrentFrame;

    const CExtTreeCtrl& refTreeCtrl;
};

class TreeItemImgList : public TreeItemImage
{
public:
    TreeItemImgList( const CExtTreeCtrl& refParentTree, int nIndex ) : TreeItemImage(), 
        nIndex( nIndex ), refTreeCtrl( refParentTree ) {};
    virtual bool IsAnimation();
    virtual void RenderItem( HDC dc, POINT upper_left, bool bNextFrame = false );
    virtual CSize GetSize();

protected:
    int nIndex;
    const CExtTreeCtrl& refTreeCtrl;
};

class TreeItemData
{
public:
	/*
    TreeItemData() : itemColor( DefaultColor ), bBold( false ), bItalic( false ), 
        nIndent( g_nDefaultIndent ), dwNextImageId( 1 )
    {}
	*/
	TreeItemData();
    ~TreeItemData();

    COLORREF itemColor;

    bool bBold;
    bool bItalic;    
    UINT nIndent;

    DWORD dwNextImageId;
    CList< TreeItemImage*, TreeItemImage* > aPreImages;
    CList< TreeItemImage*, TreeItemImage* > aPostImages;
		
	char m_szIP			[15+1];	
	char m_szID			[20+1];	
	char m_szNickName	[C_NICK_NAME_LEN + 1];
	char m_szRealName	[20 + 1];
	char m_szGrpName	[C_GRP_NAME_LEN+1];
	BOOL m_bConnStatus; 
	int	 m_nBockMode;
	long m_nStatusFlags;
	char m_szItComp     [C_IT_COMP_LEN+1];
};

struct TreeItemPositionInfo
{
    TreeItemPositionInfo() : rectLabel( 0, 0, 0, 0 ), rectLeftIcons( 0, 0, 0, 0 ), 
        rectButton( 0, 0, 0, 0 ), rectRightIcons( 0, 0, 0, 0 ), nItemWidth( 0 )
    {};

    CRect rectLabel;
    CRect rectButton;
    CRect rectLeftIcons;
    CRect rectRightIcons;

    int nItemWidth;
};

// CExtTreeCtrl

class CExtTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CExtTreeCtrl)

	// m_TrackSelectionColor : 마우스 MOVE 시 Tracking Color..
	
    // Construction / destruction
    CExtTreeCtrl() : m_bMulti( true ), m_hSelect( NULL ), m_bBandLabel( false ),
        m_bEmulated( false ), m_pDefaultItemFont( NULL ), m_pBoldItemFont( NULL ),
        m_pItalicItemFont( NULL ), m_pBoldItalicItemFont( NULL ),
        m_nDefaultIndent( g_nDefaultIndent ), m_nElementGap( g_nDefaultElementGap ),
        m_SelectionColor( DefaultColor ), m_BackgroundColor( DefaultColor ),
        m_TrackSelectionColor( RGB(203,100,24) ), m_SelectedItemColor( DefaultColor ),
        m_SelectionNonFocusedColor( DefaultColor ), m_SelectedNonFocusedItemColor( DefaultColor ),
        m_uAnimationTimer( 0 ), m_nAnimatedObjects( 0 ), 
        m_nAnimationDelay( g_nDefaultAnimationDelay ), m_hTrackSelection( NULL ),
        m_bHorzScrollerVisible( false )
    {}
    virtual ~CExtTreeCtrl();

    // Multiple selection support
    bool SetMultiSelect( bool bMulti );
    bool IsMultiSelect() const
    {
        return m_bMulti;
    };

    UINT GetSelectedCount() const;
    HTREEITEM GetFirstSelectedItem() const;
    HTREEITEM GetNextSelectedItem( HTREEITEM hItem ) const;
    bool IsSelected( HTREEITEM hItem ) const
    {
        return ( TVIS_SELECTED & CTreeCtrl::GetItemState( hItem, TVIS_SELECTED ) ) != 0;
    };
    void GetSelectedList( CTreeItemList& list ) const;

    //Note: GetSelectedItem should be taken as GetFocusedItem
    // when in multi-select mode
    HTREEITEM GetFocusedItem() const;
    BOOL FocusItem( HTREEITEM hItem );

    // SelectChildren returns TRUE if focus was on a child item
    BOOL SelectChildren( HTREEITEM hParent, BOOL bSelect = TRUE, BOOL bRecurse = TRUE );
    void SelectAll( bool bSelect = true ); //doesn't affect focus
    void SelectRange( HTREEITEM hFirst, HTREEITEM hLast, BOOL bOnly = TRUE );

    // Note: not virtual, so must call this class (or derivative)
    BOOL SetItemState( HTREEITEM hItem, UINT nState, UINT nStateMask );
    UINT GetItemState( HTREEITEM hItem, UINT nStateMask ) const;
    BOOL SelectItem( HTREEITEM hItem );

    // Custom appearance support

    // Operations that affect whole tree control
    BOOL SetFont( const CFont* pFont );
    void SetSelectionColor( COLORREF colorFocused, COLORREF colorNonFocused );
    void SetSelectedItemColor( COLORREF colorFocused, COLORREF colorNonFocused );
    void SetTrackSelectionColor( COLORREF color );
    COLORREF SetBkColor( COLORREF color );
    COLORREF GetBkColor() const;
    bool SetTreeBitmaps( HBITMAP hExpanded = NULL, HBITMAP hCollapsed = NULL, HBITMAP hEmpty = NULL );
    void SetDefaultIndent( UINT nIndent );
    void SetElementGap( int nGap );
    int GetAnimationDelay();
    int SetAnimationDelay( int nDelay );

    // Per-item operations
    COLORREF GetItemColor( HTREEITEM hItem ) const;
    BOOL SetItemColor( HTREEITEM hItem, COLORREF color );
    BOOL SetItemTextAttributes( HTREEITEM hItem, bool bBold, bool bItalic );
    BOOL SetItemIndent( HTREEITEM hItem, int nIndent, bool bRecursive );
    DWORD AddItemBitmap( HTREEITEM hItem, HBITMAP hBmp, DWORD where );
    DWORD AddItemIcon( HTREEITEM hItem, HICON hIcon, DWORD where );
    DWORD AddItemBitmapFromImageList( HTREEITEM hItem, int nImageIndex, DWORD where );
    DWORD AddItemAnimation( HTREEITEM hItem, DWORD where, int nFrames, const HBITMAP hBmp[] );
    BOOL DeleteItemImage( HTREEITEM hItem, DWORD idImage );

    // Some overrides to preserve correct functionality
    BOOL DeleteItem( HTREEITEM hItem );
    BOOL DeleteAllItems();

    // BOOL SetItemData( HTREEITEM hItem, DWORD_PTR dwData );
    // DWORD_PTR GetItemData( HTREEITEM hItem ) const;

    BOOL GetItemRect( HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly ) const;

	TreeItemData* GetCustomItemData( HTREEITEM hItem ) const;

	BOOL SetItemUserInfo( HTREEITEM hItem, const char* szIP , const char* szID ,
									const char* szNickName, const char* szRealName,  const char* szGrpName,
									const BOOL& bConnStatus, const int nBockMode, const long& n_StatusFlags);

	BOOL GetItemUserInfo( HTREEITEM hItem, char* szIP , char* szID, char* szNickName, char* szRealName, 
						char* szGrpName, BOOL& bConnStatus, int* npBockMode, long* n_StatusFlags);	

public:
	HTREEITEM m_CurHotItem;	
	char m_szSelItemIP [15+1];
	char m_szSelItemID [20+1];
private:
    bool        m_bMulti;
    HTREEITEM   m_hSelect;  //for shift selection
    HTREEITEM   m_hTrackSelection;
    bool        m_bEmulated;
    bool        m_bBandLabel;

    CFont*      m_pDefaultItemFont;
    CFont*      m_pBoldItemFont;
    CFont*      m_pItalicItemFont;
    CFont*      m_pBoldItalicItemFont;

    CBitmapHandle m_bmpCollapsed;
    CBitmapHandle m_bmpExpanded;
    CBitmapHandle m_bmpEmpty;

    UINT        m_nDefaultIndent;

    COLORREF    m_SelectionColor;
    COLORREF    m_SelectionNonFocusedColor;
    COLORREF    m_BackgroundColor;
    COLORREF    m_TrackSelectionColor;
    COLORREF    m_SelectedItemColor;
    COLORREF    m_SelectedNonFocusedItemColor;

    int         m_nElementGap;

    CMap< HTREEITEM, HTREEITEM&, TreeItemPositionInfo, TreeItemPositionInfo& > m_mapPositionCache;
    CMap< HTREEITEM, HTREEITEM&, TreeItemData*, TreeItemData* > m_mapItemData;

    // Animation support
    int         m_nAnimationDelay;
    int         m_nAnimatedObjects;
    UINT_PTR    m_uAnimationTimer;

    // Value is the animation count for item
    CMap< HTREEITEM, HTREEITEM&, int, int& > m_mapAnimations;
    CMap< HTREEITEM, HTREEITEM&, int, int& > m_mapNewFrames;

    bool m_bHorzScrollerVisible;

	CString m_strFileAll;
	int     m_nFileCnt; // 선택한 파일이 몇개인가 

public: //kojh
	HTREEITEM HitTest( TVHITTESTINFO* pHitTestInfo ) const;
	HTREEITEM HitTest( CPoint pt, UINT* pFlags = NULL ) const;


protected:
    // Implementation stuff
    bool SetBandingHit( bool bLabel );
    LRESULT ForwardReflectedNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void DoPreSelection( HTREEITEM hItem, BOOL bLeft, UINT nFlags );
    void DoAction( HTREEITEM hItem, BOOL bLeft, UINT nFlags, CPoint point );
    void DoBanding( BOOL bLeft, UINT nFlags, CPoint point );
    void UpdateSelectionForRect( LPCRECT pRect, UINT nFlags, CTreeItemList& list );
    void SelectAllIgnore( bool bSelect, HTREEITEM hIgnore );
    BOOL _SendNotify( LPNMHDR pNMHDR );

    //TreeItemData* GetCustomItemData( HTREEITEM hItem ) const;
    bool ItemExists( HTREEITEM hItem ) const;
    TreeItemData* CreateCustomItemData( HTREEITEM hItem );

    void UpdateAnimationStatus();
	

	
    // Message handlers
	//{{AFX_MSG(CExtTreeCtrl)
    afx_msg void OnLButtonDown( UINT nStatus, CPoint pnt );
    afx_msg void OnRButtonDown( UINT nStatus, CPoint pnt );
    afx_msg void OnSetFocus( CWnd *pWnd );
    afx_msg void OnKillFocus( CWnd *pWnd );
    afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg void OnTimer( UINT idEvent );
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnItemExpanding( NMHDR *pNotifyStruct, LRESULT* result ); 
    afx_msg void OnCustomDraw( NMHDR *pNotifyStruct, LRESULT* result );
    afx_msg void OnHScroll( UINT nScrollType, UINT nPos, CScrollBar *pScrollBar );
    afx_msg BOOL OnSetCursor( CWnd *pWnd, UINT uHitTest, UINT uMouseMsg );
    afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnButtonDown( BOOL bLeft, UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClick( UINT nFlags, CPoint pnt );
	afx_msg void PaintItem( HTREEITEM hItem, HDC hDC, CRect rect );
	afx_msg LRESULT OnEraseBackground( HDC hDC );	

    void UpdateHorzScroller();

    DWORD ReplaceItemImage( TreeItemData* pData, DWORD dwTarget, TreeItemImage* pImage );

	void FileFindRecurse (LPCTSTR pstr);
	
	DECLARE_MESSAGE_MAP()
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTTREECTRL_H__CD455D84_6BDF_4A42_BBA6_6B58B0728393__INCLUDED_)
