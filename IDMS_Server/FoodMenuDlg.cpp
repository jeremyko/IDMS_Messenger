// FoodMenuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "idms_server.h"
#include "FoodMenuDlg.h"
#include  <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFoodMenuDlg dialog


CFoodMenuDlg::CFoodMenuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFoodMenuDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFoodMenuDlg)
	m_strMenu11 = _T("");
	m_strMenu12 = _T("");
	m_strMenu21 = _T("");
	m_strMenu22 = _T("");
	m_strMenu31 = _T("");
	m_strMenu32 = _T("");
	m_strMenu41 = _T("");
	m_strMenu42 = _T("");
	m_strMenu51 = _T("");
	m_strMenu52 = _T("");
	m_strDate11 = _T("");
	m_strDate12 = _T("");
	m_strDate21 = _T("");
	m_strDate22 = _T("");
	m_strDate31 = _T("");
	m_strDate32 = _T("");
	m_strDate41 = _T("");
	m_strDate42 = _T("");
	m_strDate51 = _T("");
	m_strDate52 = _T("");
	m_Cal11 = _T("");
	m_Cal12 = _T("");
	m_Cal21 = _T("");
	m_Cal22 = _T("");
	m_Cal31 = _T("");
	m_Cal32 = _T("");
	m_Cal41 = _T("");
	m_Cal42 = _T("");
	m_Cal51 = _T("");
	m_Cal52 = _T("");
	//}}AFX_DATA_INIT
}


void CFoodMenuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFoodMenuDlg)
	DDX_Control(pDX, IDC_DATESELECT, m_FoodMenuDateCtrl);
	DDX_Text(pDX, IDC_EDIT_1stLunchMenu, m_strMenu11);
	DDX_Text(pDX, IDC_EDIT_1stDinnerMenu, m_strMenu12);
	DDX_Text(pDX, IDC_EDIT_2ndLunchMenu, m_strMenu21);
	DDX_Text(pDX, IDC_EDIT_2ndDinnerMenu, m_strMenu22);
	DDX_Text(pDX, IDC_EDIT_3rdLunchMenu, m_strMenu31);
	DDX_Text(pDX, IDC_EDIT_3rdDinnerMenu, m_strMenu32);
	DDX_Text(pDX, IDC_EDIT_4thLunchMenu, m_strMenu41);
	DDX_Text(pDX, IDC_EDIT_4thDinnerMenu, m_strMenu42);
	DDX_Text(pDX, IDC_EDIT_5thLunchMenu, m_strMenu51);
	DDX_Text(pDX, IDC_EDIT_5thDinnerMenu, m_strMenu52);
	DDX_Text(pDX, IDC_STATIC_11, m_strDate11);
	DDX_Text(pDX, IDC_STATIC_12, m_strDate12);
	DDX_Text(pDX, IDC_STATIC_21, m_strDate21);
	DDX_Text(pDX, IDC_STATIC_22, m_strDate22);
	DDX_Text(pDX, IDC_STATIC_31, m_strDate31);
	DDX_Text(pDX, IDC_STATIC_32, m_strDate32);
	DDX_Text(pDX, IDC_STATIC_41, m_strDate41);
	DDX_Text(pDX, IDC_STATIC_42, m_strDate42);
	DDX_Text(pDX, IDC_STATIC_51, m_strDate51);
	DDX_Text(pDX, IDC_STATIC_52, m_strDate52);
	DDX_Text(pDX, IDC_EDIT_1stLunchCal, m_Cal11);
	DDX_Text(pDX, IDC_EDIT_1stDinnerCal, m_Cal12);
	DDX_Text(pDX, IDC_EDIT_2ndLunchCal, m_Cal21);
	DDX_Text(pDX, IDC_EDIT_2ndDinnerCal, m_Cal22);
	DDX_Text(pDX, IDC_EDIT_3rdLunchCal, m_Cal31);
	DDX_Text(pDX, IDC_EDIT_3rdDinnerCal, m_Cal32);
	DDX_Text(pDX, IDC_EDIT_4thLunchCal, m_Cal41);
	DDX_Text(pDX, IDC_EDIT_4thDinnerCal, m_Cal42);
	DDX_Text(pDX, IDC_EDIT_5thLunchCal, m_Cal51);
	DDX_Text(pDX, IDC_EDIT_5thDinnerCal, m_Cal52);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFoodMenuDlg, CDialog)
	//{{AFX_MSG_MAP(CFoodMenuDlg)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATESELECT, OnDatetimechangeDateselect)		
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFoodMenuDlg message handlers

void CFoodMenuDlg::OnBtnSave() 
{
	// TODO: Add your control notification handler code here
	
	int nRtn = AfxMessageBox("저장 하시겠습니까 ?",MB_OKCANCEL);
	
	if(IDOK != nRtn )
	{
		return;
	}

	UpdateData(TRUE);
	
	// FOOD_DAY delim LD_FLAG delim MENU delim CAL 
	m_strMenu.Format(	"%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c%s%c%c%c%s%c%s%c",  
						m_strDate11,DELIM1, '1', DELIM1, m_strMenu11, DELIM1, m_Cal11 , DELIM1 ,
						m_strDate12,DELIM1, '2', DELIM1, m_strMenu12, DELIM1, m_Cal12 , DELIM1 , // 1 st
						m_strDate21,DELIM1, '1', DELIM1, m_strMenu21, DELIM1, m_Cal21 , DELIM1 , 
						m_strDate22,DELIM1, '2', DELIM1, m_strMenu22, DELIM1, m_Cal22 , DELIM1 , // 2nd
						m_strDate31,DELIM1, '1', DELIM1, m_strMenu31, DELIM1, m_Cal31 , DELIM1 , 
						m_strDate32,DELIM1, '2', DELIM1, m_strMenu32, DELIM1, m_Cal32 , DELIM1 , // 3rd
						m_strDate41,DELIM1, '1', DELIM1, m_strMenu41, DELIM1, m_Cal41 , DELIM1 , 
						m_strDate42,DELIM1, '2', DELIM1, m_strMenu42, DELIM1, m_Cal42 , DELIM1 , // 4th
						m_strDate51,DELIM1, '1', DELIM1, m_strMenu51, DELIM1, m_Cal51 , DELIM1 , 
						m_strDate52,DELIM1, '2', DELIM1, m_strMenu52, DELIM1, m_Cal52 , DELIM1   // 5th
					);

	// 파일 데이터 저장	
	CTime iDT =  CTime::GetCurrentTime();
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle(IDMS_MSN_SVR_NAME), szDirTmp, sizeof(szDirTmp));	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for(int i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	TRACE("%s\n" , szDirTmp);		
	CString strTmp;	
	CString strFileNameAll;	

	strcat( szDirTmp , "\\food_menu\\" );

	if (_access( szDirTmp ,0) ==-1 ) 
	{
		CreateDirectory(szDirTmp, NULL);		
	}
	
	strFileNameAll.Format("%s\\%s_FoodMenu.txt", szDirTmp ,m_strMonDayOfWeek );	
	
	writeLogFileW(strFileNameAll, "%s", m_strMenu);
		
	m_bChnged = TRUE;
	CDialog::OnOK();
	
}

void CFoodMenuDlg::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	m_bChnged = FALSE;
	CDialog::OnCancel();
}

void CFoodMenuDlg::SetDateLabel() 
{
	UpdateData(TRUE);

	SYSTEMTIME stDate; 
	m_FoodMenuDateCtrl.GetTime(&stDate);
				
	DWORD nYYYY = stDate.wYear;
	DWORD nMM = stDate.wMonth;
	DWORD nDD = stDate.wDay;	
	
	m_strDate11.Format("%d%0.2d%0.2d", nYYYY, nMM, nDD);
	m_strDate12 = m_strDate11 ;
	//m_strDate12 += " 석식";
	//m_strDate11 += " 중식";	

	CTimeSpan tSp1(1,0,0,0);
	CTimeSpan tSp2(2,0,0,0);
	CTimeSpan tSp3(3,0,0,0);
	CTimeSpan tSp4(4,0,0,0);
	
	CTime Tm(stDate);

	CTime TmpTm2 = Tm + tSp1 ;
	m_strDate21.Format("%d%0.2d%0.2d", TmpTm2.GetYear(), TmpTm2.GetMonth(), TmpTm2.GetDay());
	m_strDate22 = m_strDate21;
	//m_strDate22 += " 석식";
	//m_strDate21 += " 중식";

	CTime TmpTm3 = Tm + tSp2 ;
	m_strDate31.Format("%d%0.2d%0.2d", TmpTm3.GetYear(), TmpTm3.GetMonth(), TmpTm3.GetDay());
	m_strDate32 = m_strDate31;
	//m_strDate32 += " 석식";
	//m_strDate31 += " 중식";

	CTime TmpTm4 = Tm + tSp3 ;
	m_strDate41.Format("%d%0.2d%0.2d", TmpTm4.GetYear(), TmpTm4.GetMonth(), TmpTm4.GetDay());
	m_strDate42 = m_strDate41;
	//m_strDate42 += " 석식";
	//m_strDate41 += " 중식";

	CTime TmpTm5 = Tm + tSp4 ;
	m_strDate51.Format("%d%0.2d%0.2d", TmpTm5.GetYear(), TmpTm5.GetMonth(), TmpTm5.GetDay());
	m_strDate52 = m_strDate51;
	//m_strDate52 += " 석식";
	//m_strDate51 += " 중식";

	UpdateData(FALSE);
}


void CFoodMenuDlg::OnDatetimechangeDateselect(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	SetDateLabel() ;	
	

	*pResult = 0;
}

BOOL CFoodMenuDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	Load( FALSE ) ; 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFoodMenuDlg::Load( BOOL bByOtherWnd) 
{
	int i = 0;
	// TODO: Add your control notification handler code here
	CTime iDT =  CTime::GetCurrentTime();						
		
	int nCurDay = iDT.GetDayOfWeek();	// 1 = Sunday, 2 = Monday, ..., 7 = Saturday
		
	if(nCurDay == 2)
	{
		//월요일		
	}
	else if(nCurDay == 3)
	{
		// 화요일
		iDT -= CTimeSpan(1, 0, 0, 0); 
	}
	else if(nCurDay == 4)
	{
		//수요일
		iDT -= CTimeSpan(2, 0, 0, 0); 
	}
	else if(nCurDay == 5)
	{
		//목
		iDT -= CTimeSpan(3, 0, 0, 0); 
	}
	else if(nCurDay == 6)
	{
		//금
		iDT -= CTimeSpan(4, 0, 0, 0); 
	}				
	
	//CString strFirstDayOfWeek = iDT.Format("%Y%m%d%H%M%S")	;
	if(!bByOtherWnd)
	{
		COleDateTime tmpOleTmDate;
		tmpOleTmDate.SetDate( atoi( (LPCSTR) iDT.Format("%Y")) , atoi( (LPCSTR) iDT.Format("%m")) , atoi( (LPCSTR) iDT.Format("%d") ));
		m_FoodMenuDateCtrl.SetTime(tmpOleTmDate);

		SetDateLabel() ;			
	}

	m_strMonDayOfWeek = iDT.Format("%Y%m%d")	;

	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];

	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	

	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle(IDMS_MSN_SVR_NAME), szDirTmp, sizeof(szDirTmp));	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( i = nLen; i>=0 ; i--)
	{
		if(szDirTmp[i] == '\\' && nPos <1 )
		{
			szDirTmp[i] = '\0';
			nPos++;
			break;
		}
	}
	
	strcat( szDirTmp, "\\food_menu\\" );

	if (_access( (LPCSTR)szDirTmp ,0) ==-1 ) 
	{
		CreateDirectory((LPCSTR)szDirTmp, NULL);		
	}
	
	m_strFileNameAll.Format("%s\\%s_FoodMenu.txt", szDirTmp ,m_strMonDayOfWeek );	

	TRACE("%s\n" , m_strFileNameAll);

	if (_access( (LPCSTR)m_strFileNameAll ,0) ==-1 ) 
	{
		return ;
	}

	CStdioFile fCmdDat(
						m_strFileNameAll,	CFile::modeNoTruncate | CFile::modeReadWrite | CFile::typeText | 
									CFile::shareDenyNone
					  );	
	
	CString rString = "";		

	//if( fCmdDat.ReadString(rString) )
	
	fCmdDat.ReadString(rString) ;
	
	if(rString.IsEmpty())
	{
		return;
	}

	TRACE("rString [%s]\n", rString );
	m_strMenu = rString ;
		
	if(!bByOtherWnd)
	{
		CString subString;
		
		i = 0; 	
		while( AfxExtractSubString(subString, (LPCTSTR)rString, i++, DELIM1 ))
		{
			TRACE("subString [%s]\n", subString );
			if( i<= 4)
			{
				//m_strDate11,DELIM1, '1', DELIM1, m_strMenu11, DELIM1, m_Cal11 , DELIM1 ,
				if(i == 3)
					m_strMenu11 = subString ;
				if(i == 4)
					m_Cal11     = subString ;			
			}
			else if(  i > 4 && i <= 8)
			{				
				if(i == 7)
					m_strMenu12 = subString ;
				if(i == 8)
					m_Cal12     = subString ;			
			}
			else if(  i > 8 && i <= 12)
			{				
				if(i == 11)
					m_strMenu21 = subString ;
				if(i == 12)
					m_Cal21     = subString ;			
			}
			else if(  i > 12 && i <= 16)
			{				
				if(i == 15)
					m_strMenu22 = subString ;
				if(i == 16)
					m_Cal22     = subString ;			
			}
			else if(  i > 16 && i <= 20)
			{				
				if(i == 19)
					m_strMenu31 = subString ;
				if(i == 20)
					m_Cal31     = subString ;			
			}
			else if(  i > 20 && i <= 24)
			{				
				if(i == 23)
					m_strMenu32 = subString ;
				if(i == 24)
					m_Cal32     = subString ;			
			}
			else if(  i > 24 && i <= 28)
			{				
				if(i == 27)
					m_strMenu41 = subString ;
				if(i == 28)
					m_Cal41     = subString ;			
			}
			else if(  i > 28 && i <= 32)
			{				
				if(i == 31)
					m_strMenu42 = subString ;
				if(i == 32)
					m_Cal42     = subString ;			
			}
			else if(  i > 32 && i <= 36)
			{				
				if(i == 35)
					m_strMenu51 = subString ;
				if(i == 36)
					m_Cal51     = subString ;			
			}
			else if(  i > 36 && i <= 40)
			{				
				if(i == 39)
					m_strMenu52 = subString ;
				if(i == 40)
					m_Cal52     = subString ;			
			}			
		} 
		
		UpdateData(FALSE);		
	}

	fCmdDat.Close();
}


BOOL CFoodMenuDlg::PreTranslateMessage(MSG* pMsg) 
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


void CFoodMenuDlg::OnBtnLoad() 
{
	// TODO: Add your control notification handler code here
	CString strFileNameAll ="";
	CString rString = "";
	char szFilter[] = "txt Files (*.txt)|*.txt; *.txt|All Files (*.*)|*.*||";

	CFileDialog tmpFileDlg(FALSE  ,"txt",NULL,NULL,szFilter,NULL);
	int rtn = tmpFileDlg.DoModal();
	
	if(IDCANCEL == rtn)
		return;	
	
	strFileNameAll = tmpFileDlg.GetPathName();

	CStdioFile fCmdDat(
						strFileNameAll,	CFile::modeNoTruncate | CFile::modeReadWrite | CFile::typeText | 
									CFile::shareDenyNone
					  );

	fCmdDat.ReadString(rString) ;
	
	if(rString.IsEmpty())
	{
		return;
	}

	TRACE("rString [%s]\n", rString );
	m_strMenu = rString ;
		
	CString subString;
		
	int i = 0; 	
	
	while( AfxExtractSubString(subString, (LPCTSTR)rString, i++, DELIM1 ))
	{
		TRACE("subString [%s]\n", subString );
		if( i<= 4)
		{
			//m_strDate11,DELIM1, '1', DELIM1, m_strMenu11, DELIM1, m_Cal11 , DELIM1 ,
			if(i == 3)
				m_strMenu11 = subString ;
			if(i == 4)
				m_Cal11     = subString ;			
		}
		else if(  i > 4 && i <= 8)
		{				
			if(i == 7)
				m_strMenu12 = subString ;
			if(i == 8)
				m_Cal12     = subString ;			
		}
		else if(  i > 8 && i <= 12)
		{				
			if(i == 11)
				m_strMenu21 = subString ;
			if(i == 12)
				m_Cal21     = subString ;			
		}
		else if(  i > 12 && i <= 16)
		{				
			if(i == 15)
				m_strMenu22 = subString ;
			if(i == 16)
				m_Cal22     = subString ;			
		}
		else if(  i > 16 && i <= 20)
		{				
			if(i == 19)
				m_strMenu31 = subString ;
			if(i == 20)
				m_Cal31     = subString ;			
		}
		else if(  i > 20 && i <= 24)
		{				
			if(i == 23)
				m_strMenu32 = subString ;
			if(i == 24)
				m_Cal32     = subString ;			
		}
		else if(  i > 24 && i <= 28)
		{				
			if(i == 27)
				m_strMenu41 = subString ;
			if(i == 28)
				m_Cal41     = subString ;			
		}
		else if(  i > 28 && i <= 32)
		{				
			if(i == 31)
				m_strMenu42 = subString ;
			if(i == 32)
				m_Cal42     = subString ;			
		}
		else if(  i > 32 && i <= 36)
		{				
			if(i == 35)
				m_strMenu51 = subString ;
			if(i == 36)
				m_Cal51     = subString ;			
		}
		else if(  i > 36 && i <= 40)
		{				
			if(i == 39)
				m_strMenu52 = subString ;
			if(i == 40)
				m_Cal52     = subString ;			
		}			
	} 
	
	UpdateData(FALSE);		
	
	fCmdDat.Close();
}
