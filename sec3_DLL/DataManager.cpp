// DataManager.cpp: implementation of the CDataManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "MemoCal.h"
#include "DataManager.h"
#include "MemoItemDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataManager::CDataManager()
{
	m_pOwnerWnd = NULL;
	m_pCalendarCtrl = NULL;
	m_pImageList = NULL;
	m_nIndex = -1;
	m_bDocOpened = false;

	CoInitialize(NULL);
}


CDataManager::~CDataManager()
{
	if( m_pNodeList != NULL ) m_pNodeList.Release();
	if( m_pDoc != NULL ) m_pDoc.Release();

	CoUninitialize();
}

void CDataManager::SetOwnerWnd(CWnd* pWnd)
{
	m_pOwnerWnd = pWnd;
}

void CDataManager::SetCalendarCtrl(CCalendarCtrl* pCtrl)
{
	m_pCalendarCtrl = pCtrl;
}

void CDataManager::SetImageList(CImageList* pImageList)
{
	m_pImageList = pImageList;
}

/*
// msxml.dll를 리소스에 포함하고 있다가 자동 등록하는 루틴
bool CDataManager::CopyFileFromResourceToSystemFolder(int nIDResource, CString strFilename)
{
	HRSRC hrSrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nIDResource), "XML_DLL");

	if( hrSrc == NULL )	return false;

    HGLOBAL hGlobal = LoadResource(AfxGetResourceHandle(), hrSrc);
    if(hGlobal == NULL) return false;

    LPVOID lpDll = LockResource(hGlobal);
    if(lpDll == NULL) return false;

    //Create the new archive from the extractor in the resources
    CFile file;
    DWORD dwSfxSize;

	if( !IsValidFile((LPCTSTR)strFilename) )
	{
		if(!file.Open(strFilename, CFile::modeCreate | CFile::modeWrite))
		{
			return false;
		}
		else
		{
			file.Write(lpDll, (UINT)SizeofResource(AfxGetResourceHandle(), hrSrc));
			dwSfxSize = file.GetPosition();
			file.Close();
		}
	}

	return true;
}

bool CDataManager::RegisterXMLDll()
{
	TCHAR szSysPath[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];
	CString strFilename;

	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;

	_tcscpy(szBuffer, szSysPath);
	strFilename = _tcscat(szBuffer, _T("\\msxml4r.dll"));
	CopyFileFromResourceToSystemFolder(IDR_XML_DLL2, strFilename);
	_tcscpy(szBuffer, szSysPath);
	strFilename = _tcscat(szBuffer, _T("\\msxml4a.dll"));
	CopyFileFromResourceToSystemFolder(IDR_XML_DLL3, strFilename);

	// msxml4.dll 등록
	_tcscpy(szBuffer, szSysPath);
	strFilename = _tcscat(szBuffer, _T("\\msxml4.dll"));
	if( CopyFileFromResourceToSystemFolder(IDR_XML_DLL1, strFilename) )
	{
		strFilename = _T("/s ") + strFilename;

		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.hwnd = m_pOwnerWnd->m_hWnd;
		sei.lpVerb = _T("open");
		sei.lpFile = _T("regsvr32.exe");
		sei.lpParameters = (LPTSTR)(LPCTSTR)strFilename;
		sei.lpDirectory = NULL;
		sei.nShow = SW_HIDE;
		sei.fMask = SEE_MASK_NOCLOSEPROCESS; // WaitForSingleObject 함수를 호출하기 위해

		if( ShellExecuteEx(&sei) == 0 )
		{
			if(sei.hProcess)
				TerminateProcess(sei.hProcess, 0);
			return false;
		}
		else
		{
			WaitForSingleObject(sei.hProcess, INFINITE);
		}
		TerminateProcess(sei.hProcess, 0);
	}

	return true;
}
*/

int ExCopyFile(CString strFrom, CString strTo) 
{ 
    SHFILEOPSTRUCT shfo;  
    char pszFrom[1024] = {0}; 
    char pszTo[1024] = {0}; 
    WORD wFlag = 0; 
    
	strcpy(pszFrom, (LPCTSTR)strFrom); 
    strcpy(pszTo, (LPCTSTR)strTo); 

    pszFrom[lstrlen(pszFrom) + 1] = 0; 

    shfo.hwnd =  NULL;
    shfo.wFunc = FO_COPY; 
    shfo.pFrom = pszFrom; 
    shfo.pTo = pszTo; 
    shfo.fFlags = 0; 
    shfo.lpszProgressTitle = "File Copy.."; 

    return SHFileOperation(&shfo); 
} 

bool CDataManager::RegisterXMLDll()
{
	TCHAR szSysPath[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];
	CString strFilename;
		
	CString strFileName;
	char szWinDir[512];		
	char szDirTmp[512];
	char szDir[1024];
	memset(szWinDir, 0x00, sizeof(szWinDir));		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	
	memset(szDir, 0x00, sizeof(szDir));	
	GetWindowsDirectory(szWinDir, sizeof(szWinDir));	
	GetModuleFileName ( GetModuleHandle("IdmsMsn"), szDirTmp, sizeof(szDirTmp));
	// "D:\DATA\DevelopSource\CD8NotBK\newMessenger\BinRelease\IdmsMsn.exe"
	// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\newMessenger\ 까지 구한다.	
	int nPos = 0;
	int nLen = strlen(szDirTmp);
	for( int i = nLen; i>=0 ; i--)
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
	strTmp.Format("%s\\msxml4.dll", szDirTmp);
	TRACE("%s\n" , strFileName);	

	// msxml4.dll 등록
	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	strFilename = _tcscat(szBuffer, _T("\\msxml4.dll"));

	//strFilename = _T("msxml4.dll"); //TEST
	if( !IsValidFile((LPCTSTR)strFilename) ) 
	{
		ExCopyFile(strTmp, strFilename);
	}
	
	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename2 = _tcscat(szBuffer, _T("\\msxml4.inf"));	
	if( !IsValidFile((LPCTSTR)strFilename2) ) 
	{		
		strTmp.Format("%s\\msxml4.inf", szDirTmp);
		ExCopyFile(strTmp, strFilename2);
	
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename3 = _tcscat(szBuffer, _T("\\msxml4a.dll"));	
	if( !IsValidFile((LPCTSTR)strFilename3) ) 
	{		
		strTmp.Format("%s\\msxml4a.dll", szDirTmp);
		ExCopyFile(strTmp, strFilename3);
	
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	if( ::GetSystemDirectory(szSysPath, MAX_PATH) == 0) return false;
	_tcscpy(szBuffer, szSysPath);
	CString strFilename4 = _tcscat(szBuffer, _T("\\msxml4r.dll"));	
	if( !IsValidFile((LPCTSTR)strFilename4) ) 
	{		
		strTmp.Format("%s\\msxml4r.dll", szDirTmp);
		ExCopyFile(strTmp, strFilename4);
	
	}
	
	strFilename = _T("/s ") + strFilename;

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.hwnd = m_pOwnerWnd->m_hWnd;
	sei.lpVerb = _T("open");
	sei.lpFile = _T("regsvr32.exe");
	sei.lpParameters = (LPTSTR)(LPCTSTR)strFilename;
	sei.lpDirectory = NULL;
	sei.nShow = SW_HIDE;
	sei.fMask = SEE_MASK_NOCLOSEPROCESS; // WaitForSingleObject 함수를 호출하기 위해

	if( ShellExecuteEx(&sei) == 0 )
	{
		if(sei.hProcess)
			TerminateProcess(sei.hProcess, 0);
		return false;
	}
	else
	{
		WaitForSingleObject(sei.hProcess, INFINITE);
	}
	TerminateProcess(sei.hProcess, 0);

	return true;
}

bool CDataManager::OpenDocument(const CString strFilename)
{
	HRESULT hr;
	CWaitCursor wait;

	hr = m_pDoc.CreateInstance(__uuidof(DOMDocument40));
	if( FAILED(hr) )
	{
		if( !RegisterXMLDll() )
		{
			TRACE(_T("Failed to install XML DOM.\r\n"));
			AfxMessageBox(_T("'MSXML 4.0'을 등록하는데 실패 하였습니다.\n"), MB_ICONERROR);
			return false;
		}
		else
		{
			hr = m_pDoc.CreateInstance(__uuidof(DOMDocument40));
			if( FAILED(hr) )
			{
				TRACE(_T("Failed to instantiate an XML DOM.\r\n"));
				AfxMessageBox(_T("'MSXML 4.0' 초기화 실패 하였습니다."), MB_ICONERROR);
				return false;
			}
		}
	}

	if( !IsValidFile((LPCTSTR)strFilename) )
		MakeEmptyFile(strFilename);
	
	if( m_pDoc->load((LPCTSTR)strFilename) != VARIANT_TRUE )
	{
		CString sError;
		sError.Format(_T("%s\r\n\r\nLine : %d\r\nCol : %d\r\nreason : %s"),
			(LPCTSTR)"다음과 같은 오류가 있어 달력 조회만 가능합니다.",
			m_pDoc->parseError->Getline(),
			m_pDoc->parseError->Getlinepos(),
			(LPCTSTR)m_pDoc->parseError->Getreason());
		AfxMessageBox(sError, MB_ICONERROR);
		return false;
	}

	m_strFilename = strFilename;
	m_bDocOpened = true;

	return true;
}

bool CDataManager::SaveDocument()
{
	if( !m_bDocOpened ) return false;

	CWaitCursor wait;
	HRESULT hr = m_pDoc->save((LPCTSTR)m_strFilename);
	wait.Restore();

	if( FAILED(hr) ) return false;

	return true;
}

bool CDataManager::IsDocumentOpend()
{
	return m_bDocOpened;
}

long CDataManager::FindFirstItem(CKoreanDate date, TCellItem* pItem)
{
	long lCount = 0;
	CString sQuery;

	if( !m_bDocOpened ) return 0;

	// 해당 일자의 메모를 모두 찾는다
	// XPath example : /memos/memo[@year='2002' and @month='03' and @day='06']
	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodePtr     pItemNode, pNode;
	m_nIndex = -1;

	try
	{
		m_pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		lCount = m_pNodeList->length;

		if( lCount > 0 )
		{
			pItemNode = m_pNodeList->Getitem(0);
			pNode = pItemNode->attributes->getNamedItem(L"icon");
			pItem->nImage = _ttoi( (LPCTSTR)pNode->text );
			pNode = pItemNode->selectSingleNode(L"title");
			pItem->strCaption = (LPCTSTR)pNode->text;

			m_nIndex++;
		}
		else
		{
			pItem->nImage = -1;
			pItem->strCaption = _T("");
		}
	}
	catch(...)
	{
		pItem->nImage = -1;
		pItem->strCaption = _T("");
		return 0;
	}

	return lCount;
}

bool CDataManager::FindNextItem(TCellItem* pItem)
{
	if( !m_bDocOpened ) return false;
	if( m_nIndex < 0 ) return false;

	MSXML2::IXMLDOMNodePtr     pItemNode, pNode;

	try
	{
		if( m_nIndex + 2 > m_pNodeList->length ) return false;

		pItemNode = m_pNodeList->Getitem(m_nIndex + 1);
		pNode = pItemNode->attributes->getNamedItem(L"icon");
		pItem->nImage = _ttoi( (LPCTSTR)pNode->text );
		pNode = pItemNode->selectSingleNode(L"title");
		pItem->strCaption = (LPCTSTR)pNode->text;

		m_nIndex++;
	}
	catch(...)
	{
		pItem->nImage = -1;
		pItem->strCaption = _T("");
		return false;
	}

	return true;
}

bool CDataManager::GetItem(CKoreanDate date, int nIndex, TCellItem* pItem)
{
	long lCount = 0;
	CString sQuery;

	if( !m_bDocOpened ) return false;

	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodePtr     pItemNode, pNode;
	m_nIndex = -1;

	try
	{
		m_pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		lCount = m_pNodeList->length;
		if( nIndex > lCount - 1 ) return false;

		if( lCount > 0 )
		{
			pItemNode = m_pNodeList->Getitem(nIndex);
			pNode = pItemNode->attributes->getNamedItem(L"icon");
			pItem->nImage = _ttoi( (LPCTSTR)pNode->text );

			//pNode = pItemNode->attributes->getNamedItem(L"AlarmHH");
			//pItem-> = (LPCTSTR)pNode->text ;

			pNode = pItemNode->selectSingleNode(L"title");
			pItem->strCaption = (LPCTSTR)pNode->text;

			m_nIndex = nIndex;
		}
		else
		{
			pItem->nImage = -1;
			pItem->strCaption = _T(""); ;
		}
	}
	catch(...)
	{
		pItem->nImage = -1;
		pItem->strCaption = _T("");
		return false;
	}

	return true;
}

// 파일 존재 여부 검사
bool CDataManager::IsValidFile(LPCTSTR lpszFilename)
{
	OFSTRUCT of;
	
	// Validate filename pointer
	if(lpszFilename == NULL || !*lpszFilename){
		return false;
	}

	// open the file for checking existance
	if(::OpenFile(lpszFilename, &of, OF_EXIST) == -1){
		return false;
	}

	return true;
}

bool CDataManager::MakeEmptyFile(CString strFilename)
{
	CFile file;
	CFileException e;

	if( !file.Open(strFilename, CFile::modeCreate|CFile::modeWrite, &e) )
		return false;

	CString strData;
	strData = _T("<?xml version=\"1.0\" encoding=\"EUC-KR\" standalone=\"yes\"?>\r\n" \
		         "<!--XML data for Memo Calendar Application-->\r\n" \
				 "<memos version=\"1.0\">\r\n" \
                 "</memos>\r\n");
	file.Write((LPCTSTR)strData, strData.GetLength());
	file.Close();

	return true;
}

bool CDataManager::GetItemData(CKoreanDate date, int nIndex, TItemData* pData)
{
	long lCount = 0;
	CString sQuery;

	if( !m_bDocOpened ) return false;
	if( nIndex < 0 ) return false;

	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodeListPtr pNodeList;
	MSXML2::IXMLDOMNodePtr     pNode;

	try
	{
		pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		lCount = pNodeList->length;
		if( lCount == 0 || nIndex > lCount - 1 ) return false;

		pNode = pNodeList->Getitem(nIndex);

		pData->nIcon = _ttoi( (LPCTSTR)(pNode->attributes->getNamedItem(L"icon")->text) );		
		pData->strTitle = (LPCTSTR)(pNode->selectSingleNode(L"title")->text);
		pData->strContent = (LPCTSTR)(pNode->selectSingleNode(L"content")->text);

		pData->strAlarmTimeYYYYMMDDHHMI = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmTimeYYYYMMDDHHMI")->text) ;				
		pData->m_strAlarmTimeYYYY = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmTimeYYYY")->text) ;
		pData->m_strAlarmTimeMonth = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmTimeMonth")->text) ;
		pData->m_strAlarmTimeDD = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmTimeDD")->text) ;
		pData->strAlarmTimeHH = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmHH")->text) ;
		pData->strAlarmTimeMM = (LPCTSTR)(pNode->attributes->getNamedItem(L"AlarmMM")->text) ;

		pData->strContent.Replace(_T("\n"), _T("\r\n"));
		pData->strContent.Replace(_T("\r\r\n"), _T("\r\n"));
	}
	catch(...)
	{
		if	(	pData->strAlarmTimeYYYYMMDDHHMI.IsEmpty() &&
				pData->m_strAlarmTimeYYYY.IsEmpty() &&
				pData->m_strAlarmTimeMonth.IsEmpty() &&
				pData->m_strAlarmTimeDD.IsEmpty() &&
				pData->strAlarmTimeHH.IsEmpty() &&
				pData->strAlarmTimeMM.IsEmpty() 
			)
		{
			TRACE("CDataManager::GetItemData NOT ERROR");
			return true;
		}

		return false;
	}

	return true;
}

bool CDataManager::InsertItem(CKoreanDate date, TItemData* pData)
{
	if( !m_bDocOpened ) return false;

	MSXML2::IXMLDOMElementPtr	pRoot;
	MSXML2::IXMLDOMElementPtr	pNewItem;
	MSXML2::IXMLDOMElementPtr	pSubItem;
	MSXML2::IXMLDOMTextPtr		pText;
	CString s;

	try
	{
		pRoot = m_pDoc->documentElement;
		pNewItem = m_pDoc->createElement(L"memo");
		if( pNewItem == NULL ) return false;

		// year attribute
		s.Format(_T("%d"), date.GetYear());
		pNewItem->setAttribute(L"year", (_bstr_t)s);
		// month attribute
		s.Format(_T("%02d"), date.GetMonth());
		pNewItem->setAttribute(L"month", (_bstr_t)s);
		// day attribute
		s.Format(_T("%02d"), date.GetDay());
		pNewItem->setAttribute(L"day", (_bstr_t)s);
		// icon attribute
		s.Format(_T("%d"), pData->nIcon);
		pNewItem->setAttribute(L"icon", (_bstr_t)s);

		s.Format(_T("%s"), pData->strAlarmTimeYYYYMMDDHHMI);
		pNewItem->setAttribute(L"AlarmTimeYYYYMMDDHHMI", (_bstr_t)s);

		s.Format(_T("%s"), pData->m_strAlarmTimeYYYY);
		pNewItem->setAttribute(L"AlarmTimeYYYY", (_bstr_t)s);

		s.Format(_T("%s"), pData->m_strAlarmTimeMonth);
		pNewItem->setAttribute(L"AlarmTimeMonth", (_bstr_t)s);

		s.Format(_T("%s"), pData->m_strAlarmTimeDD);
		pNewItem->setAttribute(L"AlarmTimeDD", (_bstr_t)s);
		
		s.Format(_T("%s"), pData->strAlarmTimeHH);
		pNewItem->setAttribute(L"AlarmHH", (_bstr_t)s);

		s.Format(_T("%s"), pData->strAlarmTimeMM);
		pNewItem->setAttribute(L"AlarmMM", (_bstr_t)s);

		pText = m_pDoc->createTextNode(L"\n\t");
		pRoot->appendChild(pText);

		// 새 메모 추가
		pRoot->appendChild(pNewItem);

		pText = m_pDoc->createTextNode(L"\n\t\t");
		pNewItem->appendChild(pText);
		// title 추가
		pSubItem = m_pDoc->createElement(L"title");
		pSubItem->text = (_bstr_t)(pData->strTitle);
		pNewItem->appendChild(pSubItem);

		pText = m_pDoc->createTextNode(L"\n\t\t");
		pNewItem->appendChild(pText);
		// content 추가
		pSubItem = m_pDoc->createElement(L"content");
		pSubItem->text = (_bstr_t)(pData->strContent);
		pNewItem->appendChild(pSubItem);

		pText = m_pDoc->createTextNode(L"\n\t");
		pNewItem->appendChild(pText);

		/*
		pText = m_pDoc->createTextNode(L"\n");
		pRoot->appendChild(pText);
		*/

		SaveDocument();
	}
	catch(...)
	{
		return false;
	}

	return true;
}

bool CDataManager::UpdateItem(CKoreanDate date, int nIndex, TItemData* pData)
{
	if( !m_bDocOpened ) return false;
	if( nIndex < 0 ) return false;

	CString sQuery;
	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodeListPtr	pNodeList;
	MSXML2::IXMLDOMNodePtr		pChild;

	try
	{
		pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		pChild = pNodeList->Getitem(nIndex);
		if( pChild == NULL ) return false;

		sQuery.Format(_T("%d"), pData->nIcon);
		pChild->attributes->getNamedItem(L"icon")->text = (_bstr_t)sQuery;

		sQuery.Format(_T("%s"), pData->strAlarmTimeYYYYMMDDHHMI);
		pChild->attributes->getNamedItem(L"AlarmTimeYYYYMMDDHHMI")->text = (_bstr_t)sQuery;

		sQuery.Format(_T("%s"), pData->m_strAlarmTimeYYYY);
		pChild->attributes->getNamedItem(L"AlarmTimeYYYY")->text = (_bstr_t)sQuery;

		sQuery.Format(_T("%s"), pData->m_strAlarmTimeMonth);
		pChild->attributes->getNamedItem(L"AlarmTimeMonth")->text = (_bstr_t)sQuery;

		sQuery.Format(_T("%s"), pData->m_strAlarmTimeDD);
		pChild->attributes->getNamedItem(L"AlarmTimeDD")->text = (_bstr_t)sQuery;
						
		sQuery.Format(_T("%s"), pData->strAlarmTimeHH);
		pChild->attributes->getNamedItem(L"AlarmHH")->text = (_bstr_t)sQuery;

		sQuery.Format(_T("%s"), pData->strAlarmTimeMM);
		pChild->attributes->getNamedItem(L"AlarmMM")->text = (_bstr_t)sQuery;
		

		pChild->selectSingleNode(L"title")->text = (_bstr_t)(pData->strTitle);
		pChild->selectSingleNode(L"content")->text = (_bstr_t)(pData->strContent);

		SaveDocument();
	}
	catch(...)
	{
		return false;
	}

	return true;
}

bool CDataManager::DeleteItem(CKoreanDate date, int nIndex)
{
	if( !m_bDocOpened ) return false;
	if( nIndex < 0 ) return false;

	CString sQuery;
	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodeListPtr	pNodeList;
	MSXML2::IXMLDOMNodePtr		pChild;

	try
	{
		pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		pChild = pNodeList->Getitem(nIndex);
		if( pChild == NULL ) return false;
		pChild = m_pDoc->documentElement->removeChild(pChild);
		if( pChild == NULL ) return false;

		SaveDocument();
	}
	catch(...)
	{
		return false;
	}

	return true;
}

bool CDataManager::MoveItem(CKoreanDate dateFrom, int nIndex, CKoreanDate dateTo)
{
	if( !m_bDocOpened ) return false;

	return true;
}

void CDataManager::OnDayContextMenu(CKoreanDate date, CPoint point)
{
	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);
	
	if( !m_bDocOpened ) return;
	if( m_pOwnerWnd == NULL ) return;

	CMenu menu;
	menu.LoadMenu(IDM_POPUP);
	CMenu* pPopup = menu.GetSubMenu(0);  // 일자 팝업 메뉴

	pPopup->TrackPopupMenu(TPM_RIGHTALIGN, point.x, point.y, m_pOwnerWnd);
	menu.DestroyMenu();

	AfxSetResourceHandle(hInstResourceMain);
}

void CDataManager::OnItemContextmenu(CKoreanDate date, int nIndex, CPoint point)
{
	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);

	if( !m_bDocOpened ) return;
	if( m_pOwnerWnd == NULL ) return;

	CMenu menu;
	menu.LoadMenu(IDM_POPUP);
	CMenu* pPopup = menu.GetSubMenu(1);  // 아이템 팝업 메뉴

	pPopup->TrackPopupMenu(TPM_RIGHTALIGN, point.x, point.y, m_pOwnerWnd);
	menu.DestroyMenu();

	AfxSetResourceHandle(hInstResourceMain);
}

void CDataManager::OnDayDblClick(CKoreanDate date)
{
	OnMenuNewItem();
}

void CDataManager::OnItemDblClick(CKoreanDate date, int nIndex)
{
	OnMenuEditItem();
}

CString CDataManager::OnQueryDayTooltip(CKoreanDate date)
{
	long lCount = 0;
	CString strTip;
	CString sQuery;

	if( !m_bDocOpened ) return CString(_T(""));

	sQuery.Format(_T("/memos/memo[@year='%d' and @month='%02d' and @day='%02d']"),
		date.GetYear(), date.GetMonth(), date.GetDay() );

	MSXML2::IXMLDOMNodeListPtr pNodeList;
	MSXML2::IXMLDOMNodePtr     pNode;

	try
	{
		pNodeList = m_pDoc->selectNodes( (_bstr_t)sQuery );
		lCount = pNodeList->length;

		strTip.Format(_T("양력 : %s\n음력 : %s"), date.GetDateString()
		                	                    , date.GetDateString(TYPE_LUNAR));

		if( lCount > 0 )
		{
			sQuery.Format(_T("\n\n%d개의 메모가 있습니다."), lCount);
		}
		else
		{
			sQuery.Format(_T("\n\n저장된 메모가 없습니다.\n" \
				             "새로 추가하시려면 엔터를 누르세요."));
		}
		strTip += sQuery;
	}
	catch(...)
	{
		return CString(_T(""));
	}

	return CString((LPCTSTR)strTip);
}

void CDataManager::OnDayKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch( nChar )
	{
	case VK_RETURN:
		OnMenuNewItem();
		break;
	}
}


void CDataManager::OnItemKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch( nChar )
	{
	case VK_RETURN:
		OnMenuEditItem();
		break;
	case VK_DELETE:
		OnMenuDeleteItem();
		break;
	}
}

void CDataManager::OnMenuNewItem()
{
	
	if( !m_bDocOpened ) return;

	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);

	CString sDlgTitle;
	CKoreanDate date;
	CMemoItemDlg dlg;

	date = m_pCalendarCtrl->GetDate();
	sDlgTitle.Format(_T("[새 메모] - %s(음력 %s)"), date.GetDateString()
		                                          , date.GetDateString(TYPE_LUNAR));

	dlg.SetImageList(m_pImageList);
	dlg.SetIconIndex(0);
	dlg.m_strDlgTitle = sDlgTitle;
	if( dlg.DoModal() == IDOK )
	{
		TItemData itemdata;
		itemdata.nIcon = dlg.GetIconIndex();
		itemdata.strTitle = dlg.GetTitle();
		itemdata.strContent = dlg.GetContent();
		
		itemdata.strAlarmTimeYYYYMMDDHHMI = dlg.strAlarmTimeYYYYMMDDHHMI;

		itemdata.m_strAlarmTimeYYYY = dlg.m_strAlarmTimeYYYY;
		itemdata.m_strAlarmTimeMonth = dlg.m_strAlarmTimeMonth;
		itemdata.m_strAlarmTimeDD = dlg.m_strAlarmTimeDD;		
	
		itemdata.strAlarmTimeHH = dlg.m_strAlarmTimeHH;
		itemdata.strAlarmTimeMM = dlg.m_strAlarmTimeMM;

		if( InsertItem(date, &itemdata) )
		{
			m_pCalendarCtrl->OnItemInserted(date);
		}
	}
	
	m_pCalendarCtrl->SetFocus();

	AfxSetResourceHandle(hInstResourceMain);
}

void CDataManager::OnMenuEditItem()
{
	if( !m_bDocOpened ) return;
	
	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);

	CString sDlgTitle;
	CKoreanDate date;
	int nIndex;
	TItemData itemdata;
	CMemoItemDlg dlg;

	date = m_pCalendarCtrl->GetDate();
	nIndex = m_pCalendarCtrl->GetSelectedItemIndex();
	if( !GetItemData(date, nIndex, &itemdata) ) return;

	sDlgTitle.Format(_T("[메모 편집] - %s(음력 %s)"), date.GetDateString()
		                                            , date.GetDateString(TYPE_LUNAR));

	dlg.m_bNewItem = true;
	dlg.SetImageList(m_pImageList);
	dlg.SetIconIndex(itemdata.nIcon);
	dlg.SetTitle(itemdata.strTitle);
	dlg.SetContent(itemdata.strContent);
	dlg.strAlarmTimeYYYYMMDDHHMI = itemdata.strAlarmTimeYYYYMMDDHHMI;

	dlg.m_strAlarmTimeYYYY = itemdata.m_strAlarmTimeYYYY;
	dlg.m_strAlarmTimeMonth = itemdata.m_strAlarmTimeMonth;
	dlg.m_strAlarmTimeDD = itemdata.m_strAlarmTimeDD;

	dlg.m_strAlarmTimeHH = itemdata.strAlarmTimeHH;
	dlg.m_strAlarmTimeMM = itemdata.strAlarmTimeMM;
	dlg.m_strDlgTitle = sDlgTitle;

	if( dlg.DoModal() == IDOK )
	{
		itemdata.nIcon = dlg.GetIconIndex();
		itemdata.strTitle = dlg.GetTitle();
		itemdata.strContent = dlg.GetContent();
		itemdata.strAlarmTimeYYYYMMDDHHMI = dlg.strAlarmTimeYYYYMMDDHHMI;

		itemdata.m_strAlarmTimeYYYY = dlg.m_strAlarmTimeYYYY;
		itemdata.m_strAlarmTimeMonth = dlg.m_strAlarmTimeMonth;
		itemdata.m_strAlarmTimeDD = dlg.m_strAlarmTimeDD;
	
		itemdata.strAlarmTimeHH = dlg.m_strAlarmTimeHH;
		itemdata.strAlarmTimeMM = dlg.m_strAlarmTimeMM;

		if( UpdateItem(date, nIndex, &itemdata) )
		{
			m_pCalendarCtrl->OnItemUpdated(date, nIndex);
		}
	}

	m_pCalendarCtrl->SetFocus();

	AfxSetResourceHandle(hInstResourceMain);
}

void CDataManager::OnMenuDeleteItem()
{
	if( !m_bDocOpened ) return;

	HINSTANCE hInstResourceMain = AfxGetResourceHandle();
	HMODULE hModule = ::GetModuleHandle("sec3_DLL.dll");
	AfxSetResourceHandle(hModule);
	
	CKoreanDate date;
	int nIndex;

	date = m_pCalendarCtrl->GetDate();
	nIndex = m_pCalendarCtrl->GetSelectedItemIndex();

	if( AfxMessageBox(_T("삭제 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) == IDYES )
	{
		if( DeleteItem(date, nIndex) )
		{
			m_pCalendarCtrl->OnItemDeleted(date, nIndex);
		}
	}

	m_pCalendarCtrl->SetFocus();

	AfxSetResourceHandle(hInstResourceMain);
}