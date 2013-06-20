// ExtTreeCtrl.cpp : implementation file
//


/********************************************************************
    filename:   CherryTree.cpp
    author:     (c) 2005 Alex Zakharenko
    
    purpose:    Implementation of advanced tree control (MFC edition)
*********************************************************************/

#include "stdafx.h"
#include "ExtTreeCtrl.h"
#include "IdmsMsnDlg.h"
#include "MsgBoxThread.h"
#include "FileTransSock.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CExtTreeCtrl

IMPLEMENT_DYNAMIC(CExtTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CExtTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CExtTreeCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT( TVN_ITEMEXPANDING, OnItemExpanding )
	ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP	

END_MESSAGE_MAP()



// CExtTreeCtrl message handlers
COLORREF TreeItemImage::GetTransparentColor()
{
    return ( TransparentColor != DefaultColor ) ? TransparentColor : DefaultTransparentColor;
}

COLORREF TreeItemImage::SetTransparentColor( COLORREF color )
{
    COLORREF old = TransparentColor;
    TransparentColor = color;
    return old;
}

TreeItemBitmap::TreeItemBitmap( HBITMAP hBmp ) : TreeItemImage(), bmp( hBmp )
{
    bmp.GetSize( sizeBitmap );
}

bool TreeItemBitmap::IsAnimation()
{
    return false;
}

CSize TreeItemBitmap::GetSize()
{
    return sizeBitmap;
}

bool TreeItemIcon::IsAnimation()
{
    return false;
}

void TreeItemIcon::RenderItem( HDC dc, POINT upper_left, bool bNextFrame )
{
    ::DrawIcon( dc, upper_left.x, upper_left.y, hIcon );
}

CSize TreeItemIcon::GetSize()
{
    return CSize( 16, 16 );
}

TreeItemAnimation::~TreeItemAnimation()
{
    aFrames.RemoveAll();
}

bool TreeItemAnimation::IsAnimation()
{
    return true;
}

CSize TreeItemAnimation::GetSize()
{
    return sizeAnimation;
}

void TreeItemAnimation::AddFrame( HBITMAP hBmp )
{
    aFrames.Add( CBitmapHandle( hBmp ) );
    if( aFrames.GetSize() == 1 )
        aFrames[0].GetSize( sizeAnimation );
}

bool TreeItemImgList::IsAnimation()
{
    return false;
}

void TreeItemImgList::RenderItem( HDC dc, POINT upper_left, bool bNextFrame )
{
    CImageList *pImageList = refTreeCtrl.GetImageList( TVSIL_NORMAL );
	pImageList->Draw( CDC::FromHandle( dc ), nIndex, upper_left, ILD_NORMAL );
}

CSize TreeItemImgList::GetSize()
{
    CImageList *pImageList = refTreeCtrl.GetImageList( TVSIL_NORMAL );

    IMAGEINFO imageInfo;
    if( pImageList->GetImageInfo( nIndex, &imageInfo ) != 0 )
        return CSize( imageInfo.rcImage.right, imageInfo.rcImage.left );

    return CSize( 0, 0 );
}

void TreeItemBitmap::RenderItem( HDC dc, POINT upper_left, bool bNextFrame )
{
    CDC BmpDC;
	BmpDC.CreateCompatibleDC( CDC::FromHandle( dc ) );
    HGDIOBJ hOldBmp = BmpDC.SelectObject( bmp );

    //::TransparentBlt( dc, upper_left.x, upper_left.y, GetSize().cx, GetSize().cy, BmpDC, 0, 0, GetSize().cx, GetSize().cy, GetTransparentColor() );	

    ::BitBlt( dc, upper_left.x, upper_left.y, GetSize().cx, GetSize().cy,   BmpDC, 0, 0, SRCCOPY );

    BmpDC.SelectObject( hOldBmp );
}

TreeItemAnimation::TreeItemAnimation( const CExtTreeCtrl& refParentTree, int nFrames, 
                                      HBITMAP hFirstFrame, ... ) : TreeItemImage(), refTreeCtrl( refParentTree )
{
    if( nFrames != 0 )
    {
        va_list marker;
        va_start( marker, hFirstFrame );

        for( int i = 0; i < nFrames; i++ )
        {
            HBITMAP hFrame = va_arg( marker, HBITMAP );
            aFrames.Add( CBitmapHandle( hFrame ) );
        }

        aFrames[0].GetSize( sizeAnimation );
    }
    else
        sizeAnimation = CSize( 0, 0 );

    nCurrentFrame = 0;
}

TreeItemAnimation::TreeItemAnimation( const CExtTreeCtrl& refParentTree, int nFrames, 
                                      const HBITMAP _aFrames[] ) : TreeItemImage(), refTreeCtrl( refParentTree )
{
    if( nFrames != 0 )
    {
        for( int i = 0; i < nFrames; i++ )
            aFrames.Add( _aFrames[ i ] );

        aFrames[0].GetSize( sizeAnimation );
    }
    else
        sizeAnimation = CSize( 0, 0 );

    nCurrentFrame = 0;
}

void TreeItemAnimation::RenderItem( HDC dc, POINT upper_left, bool bNextFrame )
{
    if( aFrames.GetSize() == 0 )
        return;

    CDC BmpDC;
	BmpDC.CreateCompatibleDC( CDC::FromHandle( dc ) );
    HGDIOBJ hOldBmp = BmpDC.SelectObject( aFrames[ nCurrentFrame ] );

    CBrush Filler;
    Filler.CreateSolidBrush( refTreeCtrl.GetBkColor() );
    ::FillRect( dc, CRect( upper_left.x, upper_left.y, upper_left.x + GetSize().cx, 
        upper_left.y + GetSize().cy ), Filler );

    // TODO: use custom function instead of TransparentBlt
    //::TransparentBlt( dc, upper_left.x, upper_left.y, GetSize().cx, GetSize().cy,  BmpDC, 0, 0, GetSize().cx, GetSize().cy, GetTransparentColor() );

    ::BitBlt( dc, upper_left.x, upper_left.y, GetSize().cx, GetSize().cy, BmpDC, 0, 0, SRCCOPY );

    BmpDC.SelectObject( hOldBmp );

    if( bNextFrame )
    {
        nCurrentFrame++;
        if( nCurrentFrame >= aFrames.GetSize() )
            nCurrentFrame = 0;
    }
}

TreeItemData::TreeItemData()
{
	itemColor = DefaultColor ;
	bBold =  false ;
	bItalic =  false ;
	nIndent =  g_nDefaultIndent ;
	dwNextImageId =  1 ;

	memset(m_szIP , 0x00, sizeof(m_szIP));	
	memset(m_szNickName , 0x00, sizeof(m_szNickName));	
	memset(m_szRealName , 0x00, sizeof(m_szRealName));	
	memset(m_szGrpName , 0x00, sizeof(m_szGrpName));
	m_bConnStatus = FALSE; 
	m_nBockMode = 0;
	m_nStatusFlags = 0l;
}

TreeItemData::~TreeItemData()
{
    while( aPreImages.GetCount() != 0 )
    {
        TreeItemImage* pData = aPreImages.GetHead();
		//TRACE("~TreeItemData() [%s]\n", m_szID );
        delete pData;
		pData = NULL;
        aPreImages.RemoveHead();
    }

    while( aPostImages.GetCount() != 0 )
    {
        TreeItemImage* pData = aPostImages.GetHead();
		//TRACE("~TreeItemData() [%s]\n", m_szID );
        delete pData;
		pData = NULL;
        aPostImages.RemoveHead();
    }    
}

CExtTreeCtrl::~CExtTreeCtrl()
{
    if( m_pDefaultItemFont )
        delete m_pDefaultItemFont;

    if( m_pBoldItemFont )
        delete m_pBoldItemFont;

    if( m_pItalicItemFont )
        delete m_pItalicItemFont;

    if( m_pBoldItalicItemFont )
        delete m_pBoldItalicItemFont;
}

UINT CExtTreeCtrl::GetSelectedCount() const
{
    UINT nCount = 0;
    HTREEITEM hItem = GetFirstSelectedItem();
    while( hItem )
    {
        nCount++;
        hItem = GetNextSelectedItem( hItem );
    }
    return nCount;
}

HTREEITEM CExtTreeCtrl::GetFirstSelectedItem() const
{
    HTREEITEM hItem = GetRootItem();
    while( hItem )
    {
        if( IsSelected( hItem ) )
            break;
        hItem = GetNextVisibleItem( hItem );
    }
    return hItem;
}

HTREEITEM CExtTreeCtrl::GetNextSelectedItem( HTREEITEM hItem ) const
{
    hItem = GetNextVisibleItem( hItem );
    while( hItem )
    {
        if( IsSelected( hItem ) )
            break;
        hItem = GetNextVisibleItem( hItem );
    }
    return hItem;
}

void CExtTreeCtrl::SelectAll( bool bSelect ) //doesn't affect focus
{
    bSelect = !!bSelect;    //ensure 0 or 1
    UINT nState = bSelect ? TVIS_SELECTED : 0;
    HTREEITEM hItem = GetRootItem();
    while( hItem )
    {
        if( IsSelected( hItem ) != bSelect )
            SetItemState( hItem, nState, TVIS_SELECTED );
        hItem = GetNextVisibleItem( hItem );
    }
}

void CExtTreeCtrl::SelectRange( HTREEITEM hFirst, HTREEITEM hLast, BOOL bOnly )
{
    //locate (and select) either first or last
    // (so order is arbitrary)
    HTREEITEM hItem = GetRootItem();
    while( hItem )
    {
        if( ( hItem == hFirst ) || ( hItem == hLast ) )
        {
            if( hFirst != hLast )
            {
                if( !IsSelected( hItem ) )
                    SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
                hItem = GetNextVisibleItem( hItem );
            }
            break;
        }

        if( bOnly && IsSelected( hItem ) )
            SetItemState( hItem, 0, TVIS_SELECTED );
        hItem = GetNextVisibleItem( hItem );
    }
    //select rest of range
    while( hItem )
    {
        if( !IsSelected( hItem ) )
            SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
        if( ( hItem == hFirst ) || ( hItem == hLast ) )
        {
            hItem = GetNextVisibleItem( hItem );
            break;
        }
        hItem = GetNextVisibleItem( hItem );
    }
    if( !bOnly )
        return;
    while( hItem )
    {
        if( IsSelected( hItem ) )
            SetItemState( hItem, 0, TVIS_SELECTED );
        hItem = GetNextVisibleItem( hItem );
    }
}

// SelectChildren returns TRUE if focus was on a child item
BOOL CExtTreeCtrl::SelectChildren( HTREEITEM hParent, BOOL bSelect , BOOL bRecurse )
{
    UINT nS = bSelect ? TVIS_SELECTED : 0;

    BOOL bFocusWasInHere = FALSE;

    HTREEITEM hItem = GetNextItem( hParent, TVGN_CHILD );
    while( hItem )
    {
        UINT nState = GetItemState( hItem, TVIS_SELECTED | TVIS_EXPANDED | TVIS_FOCUSED );
        if( ( nState & TVIS_SELECTED ) != nS )
            SetItemState( hItem, nS, TVIS_SELECTED );
        bFocusWasInHere |= ( nState & TVIS_FOCUSED );
        if( bRecurse && ( nState & TVIS_EXPANDED ) )
            bFocusWasInHere |= SelectChildren( hItem, bSelect, bRecurse );
        hItem = GetNextSiblingItem( hItem );
    }
    return bFocusWasInHere;
}

// Note: not virtual, so must call this class (or derivative)
BOOL CExtTreeCtrl::SetItemState( HTREEITEM hItem, UINT nState, UINT nStateMask )
{
    ASSERT( hItem );

    if( !m_bMulti )
		return CTreeCtrl::SetItemState( hItem, nState, nStateMask );

    HTREEITEM hFocus = GetSelectedItem();       //current focus
    BOOL bWasFocus = ( hFocus == hItem );
    BOOL bFocusWasSel = hFocus && IsSelected( hFocus );   //selection state of current focus
    BOOL bWasSel = IsSelected( hItem );       //select state of acting item

    UINT nS = nState & ~TVIS_FOCUSED;
    UINT nSM = nStateMask & ~TVIS_FOCUSED;

    if( nStateMask & TVIS_FOCUSED )
    {
        //wanted to affect focus
        if( nState & TVIS_FOCUSED )
        {
            //wanted to set focus
            if( !bWasFocus && bFocusWasSel )
            {
                //because SelectItem would de-select the current 'real' selection
                // (the one with focus), need to make the tree ctrl think there is
                // no 'real' selection but still keep the the old item selected
                //it has to be done before the SelectItem call because
                // otherwise the TVN_SELCHANGING/ED notification handlers
                // wouldn't be able to get the proper list of selected items
				CTreeCtrl::SelectItem( NULL );  //will cause notify, but can be taken as focus change
				CTreeCtrl::SetItemState( hFocus, TVIS_SELECTED, TVIS_SELECTED );
                UpdateWindow();
            }
			if( !CTreeCtrl::SelectItem( hItem ) )    //set focus (will consequently select, if not already focused)
                return FALSE;
            if( nStateMask & TVIS_SELECTED )
            {
                //wanted to affect select state
                if( nState & TVIS_SELECTED )
                {
                    //wanted to select, already done if wasn't focused
                    if( !bWasFocus || bFocusWasSel )
                    {
                        nS &= ~TVIS_SELECTED;
                        nSM &= ~TVIS_SELECTED;
                    }
                }
                //else wanted to clear, base call will do that
            }
            else
            {
                //didn't want to affect select state
                if( !bWasSel )
                {
                    //it wasn't previously selected, let base clear (correct)
                    nS &= ~TVIS_SELECTED;
                    nSM |= TVIS_SELECTED;
                }
                //else was already selected, no harm done
            }
        }
        else
        {
            //wanted to clear focus
            if( bWasFocus )
            {
                //it had the focus
				CTreeCtrl::SelectItem( NULL );  //clear focus
                if( !( nStateMask & TVIS_SELECTED ) )
                {
                    //didn't want to affect select state
                    if( bWasSel )
                    {
                        //it was selected, so restore
                        ASSERT( !( nS & TVIS_SELECTED ) );
                        ASSERT( !( nSM & TVIS_SELECTED ) );
                        //set state here, to avoid double-notify
						CTreeCtrl::SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
                        //let base do other states
                    }
                }
                else if ( nState & TVIS_SELECTED )
                {
                    //wanted to select (but clear focus)
                    if( bWasSel )
                    {
                        //if was selected, restore
						CTreeCtrl::SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
                    }
                    //don't want to notify, default did it
                    nS &= ~TVIS_SELECTED;
                    nSM &= ~TVIS_SELECTED;
                }
            }
        }
    }

    if( !nSM )
        return TRUE;    //no other states to alter

    if( nSM & TVIS_SELECTED )
    {
        //still need to alter selection state
        NMTREEVIEW nmtv;
        nmtv.hdr.hwndFrom = m_hWnd;
        nmtv.hdr.idFrom = ::GetDlgCtrlID( m_hWnd );
        nmtv.hdr.code = TVN_SELCHANGING;
        nmtv.itemOld.mask = nmtv.itemNew.mask = 0;
        nmtv.itemOld.hItem = nmtv.itemNew.hItem = NULL;
        TVITEM& item = ( nS & TVIS_SELECTED ) ? nmtv.itemNew : nmtv.itemOld;
        item.mask = TVIF_HANDLE | TVIF_PARAM;
        item.hItem = hItem;
        item.lParam = GetItemData( hItem );
        if( _SendNotify( &nmtv.hdr ) )
            return FALSE;   //sel-changing stopped
		VERIFY( CTreeCtrl::SetItemState( hItem, nS, nSM ) );
        nmtv.hdr.code = TVN_SELCHANGED;
        _SendNotify( &nmtv.hdr );
        nS &= ~TVIS_SELECTED;
        nSM &= ~TVIS_SELECTED;
    }
    if( !nSM )
        return TRUE;
	return CTreeCtrl::SetItemState( hItem, nS, nSM );
}

BOOL CExtTreeCtrl::SetFont( const CFont* pFont )
{
    ASSERT( ::IsWindow( m_hWnd ) );

    LOGFONT fontinfo;
	if( ::GetObject( pFont->m_hObject, sizeof( fontinfo ), &fontinfo ) == 0 )
        return FALSE;

    fontinfo.lfQuality = DRAFT_QUALITY;

    if( m_pDefaultItemFont )
        delete m_pDefaultItemFont;
    if( m_pBoldItemFont )
        delete m_pBoldItemFont;
    if( m_pItalicItemFont )
        delete m_pItalicItemFont;

    m_pDefaultItemFont = new CFont;
    if( m_pDefaultItemFont->CreateFontIndirect( &fontinfo ) != NULL )
    {
        LONG lOldWeight = fontinfo.lfWeight;
        fontinfo.lfWeight = FW_BLACK;

        m_pBoldItemFont = new CFont;
        if( m_pBoldItemFont->CreateFontIndirect( &fontinfo ) != NULL )
        {
            fontinfo.lfWeight = lOldWeight;
            fontinfo.lfItalic = TRUE;
            m_pItalicItemFont = new CFont;
            if( m_pItalicItemFont->CreateFontIndirect( &fontinfo ) != NULL )
            {
                fontinfo.lfWeight = FW_BLACK;
                m_pBoldItalicItemFont = new CFont;
                if( m_pBoldItalicItemFont->CreateFontIndirect( &fontinfo ) )
                {
                    // Calculating new item height
                    CDC dc;
                    dc.CreateCompatibleDC( NULL );
                    HGDIOBJ hOldFont = dc.SelectObject( m_pDefaultItemFont );

                    CRect rect( 0, 0, 0, 0 );

                    dc.DrawText( _T( "ABCDEFGabcdefg" ), -1, rect, DT_TOP | DT_LEFT | DT_NOCLIP | DT_CALCRECT );
                    SetItemHeight( rect.Height() + 2 );

                    dc.SelectObject( hOldFont );

                    return TRUE;
                }
                delete m_pBoldItalicItemFont;
                m_pBoldItalicItemFont = NULL;
            }
            delete m_pItalicItemFont;
            m_pItalicItemFont = NULL;
        }
        delete m_pBoldItemFont;
        m_pBoldItemFont = NULL;
    }
    delete m_pDefaultItemFont;
    m_pDefaultItemFont = NULL;

    return FALSE;
}

UINT CExtTreeCtrl::GetItemState( HTREEITEM hItem, UINT nStateMask ) const
{
	UINT n = CTreeCtrl::GetItemState(hItem, nStateMask & ~TVIS_FOCUSED);
    if (nStateMask & TVIS_FOCUSED)
        if (GetSelectedItem() == hItem)
            n |= TVIS_FOCUSED;
    return n;
}

BOOL CExtTreeCtrl::SelectItem( HTREEITEM hItem )
{
    if( m_bMulti )
    {
        TRACE( _T( "Use SetItemState or FocusItem when in multi-select mode\n" ) );
        ASSERT(FALSE);
    }
	return CTreeCtrl::SelectItem(hItem);
}

HTREEITEM CExtTreeCtrl::GetFocusedItem() const
{
    ASSERT( m_bMulti );
	return CTreeCtrl::GetSelectedItem();
};

BOOL CExtTreeCtrl::FocusItem( HTREEITEM hItem )
{
    ASSERT( m_bMulti );

    BOOL bRet = FALSE;
    if( hItem )
        bRet = SetItemState( hItem, TVIS_FOCUSED, TVIS_FOCUSED );
    else
    {
		hItem = CTreeCtrl::GetSelectedItem();
        if( hItem )
            bRet = SetItemState( hItem, 0, TVIS_FOCUSED );
    }
    return bRet;
}

bool CExtTreeCtrl::SetMultiSelect( bool bMulti )
{
    bool b = m_bMulti;
    m_bMulti = bMulti;
    if( !m_bMulti )
    {
        HTREEITEM hItem = GetSelectedItem();
        if( hItem && !IsSelected( hItem ) )
            hItem = NULL;
        SelectAllIgnore( FALSE, hItem );
        if( hItem )
            SelectItem( hItem );
    }
    return b;
}

bool CExtTreeCtrl::SetBandingHit( bool bLabel )
{
    bool b = m_bBandLabel;
    m_bBandLabel = bLabel;
    return b;
};

void CExtTreeCtrl::GetSelectedList( CTreeItemList& list ) const
{
    list.RemoveAll();

    HTREEITEM hItem = GetFirstSelectedItem();
    while( hItem )
    {
        list.AddTail( hItem );
        hItem = GetNextSelectedItem( hItem );
    }
}

void CExtTreeCtrl::SetSelectionColor( COLORREF colorFocused, COLORREF colorNonFocused )
{
    m_SelectionColor = colorFocused;
    m_SelectionNonFocusedColor = colorNonFocused;
    Invalidate();
}

COLORREF CExtTreeCtrl::SetBkColor( COLORREF color )
{
    COLORREF colorOld = color;

	CTreeCtrl::SetBkColor( color );

    m_BackgroundColor = color;
    Invalidate();

    return colorOld;
}

COLORREF CExtTreeCtrl::GetBkColor() const
{
    COLORREF color = m_BackgroundColor;
    if( color == DefaultColor )
		color = CTreeCtrl::GetBkColor();
    if( color == DefaultColor )
        color = GetSysColor( COLOR_WINDOW );

    return color;
}

COLORREF CExtTreeCtrl::GetItemColor( HTREEITEM hItem ) const
{
    ASSERT(::IsWindow( m_hWnd ) );
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        return DefaultColor;
    return pData->itemColor;
}


BOOL CExtTreeCtrl::SetItemUserInfo(	HTREEITEM hItem, const char* szIP , const char* szID,  const char* szNickName, 
								    const char* szRealName, const char* szGrpName,
									const BOOL& bConnStatus, const int nBlockMode, const long& n_StatusFlags)
{
	ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    
	TreeItemData* pData = GetCustomItemData( hItem );

    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    if( pData != NULL )
    {        
		strncpy(pData->m_szIP , szIP, sizeof(pData->m_szIP));
		strncpy(pData->m_szID , szID, sizeof(pData->m_szID));
		strncpy(pData->m_szNickName , szNickName, sizeof(pData->m_szNickName));
		strncpy(pData->m_szRealName , szRealName, sizeof(pData->m_szRealName));
		strncpy(pData->m_szGrpName , szGrpName, sizeof(pData->m_szGrpName));
		
		pData->m_bConnStatus = bConnStatus;
		pData->m_nBockMode = nBlockMode;
		pData->m_nStatusFlags = n_StatusFlags;

        return TRUE;
    }

    return FALSE;

}

BOOL CExtTreeCtrl::GetItemUserInfo(	HTREEITEM hItem, char* szIP , char* szID , 
									char* szNickName, char* szRealName, 
									char* szGrpName, BOOL& bConnStatus, 
									int* nBockMode, long* n_StatusFlags)
{
	ASSERT(::IsWindow( m_hWnd ) );
    TreeItemData* pData = GetCustomItemData( hItem );

    if( pData == NULL )
        return FALSE;	
	
	memset(szIP , 0x00, sizeof(szIP));
	memset(szID , 0x00, sizeof(szID));
	memset(szNickName , 0x00, sizeof(szNickName));
	memset(szRealName , 0x00, sizeof(szRealName));
	memset(szGrpName , 0x00, sizeof(szGrpName));

	strncpy(szIP, pData->m_szIP , sizeof(pData->m_szIP));
	strncpy(szID, pData->m_szID , sizeof(pData->m_szID));
	strncpy(szNickName, pData->m_szNickName , sizeof(pData->m_szNickName));
	strncpy(szRealName, pData->m_szRealName , sizeof(pData->m_szRealName));	
	strncpy(szGrpName, pData->m_szGrpName , sizeof(pData->m_szGrpName));	
		
	bConnStatus = pData->m_bConnStatus;
	*nBockMode = pData->m_nBockMode;
	*n_StatusFlags = pData->m_nStatusFlags;

    return TRUE;
}

BOOL CExtTreeCtrl::SetItemColor( HTREEITEM hItem, COLORREF color )
{
    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    if( pData != NULL )
    {
        pData->itemColor = color;

        CRect rect;
        if( GetItemRect( hItem, rect, TRUE ) != 0 )
            InvalidateRect( rect );

        return TRUE;
    }

    return FALSE;
}

BOOL CExtTreeCtrl::SetItemTextAttributes( HTREEITEM hItem, bool bBold, bool bItalic )
{
    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    if( pData != NULL )
    {
        pData->bBold = bBold;
        pData->bItalic = bItalic;

        CRect rect;
        if( GetItemRect( hItem, rect, TRUE ) != 0 )
            InvalidateRect( rect );

        return TRUE;
    }

    return FALSE;
}

BOOL CExtTreeCtrl::DeleteItem( HTREEITEM hItem )
{
    HTREEITEM hChild;
	TreeItemData* pData = NULL;

    while( ( hChild = GetChildItem( hItem ) ) != NULL )
	{		
        DeleteItem( hChild );
	}

    BOOL bRes = FALSE;
    if( hItem != TVI_ROOT )
    {
        pData = GetCustomItemData( hItem );
				
		//TEST
		m_mapItemData.RemoveKey(hItem);

        bRes = CTreeCtrl::DeleteItem( hItem );
        
		if( pData != NULL )
            delete pData;
    }
    return bRes;
}

BOOL CExtTreeCtrl::DeleteAllItems()
{
    return DeleteItem( TVI_ROOT );
}

bool CExtTreeCtrl::SetTreeBitmaps( HBITMAP hExpanded, HBITMAP hCollapsed, HBITMAP hEmpty )
{
    m_bmpEmpty.Attach( hEmpty );
    m_bmpExpanded.Attach( hExpanded );
    m_bmpCollapsed.Attach( hCollapsed );
    return true;
}

BOOL CExtTreeCtrl::SetItemIndent( HTREEITEM hItem, int nIndent, bool bRecursive )
{
    ASSERT( ::IsWindow( m_hWnd ) );

    // If it's not root, updating indent
    if( hItem != TVI_ROOT )
    {
        if( !ItemExists( hItem ) )
            return FALSE;
        TreeItemData* pData = GetCustomItemData( hItem );
        if( pData == NULL )
            pData = CreateCustomItemData( hItem );

        if( pData != NULL )
        {
            pData->nIndent = nIndent;

            CRect rect;
            if( GetItemRect( hItem, rect, TRUE ) != 0 )
                InvalidateRect( rect );
        }
    }

    if( bRecursive )
    {
        // Calling same function for all children
        HTREEITEM hChild = GetChildItem( hItem );
        while( hChild != NULL )
        {
            SetItemIndent( hChild, nIndent, bRecursive );
            hChild = GetNextSiblingItem( hChild );
        }
    }

    return TRUE;
}

DWORD CExtTreeCtrl::ReplaceItemImage( TreeItemData* pData, DWORD dwTarget, TreeItemImage* pImage )
{
    CList< TreeItemImage*, TreeItemImage* >* aLists[] = { &pData->aPreImages, &pData->aPostImages };

    for( int i = 0; i < sizeof( aLists ) / sizeof( CList< TreeItemImage*,  TreeItemImage* >* ); i++ )
    {
        POSITION pos = aLists[i]->GetHeadPosition();
        while( pos != NULL )
        {
            POSITION oldPos = pos;
            TreeItemImage* pCurImage = aLists[i]->GetNext( pos );
            if( pCurImage->dwItemId == dwTarget )
            {
                aLists[i]->SetAt( oldPos, pImage );
                delete pCurImage;
                return pImage->dwItemId;
            }
        }
    }
    return 0;
}

DWORD CExtTreeCtrl::AddItemBitmap( HTREEITEM hItem, HBITMAP hBmp, DWORD where )
{
    DWORD IdItem = 0;

    ASSERT( ::IsWindow( m_hWnd ) );
    
	if( !ItemExists( hItem ) )
        return FALSE;
    
	TreeItemData* pData = GetCustomItemData( hItem );
    
	if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    CList< TreeItemImage* , TreeItemImage* >* pTarget;
    if( where == ppFirstRight || where == ppLastRight )
        pTarget = &pData->aPostImages;
    else
        pTarget = &pData->aPreImages;

	//pTarget->AssertValid();
	
    TreeItemBitmap* pBmp = new TreeItemBitmap( hBmp );
    pBmp->dwItemId = pData->dwNextImageId++;

    CRect rect;
    if( GetItemRect( hItem, rect, TRUE ) != 0 )
        InvalidateRect( rect );

    if( pData != NULL )
    {
        if( where & ppReservedPositions )
        {
            if( where == ppLastLeft || where == ppLastRight ) 
                pTarget->AddTail( pBmp );
            else
                pTarget->AddHead( pBmp );

            IdItem = pBmp->dwItemId;
        }
        else
		{
            IdItem = ReplaceItemImage( pData, where, pBmp );			
		}
    }

    return IdItem;
}

DWORD CExtTreeCtrl::AddItemIcon( HTREEITEM hItem, HICON hIcon, DWORD where )
{
    DWORD IdItem = 0;

    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    CList< TreeItemImage*, TreeItemImage* >* pTarget;
    if( where == ppFirstRight || where == ppLastRight )
        pTarget = &pData->aPostImages;
    else
        pTarget = &pData->aPreImages;

    TreeItemIcon* pIcon = new TreeItemIcon( hIcon );
    pIcon->dwItemId = pData->dwNextImageId++;

    CRect rect;
    if( GetItemRect( hItem, rect, TRUE ) != 0 )
        InvalidateRect( rect );

    if( pData != NULL )
    {
        if( where & ppReservedPositions )
        {
            if( where == ppLastLeft || where == ppLastRight ) 
                pTarget->AddTail( pIcon );
            else
                pTarget->AddHead( pIcon );

            IdItem = pIcon->dwItemId;
        }
        else
            IdItem = ReplaceItemImage( pData, where, pIcon );
    }

    return IdItem;
}

DWORD CExtTreeCtrl::AddItemAnimation( HTREEITEM hItem, DWORD where, int nFrames, const HBITMAP aBmp[] )
{
    DWORD IdItem = 0;

    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    CList< TreeItemImage*, TreeItemImage* >* pTarget;
    if( where == ppFirstRight || where == ppLastRight )
        pTarget = &pData->aPostImages;
    else
        pTarget = &pData->aPreImages;

    TreeItemAnimation* pAnimation = new TreeItemAnimation( *this, nFrames, aBmp );
    pAnimation->dwItemId = pData->dwNextImageId++;

    CRect rect;
    if( GetItemRect( hItem, rect, TRUE ) != 0 )
        InvalidateRect( rect );

    if( pData != NULL )
    {
        if( where & ppReservedPositions )
        {
            if( where == ppLastLeft || where == ppLastRight ) 
                pTarget->AddTail( pAnimation );
            else
                pTarget->AddHead( pAnimation );

            IdItem = pAnimation->dwItemId;
        }
        else
            IdItem = ReplaceItemImage( pData, where, pAnimation );

        m_mapAnimations[ hItem ]++;
        m_nAnimatedObjects++;

        UpdateAnimationStatus();
    }

    return IdItem;
}

DWORD CExtTreeCtrl::AddItemBitmapFromImageList( HTREEITEM hItem, int nImageIndex, DWORD where )
{
    DWORD IdItem = 0;

    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    CList< TreeItemImage*,  TreeItemImage* >* pTarget;
    if( where == ppFirstRight || where == ppLastRight )
        pTarget = &pData->aPostImages;
    else
        pTarget = &pData->aPreImages;

    TreeItemImgList* pItem = new TreeItemImgList( *this, nImageIndex );
    pItem->dwItemId = pData->dwNextImageId++;

    CRect rect;
    if( GetItemRect( hItem, rect, TRUE ) != 0 )
        InvalidateRect( rect );

    if( pData != NULL )
    {
        if( where & ppReservedPositions )
        {
            if( where == ppLastLeft || where == ppLastRight ) 
                pTarget->AddTail( pItem );
            else
                pTarget->AddHead( pItem );

            IdItem = pItem->dwItemId;
        }
        else
            IdItem = ReplaceItemImage( pData, where, pItem );
    }

    return IdItem;
}

BOOL CExtTreeCtrl::DeleteItemImage( HTREEITEM hItem, DWORD idImage )
{
    ASSERT( ::IsWindow( m_hWnd ) );
    if( !ItemExists( hItem ) )
        return FALSE;
    TreeItemData* pData = GetCustomItemData( hItem );
    if( pData == NULL )
        pData = CreateCustomItemData( hItem );

    CList< TreeItemImage*, TreeItemImage* >* pTarget;

    if( pData != NULL )
    {
        for( int i = 0; i < 2; i++ )
        {
            if( i == 0 )
                pTarget = &pData->aPreImages;
            else
                pTarget = &pData->aPreImages;

            POSITION pos = pTarget->GetHeadPosition();

            while( pos != NULL )
            {
                TreeItemImage* img = pTarget->GetAt( pos );
                if( img->dwItemId == idImage )
                {
                    if( img->IsAnimation() )
                    {
                        m_mapAnimations[ hItem ]--;
                        m_nAnimatedObjects--;
                        UpdateAnimationStatus();
                    }

                    delete img;
                    pTarget->RemoveAt( pos );


                    return TRUE;
                }

                pTarget->GetNext( pos );
            }
        }
    }

    return FALSE;
}

void CExtTreeCtrl::UpdateAnimationStatus()
{
    if( m_nAnimatedObjects == 0 )
    {
        if( m_uAnimationTimer != 0 )
        {
            KillTimer( m_uAnimationTimer );
            m_uAnimationTimer = 0;
        }
    }
    else
    {
        if( m_uAnimationTimer == 0 )
            m_uAnimationTimer = SetTimer( g_uAnimationTimerId, m_nAnimationDelay, NULL );
    }
}

int CExtTreeCtrl::GetAnimationDelay()
{
    return m_nAnimationDelay;
}

int CExtTreeCtrl::SetAnimationDelay( int nDelay )
{
    int nOld = m_nAnimationDelay;
    m_nAnimationDelay = nDelay;

    if( m_uAnimationTimer != 0 )
    {
        KillTimer( m_uAnimationTimer );
        m_uAnimationTimer = SetTimer( g_uAnimationTimerId, m_nAnimationDelay, NULL );
    }

    return nOld;
}

void CExtTreeCtrl::SetDefaultIndent( UINT nIndent )
{
    m_nDefaultIndent = nIndent;
}

void CExtTreeCtrl::SetElementGap( int nGap )
{
    m_nElementGap = nGap;
    Invalidate();
}

// Implementation
LRESULT CExtTreeCtrl::ForwardReflectedNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult = 0;
    switch( uMsg )
    {
    case OCM_COMMAND:
    case OCM_NOTIFY:
    case OCM_PARENTNOTIFY:
    case OCM_DRAWITEM:
    case OCM_MEASUREITEM:
    case OCM_COMPAREITEM:
    case OCM_DELETEITEM:
    case OCM_VKEYTOITEM:
    case OCM_CHARTOITEM:
    case OCM_HSCROLL:
    case OCM_VSCROLL:
    case OCM_CTLCOLORBTN:
    case OCM_CTLCOLORDLG:
    case OCM_CTLCOLOREDIT:
    case OCM_CTLCOLORLISTBOX:
    case OCM_CTLCOLORMSGBOX:
    case OCM_CTLCOLORSCROLLBAR:
    case OCM_CTLCOLORSTATIC:
        // Skipping reflector
        lResult = GetParent()->SendMessage( uMsg - OCM__BASE, wParam, lParam );
        break;
    default:
        bHandled = FALSE;
        break;
    }
    return lResult;
}

void CExtTreeCtrl::OnButtonDown( BOOL bLeft, UINT nFlags, CPoint point )
{
    UINT nHF;
    HTREEITEM hItem;

    BOOL bBase = !m_bMulti;
    if( !bBase )
    {
        hItem = HitTest( point, &nHF );
        if( hItem )
        {
            //base always handles expanding items
            //(doesn't really mean much to right button, but pass anyway)
            bBase = ( nHF & ( TVHT_ONITEMBUTTON /*|TVHT_ONITEMINDENT*/ ) );
            if( !bBase && bLeft && ( GetStyle() & TVS_CHECKBOXES ) )
            {
                //when the tree has check-boxes, the default handler makes
                // a quick selection of the clicked item, then re-selects
                // the previously selected item - to cause a sel-changed
                // notification.  Fortunately it doesn't affect the multi-
                // selection, so can pass on.
                bBase = ( nHF & TVHT_ONITEMSTATEICON );
#ifdef _MST_MULTI_CHECK
                //Use the above define if you want all selected items to
                // be checked the same when any one of them is checked
                // - Interestingly this doesn't happen in the listview control
                //  (LVS_EX_CHECKBOXES)
                if( bBase )
                {
                    //the default selection notification would mess
                    // the multi-selection up, so generate the notification
                    // manually
                    // (anyway, this is smoother than the selection flicker
                    //  the default gives)
                    NMTREEVIEW nmtv;
#ifdef TVN_CHKCHANGE
                    nmtv.hdr.code = TVN_CHKCHANGE;
#else
                    nmtv.hdr.code = TVN_SELCHANGED;
#endif
                    nmtv.hdr.hwndFrom = m_hWnd;
                    nmtv.hdr.idFrom = ::GetDlgCtrlID( m_hWnd );
                    nmtv.itemOld.hItem = NULL;
                    nmtv.itemNew.mask = TVIF_HANDLE|TVIF_PARAM;

                    BOOL bChk = !GetCheck( hItem );
                    if( IsSelected( hItem ) )
                    {
                        HTREEITEM h = GetFirstSelectedItem();
                        while( h )
                        {
                            if( !GetCheck( h ) == bChk )
                            {     //! to ensure 0 or 1
                                SetCheck( h, bChk );
#ifdef TVN_CHKCHANGE
                                //only send multiple check-change
                                // notifications (not sel-changed)
                                if( h != hItem ) //clicked item will be done last
                                {
                                    nmtv.itemNew.hItem = h;
                                    nmtv.itemNew.lParam = GetItemData( h );
                                    _SendNotify( &nmtv.hdr );
                                }
#endif
                            }
                            h = GetNextSelectedItem( h );
                        }
                    }
                    else
                        SetCheck( hItem, bChk );
                    //notify clicked item
                    nmtv.itemNew.hItem = hItem;
                    nmtv.itemNew.lParam = GetItemData( hItem );
                    _SendNotify( &nmtv.hdr );
                    return;
                }
#endif
            }
        }
    }
    if( bBase )
    {
        if( bLeft )
            DefWindowProc( WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
        else
            DefWindowProc( WM_RBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
        return;
    }

    if( !hItem || ( nHF & ( TVHT_ONITEMRIGHT | TVHT_NOWHERE | TVHT_ONITEMINDENT ) ) )
    {
        //clicked in space, do rubber-banding
        DoBanding( bLeft, nFlags, point );
        return;
    }

    ASSERT( nHF & ( TVHT_ONITEM | TVHT_ONITEMSTATEICON ) );  //nothing else left

    DoPreSelection( hItem, bLeft, nFlags );
    DoAction( hItem, bLeft, nFlags, point );
}

void CExtTreeCtrl::DoPreSelection( HTREEITEM hItem, BOOL bLeft, UINT nFlags )
{
    if( bLeft )
    {
        //if shift key down, select immediately
        if( _bShift )
        {
            if( !m_hSelect )
                m_hSelect = GetSelectedItem();  //focus
            SelectRange( m_hSelect, hItem, !_bCtrl );
            SetItemState( hItem, TVIS_FOCUSED, TVIS_FOCUSED ); //focus changes to last clicked
        }
        else
        {
            if( !_bCtrl )
            {
                //if ctrl was down, then the selection is delayed until
                // mouse up, otherwise select the one item
                if( !IsSelected( hItem ) )
                    SelectAllIgnore( FALSE, hItem );
                SetItemState( hItem, TVIS_SELECTED | TVIS_FOCUSED, TVIS_SELECTED | TVIS_FOCUSED );
            }
            m_hSelect = NULL;   //reset when a non-shift operation occurs
        }
        return;
    }

    //right mouse
    if( nFlags & ( MK_CONTROL | MK_SHIFT ) )
    {
        if (!_bShift)
            m_hSelect = hItem;
        return;     //do nothing if shift or ctrl
    }
    if( !IsSelected( hItem ) )
        SelectAllIgnore( FALSE, hItem );
    SetItemState( hItem, TVIS_SELECTED | TVIS_FOCUSED, TVIS_SELECTED | TVIS_FOCUSED );
}

void CExtTreeCtrl::DoAction( HTREEITEM hItem, BOOL bLeft, UINT nFlags, CPoint point )
{
    ::SetCapture( m_hWnd );
    ASSERT( ::GetCapture() == m_hWnd );

    MSG msg;
    UINT nDone = 0;
    CPoint pt;
    CSize sizeDrag( ::GetSystemMetrics( SM_CXDRAG ), ::GetSystemMetrics( SM_CYDRAG ) );

    while( !nDone && ::GetMessage( &msg, NULL, 0, 0 ) )
    {
        if( ::GetCapture() != m_hWnd )
            break;

        switch( msg.message )
        {
        case WM_MOUSEMOVE:
            pt.x = GET_X_LPARAM( msg.lParam );
            pt.y = GET_Y_LPARAM( msg.lParam );
            if( ( abs( pt.x - point.x ) > sizeDrag.cx )
                || ( ( abs( pt.y - point.y ) > sizeDrag.cy ) ) )
                nDone = 2;
            //because we exit loop, button up will still be dispatched
            // which means WM_CONTEXTMENU will be sent after TVN_BEGINRDRAG
            // - this is the same behaviour as original tree
            break;

        case WM_LBUTTONUP:
            {
                if( !_bShift && !_bCtrl )
                {
                    UINT uFlags;
                    HTREEITEM hItem = HitTest( point, &uFlags );
                    if( hItem && ( GetStyle() & TVS_SINGLECLICKEXPAND ) )
                    {
                        if( uFlags & TVHT_ONITEM )
                            Expand( hItem, TVE_TOGGLE );
                    }
                }
            }
            nDone = 1;
            break;

        case WM_RBUTTONUP:
            nDone = 1;
            break;

        default:
            ::DispatchMessage( &msg );
            break;
        }
    }

    ::ReleaseCapture();
    ASSERT( ::GetCapture() != m_hWnd );

    //construct tree notification info
    NMTREEVIEW nmtv;
    nmtv.hdr.hwndFrom = m_hWnd;
    nmtv.hdr.idFrom = ::GetDlgCtrlID( m_hWnd );
    nmtv.itemNew.mask = TVIF_HANDLE | TVIF_PARAM;
    nmtv.itemNew.hItem = hItem;
    nmtv.itemNew.lParam = GetItemData( hItem );
    DWORD dwStyle = GetStyle();

    if( nDone == 1 )
    {
        //click
        if( !_bShift && bLeft )
        {
            UINT nState = TVIS_SELECTED;
            if( _bCtrl )
                nState ^= ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED );
            else
                SelectAllIgnore( FALSE, hItem );
            SetItemState( hItem, TVIS_FOCUSED | nState, TVIS_FOCUSED | TVIS_SELECTED );
        }
        if( ::GetFocus() != m_hWnd )
            ::SetFocus( m_hWnd );
        nmtv.hdr.code = bLeft ? NM_CLICK : NM_RCLICK;
        _SendNotify( &nmtv.hdr );
    }
    else if ( nDone == 2 )
    {
        //drag
        SetItemState( hItem, TVIS_FOCUSED | TVIS_SELECTED, TVIS_FOCUSED | TVIS_SELECTED );
        if( !( dwStyle & TVS_DISABLEDRAGDROP ) )
        {
            nmtv.hdr.code = bLeft ? TVN_BEGINDRAG : TVN_BEGINRDRAG;
            nmtv.ptDrag = point;
            _SendNotify( &nmtv.hdr );
        }
    }
}

void CExtTreeCtrl::DoBanding( BOOL bLeft, UINT nFlags, CPoint point )
{
    int nAnimatedObjects = m_nAnimatedObjects;
    m_nAnimatedObjects = 0;
    UpdateAnimationStatus();

    if( ::GetFocus() != m_hWnd )
        ::SetFocus( m_hWnd );

    ::SetCapture( m_hWnd );

    CTreeItemList list;
    if( nFlags & ( MK_SHIFT | MK_CONTROL ) )
        GetSelectedList( list );

    CClientDC dc( this );
    CRect rectCli;
    GetClientRect( &rectCli );

    MSG msg;
    BOOL bDone = FALSE;
    CPoint pt;
    CSize sizeDrag( ::GetSystemMetrics( SM_CXDRAG ), ::GetSystemMetrics( SM_CYDRAG ) );
    BOOL bDrag = FALSE;
    CSize sizeEdge( 1, 1 );

    UINT_PTR nTimer = SetTimer( 1, MST_TIMER_PERIOD, NULL );  //for scroll
    CPoint ptScr( GetScrollPos( SB_HORZ ), GetScrollPos( SB_VERT ) );
    CRect rect( 0, 0, 0, 0 );
    UINT h = 0;
    HTREEITEM hItem = GetRootItem();
    if( hItem )
    {
        GetItemRect( hItem, &rect, FALSE );
        ptScr.y *= ( h = rect.Height() );     //this assumes equal height items
    }
    point += ptScr;

    while( !bDone && ::GetMessage( &msg, NULL, 0, 0 ) )
    {
        if( ::GetCapture() != m_hWnd )
            break;

        switch( msg.message )
        {
        case WM_TIMER:
            pt = msg.pt;
            ScreenToClient( &pt );
            if( rectCli.PtInRect( pt ) )
            {
                ::DispatchMessage( &msg );
                break;
            }
            msg.lParam = MAKELPARAM( pt.x, pt.y );
            //fall through to mousemove

        case WM_MOUSEMOVE:
            pt.x = GET_X_LPARAM( msg.lParam );
            pt.y = GET_Y_LPARAM( msg.lParam );
            if( !bDrag )
            {
                if( ( abs( pt.x - point.x ) <= sizeDrag.cx )
                    && ( ( abs( pt.y - point.y ) <= sizeDrag.cy ) ) )
                    break;
                bDrag = TRUE;
                if( !( nFlags & ( MK_CONTROL | MK_SHIFT ) ) )
                    SelectAll( FALSE );
                UpdateWindow();
                rect.SetRect( point, point );
                dc.DrawDragRect( rect, sizeEdge, NULL, sizeEdge );
            }

            dc.DrawDragRect( rect, sizeEdge, NULL, sizeEdge );    //delete

            if( pt.y < rectCli.top )
                ::SendMessage( m_hWnd, WM_VSCROLL, SB_LINEUP, 0 );
            else if( pt.y >= rectCli.bottom )
                ::SendMessage( m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0 );
            if( pt.x < rectCli.left )
                ::SendMessage( m_hWnd, WM_HSCROLL, SB_LINELEFT, 0 );
            else if( pt.x >= rectCli.right )
                ::SendMessage( m_hWnd, WM_HSCROLL, SB_LINERIGHT, 0 );

            ptScr = point;
            ptScr.x -= GetScrollPos( SB_HORZ );
            ptScr.y -= GetScrollPos( SB_VERT ) * h;
            rect.SetRect( ptScr, pt );
            rect.NormalizeRect();
            UpdateSelectionForRect( rect, nFlags, list );
            dc.DrawDragRect( rect, sizeEdge, NULL, sizeEdge );    //draw
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            bDone = TRUE;
            break;

        case WM_KEYDOWN:
            if( LOWORD( msg.wParam ) == VK_ESCAPE )
            {
                SelectAll( FALSE );
                bDone = TRUE;
                break;
            }
            //dispatch

        default:
            ::DispatchMessage( &msg );
            break;
        }
    }
    KillTimer( nTimer );
    ::ReleaseCapture();

    if( bDrag )
        dc.DrawDragRect( rect, sizeEdge, NULL, sizeEdge );
    else
        if( !( nFlags & ( MK_CONTROL | MK_SHIFT ) ) )
            SelectAll( FALSE );

    //construct notification info
    NMHDR hdr;
    hdr.hwndFrom = m_hWnd;
    hdr.idFrom = ::GetDlgCtrlID( m_hWnd );
    hdr.code = bLeft ? NM_CLICK : NM_RCLICK;
    _SendNotify( &hdr );

    m_nAnimatedObjects = nAnimatedObjects;
    UpdateAnimationStatus();

    //when banding, make sure we receive WM_CONTEXTMENU
    // every time - which is what the listview ctrl does
    ::DispatchMessage( &msg );
}

void CExtTreeCtrl::UpdateSelectionForRect( LPCRECT pRect, UINT nFlags, CTreeItemList& list )
{
    CRect rect;
    BOOL bSel;
    POSITION pos;

    HTREEITEM hItem = GetRootItem();
    while( hItem )
    {
        bSel = IsSelected( hItem );
        GetItemRect( hItem, &rect, m_bBandLabel );
        if( rect.IntersectRect( rect, pRect ) )
        {
            //item in rect
            pos = list.Find( hItem );
            if( !bSel && !pos )
                SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
            else if( _bCtrl && pos )
                SetItemState( hItem, 0, TVIS_SELECTED );
            else if( _bShift && pos )
                list.RemoveAt( pos );     //if shift and in rect, don't lock anymore
        }
        else
        {
            //item not in rect
            pos = list.Find( hItem );
            if( bSel && !pos )
                SetItemState( hItem, 0, TVIS_SELECTED );
            else if( pos )
                SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
        }
        hItem = GetNextVisibleItem( hItem );
    }
    UpdateWindow();
}

void CExtTreeCtrl::SelectAllIgnore( bool bSelect, HTREEITEM hIgnore )
{
    //special case to avoid multiple notifications for
    // the same item
    bSelect = !!bSelect;    //ensure 0 or 1
    UINT nState = bSelect ? TVIS_SELECTED : 0;
    HTREEITEM hItem = GetRootItem();
    while( hItem )
    {
        if( hItem != hIgnore )
            if( IsSelected( hItem ) != bSelect )
                SetItemState( hItem, nState, TVIS_SELECTED );
        hItem = GetNextVisibleItem( hItem );
    }
}

BOOL CExtTreeCtrl::_SendNotify( LPNMHDR pNMHDR )
{
    ASSERT( ::GetParent( m_hWnd ) );    //never expected this

    bool b = m_bEmulated;
    m_bEmulated = true;
    bool bRes = GetParent()->SendMessage( WM_NOTIFY, ( WPARAM )pNMHDR->idFrom,
        ( LPARAM )pNMHDR ) != 0;
    m_bEmulated = b;
    return bRes;
}

TreeItemData* CExtTreeCtrl::GetCustomItemData( HTREEITEM hItem ) const
{
    TreeItemData* pData = NULL;
    if( !m_mapItemData.Lookup( hItem, pData ) )
        return NULL;

    return pData;
}

bool CExtTreeCtrl::ItemExists( HTREEITEM hItem ) const
{
    TVITEM item = { 0 };
    item.hItem = hItem;
    item.mask = TVIF_PARAM;
    return ::SendMessage( m_hWnd, TVM_GETITEM, 0, ( LPARAM )&item ) != 0;
}

TreeItemData* CExtTreeCtrl::CreateCustomItemData( HTREEITEM hItem )
{
    TreeItemData* pData = new TreeItemData;
    m_mapItemData[ hItem ] = pData;
    return pData;
}

void CExtTreeCtrl::OnLButtonDown( UINT nStatus, CPoint pnt )
{
    OnButtonDown( TRUE, nStatus, pnt );	
}


void CExtTreeCtrl::OnRButtonDown( UINT nStatus, CPoint pnt )
{	
    OnButtonDown( FALSE, nStatus, pnt );
	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];
	BOOL bConnStatus = FALSE;
	int nBockMode = 0;
	long n_StatusFlags = 0l;
	UINT nHF;	
	UINT nCnt = GetSelectedCount();
	HTREEITEM hItem = HitTest( pnt, &nHF );

    if( hItem )
    {
		GetItemUserInfo(hItem, szSelItemIP,szSelItemID, szNickName,szRealName, szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
		TRACE("■ OnRButtonDown : 선택된 아이템 [%s]\n", szSelItemIP);				
	}
	
		
	if( nCnt == 0 ||  hItem == NULL) 
	{
		// 빈공간
		CPoint pos;
		CMenu theMenu;
		CWnd* pTarget = AfxGetMainWnd();
		CMenu menu;
		theMenu.LoadMenu(IDR_RIGHT_GRP_MENU);
		CMenu *subMenu = (CMenu*)theMenu.GetSubMenu(0);
		GetCursorPos(&pos);
		menu.LoadMenu(IDR_RIGHT_GRP_MENU);
		::SetMenuDefaultItem(menu, 0, TRUE);

		subMenu->EnableMenuItem(ID_GRP_MNG_MODIFY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		// 그룹 삭제, 대화상대 추가 , 공지, 파일전송 는 비활성화 
		subMenu->EnableMenuItem(ID_GRP_MNG_DEL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);				
		subMenu->EnableMenuItem(ID_GRP_MNG_ADDCOMP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);				
		subMenu->EnableMenuItem(ID_GRP_MNG_ALLMSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		
		subMenu->EnableMenuItem(ID_GRP_MNG_SENDFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		
		
		pTarget->SetForegroundWindow(); 
		::TrackPopupMenu(subMenu->GetSafeHmenu(), 0, pos.x, pos.y, 0, pTarget->GetSafeHwnd(), NULL);
		pTarget->PostMessage(WM_NULL, 0, 0);
	}
	else
	{
		if( strncmp(szSelItemIP,"GRP",3)==0 )
		{
			//Grp_node경우
			CPoint pos;
			CMenu theMenu;
			CWnd* pTarget = AfxGetMainWnd();
			CMenu menu;
			theMenu.LoadMenu(IDR_RIGHT_GRP_MENU);
			CMenu *subMenu = (CMenu*)theMenu.GetSubMenu(0);
			GetCursorPos(&pos);
			menu.LoadMenu(IDR_RIGHT_GRP_MENU);
			::SetMenuDefaultItem(menu, 0, TRUE);
			
			//그룹 선택시는 그룹 추가 메뉴 비활성화 
			subMenu->EnableMenuItem(ID_GRP_MNG_ADD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

			if(nCnt > 1 )
			{
				//여러 그룹 선택시 대화상대 추가메뉴 비활성화.
				subMenu->EnableMenuItem(ID_GRP_MNG_ADDCOMP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

				//여러 그룹 선택시 그룹명 변경 메뉴 비활성화.
				subMenu->EnableMenuItem(ID_GRP_MNG_MODIFY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

				//여러 그룹 선택시 그룹에 공지보내기 메뉴 비활성화.
				subMenu->EnableMenuItem(ID_GRP_MNG_ALLMSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

				//여러 그룹 선택시 그룹에 파일보내기 메뉴 비활성화.
				subMenu->EnableMenuItem(ID_GRP_MNG_SENDFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			}
			else
			{
				if( strcmp(szGrpName, "인터넷 즐겨찾기") == 0)
				{					
					//즐겨찾기그룹 대화상대 추가메뉴 비활성화.
					subMenu->EnableMenuItem(ID_GRP_MNG_ADDCOMP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					//즐겨찾기 그룹 그룹명 변경 메뉴 비활성화.
					subMenu->EnableMenuItem(ID_GRP_MNG_MODIFY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					//즐겨찾기 그룹 그룹에 공지보내기 메뉴 비활성화.
					subMenu->EnableMenuItem(ID_GRP_MNG_ALLMSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					//즐겨찾기 그룹 그룹에 파일보내기 메뉴 비활성화.
					subMenu->EnableMenuItem(ID_GRP_MNG_SENDFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

					//즐겨찾기 그룹 삭제 메뉴 비활성화.
					subMenu->EnableMenuItem(ID_GRP_MNG_DEL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
			
			pTarget->SetForegroundWindow(); 
			::TrackPopupMenu(subMenu->GetSafeHmenu(), 0, pos.x, pos.y, 0, pTarget->GetSafeHwnd(), NULL);
			pTarget->PostMessage(WM_NULL, 0, 0);
		}
		else
		{			
			// Child Node경우
			CPoint pos;
			CMenu theMenu;
			CWnd* pTarget = AfxGetMainWnd();
			CMenu menu;
			CMenu *subMenu;
			
			if(nCnt < 2)
			{
				if(nBockMode == 9)
				{
					theMenu.LoadMenu(IDR_MENU_ITCOMP);
					subMenu = (CMenu*)theMenu.GetSubMenu(0);
					GetCursorPos(&pos);
					menu.LoadMenu(IDR_MENU_ITCOMP);
					::SetMenuDefaultItem(menu, 0, TRUE);
				}
				else
				{
					theMenu.LoadMenu(IDR_RIGHT_CLICK_MENU);
					subMenu = (CMenu*)theMenu.GetSubMenu(0);
					GetCursorPos(&pos);
					menu.LoadMenu(IDR_RIGHT_CLICK_MENU);
					::SetMenuDefaultItem(menu, 0, TRUE);

					// 대화상대 2명이상 아니면 공지사항, 멀티Chat메뉴 비활성화
					subMenu->EnableMenuItem(IDRCLICK_ALLMSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					subMenu->EnableMenuItem(IDRCLICK_MULTICHAT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);	//2명

					//OnLine경우는 쪽지 남기기 비활성화
					if(bConnStatus == 1)
					{
						subMenu->EnableMenuItem(IDRCLICK_WRITE_MSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);				
					}			
				}
			}
			else
			{
				CTreeItemList itemList;
				char szSelIDs[1024];
				BOOL bNotItComp = FALSE;
				memset(szSelIDs , 0x00, sizeof(szSelIDs));

				GetSelectedList( itemList );
				int nCnt = itemList.GetCount();
				
				POSITION pos1 = itemList.GetHeadPosition();
				for (int i=0;i < nCnt; i++)
				{
					HTREEITEM hSel = itemList.GetNext(pos1);		
					
					bConnStatus = FALSE;
					nBockMode = 0;
					n_StatusFlags = 0l;

					GetItemUserInfo(hSel, szSelItemIP, szSelItemID, szNickName, szRealName, szGrpName, 
												bConnStatus,&nBockMode,&n_StatusFlags);

					TRACE("■ 선택된 nBockMode [%d]\n", nBockMode);
				
					if(nBockMode != 9)
					{
						bNotItComp = TRUE;
						break;
					}
				}

				if(bNotItComp == TRUE)
				{
					theMenu.LoadMenu(IDR_RIGHT_CLICK_MENU);
					subMenu = (CMenu*)theMenu.GetSubMenu(0);
					GetCursorPos(&pos);
					menu.LoadMenu(IDR_RIGHT_CLICK_MENU);
					::SetMenuDefaultItem(menu, 0, TRUE);
					
					//쪽지 남기기 비활성화
					subMenu->EnableMenuItem(IDRCLICK_WRITE_MSG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
				else
				{
					// 즐겨찾기만 선택한 경우와, 구분처리 필요
					theMenu.LoadMenu(IDR_MENU_ITCOMP);
					subMenu = (CMenu*)theMenu.GetSubMenu(0);
					GetCursorPos(&pos);
					menu.LoadMenu(IDR_MENU_ITCOMP);
					::SetMenuDefaultItem(menu, 0, TRUE);

					// 여러 상대 선택시는 변경 비활성화
					subMenu->EnableMenuItem(IDRCLICK_ITCOMP_MODIFY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}			

			pTarget->SetForegroundWindow(); 
			::TrackPopupMenu(subMenu->GetSafeHmenu(), 0, pos.x, pos.y, 0, pTarget->GetSafeHwnd(), NULL);
			pTarget->PostMessage(WM_NULL, 0, 0);			
		}
	}	
}

void CExtTreeCtrl::OnSetFocus( CWnd *pPrevWnd )
{
    DefWindowProc( WM_SETFOCUS, ( WPARAM )pPrevWnd->GetSafeHwnd(), 0 );
    if( m_bMulti )
    {
        //'emulated' selected items will remain greyed
        // if application gets covered
        HTREEITEM hItem = GetFirstSelectedItem();
        RECT rect;
        while( hItem )
        {
            GetItemRect( hItem, &rect, TRUE );
            InvalidateRect( &rect );
            hItem = GetNextSelectedItem( hItem );
        }
    }
}

void CExtTreeCtrl::OnKillFocus( CWnd *pNewFocus )
{
    DefWindowProc( WM_KILLFOCUS, ( WPARAM )pNewFocus->GetSafeHwnd(), 0 );
    if( m_bMulti )
    {
        //'emulated' selected items may not get
        // refreshed to grey
        HTREEITEM hItem = GetFirstSelectedItem();
        RECT rect;
        while( hItem )
        {
            GetItemRect( hItem, &rect, TRUE );
            InvalidateRect( &rect );
            hItem = GetNextSelectedItem( hItem );
        }
    }
}

void CExtTreeCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if( !m_bMulti )
    {
		CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
        return;
    }

    BOOL bCtrl = ( GetKeyState( VK_CONTROL ) & 0x8000 );
    BOOL bShift = ( GetKeyState( VK_SHIFT ) & 0x8000 );

    BOOL bDir = FALSE;
    HTREEITEM hSel = NULL;
    switch( nChar )
    {
    case VK_UP:
        bDir = TRUE;

    case VK_DOWN:
        //common
        hSel = GetSelectedItem();
        if( !m_hSelect )
            m_hSelect = hSel;

        if( !bCtrl && !bShift )
        {
            m_hSelect = NULL;   //reset
            SelectAll( FALSE );
        }
        break;
        //      case VK_SPACE:
        //          hSel = GetSelectedItem();
        //          if (hSel) {
        //              BOOL b = IsSelected(hSel);
        //              if (bCtrl)
        //                  SetItemState(hSel, b ? 0 : TVIS_SELECTED, TVIS_SELECTED);
        //              else if (!b)
        //                  SetItemState(hSel, TVIS_SELECTED, TVIS_SELECTED);
        //          }
        //          return;     //don't call base class (it would start a search on the char)
        //TODO: the text search isn't stopped by this ^.  It may be done in the TranslateMessage,
        // so would have to trap PreTranslateMessage to avoid it.  If 'space' selection is
        // required then need to implement - I'm not bothered.
    }

    DefWindowProc( WM_KEYDOWN, nChar, MAKELPARAM( nRepCnt, nFlags ) );
    if( !hSel || ( !bCtrl && !bShift ) )
        return;

    HTREEITEM hNext = bDir ? GetPrevVisibleItem( hSel ) : GetNextVisibleItem( hSel );
    if( !hNext )
        hNext = hSel;
    if( bShift )
        SelectRange( m_hSelect, hNext, TRUE );
    else if( bCtrl )
        SetItemState( hNext, TVIS_FOCUSED, TVIS_FOCUSED );
}

void CExtTreeCtrl::OnLButtonDblClick( UINT nFlags, CPoint pnt )
{
    UINT uFlags;
    HTREEITEM hItem = HitTest( pnt, &uFlags );
    if( hItem )
    {
        if( uFlags & TVHT_ONITEM )
            Expand( hItem, TVE_TOGGLE );
    }
}

void CExtTreeCtrl::OnItemExpanding( NMHDR *pNotifyStruct, LRESULT* result )
{
	*result = 0;
	if( m_bMulti )
	{
		LPNMTREEVIEW pNMTreeView = ( LPNMTREEVIEW )pNotifyStruct;
		if( ( pNMTreeView->action == TVE_COLLAPSE ) || ( pNMTreeView->action == TVE_COLLAPSERESET ) )
		{
			//clear selection of children, it would be confusing otherwise
			// - the notifications can be over-ridden to stop the de-selection
			// if required
			//unfortunately, the parent window can't over-ride this functionality
			// because MFC gives this class first crack.  So if changes are required
			// a derived class will have to be used..
			ASSERT( pNMTreeView->itemNew.hItem );

			//if a descendent item has focus the parent will get selected as a
			// consequence of collapsing the tree, so preserve
			// (if the parent was already selected it will gain focus, but
			//  that's acceptable)
			BOOL bWasSel = IsSelected( pNMTreeView->itemNew.hItem );
			BOOL bWasFocus = SelectChildren( pNMTreeView->itemNew.hItem, FALSE, TRUE );
			if( bWasFocus && !bWasSel )
				CTreeCtrl::SelectItem( NULL );  //stop parent from gaining selection
		}
	}
}

void CExtTreeCtrl::OnCustomDraw( NMHDR *pNotifyStruct, LRESULT* result )
{
	LPNMTVCUSTOMDRAW pTVCustomDraw = reinterpret_cast< LPNMTVCUSTOMDRAW >( pNotifyStruct );

    *result = CDRF_NOTIFYITEMDRAW;

    HTREEITEM hItem = reinterpret_cast< HTREEITEM >( pTVCustomDraw->nmcd.dwItemSpec );

    switch( pTVCustomDraw->nmcd.dwDrawStage )
    {
    case CDDS_PREPAINT:
        *result = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_PREERASE:
        *result = CDRF_SKIPDEFAULT;
        break;

    case CDDS_ITEMPREPAINT:
        {
            CDC dc;
			dc.Attach( pTVCustomDraw->nmcd.hdc );

			CDC MemDC;
            BOOL bRes = MemDC.CreateCompatibleDC( &dc );

            CBitmap MemBmp;
            CRect rc( pTVCustomDraw->nmcd.rc );
            MemBmp.CreateCompatibleBitmap( &dc, rc.Width(), rc.Height() );

            HGDIOBJ hOldBmp = MemDC.SelectObject( MemBmp );

            //MemDC.BitBlt( 0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCCOPY );
            CBrush Filler;
            Filler.CreateSolidBrush( GetBkColor() );
            MemDC.FillRect( CRect( 0, 0, rc.Width(), rc.Height() ), &Filler );

            PaintItem( hItem, MemDC, CRect( 0, 0, rc.Width(), rc.Height() ) );
            CSize sizeOffset( rc.left, rc.top );
            TreeItemPositionInfo& tipi = m_mapPositionCache[ hItem ];
            tipi.rectButton.OffsetRect( sizeOffset );
            tipi.rectLabel.OffsetRect( sizeOffset );
            tipi.rectLeftIcons.OffsetRect( sizeOffset );
            tipi.rectRightIcons.OffsetRect( sizeOffset );

            dc.BitBlt( rc.left, rc.top, rc.Width(), rc.Height(), &MemDC, 0, 0, SRCCOPY );

            MemDC.SelectObject( hOldBmp );
			
			dc.Detach();
        }
        *result = CDRF_SKIPDEFAULT;
        break;
    }
}

void CExtTreeCtrl::PaintItem( HTREEITEM hItem, HDC hDC, CRect rect )
{
    CDC dc;
	dc.Attach( hDC );

    TreeItemData* pItemData = GetCustomItemData( hItem );
	m_mapPositionCache[ hItem ];
    TreeItemPositionInfo tipi;
	m_mapPositionCache.Lookup( hItem, tipi );

    TCHAR tstrBuf[ 255 ];
    TVITEM item;
    item.hItem = hItem;
    item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
    item.pszText = tstrBuf;
    item.cchTextMax = 255;
    GetItem( &item );

    COLORREF color = ( pItemData != NULL ) ? pItemData->itemColor : RGB( 0, 0, 0 ); 
    bool bBold = ( pItemData != NULL ) ? pItemData->bBold : false;
    if( item.state & TVIS_BOLD )
        bBold = true;
    bool bItalic = ( pItemData != NULL ) ? pItemData->bItalic : false;
    CFont* aFontMap[2][2] = { { m_pDefaultItemFont, m_pBoldItemFont },
    { m_pItalicItemFont, m_pBoldItalicItemFont } };
    CFont* pFont = aFontMap[ bItalic ][ bBold ];

    bool bFocused = GetFocus()->GetSafeHwnd() == GetSafeHwnd();

    int nItemOffset = -GetScrollPos( SB_HORZ );
    tipi.nItemWidth = -nItemOffset;
    HTREEITEM hIter = hItem;
    while( hIter != NULL && hIter != TVI_ROOT )
    {
        TreeItemData* pData = GetCustomItemData( hIter );
        if( pData != NULL )
            nItemOffset += pData->nIndent;
        else
            nItemOffset += m_nDefaultIndent;

        hIter = GetParentItem( hIter );
    }

    ItemState state = isLeaf;
    if( GetChildItem( hItem ) != NULL )
    {
        if( ( item.state & TVIS_EXPANDED ) != 0 )
            state = isExpanded;
        else
            state = isCollapsed;
    }

    CBitmapHandle* aBitmapMap[] = { &m_bmpEmpty, &m_bmpExpanded, &m_bmpCollapsed };
    CBitmapHandle* pbmpState = aBitmapMap[ state ];

    HGDIOBJ hOldFont = NULL;
    if( pFont != NULL )
        hOldFont = dc.SelectObject( pFont );

    COLORREF BackColor;
    if( item.state & TVIS_SELECTED )
    {
        if( bFocused )
            BackColor = ( m_SelectionColor != DefaultColor ) ? m_SelectionColor :                
				::GetSysColor( COLOR_HIGHLIGHT ); 
        else
            BackColor = ( m_SelectionNonFocusedColor != DefaultColor ) ? m_SelectionNonFocusedColor :                
				::GetSysColor( COLOR_INACTIVECAPTIONTEXT ); 
    }
    else
        BackColor = GetBkColor();

    dc.SetBkColor( BackColor );

    if( pbmpState != NULL && pbmpState->m_hBitmap != NULL )
    {
        CDC BmpDC;
        BmpDC.CreateCompatibleDC( &dc );
        HGDIOBJ hOldBmp = BmpDC.SelectObject( pbmpState->m_hBitmap );

        CSize size;
        pbmpState->GetSize( size );

        dc.BitBlt( nItemOffset + rect.left - size.cx - m_nElementGap, rect.top, 
            rect.Width() - size.cx - m_nElementGap, rect.bottom, &BmpDC, 0, 0, SRCCOPY );

        BmpDC.SelectObject( hOldBmp );

        tipi.rectButton = CRect( nItemOffset + rect.left - size.cx - m_nElementGap, rect.top, 
            rect.Width() - size.cx - m_nElementGap, rect.bottom );
    }

    bool bSwitchFrame = false;
    int nVal;
    if( m_mapNewFrames.Lookup( hItem, nVal ) )
    {
        bSwitchFrame = true;
        m_mapNewFrames.RemoveKey( hItem );
    }

    // Drawing pre-images
    CRect rectImage = CRect( 0, 0, 0, 0 );
    if( pItemData != NULL )
    {
        POSITION pos = pItemData->aPreImages.GetHeadPosition();

        while( pos != NULL )
        {
            TreeItemImage* pImage = pItemData->aPreImages.GetNext( pos );
            pImage->RenderItem( dc, CPoint( rect.left + nItemOffset, rect.top ), bSwitchFrame );

            rectImage.UnionRect( rectImage, CRect( CPoint( rect.left + nItemOffset, rect.top ),
                pImage->GetSize() ) );

            nItemOffset += pImage->GetSize().cx + m_nElementGap;
        }
    }

    tipi.rectLeftIcons = rectImage;

    // Drawing text
    // 1. Measuring text
    if( item.state & TVIS_SELECTED )
    {
        if( bFocused )
            dc.SetTextColor( m_SelectedItemColor != DefaultColor ? m_SelectedItemColor : DefaultSelectedItemColor );
        else
            dc.SetTextColor( m_SelectedNonFocusedItemColor != DefaultColor ? m_SelectedNonFocusedItemColor : DefaultSelectedNonFocusedItemColor );
    }
    else
    {
        if( hItem != m_hTrackSelection )
            dc.SetTextColor( color );
        else
		{
            dc.SetTextColor( m_TrackSelectionColor != DefaultColor ? m_TrackSelectionColor :
                GetSysColor( COLOR_HOTLIGHT ) ); 
			m_CurHotItem =  hItem;
		}
    }

    CRect rectText( rect.left + nItemOffset, rect.top , 0, 0  );
    dc.DrawText( item.pszText, ( int )_tcslen( item.pszText ), &rectText, DT_LEFT | DT_VCENTER | DT_CALCRECT ); //DT_SINGLELINE kojh
    rectText.right += g_nLabelLeadIn + g_nLabelLeadOut;
    tipi.rectLabel = rectText;

    rectText.top = rect.top;
    rectText.bottom = rect.bottom;

    // 2. Drawing text
    CBrush Filler;
    Filler.CreateSolidBrush( BackColor );
    dc.FillRect( rectText, &Filler );

    rectText.left += g_nLabelLeadIn;

    dc.SetBkMode( TRANSPARENT );
    dc.DrawText( item.pszText, ( int )_tcslen( item.pszText ), &rectText, DT_LEFT | DT_VCENTER );
    dc.SetBkMode( OPAQUE );

    nItemOffset += rectText.Width() + m_nElementGap;

    // Drawing post-images
    rectImage = CRect( 0, 0, 0, 0 );
    if( pItemData != NULL )
    {
        POSITION pos = pItemData->aPostImages.GetHeadPosition();

        while( pos != NULL )
        {
            TreeItemImage* pImage = pItemData->aPostImages.GetNext( pos );
            pImage->RenderItem( dc, CPoint( rect.left + nItemOffset, rect.top), bSwitchFrame );

            rectImage.UnionRect( rectImage, CRect( CPoint( rect.left + nItemOffset, rect.top ),
                pImage->GetSize() ) );

            nItemOffset += pImage->GetSize().cx + m_nElementGap;
        }
    }

    tipi.rectRightIcons = rectImage;
    tipi.nItemWidth += nItemOffset;

	dc.SelectObject( hOldFont );

	dc.Detach();

	m_mapPositionCache[ hItem ] = tipi;
}

void CExtTreeCtrl::UpdateHorzScroller()
{
    POSITION pos = m_mapPositionCache.GetStartPosition();

    int nMaxWidth = 0;
    while( pos != NULL )
    {
        HTREEITEM hItem = NULL; 
		TreeItemPositionInfo tipi;
		m_mapPositionCache.GetNextAssoc( pos, hItem, tipi );

		int nWidth = tipi.nItemWidth;

		CRect rect;
		if( GetItemRect( hItem, &rect, FALSE ) != 0 )
			nMaxWidth = max( nMaxWidth, nWidth );
    }

    INT nMaxPos, nMinPos;
    GetScrollRange( SB_HORZ, &nMinPos, &nMaxPos );
    CRect rect;
    GetClientRect( &rect );
    
    ShowScrollBar( SB_HORZ, m_bHorzScrollerVisible = ( rect.Width() < nMaxWidth ) );
    
    if( nMaxWidth != 0 && rect.Width() < nMaxWidth /*&& nMaxPos != nMaxWidth */)
	{
		SCROLLINFO si;
		si.cbSize = sizeof( si );
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = nMaxWidth;
		si.nPage = rect.Width();
        SetScrollInfo( SB_HORZ, &si );
	}
}

HTREEITEM CExtTreeCtrl::HitTest( TVHITTESTINFO* pHitTestInfo ) const
{
	pHitTestInfo->hItem = HitTest( pHitTestInfo->pt, &pHitTestInfo->flags );
	return pHitTestInfo->hItem;
}

HTREEITEM CExtTreeCtrl::HitTest( CPoint pt, UINT* pFlags ) const
{
    CRect rectClient;
    GetClientRect( rectClient );

	HTREEITEM hItem = NULL;

	if( pFlags != NULL )
		*pFlags = TVHT_NOWHERE;
    if( ( pFlags != NULL ) && !rectClient.PtInRect( pt ) )
    {
        if( rectClient.left > pt.x )
            *pFlags |= TVHT_TOLEFT;
        if( rectClient.top > pt.y )
            *pFlags |= TVHT_ABOVE;
        if( rectClient.right > pt.x )
            *pFlags |= TVHT_TORIGHT;
        if( rectClient.bottom > pt.y )
            *pFlags |= TVHT_BELOW;
    }
    else
    {
        HTREEITEM hCurItem = GetFirstVisibleItem();
        while( hCurItem != NULL )
        {
            CRect rect;
			CTreeCtrl::GetItemRect( hCurItem, rect, FALSE );

            if( rect.PtInRect( pt ) )
            {
                hItem = hCurItem;
                break;
            }

            hCurItem = GetNextVisibleItem( hCurItem );
        }

        if( hItem == NULL )
		{
			if( pFlags != NULL )
				*pFlags = TVHT_NOWHERE;
		}
        else
        {
            // Item found. Detecting exact hit type
            TreeItemPositionInfo tipi;
			if( m_mapPositionCache.Lookup( hItem, tipi ) == TRUE )
			{
				if( hItem == NULL && pFlags != NULL )
					*pFlags = TVHT_ONITEMRIGHT;

				if( pFlags != NULL )
				{
					if( tipi.rectButton.left < pt.x &&
						tipi.rectButton.right > pt.x )
						*pFlags = TVHT_ONITEMBUTTON;
					if( tipi.rectLabel.left < pt.x &&
						tipi.rectLabel.right > pt.x )
						*pFlags = TVHT_ONITEMLABEL;
					if( tipi.rectLeftIcons.left < pt.x &&
						tipi.rectLeftIcons.right > pt.x )
						*pFlags = TVHT_ONITEMICON;
					if( tipi.rectRightIcons.left < pt.x &&
						tipi.rectRightIcons.right > pt.x )
						*pFlags = TVHT_ONITEMICON;
					if( tipi.rectButton.left > pt.x )
						*pFlags = TVHT_ONITEMINDENT;
				}
			}
        }
    }

    return hItem;
}

void CExtTreeCtrl::OnHScroll( UINT nScrollType, UINT nPos, CScrollBar* pScrollBar )
{
    if( !m_bHorzScrollerVisible )
        return;

    int nCurPos = GetScrollPos( SB_HORZ );

    SCROLLINFO si;
    si.cbSize = sizeof( si );
    si.fMask = SIF_ALL;
    GetScrollInfo( SB_HORZ, &si );

    int nOffset;
    switch( nScrollType )
    {
    case SB_LINERIGHT:
        nOffset = 5;
        break;

    case SB_LINELEFT:
        nOffset = -5;
        break;

    case SB_PAGERIGHT:
        nOffset = 225;
        break;

    case SB_PAGELEFT:
        nOffset = -225;
        break;

    case SB_THUMBTRACK:
        nOffset = nPos - nCurPos;
        break;

    default:
        nOffset = 0;
    }

    if( nOffset != 0 )
    {
        if( nCurPos + nOffset > si.nMax - ( int )si.nPage )
            nOffset = si.nMax - ( int )si.nPage - nCurPos;
        if( nCurPos + nOffset < si.nMin )
            nOffset = si.nMin - nCurPos;

        SetScrollPos( SB_HORZ, nCurPos + nOffset );

        ScrollWindow( -nOffset, 0 );

        // Updating item position cache
        POSITION pos = m_mapPositionCache.GetStartPosition();

        while( pos != NULL )
        {
            TreeItemPositionInfo tipi;
			HTREEITEM hItem = NULL;
			m_mapPositionCache.GetNextAssoc( pos, hItem, tipi );
            tipi.rectButton.OffsetRect( CPoint( -nOffset, 0 ) );
            tipi.rectLabel.OffsetRect( CPoint( -nOffset, 0 ) );
            tipi.rectLeftIcons.OffsetRect( CPoint( -nOffset, 0 ) );
            tipi.rectRightIcons.OffsetRect( CPoint( -nOffset, 0 ) );
			m_mapPositionCache[ hItem ] = tipi;
        }
    }

	return;
    //return nOffset != 0;
}

void CExtTreeCtrl::OnTimer( UINT idEvent )
{
    if( idEvent == m_uAnimationTimer )
    {
        POSITION pos = m_mapAnimations.GetStartPosition();

        while( pos != 0 )
        {
            HTREEITEM hItem;
            int nCount;
            m_mapAnimations.GetNextAssoc( pos, hItem, nCount );

            if( nCount != 0 )
            {
                CRect rect;
                if( GetItemRect( hItem, rect, FALSE ) != 0 )
                {
                    m_mapNewFrames[ hItem ] = 0;
                    InvalidateRect( rect, FALSE );
                }
            }

        }
    }
    else
		CTreeCtrl::OnTimer( idEvent );
}

BOOL CExtTreeCtrl::OnSetCursor( CWnd *pWnd, UINT uHitTest, UINT uMouseMsg )
{
    HTREEITEM hOldSelection = m_hTrackSelection;

    if( GetStyle() & TVS_TRACKSELECT ||
        GetStyle() & TVS_TRACKGROUPSELECT )
    {
        CPoint pnt;
        GetCursorPos( &pnt );
        ScreenToClient( &pnt );

        UINT uFlags;
        HTREEITEM hItem = HitTest( pnt, &uFlags );

        bool bTrack = false;
        if( hItem != NULL )
        {
			// all hot
			m_hTrackSelection = hItem; //kojh

			// 커서는 parent 인것만 kojh
            bool bIsParent = GetChildItem( hItem ) != NULL;
            bTrack = ( !( GetStyle() & TVS_TRACKGROUPSELECT ) || bIsParent )     && uFlags & TVHT_ONITEM;
        }		

        if( bTrack )
        {
            //m_hTrackSelection = hItem; ////kojh
            SetCursor( LoadCursor( NULL, IDC_HAND ) );
        }
        else
        {
            //m_hTrackSelection = NULL; ////kojh
			// 즐겨찾기는 원클릭으로 실행되게 
			TreeItemData* pData = GetCustomItemData( hItem );    
			
			if(pData && pData->m_nBockMode == 9)
				SetCursor( LoadCursor( NULL, IDC_HAND ) );
			else
				SetCursor( LoadCursor( NULL, IDC_ARROW ) );
        }

        if( m_hTrackSelection != hOldSelection )
        {
            CRect rect;
            if( GetItemRect( m_hTrackSelection, rect, TRUE ) != 0 )
                InvalidateRect( rect );
            if( GetItemRect( hOldSelection, rect, TRUE ) != 0 )
                InvalidateRect( rect );
        }

        return TRUE;
    }
    else
        return FALSE;
}

BOOL CExtTreeCtrl::GetItemRect( HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly ) const
{
    TreeItemPositionInfo tipi;
    if( m_mapPositionCache.Lookup( hItem, tipi ) )
    {
        CRect rect;
        GetClientRect( &rect );

        *lpRect = tipi.rectLabel;
        if( bTextOnly == FALSE )
        {
            lpRect->left = 0;
            lpRect->right = rect.Width();
        }

        // Final visibility check
        CRect tmp;
		if( CTreeCtrl::GetItemRect( hItem, &tmp, bTextOnly ) != 0 && tmp.IntersectRect( tmp, rect ) != 0 )
            return TRUE;
    }

    *lpRect = CRect( 0, 0, 0, 0 );
    return FALSE;
}

void CExtTreeCtrl::SetSelectedItemColor( COLORREF colorFocused, COLORREF colorNonFocused )
{
    m_SelectedItemColor = colorFocused;
    m_SelectedNonFocusedItemColor = colorNonFocused;
    Invalidate();
}

void CExtTreeCtrl::OnPaint()
{
    CRect rc;
    GetUpdateRect( &rc, FALSE );
    ValidateRect( &rc );
    InvalidateRect( &rc, FALSE );

    PAINTSTRUCT ps;
    CDC *pDC = BeginPaint( &ps );

    CDC dcOffscreen;
    dcOffscreen.CreateCompatibleDC( pDC );

    CBitmap bmp;
    // This bitmap may be shrinked down to rect.Width() x rect.Height()
    bmp.CreateCompatibleBitmap( pDC, ps.rcPaint.right, ps.rcPaint.bottom );

    HGDIOBJ hOldBmp = dcOffscreen.SelectObject( bmp );

    DefWindowProc( WM_PAINT, ( WPARAM )dcOffscreen.m_hDC, 0 );

    pDC->BitBlt( ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
        ps.rcPaint.bottom - ps.rcPaint.top, &dcOffscreen, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY );

    dcOffscreen.SelectObject( hOldBmp );

    EndPaint( &ps );

	UpdateHorzScroller();
}

LRESULT CExtTreeCtrl::OnEraseBackground( HDC hDC )
{
    return 1;
}


void CExtTreeCtrl::OnDestroy()
{
	DeleteAllItems();

	CTreeCtrl::OnDestroy();
}

void CExtTreeCtrl::OnDropFiles(HDROP hDropInfo) 
{
	BOOL bWorking = FALSE;
	char szFileName[2048];
	memset(szFileName, 0x00, sizeof(szFileName));	
	m_strFileAll = ""; //init;
	m_nFileCnt = 0;
	CString strRootPath = "";
	int nFiles;
	
	// 드롭된 파일의 갯수
	// 파일 사이즈가 0 인것은 제외해야함..
	nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, szFileName, 2048);
	
	CIdmsMsnDlg *pDlg = (CIdmsMsnDlg *)GetParent();	
	
	char szSelItemIP	[15 + 1];	
	char szSelItemID    [20 + 1];
	char szRealName		[20 + 1];
	char szNickName		[C_NICK_NAME_LEN +1];
	char szGrpName      [C_GRP_NAME_LEN +1];

	BOOL bConnStatus = FALSE;	
	int nBockMode = 0;
	long n_StatusFlags = 0l;

	TRACE("■ CExtTreeCtrl::OnDropFiles 선택된 아이템 [%s]\n", m_szSelItemIP);

	if(nFiles == 1)
	{
		//파일하나혹은 폴더한개만

		CTreeItemList itemList;
		GetSelectedList( itemList );	
		int nCnt = itemList.GetCount();
		
		// 1) 사용자가 선택된 상태에서 파일을 마우스 Drag & Drop 으로 , 선택 안된 사용자에게 떨구었을 경우는 
		// => 선택안된 사용자 + 선택한 사용자 모두에게 전송함..
        
		// 2) 20061129 선택 사용자가 1명만 선택된 경우 &  선택 안된 사용자에게 떨구었을 경우는 
		// => 선택안된 사용자에게 전송함. (즉 사용자가 여러명을 선택한 경우만 1 로 처리한다)

		DragQueryFile(hDropInfo, 0, szFileName, 2048);		
		CFileFind fileFind;	
		bWorking = fileFind.FindFile(szFileName);	
		if(bWorking )
		{
			bWorking = fileFind.FindNextFile();		
			
			{
				// C:\\TEST\\DOC 라면 C:\\TEST 를 ROOT 로 한다.		
				CString strTmpPath = szFileName;
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll
			}
		}		

		m_strFileAll = strRootPath + "|" + m_strFileAll  ;// RootPath|파일경로		
		
		/////////////////////////////////////////////////////////////////
		if( strlen(m_szSelItemID) > 0) // 마우스 drop한 사용자에게 먼저 전송 
		{
			CBuddyInfo * pCompanion;
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_szSelItemID, ( CObject*& ) pCompanion ) ) 
			{	
				if(pCompanion->m_nYouBlocked == 4)
				{
					char szFmt [100] ;
					memset(szFmt,0x00,sizeof(szFmt));
					sprintf(szFmt, "상대방[%s]이 대화차단한 경우 파일을 전송할수 없습니다", pCompanion->m_szUserName);
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);
					pMsgThread->CreateThread();
				}
				else
				{
					if(pCompanion->m_bConnStatus == TRUE)
					{						
						TRACE("szFileName [%s]\n",m_strFileAll);
									
						CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, 
																					(LPSTR)(LPCSTR)pCompanion->m_szIpAddr );
						
						if( NULL == pSpckObjMap )
						{					
							TRACE("socket connect fail! [%s]\n", m_strFileAll);	
													
							CString strErrMsg;
							strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
							CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
							pMsgThread->CreateThread();
						}
						else
						{				
							TRACE("\nsocket connected! [%s]\n", m_strFileAll);
							
							CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID,
								CString(pCompanion->m_szIpAddr), (LPSTR)(LPCSTR) m_strFileAll , m_nFileCnt) ;
						} 
					}				
					else
					{
						char szFmt [100] ;
						memset(szFmt,0x00,sizeof(szFmt));
						sprintf(szFmt, "OffLine 상대 [%s] 에게 파일을 전송할수 없습니다", pCompanion->m_szUserName);				
					
						CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
						
						pMsgThread->CreateThread();
					}
				}				
			}	
		}

		if(nCnt == 1)
		{
			return;
		}

		// 마우스 drop한 사용자제외후 선택한 사용자들에게 전송 		
		
		TRACE("nCnt [%d]\n", nCnt);	
		POSITION pos = itemList.GetHeadPosition();
		for (int i=0;i < nCnt; i++)
		{
			HTREEITEM hSel = itemList.GetNext(pos);
			
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;

			GetItemUserInfo(hSel, szSelItemIP, szSelItemID , szNickName,szRealName,szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);

			if(strcmp(szSelItemIP, "GRP") != 0 && strcmp(szSelItemID, m_szSelItemID) != 0 )
			{
				if (nBockMode == 4)
				{
					char szFmt [100] ;
					memset(szFmt,0x00,sizeof(szFmt));
					sprintf(szFmt, "상대방[%s]이 대화차단한 경우 파일을 전송할수 없습니다", szRealName);
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
					
					pMsgThread->CreateThread();
				}
				else
				{
					if(bConnStatus == TRUE)
					{						
						TRACE("szFileName [%s]\n",m_strFileAll);
			
						CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)szSelItemIP );
						
						if( NULL == pSpckObjMap )
						{					
							TRACE("socket connect fail! [%s]\n", m_strFileAll);	
													
							CString strErrMsg;
							strErrMsg.Format("%s 에 접속할수 없습니다", szSelItemIP);
							CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
							pMsgThread->CreateThread();
						}
						else
						{				
							TRACE("\nsocket connected! [%s]\n", m_strFileAll);
							
							CFileTransSock::Instance().InformFileSend( pSpckObjMap, szSelItemID,
								CString(szSelItemIP), (LPSTR)(LPCSTR) m_strFileAll , m_nFileCnt) ;
						} 
					}				
					else
					{
						char szFmt [100] ;
						memset(szFmt,0x00,sizeof(szFmt));
						sprintf(szFmt, "OffLine 상대 [%s] 에게 파일을 전송할수 없습니다", szRealName);				
					
						CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
						
						pMsgThread->CreateThread();
					}
				}				
			}

			TRACE("■ 선택된 아이템 [%s]\n", szSelItemIP);			
		} //for
	}
	else
	{	
		// 파일 + 폴더 여러개
		CTreeItemList itemList;
		GetSelectedList( itemList );	
		int nCnt = itemList.GetCount();

		CString strfilesAll;
		CString strfiles;
		for(int index=0 ; index < nFiles ; index++)
		{				
			DragQueryFile(hDropInfo, index, szFileName, 2048);     
			CFileFind fileFind;
			//FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll			
			bWorking = fileFind.FindFile(szFileName);	
			if(bWorking )
			{
				bWorking = fileFind.FindNextFile();		
				
				// C:\\TEST\\DOC 라면 C:\\TEST 를 ROOT 로 한다.		
				CString strTmpPath = szFileName;
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll				
			}			
		}

		m_strFileAll = strRootPath + "|" + m_strFileAll  ; // RootPath|파일경로		

		/////////////////////////////////////////////////////////////////
		if( strlen(m_szSelItemID) > 0)
		{
			CBuddyInfo * pCompanion;
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_szSelItemID, ( CObject*& ) pCompanion ) ) 
			{	
				if (nBockMode == 4)
				{
					char szFmt [100] ;
					memset(szFmt,0x00,sizeof(szFmt));
					sprintf(szFmt, "상대방[%s]이 대화차단한 경우 파일을 전송할수 없습니다", pCompanion->m_szUserName);
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
					
					pMsgThread->CreateThread();
				}
				else
				{
					if(pCompanion->m_bConnStatus == TRUE)
					{						
						TRACE("szFileName [%s]\n",m_strFileAll);
			
						CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr );
						
						if( NULL == pSpckObjMap )
						{					
							TRACE("socket connect fail! [%s]\n", m_strFileAll);	
													
							CString strErrMsg;
							strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
							CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
							pMsgThread->CreateThread();
						}
						else
						{				
							TRACE("\nsocket connected! [%s]\n", m_strFileAll);
							
							CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID,
								CString(pCompanion->m_szIpAddr), (LPSTR)(LPCSTR) m_strFileAll , m_nFileCnt) ;
						} 
					}
					else 
					{
						char szFmt [100] ;
						memset(szFmt,0x00,sizeof(szFmt));
						sprintf(szFmt, "OffLine 상대 [%s] 에게 파일을 전송할수 없습니다", pCompanion->m_szUserName);				
					
						CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
						
						pMsgThread->CreateThread();
					}
				}				
			}	
		}		
		
		if(nCnt == 1)
		{
			return;
		}

		TRACE("nCnt [%d]\n", nCnt);	
		POSITION pos = itemList.GetHeadPosition();
		for (int i=0;i < nCnt; i++)
		{
			HTREEITEM hSel = itemList.GetNext(pos);
			
			bConnStatus = FALSE;
			nBockMode = 0;
			n_StatusFlags = 0l;

			GetItemUserInfo(hSel, szSelItemIP, szSelItemID , szNickName,szRealName,szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);

			if(strcmp(szSelItemIP, "GRP") != 0 && strcmp(szSelItemID, m_szSelItemID) != 0 )
			{
				if (nBockMode == 4)
				{
					char szFmt [100] ;
					memset(szFmt,0x00,sizeof(szFmt));
					sprintf(szFmt, "상대방[%s]이 대화차단한 경우 파일을 전송할수 없습니다", szRealName);
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
					
					pMsgThread->CreateThread();
				}
				else
				{
					if(bConnStatus == TRUE)
					{						
						TRACE("szFileName [%s]\n",m_strFileAll);
			
						CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)szSelItemIP );
						
						if( NULL == pSpckObjMap )
						{					
							TRACE("socket connect fail! [%s]\n", m_strFileAll);	
													
							CString strErrMsg;
							strErrMsg.Format("%s 에 접속할수 없습니다", szSelItemIP);
							CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
							pMsgThread->CreateThread();
						}
						else
						{				
							TRACE("\nsocket connected! [%s]\n", m_strFileAll);
							
							CFileTransSock::Instance().InformFileSend( pSpckObjMap,szSelItemID,
								CString(szSelItemIP), (LPSTR)(LPCSTR) m_strFileAll , m_nFileCnt) ;
						} 
					}
					else
					{
						char szFmt [100] ;
						memset(szFmt,0x00,sizeof(szFmt));
						sprintf(szFmt, "OffLine 상대 [%s] 에게 파일을 전송할수 없습니다", szRealName);				
					
						CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,szFmt);					
						
						pMsgThread->CreateThread();
					}
				}				
			}

			TRACE("■ 선택된 아이템 [%s]\n", szSelItemID);			
		}				
	}
}


void CExtTreeCtrl::FileFindRecurse(LPCTSTR pstr)
{	
	BOOL bWorking = FALSE;
	CFileFind finder;
	

	bWorking = finder.FindFile(pstr);	

	if(bWorking )
	{
		bWorking = finder.FindNextFile();		

		if (finder.IsDots() == FALSE && finder.IsDirectory()==FALSE)
		{
			TRACE("file\n");			
			m_strFileAll =  m_strFileAll + CString(pstr) + "|";
			m_nFileCnt++;
		}
		else 
		if (finder.IsDots() == TRUE || finder.IsDirectory()==TRUE)		
		{			
			CString strWildcard(pstr);
			strWildcard += _T("\\*.*");			
			
			bWorking = finder.FindFile(strWildcard);
			
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
												
				if (finder.IsDots())
					continue;
				
				// if it's a directory, recursively search it

				if (finder.IsDirectory())
				{	
					// 폴더이면
					CString str = finder.GetFilePath();
					FileFindRecurse(str);
				}else{
					// 피일이면 (구하는 문자열)
					CString str = finder.GetFilePath();
					m_strFileAll = m_strFileAll + str + "|";// '|' 구분자는 파일경로단위
					m_nFileCnt++;
				}
			} // end of While			
		}		
	}
	finder.Close();
	
}


/////////////////////////////////////////////////////////////////////////////
// CExtTreeCtrl message handlers

void CExtTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	UINT uFlags;
	memset(m_szSelItemIP, 0x00, sizeof(m_szSelItemIP));
	HTREEITEM hItem = HitTest(point, &uFlags); 
	if( (hItem != NULL) && ( TVHT_ONITEM & uFlags)) 
	{		
		//BOOL b = Select(hItem, TVGN_CARET);
		
		char szRealName	[20 + 1];
		char szNickName	[C_NICK_NAME_LEN];
		char szGrpName  [C_GRP_NAME_LEN]; 
		BOOL bConnStatus = FALSE;
		int nBockMode = 0;	
		long n_StatusFlags = 0l;
		
		GetItemUserInfo(hItem, m_szSelItemIP, m_szSelItemID, szNickName, szRealName, szGrpName, bConnStatus, &nBockMode, &n_StatusFlags);
		
	}	
	

	CTreeCtrl::OnMouseMove(nFlags, point);
}
