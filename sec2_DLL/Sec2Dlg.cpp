// Sec2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "sec2_DLL.h"
#include "Sec2Dlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSec2Dlg dialog



CSec2Dlg::CSec2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSec2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSec2Dlg)
	//}}AFX_DATA_INIT
}


void CSec2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSec2Dlg)
	DDX_Control(pDX, IDC_LIST_LUNCH, m_LunchList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSec2Dlg, CDialog)
	//{{AFX_MSG_MAP(CSec2Dlg)	
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	//메인에서 오는 데이터
	ON_MESSAGE(WM_SECTION_RESPONSE,   OnMainResponse)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSec2Dlg message handlers


void CSec2Dlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	//m_cControlPos.AddControl(&m_staNick	, CP_RESIZE_HORIZONTAL );		
	//m_cControlPos.AddControl(&m_btnMyStatus	, CP_MOVE_HORIZONTAL );		
	m_cControlPos.AddControl(IDC_LIST_LUNCH	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );		
	//m_cControlPos.AddControl(&m_statusBar	, CP_RESIZE_HORIZONTAL	| CP_MOVE_VERTICAL	);	
	
}

void CSec2Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();
}

BOOL CSec2Dlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	CTime iDT =  CTime::GetCurrentTime();						

	int nCurDay = iDT.GetDayOfWeek();	// 1 = Sunday, 2 = Monday, ..., 7 = Saturday
	
	m_LunchList.m_nColorRow1 = -1; m_LunchList.m_nColorRow2 = -1;		
	if(nCurDay == 2)
	{
		//월요일
		m_LunchList.m_nColorRow1 = 0; m_LunchList.m_nColorRow2 = 1;
	}
	else if(nCurDay == 3)
	{
		// 화요일
		m_LunchList.m_nColorRow1 = 2; m_LunchList.m_nColorRow2 = 3;
	}
	else if(nCurDay == 4)
	{
		//수요일
		m_LunchList.m_nColorRow1 = 4; m_LunchList.m_nColorRow2 = 5;
	}
	else if(nCurDay == 5)
	{
		//목
		m_LunchList.m_nColorRow1 = 6; m_LunchList.m_nColorRow2 = 7;
	}
	else if(nCurDay == 6)
	{
		//금
		m_LunchList.m_nColorRow1 = 8; m_LunchList.m_nColorRow2 = 9;
	}			
	

	UpdateData(FALSE);

	CWinApp *pApp = AfxGetApp();
	CWnd *pWnd = pApp->GetMainWnd();	
		
	m_LunchList.SetSize(3, 10);
	   
	m_LunchList.SetColHeading(0,_T("날자"));
	m_LunchList.SetColHeading(1,_T("메뉴"));	
	m_LunchList.SetColHeading(2,_T("열량"));		
	
	m_LunchList.SetColWidth(0,100);
	m_LunchList.SetColWidth(1,150);
	m_LunchList.SetColWidth(2,80);
	
	
	SetResize();

	// 용도에 맞는 구분값으로 요청
	::PostMessage( pWnd->GetSafeHwnd(), WM_SECTION_REQUEST, (WPARAM)NULL, (LPARAM)"Sec2_Req_LunchMenu");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CSec2Dlg::OnMainResponse(WPARAM wParam, LPARAM lParam)
{
	TRACE("%s\n", (char*)wParam);
	
	// 용도에 맞는 구분값으로 분류한후 처리

	if(strcmp("Sec2_Res_FoodMenu", (char*)wParam)==0)
	{
		CPtrArray* pArray  = (CPtrArray*)lParam;
		
		for(int i = 0; i < pArray->GetSize(); i++)
		{
			S_FOOD_MENU_CLI *pData = static_cast<S_FOOD_MENU_CLI*>(pArray->GetAt(i));		
		}	
		
		
		int k=0 ;
		for(int j = 0; j < pArray->GetSize(); j++)
		{			
			S_FOOD_MENU_CLI *pData = static_cast<S_FOOD_MENU_CLI*>(pArray->GetAt(j));

			CString strTmp(pData->m_szMenu);
			strTmp.Replace(" ", "\r\n");

			if(pData != NULL)
			{				
				if( strncmp("1", pData->m_szLD_FLAG, 1)==0 )
				{
					//중식
					char szTmpDate[100+1];
					memset(szTmpDate, 0x00, sizeof(szTmpDate));
					switch(k)
					{
					case 0:
						sprintf(szTmpDate,"%s\r\n월요일\r\n        (중식)",pData->m_szDate);
						break; 
					case 2:
						sprintf(szTmpDate,"%s\r\n화요일\r\n        (중식)",pData->m_szDate);
						break;
					case 4:
						sprintf(szTmpDate,"%s\r\n수요일\r\n        (중식)",pData->m_szDate);
						break;
					case 6:
						sprintf(szTmpDate,"%s\r\n목요일\r\n        (중식)",pData->m_szDate);
						break;
					case 8:
						sprintf(szTmpDate,"%s\r\n금요일\r\n        (중식)",pData->m_szDate);
						break;
					}					
					m_LunchList.SetCellText(0,k,szTmpDate);
					m_LunchList.SetCellText(1,k,strTmp);
					m_LunchList.SetCellText(2,k,pData->m_szCal);
					k++;
				}
				else
				{
					//석식 
					char szTmpDate[100+1];
					memset(szTmpDate, 0x00, sizeof(szTmpDate));
					switch(k)
					{
					case 1:
						//sprintf(szTmpDate,"%s 월요일 (석식)",pData->m_szDate);
						sprintf(szTmpDate,"\r\n        (석식)",pData->m_szDate);
						break;
					case 3:
						//sprintf(szTmpDate,"%s 화요일 (석식)",pData->m_szDate);
						sprintf(szTmpDate,"\r\n        (석식)",pData->m_szDate);
						break;
					case 5:
						//sprintf(szTmpDate,"%s 수요일 (석식)",pData->m_szDate);
						sprintf(szTmpDate,"\r\n        (석식)",pData->m_szDate);
						break;
					case 7:
						//sprintf(szTmpDate,"%s 목요일 (석식)",pData->m_szDate);
						sprintf(szTmpDate,"\r\n        (석식)",pData->m_szDate);
						break;
					case 9:
						//sprintf(szTmpDate,"%s 금요일 (석식)",pData->m_szDate);
						sprintf(szTmpDate,"\r\n        (석식)",pData->m_szDate);
						break;
					}					
					m_LunchList.SetCellText(0,k,szTmpDate);
					m_LunchList.SetCellText(1,k,strTmp);
					m_LunchList.SetCellText(2,k,pData->m_szCal);
					k++;
				}				
				
				delete pData;
				pData = NULL;
			}
		}	
		
		delete pArray;
	}	

	return TRUE;
}

BOOL CSec2Dlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		return TRUE;
	}

	
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}
