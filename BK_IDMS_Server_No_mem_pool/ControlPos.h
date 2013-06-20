//------------------------------------------------------------------------------
// ControlPos.h
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
#ifndef CONTROLPOS_H_
#define CONTROLPOS_H_

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

//----------------------------------------------
// these #define's specify HOW the control
// will move. they can be combined with the
// bitwise or operator
//
#define CP_MOVE_HORIZONTAL					1
#define CP_MOVE_VERTICAL					2
#define CP_RESIZE_HORIZONTAL				4
#define CP_RESIZE_VERTICAL					8

class  CControlPos
{
public:
	CControlPos(CWnd* pParent = NULL);
	virtual ~CControlPos();

public:
	void SetParent(CWnd* pParent);

	BOOL AddControl(CWnd* pControl,  const DWORD& dwStyle = CP_MOVE_HORIZONTAL );
	BOOL AddControl(const UINT& unId,  const DWORD& dwStyle = CP_MOVE_HORIZONTAL );
	BOOL RemoveControl(CWnd* pControl);
	BOOL RemoveControl(const UINT& unId);
	void ResetControls(void);
	//virtual void MoveControls(int nHeightParent, int nWidthParent);
	virtual void MoveControls();

	//---------------------------------------------------
	// most of the time, you don't want to move controls
	// if the user reduces window size [controls can
	// overlap and cause "issues"]
	// negative moves won't move controls when the parent
	// window is getting smaller than its original size
	//
	void SetNegativeMoves(const BOOL& fNegativeMoves = TRUE);
	BOOL GetNegativeMoves(void) const;

protected:
	virtual void UpdateParentSize(void);

private:
	CWnd*  m_pParent;		
	int    m_nOldParentWidth;			
	int    m_nOldParentHeight;
	int    m_nFirstOriPWidth;
	int    m_nFirstOriPHeight;
	int    m_nOriginalParentWidth;
	int    m_nOriginalParentHeight;
	BOOL   m_fNegativeMoves;

	int		m_DontMoveWndSizeWidth;
	int		m_DontMoveWndSizeHeight;
	
	BOOL    m_bISnapShot;
	BOOL    m_bUpflag;
	BOOL	m_bDownflag;
	CObArray m_awndControls;
};

//----------------------------------------------------
// internal structure used to hold all information
// about a CWnd* control
//
typedef struct tagCONTROLDATA
{
	HWND  hControl;	// HWND's never change; some MFC functions return temporary CWnd *'s
	DWORD dwStyle;		// check the #define's above
	
} CONTROLDATA, *LPCONTROLDATA;

#endif
