// FileSendDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "filetransferdlg.h"
#include "FileSendDlg.h"
#include "FileTransSock.h"
#include "MsgBoxThread.h"
#include <MSWSOCK.H>		// MFC socket extensions
#include  <io.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileSendDlg dialog


CFileSendDlg::CFileSendDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileSendDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileSendDlg)
	m_StrSendFileEdit = _T("");
	m_StrSentAmt = _T("");
	//}}AFX_DATA_INIT

	m_nSentBytes =0;
	m_nTotalBytes = 0;
	m_strRootPath = "";
	m_pSpckObjMap = NULL;
	//m_hFile = NULL;
	memset(m_szCurFile, 0x00, sizeof(m_szCurFile));
	memset(m_szSockObjID, 0x00, sizeof(m_szSockObjID));
}

CFileSendDlg::CFileSendDlg(int nFileCnt,  CSockObjMapInfoFile * pSpckObjMap,char* szToId, CString& strFiles)   
{
	m_nTotalBytes = 0;
	m_nSentBytes =0;
	m_pSpckObjMap = NULL;
	memset(m_szIP, 0x00, sizeof(m_szIP));
	memset(m_szToID, 0x00, sizeof(m_szToID));
	m_nFileSent = 0;
	m_nFileCntToSend = nFileCnt;
	m_nTotalFileCnt = nFileCnt ;
	m_pSpckObjMap = pSpckObjMap ;
	strcpy( m_szIP , pSpckObjMap->m_pSockObj->m_szIP );
	strcpy( m_szToID , szToId );
	
	m_strRootPath = "";
	m_strFiles = strFiles ;	
	memset(m_szCurFile, 0x00, sizeof(m_szCurFile));

	//m_strFiles 는 루트패스|파일패스 이다..
		
	if(m_strRootPath.IsEmpty())
	{
		int nFound = m_strFiles.Find("|", 0);
		m_strRootPath = m_strFiles.Mid(0 , nFound);
	}

	memset(m_szSockObjID, 0x00, sizeof(m_szSockObjID));
	strcpy(m_szSockObjID,m_pSpckObjMap->m_pSockObj->m_szSockObjID );
}

void CFileSendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileSendDlg)
	DDX_Control(pDX, IDC_BTN_FILE_SEND_CANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_LIST_SEND, m_ListSend);
	DDX_Control(pDX, IDC_SEND_BYTES_PROGRESS, m_SentBytesProgress);
	DDX_Control(pDX, IDC_SEND_PROGRESS, m_SendFileProgress);
	DDX_Text(pDX, IDC_EDIT_SEND_FILE, m_StrSendFileEdit);
	DDX_Text(pDX, IDC_STATIC_SENT_AMT, m_StrSentAmt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileSendDlg, CDialog)
	//{{AFX_MSG_MAP(CFileSendDlg)
	ON_BN_CLICKED(IDC_BTN_FILE_SEND_CANCEL, OnBtnFileSendCancel)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_SEND_THIS_INFORMED_FILE,	OnSendThisInformedFile	)
	ON_MESSAGE(WM_FOLDER_SELECTING,			OnFolderSelecting	)	
	ON_MESSAGE(WM_NO_THANKS_FILE_TRANSFER,	OnNoThanksFileTransfer	)		
	ON_MESSAGE(WM_SEND_BYTES,				OnSendBytes	)	
	ON_MESSAGE(WM_DISCONNECTED,				OnDisconnected	)	
		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileSendDlg message handlers

void CFileSendDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	m_cControlPos.AddControl(IDC_EDIT_SEND_FILE	, CP_RESIZE_HORIZONTAL );		
	m_cControlPos.AddControl(IDC_LIST_SEND	, CP_RESIZE_HORIZONTAL  |  CP_RESIZE_VERTICAL  );		
	m_cControlPos.AddControl(IDC_SEND_BYTES_PROGRESS, CP_MOVE_VERTICAL |CP_RESIZE_HORIZONTAL    );		
	m_cControlPos.AddControl(IDC_SEND_PROGRESS, CP_MOVE_VERTICAL |CP_RESIZE_HORIZONTAL    );		
	m_cControlPos.AddControl(IDC_BTN_FILE_SEND_CANCEL	, CP_MOVE_VERTICAL |CP_MOVE_HORIZONTAL  );		

	m_cControlPos.AddControl(IDC_STATIC_FILE	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_TOTAL	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_BYTE	, CP_MOVE_VERTICAL   );		
	m_cControlPos.AddControl(IDC_STATIC_SENT_AMT	, CP_MOVE_VERTICAL   );		
	
}

void CFileSendDlg::OnBtnFileSendCancel() 
{
	// TODO: Add your control notification handler code here
	
	if(m_nFileSent == m_nTotalFileCnt)
	{				
		/*
		::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 
				
		if( (DWORD)m_pSpckObjMap->m_pThreadObj != 0xfeeefeee && 
			(DWORD)m_pSpckObjMap->m_pSockObj != 0xfeeefeee	)
		{
			CFileTransSock::Instance().RemoveSocketObj(m_pSpckObjMap->m_pThreadObj, m_pSpckObjMap->m_pSockObj);
			CFileTransSock::Instance().FreeSocketObj( m_pSpckObjMap->m_pSockObj);	
			CFileTransSock::Instance().RenumberThreadArray(m_pSpckObjMap->m_pThreadObj); //20080707
		}
		
		::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 
		*/
		
		shutdown (m_pSpckObjMap->m_pSockObj->s ,SD_BOTH ) ;
		//closesocket(m_pSpckObjMap->m_pSockObj->s);

		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_SENDDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();
	}
	else
	{
		CFileTransSock::Instance().CancelSendFile(m_pSpckObjMap);	
					
		/*
		::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 

		if( (DWORD)m_pSpckObjMap->m_pThreadObj != 0xfeeefeee && 
			(DWORD)m_pSpckObjMap->m_pSockObj != 0xfeeefeee	)
		{
			CFileTransSock::Instance().RemoveSocketObj(m_pSpckObjMap->m_pThreadObj, m_pSpckObjMap->m_pSockObj);
			CFileTransSock::Instance().FreeSocketObj( m_pSpckObjMap->m_pSockObj);	
			CFileTransSock::Instance().RenumberThreadArray(m_pSpckObjMap->m_pThreadObj); 
		}
		
		::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 
		*/

		shutdown (m_pSpckObjMap->m_pSockObj->s ,SD_BOTH ) ;
		//closesocket(m_pSpckObjMap->m_pSockObj->s);

		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		pMainWnd->PostMessage(WM_FILE_SENDDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	

		//CDialog::OnOK();	
	}
}

BOOL CFileSendDlg::IsFileAlreadyOpen(char *filename) 
{ 
    HFILE theFile = HFILE_ERROR; 
    DWORD lastErr = NO_ERROR; 
    
	// Attempt to open the file exclusively.     
    theFile = _lopen(filename, OF_READ | OF_SHARE_EXCLUSIVE); 
    
	if (theFile == HFILE_ERROR) 
        // Save last error... 
        lastErr = GetLastError(); 
    else // If the open was successful, close the file. 
        _lclose(theFile); 
    
    // Return TRUE if there is a sharing-violation.     
    return ((theFile == HFILE_ERROR) && (lastErr == ERROR_SHARING_VIOLATION)); 
}

LRESULT CFileSendDlg::OnSendThisInformedFile (WPARAM wParam, LPARAM lParam)
{	
	BOOL nCopyRtn = FALSE;
	HANDLE hFile = 0;
	m_nPrePercent = 0;
	m_nSentBytes = 0;
	m_nTotalBytes = 0;

	m_nFileSent++;

	char* szFileDel = (char *) wParam ;	
	char  szFile [360 + 1];
	char  szFileOld [360 + 1];

	memset(szFile, 0x00, sizeof(szFile));
	memset(szFileOld, 0x00, sizeof(szFileOld));

	strcpy(szFile, (char*) wParam )	;
	
	TRACE("m_szCurFile [%s]\n", m_szCurFile );

	if	(	strlen(m_szCurFile) > 0 &&
			strncmp	(	m_szCurFile + ( strlen(m_szCurFile) - strlen("_OpenedMark_") ) , 
						"_OpenedMark_", 
						strlen("_OpenedMark_") 
					) == 0 
		)
	{		
		DeleteFile(m_szCurFile) ;
	}

	strncpy(m_szCurFile , (char *) wParam , sizeof(m_szCurFile));
	
	delete [] szFileDel ;
	szFileDel = NULL;
	
	SOCKET_OBJ_FILE* pSockObj = (SOCKET_OBJ_FILE*) lParam ;
	
	// file을 보낸다!!		
	// Full Path 에서 root 문자열 제거 	
	
	char szFileNamePathExceptTop[360+1];
	
	memset(szFileNamePathExceptTop, 0x00, sizeof(szFileNamePathExceptTop));

	getFileNamePathExceptRoot(szFile, (LPCSTR)m_strRootPath, szFileNamePathExceptTop );
		
	TRACE("szFileNamePathExceptTop [%s]\n", szFileNamePathExceptTop );

	// 동적으로 데이터 사이즈 변동 	
	BYTE* pSzBufHeader = NULL;
	
	FILE_HEADER sFileHeader;	
	int iLength = 0, nTotalLen = 0;		
	char szTemp[20];	
	
	memset(&sFileHeader, NULL, sizeof(sFileHeader));
	
	m_StrSendFileEdit.Format( "파일을 전송중입니다 [%d / %d]\r\n\r\n%s" ,m_nFileSent, m_nTotalFileCnt, szFileNamePathExceptTop );
	UpdateData(FALSE);

	//변수 선언	
	DWORD nFilePoint = 0;
	__int64 dwFileSize = 0;
				
	if( strlen(szFile) !=0 )
	{
		nFilePoint = 0;		
		char szTmpFileSize[20];
		memset(szTmpFileSize,0x00, sizeof(szTmpFileSize));
		
		HANDLE h;
		WIN32_FIND_DATA info;
	
		if ((h = FindFirstFile( szFile,&info))  != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			
			if ((info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0) // Is it a file?
			{
				union
				{
					struct { DWORD low, high; } lh;
					__int64 size; // MS large int extension
				} file;
				
				file.lh.low = info.nFileSizeLow;
				file.lh.high= info.nFileSizeHigh;
				
				dwFileSize = file.size; // will be casted to double
				sprintf(szTmpFileSize, "%I64d", dwFileSize );				
			}
			// It's a directory, not a file
		}
		
		if	(IsFileAlreadyOpen (szFile) )
		{
			if( dwFileSize >= 10000000 ) // 10 MB 정도 한도에서 copy  
			{
				dwFileSize = 0 ; 
				
				CString strErrMsg;
				strErrMsg.Format("%s \n을 보낼수 없음(파일 사용중이고 대용량 파일임.\n복사후 전송 가능한 기준은 10 MB)", szFile);
				CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
				pMsgThread->CreateThread();

				/*
				LVFINDINFO info;
				int nIndex = -1;		
				info.flags = LVFI_PARAM;
				info.flags = LVFI_PARTIAL|LVFI_STRING;
				info.psz = szFile;		
				nIndex = m_ListSend.FindItem(&info);
					
				if(nIndex != -1)
				{
					LVITEM rItem;		
					
					ZeroMemory ( &rItem, sizeof(LVITEM) );
					rItem.iItem = nIndex;
					rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
					rItem.iSubItem = 1;			
					rItem.pszText = "다른 프로세스가 사용중인 파일이며, 보낼수 없습니다.";
					rItem.iImage = -1;		
					m_ListSend.SetItem(&rItem);
					m_ListSend.EnsureVisible(nIndex,FALSE);
					
					RECT tempRect;
					m_ListSend.GetItemRect(nIndex, &tempRect, LVIR_BOUNDS);
					CRgn *pTempCRgn =  new CRgn();
					pTempCRgn->CreateRectRgnIndirect(&tempRect);					
					m_ListSend.InvalidateRgn(pTempCRgn) ;
					pTempCRgn->DeleteObject();
					delete pTempCRgn;				
				}
				*/
			}
			else
			{
				// 파일이 열린 경우  파일명을 변경해서 복사후 전송한다.
				// 이미 복사한 상태인지도 확인
				
				char szNewFilePath [360+1];
				memset(szNewFilePath, 0x00, sizeof(szNewFilePath));
				strcpy(szNewFilePath, szFile );
				strcat(szNewFilePath, "_OpenedMark_");
				strcat(szFileNamePathExceptTop, "_OpenedMark_");
				
				strncpy(m_szCurFile , szNewFilePath , sizeof(m_szCurFile));

				if( (_access( szNewFilePath, 0 )) == -1 )
				{
					//파일/폴더가 존재하지 않는 경우 
					BOOL nRtn = CopyFile(szFile, szNewFilePath, FALSE );				
								
					if(!nRtn)
					{
						// 복사도 실패!!
						strcpy(szNewFilePath, szFileOld ); //20080627					
						dwFileSize = 0 ; 
						
						LVFINDINFO info;
						int nIndex = -1;		
						info.flags = LVFI_PARAM;
						info.flags = LVFI_PARTIAL|LVFI_STRING;
						info.psz = szFile;		
						nIndex = m_ListSend.FindItem(&info);
						
						if(nIndex != -1)
						{
							LVITEM rItem;		
							
							ZeroMemory ( &rItem, sizeof(LVITEM) );
							rItem.iItem = nIndex;
							rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
							rItem.iSubItem = 1;			
							rItem.pszText = "다른 프로세스가 사용중인 파일이며, Copy해서 보낼수도 없습니다.";
							rItem.iImage = -1;		
							m_ListSend.SetItem(&rItem);
							m_ListSend.EnsureVisible(nIndex,FALSE);
							
							RECT tempRect;
							m_ListSend.GetItemRect(nIndex, &tempRect, LVIR_BOUNDS);
							CRgn *pTempCRgn =  new CRgn();
							pTempCRgn->CreateRectRgnIndirect(&tempRect);					
							m_ListSend.InvalidateRgn(pTempCRgn) ;
							pTempCRgn->DeleteObject();
							delete pTempCRgn;				
						}					
					}
				}

				strcpy(szFile,  szNewFilePath);	
			}			
		}		
	}
	else
	{		
		return FALSE ;
	}
			
	m_nTotalBytes = dwFileSize ; 	
	m_SentBytesProgress.SetRange(0, 100 );
	m_SentBytesProgress.SetStep(1);
	m_SentBytesProgress.SetPos(0);

	// 헤더부 설정		
	memcpy(sFileHeader.szUsage, (char*)USAGE_MAKE_THIS_FILE, sizeof(sFileHeader.szUsage)); 	
	memcpy(sFileHeader.szFilePath, szFileNamePathExceptTop, sizeof(sFileHeader.szFilePath));	
	sprintf(szTemp, "%0*d", sizeof(sFileHeader.szLength)-1, dwFileSize );		
	memcpy(sFileHeader.szLength, szTemp, sizeof(sFileHeader.szLength));	
	sprintf(szTemp, "%0*I64d", sizeof(sFileHeader.szFileLength)-1, dwFileSize );		
	memcpy(sFileHeader.szFileLength, szTemp, sizeof(sFileHeader.szFileLength));	
	memcpy(sFileHeader.szFromID, CChatSession::Instance().m_szMyUserID , min(sizeof(sFileHeader.szFromID), strlen(CChatSession::Instance().m_szMyUserID))); 
	
	memcpy(sFileHeader.szFromName, (LPCSTR) CChatSession::Instance().m_LoginName , //20080710
			min(sizeof(sFileHeader.szFromName), strlen( (LPCSTR) CChatSession::Instance().m_LoginName))); 

	memcpy(sFileHeader.szToIP, pSockObj->m_szIP, sizeof(pSockObj->m_szIP) ); 

	
	/* TransmitFile file handle구하기 

		hFile = CreateFile( szFile, 
							GENERIC_READ, 
							FILE_SHARE_READ, 
							NULL, 
							OPEN_EXISTING,
							FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, 
							NULL);
		
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD nErr = GetLastError();
			
			if	( 
					nErr == ERROR_SHARING_VIOLATION && 
					dwFileSize <= 10000000 // 10 mb 정도만 
				)
			{
				// 파일이 열린 경우  파일명을 변경해서 복사후 전송한다.
				CloseHandle(hFile);	
								
				char szNewFilePath [260+1];
				memset(szNewFilePath, 0x00, sizeof(szNewFilePath));
				strcpy(szNewFilePath, szFile );
				strcat(szNewFilePath, "_OpenedMark_");
				strcat(szFileNamePathExceptTop, "_OpenedMark_");

				BOOL nRtn = CopyFile(szFile, szNewFilePath, FALSE );				
				strcpy(szFile,  szNewFilePath);		
				strncpy(m_szCurFile , szNewFilePath , sizeof(m_szCurFile));

				if(!nRtn)
				{
					CFileTransSock::Instance().CancelSendFile(m_pSpckObjMap);
					
					CDialog::OnOK();
					return FALSE;
				}
				
				
				hFile = CreateFile( szNewFilePath, 
							GENERIC_READ, 
							FILE_SHARE_READ, 
							NULL, 
							OPEN_EXISTING,
							FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, 
							NULL);
		
				if(hFile == INVALID_HANDLE_VALUE)
				{
					DWORD nErr = GetLastError();
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
					
					::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
					// Free the buffer.
					LocalFree( lpMsgBuf );
					
					CloseHandle(hFile);								
										
					CFileTransSock::Instance().CancelSendFile(m_pSpckObjMap);
					
					CDialog::OnOK();
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
				
				::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
				
				// Free the buffer.
				LocalFree( lpMsgBuf );				
				CloseHandle(hFile);				
				CFileTransSock::Instance().CancelSendFile(m_pSpckObjMap);
				
				CDialog::OnOK();
				return FALSE;
			}
		}
		*/
	// ----------- TransmitFile or send ? ------------------------------------------------------------------
			
	// **** 1) TransmitFile *****
	/*
	TRANSMIT_FILE_BUFFERS FileBuffer; 	
	memset(&FileBuffer,0x00, sizeof(FileBuffer));

	FileBuffer.Head = &sFileHeader;
	FileBuffer.HeadLength = FILE_HEADER_LENGTH ;
	FileBuffer.TailLength = 0;	
	FileBuffer.Tail = NULL;
		
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN); 

	DWORD nRtn = GetLastError();
	
	if( ::TransmitFile(pSockObj->s, hFile, 0, 0, NULL,&FileBuffer , 0) == TRUE) 
	{
		m_nFileSent++;
		m_nFileCntToSend--;
		m_SendFileProgress.SetPos(m_nFileSent);

		if(m_nFileCntToSend == 0 )
		{
			TRACE("다 보냈다!\n");
			CloseHandle(hFile);  
			delete [] szFile ;
	
					
			CDialog::OnOK();

			return TRUE;
		}		
	}
	else
	{
		int nRtn = WSAGetLastError ();
		TRACE("\n\nError [%ld]\n\n",nRtn );		
	}
	*/
	
		
	// **** 2) sock *****	
		
	CSockObjMapInfoFile *pa2= NULL;

	if( CFileTransSock::Instance(). mSockObjMap.Lookup( (LPCSTR)pSockObj->m_szSockObjID, ( CObject*& ) pa2 )) 
	{
		// 전송데이터 모으기
		BUFFER_OBJ_FILE *newbuf = NULL;
		
		memset(pSockObj->m_szLocalFilePath, 0x00, sizeof(pSockObj->m_szLocalFilePath));
		strcpy(pSockObj->m_szLocalFilePath, szFile );
		
		// 먼저 헤더길이만큼 따로 버퍼할당후 저장.
		pSzBufHeader = new BYTE [FILE_HEADER_LENGTH]; 
		memset(pSzBufHeader, NULL, sizeof(pSzBufHeader));	
		memcpy(pSzBufHeader, &sFileHeader, FILE_HEADER_LENGTH);
		
		newbuf = CFileTransSock::Instance().GetBufferObj( FILE_HEADER_LENGTH );		
		memcpy(newbuf->buf, pSzBufHeader , FILE_HEADER_LENGTH );

		::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 
		CFileTransSock::Instance().EnqueueBufferObj( pSockObj, newbuf  , FALSE); 
		::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 

		if(pSzBufHeader)
		{
			delete[] pSzBufHeader; 
			pSzBufHeader = NULL;
		}
		
		// 파일 데이터들은 파일 읽기 위치만 저장 
		CSockObjMapInfoFile * pSpckObjMap = static_cast<CSockObjMapInfoFile*>(pa2);
				
		int nTimes = dwFileSize / MAX_BUFF_SIZE ;

		if(pSpckObjMap)
		{						
			if(dwFileSize >= MAX_BUFF_SIZE )
			{				
				int nOffSet = 0;

				for ( int i=0; i < dwFileSize / MAX_BUFF_SIZE ; i++ )
				{					
					newbuf = CFileTransSock::Instance().GetBufferObj( sizeof(BUFFER_OBJ_FILE) );
					
					newbuf->bIsFileData = TRUE;						
					newbuf->buf = NULL;
					newbuf->buflen = 0;
					newbuf->dwReadFilePosition = nOffSet ;
					newbuf->dwTotalFileLength = dwFileSize ;
					newbuf->dwReadLength = MAX_BUFF_SIZE; 
					
					::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 
					CFileTransSock::Instance().EnqueueBufferObj( pSockObj, newbuf , FALSE); 
					::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 

					nOffSet+= MAX_BUFF_SIZE ;					
				}
				
				if( (dwFileSize % MAX_BUFF_SIZE) > 0 )
				{
					newbuf = CFileTransSock::Instance().GetBufferObj( sizeof(BUFFER_OBJ_FILE) );
					
					newbuf->bIsFileData = TRUE;										
					newbuf->buf = NULL;
					newbuf->buflen = 0;
					newbuf->dwReadFilePosition = nOffSet  ;
					newbuf->dwTotalFileLength = dwFileSize ;
					newbuf->dwReadLength = dwFileSize % MAX_BUFF_SIZE; 
					
					::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 							
					CFileTransSock::Instance().EnqueueBufferObj( pSockObj, newbuf  , FALSE); 
					::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 

					nTimes++;
				}
			}
			else
			{				
				newbuf = CFileTransSock::Instance().GetBufferObj( sizeof(BUFFER_OBJ_FILE) );
				
				newbuf->bIsFileData = TRUE;					
				newbuf->buf = NULL;
				newbuf->buflen = 0;
				newbuf->dwReadFilePosition = 0  ;
				newbuf->dwTotalFileLength = dwFileSize ;
				newbuf->dwReadLength = dwFileSize ; 
				
				::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 							
				CFileTransSock::Instance().EnqueueBufferObj(pSockObj, newbuf, FALSE); 
				::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 
			}			
						
			::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 	
			
			int rc = CFileTransSock::Instance().SendPendingData(pSockObj);						
															
			if (rc == -1) 
			{
				CFileTransSock::Instance().RemoveSocketObj(pSpckObjMap->m_pThreadObj,pSockObj );
				CFileTransSock::Instance().FreeSocketObj( pSockObj );
//				CFileTransSock::Instance().RenumberThreadArray(pSpckObjMap->m_pThreadObj); //20080707
			}			
			
			::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 
		}
	}		
	
	CloseHandle(hFile);  
		
	return TRUE;
}

	

LRESULT CFileSendDlg::OnSendBytes (WPARAM wParam, LPARAM lParam)
{	
	int nSentBytes = (int) wParam;
			
	m_nSentBytes+= nSentBytes;

	float nPercent = 0;
	
	// 파일 사이즈가 0 이면 ..
	if(m_nTotalBytes == 0 )
	{
		TRACE("파일 사이즈가 0 \n");
		
		nPercent = 100 ;		
	}
	else
	{
		nPercent = ((float) m_nSentBytes / (float) m_nTotalBytes) * (float) 100 ;
	}
	
	m_StrSentAmt.Format("%I64d / %I64d", m_nSentBytes, m_nTotalBytes );

	TRACE("OnSendBytes : nSentBytes [%d] m_nTotalBytes [%d] Percent [%f]\n", nSentBytes,m_nTotalBytes, nPercent);

	if( nPercent >= (float) 100)
	{
		m_nFileCntToSend--;

		TRACE("--  m_nFileCntToSend [%d] %d/%d \n", m_nFileCntToSend, m_nFileSent, m_nTotalFileCnt);
		
		//nPercent = 100;
		
		if(m_nTotalFileCnt == 1 )
		{
			m_SendFileProgress.SetPos(100);
		}
		else
		{
			m_SendFileProgress.SetPos(m_nFileSent);
		}
		
		if( m_nPrePercent == 0 )
		{
			m_SentBytesProgress.SetPos(0);
			UpdateData(FALSE);			
		}
		
		// 전송 표시  				
		//////////////////////////////////////////////
		LVFINDINFO info;
		int nIndex = -1;		
		info.flags = LVFI_PARAM;
		info.flags = LVFI_PARTIAL|LVFI_STRING;
		info.psz = m_szCurFile;		
		nIndex = m_ListSend.FindItem(&info);
		
		if(nIndex != -1)
		{
			LVITEM rItem;		
			
			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 1;			
			rItem.pszText = "완료";
			rItem.iImage = -1;		
			m_ListSend.SetItem(&rItem);
			m_ListSend.EnsureVisible(nIndex,FALSE);
			//m_ListSend.Invalidate();
			RECT tempRect;
			m_ListSend.GetItemRect(nIndex, &tempRect, LVIR_BOUNDS);
			CRgn *pTempCRgn =  new CRgn();
			pTempCRgn->CreateRectRgnIndirect(&tempRect);					
			m_ListSend.InvalidateRgn(pTempCRgn) ;
			pTempCRgn->DeleteObject();
			delete pTempCRgn;				
		}
		//////////////////////////////////////////////	
		
		if(m_nFileSent == m_nTotalFileCnt)
		{
			TRACE("다 보냈다!\n");	

			if	(	strncmp	(	m_szCurFile + ( strlen(m_szCurFile) - strlen("_OpenedMark_") ) , 
								"_OpenedMark_", 
								strlen("_OpenedMark_") 
							) == 0 
				)
			{		
				//CFile::Remove(m_szCurFile) ;
				//CloseHandle(m_hFile);
				if ( DeleteFile(m_szCurFile) == 0 )
				{
					TRACE( "Error [%d]\n", GetLastError() );
				}
			}
			
			m_StrSendFileEdit.Format( "파일전송을 완료하였습니다 [전체 %d 개]" , m_nTotalFileCnt );	

			m_SentBytesProgress.SetPos(100);

			UpdateData(FALSE);
			
			m_BtnCancel.SetWindowText("닫 기");

			//CDialog::OnOK();

			return TRUE;
		}

		nPercent = 100;
	}
	
	if( (int) nPercent <=0)
	{
		nPercent =0;
	}

	if(m_nTotalFileCnt == 1 )
	{
		m_SendFileProgress.SetPos(nPercent);
	}

	m_SentBytesProgress.SetPos(nPercent);
	UpdateData(FALSE);

	m_nPrePercent = nPercent ;

	TRACE("Sent Bytes : [%d] [%d] \n", m_nSentBytes, (int)nPercent );
	
	return TRUE;
}


LRESULT CFileSendDlg::OnNoThanksFileTransfer (WPARAM wParam, LPARAM lParam)
{	
	/*
	::EnterCriticalSection(& CFileTransSock::Instance().m_CS); 

	if( (DWORD)m_pSpckObjMap->m_pThreadObj != 0xfeeefeee && 
			(DWORD)m_pSpckObjMap->m_pSockObj != 0xfeeefeee	)
	{
		CFileTransSock::Instance().RemoveSocketObj(m_pSpckObjMap->m_pThreadObj, m_pSpckObjMap->m_pSockObj);
		CFileTransSock::Instance().FreeSocketObj( m_pSpckObjMap->m_pSockObj);	
		CFileTransSock::Instance().RenumberThreadArray(m_pSpckObjMap->m_pThreadObj); //20080707
	}

	::LeaveCriticalSection(& CFileTransSock::Instance().m_CS); 
	*/

	shutdown (m_pSpckObjMap->m_pSockObj->s ,SD_BOTH ) ;
	//closesocket(m_pSpckObjMap->m_pSockObj->s);

	AfxMessageBox("상대방이 전송을 취소하였습니다.");	

	
	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_FILE_SENDDLG_CLOSE, (WPARAM)m_szSockObjID, (LPARAM)0);	
	

	//CDialog::OnOK();
	return TRUE;
}

LRESULT CFileSendDlg::OnFolderSelecting (WPARAM wParam, LPARAM lParam)
{
	m_StrSendFileEdit = "상대방이 파일 저장 경로를 설정중입니다" ;

	UpdateData(FALSE);
	return TRUE;
}

BOOL CFileSendDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//m_szToID
	char szTmp[255];
	memset(szTmp, 0x00, sizeof(szTmp));
	sprintf(szTmp, "파일보내기 : 대화상대 [%s] IP [%s]",m_szToID, m_szIP);	
	SetWindowText(szTmp);

	if(m_nTotalFileCnt == 1 )
	{
		m_SendFileProgress.SetRange(0, 100);
	}
	else
	{
		m_SendFileProgress.SetRange(0, m_nTotalFileCnt);
	}
	m_SendFileProgress.SetStep(1);
	m_SendFileProgress.SetPos(0);
		
	m_SentBytesProgress.SetStartColor(RGB(81,211,255));
	m_SentBytesProgress.SetEndColor(RGB(141,28,255));

	m_SendFileProgress.SetStartColor(RGB(81,211,255));
	m_SendFileProgress.SetEndColor(RGB(141,28,255));

	//m_SentBytesProgress.SetForeColour( RGB(58,166,255) );
	//m_SendFileProgress.SetForeColour ( RGB(58,166,255) );
			
	m_StrSendFileEdit = "접속을 기다리고 있습니다" ;

	UpdateData(FALSE);

	m_ListSend.SetTextColor( RGB(0,0,0)  );
	m_ListSend.SetBgColor( RGB(225,204,247)  );

	m_ListSend.InsertColumn(0,"File");	
	m_ListSend.InsertColumn(1,"Status");	
	
	m_ListSend.SetColumnWidth(0, 800 );	
	m_ListSend.SetColumnWidth(1, 500  );	
	
		
	int nPos = -1;
	char szTmpOut[1024];		
	char* szData = (LPSTR)(LPCSTR) m_strFiles ;	

	memset(szTmpOut, 0x00, sizeof(szTmpOut));	
	
	nPos = GetStrByDelim(szData, szTmpOut, '|');
		
	while(1)
	{	
		TRACE("3\n");
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim(szData, szTmpOut, '|');
		
		if	(	
				strcmp(szTmpOut , "NotFound" ) != 0 && 
				strlen(szTmpOut) > 0
			)
		{
			TRACE("szTmpOut [%s]\n", szTmpOut );
			
			m_StrArySend.Add( szTmpOut );									
			
			LVITEM lvi0;		
			lvi0.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
			int tempCnt = m_ListSend.GetItemCount();
			lvi0.iItem = tempCnt;		
			lvi0.iSubItem = 0;
			lvi0.pszText = szTmpOut ;
			lvi0.iImage = 1;
			m_ListSend.InsertItem(&lvi0);
			m_ListSend.SetItem(&lvi0);

			LVITEM lvi;		
			lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = tempCnt;		
			lvi.iSubItem = 1;
			lvi.pszText = "대기중" ; 
			lvi.iImage = 1;
			m_ListSend.InsertItem(&lvi);
			m_ListSend.SetItem(&lvi);
		
		}
		else
		{
			break;
		}
	}
	///////////////////////////////////////////////////////////////////
	SetResize();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CFileSendDlg::OnDisconnected (WPARAM wParam, LPARAM lParam)
{
	if(m_nFileSent != m_nTotalFileCnt)
	{
		AfxMessageBox("파일 전송이 완료되기전에, 종료되었습니다.");	
		CDialog::OnOK();
	}

	return TRUE; 
}

void CFileSendDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
}

BOOL CFileSendDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}
