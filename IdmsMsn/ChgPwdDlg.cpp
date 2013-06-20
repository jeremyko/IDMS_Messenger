// ChgPwdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IdmsMsn.h"
#include "ChgPwdDlg.h"
#include "ChatSession.h"
#include "DesWrapper.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChgPwdDlg dialog


CChgPwdDlg::CChgPwdDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChgPwdDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChgPwdDlg)
	m_strPwOld = _T("");
	m_strPwNew = _T("");
	m_strPwCfrm = _T("");
	m_strId = _T("");
	//}}AFX_DATA_INIT
}


void CChgPwdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChgPwdDlg)
	DDX_Text(pDX, ID_EB_CHANGEPW_OLD, m_strPwOld);
	DDX_Text(pDX, ID_EB_CHANGEPW_NEW, m_strPwNew);
	DDX_Text(pDX, ID_EB_CHANGEPW_CONFIRM, m_strPwCfrm);
	DDX_Text(pDX, ID_EB_CHANGEPW_ID, m_strId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChgPwdDlg, CDialog)
	//{{AFX_MSG_MAP(CChgPwdDlg)
	ON_BN_CLICKED(ID_BTN_CHANGEPW_SAVE, OnBtnChangepwSave)
	ON_BN_CLICKED(ID_BTN_CHANGEPW_RESET, OnBtnChangepwReset)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CHK_USERID, OnUserChkRslt)
	ON_MESSAGE(WM_UPDATE_PASSWD, OnChgPassWdRslt)
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChgPwdDlg message handlers

LRESULT CChgPwdDlg::OnUserChkRslt(WPARAM wParam, LPARAM lParam)
{	
	CString strTmp;
	S_CHK_USERID * pRslt = (S_CHK_USERID*) lParam;
	CSP_Utility<S_CHK_USERID> spTest = pRslt;

	if(strcmp(pRslt->szExists ,"N") == 0 )
	{
		AfxMessageBox("��ϵ� ����ڰ� �ƴմϴ�");
		return FALSE;
	}

	/*
	CString strEncrpTmp;
	strEncrpTmp = GetEncryptPw( "CYH", "22222222");
	AfxMessageBox(strEncrpTmp);
	*/

	GetDecryptPw( m_strId, CString(pRslt->szPasswd) , strTmp);
	if( strTmp != m_strPwOld )
	{
		AfxMessageBox("���� ��й�ȣ�� Ʋ���ϴ�");
		
		GetDlgItem(ID_EB_CHANGEPW_OLD)->SetFocus();
		return FALSE;
	}

	//SQL Send
	CString strQuery;
	strQuery.Format("UPDATE KM_DOCUSER SET PASSWD = '%s' WHERE USER_ID = '%s'"
				  , GetEncryptPw(m_strId, m_strPwNew)
				  , m_strId
				 );
	
	if(CChatSession::Instance().RequestRawSQL(RAWSQL_UPDATE_PASSWD, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	{
		AfxMessageBox("�ٽ� �õ��غ��ð� ����ؼ� �����߻��� �����ڿ��� �����ϼ���", MB_ICONINFORMATION);
		return FALSE;
	}	

	return TRUE;
}

LRESULT CChgPwdDlg::OnChgPassWdRslt(WPARAM wParam, LPARAM lParam)
{	
	S_COM_RESPONSE * pComRslt = (S_COM_RESPONSE*) lParam;
	CSP_Utility<S_COM_RESPONSE> spTest = pComRslt;

	if(strcmp(pComRslt->szRsltCd ,"Y") == 0 )
	{
		AfxMessageBox("��й�ȣ�� ����Ǿ����ϴ�", MB_ICONINFORMATION);
	}
	else
	{
		AfxMessageBox("�ٽ� �õ��غ��ð� ����ؼ� �����߻��� �����ڿ��� �����ϼ���", MB_ICONINFORMATION);
		return FALSE;
	}
	
	return TRUE;
}

CString CChgPwdDlg::GetEncryptPw(const CString& strId, const CString& strPw)
{
	CDesWrapper	des;
	CString		strEncrypt;

	des.SetKey(strId);
	strEncrypt = des.GetEncrypt(strPw);

	return strEncrypt;
}

void CChgPwdDlg::GetDecryptPw(const CString& strId, const CString& strPw, CString& strDecrypt)
{
	CDesWrapper	des;
	//CString		strDecrypt;

	des.SetKey(strId);
	des.GetDecrypt(strPw, strDecrypt);

	//return strDecrypt;
}

void CChgPwdDlg::OnBtnChangepwSave() 
{
	// TODO: Add your control notification handler code here	
	
	UpdateData(TRUE);

	// ���̵�,������й�ȣ �˻�
	
	if( m_strId.IsEmpty() )
	{
		AfxMessageBox("������̸��� �Է��ϼ���");
		GetDlgItem(ID_EB_CHANGEPW_ID)->SetFocus();
		return;
	}
	else  
	if( m_strPwOld.IsEmpty() )
	{
		AfxMessageBox("������й�ȣ�� �Է��ϼ���");
		GetDlgItem(ID_EB_CHANGEPW_OLD)->SetFocus();		
		return;
	}

	// �����й�ȣ �˻�	
	if( m_strPwNew.IsEmpty() )
	{
		AfxMessageBox("�����й�ȣ�� �Է��ϼ���");
		GetDlgItem(ID_EB_CHANGEPW_NEW)->SetFocus();
		return;
	}
	else 
	if( m_strPwNew != m_strPwCfrm )
	{
		AfxMessageBox("�Է��Ͻ� �� ��й�ȣ�� �ٸ��ϴ�");
		GetDlgItem(ID_EB_CHANGEPW_NEW)->SetFocus();		
		return;
	}else
		if( CheckUserPw(m_strPwNew, ID_EB_CHANGEPW_NEW) == FALSE )
			return;
	
	// User ID Check..
	CString strQuery;
	strQuery.Format("SELECT PASSWD FROM KM_DOCUSER WHERE USER_ID = '%s'", m_strId);
	
	if(CChatSession::Instance().RequestRawSQL(RAWSQL_CHK_USERID, (LPCSTR)strQuery, strlen((LPCSTR)strQuery) ) != TRUE)
	{
		return ;
	}	


	/*
	CWaitCursor wait;
	CDBWrapper	db;
	CString		strTmp;

	// db open
	if( db.Open() == FALSE )
		return;

	// set sql
	strTmp.Format("SELECT PASSWD FROM KM_DOCUSER WHERE USER_ID = '%s'", m_strId);

	// run query : search user
	if( db.RunSql(strTmp) == FALSE )
		return;
	
	// result
	if( db.GetRecordCount() <= 0 )
	{
		AfxMessageBox("��ϵ� ����ڰ� �ƴմϴ�");
		return;
	}
	
	// password check
	strTmp = GetDecryptPw(m_strId, db.GetData("PASSWD"));
	if( strTmp != m_strPwOld )
	{
		AfxMessageBox("���� ��й�ȣ�� Ʋ���ϴ�");
		GetDlgItem(ID_EB_CHANGEPW_OLD)->SetFocus();
		return;
	}
		
	// set sql
	strTmp.Format("UPDATE KM_DOCUSER SET PASSWD = '%s' WHERE USER_ID = '%s'"
				  , GetEncryptPw(m_strId, m_strPw1)
				  , m_strId
				 );

	// run query : update password
	if( db.RunSql(strTmp) == FALSE ){
		AfxMessageBox("�ٽ� �õ��غ��ð� ����ؼ� �����߻��� �����ڿ��� �����ϼ���", MB_ICONINFORMATION);
		return;
	}else
		AfxMessageBox("��й�ȣ�� ����Ǿ����ϴ�", MB_ICONINFORMATION);
	 */
}

// PASSWORD ��ȿ���˻�
BOOL CChgPwdDlg::CheckUserPw(const CString& strSrcPw, int nID )
{
	// 6-8�ڸ�	
	if( strSrcPw.GetLength() < 6 || strSrcPw.GetLength() > 8 )
	{
		AfxMessageBox("��й�ȣ�� 6-8�ڸ� �Դϴ�", MB_ICONINFORMATION);

		if( nID != -1 ) 
		{
			GetDlgItem(nID)->SetFocus();
			((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
		}

		return FALSE;
	}

	if( strSrcPw.Find(_T(' '), 0) >= 0 )
	{
		AfxMessageBox("��й�ȣ�� ������ ������ �� �����ϴ�", MB_ICONINFORMATION);

		if( nID != -1 ) 
		{
			GetDlgItem(nID)->SetFocus();
			((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
		}

		return FALSE;
	}
	
	
	return TRUE;
}


void CChgPwdDlg::OnBtnChangepwReset() 
{
	// TODO: Add your control notification handler code here
	m_strPwOld	= _T("");
	m_strPwNew	= _T("");
	m_strPwCfrm	= _T("");
	m_strId		= _T("");

	UpdateData(FALSE);
}

void CChgPwdDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CChgPwdDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//! ������ �޽����� ������ �ְ� �غ�.
	CChatSession::Instance().SetOwnerWnd(this->GetSafeHwnd());

	((CEdit*)GetDlgItem(ID_EB_CHANGEPW_ID	))->SetLimitText(20);
	((CEdit*)GetDlgItem(ID_EB_CHANGEPW_OLD))->SetLimitText(8) ;
	((CEdit*)GetDlgItem(ID_EB_CHANGEPW_NEW	))->SetLimitText(8) ;
	((CEdit*)GetDlgItem(ID_EB_CHANGEPW_CONFIRM	))->SetLimitText(8) ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
