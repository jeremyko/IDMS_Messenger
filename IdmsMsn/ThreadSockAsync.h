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
	//스레드 함수
protected:	
	friend UINT fThreadProc( LPVOID pParam );
//생성자
public:
	CThreadSockAsync();
	virtual ~CThreadSockAsync();	
	DECLARE_DYNCREATE(CThreadSockAsync)


//일반 함수
public:		
		
	BOOL SocketConnect(UINT nSocketPort, LPCTSTR lpSocketAddress, BOOL bStartThread = TRUE); 
	
	int	 Start();				 ///< 스레드 가동 
	int	 Stop(BOOL wait = TRUE); ///< 스레드 종료( true : 스레드가 종료될때까지 대기)
	void Close();				 ///< 스레드 종료 및 소켓 연결 종료 
	void CloseForTerminate();   ///< 스레드 종료 및 소켓 연결 종료 
	
	int  Send(char* lpBuf,int iBufLen, const char* szUsage = NULL );	///< 큐 이용하지않은 직접 전송 함수	
	int     GetOnePacketLength(char *pData, int iLength);
	CString	GetCurrentTimeByString(BOOL bOnlyTime = TRUE);
	void	SetParentWnd(HWND hWnd);

//일반 변수
public:
	BOOL        mbConnected;
	BOOL		m_bDeadThreadFlag;
	BOOL		m_bRunExitFlag;					///< 쓰레드 Run 종료 플래그		
	HWND		m_hOwnerWnd;	
	int			m_nClassID;						///< 스레드 디버깅 목적으로 정의한 변수
	int			m_bThreadStart;					///< 스레드 가동 여부		
	char		*m_pBuff;						///< 데이타가 들어갈 버퍼
	int			m_iBuffLength;					///< 모아놓은(m_pBuff) 데이터의 크기		

//소켓 관련 변수
public:	
	SOCKET		m_hSocket;	
	SOCKADDR_IN m_InternetAddr;


//내부 사용 멤버 함수
protected:
	plugware::CCriticalSectionLocalScope m_criSecLocalScopeSend;	
	int						_init();			///< 초기화
	int						_shutdown();		///< 쓰레드 종료시 작업	
	unsigned				_run();				///< 쓰레드 몸체
	int						_do_recv();			///< 데이터 수신 
	int						_do_send();			///< 데이터 전송(큐 이용)
	
	//! 전송할 데이터를 큐(STL deque)에 넣고 사용한다	
	void					_put_buffer(char* data, int nLen); ///< 큐에 전송할 데이터 넣기
	void					_clear_buffer();		 ///< 큐 모두 지움
	//! 큐로부터 얻은 데이터 다시 제자리에
	void					_unget_buffer(SBUFDATA data); 
	//! 큐에 있는 데이터 얻음
	SBUFDATA				_get_buffer();		
		


//내부 사용 멤버 변수	
protected:
	CWinThread				*m_pThread;	
	std::deque<SBUFDATA>	m_bufs;
	CRITICAL_SECTION		m_cs;	
	HANDLE					m_work_event;   ///< send 시 발생 
	HANDLE					m_kill_event;   ///< thread 종료
	HWND					m_pHWnd;


// Overridable
protected:
	//! 데이터를 받았을때 처리하는 함수
	virtual BOOL ProcessRecvData(char *pBuff, int nPacketLen) {return FALSE;};

	//! 연결이 끊겼을 경우 처리되는 함수
	virtual void ProcessWhenConnClosed(){};

};

#endif 
