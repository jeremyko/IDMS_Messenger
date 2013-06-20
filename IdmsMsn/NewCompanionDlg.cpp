// NewCompanionClg.cpp : implementation file
//

#include "stdafx.h"
#include "..\\common\\AllCommonDefines.h"
#include "IdmsMsn.h"
#include "NewCompanionDlg.h"
#include "ChatSession.h"
#include "MsgBoxThread.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewCompanionDlg dialog


CNewCompanionDlg::CNewCompanionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewCompanionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewCompanionDlg)
	m_strCompID = _T("");
	m_nRadioSel = -1;
	//}}AFX_DATA_INIT

	memset(m_szGrpName, 0x00, sizeof(m_szGrpName));
}

CNewCompanionDlg::CNewCompanionDlg(CWnd* pParent , const char* szSelGrpName)	
: CDialog(CNewCompanionDlg::IDD, pParent)
{
	memset(m_szGrpName, 0x00, sizeof(m_szGrpName));
	strncpy(m_szGrpName, szSelGrpName, sizeof(m_szGrpName));
}


void CNewCompanionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewCompanionDlg)
	DDX_Control(pDX, IDC_BUTTON_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDC_LIST_LOGIN_ID, m_ListLogInID);
	DDX_Control(pDX, ID_BN_SEARCH, m_btnSearch);	
	DDX_Control(pDX, ID_EB_USERNAME, m_EditUserName);
	DDX_Control(pDX, ID_EB_USERID, m_EditUserID);
	DDX_Control(pDX, IDC_TREE_USER_LISTALL, m_TreeListAll);
	DDX_Control(pDX, IDC_EDIT_COMP_ID, m_CEditAddID);
	DDX_Control(pDX, IDC_COMBO_GRP_SET, m_ComBoGrpName);
	DDX_Text(pDX, IDC_EDIT_COMP_ID, m_strCompID);
	DDX_Radio(pDX, IDC_RADIO_SEARCH, m_nRadioSel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewCompanionDlg, CDialog)
	//{{AFX_MSG_MAP(CNewCompanionDlg)	
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	ON_BN_CLICKED(IDC_BTN_ADD_CLOSE, OnBtnAddClose)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BN_SEARCH, OnBnSearch)
	ON_BN_CLICKED(IDC_RADIO_LISTALL, OnRadioListall)
	ON_BN_CLICKED(IDC_RADIO_SEARCH, OnRadioSearch)
	ON_BN_CLICKED(IDC_RADIO_SELID, OnRadioSelid)
	ON_NOTIFY(NM_CLICK, IDC_LIST_LOGIN_ID, OnClickListLoginId)
	ON_EN_SETFOCUS(ID_EB_USERNAME, OnSetfocusEbUsername)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnButtonUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SEARCH_ID  	,   OnSearchID   		)
	ON_MESSAGE(WM_ALL_USER_ID_TEAM_FORADD,    OnAllUserIdTeam)	
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewCompanionDlg message handlers

BOOL CNewCompanionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_ListLogInID.SetExtendedStyle(m_ListLogInID.GetExtendedStyle() |LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ); //| LVS_EX_CHECKBOXES		
	m_ListLogInID.InsertColumn(0,"사용자명");
	m_ListLogInID.InsertColumn(1,"사용자ID");
	m_ListLogInID.InsertColumn(2,"소속사");	
	m_ListLogInID.InsertColumn(3,"소속팀");	
	m_ListLogInID.InsertColumn(4,"업무팀");	
	m_ListLogInID.InsertColumn(5,"연락처1");	
	m_ListLogInID.InsertColumn(6,"연락처2");	
	m_ListLogInID.InsertColumn(7,"직급");	
	
	m_ListLogInID.SetColumnWidth(0, 70);
	m_ListLogInID.SetColumnWidth(1, 100);
	m_ListLogInID.SetColumnWidth(2, 70);
	m_ListLogInID.SetColumnWidth(3, 70);
	m_ListLogInID.SetColumnWidth(4, 70);
	m_ListLogInID.SetColumnWidth(5, 120);
	m_ListLogInID.SetColumnWidth(6, 140);
	m_ListLogInID.SetColumnWidth(7, 60);

	m_nRadioSel = 0; // 0 : 검색해서 추가 / 1: 사용자 아이디를 아는경우 / 2 : 전체 목록에서 

	// Font 설정
	CFont font;	
	//font.CreatePointFont( 100, "굴림" );
	font.CreatePointFont( 100, "Tahoma" );	
	m_TreeListAll.SetFont( &font );	
	m_TreeListAll.SetBkColor( RGB(208, 207, 235) );		

	RECT rc;
	m_ListLogInID.GetWindowRect(&rc);
    ScreenToClient(&rc);
	m_TreeListAll.MoveWindow(&rc);

	m_TreeListAll.ShowWindow(SW_HIDE);
	m_CEditAddID.EnableWindow(FALSE);
	m_btnUpdate.ShowWindow(SW_HIDE);

	m_EditUserID.SetFocus();
	m_bTreeMade = FALSE;	
	int nPos = -1;
	m_strID ="";
	SetTimer(0, 100, NULL);
	
	// TODO: Add extra initialization here
	m_ComBoGrpName.Clear();

	CStringArray* pStrAry = CChatSession::Instance().getGrpNames();
	
	for(int i=0; i < pStrAry->GetSize(); i++)
	{	
		if( strlen(m_szGrpName) > 0 && pStrAry->ElementAt(i) == CString(m_szGrpName) )
		{
			nPos = i;
		}

		m_ComBoGrpName.AddString( pStrAry->ElementAt(i) );
	}
	
	if(nPos != -1)
	{
		m_ComBoGrpName.SetCurSel(nPos);
		m_ComBoGrpName.EnableWindow(FALSE);
	}
	else
	{
		m_ComBoGrpName.SetCurSel(0);
		m_ComBoGrpName.EnableWindow(TRUE);
	}
	
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CNewCompanionDlg::OnBtnClose() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

// ID 검색 m_strCompID
/*
void CNewCompanionDlg::OnBtnUserFind() 
{
	// TODO: Add your control notification handler code here
	CLoginIDSearch dlg;
	if(dlg.DoModal()==IDOK)
	{
		m_strCompID = dlg.m_strID;
		UpdateData(FALSE);
	}	
}
*/

int	CNewCompanionDlg::SetTree()
{
	// 그룹을 등록한다.	
	HTREEITEM hSub;
	HTREEITEM hCutItem;
	BOOL      bFirst = TRUE;

	CRect rt;

	m_TreeListAll.DeleteAllItems();
	
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
						hSub = m_TreeListAll.InsertItem( "TEMP", 0, 1);
						bFirst = FALSE;			
					}		
					
					// 자신은 목록에 안나오게 처리
					//////////////////////////////////////////////////
					if(strncmp(CChatSession::Instance().m_szMyUserID, szTmpUserID, sizeof(CChatSession::Instance().m_szMyUserID)) == 0 )
					{
						break;
					}
					//////////////////////////////////////////////////
					HTREEITEM hCurrent = m_TreeListAll.GetRootItem();
					
					BOOL bFound = FALSE;
					
					//비교				
					while (hCurrent != NULL)
					{
						hCurrent = m_TreeListAll.GetNextItem(hCurrent, TVGN_NEXT);
						// 비교
						if(hCurrent)
						{
							CString ItemText = m_TreeListAll.GetItemText(hCurrent);
							
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
									hCutItem=m_TreeListAll.InsertItem(szIDName, 0, 0, hCurrent);
								}
								else
								{
									// OffLine
									pData->m_nConStatus = 0;
									hCutItem=m_TreeListAll.InsertItem(szIDName, 1, 1, hCurrent);
								}
								
								strncpy(pData->m_szID, szTmpUserID, sizeof(pData->m_szID) );
								m_TreeListAll.SetItemData(hCutItem, (DWORD)pData);
								bFound = TRUE;								
							}
						}			
					}
					
					if(!bFound)
					{
						hSub     = m_TreeListAll.InsertItem( szTmpTeamName, 0, 0);

						char szIDName [100];
						memset(szIDName, 0x00, sizeof(szIDName) );
						sprintf(szIDName, "%s [%s]", szTmpName, szTmpUserID )	;							

						S_ALLID_TREEINFO* pData = new S_ALLID_TREEINFO ;								
						memset(pData, 0x00, sizeof(pData));

						if(strcmp("Y", szTmpOut) == 0)
						{
							// OnLine
							pData->m_nConStatus = 1;
							hCutItem = m_TreeListAll.InsertItem(szIDName, 0, 0, hSub);
						}
						else
						{
							// OffLine
							pData->m_nConStatus = 0;
							hCutItem = m_TreeListAll.InsertItem(szIDName, 1, 1, hSub);
						}					

						strncpy(pData->m_szID, szTmpUserID, sizeof(pData->m_szID) );
						m_TreeListAll.SetItemData(hCutItem, (DWORD)pData);
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
		

	HTREEITEM hCurrent = m_TreeListAll.GetRootItem();
	
	if(hCurrent)
	{
		CString ItemText = m_TreeListAll.GetItemText(hCurrent);
		
		if ( ItemText == "TEMP") // for safety..
		{
			m_TreeListAll.DeleteItem(hCurrent);
		}
	}
	
	m_bTreeMade = TRUE;

	return TRUE;
}	

void CNewCompanionDlg::ClearAllTreeData() 
{
	// TODO: Add your specialized code here and/or call the base class
	HTREEITEM hChildItem ;
	HTREEITEM hCurrent = m_TreeListAll.GetRootItem();	
		
	while (hCurrent != NULL)
	{
		if (m_TreeListAll.ItemHasChildren(hCurrent))
		{
			hChildItem = m_TreeListAll.GetChildItem(hCurrent);
			
			//ItemTextTmp = m_TreeAllMsg.GetItemText(hChildItem);
			S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeListAll.GetItemData(hChildItem);
			if(pData)
			{
				delete pData;
				pData = NULL;
			}			
			
			while (hChildItem != NULL)
			{
				hChildItem = m_TreeListAll.GetNextItem(hChildItem, TVGN_NEXT);
				if(hChildItem)
				{											
					S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeListAll.GetItemData(hChildItem);
					
					if(pData)
					{
						delete pData;
						pData = NULL;
					}	
				}				
			}		
		}

		hCurrent = m_TreeListAll.GetNextItem(hCurrent, TVGN_NEXT);
	}

	CDialog::PostNcDestroy();
}


LRESULT CNewCompanionDlg::OnAllUserIdTeam(WPARAM wParam, LPARAM lParam) 
{
	char* pSzRawData = (char*) lParam ;
	
	m_strAllUserInfo = pSzRawData ;

	delete [] pSzRawData;

	SetTree();

	return TRUE;
}

void CNewCompanionDlg::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	ClearAllTreeData() ;
	CDialog::OnCancel();
}

void CNewCompanionDlg::OnBtnAddClose() 
{
	// TODO: Add your control notification handler code here
	//UpdateData(FALSE);
	UpdateData(TRUE);

	if(m_nRadioSel == 0) 
	{		
		m_strCompID = m_strID;		
	}	
	else if(m_nRadioSel == 2) 
	{
		//전체 목록 
		HTREEITEM itemSel = m_TreeListAll.GetSelectedItem();		
		S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeListAll.GetItemData(itemSel);

		if(pData)
		{
			m_strCompID = pData->m_szID;
		}
		else
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"사용자를 선택하세요.");
			pMsgThread->CreateThread();
			return ;
		}		
	}

	// ID 비교
	if(m_strExID != m_strCompID)
	{		
		if(m_strCompID.GetLength() > 0)
		{			
			char szID[20+1]; 
			char szGrpName[C_GRP_NAME_LEN];
			memset(szID, 0x00, sizeof(szID));
			memset(szGrpName, 0x00, sizeof(szGrpName));

			strncpy(szID, (LPCSTR)m_strCompID, sizeof(szID));
			CString strTmpGrp;
			
			
			int nPos = m_ComBoGrpName.GetCurSel();
			if(nPos == -1)
			{
				
				CString strFmt ;
				strFmt = "그룹을 선택하세요.";				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,strFmt);
				
				pMsgThread->CreateThread();

				return ;
				
			}
			m_ComBoGrpName.GetLBText(m_ComBoGrpName.GetCurSel(),strTmpGrp ) ;
			strncpy(szGrpName, (LPCSTR)strTmpGrp, sizeof(szGrpName));
			
			// 자기 자신은 추가불가
			if(strcmp ( CChatSession::Instance().m_szMyUserID, (LPCSTR)m_strCompID)==0)
			{
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"자신을 대화상대로 등록할수 없습니다.");					
				pMsgThread->CreateThread();
				return;
			}

			// Client에서 먼저 중복 체크
			CBuddyInfo * pCompanion;
			if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_strCompID, ( CObject*& ) pCompanion ) ) 
			{	
				// 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함)
				if(pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2)
				{				
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"이미 대화상대에 존재합니다");
					
					pMsgThread->CreateThread();

					return;
				}
				if(pCompanion->m_nYouBlocked == 3)
				{
					// 상대방에게 대화상대 다시 추가되었음을 알림.
					if(CChatSession::Instance().AckYouAllowedAsCompanion((LPCSTR)m_strCompID, szGrpName) != TRUE)
					{
						return ;
					}

					CDialog::OnOK();
					return ;
				}
				else if(pCompanion->m_nYouBlocked == 4 || pCompanion->m_nYouBlocked == 5)
				{				
					CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"추가할 수 없습니다. 상대방이 당신을 대화상대에서 삭제한 상태입니다. 상대방이 당신을 대화상대로 다시추가해야 가능합니다.");	
					
					pMsgThread->CreateThread();

					return;
				}
			}	

			// 서버로 전송 => 중복 체크 해야함 (서버에서?)			
			TRACEX("대화 상대 추가 요청!\n");			
			if(CChatSession::Instance().RequestAddCompanion(m_strCompID, strTmpGrp) != TRUE)
			{
				return ;
			}
			

			//::SendMessage(m_pHwnd, WM_ADD_COMP_LIST, (WPARAM)szIP, (LPARAM)szGrpName);

			m_strExID = m_strCompID;
		}	
	}

	ClearAllTreeData() ;
	CDialog::OnOK();
	
}

void CNewCompanionDlg::OnBtnAdd() 
{
	// TODO: Add your control notification handler code here
	//UpdateData(FALSE);
	UpdateData(TRUE);

	// 0 : 검색해서 추가 / 1: 사용자 아이디를 아는경우 / 2 : 전체 목록에서 	
	if(m_nRadioSel == 0) 
	{		
		m_strCompID = m_strID;		
	}	
	else if(m_nRadioSel == 2) 
	{
		//전체 목록 
		HTREEITEM itemSel = m_TreeListAll.GetSelectedItem();		
		S_ALLID_TREEINFO* pData = (S_ALLID_TREEINFO*) m_TreeListAll.GetItemData(itemSel);
		if(pData)
		{
			m_strCompID = pData->m_szID;
		}
		else
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"사용자를 선택하세요.");
			pMsgThread->CreateThread();
			return ;
		}
	}

	if(m_strCompID.GetLength() > 0)
	{
		char szID[20+1]; 
		char szGrpName[C_GRP_NAME_LEN];

		memset(szID, 0x00, sizeof(szID));
		memset(szGrpName, 0x00, sizeof(szGrpName));

		strncpy(szID, (LPCSTR)m_strCompID, sizeof(szID));
		CString strTmpGrp;
		int nPos = m_ComBoGrpName.GetCurSel();
		if(nPos == -1)
		{			
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"그룹을 선택하세요.");
			
			pMsgThread->CreateThread();

			return ;
		}

		m_ComBoGrpName.GetLBText(m_ComBoGrpName.GetCurSel(),strTmpGrp ) ;
		strncpy(szGrpName, (LPCSTR)strTmpGrp, sizeof(szGrpName));
		
		// 자기 자신은 추가불가
		if(strcmp ( CChatSession::Instance().m_szMyUserID, (LPCSTR)m_strCompID)==0)
		{
			CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"자신을 대화상대로 등록할수 없습니다.");					
			pMsgThread->CreateThread();
			return;
		}

		// Client에서 먼저 중복 체크
		CBuddyInfo * pCompanion;
		if( CChatSession::Instance().mBuddyInfoMap.Lookup(m_strCompID, ( CObject*& ) pCompanion ) ) 
		{	
			// 1 (내가추가) 2(상대방이 나를 추가) 3(내가 삭제함) 4(상대방이 차단함) 5(상대방이 나를 삭제하고 나도 상대방을 삭제함)
			if(pCompanion->m_nYouBlocked == 1 || pCompanion->m_nYouBlocked == 2)
			{				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"이미 대화상대에 존재합니다");
				
				pMsgThread->CreateThread();

				return;
			}
			if(pCompanion->m_nYouBlocked == 3)
			{
				// 상대방에게 대화상대 다시 추가되었음을 알림.
				if(CChatSession::Instance().AckYouAllowedAsCompanion((LPCSTR)m_strCompID, szGrpName) != TRUE)
				{
					return ;
				}
				m_strExID = m_strCompID;
				
				return;
			}
			else if(pCompanion->m_nYouBlocked == 4 || pCompanion->m_nYouBlocked == 5)
			{				
				CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"추가할 수 없습니다. 상대방이 당신을 대화상대에서 삭제한 상태입니다. 상대방이 당신을 대화상대로 다시추가해야 가능합니다.");	
				
				pMsgThread->CreateThread();

				return;
			}
		}	

		// 서버로 전송
		TRACEX("대화 상대 추가 요청!\n");		
		if(CChatSession::Instance().RequestAddCompanion(m_strCompID, strTmpGrp) != TRUE)
		{
			return ;
		}	
		
		m_strExID = m_strCompID;
	}
	else
	{				
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"대화상대 정보를 입력하세요.");
		
		pMsgThread->CreateThread();

		m_CEditAddID.SetFocus();
	}
}

void CNewCompanionDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 0)
	{
		//m_CEditAddID.SetFocus();
		m_EditUserID.SetFocus();
	}
	
	KillTimer(0);
	
	CDialog::OnTimer(nIDEvent);
}

BOOL CNewCompanionDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	/*
	if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}
	*/

	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{			
		// 0 : 검색해서 추가 / 1: 사용자 아이디를 아는경우 / 2 : 전체 목록에서 	
		UpdateData();
		if(m_nRadioSel == 0) 
		{
			RefreshList();
		}
		/*
		else if(m_nRadioSel == 1) 
		{			
			OnBtnAddClose();
		}
		*/
		
		return TRUE;	
	}
	
	if( pMsg->wParam == VK_ESCAPE)
	{		
		return TRUE;		
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CNewCompanionDlg::RefreshList()
{	
	UpdateData(TRUE);

	TRACEX("RefreshList()\n");

	m_ListLogInID.DeleteAllItems();
	
	CString strUserId, strUserName;
	GetDlgItemText(ID_EB_USERID  , strUserId  );
	GetDlgItemText(ID_EB_USERNAME, strUserName);
			
	if(strUserId.GetLength() == 0 && strUserName.GetLength() == 0)
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"조회 정보를 입력하세요");							
		pMsgThread->CreateThread();	
		
		return;
	}

	if(CChatSession::Instance().RequestLogInID( (LPCSTR)strUserId, (LPCSTR)strUserName ) != TRUE)
	{
		return  ;
	}

	//m_staMsg= "조회중입니다";
	//UpdateData(FALSE);
}


void CNewCompanionDlg::OnBnSearch() 
{
	// TODO: Add your control notification handler code here
	RefreshList();
}

void CNewCompanionDlg::OnRadioListall() 
{
	// TODO: Add your control notification handler code here	
	//m_nRadioSel = 0; // 0 : 검색해서 추가 / 1: 사용자 아이디를 아는경우 / 2 : 전체 목록에서 	
	
	SetDlgItemText(ID_EB_USERID  , "");
	SetDlgItemText(ID_EB_USERNAME, "");

	m_btnUpdate.ShowWindow(SW_SHOW);
	m_ListLogInID.ShowWindow(SW_HIDE);
	m_EditUserID.EnableWindow(FALSE);
	m_EditUserName.EnableWindow(FALSE);
	m_btnSearch.EnableWindow(FALSE);

	m_TreeListAll.ShowWindow(SW_SHOW);
	m_CEditAddID.EnableWindow(FALSE);

	if(m_bTreeMade == FALSE)
		RequestAllUserInfo();

}

BOOL CNewCompanionDlg ::RequestAllUserInfo()
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
	
	memcpy(sComHeader.m_szUsage, ALL_USER_ID_TEAM_FORADD, sizeof(sComHeader.m_szUsage));		
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

void CNewCompanionDlg::OnRadioSearch() 
{	
	m_strID = "";
	SetDlgItemText(ID_EB_USERID  , "");
	SetDlgItemText(ID_EB_USERNAME, "");

	// TODO: Add your control notification handler code here
	m_ListLogInID.ShowWindow(SW_SHOW);
	m_btnUpdate.ShowWindow(SW_HIDE);
	m_btnSearch.EnableWindow(TRUE);
	m_ListLogInID.EnableWindow(TRUE);
	m_EditUserID.EnableWindow(TRUE);
	m_EditUserName.EnableWindow(TRUE);

	m_TreeListAll.ShowWindow(SW_HIDE);
	m_CEditAddID.EnableWindow(FALSE);

	m_EditUserID.SetFocus();

	
}

void CNewCompanionDlg::OnRadioSelid() 
{
	m_strID = "";
	SetDlgItemText(ID_EB_USERID  , "");
	SetDlgItemText(ID_EB_USERNAME, "");
	
	// TODO: Add your control notification handler code here
	m_TreeListAll.ShowWindow(SW_HIDE);
	m_btnUpdate.ShowWindow(SW_HIDE);
	m_CEditAddID.EnableWindow(TRUE);

	m_EditUserID.EnableWindow(FALSE);
	m_EditUserName.EnableWindow(FALSE);
	m_ListLogInID.EnableWindow(FALSE);
	m_btnSearch.EnableWindow(FALSE);

	m_CEditAddID.SetFocus();

	
}


void CNewCompanionDlg::OnClickListLoginId(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	NMITEMACTIVATE* pHdr  = (NMITEMACTIVATE*) pNMHDR ;
	
	m_strID = m_ListLogInID.GetItemText(pHdr->iItem, 1) ;

	*pResult = 0;
}

LRESULT CNewCompanionDlg::OnSearchID(WPARAM wParam, LPARAM lParam)
{
	//m_staMsg= "";
	//UpdateData(FALSE);
	
	m_ListLogInID.DeleteAllItems();

	CPtrArray* pArray  = (CPtrArray*)lParam;

	if( pArray->GetSize() == 0 )
	{
		CMsgBoxThread* pMsgThread = new CMsgBoxThread(this,"조회 결과가 없습니다.");							
		pMsgThread->CreateThread();	
	}

	for(int i = 0; i < pArray->GetSize(); i++)
	{		
		S_SEARCHID_CLI* pData = static_cast<S_SEARCHID_CLI*>(pArray->GetAt(i));		
		

		CString strItem;		
		LVITEM lvi;		
		lvi.mask =  LVFI_PARAM | LVIF_IMAGE | LVIF_TEXT;				
		int tempCnt = m_ListLogInID.GetItemCount();
		lvi.iItem = tempCnt;		

		//1
		lvi.iSubItem = 0;
		lvi.pszText = pData->szUserName ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//2
		lvi.iSubItem = 1;
		lvi.pszText = pData->szUserId;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);				
		//3
		lvi.iSubItem = 2;
		lvi.pszText = pData->szCompany ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);				
		//4
		lvi.iSubItem = 3;
		lvi.pszText = pData->szTopTeamName;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//5
		lvi.iSubItem = 4;
		lvi.pszText = pData->szTeamName ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//6
		lvi.iSubItem = 5;
		lvi.pszText = pData->szTel1;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//7
		lvi.iSubItem = 6;
		lvi.pszText = pData->szTel2 ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);
		//8
		lvi.iSubItem = 7;
		lvi.pszText = pData->szJobClassName;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//9
		lvi.iSubItem = 8;
		lvi.pszText = pData->szCompanyuCode ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);
		//10
		lvi.iSubItem = 9;
		lvi.pszText = pData->szTopTeamCode;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//11
		lvi.iSubItem = 10;
		lvi.pszText = pData->szTeamCode ;
		lvi.iImage = 0; // default 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
		//12
		lvi.iSubItem = 11;
		lvi.pszText = pData->szJobClassCode;
		lvi.iImage = -1; 
		m_ListLogInID.InsertItem(&lvi);
		m_ListLogInID.SetItem(&lvi);		
	}

	// Clear
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		S_SEARCHID_CLI *pData = static_cast<S_SEARCHID_CLI*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}	

	delete pArray;	
     
	return TRUE;
}

void CNewCompanionDlg::OnSetfocusEbUsername() 
{
	// TODO: Add your control notification handler code here
	m_EditUserName.SetHan(TRUE);
}


void CNewCompanionDlg::OnButtonUpdate() 
{
	// TODO: Add your control notification handler code here
	RequestAllUserInfo();
}
