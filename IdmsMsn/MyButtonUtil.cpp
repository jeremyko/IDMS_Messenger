// MyButtonUtil.cpp: implementation of the CMyButtonUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyButtonUtil.h"
#include "MyButton.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyButtonUtil::CMyButtonUtil()
{

}

CMyButtonUtil::~CMyButtonUtil()
{

}

void  CMyButtonUtil::AutoSetting(UINT nType, CWnd *wndButton, CWnd* wndParent)
{
	/*
	HINSTANCE hInstResource = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("CommonCtrl.dll");
	AfxSetResourceHandle(hModule);
	
	CMyButton *pMyButton = (CMyButton*)wndButton;
	switch(nType)
	{
	case BUTTON_SETTING: 
		{
			pMyButton->AutoSize(wndParent, IDB_BUTTON_SETTING_OVER);
			pMyButton->SetBitmaps(IDB_BUTTON_SETTING_OVER, RGB(255, 0, 255), IDB_BUTTON_SETTING, RGB(255, 0, 255));
			//SetFlat(FALSE);	//FALSE : 보통 버튼(튀어나온 버튼), 눌렀을때 이미지가 바뀜
			pMyButton->DrawBorder(FALSE);
			pMyButton->DrawBackColor();	
		}break;
	
    
	AfxSetResourceHandle(hInstResource);
	*/
}
