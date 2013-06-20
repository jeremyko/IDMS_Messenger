// AllMsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "MsgBoxThread.h"
#include "AllMsgDlg.h"
#include "FileTransSock.h"
#include "INIManager.h"

extern "C"
{
    #include <direct.h>
}

#include "..\\common\\AllCommonDefines.h"
#include "..\\common\\utility.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAllMsgDlg dialog

CAllMsgDlg::CAllMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAllMsgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAllMsgDlg)
	m_StrAllMsg = _T("");
	m_bChkResend = FALSE;
	m_RadioAllMsgTo = -1;
	//}}AFX_DATA_INIT

	
}

CAllMsgDlg::CAllMsgDlg(CStringArray& AryID, CStringArray& AryName)
{
	m_StringAryID.Copy(AryID) ; 
	m_StringAryName.Copy(AryName) ; 
	m_CEditAllMsg.m_nParentKind = C_ALLMSGDLG;

	m_bChkResend = FALSE;
	
}


void CAllMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAllMsgDlg)
	DDX_Control(pDX, IDC_BUTTON_EXPANDALL, m_btnExpandAll);	
	DDX_Control(pDX, IDC_CHECK_RESEND, m_btnResend);
	DDX_Control(pDX, IDC_TREE_ALL_MSG, m_TreeAllMsg);
	DDX_Control(pDX, IDC_BUTTON_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDC_CHK_SELECT_ALL, m_chkAll);
	DDX_Control(pDX, IDC_BTN_SENDALLMSG, m_BtnSendAllMsg);
	DDX_Control(pDX, IDC_LIST_ALLMSG_STATUS, m_CListCtrlAllMsgStatus);
	DDX_Control(pDX, IDC_EDIT_ALLMSG, m_CEditAllMsg);
	DDX_Text(pDX, IDC_EDIT_ALLMSG, m_StrAllMsg);
	DDV_MaxChars(pDX, m_StrAllMsg, 102400);
	DDX_Check(pDX, IDC_CHECK_RESEND, m_bChkResend);
	DDX_Radio(pDX, IDC_RADIO_SELTEAM, m_RadioAllMsgTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAllMsgDlg, CDialog)
	//{{AFX_MSG_MAP(CAllMsgDlg)
	ON_BN_CLICKED(IDC_BTN_ALLMSG_OK, OnBtnAllmsgOk)	
	ON_BN_CLICKED(IDC_BTN_SENDALLMSG, OnBtnSendallmsg)
	ON_WM_TIMER()
	ON_EN_SETFOCUS(IDC_EDIT_ALLMSG, OnSetfocusEditAllmsg)	
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHK_SELECT_ALL, OnChkSelectAll)
	ON_BN_CLICKED(IDC_RADIO_SELID, OnRadioSelid)
	ON_BN_CLICKED(IDC_RADIO_SELTEAM, OnRadioSelteam)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnButtonUpdate)
	ON_NOTIFY(NM_CLICK, IDC_TREE_ALL_MSG, OnClickTreeAllMsg)
	ON_BN_CLICKED(IDC_BUTTON_EXPANDALL, OnButtonExpandall)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ALLMSG_ACK,   OnAllMsgAck)
	ON_MESSAGE(WM_BUDDY_OFFLINE,OnAllMsgStop)	
	ON_MESSAGE(WM_BUDDY_ONLINE,   OnAllMsgResume)	
	ON_MESSAGE(WM_TO_ALLMSG,    OnAllMsgIPData)	
	ON_MESSAGE(WM_ALL_USER_ID_TEAM,    OnAllUserIdTeam)	
	
	
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAllMsgDlg message handlers

void CAllMsgDlg::FileDropped(const char* szFileName, int nLen, int nFileCnt)
{
	UpdateData(TRUE);

	if(m_RadioAllMsgTo == 1)
	{
		// 선택 사용자
		int nSize = m_StringAryID.GetSize();	
		CString strID;
		for(int i=0; i < nSize; i++)
		{
			strID = m_StringAryID.ElementAt(i);	
			
			if(strncmp(CChatSession::Instance().m_szMyIPAddr, (LPCSTR)strID, sizeof(CChatSession::Instance().m_szMyIPAddr)) != 0 )
			{
				CBuddyInfo * pCompanion;
						
				if( CChatSession::Instance().mBuddyInfoMap.Lookup(strID, ( CObject*& ) pCompanion ))
				{
					if(pCompanion)
					{
						TRACE("szFileName [%s]\n",szFileName);
			
						CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr);
						
						if( NULL == pSpckObjMap )
						{					
							TRACE("socket connect fail! [%s]\n", szFileName);	
													
							CString strErrMsg;
							strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
							CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
							pMsgThread->CreateThread();
						}
						else
						{				
							TRACE("\nsocket connected! [%s]\n", szFileName);
							
							CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID, CString(pCompanion->m_szIpAddr), 
																		(LPSTR)(LPCSTR) szFileName , nFileCnt) ;
						}
					}
				}
			}		
		}
	}
	else
	{		
		SelRcvPerson(); 		
			
		///////////////////////////////////////

		// 선택했는지 체크 
		if(m_nCntSelIDOnLine == 0 )
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "파일을 보낼수 없습니다. OnLine 된 상대방이 없습니다!.");
			pMsgThread->CreateThread();
			return;
		}

		HTREEITEM hChildItem ;
		HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		CString ItemText, ItemTextTmp;
			
		while (hCurrent != NULL)
		{
			if (m_TreeAllMsg.ItemHasChildren(hCurrent))
			{
				hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);

				if(m_TreeAllMsg.GetCheck(hChildItem))
				{					
					S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
					
					if(pData->m_nConStatus == 1)
					{
						// OnLine일때만 
						CBuddyInfo * pCompanion;
						
						if( CChatSession::Instance().mBuddyInfoMap.Lookup(pData->m_szID, ( CObject*& ) pCompanion ))
						{
							if(pCompanion)
							{
								TRACE("szFileName [%s]\n",szFileName);
								
								CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr);
								
								if( NULL == pSpckObjMap )
								{					
									TRACE("socket connect fail! [%s]\n", szFileName);	
									
									CString strErrMsg;
									strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
									CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
									pMsgThread->CreateThread();
								}
								else
								{				
									TRACE("\nsocket connected! [%s]\n", szFileName);
									
									CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID, CString(pCompanion->m_szIpAddr), 
										(LPSTR)(LPCSTR) szFileName , nFileCnt) ;
								}
							}
						}						
					}					
				}
				
				while (hChildItem != NULL)
				{
					hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
					if(hChildItem)
					{
						if(m_TreeAllMsg.GetCheck(hChildItem))
						{							
							S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
												
							if(pData->m_nConStatus == 1)
							{
								// OnLine일때만 
								CBuddyInfo * pCompanion;
						
								if( CChatSession::Instance().mBuddyInfoMap.Lookup(pData->m_szID, ( CObject*& ) pCompanion ))
								{
									if(pCompanion)
									{
										TRACE("szFileName [%s]\n",szFileName);
										
										CSockObjMapInfoFile * pSpckObjMap = CFileTransSock::Instance().SocketConnect(C_FILE_SVR_PORT, (LPSTR)(LPCSTR)pCompanion->m_szIpAddr);
										
										if( NULL == pSpckObjMap )
										{					
											TRACE("socket connect fail! [%s]\n", szFileName);	
											
											CString strErrMsg;
											strErrMsg.Format("%s 에 접속할수 없습니다", pCompanion->m_szIpAddr);
											CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, strErrMsg);	
											pMsgThread->CreateThread();
										}
										else
										{				
											TRACE("\nsocket connected! [%s]\n", szFileName);
											
											CFileTransSock::Instance().InformFileSend( pSpckObjMap, pCompanion->m_szBuddyID, 
												CString(pCompanion->m_szIpAddr), 
												(LPSTR)(LPCSTR) szFileName , nFileCnt) ;
										}
									}
								}								
							}
						}
					}				
				}		
			}

			hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
		}
	}	
}

LRESULT CAllMsgDlg::OnAllMsgStop(WPARAM wParam, LPARAM lParam)
{
	char szTmpID[20+1];
	char szMsg[20+1+10];

	memset(szTmpID, 0x00, sizeof(szTmpID));	
	memset(szMsg, 0x00, sizeof(szMsg));	

	strcpy(szTmpID, (char*)lParam);	
	
	// 해당 ID 를 ListCtrl에서 찾아서 OffLine 표시 

	if(m_RadioAllMsgTo == 1)
	{
		// 선택 사용자		
		
		LVFINDINFO info;
		int nIndex = -1;

		info.flags = LVFI_PARAM;	

		info.flags = LVFI_PARTIAL|LVFI_STRING;
		info.psz = szTmpID;		
		nIndex = m_CListCtrlAllMsgStatus.FindItem(&info);
		
		if(nIndex != -1)
		{
			LVITEM rItem;				
								
			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 0;		
			strcpy(szMsg, "(OffLine)");
			strcat(szMsg, szTmpID);
			rItem.pszText = szMsg;
			rItem.iImage = 2;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);

			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 1;	
			rItem.pszText = "OffLine";
			rItem.iImage = -1;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);
			
		}
		
		FlashWindow(TRUE);
		/*
		FLASHWINFO fwi;
		fwi.cbSize  = sizeof(FLASHWINFO);
		fwi.hwnd =  this->GetSafeHwnd();
		fwi.dwFlags = FLASHW_ALL ;
		fwi.uCount  = 3; 
		fwi.dwTimeout = 0; 
		FlashWindowEx(&fwi);		
		*/

		//m_CEditAllMsg.EnableWindow(FALSE);
		//m_BtnSendAllMsg.EnableWindow(FALSE);
	}
	else
	{
		// Team
		/* 의미없다. 서버에서는 내 대화목록에 대해서만 OFFLINE을 알려준다.
		// 팀별 선택 => 트리에서 OffLine으로 변경		
		HTREEITEM hChildItem ;
		HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		CString ItemText, ItemTextTmp;
		S_ALLMSG_TREEINFO* pData = NULL;	
		while (hCurrent != NULL)
		{
			if (m_TreeAllMsg.ItemHasChildren(hCurrent))
			{
				hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
				pData = (S_ALLMSG_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem) ;

				if( strcmp( szTmpID, pData->m_szID ) == 0 )
				{
					m_TreeAllMsg.SetItemImage(hChildItem,1, 1);
					pData->m_nConStatus = 0;
					m_TreeAllMsg.SetItemData(hChildItem, (DWORD)pData);
				}
				
				
				while (hChildItem != NULL)
				{
					hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
					
					if(hChildItem)
					{
						pData = (S_ALLMSG_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem) ;
						
						if( strcmp( szTmpID, pData->m_szID ) == 0 )
						{
							m_TreeAllMsg.SetItemImage(hChildItem,1, 1);
							pData->m_nConStatus = 0;
							m_TreeAllMsg.SetItemData(hChildItem, (DWORD)pData);
						}
					}				
				}		
			}

			hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
		}
		*/
	}

	return TRUE;
}


LRESULT CAllMsgDlg::OnAllMsgResume(WPARAM wParam, LPARAM lParam)
{	
	char szTmpID[20+1];
	char szTmp[20+1+10];
	memset(szTmpID, 0x00, sizeof(szTmpID));	
	memset(szTmp, 0x00, sizeof(szTmp));	

	strcpy(szTmpID, (char*)lParam);	

	if(m_RadioAllMsgTo == 1)
	{
		// 선택 사용자
		sprintf(szTmp, "(OffLine)%s", szTmpID);

		// 해당 ID 를 ListCtrl에서 찾아서 OnLine 표시 	
		LVFINDINFO info;
		int nIndex = -1;

		info.flags = LVFI_PARAM;	

		info.flags = LVFI_PARTIAL|LVFI_STRING;
		info.psz = szTmp;		
		nIndex = m_CListCtrlAllMsgStatus.FindItem(&info);
		
		if(nIndex != -1)
		{
			LVITEM rItem;				
						
			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 0;			
			rItem.pszText = szTmpID;
			rItem.iImage = 0;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);

			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 1;	
			rItem.pszText = "OnLine";
			rItem.iImage = -1;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);
			
		}
		
		FlashWindow(TRUE);
		/*
		FLASHWINFO fwi;
		fwi.cbSize  = sizeof(FLASHWINFO);
		fwi.hwnd =  this->GetSafeHwnd();
		fwi.dwFlags = FLASHW_ALL ;
		fwi.uCount  = 3; 
		fwi.dwTimeout = 0; 
		FlashWindowEx(&fwi);		
		*/
	}
	else
	{
		// Team
		/* 의미없다. 서버에서는 내 대화목록에 대해서만 ONLINE을 알려준다.
		// 팀별 선택 => 트리에서 OnLine으로 변경		
		HTREEITEM hChildItem ;
		HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		CString ItemText, ItemTextTmp;
		S_ALLMSG_TREEINFO* pData = NULL;	
		while (hCurrent != NULL)
		{
			if (m_TreeAllMsg.ItemHasChildren(hCurrent))
			{
				hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
				pData = (S_ALLMSG_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem) ;

				if( strcmp( szTmpID, pData->m_szID ) == 0 )
				{
					m_TreeAllMsg.SetItemImage(hChildItem,0, 0);
					pData->m_nConStatus = 1;
					m_TreeAllMsg.SetItemData(hChildItem, (DWORD)pData);
				}
				
				
				while (hChildItem != NULL)
				{
					hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
					
					if(hChildItem)
					{
						pData = (S_ALLMSG_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem) ;
						
						if( strcmp( szTmpID, pData->m_szID ) == 0 )
						{
							m_TreeAllMsg.SetItemImage(hChildItem,0, 0);
							pData->m_nConStatus = 1;
							m_TreeAllMsg.SetItemData(hChildItem, (DWORD)pData);
						}
					}				
				}		
			}

			hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
		}
		*/
	}
	
	return TRUE;
}


void CAllMsgDlg::OnBtnAllmsgOk() 
{
	// TODO: Add your control notification handler code here
	
	
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();	
		
	while (hCurrent != NULL)
	{
		if (m_TreeAllMsg.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
			
			//ItemTextTmp = m_TreeAllMsg.GetItemText(hChildItem);
			S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
			delete pData;
			pData = NULL;
			
			while (hChildItem != NULL)
			{
				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				if(hChildItem)
				{											
					S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
					
					delete pData;
					pData = NULL;
				}				
			}		
		}

		hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
	}	

	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_ALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);

	CDialog::OnOK();
}

LRESULT CAllMsgDlg::OnAllUserIdTeam(WPARAM wParam, LPARAM lParam) 
{
	char* pSzRawData = (char*) lParam ;
	
	m_strAllUserInfo = pSzRawData ;

	delete [] pSzRawData;

	SetTree();

	return TRUE;
}


LRESULT CAllMsgDlg::OnAllMsgIPData(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here
	
	// 기존 데이터에 +  , 중복 제거 	
		
	m_StringAryID.Append( *(CStringArray*) wParam ) ; 
	m_StringAryName.Append( *(CStringArray*) lParam ) ; 

	// 공지 대상이 변경될수 있으므로 다시 구성
	m_CListCtrlAllMsgStatus.DeleteAllItems();
	
	int nSize = m_StringAryID.GetSize();	
	for(int i=0; i < nSize; i++)
	{
		CString strID = m_StringAryID.ElementAt(i);	
		CString strName = m_StringAryName.ElementAt(i);

		// 중복된것은 저장안함.
		LVFINDINFO info;
		int nIndex = -1;

		info.flags = LVFI_PARAM;
		info.flags = LVFI_PARTIAL|LVFI_STRING;
		info.psz = (LPCSTR)strID;		
		nIndex = m_CListCtrlAllMsgStatus.FindItem(&info);
	
		if(nIndex != -1)
		{
			//#define TRACEX  TRACE("%s(%d) : ", __FILE__, __LINE__); TRACE
			TRACE("중복은 저장안함\n");
			continue;
		}
		
		CString strItem;		
		LVITEM lvi;		
		lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
		int tempCnt = m_CListCtrlAllMsgStatus.GetItemCount();
		lvi.iItem = tempCnt;		
				
		//ID
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strID);
		lvi.iImage = 0;
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
		//
		lvi.iSubItem = 1;
		lvi.pszText = "OnLine"; // 공지사항창이 열릴때는 온라인인것만 표시됨.
		lvi.iImage = -1;
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
		//Name
		lvi.iSubItem = 2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strName);		
		lvi.iImage = -1;
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);		
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
	}

	TRACE("OnAllMsgIPData\n");

	return TRUE;
}


int	CAllMsgDlg::SetTree()
{
	// 그룹을 등록한다.	
	HTREEITEM hSub;
	HTREEITEM hCutItem;
	BOOL      bFirst = TRUE;

	CRect rt;

	m_TreeAllMsg.DeleteAllItems();
	
	//m_strAllUserInfo	
	//////////////////////////////////////////////
	char szTmpOut [256];
	char szTmpUserID [20+1];
	char szTmpName[50+1];	
	char szTmpTeamName[50+1];
	int nPos = -1 ;
	int nIndex = 0;
	
	while( 1)
	{
		memset(szTmpOut, 0x00, sizeof(szTmpOut));
		nPos = GetStrByDelim( (LPSTR)(LPCSTR)m_strAllUserInfo, szTmpOut, DELIM1);
				
		if( strcmp(szTmpOut ,"NotFound") != 0 )		
		{
			// USER_ID, USER_NAME, TEAM_GN, TEAM_NAME, CON_STATUS
			switch(nIndex % 5)
			{
			case 0:	 // USER_ID			
				memset(szTmpUserID, 0x00, sizeof(szTmpUserID));
				strncpy(szTmpUserID, szTmpOut, sizeof(szTmpUserID));
				break;
			case 1: // USER_NAME
				memset(szTmpName, 0x00, sizeof(szTmpName));
				strncpy(szTmpName, szTmpOut, sizeof(szTmpName));
				break; 
			case 2:	// TEAM_GN			
				break;			
			case 3: // TEAM_NAME
				memset(szTmpTeamName, 0x00, sizeof(szTmpTeamName));
				strncpy(szTmpTeamName, szTmpOut, sizeof(szTmpTeamName));
				break;
			case 4: // CON_STATUS
				{
					if( bFirst)
					{
						hSub = m_TreeAllMsg.InsertItem( "TEMP", 0, 1);
						bFirst = FALSE;			
					}		
					
					// 자신은 목록에 안나오게 처리
					//////////////////////////////////////////////////
					if(strncmp(CChatSession::Instance().m_szMyUserID, szTmpUserID, sizeof(CChatSession::Instance().m_szMyUserID)) == 0 )
					{
						break;
					}
					//////////////////////////////////////////////////
					HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
					
					BOOL bFound = FALSE;
					
					//비교				
					while (hCurrent != NULL)
					{
						hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
						// 비교
						if(hCurrent)
						{
							CString ItemText = m_TreeAllMsg.GetItemText(hCurrent);
							
							if ( ItemText == szTmpTeamName )
							{	
								char szIDName [100];
								memset(szIDName, 0x00, sizeof(szIDName) );
								sprintf(szIDName, "%s [%s]", szTmpName, szTmpUserID );
								
								S_ALLID_TREEINFO* pData = new S_ALLID_TREEINFO ;								
								memset(pData, 0x00, sizeof(pData));

								if(strcmp("Y", szTmpOut) == 0)
								{
									// OnLine
									pData->m_nConStatus = 1;
									hCutItem=m_TreeAllMsg.InsertItem(szIDName, 0, 0, hCurrent);
								}
								else
								{
									// OffLine
									pData->m_nConStatus = 0;
									hCutItem=m_TreeAllMsg.InsertItem(szIDName, 1, 1, hCurrent);
								}
								
								strncpy(pData->m_szID, szTmpUserID, sizeof(pData->m_szID) );
								m_TreeAllMsg.SetItemData(hCutItem, (DWORD)pData);
								bFound = TRUE;								
							}
						}			
					}
					
					if(!bFound)
					{
						hSub     = m_TreeAllMsg.InsertItem( szTmpTeamName, 0, 0);

						char szIDName [100];
						memset(szIDName, 0x00, sizeof(szIDName) );
						sprintf(szIDName, "%s [%s]", szTmpName, szTmpUserID )	;							

						S_ALLID_TREEINFO* pData = new S_ALLID_TREEINFO ;								
						memset(pData, 0x00, sizeof(pData));

						if(strcmp("Y", szTmpOut) == 0)
						{
							// OnLine
							pData->m_nConStatus = 1;
							hCutItem = m_TreeAllMsg.InsertItem(szIDName, 0, 0, hSub);
						}
						else
						{
							// OffLine
							pData->m_nConStatus = 0;
							hCutItem = m_TreeAllMsg.InsertItem(szIDName, 1, 1, hSub);
						}					

						strncpy(pData->m_szID, szTmpUserID, sizeof(pData->m_szID) );
						m_TreeAllMsg.SetItemData(hCutItem, (DWORD)pData);
					}					
				}
				
				break;			
			}
		}		

		if( nPos <  0 )
		{				
			break;
		}	
		
		nIndex++;
	}
	//////////////////////////////////////////////
		

	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
	
	if(hCurrent)
	{
		CString ItemText = m_TreeAllMsg.GetItemText(hCurrent);
		
		if ( ItemText == "TEMP") // for safety..
		{
			m_TreeAllMsg.DeleteItem(hCurrent);
		}
	}
	
	m_bTreeMade = TRUE;

	return TRUE;
}	


// 전송 버튼 
void CAllMsgDlg::OnBtnSendallmsg() 
{
	// TODO: Add your control notification handler code here	

	UpdateData(TRUE);
	int i = 0;
	CString strMsg;
	
	m_CEditAllMsg.GetWindowText(strMsg);
	//strMsg+= DELIM1;
	strMsg.GetLength() ; 
		
	if(strMsg.GetLength() == 0 )
	{		
		CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "내용을 입력하세요.");	
		pMsgThread->CreateThread();
		return;
	}
	
	// 성능을 고려해서 Max 전송양을 정하자
	if(strMsg.GetLength() > MAX_CHAT_LEN )
		strMsg = strMsg.Mid(0, MAX_CHAT_LEN);

	if(m_RadioAllMsgTo == 1)
	{
		// 선택 사용자
		//먼저 수신상태를 모두 수신안됨 로 변경
		int numListCtrl = m_CListCtrlAllMsgStatus.GetItemCount();

		if(numListCtrl == 0)
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "공지 대상을 선택하세요.");	
			pMsgThread->CreateThread();
			return;
		}

		CString strIDNotAcked, strID, strToIDs, strStatus;

		for( i=0; i< numListCtrl; i++)
		{
			strID = m_CListCtrlAllMsgStatus.GetItemText(i, 0);
			strStatus = m_CListCtrlAllMsgStatus.GetItemText(i, 1 );
					
			LVITEM rItem;		
			ZeroMemory ( &rItem, sizeof(LVITEM) );
					
			rItem.iItem = i;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;
			rItem.iSubItem = 0;
			rItem.pszText = (LPTSTR)(LPCTSTR)(strID);		
			
			if(m_bChkResend)
			{
				//수신못한 사람에게 재전송 => 아이콘은 변동없음..
				if( strStatus == "OffLine" || strStatus != "수신")
				{				
					strToIDs+=strID;
					strToIDs+=DELIM1;
				}
			}
			else
			{
				//수신못한 사람에게 재전송 아닌경우 => 모두 수신 안함 상태로
				rItem.iImage = 2;

				m_CListCtrlAllMsgStatus.SetItem(&rItem);   		
				m_CListCtrlAllMsgStatus.Invalidate();

				strToIDs+=strID;
				strToIDs+=DELIM1;
			}		
		}		

		/*
		if(m_ChkResend)
		{
			//수신못한 사람에게 재전송
			// strToIDs
		}
		else
		{		
			nSize = m_StringAryID.GetSize();	
			
			for(int i=0; i < nSize; i++)
			{
				strID = m_StringAryID.ElementAt(i);	
				strToIDs+=strID;
				strToIDs+=DELIM1;
			}
		}
		*/
		
		// 보낸다 !!!
		
		char szTemp[20];
		char* pSzBuf = NULL;	
		int nTotalLen = 0;		
		S_COMMON_HEADER sComHeader;			
		//S_CHAT_HEADER_LENGTH
		S_CHAT_HEADER sChatHeader;

		memset(&sComHeader, NULL, sizeof(sComHeader));		
		memset(&sChatHeader, NULL, sizeof(sChatHeader));	
		memset(szTemp, NULL, sizeof(szTemp));	
		
		char szTempCnt[4+1+1];
		memset(szTempCnt, NULL, sizeof(szTempCnt));	
		sprintf(szTempCnt, "%d%c" , numListCtrl, DELIM1);
		
		char szSenderName[50+1+1+1];
		memset(szSenderName, NULL, sizeof(szSenderName));	
		sprintf(szSenderName, "%c%s%c" , DELIM2,(LPCSTR) CChatSession::Instance().m_LoginName,DELIM2);

		// 헤더부 설정
		// 메시지 영역 앞 20바이트를 이름으로 대체한다!
		int iLength = S_CHAT_HEADER_LENGTH + strlen(szTempCnt) +  strToIDs.GetLength() + strMsg.GetLength() + strlen(szSenderName); 
			
		memcpy(sComHeader.m_szUsage, ALL_MSG, sizeof(sComHeader.m_szUsage));		
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
		memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
		sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
		memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));			
		
		memcpy(sChatHeader.szIDFrom, CChatSession::Instance().m_szMyUserID, sizeof(sChatHeader.szIDFrom) ); 
		memcpy(sChatHeader.szIDTo, "", sizeof(sChatHeader.szIDTo) ); 
		
		// 전송데이터 모으기 

		// ToID갯수 DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 메시지 DELIM1 전송자이름
		
		TRACE(" strlen(szTempCnt) [%d]\n", strlen(szTempCnt));
		TRACE(" strToIDs.GetLength() [%d]\n", strToIDs.GetLength() );

		pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
		memset(pSzBuf, NULL, sizeof(pSzBuf));	
		memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
		memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );	
		
		memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, szTempCnt , strlen(szTempCnt));	
		memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt), (LPCSTR)strToIDs, strToIDs.GetLength()  );			
		memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strToIDs.GetLength(), 
			(LPCSTR)strMsg, strMsg.GetLength()  );	

		memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + strToIDs.GetLength()+strMsg.GetLength(), szSenderName, strlen(szSenderName) );	

		nTotalLen = COMMON_HEADER_LENGTH + iLength ;
		
		TRACE(" pSzBuf [%s]\n", (char*) pSzBuf );

		if(CChatSession::Instance().Send(pSzBuf, nTotalLen, "OnBtnSendallmsg" ) < 0)	
		{
			TRACEX("Send return FAIL!\n");		
			return  ;
		}
	}
	else
	{
		// 팀 => OffLine 사용자들이 존재 -> 쪽지로 보낸다.
		SelRcvPerson(); 		

		// 선택했는지 체크 
		
		int nLenOn = m_UserIDSelectedOnLine.GetLength();
		int nLenOff = m_UserIDSelectedOffLine.GetLength();
		
		//if(nLenOn == 0 && nLenOff == 0)
		if(nLenOn == 0 ) // 20061028 OnLine 상대에게만 전송하게끔 작성함, 추후 OffLine 작업...
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "공지 대상을 선택하세요.");	
			pMsgThread->CreateThread();
			return;
		}
		
		// tree에서 수신안된 상태로 아이콘 변경
		HTREEITEM hChildItem ;
		HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		CString ItemText, ItemTextTmp;
		
		while (hCurrent != NULL)
		{
			if (m_TreeAllMsg.ItemHasChildren(hCurrent))
			{
				hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);				
				S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);

				if(pData->m_nConStatus == 1)
				{
					//OnLine
					m_TreeAllMsg.SetItemImage(hChildItem,0, 0);						
				}	
				else
				{
					//OffLine
					m_TreeAllMsg.SetItemImage(hChildItem,1, 1);
				}				
				
				while (hChildItem != NULL)
				{
					hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
					
					if(hChildItem)
					{
						S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);

						if(pData->m_nConStatus == 1)
						{
							//OnLine
							m_TreeAllMsg.SetItemImage(hChildItem,0, 0);						
						}	
						else
						{
							//OffLine
							m_TreeAllMsg.SetItemImage(hChildItem,1, 1);
						}
					}				
				}		
			}
			
			hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
		}
		
		if( nLenOn > 0) 
		{
			// OnLine상대방에게 공지사항 전송 
			char szTemp[20];
			char* pSzBuf = NULL;	
			int nTotalLen = 0;		
			S_COMMON_HEADER sComHeader;			
			//S_CHAT_HEADER_LENGTH
			S_CHAT_HEADER sChatHeader;
			memset(&sComHeader, NULL, sizeof(sComHeader));		
			memset(&sChatHeader, NULL, sizeof(sChatHeader));	
			memset(szTemp, NULL, sizeof(szTemp));	
			
			char szTempCnt[4+1+1];
			memset(szTempCnt, NULL, sizeof(szTempCnt));	
			sprintf(szTempCnt, "%d%c" , m_nCntSelIDOnLine, DELIM1);

			char szSenderName[50+1+1+1];
			memset(szSenderName, NULL, sizeof(szSenderName));	
			sprintf(szSenderName, "%c%s%c" , DELIM2,(LPCSTR) CChatSession::Instance().m_LoginName,DELIM2);
			
			// 헤더부 설정			
			int iLength = S_CHAT_HEADER_LENGTH + (int) strlen(szTempCnt) +  m_UserIDSelectedOnLine.GetLength() + strMsg.GetLength() + strlen(szSenderName) ; 			
				
			memcpy(sComHeader.m_szUsage, ALL_MSG, sizeof(sComHeader.m_szUsage));		
			sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
			memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
			sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
			memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));			
			
			memcpy(sChatHeader.szIDFrom, CChatSession::Instance().m_szMyUserID, sizeof(sChatHeader.szIDFrom) ); 
			memcpy(sChatHeader.szIDTo, "", sizeof(sChatHeader.szIDTo) ); 
			
			// 전송데이터 모으기 

			// ToID갯수 DELIM1 ToID DELIM1 ToID DELIM1 ... DELIM1 메시지 DELIM1 전송자이름		
			
			TRACE(" strlen(szTempCnt) [%d]\n", strlen(szTempCnt));
			TRACE(" m_UserIDSelectedOnLine.GetLength() [%d]\n", m_UserIDSelectedOnLine.GetLength() );

			pSzBuf = new char [COMMON_HEADER_LENGTH + iLength  ]; 
			memset(pSzBuf, NULL, sizeof(pSzBuf));	
			memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
			memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sChatHeader, S_CHAT_HEADER_LENGTH );	
			
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH, szTempCnt , strlen(szTempCnt));	
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt), (LPCSTR)m_UserIDSelectedOnLine, m_UserIDSelectedOnLine.GetLength()  );			
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + m_UserIDSelectedOnLine.GetLength(), (LPCSTR)strMsg, strMsg.GetLength()  );
			memcpy(pSzBuf + COMMON_HEADER_LENGTH + S_CHAT_HEADER_LENGTH + strlen(szTempCnt) + m_UserIDSelectedOnLine.GetLength() + strMsg.GetLength(), szSenderName ,strlen(szSenderName) );	

			nTotalLen = COMMON_HEADER_LENGTH + iLength ;

			if(CChatSession::Instance().Send(pSzBuf, nTotalLen,"OnBtnSendallmsg") < 0)	
			{
				TRACEX("Send return FAIL!\n");		
				return  ;
			}
		}
		else
		{
			// OffLine상대방에게 공지사항 쪽지로 전송 
			// CChatSession::Instance().send_WriteSlipMsg( m_szToID, (LPCSTR)m_strSlipMsg);
		}		
	}	

	// 파일에 저장	
	CTime iDT =  CTime::GetCurrentTime();
	CString strTmp;	
	//CString strtmDetailed = iDT.Format("%Y%m%d%H%M%S");
	CString strtmSimple = iDT.Format("%Y%m%d");	

	/////////////////////////////////////////////////////////
	CString strFileName ="";
	CString strUseLogFolder = "";
	CString strLogFolderPath = "";
	CINIManager::Instance().GetValue(INI_USE_LOGFOLDER, strUseLogFolder);
		
	char szDirTmp[512];		
	memset(szDirTmp, 0x00, sizeof(szDirTmp));	

	if(strUseLogFolder == "Y")
	{
		CINIManager::Instance().GetValue(INI_LOGFOLDER_PATH, strLogFolderPath);
		strcpy( szDirTmp, (LPCSTR)strLogFolderPath);
		
		if(szDirTmp[ strlen(szDirTmp) - 1 ] == '\\')
		{
			szDirTmp[ strlen(szDirTmp) - 1 ] = 0x00 ;
		}
				
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}
	else
	{
		//char szWinDir[512];		
		char szDirTmp[512];
		char szDir[1024];

		//memset(szWinDir, 0x00, sizeof(szWinDir));		
		memset(szDirTmp, 0x00, sizeof(szDirTmp));	
		memset(szDir, 0x00, sizeof(szDir));	

		//GetWindowsDirectory(szWinDir, sizeof(szWinDir));		
		GetModuleFileName ( GetModuleHandle(IDMS_MSN_NAME), szDirTmp, sizeof(szDirTmp));

		// "D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\BinRelease\EasyP2P_Messenger.exe"
		// 이런 문자열에서 D:\DATA\DevelopSource\CD8NotBK\EasyP2P_Messenger\ 까지 구한다.
		
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

		TRACE("%s\n" , szDirTmp);	
		
		strFileName.Format("%s\\공지사항\\",szDirTmp);	
	}	
	
	if (_chdir( (LPCSTR)strFileName) != 0)
	{
		_mkdir((LPCSTR)strFileName);								
	}
	
	CString StrSndDate = iDT.Format("%Y%m%d/%H%M%S");	

	CString strFileNameAll;	
	strFileNameAll.Format("%s\\공지사항%s.txt", strFileName, strtmSimple);
	writeLogFile(strFileNameAll, "▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\r\n");
	writeLogFile(strFileNameAll, "★ 보낸날자: %s\r\n", StrSndDate);
	writeLogFile(strFileNameAll, "\r\n");		//공지메시지
	writeLogFile(strFileNameAll, "   %s\r\n", strMsg);		//공지메시지
	writeLogFile(strFileNameAll, "\r\n\r\n");			//공지메시지
	
	CMsgBoxThread* pMsgThread = new CMsgBoxThread( this, "공지사항을 전송하였습니다. \r\n전달 여부는 좌측 트리에서 확인 하세요.");
	pMsgThread->CreateThread();

}


int	CAllMsgDlg::SelRcvPerson()
{
	m_nCntSelIDOnLine = 0;
	m_nCntSelIDOffLine = 0;
	m_UserIDSelectedOnLine ="";
	m_UserIDSelectedOffLine ="";
	
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
	CString ItemText, ItemTextTmp;
		
	while (hCurrent != NULL)
	{
		if (m_TreeAllMsg.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);

			if(m_TreeAllMsg.GetCheck(hChildItem))
			{				
				S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);				

				/* TEST !! */

				if(pData->m_nConStatus == 1)
				{
					//OnLine
					ItemText.Format("%s%c", pData->m_szID,DELIM1);
					m_UserIDSelectedOnLine+=ItemText;				
					m_nCntSelIDOnLine++;
				}
				else
				{
					//OffLine
					ItemText.Format("%s%c", pData->m_szID,DELIM1);
					m_UserIDSelectedOffLine+=ItemText;				
					m_nCntSelIDOffLine++;
				}
			}
			
			while (hChildItem != NULL)
			{
				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				if(hChildItem)
				{
					if(m_TreeAllMsg.GetCheck(hChildItem))
					{						
						S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
						
						
						if(pData->m_nConStatus == 1)
						{
							//OnLine
							ItemText.Format("%s%c", pData->m_szID,DELIM1);
							m_UserIDSelectedOnLine+=ItemText;				
							m_nCntSelIDOnLine++;
						}
						else
						{
							//OffLine
							ItemText.Format("%s%c", pData->m_szID,DELIM1);
							m_UserIDSelectedOffLine+=ItemText;				
							m_nCntSelIDOffLine++;
						}
					}
				}				
			}		
		}

		hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
	}	

	return TRUE;
}


BOOL CAllMsgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_bCheckAll     = FALSE;
	m_bExpandAll = FALSE;
	m_bTreeMade = FALSE;

	m_chkAll.ShowWindow(SW_HIDE);	
	m_btnExpandAll.ShowWindow(SW_HIDE);
	m_btnUpdate.ShowWindow(SW_HIDE);

	m_imageSmall.Create(16,16,ILC_COLOR4,1,1);	
	m_imageSmall.Add(AfxGetApp()->LoadIcon(IDI_ICON_SENDALLMSG_DEFAULT));
	m_imageSmall.Add(AfxGetApp()->LoadIcon(IDI_ICON_SENDALLMSG_OK));
	m_imageSmall.Add(AfxGetApp()->LoadIcon(IDI_ICON_SENDALLMSG_FAIL));	
	m_CListCtrlAllMsgStatus.SetImageList(&m_imageSmall, LVSIL_SMALL);
	
	m_CListCtrlAllMsgStatus.SetExtendedStyle(m_CListCtrlAllMsgStatus.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ); //| LVS_EX_CHECKBOXES

	int nSize = m_StringAryID.GetSize();	
	
	if(nSize > 0)
	{
		m_RadioAllMsgTo = 1; // 선택 사용자
		m_CListCtrlAllMsgStatus.ShowWindow(SW_SHOW);
		m_TreeAllMsg.ShowWindow(SW_HIDE);
		
		m_btnResend.ShowWindow(SW_SHOW);
		m_chkAll.ShowWindow(SW_HIDE);
		m_btnExpandAll.ShowWindow(SW_HIDE);
		m_btnUpdate.ShowWindow(SW_HIDE);		
	}
	else
	{
		RECT rc;
		m_CListCtrlAllMsgStatus.GetWindowRect(&rc);
        ScreenToClient(&rc);
		m_TreeAllMsg.MoveWindow(&rc);

		m_RadioAllMsgTo = 0; // 팀
		m_CListCtrlAllMsgStatus.ShowWindow(SW_HIDE);
		m_TreeAllMsg.ShowWindow(SW_SHOW);		

		m_btnResend.ShowWindow(SW_HIDE);
		m_chkAll.ShowWindow(SW_SHOW);
		m_btnExpandAll.ShowWindow(SW_SHOW);
		m_btnUpdate.ShowWindow(SW_SHOW);
				
		RequestAllUserInfo();

	}

	//LVCOLUMN col;
	//col.mask = LVCF_WIDTH | LVCF_TEXT;
	
	m_CListCtrlAllMsgStatus.InsertColumn(0,"ID");
	m_CListCtrlAllMsgStatus.InsertColumn(1,"상태");
	m_CListCtrlAllMsgStatus.InsertColumn(2,"이름");	
	
	m_CListCtrlAllMsgStatus.SetColumnWidth(0, 100);
	m_CListCtrlAllMsgStatus.SetColumnWidth(1, 60);
	m_CListCtrlAllMsgStatus.SetColumnWidth(2, 120);

	for(int i=0; i < nSize; i++)
	{
		CString strID = m_StringAryID.ElementAt(i);	
		CString strName = m_StringAryName.ElementAt(i);

		CString strItem;		
		LVITEM lvi;		
		lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
		int tempCnt = m_CListCtrlAllMsgStatus.GetItemCount();
		lvi.iItem = tempCnt;		
				
		//ID
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strID);
		lvi.iImage = 0; // default 
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
		//
		lvi.iSubItem = 1;
		lvi.pszText = "OnLine";
		lvi.iImage = -1; 
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
		//Name
		lvi.iSubItem = 2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strName);		
		lvi.iImage = -1;
		m_CListCtrlAllMsgStatus.InsertItem(&lvi);		
		m_CListCtrlAllMsgStatus.SetItem(&lvi);		
	}	
  
	m_CEditAllMsg.SetLimitText(1024000);
	
	SetTimer(0, 100, NULL);
	
	SetResize();	 
	
	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	UpdateData(FALSE);
	
	//Tree 
	//m_TreeAllMsg.ModifyStyle( 0, TVS_TRACKGROUPSELECT | TVS_SINGLECLICKEXPAND | TVS_CHECKBOXES | TVS_HASLINES  );
	m_TreeAllMsg.ModifyStyle( 0, WS_CHILD | WS_VISIBLE |WS_BORDER |	WS_TABSTOP | TVS_HASBUTTONS |TVS_HASLINES |TVS_LINESATROOT |	TVS_CHECKBOXES  );
	
	// Font 설정
	CFont font;	
	//font.CreatePointFont( 100, "굴림" );
	font.CreatePointFont( 100, "Tahoma" );
	
	m_TreeAllMsg.SetFont( &font );
	//m_TreeAllMsg.SetFocus();
	m_ColorBG  = RGB(208, 207, 235);
	m_TreeAllMsg.SetBkColor( m_ColorBG );		
	
	m_TreeAllMsg.SetBitmaps(IDB_BITMAP_ALLMSG_TREE,IDB_BITMAP_ALLMSG_STATUS);

	if(nSize > 0)
	{		
		m_TreeAllMsg.ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAllMsgDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 0)
	{		
		m_CEditAllMsg.SetFocus();
		KillTimer(0);
	}

	CDialog::OnTimer(nIDEvent);
}

LRESULT CAllMsgDlg::OnAllMsgAck(WPARAM wParam, LPARAM lParam)
{		
	char* szFromID = (char*)(lParam);

	if(m_RadioAllMsgTo == 1)
	{
		// 선택 사용자
		// 수신했음으로 변경	
		LVFINDINFO info;
		int nIndex = -1;

		info.flags = LVFI_PARAM;	

		info.flags = LVFI_PARTIAL|LVFI_STRING;
		info.psz = szFromID;		
		nIndex = m_CListCtrlAllMsgStatus.FindItem(&info);
		
		if(nIndex != -1)
		{
			LVITEM rItem;
			//CString strItem;
			
			TRACE("화면에서 찾아서 갱신\n");
						
			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 0;			
			rItem.pszText = szFromID;
			rItem.iImage = 1;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);

			ZeroMemory ( &rItem, sizeof(LVITEM) );
			rItem.iItem = nIndex;
			rItem.mask =  LVIF_IMAGE | LVIF_TEXT;			
			rItem.iSubItem = 1;	
			rItem.pszText = "수신";		
			rItem.iImage = -1;		
			m_CListCtrlAllMsgStatus.SetItem(&rItem);
			
		}
		
		FlashWindow(TRUE);
		/*
		FLASHWINFO fwi;
		fwi.cbSize  = sizeof(FLASHWINFO);
		fwi.hwnd =  this->GetSafeHwnd();
		fwi.dwFlags = FLASHW_ALL ;
		fwi.uCount  = 3; 
		fwi.dwTimeout = 0; 
		FlashWindowEx(&fwi);		
		*/
	}
	else
	{
		// 팀별 선택 => 트리에서 수신했음으로 변경		
		HTREEITEM hChildItem ;
		HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		CString ItemText, ItemTextTmp;
			
		while (hCurrent != NULL)
		{
			if (m_TreeAllMsg.ItemHasChildren(hCurrent))
			{
				hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
				
				if( strcmp( szFromID, ((S_ALLID_TREEINFO*) (m_TreeAllMsg.GetItemData(hChildItem)))->m_szID ) == 0 )
				{	
					//ItemTextTmp = m_TreeAllMsg.GetItemText(hChildItem); 
					//ItemText.Format("%s 전달 성공", ItemTextTmp);
					//m_TreeAllMsg.SetItemText(hChildItem, ItemText);

					m_TreeAllMsg.SetItemImage(hChildItem,2, 2);
				}
				
				
				while (hChildItem != NULL)
				{
					hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				
					if(hChildItem)
					{
						if( strcmp( szFromID, ((S_ALLID_TREEINFO*) (m_TreeAllMsg.GetItemData(hChildItem)))->m_szID ) == 0 )
						{
							//ItemTextTmp = m_TreeAllMsg.GetItemText(hChildItem); //name 
							//ItemText.Format("%s 전달 성공", ItemTextTmp);
							//m_TreeAllMsg.SetItemText(hChildItem, ItemText);

							m_TreeAllMsg.SetItemImage(hChildItem,2, 2);
						}
					}				
				}		
			}

			hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
		}		
	}
	
	
	delete szFromID;

	return TRUE;
}

void CAllMsgDlg::OnSetfocusEditAllmsg() 
{
	// TODO: Add your control notification handler code here
	m_CEditAllMsg.SetHan(TRUE);
}


void CAllMsgDlg::SetResize() 
{	
	m_cControlPos.SetParent(this);
	
	m_cControlPos.AddControl(IDC_LIST_ALLMSG_STATUS	, CP_RESIZE_VERTICAL );		
	m_cControlPos.AddControl(IDC_TREE_ALL_MSG	, CP_RESIZE_VERTICAL );		
	m_cControlPos.AddControl(IDC_EDIT_ALLMSG	, CP_RESIZE_HORIZONTAL | CP_RESIZE_VERTICAL	 );	
	m_cControlPos.AddControl(IDC_BTN_ALLMSG_OK	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );		
	m_cControlPos.AddControl(IDC_BTN_SENDALLMSG	, CP_MOVE_HORIZONTAL	| CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_CHECK_RESEND	, CP_MOVE_VERTICAL );	

	m_cControlPos.AddControl(IDC_CHK_SELECT_ALL	, CP_MOVE_VERTICAL );	
	m_cControlPos.AddControl(IDC_BUTTON_EXPANDALL	, CP_MOVE_VERTICAL );		
	m_cControlPos.AddControl(IDC_BUTTON_UPDATE	, CP_MOVE_VERTICAL );	
	
}

void CAllMsgDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cControlPos.MoveControls();	
}

void CAllMsgDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	ClearAllTreeData() ;

	CWinApp *pApp = AfxGetApp();
	CWnd *pMainWnd = pApp->GetMainWnd();
	pMainWnd->PostMessage(WM_ALLMSG_CLOSE, (WPARAM)0, (LPARAM)0);	

	CDialog::OnClose();
}


BOOL CAllMsgDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CAllMsgDlg::OnChkSelectAll() 
{
	// TODO: Add your control notification handler code here
	
	m_bCheckAll = !m_bCheckAll;

	if(m_bCheckAll)		m_chkAll.SetWindowText("전체해제");
	else				m_chkAll.SetWindowText("전체선택");

	
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		
	while (hCurrent != NULL)
	{
		m_TreeAllMsg.SetCheck(hCurrent, m_bCheckAll);
		
		if (m_TreeAllMsg.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
			while (hChildItem != NULL)
			{
				m_TreeAllMsg.SetCheck(hChildItem, m_bCheckAll);
				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);							
			}		
		}
		hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
	}	

	//m_TreeAllMsg.Expand(hChildItem, TVE_EXPAND);
	//m_TreeAllMsg.Expand(hChildItem, TVE_COLLAPSE);   
}

void CAllMsgDlg::OnRadioSelid() 
{
	// TODO: Add your control notification handler code here
	m_CListCtrlAllMsgStatus.ShowWindow(SW_SHOW);
	m_TreeAllMsg.ShowWindow(SW_HIDE);

	m_btnResend.ShowWindow(SW_SHOW);
	m_chkAll.ShowWindow(SW_HIDE);
	m_btnExpandAll.ShowWindow(SW_HIDE);
	m_btnUpdate.ShowWindow(SW_HIDE);	
}

void CAllMsgDlg::OnRadioSelteam() 
{
	// TODO: Add your control notification handler code here
	RECT rc;
	m_CListCtrlAllMsgStatus.GetWindowRect(&rc);
    ScreenToClient(&rc);
	m_TreeAllMsg.MoveWindow(&rc);

	m_CListCtrlAllMsgStatus.ShowWindow(SW_HIDE);
	m_TreeAllMsg.ShowWindow(SW_SHOW);

	m_btnResend.ShowWindow(SW_HIDE);
	m_chkAll.ShowWindow(SW_SHOW);
	m_btnExpandAll.ShowWindow(SW_SHOW);
	m_btnUpdate.ShowWindow(SW_SHOW);
	
	if(m_bTreeMade == FALSE )
	{
		RequestAllUserInfo();
	}	
}

BOOL CAllMsgDlg ::RequestAllUserInfo()
{
	// 서버로 전체 목록을 요청한다.	
	char szTemp[20];
	char* pSzBuf = NULL;	
	int nTotalLen = 0;		
	S_COMMON_HEADER sComHeader;			
	S_FROMTOID_HEADER sFromIDHeader;
	memset(&sComHeader, NULL, sizeof(sComHeader));		
	memset(&sFromIDHeader, NULL, sizeof(sFromIDHeader));	
	memset(szTemp, NULL, sizeof(szTemp));		
		
	// 헤더부 설정
	int iLength = FROMTOID_HEADER_LENGTH  ; 
	
	memcpy(sComHeader.m_szUsage, ALL_USER_ID_TEAM, sizeof(sComHeader.m_szUsage));		
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szLength)-1, iLength );	 
	memcpy(sComHeader.m_szLength, szTemp, sizeof(sComHeader.m_szLength));	
	sprintf(szTemp, "%0*d", sizeof(sComHeader.m_szCount)-1, 1);
	memcpy(sComHeader.m_szCount, szTemp, sizeof(sComHeader.m_szCount));	
	
	memcpy(sFromIDHeader.m_szIP, CChatSession::Instance().m_szMyIPAddr, sizeof(sFromIDHeader.m_szIP) ); 
	memcpy(sFromIDHeader.m_szFromID, CChatSession::Instance().m_szMyUserID, sizeof(sFromIDHeader.m_szFromID) ); 
	memcpy(sFromIDHeader.m_szToID, "", sizeof(sFromIDHeader.m_szToID) ); 
			
	pSzBuf = new char [COMMON_HEADER_LENGTH + iLength ]; 
	memset(pSzBuf, NULL, sizeof(pSzBuf));	
	memcpy(pSzBuf, &sComHeader, COMMON_HEADER_LENGTH);
	memcpy(pSzBuf+COMMON_HEADER_LENGTH, &sFromIDHeader, FROMTOID_HEADER_LENGTH );	
		
	nTotalLen = COMMON_HEADER_LENGTH + iLength ;
	
	TRACE(" pSzBuf [%s]\n", (char*) pSzBuf );
	
	if(CChatSession::Instance().Send(pSzBuf, nTotalLen,"RequestAllUserInfo") < 0)	
	{
		TRACEX("Send return FAIL!\n");		
		return  FALSE;
	}

	return  TRUE;
}

void CAllMsgDlg::OnButtonUpdate() 
{
	// TODO: Add your control notification handler code here
	RequestAllUserInfo();
}

void CAllMsgDlg::OnClickTreeAllMsg(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CString strSelItem;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CPoint pt;
	CRect rt;
	GetCursorPos(&pt);
	m_TreeAllMsg.ScreenToClient(&pt);
	UINT flag;
	HTREEITEM hSel = m_TreeAllMsg.HitTest(pt, &flag);
	
	if ((hSel != NULL) && (/*TVHT_ONITEM*/TVHT_ONITEMSTATEICON  & flag))
	{		
		strSelItem = m_TreeAllMsg.GetItemText(hSel);				

		if (m_TreeAllMsg.ItemHasChildren(hSel))
		{			
			HTREEITEM hNextItem;
			HTREEITEM hChildItem = m_TreeAllMsg.GetChildItem(hSel);

			m_TreeAllMsg.SetCheck(hChildItem, !m_TreeAllMsg.GetCheck(hSel));
			
			m_TreeAllMsg.GetItemRect(hChildItem, &rt, false);
			m_TreeAllMsg.InvalidateRect(&rt, true);

			while (hChildItem != NULL)
			{
				hNextItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				
				m_TreeAllMsg.SetCheck(hNextItem, !m_TreeAllMsg.GetCheck(hSel));
				m_TreeAllMsg.GetItemRect(hNextItem, &rt, false);
				m_TreeAllMsg.InvalidateRect(&rt, true);

				hChildItem = hNextItem;
			}
			
			m_TreeAllMsg.UpdateData(FALSE);
		}
		

		// 선택한 아이템의 parent를 구해서 Check 상태를 체크한다.
		// 만약 Check 된 아이템이 없으면 Parent 아이템도 체크를 해제한다.
				
		BOOL bAllUnChk = TRUE;

		HTREEITEM hParentOfSelected = m_TreeAllMsg.GetParentItem(hSel);
		
		CString ItemText = m_TreeAllMsg.GetItemText(hSel);	
		ItemText = m_TreeAllMsg.GetItemText(hParentOfSelected);	
		
		if ( (hParentOfSelected != NULL) && m_TreeAllMsg.ItemHasChildren(hParentOfSelected))
		{
			HTREEITEM hChildItem = m_TreeAllMsg.GetChildItem(hParentOfSelected);
			
			while (hChildItem != NULL)
			{
				
				TVITEM item;
				TCHAR szText[1024];
				item.hItem = hChildItem;
				item.mask = TVIF_TEXT | TVIF_HANDLE;
				item.pszText = szText;
				item.cchTextMax = 1024;
				
				BOOL bWorked = m_TreeAllMsg.GetItem(&item);
				
				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				
				if (bWorked )
				{
					if(strSelItem == CString(item.pszText))
					{					
						if(!m_TreeAllMsg.GetCheck(item.hItem))
							bAllUnChk = FALSE;
					}
					else
					{
						if(m_TreeAllMsg.GetCheck(item.hItem))
							bAllUnChk = FALSE;
					}
				}				
			}
									
			if(bAllUnChk == TRUE)
			{
				//TRACE("체크된것이 하나도 없다. 부모를 UnCheck상태로..\n");
				m_TreeAllMsg.SetCheck(hParentOfSelected, FALSE);
				m_TreeAllMsg.GetItemRect(hParentOfSelected, &rt, false);
				m_TreeAllMsg.InvalidateRect(&rt, true);
			}
			else
			{
				//TRACE("체크된것이 존재한다. 부모를 Check상태로..\n");
				m_TreeAllMsg.SetCheck(hParentOfSelected, TRUE);
				m_TreeAllMsg.GetItemRect(hParentOfSelected, &rt, false);
				m_TreeAllMsg.InvalidateRect(&rt, true);
			}
		}		
	}
	*pResult = 0;
}


void CAllMsgDlg::ClearAllTreeData() 
{
	// TODO: Add your specialized code here and/or call the base class
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();	
		
	while (hCurrent != NULL)
	{
		if (m_TreeAllMsg.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
			
			//ItemTextTmp = m_TreeAllMsg.GetItemText(hChildItem);
			S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
			if(pData)
			{
				delete pData;
				pData = NULL;
			}			
			
			while (hChildItem != NULL)
			{
				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);
				if(hChildItem)
				{											
					S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeAllMsg.GetItemData(hChildItem);
					
					if(pData)
					{
						delete pData;
						pData = NULL;
					}	
				}				
			}		
		}

		hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
	}

	CDialog::PostNcDestroy();
}

void CAllMsgDlg::OnButtonExpandall() 
{
	// TODO: Add your control notification handler code here
	m_bExpandAll = !m_bExpandAll;

	if(m_bExpandAll)	m_btnExpandAll.SetWindowText("전체 접기");
	else				m_btnExpandAll.SetWindowText("전체 펼치기");

	
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeAllMsg.GetRootItem();
		
	while (hCurrent != NULL)
	{
		if(m_bExpandAll)
		{
			m_TreeAllMsg.Expand(hCurrent, TVE_EXPAND);
		}
		else
		{
			m_TreeAllMsg.Expand(hCurrent, TVE_COLLAPSE);
		}
		
		if (m_TreeAllMsg.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeAllMsg.GetChildItem(hCurrent);
			while (hChildItem != NULL)
			{				
				if(m_bExpandAll)
				{
					m_TreeAllMsg.Expand(hChildItem, TVE_EXPAND);
				}
				else
				{
					m_TreeAllMsg.Expand(hChildItem, TVE_COLLAPSE);
				}

				hChildItem = m_TreeAllMsg.GetNextItem(hChildItem, TVGN_NEXT);							
			}		
		}
		hCurrent = m_TreeAllMsg.GetNextItem(hCurrent, TVGN_NEXT);
	}
}
