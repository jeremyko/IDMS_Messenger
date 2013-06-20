// ConvEdit.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "ConvEdit.h"
//#include "ChatMSgDlg.h"
//#include "MultiChatMSgDlg.h"
#include "MultiChatWnd.h"
#include "AllMsgDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "imm.h"
#pragma comment(lib,"imm32.lib")

/////////////////////////////////////////////////////////////////////////////
// CConvEdit

CConvEdit::CConvEdit()
{
	m_Brush.CreateSolidBrush(RGB(255, 255, 255));
	m_Font.CreateFont(15, 15, 0, 0, FW_BOLD, FALSE, FALSE,0 , 0, 0, 0, 0, 0, "Verdana");
	m_bEnabled = TRUE;
}

CConvEdit::~CConvEdit()
{
}


BEGIN_MESSAGE_MAP(CConvEdit, CEdit)
	//{{AFX_MSG_MAP(CConvEdit)
	ON_WM_DROPFILES()
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvEdit message handlers

void CConvEdit::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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
	
	if(nFiles == 1)
	{
		//파일하나혹은 폴더한개만
		DragQueryFile(hDropInfo, 0, szFileName, 2048);		
		CFileFind fileFind;	
		bWorking = fileFind.FindFile(szFileName);	
		if(bWorking )
		{
			bWorking = fileFind.FindNextFile();		
			
			{
				// C:\\TEST\\DOC 라면 C:\\TEST 를 ROOT 로 한다.		
				CString strTmpPath(szFileName);
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll
			}
		}		

		m_strFileAll = strRootPath + "|" + m_strFileAll  ;// RootPath|파일경로
		TRACE("Drop File Name Len : [%d] Cnt [%d]\n", m_strFileAll.GetLength(), m_nFileCnt);

		if(m_nParentKind == C_CHATDLG)
		{
			//CChatMSgDlg *pDlg = (CChatMSgDlg *)GetParent();
			//pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		else 
		if(m_nParentKind == C_MULTICHATDLG)
		{
			CMultiChatWnd *pDlg = (CMultiChatWnd *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		else 
		if(m_nParentKind == C_ALLMSGDLG)
		{
			CAllMsgDlg *pDlg = (CAllMsgDlg *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
		
	}
	else
	{	
		// 파일 + 폴더 여러개
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
				CString strTmpPath (szFileName);
				int nPos = strTmpPath.ReverseFind('\\') ;
				strRootPath = strTmpPath.Mid(0, nPos);
				
				FileFindRecurse(szFileName); // directory 이면 내부 파일명들을 얻어온다. -> m_strFileAll				
			}			
		}

		m_strFileAll = strRootPath + "|" + m_strFileAll  ; // RootPath|파일경로
		TRACE("Drop File Name Len : [%d] Cnt [%d]\n", m_strFileAll.GetLength(), m_nFileCnt);
		if(m_nParentKind == C_CHATDLG)
		{
			//CChatMSgDlg *pDlg = (CChatMSgDlg *)GetParent();
			//pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);			
		}
		else 
		if(m_nParentKind == C_MULTICHATDLG)
		{
			CMultiChatWnd *pDlg = (CMultiChatWnd *)GetParent();
			pDlg->FileDropped((LPCTSTR)(LPCSTR)m_strFileAll, m_strFileAll.GetLength(), m_nFileCnt);		
		}
	}
	
	CEdit::OnDropFiles(hDropInfo);
}


void CConvEdit::FileFindRecurse(LPCTSTR pstr)
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
	



void CConvEdit::Enable(BOOL bEnable)
{
	EnableWindow(bEnable);
	m_bEnabled = bEnable;

	if (bEnable)
	{
		m_Brush.DeleteObject();
		m_Brush.CreateSolidBrush(RGB(255, 255, 255));
	}
	else
	{
		m_Brush.DeleteObject();
		m_Brush.CreateSolidBrush(RGB(214, 215, 204));
	}

	Invalidate();
}

int CConvEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	// TODO: Add your specialized creation code here
	DragAcceptFiles(TRUE);
	SetMargins(1, 1);
	SetFont(&m_Font);
	
	return 0;
}

void CConvEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 13)
	{
		//CConversationDlg *Conv = (CConversationDlg *)GetParent();
		//Conv->OnSend();
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CConvEdit::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 0x73 && nFlags == 0x201F) //Alt+S
	{
		//CConversationDlg *Conv = (CConversationDlg *)GetParent();
		//Conv->OnSend();
	}
	else
	CEdit::OnSysChar(nChar, nRepCnt, nFlags);
}

void CConvEdit::SetHan(BOOL bFlag)
{
	HIMC hIMC;

	hIMC = ImmGetContext(m_hWnd);
	if( hIMC == NULL )
		return;

	if( ImmSetConversionStatus(hIMC, bFlag, IME_SMODE_NONE) == 0 )
		return;

	if( ImmReleaseContext(m_hWnd, hIMC) == 0 )
		return;
}
