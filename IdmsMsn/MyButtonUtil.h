// MyButtonUtil.h: interface for the CMyButtonUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_)
#define AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



// Define
#define	BUTTON_SETUP				0  //ȯ�漳��
#define BUTTON_CLOSE				1	//�ݱ�  
#define BUTTON_CLOSE_SMALL			2	//�ݱ� (Small Size)
#define BUTTON_REFRESH				3	//ȭ�� ���� 
#define BUTTON_SAVE					4	//ȭ�� ����
#define BUTTON_PRINT				5	//�μ� 
#define BUTTON_PRINT_SMALL			6	//�μ� (Small Size)
#define BUTTON_EXCEL				7	//���� ��ȯ
#define BUTTON_SETTING				8	//ȯ�� ����
#define BUTTON_SEARCH				9	//��ȸ
#define BUTTON_SEARCH_SMALL			10	//��ȸ (Small Size)



//���� ��ȸ
#define BUTTON_TAB_NETWORK			12   //�� - ����
#define BUTTON_TAB_NODE				13   //�� - �ܱ�


//MX2800, ETC, N-DCS
#define BUTTON_TAB_MX2800			14	 //�� - MX2800 
#define BUTTON_TAB_ETC				15   //�� - ��Ÿ���
#define BUTTON_TAB_NDCS				16   //�� - N-DCS

//���������̷� 
#define BUTTON_TAB_NODE_MUX			17   //�� - �ܱ� MUX

// ���� ����ܱ�,�뱹��� ��ȸ ȭ�� 
#define BUTTON_TAB_MX2800_EX		20   //�� - MX2800
#define BUTTON_TAB_ETC_EX			21   //�� - ��Ÿ���


//���� ����
#define BUTTON_MNG_ADD				101	 //�߰� 
#define BUTTON_MNG_MODI				102  //���� 
#define BUTTON_MNG_DEL				103  //����
#define BUTTON_MNG_ADD_SMALL		104	 //�߰�(Small) 
#define BUTTON_MNG_MODI_SMALL		105  //����(Small)  
#define BUTTON_MNG_DEL_SMALL		106  //����(Small) 
#define BUTTON_MNG_CLOSE_SMALL		107  //�ݱ�(Small) 
#define BUTTON_SEARCH_VERYSMALL		108  //�˻�(Small) 
#define	BUTTON_SEARCH_DLG			109	 //�˻�â �ٿ�� ���� ��ȸ��ư (�ܱ�/������� ���� ..)
#define	BUTTON_RELATION_CANCEL		110	 //�뱹 ��ҹ�ư


//���� ���� / ���� ȭ��
#define BUTTON_DUMMY_CONN			200  //�����������
#define BUTTON_RECONNECT			201  //�翬��
#define BUTTON_SCREENSAVE			202	 //ȭ������
#define BUTTON_GALMURI				203  //������ ����
#define BUTTON_GALMURIEND			204  //������ �� 
#define BUTTON_DUMMY_ADD			205	 //��� (������� ���� ȭ��)  
#define BUTTON_DUMMY_MODI			206  //���� (������� ���� ȭ��) 
#define BUTTON_DUMMY_DEL			207  //���� (������� ���� ȭ��)  



//��� ��Ȳ
#define BUTTON_ALERTLOG				301	 //��ַα� 
#define BUTTON_ALERTSTATUS			303  //�����Ȳ
#define BUTTON_ALERTSTATUSDB		304  //����̷� DB

//��û��������
#define BUTTON_ALERTSOUND_PATH		302  //��ֻ��� 
#define BUTTON_ALERTSOUND_PLAY		305  //��ֻ��� ���	 


//���� ���� 
#define BUTTON_DFILTER_NETWORK_ALL  401  //��μ���
#define BUTTON_DFILTERCFG			402	 //�������� ����
#define BUTTON_DFILTERCFGBRIEF		403	 //��������


//��������
#define BUTTON_ADD_DIALOG			501	 //���ȭ��
#define BUTTON_RUN					502	 //����
#define BUTTON_REMOTE_CANCEL		503	 //���


//�ܱ����� ����
#define BUTTON_DOWN					601	 //��
#define BUTTON_UP					602	 //��

//CRS
#define BUTTON_REFRESH_DB			701	 //DB���� 


//�۾���/Inhibit ��ȸ
#define BUTTON_TAB_WORKING			801	 //�� - �۾��� 
#define BUTTON_TAB_INHIBIT			802  //�� - Inhibit


//������
#define BUTTON_SUNBUNJANG			901	 //������

//�������� ���� ��ư 
#define BUTTON_DUMMYTERMINAL_SHOW	902

//��Ÿ
#define BUTTON_OK					1000
#define BUTTON_CANCEL				1001
#define BUTTON_APPLY				1002  //����
#define BUTTON_APPLY2				1003  //����
#define BUTTON_COLOR_TEXT			1004
#define BUTTON_COLOR_BG				1005

//��û�溸 �ݱ�
#define BUTTON_CLOSE_AUDIBLE		1006

//����â
#define BUTTON_RESPONSEWND			1007


// MX-2800 ����
#define BUTTON_MX2800_ALARM_SEARCH	1050
#define BUTTON_MX2800_ALARM_SYNC	1051
#define BUTTON_MX2800_STATUS_SEARCH	1052
#define BUTTON_MX2800_RESPONSE		1053



class CMyButtonUtil  
{
public:
	CMyButtonUtil();
	virtual ~CMyButtonUtil();

	void  AutoSetting(UINT nType, CWnd *pButton, CWnd* wndParent);
};

#endif // !defined(AFX_MYBUTTONUTIL_H__A402F54D_2F0B_498E_A07A_BF1BA7CF1D8B__INCLUDED_)
