// MemoItemDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "memocal.h"
#include "MemoItemDlg.h"
#include <imm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMemoItemDlg dialog


CMemoItemDlg::CMemoItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoItemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMemoItemDlg)
	m_DTPickTime = 0;
	//}}AFX_DATA_INIT

	m_pImageList = NULL;
	m_nIconIndex = -1;
	m_bNewItem = false;
	m_bUseAlarm = false;
}


void CMemoItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMemoItemDlg)
	DDX_Control(pDX, IDC_DATETIMEPICKER_DATE, m_DateCtrl);
	DDX_Control(pDX, IDC_DATETIMEPICKER_TIME, m_DateTimeCtrl);
	DDX_Control(pDX, IDC_BTN_ICON, m_btnIcon);
	DDX_Check(pDX, IDC_CHECK_ALARM, m_bUseAlarm);	
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_TIME, m_DTPickTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMemoItemDlg, CDialog)
	//{{AFX_MSG_MAP(CMemoItemDlg)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_ALARM, OnCheckAlarm)
	ON_BN_CLICKED(ID_CLOSE, OnClose)
	
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_ICONCANCELED, OnIconCanceled)
	ON_MESSAGE(UM_ICONSELECTED, OnIconSelected)	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoItemDlg message handlers

void CMemoItemDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
			
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();
}

void CMemoItemDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	m_cControlPos.AddControl(IDC_CONTENT	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );	
	m_cControlPos.AddControl(IDOK	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );		
	m_cControlPos.AddControl(IDCANCEL	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_CHECK_ALARM, CP_MOVE_VERTICAL );	
	//m_cControlPos.AddControl(IDC_DATETIMEPICKER_DATE, CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_DATETIMEPICKER_TIME, CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_TITLE, CP_RESIZE_HORIZONTAL );	
	m_cControlPos.AddControl(ID_CLOSE, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_DATETIMEPICKER_DATE, CP_MOVE_VERTICAL );	
}

BOOL CMemoItemDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText(m_strDlgTitle);
	
	SetResize();
	
	HIMC himc = ImmGetContext(GetSafeHwnd());
	//ImmSetConversionStatus(himc, 0, 0);// 영문전환시
	ImmSetConversionStatus(himc, 1, 0);// 한글전환시
	ImmReleaseContext(GetSafeHwnd(), himc); 
	
	GetDlgItem(IDC_TITLE)->SetWindowText((LPCTSTR)m_strTitle);
	GetDlgItem(IDC_CONTENT)->SetWindowText((LPCTSTR)m_strContent);

	m_btnIcon.SetImageList(m_pImageList);
	m_btnIcon.SetIcon( m_pImageList->ExtractIcon(m_nIconIndex) );
	m_btnIcon.SetAlign(CButtonST::ST_ALIGN_HORIZ_RIGHT);

	//CWnd* p1 = GetDlgItem(IDC_DATETIMEPICKER_DATE);
	//::EnableWindow( p1->GetSafeHwnd() , FALSE);

	CWnd* p2 = GetDlgItem(IDC_DATETIMEPICKER_TIME);
	::EnableWindow( p2->GetSafeHwnd() , FALSE);	

	m_nAlarmChked = -1;

	CDateTimeCtrl* pWnd = static_cast<CDateTimeCtrl*> ( GetDlgItem(IDC_DATETIMEPICKER_TIME) );
	pWnd->SetFormat("tt HH:mm");

	if(m_nIconIndex == 0 )
	{
		m_nAlarmChked = 1;
		m_bUseAlarm = TRUE;
		m_DateTimeCtrl.EnableWindow( TRUE);	
		m_DateCtrl.EnableWindow( TRUE);	
	}
	else
	{
		m_nAlarmChked = -1;
		m_bUseAlarm = FALSE;
		m_DateTimeCtrl.EnableWindow( FALSE);	
		m_DateCtrl.EnableWindow( FALSE);	
	}

	SetTimer(100, 100, NULL);
			

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMemoItemDlg::SetImageList(CImageList *pImageList)
{
	m_pImageList = pImageList;
}

int CMemoItemDlg::GetIconIndex()
{
	return m_nIconIndex;
}

CString CMemoItemDlg::GetTitle()
{
	return m_strTitle;
}

CString CMemoItemDlg::GetContent()
{
	return m_strContent;
}

void CMemoItemDlg::SetIconIndex(const int nIndex)
{
	if( m_pImageList == NULL ) return;
	if( nIndex > m_pImageList->GetImageCount() - 1 ) return;
	
	m_nIconIndex = nIndex;
}

void CMemoItemDlg::SetTitle(const CString strTitle)
{
	m_strTitle = strTitle;
}

void CMemoItemDlg::SetContent(const CString strContent)
{
	m_strContent = strContent;
}

void CMemoItemDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	if( bShow && m_bNewItem )
		GetDlgItem(IDC_CONTENT)->SetFocus();

//	if(!m_strAlarmTimeHH.IsEmpty())
//	{
//		//m_DTPickTime
//		CDateTimeCtrl* pWnd = static_cast<CDateTimeCtrl*> ( GetDlgItem(IDC_DATETIMEPICKER_TIME) );
//		COleDateTime tmpOleTm;
//		int nRn = tmpOleTm.SetTime( atoi( (LPCSTR) m_strAlarmTimeHH) , atoi( (LPCSTR) m_strAlarmTimeMM) , 0);
//
//		pWnd->SetTime(tmpOleTm);
//	}
}

LRESULT CMemoItemDlg::OnIconCanceled(WPARAM wParam, LPARAM lParam)
{
	m_btnIcon.SetState(FALSE);

	return 0;
}

LRESULT CMemoItemDlg::OnIconSelected(WPARAM wParam, LPARAM lParam)
{
	m_btnIcon.SetState(FALSE);

	m_nIconIndex = (int)wParam;
	m_btnIcon.SetIcon( m_pImageList->ExtractIcon(m_nIconIndex) );

	return 0;
}


void CMemoItemDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	if( nIDEvent == 100 )
	{
		//m_DTPickDate =  CTime::GetCurrentTime();

		if(m_strAlarmTimeHH.IsEmpty())
		{
			m_DTPickTime =  CTime::GetCurrentTime();
		}
		else
		{						
			if(m_strAlarmTimeYYYY.IsEmpty())
			{				
				SYSTEMTIME stDate; 				
				m_DateCtrl.GetTime(&stDate);
				
				DWORD nYYYY = stDate.wYear;
				DWORD nMM = stDate.wMonth;
				DWORD nDD = stDate.wDay;				
				m_strAlarmTimeYYYY.Format("%d", nYYYY);
				m_strAlarmTimeMonth.Format("%0.2d", nMM);
				m_strAlarmTimeDD.Format("%0.2d",nDD);
			}

			CDateTimeCtrl* pWndDate = static_cast<CDateTimeCtrl*> ( GetDlgItem(IDC_DATETIMEPICKER_DATE) );
			COleDateTime tmpOleTmDate;
			int nRn = tmpOleTmDate.SetDate( atoi( (LPCSTR) m_strAlarmTimeYYYY) , atoi( (LPCSTR) m_strAlarmTimeMonth) , atoi( (LPCSTR) m_strAlarmTimeDD ));
			pWndDate->SetTime(tmpOleTmDate);
			
			if(m_strAlarmTimeHH.IsEmpty())
			{				
				SYSTEMTIME stTime; 
				m_DateTimeCtrl.GetTime(&stTime);	

				DWORD nHour = stTime.wHour;
				DWORD nMin  = stTime.wMinute;

				m_strAlarmTimeHH.Format("%d", nHour );
				m_strAlarmTimeMM.Format("%d", nMin );
			}
			CDateTimeCtrl* pWnd = static_cast<CDateTimeCtrl*> ( GetDlgItem(IDC_DATETIMEPICKER_TIME) );
			COleDateTime tmpOleTm;
			nRn = tmpOleTm.SetTime( atoi( (LPCSTR) m_strAlarmTimeHH) , atoi( (LPCSTR) m_strAlarmTimeMM) , 0);
			pWnd->SetTime(tmpOleTm);

			

			KillTimer(100);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CMemoItemDlg::OnCheckAlarm() 
{
	// TODO: Add your control notification handler code here
	
	if( -1 == m_nAlarmChked )
	{
		m_nAlarmChked *= -1;
		m_DateTimeCtrl.EnableWindow( TRUE);	
		m_DateCtrl.EnableWindow( TRUE);	
	}
	else if( 1 == m_nAlarmChked )
	{
		m_nAlarmChked *= -1;
		m_DateTimeCtrl.EnableWindow( FALSE);	
		m_DateCtrl.EnableWindow( FALSE);	
	}	
	
	UpdateData(TRUE);

	m_strAlarmTimeHH = m_DTPickTime.Format("%H");	
	m_strAlarmTimeMM = m_DTPickTime.Format("%M");	
}

void CMemoItemDlg::OnClose() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CString strTmp;
	m_DateTimeCtrl.GetWindowText(strTmp);
		
	
	CTime tmDest;
	SYSTEMTIME stTime,stDate; 
	m_DateTimeCtrl.GetTime(&stTime);
	m_DateCtrl.GetTime(&stDate);
	
	DWORD nYYYY = stDate.wYear;
	DWORD nMM = stDate.wMonth;
	DWORD nDD = stDate.wDay;
	
	m_strAlarmTimeYYYY.Format("%d", nYYYY);
	m_strAlarmTimeMonth.Format("%0.2d", nMM);
	m_strAlarmTimeDD.Format("%0.2d",nDD);

	DWORD nHour = stTime.wHour;
	DWORD nMin  = stTime.wMinute;
	
	strAlarmTimeYYYYMMDDHHMI.Format("%d%0.2d%0.2d%0.2d%0.2d", nYYYY, nMM, nDD,nHour,nMin);
	m_strAlarmTimeHH.Format("%d", nHour );
	m_strAlarmTimeMM.Format("%d", nMin );

	//m_strAlarmTimeHH = stTime.wHour ; //tmDest.Format("%H");	
	//m_strAlarmTimeMM = stTime.wMinute; //tmDest.Format("%M");

	GetDlgItem(IDC_TITLE)->GetWindowText(m_strTitle);
	GetDlgItem(IDC_CONTENT)->GetWindowText(m_strContent);

	if( m_strTitle.GetLength() == 0 )
	{
		AfxMessageBox(_T("제목을 입력하세요."));
		GetDlgItem(IDC_TITLE)->SetFocus();
		return;
	}

	// Alarm 처리 
	
	//strYMD = m_DTPickOnceTime.Format("%Y_%m_%d_");
	//strYMDUI = m_DTPickOnceTime.Format("%Y년%m월%d일");
	//strHMS = m_DTPick.Format(" %H_%M_ ");	
	
	
	
	//strUIReservedDT = strYMDUI + strHMSUI;
	

	if(m_nAlarmChked == 1 )
		m_nIconIndex = 0 ; // Alarm 아이콘 지정한다.
	else
		m_nIconIndex = 1 ; 

	CDialog::OnOK();
}


