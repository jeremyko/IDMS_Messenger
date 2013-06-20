// OptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "OptionDlg.h"
#include "MsgBoxThread.h"
#include "INIManager.h"
#include "ChatSession.h"
#include <Mmsystem.h> //sound

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionDlg dialog


COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionDlg)
	m_strCtrlOptionNick = _T("");
	m_nAwayTimeComBo = 0;
	m_ChkUseAutoLogin = FALSE;
	m_bAlwaysOnTop = FALSE;
	m_bUseDownFolder = FALSE;
	m_strDownFolder = _T("");
	m_bUseSound = FALSE;
	m_strLogFolder = _T("");
	m_bUseLogFolder = FALSE;
	//}}AFX_DATA_INIT

	m_bNickChg = FALSE;
	memset(m_szNick , 0x00, sizeof(m_szNick));	

	m_nAwayTimeSecOld = 0;
	m_dwMask = 0;
    m_dwEffect = 0;
	m_bFromMainDlg = TRUE;
}

COptionDlg::COptionDlg( const char* szNick, BOOL bFromMainDlg,  CWnd* pParent  ) 
: CDialog(COptionDlg::IDD, pParent)
{
	m_bNickChg = FALSE;
	strncpy(m_szNick , szNick, sizeof(m_szNick));	
	m_dwMask = 0;
    m_dwEffect = 0;
	m_bFromMainDlg = bFromMainDlg ;
}
void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionDlg)		
	DDX_Control(pDX, IDC_BTN_LOG_FOLDER, m_btnLogFolderPath);
	DDX_Control(pDX, IDC_RICHEDIT_EXAMPLE, m_REditExample);
	DDX_Control(pDX, IDC_COLOURPICKER_BG, m_ColourPickerBG);	
	DDX_Control(pDX, IDC_COLOURPICKER_BUDDY, m_ColourPickerBuddy);
	DDX_Control(pDX, IDC_COLOURPICKER_ME, m_ColourPickerMe);
	DDX_Control(pDX, IDC_CHK_USE_SOUND, m_ChkUseSound);
	DDX_Control(pDX, IDC_CHK_DOWN_FOLDER, m_ChkUseDownFolder);
	DDX_Control(pDX, IDC_CHK_ALWAYSONTOP, m_ChkBtnAlwaysOnTop);
	DDX_Control(pDX, IDC_CHK_AUTO_LOGIN, m_ChkBtnAutoLogIn);
	DDX_Control(pDX, IDC_BTN_SELECT_SOUND, m_btnSelectSound);
	DDX_Control(pDX, IDC_BTN_DOWN_FOLDER, m_btnDownFolderPath);
	DDX_Control(pDX, IDC_EDIT_NICK_NAME, m_EditNick);
	DDX_Text(pDX, IDC_EDIT_NICK_NAME, m_strCtrlOptionNick);
	DDV_MaxChars(pDX, m_strCtrlOptionNick, 100);
	DDX_CBIndex(pDX, IDC_COMBO_AWAY_SEC, m_nAwayTimeComBo);
	DDX_Check(pDX, IDC_CHK_AUTO_LOGIN, m_ChkUseAutoLogin);
	DDX_Check(pDX, IDC_CHK_ALWAYSONTOP, m_bAlwaysOnTop);
	DDX_Check(pDX, IDC_CHK_DOWN_FOLDER, m_bUseDownFolder);
	DDX_Text(pDX, IDC_STATIC_DOWN_FOLER, m_strDownFolder);
	DDX_Check(pDX, IDC_CHK_USE_SOUND, m_bUseSound);
	DDX_Text(pDX, IDC_STATIC_LOG_FOLER, m_strLogFolder);
	DDX_Check(pDX, IDC_CHK_LOG_FOLDER, m_bUseLogFolder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	//{{AFX_MSG_MAP(COptionDlg)
	ON_BN_CLICKED(IDC_BTN_OPT_OK, OnBtnOptOk)
	ON_BN_CLICKED(IDC_BTN_OPT_CANCLE, OnBtnOptCancle)
	ON_BN_CLICKED(IDC_BTN_DOWN_FOLDER, OnBtnDownFolder)
	ON_BN_CLICKED(IDC_CHK_DOWN_FOLDER, OnChkDownFolder)
	ON_BN_CLICKED(IDC_BTN_SELECT_SOUND, OnBtnSelectSound)
	ON_BN_CLICKED(IDC_BTN_STOP_SOUND, OnBtnStopSound)
	ON_BN_CLICKED(IDC_BTN_PLAY_SOUND, OnBtnPlaySound)
	ON_BN_CLICKED(IDC_CHK_USE_SOUND, OnChkUseSound)
	ON_BN_CLICKED(IDC_BUTTON_FONT, OnButtonFont)
	ON_BN_CLICKED(IDC_COLOURPICKER_ME, OnColourpickerMe)	
	ON_BN_CLICKED(IDC_BTN_LOG_FOLDER, OnBtnLogFolder)
	ON_BN_CLICKED(IDC_CHK_LOG_FOLDER, OnChkLogFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionDlg message handlers

void COptionDlg::OnBtnOptOk() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_strCtrlOptionNick.TrimLeft();
	m_strCtrlOptionNick.TrimRight();
	m_strCtrlOptionNick.Replace("'","''");

	if(m_strCtrlOptionNick.GetLength() == 0 )
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"대화명을 입력하세요.");				
		pMsgThread->CreateThread();	
		m_EditNick.SetFocus();
		
		return;
	}
	
	if(m_strCtrlOptionNick.Compare(m_szNick) != 0)
	{
		m_bNickChg = TRUE;
		strncpy(m_szNick, (LPCSTR)m_strCtrlOptionNick, C_NICK_NAME_LEN );
	}

	//자동로그인
	if(m_ChkUseAutoLogin)
	{
		CINIManager::Instance().SetValue(INI_USE_AUTO_LOGIN, "Y");
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_AUTO_LOGIN, "N");
	}

	// 항상위
	if(m_bAlwaysOnTop)
	{
		CINIManager::Instance().SetValue(INI_ALWAYS_ON_TOP, "Y");
	}
	else
	{
		CINIManager::Instance().SetValue(INI_ALWAYS_ON_TOP, "N");
	}

	// 파일 수신 폴더
	if(m_bUseDownFolder)
	{
		CINIManager::Instance().SetValue(INI_USE_DOWNFOLDER, "Y");
		CINIManager::Instance().SetValue(INI_DOWNFOLDER_PATH, (LPSTR)(LPCSTR)m_strDownFolder);
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_DOWNFOLDER, "N");
		CINIManager::Instance().SetValue(INI_DOWNFOLDER_PATH, (LPSTR)(LPCSTR)m_strDownFolder);		
	}

	// 대화내용저장 
	if(m_bUseLogFolder)
	{
		CINIManager::Instance().SetValue(INI_USE_LOGFOLDER, "Y");
		CINIManager::Instance().SetValue(INI_LOGFOLDER_PATH, (LPSTR)(LPCSTR)m_strLogFolder);
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_LOGFOLDER, "N");
		CINIManager::Instance().SetValue(INI_LOGFOLDER_PATH, (LPSTR)(LPCSTR)m_strLogFolder);		
	}
		
	ApplySoundConfig();
	
	//sound
	if(m_bUseSound)
	{
		CINIManager::Instance().SetValue(INI_USE_SOUND, "Y");
		CINIManager::Instance().SetValue(INI_SOUND_PATH, (LPSTR)(LPCSTR)m_strSoundPath);
	}
	else
	{
		CINIManager::Instance().SetValue(INI_USE_SOUND, "N");
		CINIManager::Instance().SetValue(INI_SOUND_PATH, (LPSTR)(LPCSTR)m_strSoundPath);		
	}

	//color buddy	
	char sztmpR[20];
	char sztmpG[20];
	char sztmpB[20];
	CChatSession::Instance().m_ColorBuddy = m_ColourPickerBuddy.GetColour();

	memset(sztmpR, 0x00, sizeof(sztmpR));
	unsigned int nValR = (UINT)GetRValue(m_ColourPickerBuddy.GetColour());
	itoa(nValR, sztmpR, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_COMP_R, sztmpR);
		
	memset(sztmpG, 0x00, sizeof(sztmpG));
	unsigned int nValG = (UINT)GetGValue(m_ColourPickerBuddy.GetColour());
	itoa(nValG, sztmpG, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_COMP_G, sztmpG);

	memset(sztmpB, 0x00, sizeof(sztmpB));
	unsigned int nValB = (UINT)GetBValue(m_ColourPickerBuddy.GetColour());
	itoa(nValB, sztmpB, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_COMP_B, sztmpB);	

	
	//Me
	memset(sztmpR, 0x00, sizeof(sztmpR));
	CChatSession::Instance().m_ColorMe = m_ColourPickerMe.GetColour();

	nValR = (UINT)GetRValue(m_ColourPickerMe.GetColour() );
	itoa(nValR, sztmpR, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_ME_R, sztmpR);
			
	memset(sztmpG, 0x00, sizeof(sztmpG));
	nValG = (UINT)GetGValue(m_ColourPickerMe.GetColour());
	itoa(nValG, sztmpG, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_ME_G, sztmpG);

	memset(sztmpB, 0x00, sizeof(sztmpB));
	nValB = (UINT)GetBValue(m_ColourPickerMe.GetColour());
	itoa(nValB, sztmpB, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_ME_B, sztmpB);	
	
	
	//BG
	CChatSession::Instance().m_ColorBG = m_ColourPickerBG.GetColour();

	memset(sztmpR, 0x00, sizeof(sztmpR));
	nValR = (UINT)GetRValue( m_ColourPickerBG.GetColour());
	itoa(nValR, sztmpR, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_BG_R, sztmpR);
		
	memset(sztmpG, 0x00, sizeof(sztmpG));
	nValG = (UINT)GetGValue( m_ColourPickerBG.GetColour() );
	itoa(nValG, sztmpG, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_BG_G, sztmpG);

	memset(sztmpB, 0x00, sizeof(sztmpB));
	nValB = (UINT)GetBValue( m_ColourPickerBG.GetColour() );
	itoa(nValB, sztmpB, 10);		
	CINIManager::Instance().SetValue(INI_COLOR_BG_B, sztmpB);	
	
	
	//
	char szSetVal[10+1];
	memset(szSetVal, 0x00,sizeof(szSetVal));	
	switch(m_nAwayTimeComBo)	
	{
	case 0: // 1 min
		CChatSession::Instance().m_AwaySec = 1 * 60; 
		break;
	case 1: // 3 min
		CChatSession::Instance().m_AwaySec = 3 * 60; 
		break;
	case 2: // 5 min
		CChatSession::Instance().m_AwaySec = 5 * 60; 
		break;
	case 3: // 10 min
		CChatSession::Instance().m_AwaySec = 10 * 60; 
		break;
	case 4: // 15 min
		CChatSession::Instance().m_AwaySec = 15 * 60; 
		break;
	case 5: // 20 min
		CChatSession::Instance().m_AwaySec = 20 * 60; 
		break;
	case 6: // 30 min
		CChatSession::Instance().m_AwaySec = 30 * 60; 
		break;
	case 7: // 설정안함
		CChatSession::Instance().m_AwaySec = -1;  
		break;
	}
	
	sprintf(szSetVal, "%d", CChatSession::Instance().m_AwaySec);	
	CINIManager::Instance().SetValue(INI_AWAY_SEC , szSetVal);

	//Font
	CChatSession::Instance().m_cf.dwMask = m_dwMask;		
    CChatSession::Instance().m_cf.dwEffects = m_dwEffect;		
    CChatSession::Instance().m_cf.yHeight = - m_lf.lfHeight*15;		
    CChatSession::Instance().m_cf.crTextColor = m_ColourPickerMe.GetColour(); ; //CChatSession::Instance().m_ColorMe;		
    CChatSession::Instance().m_cf.bCharSet =  m_lf.lfCharSet;		
    CChatSession::Instance().m_cf.bPitchAndFamily = m_lf.lfPitchAndFamily;			
	CChatSession::Instance().m_cf.crBackColor = CChatSession::Instance().m_ColorBG;        
    strcpy(CChatSession::Instance().m_cf.szFaceName, (LPCTSTR)m_fn);				
	CChatSession::Instance().m_Font.Detach();
	CChatSession::Instance().m_Font.CreateFontIndirect(&m_lf);
	BOOL nRtn = AfxGetApp()->WriteProfileBinary(INI_SEC_CONVER_FONT, INI_FONT, (LPBYTE)&m_lf, sizeof(LOGFONT));		
	

	CINIManager::Instance().SaveINIFile();

	// 대화명 변경 작업			
	if(m_bNickChg)
	{		
		if(CChatSession::Instance().RequestChgNick( CChatSession::Instance().mMyInfo.m_szNickName, m_szNick) != TRUE)
		{
			TRACE("CChatSession::RequestChgNick return FAIL!\n");		
			return ;
		}	

		CChatSession::Instance().m_Nick = m_szNick;
		strncpy(CChatSession::Instance().mMyInfo.m_szNickName, m_szNick, sizeof(CChatSession::Instance().mMyInfo.m_szNickName) ); 
		CChatSession::Instance().m_strMyNick = " (온라인) " + CChatSession::Instance().m_Nick ;
	}	
    
	//GetParent()->PostMessage(WM_CHG_CONFIG, NULL, NULL);
	
	m_ExampleFont.DeleteObject();

	CDialog::OnOK();
}

int COptionDlg::ApplySoundConfig()
{
	GetDlgItemText(IDC_STATIC_SOUND_PATH, m_strSoundPath);
	
	sndPlaySound(NULL, SND_ASYNC);
	
	return TRUE;
}

void COptionDlg::OnBtnOptCancle() 
{
	// TODO: Add your control notification handler code here
	sndPlaySound(NULL, SND_ASYNC);
	m_bNickChg = FALSE;
	m_ExampleFont.DeleteObject();
	CDialog::OnCancel();
}

BOOL COptionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	if(!m_bFromMainDlg)
	{
		m_ChkBtnAlwaysOnTop.EnableWindow(FALSE);
	}
	else
	{
		m_ChkBtnAlwaysOnTop.EnableWindow(TRUE);
	}

	CChatSession::Instance().m_Font.GetLogFont(&m_lf); 	
	m_ExampleFont.CreateFontIndirect(&m_lf); //Copy
	
	m_REditExample.Initialize();
	m_REditExample.SetRole(ROLE_DISPLAY);

	TRACE("m_szNick [%s]\n", m_szNick);

	m_strCtrlOptionNick = m_szNick ;
	m_strCtrlOptionNick.Replace("''","'");
	
	switch(m_nAwayTimeSecOld)	
	{
	case 1 * 60: // 1 min
		m_nAwayTimeComBo = 0;
		break;
	case 3 * 60: // 3 min
		m_nAwayTimeComBo = 1;
		break;
	case 5 * 60: // 5 min		
		m_nAwayTimeComBo = 2;
		break;
	case 10 * 60: // 10 min		
		m_nAwayTimeComBo = 3;
		break;
	case 15 * 60: // 15 min		
		m_nAwayTimeComBo = 4;
		break;
	case 20 * 60: // 20 min		
		m_nAwayTimeComBo = 5;
		break;
	case 30 * 60: // 30 min		
		m_nAwayTimeComBo = 6;
		break;
	case -1: // 설정안함		
		m_nAwayTimeComBo = 7;
		break;
	}
	
	CString strVal;
	CINIManager::Instance().GetValue(INI_USE_AUTO_LOGIN, strVal);

	if(strVal.Compare("Y") == 0)
	{
		m_ChkUseAutoLogin = TRUE;
	}

	CString strAlwaysTop;
	CINIManager::Instance().GetValue(INI_ALWAYS_ON_TOP, strAlwaysTop);

	if(strAlwaysTop == "Y")
		m_bAlwaysOnTop = TRUE; 
	else
		m_bAlwaysOnTop = FALSE; 
	
	//INI_USE_DOWNFOLDER
	CString strUseDownFolder;
	CINIManager::Instance().GetValue(INI_USE_DOWNFOLDER, strUseDownFolder);

	if(strUseDownFolder == "Y")
	{
		m_nFlipFlopDownFolder = 1;
		m_bUseDownFolder = TRUE; 
		m_btnDownFolderPath.EnableWindow(TRUE);
	}
	else
	{   
		m_nFlipFlopDownFolder = -1 ;
		m_bUseDownFolder = FALSE; 
		m_btnDownFolderPath.EnableWindow(FALSE);
	}

	// Download Path
	CINIManager::Instance().GetValue(INI_DOWNFOLDER_PATH, m_strDownFolder);

	//Log
	CString strUseLogFolder;
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);

	if(strUseLogFolder == "Y")
	{
		m_nFlipFlopLogFolder = 1;
		m_bUseLogFolder = TRUE; 
		m_btnLogFolderPath.EnableWindow(TRUE);
	}
	else
	{   
		m_nFlipFlopLogFolder = -1 ;
		m_bUseLogFolder = FALSE; 
		m_btnLogFolderPath.EnableWindow(FALSE);
	}
	CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, m_strLogFolder);
	
	//SOUND
	CString strUseSound;
	CINIManager::Instance().GetValue(INI_USE_SOUND, strUseSound);

	if(strUseSound == "Y")
	{
		m_nFlipFlopSound = 1;
		m_bUseSound = TRUE; 
		m_btnSelectSound.EnableWindow(TRUE);
		CINIManager::Instance().GetValue(INI_SOUND_PATH, m_strSoundPath);
	}
	else
	{   
		m_nFlipFlopSound = -1 ;
		m_bUseSound = FALSE; 
		m_btnSelectSound.EnableWindow(FALSE);
		CINIManager::Instance().GetValue(INI_SOUND_PATH, m_strSoundPath);
	}
	SetDlgItemText(IDC_STATIC_SOUND_PATH, m_strSoundPath);
	
	/*
	m_ChkBtnAutoLogIn.SetIcon( IDI_ICON_CHK_ON, IDI_ICON_CHK_OFF);
	m_ChkBtnAutoLogIn.DrawBorder(FALSE);
	m_ChkBtnAutoLogIn.EnableBalloonTooltip();
	
	m_ChkBtnAlwaysOnTop.SetIcon( IDI_ICON_CHK_ON, IDI_ICON_CHK_OFF);
	m_ChkBtnAlwaysOnTop.DrawBorder(FALSE);
	m_ChkBtnAlwaysOnTop.EnableBalloonTooltip();
	
	m_ChkUseDownFolder.SetIcon( IDI_ICON_CHK_ON, IDI_ICON_CHK_OFF);
	m_ChkUseDownFolder.DrawBorder(FALSE);
	m_ChkUseDownFolder.EnableBalloonTooltip();
	
	m_ChkUseSound.SetIcon( IDI_ICON_CHK_ON, IDI_ICON_CHK_OFF);
	m_ChkUseSound.DrawBorder(FALSE);
	m_ChkUseSound.EnableBalloonTooltip();	
	*/
	
	UpdateData(FALSE);
	
	//m_ColourMe.SetSelectionMode(m_nMode? CP_MODE_BK: CP_MODE_TEXT);
	m_ColourPickerMe.SetSelectionMode( CP_MODE_BK);
	//m_ColourBuddy.SetSelectionMode(m_nMode? CP_MODE_BK: CP_MODE_TEXT);
	m_ColourPickerBuddy.SetSelectionMode(CP_MODE_BK);
	m_ColourPickerBG.SetSelectionMode(CP_MODE_BK);
	
	//Bg
	CString szColorBGR, szColorBGG , szColorBGB;	
	CINIManager::Instance().GetValue( INI_COLOR_BG_R, szColorBGR);
	CINIManager::Instance().GetValue( INI_COLOR_BG_G, szColorBGG);
	CINIManager::Instance().GetValue( INI_COLOR_BG_B, szColorBGB);	
	CChatSession::Instance().m_ColorBG = RGB	(
						atoi( szColorBGR),
						atoi( szColorBGG),
						atoi( szColorBGB)
					);

	m_ColourPickerBG.SetColour(CChatSession::Instance().m_ColorBG);
	
	//Buddy
	CString szColorCompR, szColorCompG , szColorCompB;	
	CINIManager::Instance().GetValue( INI_COLOR_COMP_R, szColorCompR);
	CINIManager::Instance().GetValue( INI_COLOR_COMP_G, szColorCompG);
	CINIManager::Instance().GetValue( INI_COLOR_COMP_B, szColorCompB);
	
	CChatSession::Instance().m_ColorBuddy = RGB	(
							atoi( (LPCSTR)szColorCompR),
							atoi( (LPCSTR)szColorCompG),
							atoi( (LPCSTR)szColorCompB)
						);
	m_ColourPickerBuddy.SetColour(CChatSession::Instance().m_ColorBuddy);

	//Me
	CString szColorMeR, szColorMeG , szColorMeB;	
	CINIManager::Instance().GetValue( INI_COLOR_ME_R, szColorMeR);
	CINIManager::Instance().GetValue( INI_COLOR_ME_G, szColorMeG);
	CINIManager::Instance().GetValue( INI_COLOR_ME_B, szColorMeB);

	CChatSession::Instance().m_ColorMe = RGB	(
						atoi( (LPCSTR)szColorMeR),
						atoi( (LPCSTR)szColorMeG),
						atoi( (LPCSTR)szColorMeB)
					);
	m_ColourPickerMe.SetColour(CChatSession::Instance().m_ColorMe);	
	
	
	// RichEdit
	m_REditExample.SetFont(&CChatSession::Instance().m_Font);
	m_REditExample.SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );

	m_REditExample.SetSel(0, -1);	
	m_REditExample.ReplaceSel( "" );
	
	int nStart = m_REditExample.GetWindowTextLength();
	m_REditExample.SetSel(nStart, nStart);		
	
	CString strMsg;	
	strMsg.Format("ec12 상대방 Message\r\n");	
	m_REditExample.ReplaceSel(strMsg);
	
	int lenAft = m_REditExample.GetWindowTextLength();
	m_REditExample.SetSel(nStart, lenAft);	
	
	CHARFORMAT2 cf;
	m_REditExample.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
	cf.crTextColor = m_ColourPickerBuddy.GetColour();
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
	LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);
	strcpy(cf.szFaceName, lf.lfFaceName);
	m_REditExample.SetSelectionCharFormat(cf);
	
	// Me
	int len = m_REditExample.GetWindowTextLength();
	m_REditExample.SetSel(len, len);		
				
	CString strMsgTmp;
	strMsgTmp.Format("\nec37 내가 보내는 Message ec36\r\n");		
	m_REditExample.ReplaceSel(strMsgTmp);	
	m_REditExample.HideSelection(TRUE, TRUE);		
	lenAft = m_REditExample.GetWindowTextLength();
	m_REditExample.SetSel(len, lenAft);			
	//CHARFORMAT2 cf;
	m_REditExample.GetSelectionCharFormat(cf);		
	cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
	//LOGFONT lf;
	CChatSession::Instance().m_Font.GetLogFont(&lf);	
	strcpy(cf.szFaceName, lf.lfFaceName);		
	cf.crTextColor = m_ColourPickerMe.GetColour();
	cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
	m_REditExample.SetSelectionCharFormat(cf);      		
	m_REditExample.HideSelection(TRUE, TRUE); // ! 		
	::PostMessage(m_REditExample.GetSafeHwnd(), EM_SCROLLCARET, 0,0);	


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void COptionDlg::OnBtnDownFolder() 
{
	UpdateData(TRUE); 
	// TODO: Add your control notification handler code here
	BROWSEINFO bi = { 0 };    
    char  szPath[2000];
	memset(szPath, 0x00, sizeof(szPath));
	//CString strPath ;

    bi.lpszTitle = _T("파일 저장 경로 선택");
    //bi.pszDisplayName = (LPSTR)(LPCSTR)strPath /*szPath*/;
	bi.pszDisplayName = szPath;
	bi.hwndOwner = this->GetSafeHwnd();	
	bi.lpszTitle ="파일을 받을때 항상 여기로 저장됩니다";
	bi.ulFlags = BIF_NEWDIALOGSTYLE ; // platform SDK 필요함 

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
        // get the name of the folder        
		//::SHGetPathFromIDList(pidl, (LPSTR)(LPCSTR)strPath /*szPath*/ );
		::SHGetPathFromIDList(pidl, szPath );

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
	}	
		
	//TRACE( "strPath   [%d] [%d]\n", strPath.GetLength() , strlen(strPath.GetBuffer(5000)) );
	//TRACE( "m_strDownFolder [%d]\n", m_strDownFolder.GetLength() );

	if( strlen(szPath) == 0  )
	//if(strPath.GetLength() == 0 )
	{
		if( m_strDownFolder.IsEmpty())
		{
			m_bUseDownFolder = FALSE;
			m_btnDownFolderPath.EnableWindow(FALSE);
			m_nFlipFlopDownFolder  = -1 ;
		}		
	}
	else
	{
		m_strDownFolder = szPath ;
		TRACE( "strPath         [%d]\n", strlen(szPath) );
		TRACE( "m_strDownFolder [%d]\n", m_strDownFolder.GetLength() );
		m_bUseDownFolder = TRUE;
		m_btnDownFolderPath.EnableWindow(TRUE);
		m_nFlipFlopDownFolder  = 1 ;
	}

	UpdateData(FALSE);
	
}

void COptionDlg::OnChkDownFolder() 
{
	// TODO: Add your control notification handler code here
	m_nFlipFlopDownFolder  *= -1 ;

	if(m_nFlipFlopDownFolder  == 1)
	{
		m_btnDownFolderPath.EnableWindow(TRUE);
	}
	else
	{
		m_btnDownFolderPath.EnableWindow(FALSE);
	}	
}

void COptionDlg::OnBtnSelectSound() 
{
	// TODO: Add your control notification handler code here
	FileDialogOpen(m_strTempSoundPath);

	if(m_strTempSoundPath != "")
	{
		SetDlgItemText(IDC_STATIC_SOUND_PATH, m_strTempSoundPath);
	}
}

void COptionDlg::OnBtnStopSound() 
{
	// TODO: Add your control notification handler code here
	sndPlaySound(NULL, SND_ASYNC);
}

void COptionDlg::OnBtnPlaySound() 
{
	// TODO: Add your control notification handler code here
	sndPlaySound(NULL, SND_ASYNC);
	if(m_strTempSoundPath != "")
		sndPlaySound((LPCSTR)m_strTempSoundPath, SND_ASYNC  );
	else
		sndPlaySound((LPCSTR)m_strSoundPath, SND_ASYNC );
}

void COptionDlg::FileDialogOpen(CString &result)
{
	char szFilter[] = "Wav Files (*.wav)|*.wav; *.wav|All Files (*.*)|*.*||";
	CFileDialog tmpFileDlg(TRUE  ,"wav",NULL,NULL,szFilter,NULL);
	int rtn = tmpFileDlg.DoModal();
	if(IDCANCEL == rtn)
		return;	
	result = tmpFileDlg.GetPathName();
}

void COptionDlg::OnChkUseSound() 
{
	// TODO: Add your control notification handler code here
	m_nFlipFlopSound  *= -1 ;

	if(m_nFlipFlopSound  == 1)
	{
		m_btnSelectSound.EnableWindow(TRUE);
	}
	else
	{
		m_btnSelectSound.EnableWindow(FALSE);
	}
}

void COptionDlg::OnButtonFont() 
{
	// TODO: Add your control notification handler code here
	
	//CHARFORMAT2 cf;
	CChatSession::Instance().m_cf.cbSize = sizeof(CHARFORMAT2);
    
    ZeroMemory(&m_lf, sizeof(LOGFONT));	
	ZeroMemory(&CChatSession::Instance().m_cf, sizeof(CChatSession::Instance().m_cf));	
	
	//CChatSession::Instance().m_Font.GetLogFont(&m_lf); 	
	
	m_ExampleFont.GetLogFont(&m_lf); 	
    CFontDialog dlg(& m_lf);	
	dlg.m_cf.lpLogFont = &m_lf;     
	
	dlg.m_cf.rgbColors = CChatSession::Instance().m_ColorMe ; 		
		
    if(dlg.DoModal() == IDOK)		
    {		
        m_lf = *dlg.m_cf.lpLogFont;		
		
        m_dwMask = CFM_FACE | CFM_SIZE |  CFM_BACKCOLOR |  CFM_PROTECTED;		//| CFM_COLOR
        m_dwEffect = 0;		
		
        if(dlg.IsBold()) { m_dwMask |= CFM_BOLD; m_dwEffect |= CFE_BOLD; }		
        if(dlg.IsItalic()) { m_dwMask |= CFM_ITALIC; m_dwEffect |= CFE_ITALIC; }		
        if(dlg.IsStrikeOut()) { m_dwMask |= CFM_STRIKEOUT; m_dwEffect |= CFE_STRIKEOUT; }		
        if(dlg.IsUnderline()) { m_dwMask |= CFM_UNDERLINE; m_dwEffect |= CFE_UNDERLINE; }		
		m_fn = dlg.GetFaceName();
		
		m_ExampleFont.DeleteObject();
		m_ExampleFont.CreateFontIndirect(&m_lf);
		m_REditExample.SetFont(&m_ExampleFont);

		m_REditExample.SetBackgroundColor(FALSE, CChatSession::Instance().m_ColorBG );

		m_REditExample.SetSel(0, -1);	
		m_REditExample.ReplaceSel( "" );
		
		int nStart = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(nStart, nStart);		
		
		CString strMsg;	
		strMsg.Format("ec12 상대방 Message\r\n");	
		m_REditExample.ReplaceSel(strMsg);
		
		int lenAft = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(nStart, lenAft);	
		
		CHARFORMAT2 cf;
		LOGFONT		lf;

		m_REditExample.GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
		cf.crTextColor = m_ColourPickerBuddy.GetColour();
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		m_ExampleFont.GetLogFont(&lf);
		strcpy(cf.szFaceName, lf.lfFaceName);
		m_REditExample.SetSelectionCharFormat(cf);
		
		// Me
		int len = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(len, len);		
					
		CString strMsgTmp;
		strMsgTmp.Format("\nec37 내가 보내는 Message ec36\r\n");		
		m_REditExample.ReplaceSel(strMsgTmp);	
		m_REditExample.HideSelection(TRUE, TRUE);		
		lenAft = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(len, lenAft);			
		
		m_REditExample.GetSelectionCharFormat(cf);		
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
		
		m_ExampleFont.GetLogFont(&lf);	
		strcpy(cf.szFaceName, lf.lfFaceName);		
		cf.crTextColor = m_ColourPickerMe.GetColour();
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
		m_REditExample.SetSelectionCharFormat(cf);      		
		m_REditExample.HideSelection(TRUE, TRUE); // ! 		
		::PostMessage(m_REditExample.GetSafeHwnd(), EM_SCROLLCARET, 0,0);

    }
}

void COptionDlg::OnColourpickerMe() 
{
	// TODO: Add your control notification handler code here
	
}



LRESULT COptionDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(message == CPN_SELENDOK)
	{
		TRACE("CPN_SELCHANGE\n");
		
		COLORREF ColBG = m_ColourPickerBG.GetColour() ;
		/*
		char sztmp[20];
		memset(sztmp, 0x00, sizeof(sztmp));
		unsigned int nVal = (UINT)GetRValue(ColBG);
		itoa(nVal, sztmp, 10);		
		TRACE("R %s\n", sztmp);

		memset(sztmp, 0x00, sizeof(sztmp));
		nVal = (UINT)GetGValue(ColBG);
		itoa(nVal, sztmp, 10);		
		TRACE("G %s\n", sztmp);

		memset(sztmp, 0x00, sizeof(sztmp));
		nVal = (UINT)GetBValue(ColBG);
		itoa(nVal, sztmp, 10);		
		TRACE("B %s\n", sztmp);
		*/

		//m_REditExample.SetBackgroundColor(FALSE, ColBG );

		::PostMessage( m_REditExample.GetSafeHwnd(), EM_SETBKGNDCOLOR, 
						(WPARAM) FALSE,  (LPARAM) ColBG   );


		//
		m_REditExample.SetSel(0, -1);	
		m_REditExample.ReplaceSel( "" );
	
		int nStart = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(nStart, nStart);		
		
		CString strMsg;		
		strMsg.Format("ec12 상대방 Message\r\n");	
		m_REditExample.ReplaceSel(strMsg);
		int lenAft = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(nStart, lenAft);			
		CHARFORMAT2 cf;
		m_REditExample.GetSelectionCharFormat(cf);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
		cf.crTextColor = m_ColourPickerBuddy.GetColour();
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;	
		LOGFONT lf;
		CChatSession::Instance().m_Font.GetLogFont(&lf);			
		strcpy(cf.szFaceName, lf.lfFaceName);
		m_REditExample.SetSelectionCharFormat(cf);
		
		// Me
		int len = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(len, len);		
				
		CString strMsgTmp;
		strMsgTmp.Format("\r\nec37 내가 보내는 Message ec36\r\n");		
		m_REditExample.ReplaceSel(strMsgTmp);	
		m_REditExample.HideSelection(TRUE, TRUE);		
		lenAft = m_REditExample.GetWindowTextLength();
		m_REditExample.SetSel(len, lenAft);			
		//CHARFORMAT2 cf;
		m_REditExample.GetSelectionCharFormat(cf);		
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE  ;
		//LOGFONT lf;
		CChatSession::Instance().m_Font.GetLogFont(&lf);	
		strcpy(cf.szFaceName, lf.lfFaceName);		
		cf.crTextColor = m_ColourPickerMe.GetColour();
		cf.dwEffects &=(unsigned long) ~CFE_AUTOCOLOR;
		m_REditExample.SetSelectionCharFormat(cf);      		
		m_REditExample.HideSelection(TRUE, TRUE); // ! 		
		::PostMessage(m_REditExample.GetSafeHwnd(), EM_SCROLLCARET, 0,0);		
		
		m_REditExample.RedrawWindow();
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void COptionDlg::OnBtnLogFolder() 
{	
	UpdateData(TRUE); 
	
	BROWSEINFO bi = { 0 };    
    char  szPath[2000];
	memset(szPath, 0x00, sizeof(szPath));
	
    bi.lpszTitle = _T("대화내용 저장 경로 선택");    
	bi.pszDisplayName = szPath;
	bi.hwndOwner = this->GetSafeHwnd();	
	bi.ulFlags = BIF_NEWDIALOGSTYLE ; // platform SDK 필요함 

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
		::SHGetPathFromIDList(pidl, szPath );
        
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
	}		
	
	if( strlen(szPath) == 0  )	
	{
		if( m_strLogFolder.IsEmpty())
		{
			m_bUseLogFolder = FALSE;
			m_btnLogFolderPath.EnableWindow(FALSE);
			m_nFlipFlopLogFolder  = -1 ;
		}		
	}
	else
	{
		m_strLogFolder = szPath ;
		TRACE( "strPath         [%d]\n", strlen(szPath) );
		TRACE( "m_strLogFolder [%d]\n", m_strLogFolder.GetLength() );
		m_bUseLogFolder = TRUE;
		m_btnLogFolderPath.EnableWindow(TRUE);
		m_nFlipFlopLogFolder  = 1 ;
	}

	UpdateData(FALSE);
}

void COptionDlg::OnChkLogFolder() 
{
	// TODO: Add your control notification handler code here
	m_nFlipFlopLogFolder  *= -1 ;

	if(m_nFlipFlopLogFolder  == 1)
	{
		m_btnLogFolderPath.EnableWindow(TRUE);
	}
	else
	{
		m_btnLogFolderPath.EnableWindow(FALSE);
	}	
}
