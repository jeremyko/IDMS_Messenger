//------------------------------------------------------------------------------
// ControlPos.cpp
// 
//	CControlPos 
//	Position controls on a form's resize 
// 
//		Copyright (c) 2000 Paul Wendt
// 
//		VERSION#	DATE			NAME	DESCRIPTION OF CHANGE
//		--------	----------	----	---------------------
//		1.01  	07/11/2000	PRW	Original creation.
// 
#include "StdAfx.h"
#include "ControlPos.h"

//------------------------------------------------------------------------------
// CControlPos::CControlPos
// 
//	default constructor 
// 
//	Access: public
// 
//	Args:
//		CWnd* pParent				=	pointer to parent window
// 
//	Return:
//		none
// 
CControlPos::CControlPos(CWnd* pParent /* = NULL */)
{
	m_pParent = pParent;
	UpdateParentSize();

	m_nOldParentHeight = 0;
	m_nOldParentWidth = 0;

	m_DontMoveWndSizeWidth = 0;
	m_DontMoveWndSizeHeight = 0;
	
	SetNegativeMoves(FALSE);
	//SetNegativeMoves(TRUE);

	ResetControls();
	
	m_bISnapShot = FALSE;
	m_bUpflag = FALSE;
	m_bDownflag = FALSE;
}

//------------------------------------------------------------------------------
// CControlPos::~CControlPos
// 
//	default destructor -- It deletes all controls. 
// 
//	Access: public
// 
//	Args:
//		none
// 
//	Return:
//		none
// 
CControlPos::~CControlPos()
{
	ResetControls();
}

//------------------------------------------------------------------------------
// CControlPos::SetParent
// 
//	This sets the parent window. It should be called from a CWnd's 
//    post-constructor function, like OnInitdialog or InitialUpdate. 
// 
//	Access: public
// 
//	Args:
//		CWnd* pParent	=	parent window
// 
//	Return:
//		none
// 
void CControlPos::SetParent(CWnd* pParent)
{
	CRect rcParentOriginalSize;

	m_pParent = pParent;

	m_pParent->GetClientRect(rcParentOriginalSize);
	m_nOriginalParentWidth = rcParentOriginalSize.right;
	m_nOriginalParentHeight = rcParentOriginalSize.bottom;	

	m_nFirstOriPWidth = rcParentOriginalSize.right;
	m_nFirstOriPHeight= rcParentOriginalSize.bottom;			

	TRACE("▶ SetParent W[%d] H[%d]\n",m_nOriginalParentWidth,m_nOriginalParentHeight);

	UpdateParentSize();
}

//------------------------------------------------------------------------------
// CControlPos::AddControl
// 
//	This adds a control to the internal list of controls in CControlPos.  
// 
//	Access: public
// 
//	Args:
//		CWnd* pControl				=	pointer to the control to be added
//		const DWORD& dwStyle		=  how the window should be moved -- see #define's
//                               in the header file
// 
//	Return:
//		BOOL 	=	TRUE if the control was added successfully, FALSE otherwise
// 
BOOL CControlPos::AddControl(CWnd* pControl, const DWORD& dwStyle /* = CP_MOVE_HORIZONTAL */)
{
	BOOL fReturnValue = TRUE;

	if (pControl && m_pParent)
	{
		LPCONTROLDATA pstControl = new CONTROLDATA;
		pstControl->hControl = pControl->GetSafeHwnd();
		pstControl->dwStyle = dwStyle;
		
		m_awndControls.Add(((CObject*)pstControl));
	}
	else
	{
		fReturnValue = FALSE;
	}

	return (fReturnValue);
}

//------------------------------------------------------------------------------
// CControlPos::AddControl
// 
//	This adds a control the internal list of controls in CControlPos. 
// 
//	Access: public
// 
//	Args:
//		const UINT& unId			=	ID of the control to add
//		const DWORD& dwStyle		=	how the window should be moved -- see #define's
//                               in the header file
// 
//	Return:
//		BOOL 	=	TRUE if the control was added successfully, FALSE otherwise
// 
BOOL CControlPos::AddControl(const UINT& unId, const DWORD& dwStyle  /* = CP_MOVE_HORIZONTAL */)
{
	CWnd* pControl;

	if (m_pParent)
	{
		pControl = m_pParent->GetDlgItem(unId);
		return (AddControl(pControl, dwStyle));
	}
	else
	{
		return (FALSE);
	}
}

//------------------------------------------------------------------------------
// CControlPos::RemoveControl
// 
//	If a client ever wants to remove a control programmatically, this 
//    function will do it. 
// 
//	Access: public
// 
//	Args:
//		CWnd* pControl	=	pointer of the window who should be removed from 
//								the internal control list [ie: will not be repositioned]
// 
//	Return:
//		BOOL 	=	TRUE if the control was found [and deleted], FALSE otherwise
// 
BOOL CControlPos::RemoveControl(CWnd* pControl)
{
	BOOL fReturnValue = FALSE;

	for (int i = 0; i < m_awndControls.GetSize(); i++)
	{
		LPCONTROLDATA pstControl = ((LPCONTROLDATA)m_awndControls.GetAt(i));
		
		if (pstControl->hControl == pControl->GetSafeHwnd())
		{
			m_awndControls.RemoveAt(i);
			delete pstControl;
			fReturnValue = TRUE;
			break;
		}
	}

	return (fReturnValue);
}

//------------------------------------------------------------------------------
// CControlPos::RemoveControl
// 
//	If a client ever wants to remove a control programmatically, this 
//    function will do it. 
// 
//	Access: public
// 
//	Args:
//		const UINT& unId  =  ID of the control that should be removed from the
//                         internal control list [ie: will not be repositioned]
// 
//	Return:
//		BOOL 	=	TRUE if the control was found [and deleted], FALSE otherwise
// 
BOOL CControlPos::RemoveControl(const UINT& unId)
{
	CWnd* pControl;

	if (m_pParent)
	{
		pControl = m_pParent->GetDlgItem(unId);
		return (RemoveControl(pControl));
	}
	else
	{
		return (FALSE);
	}
}

//------------------------------------------------------------------------------
// CControlPos::ResetControls
// 
//	This function removes all controls from the CControlPos object 
// 
//	Access: public
// 
//	Args:
//		none
// 
//	Return:
//		none
// 
void CControlPos::ResetControls(void)
{
	while (m_awndControls.GetSize() > 0)
	{
		int   nHighIdx = m_awndControls.GetUpperBound();
		LPCONTROLDATA pstControl = ((LPCONTROLDATA)m_awndControls.GetAt(nHighIdx));
		if (pstControl)
		{
			m_awndControls.RemoveAt(nHighIdx);
			delete pstControl;
		}
	}
}

//------------------------------------------------------------------------------
// CControlPos::MoveControls
// 
//	This function takes care of moving all controls that have been added to 
//    the object [see AddControl].  This function should be called from the 
//    WM_SIZE handler-function [typically OnSize]. 
// 
//	Access: public
// 
//	Args:
//		none
// 
//	Return:
//		none
// 
/*
void CControlPos::MoveControls(int nHeightParent, int nWidthParent)
{
	//TRACE("MoveControls !!\n");
	if (m_pParent)
	{		
		for (int i = 0; i < m_awndControls.GetSize(); i++)
		{			
			LPCONTROLDATA pstControl = ((LPCONTROLDATA)m_awndControls.GetAt(i));
			CRect rcParentBounds;
			CRect rcBounds;
			
			CWnd* pControl = m_pParent->FromHandle(pstControl->hControl);

			pControl->GetWindowRect(rcBounds);
			m_pParent->GetClientRect(rcParentBounds);

			// kojh
			
			if(pstControl->nConType == 1)
			{									
				pControl->GetWindowRect(&m_rcRichEdit);	 // Screen Cor				

				if(m_rcRichEdit.Height() <= 0) 
				{	
					if( m_bISnapShot== FALSE && m_bUpflag == TRUE)
					{						
						m_bISnapShot = TRUE;
						
						m_nOriginalParentHeight = nHeightParent;														
						UpdateParentSize();	
						return;
					}else{						
						if(nHeightParent  <= m_nOriginalParentHeight)
						{
							//TRACE("Size 변동 없음 #2\n");
							m_bISnapShot = TRUE;
							return;
						}
						else{
							TRACE("Size 변동 !!\n");
							m_bISnapShot = FALSE;
							//m_nOriginalParentWidth = m_nFirstOriPWidth;   // 일정 크기 이상이면 다시 최초 크기를 설정
							m_nOriginalParentHeight = m_nFirstOriPHeight ;							
						}
					}
				}
			}
			
		
			else if(pstControl->nConType == CON_STATIC)
			{
				pControl->GetWindowRect(&m_rcSplitter);
				
			}
			else if(pstControl->nConType == CON_CEDIT)
			{
				pControl->GetWindowRect(&m_rcEdit);		
				
			}

						
			if ((pstControl->dwStyle & (CP_RESIZE_VERTICAL)) == (CP_RESIZE_VERTICAL))
			{				
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.bottom > m_nOriginalParentHeight) // 화면을 크게하는경우 
					{
						if (m_nOriginalParentHeight <= m_nOldParentHeight)						
						{		
							//TRACE("화면을 크게하는경우 컨트롤을 증가[%s]\n",pstControl->szName);							
							rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight; // 현재 확대된 화면 - 이전 화면 크기만큼 컨트롤을 증가.							
						}
						else
						{							
							rcBounds.bottom += rcParentBounds.bottom - m_nOriginalParentHeight; 
						}
					}
					else 
					{
						if (m_nOldParentHeight > m_nOriginalParentHeight) 
						{							
							rcBounds.bottom += m_nOriginalParentHeight - m_nOldParentHeight; 							
						}
					}
				}
				else				
				{
					rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight;
				}
				
//				if(pstControl->nConType == CON_CEDIT) 	
//				{
//					pControl->GetWindowRect(&m_rcEdit);	
//					
//				}
			}

			if ((pstControl->dwStyle & (CP_RESIZE_HORIZONTAL)) == (CP_RESIZE_HORIZONTAL))
			{				
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.right > m_nOriginalParentWidth)
					{
						if (m_nOriginalParentWidth <= m_nOldParentWidth)
						{
							rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
						}
						else
						{
							rcBounds.right += rcParentBounds.right - m_nOriginalParentWidth;
						}
					}
					else
					{
						if (m_nOldParentWidth > m_nOriginalParentWidth)
						{
							rcBounds.right += m_nOriginalParentWidth - m_nOldParentWidth;
						}
					}
				}
				else				
				{
					rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
				}

//				if(pstControl->nConType == CON_CEDIT) 
//				{
//					pControl->GetWindowRect(&m_rcEdit);	
//					
//				}
			}

			if ((pstControl->dwStyle & (CP_MOVE_VERTICAL)) == (CP_MOVE_VERTICAL))
			{				
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.bottom > m_nOriginalParentHeight)
					{
						if (m_nOriginalParentHeight <= m_nOldParentHeight)
						{
							rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight; 
							rcBounds.top += rcParentBounds.bottom - m_nOldParentHeight;    
						}
						else
						{
							rcBounds.bottom += rcParentBounds.bottom - m_nOriginalParentHeight; 
							rcBounds.top += rcParentBounds.bottom - m_nOriginalParentHeight;    
							
						}
					}
					else
					{
						if (m_nOldParentHeight > m_nOriginalParentHeight)
						{
							rcBounds.bottom += m_nOriginalParentHeight - m_nOldParentHeight; 
							rcBounds.top += m_nOriginalParentHeight - m_nOldParentHeight;    
							
						}
					}
				}
				else				
				{
					rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight; 
					rcBounds.top += rcParentBounds.bottom - m_nOldParentHeight; 
				}

				//TEST
//				if(pstControl->nConType == CON_STATIC) 
//				{
//					pControl->GetWindowRect(&m_rcSplitter);					
//					
//				}
//				else if(pstControl->nConType == CON_CEDIT)
//				{
//					pControl->GetWindowRect(&m_rcEdit);	
//					
//				}
			}

			if ((pstControl->dwStyle & (CP_MOVE_HORIZONTAL)) == (CP_MOVE_HORIZONTAL))
			{				
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.right > m_nOriginalParentWidth)
					{
						if (m_nOriginalParentWidth <= m_nOldParentWidth)
						{
							rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
							rcBounds.left += rcParentBounds.right - m_nOldParentWidth;
						}
						else
						{
							rcBounds.right += rcParentBounds.right - m_nOriginalParentWidth;
							rcBounds.left += rcParentBounds.right - m_nOriginalParentWidth;
						}
					}
					else
					{
						if (m_nOldParentWidth > m_nOriginalParentWidth)
						{
							rcBounds.right += m_nOriginalParentWidth - m_nOldParentWidth;
							rcBounds.left += m_nOriginalParentWidth - m_nOldParentWidth;
						}
					}
				}
				else				
				{
					rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
					rcBounds.left += rcParentBounds.right - m_nOldParentWidth;
				}

//				if(pstControl->nConType == CON_CEDIT) 
//				{
//					pControl->GetWindowRect(&m_rcEdit);	
//					//pControl->ScreenToClient(&m_rcEdit);
				}
			}

			//TEST kojh	
			m_pParent->ScreenToClient(rcBounds); // 이렇하면 동작함
			m_pParent->ScreenToClient(m_rcEdit); // 이렇하면 동작함
			m_pParent->ScreenToClient(m_rcSplitter); // 이렇하면 동작함

			//if(pstControl->nConType == CON_REDIT)
			{
				// Splitter 보다 항상 위로..				
				if(m_rcSplitter.Height() > 0)
				{					
					//rcBounds -> GetWindowRect Screen좌표,  m_rcRichEdit,m_rcSplitter->Screen좌표
					
					// 화면 갱신 문제...
					//TRACE("Splitter 보다 항상 위로 re.bottom [%d] sp.top[%d]\n", m_rcRichEdit.bottom, m_rcSplitter.top);					
					//m_rcRichEdit.bottom = m_rcSplitter.top - 5;					
					//rcBounds.bottom = m_rcRichEdit.bottom;
					//TRACE("rcBounds.bottom [%d] rcBounds.top[%d]\n", rcBounds.bottom, rcBounds.top);
					

					//rcBounds.bottom = rcParentBounds.bottom - 100 ; // 이렇 하면 동작함
					//TRACE("rcParentBounds.bottom [%d] m_rcEdit.Height[%d] m_rcSplitter.Height[%d]\n", rcParentBounds.bottom, m_rcEdit.Height(),m_rcSplitter.Height());					
					rcBounds.bottom = rcParentBounds.bottom - m_rcEdit.Height() - m_rcSplitter.Height() - 10; // 이렇 하면 동작함
					
				}				
			}
				
			//m_pParent->ScreenToClient(rcBounds); // 화면 갱신 문제...
			//if(pstControl->nConType == CON_STATIC) // CON_REDIT, CON_STATIC, CON_CEDIT, CON_BTN
			{				
				//rcBounds.bottom = rcParentBounds.bottom - m_rcEdit.Height() - m_rcSplitter.Height() -30; // 이렇 하면 동작함
			}

			//if(pstControl->nConType == CON_CEDIT) // Edit
			{
				// parentd client 영역 보다 항상 위로..
				if(m_rcSplitter.Height() > 0)
				{					
					rcBounds.bottom = rcParentBounds.bottom - 5 ;
					rcBounds.right = rcParentBounds.right - 80;
				}
			}			
			else if(pstControl->nConType == CON_BTN) // 버튼
			{
				// parentd client 영역 보다 항상 위로..
				if(m_rcSplitter.Height() > 0)
				{	
					rcBounds.left = rcParentBounds.right - 65 ;
					rcBounds.top = rcParentBounds.bottom - 50 ;
					rcBounds.bottom = rcParentBounds.bottom - 5 ;
					rcBounds.right = rcParentBounds.right - 5;
				}
			}
			//TEST kojh
			pControl->MoveWindow(rcBounds);
			pControl->Invalidate();
		}

		UpdateParentSize();		
	}
}
*/

//------------------------------------------------------------------------------
// CControlPos::SetNegativeMoves
// 
//	This sets the NegativeMoves boolean parameter of the object. When the 
//    parent window becomes smaller than it started, setting this to FALSE 
//    will not allow controls to be moved; the parent size may change, but 
//    it'll just force the controls to go off of the 
//    This parameter defaults to FALSE on object creation.
// 
//	Access: public
// 
//	Args:
//		const BOOL& fNegativeMoves /* = TRUE */	=	value to set
// 
//	Return:
//		none
// 
void CControlPos::SetNegativeMoves(const BOOL& fNegativeMoves /* = TRUE */)
{
	m_fNegativeMoves = fNegativeMoves;
}

//------------------------------------------------------------------------------
// CControlPos::GetNegativeMoves
// 
//	This function returns whether or not negative moves are enabled. 
// 
//	Access: public
// 
//	Args:
//		none
// 
//	Return:
//		BOOL 	=	TRUE if negative moves are enabled, FALSE otherwise
// 
BOOL CControlPos::GetNegativeMoves(void) const
{
	return (m_fNegativeMoves);
}

//------------------------------------------------------------------------------
// CControlPos::UpdateParentSize
// 
//	Since CControlPos keeps track of the parent's size, it gets updated 
//    every time it tells us to size the controls. We keep track so we know 
//    how much it changed from the last WM_SIZE message. 
// 
//	Access: protected
// 
//	Args:
//		none
// 
//	Return:
//		none
// 
void CControlPos::UpdateParentSize(void)
{
	m_bUpflag = FALSE;
	m_bDownflag = FALSE;

	if (m_pParent)
	{
		int nOldHeight = 0;
		CRect rcBounds;
		m_pParent->GetClientRect(rcBounds);

		nOldHeight = m_nOldParentHeight;

		m_nOldParentWidth = rcBounds.Width();
		m_nOldParentHeight = rcBounds.Height();

		if((nOldHeight - m_nOldParentHeight) < 0)
		{
			//TRACE("DOWN DOWN !!\n");
			m_bUpflag = FALSE;
			m_bDownflag = TRUE;
		}else 
		if((nOldHeight - m_nOldParentHeight) > 0)
		{
			//TRACE("UP UP !!\n");
			m_bUpflag = TRUE;
			m_bDownflag = FALSE;
		}else{
			//TRACE("??? !!\n");
			//m_bUpflag = TRUE;
			//m_bDownflag = TRUE;
		}

		// m_nOriginalParentWidth 
		// m_nOriginalParentHeight
	}
}















///////////////////////////////////////////////////////////////////////////////
// BAck Up kojh
///////////////////////////////////////////////////////////////////////////////

void CControlPos::MoveControls(void)
{
	if (m_pParent)
	{		
		for (int i = 0; i < m_awndControls.GetSize(); i++)
		{			
			LPCONTROLDATA pstControl = ((LPCONTROLDATA)m_awndControls.GetAt(i));
			CRect rcParentBounds;
			CRect rcBounds;
			CWnd* pControl = m_pParent->FromHandle(pstControl->hControl);

			pControl->GetWindowRect(rcBounds);
			m_pParent->GetClientRect(rcParentBounds);

			if ((pstControl->dwStyle & (CP_RESIZE_VERTICAL)) == (CP_RESIZE_VERTICAL))
			{
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.bottom > m_nOriginalParentHeight)
					{
						if (m_nOriginalParentHeight <= m_nOldParentHeight)
						{		
							rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight;	 // Ori													
						}
						else
						{
							rcBounds.bottom += rcParentBounds.bottom - m_nOriginalParentHeight;
						}
					}
					else
					{
						if (m_nOldParentHeight > m_nOriginalParentHeight)
						{
							rcBounds.bottom += m_nOriginalParentHeight - m_nOldParentHeight; //Ori							
						}
					}
				}
				else
				{
					rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight;
				}
			}

			if ((pstControl->dwStyle & (CP_RESIZE_HORIZONTAL)) == (CP_RESIZE_HORIZONTAL))
			{
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.right > m_nOriginalParentWidth)
					{
						if (m_nOriginalParentWidth <= m_nOldParentWidth)
						{
							rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
						}
						else
						{
							rcBounds.right += rcParentBounds.right - m_nOriginalParentWidth;
						}
					}
					else
					{
						if (m_nOldParentWidth > m_nOriginalParentWidth)
						{
							rcBounds.right += m_nOriginalParentWidth - m_nOldParentWidth;
						}
					}
				}
				else
				{
					rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
				}
			}

			if ((pstControl->dwStyle & (CP_MOVE_VERTICAL)) == (CP_MOVE_VERTICAL))
			{
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.bottom > m_nOriginalParentHeight)
					{
						if (m_nOriginalParentHeight <= m_nOldParentHeight)
						{
							rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight; // Ori
							rcBounds.top += rcParentBounds.bottom - m_nOldParentHeight;    // Ori
						
						}
						else
						{
							rcBounds.bottom += rcParentBounds.bottom - m_nOriginalParentHeight; // Ori
							rcBounds.top += rcParentBounds.bottom - m_nOriginalParentHeight;    // Ori						
							
						}
					}
					else
					{
						if (m_nOldParentHeight > m_nOriginalParentHeight)
						{
							rcBounds.bottom += m_nOriginalParentHeight - m_nOldParentHeight; // Ori
							rcBounds.top += m_nOriginalParentHeight - m_nOldParentHeight;    // Ori							
						}
					}
				}
				else
				{
					rcBounds.bottom += rcParentBounds.bottom - m_nOldParentHeight; // Ori
					rcBounds.top += rcParentBounds.bottom - m_nOldParentHeight; //Ori					
				}
			}

			if ((pstControl->dwStyle & (CP_MOVE_HORIZONTAL)) == (CP_MOVE_HORIZONTAL))
			{
				if (!m_fNegativeMoves)
				{
					if (rcParentBounds.right > m_nOriginalParentWidth)
					{
						if (m_nOriginalParentWidth <= m_nOldParentWidth)
						{
							rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
							rcBounds.left += rcParentBounds.right - m_nOldParentWidth;
						}
						else
						{
							rcBounds.right += rcParentBounds.right - m_nOriginalParentWidth;
							rcBounds.left += rcParentBounds.right - m_nOriginalParentWidth;
						}
					}
					else
					{
						if (m_nOldParentWidth > m_nOriginalParentWidth)
						{
							rcBounds.right += m_nOriginalParentWidth - m_nOldParentWidth;
							rcBounds.left += m_nOriginalParentWidth - m_nOldParentWidth;
						}
					}
				}
				else
				{
					rcBounds.right += rcParentBounds.right - m_nOldParentWidth;
					rcBounds.left += rcParentBounds.right - m_nOldParentWidth;
				}
			}

			m_pParent->ScreenToClient(rcBounds);
			pControl->MoveWindow(rcBounds);
		}

		UpdateParentSize();
	}
}
