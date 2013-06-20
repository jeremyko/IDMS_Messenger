#if !defined(__THREADSOCKASYNC__)
#define __THREADSOCKASYNC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>
#include <winsock2.h>
#include "SP_Utility.h"
//#include "AllDefines.h"
#include "..\\common\\AllCommonDefines.h"
#include "..\\common\\utility.h"



typedef struct _SBUFDATA
{
	int nLen;
	char* pData;	
}SBUFDATA;

class CThreadSockAsync: public CObject  
{
	//������ �Լ�
protected:	
	friend UINT fThreadProc( LPVOID pParam );
//������
public:
	CThreadSockAsync();
	virtual ~CThreadSockAsync();	
	DECLARE_DYNCREATE(CThreadSockAsync)


//�Ϲ� �Լ�
public:		
		
	BOOL SocketConnect(UINT nSocketPort, LPCTSTR lpSocketAddress, BOOL bStartThread = TRUE); 
	
	int	 Start();				 ///< ������ ���� 
	int	 Stop(BOOL wait = TRUE); ///< ������ ����( true : �����尡 ����ɶ����� ���)
	void Close();				 ///< ������ ���� �� ���� ���� ���� 
	void CloseForTerminate();   ///< ������ ���� �� ���� ���� ���� 
	
	int  Send(char* lpBuf,int iBufLen, const char* szUsage = NULL );	///< ť �̿��������� ���� ���� �Լ�	
	int     GetOnePacketLength(char *pData, int iLength);
	CString	GetCurrentTimeByString(BOOL bOnlyTime = TRUE);
	void	SetParentWnd(HWND hWnd);

//�Ϲ� ����
public:
	BOOL        mbConnected;
	BOOL		m_bDeadThreadFlag;
	BOOL		m_bRunExitFlag;					///< ������ Run ���� �÷���		
	HWND		m_hOwnerWnd;	
	int			m_nClassID;						///< ������ ����� �������� ������ ����
	int			m_bThreadStart;					///< ������ ���� ����		
	char		*m_pBuff;						///< ����Ÿ�� �� ����
	int			m_iBuffLength;					///< ��Ƴ���(m_pBuff) �������� ũ��		

//���� ���� ����
public:	
	SOCKET		m_hSocket;	
	SOCKADDR_IN m_InternetAddr;


//���� ��� ��� �Լ�
protected:
	plugware::CCriticalSectionLocalScope m_criSecLocalScopeSend;	
	int						_init();			///< �ʱ�ȭ
	int						_shutdown();		///< ������ ����� �۾�	
	unsigned				_run();				///< ������ ��ü
	int						_do_recv();			///< ������ ���� 
	int						_do_send();			///< ������ ����(ť �̿�)
	
	//! ������ �����͸� ť(STL deque)�� �ְ� ����Ѵ�	
	void					_put_buffer(char* data, int nLen); ///< ť�� ������ ������ �ֱ�
	void					_clear_buffer();		 ///< ť ��� ����
	//! ť�κ��� ���� ������ �ٽ� ���ڸ���
	void					_unget_buffer(SBUFDATA data); 
	//! ť�� �ִ� ������ ����
	SBUFDATA				_get_buffer();		
		


//���� ��� ��� ����	
protected:
	CWinThread				*m_pThread;	
	std::deque<SBUFDATA>	m_bufs;
	CRITICAL_SECTION		m_cs;	
	HANDLE					m_work_event;   ///< send �� �߻� 
	HANDLE					m_kill_event;   ///< thread ����
	HWND					m_pHWnd;


// Overridable
protected:
	//! �����͸� �޾����� ó���ϴ� �Լ�
	virtual BOOL ProcessRecvData(char *pBuff, int nPacketLen) {return FALSE;};

	//! ������ ������ ��� ó���Ǵ� �Լ�
	virtual void ProcessWhenConnClosed(){};

};

#endif 
