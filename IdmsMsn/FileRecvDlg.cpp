// FileRecvDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "filetransferdlg.h"
#include "FileRecvDlg.h"
#include "FileTransSock.h"
#include "INIManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileRecvDlg dialog


CFileRecvDlg::CFileRecvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileRecvDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileRecvDlg)
	m_strConFirmEdit = _T("");
	m_StrRcvAmt = _T("");
	//}}AFX_DATA_INIT

	m_bIsCanceled = FALSE;
	m_pFiles = NULL;
	m_pSockObj = NULL;
	m_nTotalFileCnt = 0;
	m_nRecvedFileCnt = 0;
	memset(m_szSavePath,0x00,sizeof(m_szSavePath));

	m_nRecvBytes = 0;
	m_nTotalBytes = 0;
	m_nPrePercent = 0;

	m_hFile = 0;
}

CFileRecvDlg::CFileRecvDlg( ST_RCV_SOMEONE_WANT_SEND_FILE_S* pFiles ,SOCKET_OBJ_FILE* pSock)
{
	m_bIsCanceled = FALSE;
	m_nTotalFileCnt = 0;
	m_nRecvedFileCnt = 0;
	m_pFiles = NULL;
	m_pSockObj = NULL;

	m_pFiles =  pFiles ;
	m_pSockObj =  pSock ;

	m_nRecvBytes = 0;
	m_nTotalBytes = 0;
	m_nPrePercent = 0;

	m_hFile = 0;
	
	memset(m_szSavePath,0x00,sizeof(m_szSavePath));
	
	memset(m_szSockObjID, 0x00, sizeof(m_szSockObjID));
	strcpy(m_szSockObjID,m_pSockObj->m_szSockObjID );
}

CFileRecvDlg::~CFileRecvDlg ()
{
	if(m_hFile)
	{
		CloseHandle(m_hFile);			
		m_hFile = 0;
	}

	if(m_pFiles)
	{
		delete m_pFiles;	
		m_pFiles = NULL;
	}
}

void CFileRecvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileRecvDlg)
	DDX_Control(pDX, IDC_BTN_OPEN_FILE_FOLDER, m_BtnOpenFolder);
	DDX_Control(pDX, IDC_BTN_FILE_CONFIRM, m_BtnOK);
	DDX_Control(pDX, IDC_BTN_FILE_NOT_CONFIRM, m_BtnCancel);
	DDX_Control(pDX, IDC_LIST_RECV, m_ListCtrlFileRecv);
	DDX_Control(pDX, IDC_RECV_BYTES_PROGRESS, m_RecvBytesProgress);
	DDX_Control(pDX, IDC_RECEIVE_PROGRESS, m_RecvFileProgress);	
	DDX_Control(pDX, IDC_EDIT_CONFIRM, m_EditConFirm);
	DDX_Text(pDX, IDC_EDIT_CONFIRM, m_strConFirmEdit);
	DDX_Text(pDX, IDC_STATIC_RECV_AMT, m_StrRcvAmt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileRecvDlg, CDialog)
	//{{AFX_MSG_MAP(CFileRecvDlg)
	ON_BN_CLICKED(IDC_BTN_FILE_CONFIRM, OnBtnFileConfirm)
	ON_BN_CLICKED(IDC_BTN_FILE_NOT_CONFIRM, OnBtnFileNotConfirm)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_OPEN_FILE_FOLDER, OnBtnOpenFileFolder)
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_MAKE_THIS_FILE,			OnFileRecved )	
	ON_MESSAGE(WM_CANCEL_FILE_TRANSFER,		OnCancelFileTransfer )	
	ON_MESSAGE(WM_RECV_BYTES,		OnBytesRecv )	
	ON_MESSAGE(WM_DISCONNECTED,				OnDisconnected	)	
	
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileRecvDlg message handlers

void CFileRecvDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);

	m_cControlPos.AddControl(IDC_EDIT_CONFIRM	, CP_RESIZE_HORIZONTAL );	
	m_cControlPos.AddControl(IDC_LIST_RECV	, CP_RESIZE_HORIZONTAL  |  CP_RESIZE_VERTICAL  );		
	m_cControlPos.AddControl(IDC_RECV_BYTES_PROGRESS, CP_MOVE_VERTICAL |CP_RESIZE_HORIZONTAL    );		
	m_cControlPos.AddControl(IDC_RECEIVE_PROGRESS, CP_MOVE_VERTICAL |CP_RESIZE_HORIZONTAL    );		
	m_cControlPos.AddControl(IDC_BTN_FILE_CONFIRM	, CP_MOVE_VERTICAL |CP_MOVE_HORIZONTAL  );		
	m_cControlPos.AddControl(IDC_BTN_OPEN_FILE_FOLDER	, CP_MOVE_VERTICAL |CP_MOVE_HORIZONTAL  );			
	m_cControlPos.AddControl(IDC_BTN_FILE_NOT_CONFIRM	, CP_MOVE_VERTICAL |CP_MOVE_HORIZONTAL  );		

	m_cControlPos.AddControl(IDC_STATIC_FILE	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_TOTAL	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_BYTE	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_RECV_AMT	, CP_MOVE_VERTICAL   );	
}

LRESULT CFileRecvDlg::OnBytesRecv (WPARAM wParam, LPARAM lParam)
{		
	DWORD dwWritten ;
	ST_RCVED_BYTES_INFO* pRecvBytes =  (ST_RCVED_BYTES_INFO*) wParam;
		
	m_nRecvBytes+= pRecvBytes->receive_length  ;	
	
	m_nTotalBytes = _atoi64 (pRecvBytes->sock->sPacketHeader.szFileLength);
			
	m_StrRcvAmt.Format("%I64d / %I64d", m_nRecvBytes, m_nTotalBytes );
	
	float nPercent = 0;
	
	if(m_nTotalBytes == 0 )
	{
		TRACE("File Size 0 !\n");
		nPercent = 100;
		m_RecvBytesProgress.SetPos(100);
		m_RecvFileProgress.SetPos(100);
		
	}
	else
	{
		nPercent = ((float) m_nRecvBytes / (float) m_nTotalBytes) * (float) 100 ;
	}
		
	if( (int) nPercent >= 100)
	{
		if(m_nPrePercent==0)
		{
			m_RecvBytesProgress.SetPos(0);
			UpdateData(FALSE);
		}

		nPercent = 100;

		m_StrRcvAmt.Format("%I64d / %I64d", m_nTotalBytes, m_nTotalBytes ); // --
	}
	
	if( (int) nPercent <=0)
		nPercent =0;
	
	TRACE("Recv Bytes[%d] : %I64d / %I64d  percent [%d] \n", pRecvBytes->receive_length, m_nRecvBytes,m_nTotalBytes, (int)nPercent );

	m_RecvBytesProgress.SetPos(nPercent);
	
	if(m_nTotalFileCnt == 1 )
	{
		m_RecvFileProgress.SetPos(nPercent);
	}

	UpdateData(FALSE);
	
	// 파일을 쓴다..
	if( m_hFile == 0 )
	{		
		//CloseHandle(m_hFile);

		if(m_szSavePath[ strlen(m_szSavePath) - 1 ] != '\\')
		{
			strcat(m_szSavePath , "\\");
		}

		char szFilePath		[360+1];
		char szSaveFilePath [360+1];		
		memset(szFilePath, 0x00,sizeof(szFilePath));
		memset(szSaveFilePath, 0x00,sizeof(szSaveFilePath));

		strncpy(szFilePath, pRecvBytes->sFileHeader->szFilePath , sizeof(szFilePath)); 
						
		if	(	strncmp	(	szFilePath + ( strlen(szFilePath) - strlen("_OpenedMark_") ) , 
							"_OpenedMark_", 
							strlen("_OpenedMark_") 
						) == 0 
			)
		{
			char szFilePathTmp [360+1];
			memset(szFilePathTmp, 0x00,sizeof(szFilePathTmp));

			strncpy(szFilePathTmp, szFilePath, ( strlen(szFilePath) - strlen("_OpenedMark_") ) );
			strcpy(szFilePath, szFilePathTmp);
		}

		strcat(szSaveFilePath, m_szSavePath		);
		strcat(szSaveFilePath, szFilePath		);
		
		string strFileName = string(szSaveFilePath) ;
		TRACE("%s\n",strFileName.c_str());
		string strTmpPath = strFileName;
		int nPos = strTmpPath.rfind('\\') ;
		string::size_type nLen = strTmpPath.length();
		strTmpPath = strTmpPath.substr(0, nPos);	

		string::size_type nIndex = 0;    
		nIndex = strTmpPath.find("\\", 0);
    
		while ((nIndex = strTmpPath.find("\\", nIndex + 1)) >= 0)
		{
			TRACE("6\n");
			if( nIndex > nLen)
			{
				break;
			}
			
			CreateDirectory(strTmpPath.substr(0, nIndex).c_str(), NULL);		
		}

		CreateDirectory(strTmpPath.c_str(), NULL);		
		
		
		m_hFile = ::CreateFile(strFileName.c_str(), GENERIC_WRITE, 
													0,
													NULL, 
													CREATE_ALWAYS, 
													FILE_ATTRIBUTE_NORMAL, 
													NULL );
		
		if (m_hFile != INVALID_HANDLE_VALUE && m_nTotalBytes != 0 )
		{			
			if(!WriteFile(m_hFile, pRecvBytes->pBuff, pRecvBytes->receive_length, &dwWritten, NULL))
			{
				if(m_hFile)
				{
					CloseHandle(m_hFile);			
					m_hFile = 0;
				}

				CWinApp *pApp = AfxGetApp();
				CWnd *pMainWnd = pApp->GetMainWnd();
				pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	
		
				//CDialog::OnOK();
				
				return FALSE;				
			} 			
		}
		else
		{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				0, // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);

			TRACE("Recv Err [%s]\n", (LPCTSTR)lpMsgBuf);
			TRACE("strFileName [%s]\n", strFileName.c_str());
			

			//::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
			//list에 출력
			LVITEM rItem;		
			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = m_nRecvedFileCnt;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 1;			
			rItem.pszText =(LPSTR)lpMsgBuf ;
			rItem.iImage = -1;		
			m_ListCtrlFileRecv.SetItem(&rItem);
			m_ListCtrlFileRecv.EnsureVisible(m_nRecvedFileCnt,FALSE);
			//m_ListCtrlFileRecv.Invalidate();
			RECT tempRect;
			m_ListCtrlFileRecv.GetItemRect(m_nRecvedFileCnt, &tempRect, LVIR_BOUNDS);
			CRgn *pTempCRgn =  new CRgn();
			pTempCRgn->CreateRectRgnIndirect(&tempRect);					
			m_ListCtrlFileRecv.InvalidateRgn(pTempCRgn) ;
			pTempCRgn->DeleteObject();
			delete pTempCRgn;				

			
			LocalFree( lpMsgBuf );		
			if(m_hFile)
			{		
				CloseHandle(m_hFile);	
				m_hFile = 0;
			}

			m_RecvBytesProgress.SetPos(100);
			//CDialog::OnOK();			

			return FALSE;
		}
	}
	else
	{
		if( m_nTotalBytes != 0)
		{
			if(!WriteFile(m_hFile, pRecvBytes->pBuff, pRecvBytes->receive_length, &dwWritten, NULL))
			{		
				if(m_hFile)
				{					
					CloseHandle(m_hFile);	
					m_hFile = 0;
				}
				
				CWinApp *pApp = AfxGetApp();
				CWnd *pMainWnd = pApp->GetMainWnd();
				pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

				//CDialog::OnOK();
			
				return FALSE;
			}		
		}
	}
	
	return TRUE;
}


LRESULT CFileRecvDlg::OnCancelFileTransfer (WPARAM wParam, LPARAM lParam)
{		
	m_bIsCanceled= TRUE;
	if(m_hFile)
	{
		CloseHandle(m_hFile);			
		m_hFile = 0;
	}

	AfxMessageBox("상대방이 전송을 취소하였습니다.");
	
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

	//CDialog::OnOK();
	
	return TRUE;
}


LRESULT CFileRecvDlg::OnFileRecved (WPARAM wParam, LPARAM lParam)
{
	if(m_hFile)
	{
		CloseHandle(m_hFile);	
		m_hFile = 0;
	}	

	LVITEM rItem;		
	ZeroMemory ( &rItem, sizeof(LVITEM) );
	rItem.iItem = m_nRecvedFileCnt;
	rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
	rItem.iSubItem = 1;			
	rItem.pszText = "완료";
	rItem.iImage = -1;		
	m_ListCtrlFileRecv.SetItem(&rItem);
	m_ListCtrlFileRecv.EnsureVisible(m_nRecvedFileCnt,FALSE);
	//m_ListCtrlFileRecv.Invalidate();
	RECT tempRect;
	m_ListCtrlFileRecv.GetItemRect(m_nRecvedFileCnt, &tempRect, LVIR_BOUNDS);
	CRgn *pTempCRgn =  new CRgn();
	pTempCRgn->CreateRectRgnIndirect(&tempRect);					
	m_ListCtrlFileRecv.InvalidateRgn(pTempCRgn) ;
	pTempCRgn->DeleteObject();
	delete pTempCRgn;				

	ST_RCVED_FILE_INFO* pFileInfo = (ST_RCVED_FILE_INFO*) lParam ;

	m_nRecvedFileCnt++;
	m_nRecvBytes = 0;
	m_nTotalBytes = 0;

	if(m_nTotalFileCnt != 1 )
	{
		m_RecvFileProgress.SetPos(m_nRecvedFileCnt);
	}

	UpdateData(FALSE);

	delete [] pFileInfo->szFilePath ;
	pFileInfo->szFilePath= NULL;
	delete pFileInfo;

	// list에서 제거 
	/*
	CString str = "";
	for (int i=0;i < m_RecvList.GetCount(); i++) 
	{ 
		m_RecvList.GetText( i, str ); 
		TRACE("%s\n", str); TRACE("%s\n", strFileName.c_str());			
		strFileName = strFileName.substr(strlen(m_szSavePath), strlen(strFileName.c_str()) );
		TRACE("%s\n", strFileName.c_str()); 
		
		if( str.Find ( strFileName.c_str() ) != -1 )
		{
			m_RecvList.DeleteString(i);
			break;
		}
	}
	*/
		
	if(m_nTotalFileCnt == m_nRecvedFileCnt )
	{
		TRACE("파일 다받았다!\n");	
		
		m_strConFirmEdit.Format("%s[%s] 님 으로 부터 파일수신을 완료하였습니다 [%d / %d]\r\n\r\n저장위치 [%s]", 
			m_pSockObj->sPacketHeader.szFromName, //20080710
			m_pSockObj->sPacketHeader.szFromID , 
			m_nRecvedFileCnt, m_nTotalFileCnt, m_szSavePath );
		UpdateData(FALSE);

		//버튼 안보이게
		m_BtnOpenFolder.EnableWindow(TRUE);
		m_BtnCancel.SetWindowText("닫 기");
		m_BtnOK.ShowWindow(SW_HIDE);
		
		//CDialog::OnOK();
	}
	else
	{
		m_RecvBytesProgress.SetPos(0);	
		UpdateData(FALSE);

		// 다음 파일 전송을 요청 
		m_strConFirmEdit.Format("%s[%s] 님 으로 부터 파일을 받고 있습니다[%d / %d]\r\n저장위치 [%s]\r\n\r\n%s", 
			m_pSockObj->sPacketHeader.szFromName, //20080710
			m_pSockObj->sPacketHeader.szFromID , 
			m_nRecvedFileCnt, m_nTotalFileCnt, m_szSavePath, m_StrAry.ElementAt(m_nRecvedFileCnt) );
		UpdateData(FALSE);
		
		TRACE("[%d][%s]\n",m_nRecvedFileCnt, m_StrAry.ElementAt(m_nRecvedFileCnt) );			
		CFileTransSock::Instance().GetInformedFile( m_pSockObj, (LPSTR)(LPCSTR)m_StrAry.ElementAt(m_nRecvedFileCnt) ) ; 			
	}

	return TRUE;
}


void CFileRecvDlg::OnBtnFileConfirm() 
{
	// TODO: Add your control notification handler code here	
	m_BtnOK.EnableWindow(FALSE);
	
	// 파일 가져오기 	
	CFileTransSock::Instance().InformFolderSelect( m_pSockObj ) ; 			
	

	CString strUseDownFolder = "";
	CString strDownFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_DOWNFOLDER, strUseDownFolder);

	if(strUseDownFolder == "Y")
	{
		// Download Path		
		CINIManager::Instance().GetValue(INI_DOWNFOLDER_PATH, strDownFolderPath);
		strncpy	(m_szSavePath, (LPSTR)(LPCSTR)strDownFolderPath, sizeof(m_szSavePath));
	}
	else
	{
		FolderDialogOpen(m_szSavePath); 
	}

	
	if(m_bIsCanceled)
	{
		
		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();
		
		return ;
	}

	if( strlen(m_szSavePath) == 0)
	{
		CFileTransSock::Instance().DenyTransferFile( m_pSockObj ) ; 
		
		AfxMessageBox("폴더 경로를 선택하세요.\r\n전송이 취소되었습니다.");
		
				
		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();
		
		return ;
	}
		
	m_nTotalFileCnt = m_StrAry.GetSize() ;
	TRACE("m_nTotalFileCnt [%d]\n", m_nTotalFileCnt);

	CFileTransSock::Instance().GetInformedFile( m_pSockObj, (LPSTR)(LPCSTR)m_StrAry.ElementAt(0) ) ; 

}

void CFileRecvDlg::FolderDialogOpen(char* szResult)
{	
	BROWSEINFO bi = { 0 };
    
    bi.lpszTitle = _T("파일 저장 경로 선택");
    bi.pszDisplayName = szResult;
	bi.ulFlags = BIF_NEWDIALOGSTYLE ; // platform SDK 필요함 

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
        // get the name of the folder        
		::SHGetPathFromIDList(pidl, szResult);

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
	}

	//SHGetSpecialFolderPath(NULL, szResult, CSIDL_STARTUP, FALSE);	
}

void CFileRecvDlg::OnBtnFileNotConfirm() 
{
	// TODO: Add your control notification handler code here	
	
	if(m_nTotalFileCnt == m_nRecvedFileCnt )
	{
		//전송완료된 상태			

		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();		
	}
	else
	{	
		if(m_hFile)
		{
			CloseHandle(m_hFile);			
			m_hFile = 0;
		}		

		CFileTransSock::Instance().DenyTransferFile( m_pSockObj ) ; 

		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();
	}
}

BOOL CFileRecvDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_ListCtrlFileRecv.SetTextColor( RGB(0,0,0)  );
	m_ListCtrlFileRecv.SetBgColor( RGB(225,204,247)  );
	m_ListCtrlFileRecv.InsertColumn(0,"File");	
	m_ListCtrlFileRecv.InsertColumn(1,"Status");	
	
	m_ListCtrlFileRecv.SetColumnWidth(0, 1000 );	
	m_ListCtrlFileRecv.SetColumnWidth(1, 60  );	

	int nPos = -1;
	char szTmpOut[1024];		
	memset(szTmpOut, 0x00, sizeof(szTmpOut));

	char* szData = (char*) m_pFiles->strFile.c_str() ;	
	
	// '|' 구분자.
	// list에 저장하고, 하나씩 꺼내서 다시 전송해달라고 요청한다..	
	
	memset(m_szRootPath, 0x00, sizeof(m_szRootPath));
	nPos = GetStrByDelim(szData, m_szRootPath, '|'); //Rootpath	
		
	while(1)
	{	
		TRACE("4\n");
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim(szData, szTmpOut, '|');
		
		if	(	
				strcmp(szTmpOut , "NotFound" ) != 0 && 
				strlen(szTmpOut) > 0
			)
		{
			TRACE("szTmpOut [%s]\n", szTmpOut );
			
			m_StrAry.Add( szTmpOut );
			m_nTotalFileCnt++;
			
			//////////////////////////////////////////////////////////
			// 리스트에 보이는 목록은 상대방이 보낸 파일 경로에서 root를 제거힌 것만.
			LVITEM lvi0;		
			lvi0.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
			int tempCnt = m_ListCtrlFileRecv.GetItemCount();
			lvi0.iItem = tempCnt;		
			lvi0.iSubItem = 0;
			//lvi0.pszText = szTmpOut ;

			TRACE("#1[%s]%d\n", szTmpOut, strlen(szTmpOut) );
			TRACE("#2[%s]%d\n", m_szRootPath, strlen(m_szRootPath) );
			
			char szTmp [1024];
			memset(szTmp, 0x00, sizeof(szTmp));

			strncpy(szTmp , szTmpOut + strlen(m_szRootPath), strlen(szTmpOut) - strlen(m_szRootPath) );
			lvi0.pszText = szTmp ;
			
			lvi0.iImage = 1;
			m_ListCtrlFileRecv.InsertItem(&lvi0);
			m_ListCtrlFileRecv.SetItem(&lvi0);

			LVITEM lvi;		
			lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = tempCnt;		
			lvi.iSubItem = 1;
			lvi.pszText = "대기중" ; 
			lvi.iImage = 1;
			m_ListCtrlFileRecv.InsertItem(&lvi);
			m_ListCtrlFileRecv.SetItem(&lvi);
		}
		else
		{
			break;
		}
	}
	
	/*
	CString str; CSize sz; 
	int dx=0; 
	CDC* pDC = m_RecvList.GetDC(); 
	for (int i=0;i < m_RecvList.GetCount();i++) 
	{ 
		m_RecvList.GetText( i, str ); 
		sz = pDC->GetTextExtent(str); 
		if (sz.cx > dx) 
			dx = sz.cx; 

		m_RecvList.SetHorizontalExtent( dx );
	} 
	m_RecvList.ReleaseDC(pDC); 
	*/

	TRACE(" Total File Cnt [%d]\n", m_nTotalFileCnt);
	
	char szBriefFiles[300];
	memset(szBriefFiles,0x00,sizeof(szBriefFiles));		
	
	CTime cTimeLog =  CTime::GetCurrentTime();
	CString strTime = cTimeLog.Format("%Y년 %m월 %d일 / %H시 %M분 %S초");
	
	sprintf(szBriefFiles,"[%s]\r\n%s(%s) 님이 파일을 전송하려 합니다[전체 %d 개], \r\n수락하려면 확 인 버튼을 누르세요.", 
		(LPCSTR)strTime, 
		m_pSockObj->sPacketHeader.szFromName , //20080710 
		m_pSockObj->sPacketHeader.szFromID , 		
		m_nTotalFileCnt );
	
	char szTmp[255];
	memset(szTmp, 0x00, sizeof(szTmp));
	sprintf(szTmp, "파일받기 : 대화상대 [%s][%s] IP [%s]", 
		m_pSockObj->sPacketHeader.szFromName , //20080710 ,
		m_pSockObj->sPacketHeader.szFromID , 
		m_pSockObj->m_szIP );	

	SetWindowText(szTmp);

	m_strConFirmEdit = szBriefFiles ;
	
	m_RecvFileProgress.SetRange(0, 100);
	if(m_nTotalFileCnt == 1 )
	{
		m_RecvFileProgress.SetRange(0, 100);
	}
	else
	{
		m_RecvFileProgress.SetRange(0, m_nTotalFileCnt);
	}
	
	m_RecvFileProgress.SetStep(1);
	m_RecvFileProgress.SetPos(0);

	m_RecvFileProgress.SetStartColor(RGB(81,211,255));
	m_RecvFileProgress.SetEndColor(RGB(141,28,255));

	m_RecvBytesProgress.SetStartColor(RGB(81,211,255));
	m_RecvBytesProgress.SetEndColor(RGB(141,28,255));

	//m_RecvFileProgress.SetForeColour ( RGB(58,166,255) );
	//m_RecvBytesProgress.SetForeColour( RGB(58,166,255) );

	UpdateData(FALSE);

	SetResize();
	
	m_BtnOpenFolder.EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileRecvDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDialog::PostNcDestroy();
}

LRESULT CFileRecvDlg::OnDisconnected (WPARAM wParam, LPARAM lParam)
{
	if(m_nTotalFileCnt != m_nRecvedFileCnt )
	{
		if(m_hFile)
		{
			CloseHandle(m_hFile);			
			m_hFile = 0;
		}

		AfxMessageBox("파일 수신이 완료되기전에, 종료되었습니다.");	

		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_RECVDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();
	}

	return TRUE; 
}


void CFileRecvDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
}

void CFileRecvDlg::OnBtnOpenFileFolder() 
{
	// TODO: Add your control notification handler code here
	char szWinDir[512];	
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDir, 0x00, sizeof(szDir));		
	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	

	CString strUseDownFolder = "";
	CString strDownFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_DOWNFOLDER, strUseDownFolder);

	if(strUseDownFolder == "Y")
	{
		// Download Path		
		CINIManager::Instance().GetValue(INI_DOWNFOLDER_PATH, strDownFolderPath);
		sprintf(szDir, "%s\\explorer.exe %s",szWinDir, (LPCSTR)strDownFolderPath);		
	}
	else
	{
		sprintf(szDir, "%s\\explorer.exe %s",szWinDir, m_szSavePath);
	}
	
	WinExec(szDir, SW_SHOWDEFAULT);	
}

BOOL CFileRecvDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}
