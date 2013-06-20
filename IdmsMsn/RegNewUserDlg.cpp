// RegNewUserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "RegNewUserDlg.h"
#include "ChatSession.h"
#include "MsgBoxThread.h"
#include "DesWrapper.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 이것은 기존 IDMS 사용부분을 약간 변형부분이고, IDMS에만 해당되는 내용임.
/////////////////////////////////////////////////////////////////////////////
// CRegNewUserDlg dialog


CRegNewUserDlg::CRegNewUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegNewUserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegNewUserDlg)
	m_strRegUserNick = _T("");	
	m_strId = _T("");
	m_strJuminNo = _T("");
	m_strName = _T("");
	m_strPw1 = _T("");
	m_strPw2 = _T("");
	m_nDocYesNo = -1;
	m_strTelOfc1 = _T("");
	m_strTelOfc2 = _T("");
	m_strTelOfc3 = _T("");
	m_strTelIn = _T("");
	m_strTelHp1 = _T("");
	m_strTelHp2 = _T("");
	m_strTelHp3 = _T("");
	//}}AFX_DATA_INIT
	m_strStatus	= _T("");
	m_strIdValid= _T("");
	m_strIdOld	= _T("");

	m_nUseFlag = CHK_WORK ;
}


void CRegNewUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegNewUserDlg)
	DDX_Control(pDX, ID_CB_GRADE, m_cbGrade);
	DDX_Control(pDX, ID_CB_TEAM, m_cbTeam);
	DDX_Control(pDX, ID_CB_COMPANY, m_cbCompany);	
	DDX_Control(pDX, IDC_EDIT_USER_NICK_NAME, m_CEditRegUserNick);	
	DDX_Text(pDX, IDC_EDIT_USER_NICK_NAME, m_strRegUserNick);
	DDV_MaxChars(pDX, m_strRegUserNick, 255);
	DDX_Text(pDX, ID_EB_USER_ID, m_strId);
	DDX_Text(pDX, ID_EB_REG_SID, m_strJuminNo);
	DDV_MaxChars(pDX, m_strJuminNo, 13);
	DDX_Text(pDX, ID_EB_REG_NAME, m_strName);
	DDX_Text(pDX, ID_EB_PWD, m_strPw1);
	DDX_Text(pDX, ID_EB_CONFIRM_PWD, m_strPw2);
	DDX_Radio(pDX, ID_RB_DIARY_YES, m_nDocYesNo);
	DDX_Text(pDX, ID_EB_TELOFC1, m_strTelOfc1);
	DDX_Text(pDX, ID_EB_TELOFC2, m_strTelOfc2);
	DDX_Text(pDX, ID_EB_TELOFC3, m_strTelOfc3);
	DDX_Text(pDX, ID_EB_TEL_BRANCH, m_strTelIn);
	DDX_Text(pDX, ID_EB_TELHP1, m_strTelHp1);
	DDX_Text(pDX, ID_EB_TELHP2, m_strTelHp2);
	DDX_Text(pDX, ID_EB_TELHP3, m_strTelHp3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegNewUserDlg, CDialog)
	//{{AFX_MSG_MAP(CRegNewUserDlg)
	ON_BN_CLICKED(ID_BN_SAVE, OnBnSave)
	ON_BN_CLICKED(ID_BTN_REG_RESET, OnBtnRegReset)
	ON_BN_CLICKED(ID_BTN_DUPID_CHECK, OnBtnDupidCheck)
	ON_CBN_SELCHANGE(ID_CB_COMPANY, OnSelchangeCbCompany)
	//}}AFX_MSG_MAP
	
	ON_MESSAGE(WM_REG_NEW_USER,   OnRegNewUserRslt)
	ON_MESSAGE(WM_CHK_DUP_ID,   OnChkDupUserIDRslt)
	ON_MESSAGE(WM_REGUSER_INFO,	OnGetUserInfoRslt)
	ON_MESSAGE(WM_TEAMCOMBO_INFO,	OnTeamInfoRslt)	
	ON_MESSAGE(WM_COMPANYCOMBO_INFO,	OnCompanyInfoRslt)	
	ON_MESSAGE(WM_CLASSCOMBO_INFO,	OnClassInfoRslt)	

	//ON_MESSAGE(WM_REG_NEW_USER,   OnRegNewUserRslt)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegNewUserDlg message handlers

// 사용자 등록 결과 처리
LRESULT CRegNewUserDlg::OnRegNewUserRslt(WPARAM wParam, LPARAM lParam)
{
	S_COM_RESPONSE * pComRslt = (S_COM_RESPONSE*) lParam;

	CSP_Utility<S_COM_RESPONSE> spTest = pComRslt;

	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		GetDlgItem(ID_EB_REG_SID		)->EnableWindow(FALSE);
		GetDlgItem(ID_EB_REG_NAME		)->EnableWindow(FALSE);
		GetDlgItem(ID_EB_USER_ID		)->EnableWindow(FALSE);
		GetDlgItem(ID_BTN_DUPID_CHECK	)->EnableWindow(FALSE);
						
		// 최초작성자가 저장하면 종료를 OK로 한다.
		if( m_strStatus == _T("00") )
		{
			m_nEndFlag = IDOK;
		}
		
		m_strStatus = "01";
		m_strCompanyCode = m_cbCompany.GetCode();
		m_strCompanyName = m_cbCompany.GetName();
		m_strTeamCode = m_cbTeam.GetCode();
		m_strTeamName = m_cbTeam.GetName();
		
		m_bModify = TRUE;
		
		AfxMessageBox("저장을 완료하였습니다",	  MB_ICONINFORMATION);

		return TRUE;
	}
	else
	{
		CString strMsg;
		strMsg.Format("사용자 등록에 실패하였습니다 [%s]", pComRslt->szErrMsg );
		AfxMessageBox(strMsg,	  MB_ICONINFORMATION);
		return FALSE;
	}	
}

// 사용자 ID 중복 CHK 결과 처리
LRESULT CRegNewUserDlg::OnChkDupUserIDRslt(WPARAM wParam, LPARAM lParam)
{	
	CString strTmp;
	S_COM_RESPONSE * pComRslt = (S_COM_RESPONSE*) lParam;
	CSP_Utility<S_COM_RESPONSE> spTest = pComRslt;

	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		TRACE("사용자 ID 이미 존재\n");
		if(m_strStatus != "00")
		{
			strTmp = "입력하신 사용자ID\n[" + m_strId + "]는 이미 등록된 사용자ID입니다\n"
		 		     "다른ID로 등록해주십시오";
		}
		else
		{
			if(m_strIdOld == m_strId)
			{
				strTmp = "기존 개발관리 사용자는 반드시 사용자ID를 댜시 설정하십시요";
			}
			else
			{
				strTmp = "입력하신 사용자ID\n[" + m_strId + "]는 이미 등록된 사용자ID입니다\n"
		 		     "다른ID로 등록해주십시오";
			}
			
		}

		AfxMessageBox(strTmp, MB_ICONINFORMATION);
		m_strId = _T("");
		SetDlgItemText(ID_EB_USER_ID, m_strId);
		GetDlgItem(ID_EB_USER_ID)->SetFocus();
		
		return FALSE;
	}
	else
	{
		if	(strcmp(pComRslt->szRsltCd ,"N") == 0 && strcmp(NODATA_FOUND, pComRslt->szErrMsg) == 0)
		{
			TRACE("사용자 ID 사용가능 \n");
			m_strIdValid = m_strId;
			strTmp = "입력하신 사용자ID\n[" + m_strId + "]는 사용가능합니다";
			AfxMessageBox(strTmp, MB_ICONINFORMATION);
			GetDlgItem(ID_EB_PWD)->SetFocus();		

			// 여기서 다음 작업을 시작한다. 
			if( m_nUseFlag == SAVE_WORK )
			{
				// 저장 작업
				// 입력값 검사
				if( chkValidation() == FALSE )
				{
					return FALSE ;
				}

				// 확인메세지
				CString strTmp;
				strTmp.Format("==== 입력하신내용 =======\n"
							  " 주민번호 : %s\n"
							  " 사용자명 : %s\n"
							  " 사용자ID : %s\n"
							  " 비밀번호 : %s\n"
							  " 소속       : %s\n"
							  " 팀          : %s\n"
							  " 직급       : %s\n"
							  " 작업일지 : %s\n"
							  " 사무실☎ : %s\n"
							  " 내선번호 : %s\n"
							  " 핸드폰☏ : %s\n"
							  "=========================\n"
							  "위내용으로 등록하시겠습니까?"
							   , m_strJuminNo
							   , m_strName
							   , m_strId
							   , m_strPw1
							   , m_cbCompany.GetName()
							   , m_cbTeam.GetName()
							   , m_cbGrade.GetName()
							   , (m_nDocYesNo==0) ? "작성" : "미작성"
							   , m_strTelOfc
							   , m_strTelIn
							   , m_strTelHp
							  );
				if( AfxMessageBox(strTmp, MB_YESNO|MB_ICONQUESTION) == IDNO )
				{
					return FALSE ;
				}
				
				if( !DoSave() )
				{
					return FALSE;
				}				
			}
		}
		else
		{
			strTmp.Format("ERROR : %s",pComRslt->szErrMsg);
			AfxMessageBox(strTmp, MB_ICONINFORMATION);
			m_strId = _T("");
			SetDlgItemText(ID_EB_USER_ID, m_strId);
			GetDlgItem(ID_EB_USER_ID)->SetFocus();
			
			return FALSE;
		}
		
	}
    
	return TRUE;
}

LRESULT CRegNewUserDlg::OnCompanyInfoRslt(WPARAM wParam, LPARAM lParam)
{	
	m_cbCompany.DeleteAllItem();

	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		CString		strName;
		CString		strCode;

		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(i));		
		
		strName = pData->m_strCodeName;		
		strCode += pData->m_strCode;
		strCode += IDMSCOMBOBOX_DELIM;		
				
		m_cbCompany.AddItem(strName, strCode);
	}
	m_cbCompany.SetCurSel(0);		
	m_cbCompany.SetCode(m_strCompany);


	// Clear	
	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
		}
	}
	
	delete pArray;
	UpdateData(FALSE);	
	
	// 소속 -> TEAM. (CODE_NAME, CODE)
	TRACEX("RAWSQL_TEAM 요청!\n");
	CString strQuery = GetQueryTeam(m_cbCompany.GetCode());
	if(CChatSession::Instance().RequestRawSQL(RAWSQL_TEAM, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	{
		return FALSE;
	}	

	return TRUE;
}

//직급 	
LRESULT CRegNewUserDlg::OnClassInfoRslt(WPARAM wParam, LPARAM lParam)
{
	m_cbGrade.DeleteAllItem();

	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		CString		strName;
		CString		strCode;

		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(i));		
		
		strName = pData->m_strCodeName;		
		strCode += pData->m_strCode;
		strCode += IDMSCOMBOBOX_DELIM;		
				
		m_cbGrade.AddItem(strName, strCode);
	}
	m_cbGrade.SetCurSel(0);		
	m_cbGrade.SetCode(m_strGrade);

	// Clear	
	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
		}
	}
	

	delete pArray;

	UpdateData(FALSE);

	return TRUE;
}

LRESULT CRegNewUserDlg::OnTeamInfoRslt(WPARAM wParam, LPARAM lParam)
{
	m_cbTeam.DeleteAllItem();

	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		CString		strName;
		CString		strCode;

		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(i));		
		
		strName = pData->m_strCodeName;		
		strCode += pData->m_strCode;
		strCode += IDMSCOMBOBOX_DELIM;		
				
		m_cbTeam.AddItem(strName, strCode);
	}
	m_cbTeam.SetCurSel(0);	
	m_cbTeam.SetCode(m_strTeamGn);			

	// Clear	
	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		SCOMBO_INFO *pData = static_cast<SCOMBO_INFO*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
		}
	}
	
	delete pArray;	
	UpdateData(FALSE);	
	
		
	return TRUE;
}

// 사용자 정보 조회 결과 
LRESULT CRegNewUserDlg::OnGetUserInfoRslt(WPARAM wParam, LPARAM lParam)
{
	CPtrArray* pArray = (CPtrArray*)lParam;
	
	for(int i = 0; i < pArray->GetSize(); i++)
	{
		SUSER_INFO *pData = static_cast<SUSER_INFO*>(pArray->GetAt(i));

		m_strJuminNo		= pData->m_strJUMIN_NO;
		m_strName			= pData->m_strUSER_NAME;
		m_strId				= pData->m_strUSER_ID;
		m_strPw1 = m_strPw2 = pData->m_strPASSWD;
		m_strGrade			= pData->m_strJOB_CLASS;
		m_strCompany        = pData->m_strCOMPANY_POS;
		
		m_strIdOld = m_strId;

		//소속 조회 
		TRACEX("RAWSQL_GETCOMPANY_INFO 요청!\n");
		CString		strSql;	
		strSql = GetQueryCompany(); // name, code 구한다.	
		if(CChatSession::Instance().RequestRawSQL(RAWSQL_GETCOMPANY_INFO, (LPCSTR)strSql, strlen((LPCSTR)strSql) ) != TRUE)
		{
			return FALSE ;
		}		
		
		m_strJobWrite		= pData->m_strJOB_WRITE_GN;

		if( m_strJobWrite == _T("Y") )
			m_nDocYesNo = 0;
		else
			m_nDocYesNo = 1;

		m_strTelOfc			= pData->m_strOFF_TEL_NO;
		m_strTelIn			= pData->m_strOFF_IN_NO;
		m_strTelHp			= pData->m_strHP_TEL_NO;	
		m_strTeamGn			= pData->m_strTEAM_GN;			
	}	
	
	// 사무실전화번호
	if( m_strTelOfc.GetLength() >= 12 )
	{
		m_strTelOfc1.Format(_T("0%d"), atoi(m_strTelOfc.Left(4)));
		m_strTelOfc2 = m_strTelOfc.Mid(4, 4);

		if( m_strTelOfc2.GetAt(0) == _T('0') )
		{
			m_strTelOfc2.Delete(0, 1);
		}

		m_strTelOfc3 = m_strTelOfc.Right(m_strTelOfc.GetLength() - 8);
	}
	else
		m_strTelOfc1 = m_strTelOfc2 = m_strTelOfc3 = _T("");
	// 핸드폰번호
	if( m_strTelHp.GetLength() >= 12 )
	{
		m_strTelHp1.Format(_T("0%d"), atoi(m_strTelHp.Left(4)));
		m_strTelHp2 = m_strTelHp.Mid(4, 4);
		
		if( m_strTelHp2.GetAt(0) == _T('0') )
		{
			m_strTelHp2.Delete(0, 1);
		}

		m_strTelHp3 = m_strTelHp.Right(m_strTelHp.GetLength() - 8);
	}
	else
		m_strTelHp1 = m_strTelHp2 = m_strTelHp3 = _T("");
				
	// Clear	
	for(int j = 0; j < pArray->GetSize(); j++)
	{
		SUSER_INFO *pData = static_cast<SUSER_INFO*>(pArray->GetAt(j));
		if(pData != NULL)
		{
			delete pData;
		}
	}

	delete pArray;			

	UpdateData(FALSE);
	

	return TRUE;
}

/*
LRESULT CRegNewUserDlg::OnRegNewUserRslt(WPARAM wParam, LPARAM lParam)
{	
	
	COM_RESPONSE * pComRslt = (COM_RESPONSE*) lParam;
	
	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		TRACE("사용자 등록\n");			
		AfxMessageBox("사용자 등록에 성공하였습니다.");
		CDialog::OnOK();
	}
	else
	{
		TRACE("사용자 등록 실패\n");			
		CString strMsg;
		strMsg.Format("사용자 등록에 실패하였습니다. %s", pComRslt->szErrMsg)	;
		strMsg.TrimRight();
		AfxMessageBox(strMsg);
	}
    
	return TRUE;
}
*/

CString CRegNewUserDlg::GetEncryptPw(const CString& strId, const CString& strPw)
{

	CDesWrapper	des;
	CString		strEncrypt;

	des.SetKey(strId);
	strEncrypt = des.GetEncrypt(strPw);

	return strEncrypt;
}


BOOL CRegNewUserDlg::IsModify()
{
	return m_bModify;
}


BOOL CRegNewUserDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//! 서버의 메시지를 받을수 있게 준비.
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());	
	
	if(m_strId.IsEmpty() == FALSE)
	{
		m_bModify = TRUE;
		InitInfo();
	}
	else
	{
		m_bModify = FALSE ;

		TRACEX("RAWSQL_GETCOMPANY_INFO 요청!\n");
		CString		strSql;	
		strSql = GetQueryCompany(); // name, code 구한다.	
		if(CChatSession::Instance().RequestRawSQL(RAWSQL_GETCOMPANY_INFO, (LPCSTR)strSql, strlen((LPCSTR)strSql) ) != TRUE)
		{
			return FALSE ;
		}

	    Sleep(1);

		// 직급 combobox setting m_cbGrade
		TRACEX("RAWSQL_GETCLASS_INFO 요청!\n");

		strSql = GetQueryClass(); // name, code 구한다.
		if(CChatSession::Instance().RequestRawSQL(RAWSQL_GETCLASS_INFO, (LPCSTR)strSql, strlen((LPCSTR)strSql) ) != TRUE)
		{
			return FALSE ;
		}

	}
	// 사용자상태에 따른 처리
	if( m_strStatus == _T("00") )
	{
		SetDlgItemText(ID_EB_PWD, _T(""));
		SetDlgItemText(ID_EB_CONFIRM_PWD, _T(""));
	}
	else
	{
		GetDlgItem(ID_EB_REG_SID		)->EnableWindow(FALSE);
		GetDlgItem(ID_EB_REG_NAME		)->EnableWindow(FALSE);
		GetDlgItem(ID_EB_USER_ID		)->EnableWindow(FALSE);
		GetDlgItem(ID_BTN_DUPID_CHECK	)->EnableWindow(FALSE);
	}

	/////////////////////////////////////////////////////////////////

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRegNewUserDlg::ReqDocListUpdSql(const CString& strNewId, const CString& strOldId, CString&  strSql)
{	
	strSql.Format("UPDATE KM_DOCLIST SET INIT_CRT_PER_ID='%s' WHERE INIT_CRT_PER_ID='%s'",strNewId,strOldId);
}

CString CRegNewUserDlg::GetQueryTeam(const CString& strCode)
{
	CString strQuery;
	
	strQuery.Format("SELECT /*+ ORDERED USE_NL(A B C) INDEX(C PK_KM_CODE) */ C.CODE_NAME, C.CODE FROM KM_CODE_REL A, KM_CODE_REL B, KM_IDMS_CODE C WHERE A.GRP_CODE='6013' AND A.CODE='%s' AND A.REL_GRP_CODE='6008' AND A.REL_GRP_CODE=B.GRP_CODE AND A.REL_CODE=B.CODE AND B.REL_GRP_CODE='1001' AND B.REL_GRP_CODE=C.GRP_CODE AND B.REL_CODE=C.CODE AND C.END_DATE > SYSDATE ORDER BY C.OUT_SEQ", strCode);

	TRACE( "GetQueryTeam [%d]\n", strQuery.GetLength());
	return strQuery;
}


CString CRegNewUserDlg::GetQueryCompany()
{
    return CString ("SELECT /*+ INDEX(A PK_KM_CODE) */ A.CODE_NAME, A.CODE FROM KM_IDMS_CODE A WHERE A.GRP_CODE='6013' AND A.END_DATE > SYSDATE ORDER BY A.OUT_SEQ");
	
	//TRACE( "GetQueryCompany [%d]\n", strQuery.GetLength());
	//return strQuery;

	
//	strQuery =	"SELECT /*+ INDEX(A PK_KM_CODE) */ "
//				"       A.CODE_NAME, A.CODE        "
//				"  FROM KM_IDMS_CODE A             "
//				" WHERE A.GRP_CODE = '6013'        "
//				"   AND A.END_DATE > SYSDATE       "
//				" ORDER BY A.OUT_SEQ               ";	
	
}

CString CRegNewUserDlg::ModuleInfoUpdSql_1(const CString& strNewId, const CString& strOldId)
{
	CString  strQuery;

	strQuery.Format("UPDATE KM_MODULE SET DESIGN_PERSON='%s' WHERE DESIGN_PERSON='%s'", strNewId, strOldId);

	TRACE( "ModuleInfoUpdSql_1 [%d]\n", strQuery.GetLength());
	return strQuery;
}

CString CRegNewUserDlg::ModuleInfoUpdSql_2(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_MODULE SET CODING_PERSON='%s' WHERE CODING_PERSON='%s'", strNewId, strOldId );
	return strSql;
}

CString CRegNewUserDlg::ModuleInfoUpdSql_3(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_MODULE SET KT_PERSON='%s' WHERE KT_PERSON='%s'", strNewId, strOldId );
	return strSql;
}

CString CRegNewUserDlg::ModuleInfoUpdSql_4(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_MODULE SET KTINSU_PERSON='%s' WHERE KTINSU_PERSON='%s'", strNewId, strOldId );
	return strSql;
}
CString CRegNewUserDlg::ModuleInfoUpdSql_5(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_MODULE SET SICC_PERSON='%s' WHERE SICC_PERSON='%s'", strNewId, strOldId );
	return strSql;
}

CString CRegNewUserDlg::ReqInfoUpdSql_1(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_REQ SET REQUIRE_PERSON='%s' WHERE REQUIRE_PERSON='%s'", strNewId, strOldId );
	return strSql;

}
CString CRegNewUserDlg::ReqInfoUpdSql_2(const CString& strNewId, const CString& strOldId)
{
	CString  strSql;
	strSql.Format("UPDATE KM_REQ SET WORK_ID='%s' WHERE WORK_ID='%s'", strNewId, strOldId );
	return strSql;

}

CString CRegNewUserDlg::ReqDtlInfoUpdSql_1(const CString& strNewId, const CString& strOldId)
{
    CString  strSql;
	strSql.Format("UPDATE KM_REQ_DTL SET ANAL_PERSON='%s' WHERE ANAL_PERSON='%s'", strNewId, strOldId );

	return strSql;
}
CString CRegNewUserDlg::ReqDtlInfoUpdSql_2(const CString& strNewId, const CString& strOldId)
{
    CString  strSql;
	strSql.Format("UPDATE KM_REQ_DTL SET WORK_PERSON='%s' WHERE WORK_PERSON='%s'", strNewId, strOldId );	
	return strSql;
}

CString CRegNewUserDlg::GetQueryClass()
{
	return CString ("SELECT /*+ INDEX(A PK_KM_CODE) */ A.CODE_NAME,A.CODE FROM KM_IDMS_CODE A WHERE A.GRP_CODE='6004' AND A.END_DATE > SYSDATE ORDER BY A.OUT_SEQ");			
}

CString CRegNewUserDlg::GetQueryTopTeam(const CString& strTeamCd)
{
	CString strQuery;

	strQuery.Format("SELECT /*+ INDEX(A PK_KM_CODE) */ A.CODE,A.CODE_NAME FROM KM_IDMS_CODE A WHERE A.GRP_CODE='6008' AND A.CODE='%s' AND A.END_DATE > SYSDATE ORDER BY A.OUT_SEQ",  strTeamCd );	

	TRACE( "GetQueryTopTeam [%d]\n", strQuery.GetLength());
	return strQuery;
}

BOOL CRegNewUserDlg::InitInfo()
{
	CString strQuery = GetQuerySearch(m_strId);			

	if(CChatSession::Instance().RequestRawSQL(RAWSQL_GETUSER_INFO, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	{
		return FALSE ;
	}	
	
	return TRUE;
}


CString CRegNewUserDlg::GetQuerySearch(const CString& strId)
{
	CString	strQuery;

	strQuery.Format("SELECT JUMIN_NO,USER_NAME,USER_ID,PASSWD,TEAM_GN,JOB_CLASS,JOB_WRITE_GN,OFF_TEL_NO, OFF_IN_NO,HP_TEL_NO,COMPANY_POS FROM KM_DOCUSER WHERE USER_ID='%s'", strId );	

	TRACE( "GetQuerySearch [%d]\n", strQuery.GetLength());
	return strQuery;
}

CString CRegNewUserDlg::GetQueryInsert()
{
	UpdateData(TRUE);

	CString strQuery;

	strQuery.Format("INSERT INTO KM_DOCUSER (JUMIN_NO,USER_NAME,USER_ID,PASSWD,TEAM_GN,JOB_CLASS,JOB_WRITE_GN,USER_GRADE,OFF_TEL_NO,OFF_IN_NO,HP_TEL_NO,START_DATE,END_DATE,COMPANY_POS,DISPLAY_ORDER,STATUS,NICK_NAME)VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','99','01','%s')",
		m_strJuminNo,m_strName,m_strId,GetEncryptPw(m_strId, m_strPw1),m_cbTeam.GetCode(),m_cbGrade.GetCode() ,m_strJobWrite ,
		_USER_GRADE,m_strTelOfc,m_strTelIn,m_strTelHp,CTime::GetCurrentTime().Format("%Y%m%d"),_END_DATE,m_cbCompany.GetCode(),m_strRegUserNick);
	
	
	TRACE( "GetQueryInsert [%d]\n", strQuery.GetLength());
	
	return strQuery;	
}

CString CRegNewUserDlg::GetQueryUpdate(const CString& strId,const CString& strOldId)
{
	CString strQuery;

	strQuery.Format("UPDATE KM_DOCUSER SET JUMIN_NO='%s',USER_NAME='%s',USER_ID='%s',PASSWD='%s',TEAM_GN='%s',JOB_CLASS='%s',JOB_WRITE_GN='%s',OFF_TEL_NO='%s',OFF_IN_NO='%s',HP_TEL_NO='%s',COMPANY_POS='%s',STATUS='01' WHERE USER_ID='%s'",
	m_strJuminNo, m_strName, strId, GetEncryptPw(m_strId, m_strPw1), m_cbTeam.GetCode(),m_cbGrade.GetCode(), m_strJobWrite, m_strTelOfc, m_strTelIn, m_strTelHp, m_cbCompany.GetCode(), strOldId );

	TRACE( "GetQueryInsert [%d]\n", strQuery.GetLength());

	return strQuery;
}


//////////////

void CRegNewUserDlg::OnBnSave() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	m_nUseFlag = SAVE_WORK ;

	//입력전 다시 사용자 ID의 중복여부를 Check해야함
	
	if( m_strId != m_strIdValid )
	{
		OnBtnDupidCheck();
	}	
	else
	
	{
		// 저장 작업, 입력값 검사
		if( chkValidation() == FALSE )
		{
			return ;
		}

		// 확인메세지
		CString strTmp;
		strTmp.Format("==== 입력하신내용 =======\n"
			" 주민번호 : %s\n"
			" 사용자명 : %s\n"
			" 사용자ID : %s\n"
			" 비밀번호 : %s\n"
			" 소속       : %s\n"
			" 팀          : %s\n"
			" 직급       : %s\n"
			" 작업일지 : %s\n"
			" 사무실☎ : %s\n"
			" 내선번호 : %s\n"
			" 핸드폰☏ : %s\n"
			"=========================\n"
			"위내용으로 등록하시겠습니까?"
			, m_strJuminNo
			, m_strName
			, m_strId
			, m_strPw1
			, m_cbCompany.GetName()
			, m_cbTeam.GetName()
			, m_cbGrade.GetName()
			, (m_nDocYesNo==0) ? "작성" : "미작성"
			, m_strTelOfc
			, m_strTelIn
			, m_strTelHp
			);
		if( AfxMessageBox(strTmp, MB_YESNO|MB_ICONQUESTION) == IDNO )
		{
			return  ;
		}
				
		if( !DoSave() )
		{
			return ;
		}				
	}
}


BOOL CRegNewUserDlg::DoSave()
{	
	CString		strQueryAllTran = _T("");
	CString		strQuery = _T("");
	CString     strError=_T("");

	m_strTelOfc.Remove('-');
	m_strTelHp .Remove('-');
	
	//USER INFO KM_DOCUSER 수정 및 등록
	if( m_bModify == FALSE )
	{
		strQuery = GetQueryInsert();
	}
	else
	{
		if(m_strId.IsEmpty() || m_strIdOld.IsEmpty())
		{
			AfxMessageBox("사용자 ID를 확인하세요");			
			return FALSE;
		}
			
		if( m_strStatus == "00")
		{
			if(m_strIdOld == m_strId)
			{
				AfxMessageBox("사용자 ID를 변경 하셔야 합니다");				
				return FALSE;
			}

		}

		strQuery = GetQueryUpdate(m_strId, m_strIdOld);
	}
	
	strQueryAllTran += strQuery ;
	
	//수정인 경우 관련 정보 테이블 Update
	if( m_bModify == TRUE )
	{
		if	(
				m_strIdOld.IsEmpty() == FALSE &&
				m_strId.IsEmpty() ==  FALSE &&
				m_strIdOld != m_strId
			)
		{
			//KM_MODULE 수정
		    strQuery = ModuleInfoUpdSql_1(m_strId, m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;
			
			//KM_MODULE 수정
		    strQuery = ModuleInfoUpdSql_2(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;
			
			//KM_MODULE 수정
		    strQuery = ModuleInfoUpdSql_3(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			strQuery = ModuleInfoUpdSql_4(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			strQuery = ModuleInfoUpdSql_5(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			//KM_REQ 수정
		    strQuery = ReqInfoUpdSql_1(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			strQuery = ReqInfoUpdSql_2(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			//KM_REQ_DTL 수정
		    strQuery = ReqDtlInfoUpdSql_1(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			//KM_REQ_DTL 수정
		    strQuery = ReqDtlInfoUpdSql_2(m_strId,m_strIdOld);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;

			//KM_DOCLIST 수정
		    ReqDocListUpdSql(m_strId,m_strIdOld, strQuery);
		    strQueryAllTran += strQuery ;
			strQueryAllTran += "!" ;
		}		
	}

	if(CChatSession::Instance().RequestRawSQL(RAWSQL_REG_NEW_USER, (LPCSTR)strQueryAllTran, strlen((LPCSTR)strQueryAllTran) ) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CRegNewUserDlg::chkValidation()
{	
	// 주민번호
	if( m_strJuminNo.GetLength() != 13 )
	{
		AfxMessageBox("주민번호 13자리를 확인하세요", MB_ICONINFORMATION);
		GetDlgItem(ID_EB_REG_SID)->SetFocus();
		((CEdit*)GetDlgItem(ID_EB_REG_SID))->SetSel(0,-1);
		return FALSE;
	}

	// 사용자명
	if( m_strName.GetLength() == 0 )
	{
		AfxMessageBox("사용자이름을 입력하세요", MB_ICONINFORMATION);
		GetDlgItem(ID_EB_REG_NAME)->SetFocus();
		((CEdit*)GetDlgItem(ID_EB_REG_NAME))->SetSel(0,-1);
		return FALSE;
	}

	// ID
	if( m_strId.IsEmpty() ){
		AfxMessageBox("사용자ID를 입력하세요");
		GetDlgItem(ID_EB_USER_ID)->SetFocus();
		return FALSE;
	}
	else
	{
		if( CheckUserId(m_strId, ID_EB_USER_ID) == FALSE )
			return FALSE;
	}

	// PASSWORD
	if( m_strPw1.IsEmpty() ){
		AfxMessageBox("비밀번호를 입력하세요");
		GetDlgItem(ID_EB_PWD)->SetFocus();
		return FALSE;
	}
	else if( m_strPw1 != m_strPw2 ){
		AfxMessageBox("입력하신 비밀번호가 일치하지 않습니다");
		GetDlgItem(ID_EB_PWD)->SetFocus();		
		return FALSE;
	}
	else
	{
		if( CheckUserPw(m_strPw1, ID_EB_PWD) == FALSE )
			return FALSE;
	}
	// 작업일지 작성
	if( m_nDocYesNo == 0 )
		m_strJobWrite = _T("Y");
	else
		m_strJobWrite = _T("N");
	// 사무실전화번호
	if( m_strTelOfc1 != _T("") || m_strTelOfc2 != _T("") || m_strTelOfc3 != _T("") )
	{
		m_strTelOfc.Format(_T("%04d-%04d-%04d")
						   , atoi(m_strTelOfc1)
						   , atoi(m_strTelOfc2)
						   , atoi(m_strTelOfc3) );
	}
	else
		m_strTelOfc = _T("");
	// 내선번호
	if( m_strTelIn != _T("") )
		m_strTelIn.Format("%d" , atoi(m_strTelIn));
	// 핸드폰
	if( m_strTelHp1 != _T("") || m_strTelHp2 != _T("") || m_strTelHp3 != _T("") )
	{
		m_strTelHp.Format(_T("%04d-%04d-%04d")
						  , atoi(m_strTelHp1)
						  , atoi(m_strTelHp2)
						  , atoi(m_strTelHp3) );
	}
	else
		m_strTelHp = _T("");

	// 소속
	if( m_cbCompany.GetCode() == _T("") )
	{
		AfxMessageBox(_T("소속을 선택하세요"));
		m_cbCompany.SetFocus();

		return FALSE;
	}
	// 팀구분
	if( m_cbTeam.GetCode() == _T("") )
	{
		AfxMessageBox(_T("팀을 선택하세요"));
		m_cbTeam.SetFocus();

		return FALSE;
	}

	return TRUE;
}

void CRegNewUserDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CRegNewUserDlg::OnBtnRegReset() 
{
	// TODO: Add your control notification handler code here
	
}

// 중복 체크
void CRegNewUserDlg::OnBtnDupidCheck() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	if( m_strId.IsEmpty() )
	{
		AfxMessageBox("사용자ID를 입력하세요");
		GetDlgItem(ID_EB_USER_ID)->SetFocus();           
		return;
	}
	else 
	if( CheckUserId(m_strId, ID_EB_USER_ID) == FALSE )
	{
		return;
	}

	char szSql [200];	
	memset(szSql, 0x00 , sizeof(szSql));
	sprintf(szSql, "SELECT USER_NAME FROM KM_DOCUSER WHERE USER_ID = '%s'", (LPCSTR)m_strId) ; 	
	
	//TEST	
	//sprintf(szSql, "SELECT USER_NAME FROM KM_DOCUSER WHERE USER_ID = '%s'!SELECT USER_NAME FROM KM_DOCUSER WHERE USER_ID = '%s'", (LPCSTR)m_strId, (LPCSTR)m_strId) ; 	
	//sprintf(szSql, "INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!INSERT INTO TB_TESTKOJH(C1) VALUES('1')!") ;	
	
	if(CChatSession::Instance().RequestRawSQL(RAWSQL_CHK_DUP_ID, szSql, strlen(szSql) ) != TRUE)
	{
		return;
	}
}

// 문자열에서 알파벳대소문자,숫자,특수문자 갯수얻기
void CRegNewUserDlg::CheckString(const CString& strSrc, int& nAlphaCap, int& nAlphaLow, int& nNumeric, int& nOther)
{
	// initialize Output value!!
	nAlphaCap = nAlphaLow = nNumeric = nOther = 0;

	char chTmp;
	for( int i=0 ; i<strSrc.GetLength() ; i++ )
	{
		chTmp = strSrc.GetAt(i);

		if	   ( chTmp >= '0' && chTmp <= '9' )	nNumeric++;
		else if( chTmp >= 'A' && chTmp <= 'Z' )	nAlphaCap++;
		else if( chTmp >= 'a' && chTmp <= 'z' )	nAlphaLow++;
		else									nOther++;
	}
}

// ID 유효성검사유효성검사
BOOL CRegNewUserDlg::CheckUserId(const CString& strSrcId, int nID/*=-1*/)
{
	// 1-20자리
	
	if( strSrcId.GetLength() > 20 )
	{
		AfxMessageBox("사용자ID는 20자리 이하의 영문 또는 영문,숫자 조합입니다", MB_ICONINFORMATION);
		
		if( nID != -1 )
		{
			GetDlgItem(nID)->SetFocus();
			((CEdit*)GetDlgItem(nID)) ->SetSel(0,-1);			
		}

		return FALSE;
	}
	else
	{
		// 숫자로 시작못함
		if( strSrcId.GetAt(0) >= '0' && strSrcId.GetAt(0) <= '9' )
		{
			AfxMessageBox("사용자ID는 숫자로 시작할 수 없습니다", MB_ICONINFORMATION);
			
			if( nID != -1 ){
				GetDlgItem(nID)->SetFocus();
				((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
			}

			return FALSE;
		}

		int nAlphaCap, nAlphaLow, nNumeric, nOther;
		CheckString(strSrcId, nAlphaCap, nAlphaLow, nNumeric, nOther);

		// 특수문자금지!!
		if( nOther > 0 )
		{
			AfxMessageBox("사용자ID는 6-8자리의 영문 또는 영문,숫자 조합입니다\n특수문자 사용불가!!", MB_ICONINFORMATION);
			
			if( nID != -1 )
			{
				GetDlgItem(nID)->SetFocus();
				((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
			}

			return FALSE;
		}		
	}
	

	return TRUE;
}

// PASSWORD 유효성검사
BOOL CRegNewUserDlg::CheckUserPw(const CString& strSrcPw, int nID )
{
	// 6-8자리	
	if( strSrcPw.GetLength() < 6 || strSrcPw.GetLength() > 8 )
	{
		AfxMessageBox("비밀번호는 6-8자리 입니다", MB_ICONINFORMATION);

		if( nID != -1 ) 
		{
			GetDlgItem(nID)->SetFocus();
			((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
		}

		return FALSE;
	}

	if( strSrcPw.Find(_T(' '), 0) >= 0 )
	{
		AfxMessageBox("비밀번호는 공백을 포함할 수 없습니다", MB_ICONINFORMATION);

		if( nID != -1 ) 
		{
			GetDlgItem(nID)->SetFocus();
			((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
		}

		return FALSE;
	}
	
	
	return TRUE;
}

void CRegNewUserDlg::OnSelchangeCbCompany() 
{
	// TODO: Add your control notification handler code here
	
	TRACEX("RAWSQL_TEAM 요청!\n");

	CString strQuery = GetQueryTeam(m_cbCompany.GetCode());			
	if(CChatSession::Instance().RequestRawSQL(RAWSQL_TEAM, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	{
		return ;
	}	

	/*
	if( InitCombo(&m_cbTeam, strSql) == FALSE )
		return;

	m_cbTeam.InsertItem(0, _T(""), _T("`"));
	m_cbTeam.SetCurSel(0);
	*/
}
