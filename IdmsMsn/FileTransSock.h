#if !defined(AFX_FILETRANSSOCK_H__E614E97D_88F9_40B3_84AD_B2284598C8AE__INCLUDED_)
#define AFX_FILETRANSSOCK_H__E614E97D_88F9_40B3_84AD_B2284598C8AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTransSock.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileTransSock window


#include <iostream>
#include <string>
                            
//const int MAX_BUFF_SIZE =  4096  ;

const int CSPINCOUNT = 4000 ;

#include "..\\common\\AllCommonDefines.h"

class CCriticalSectionLocalScope
{
public:     // interface
	CCriticalSectionLocalScope() throw();
	~CCriticalSectionLocalScope();        
	// modifiers
	void       unlock();
	const bool acquire_lock();
private:
	CRITICAL_SECTION m_h_cs;    
}; 


// scoped locking mechanism    	
class CAutoLock
{
public:
	~CAutoLock();
	CAutoLock(CCriticalSectionLocalScope&, char* szUsage );
	const CAutoLock& operator=(const CAutoLock&);
	CCriticalSectionLocalScope& m_c_lock;
	CString  m_strUsage;
};     

// CCriticalSectionLocalScope 구현     	
inline CCriticalSectionLocalScope::CCriticalSectionLocalScope()
{
	::InitializeCriticalSection(&m_h_cs);
}

inline CCriticalSectionLocalScope::~CCriticalSectionLocalScope()
{
	::DeleteCriticalSection(&m_h_cs);
}

inline const bool CCriticalSectionLocalScope::acquire_lock()
{
#if(_WIN32_WINNT >= 0x0400)
	return 1L & TryEnterCriticalSection(&m_h_cs);
#else
	EnterCriticalSection(&m_h_cs);
	return true;
#endif //_WIN32_WINNT >= 0x0400
}

inline void CCriticalSectionLocalScope::unlock()
{
	::LeaveCriticalSection(&m_h_cs);
}

// CAutoLock 구현     
inline CAutoLock::CAutoLock(CCriticalSectionLocalScope& c_lock, char* szUsage )
: m_c_lock(c_lock) , m_strUsage(szUsage)
{ 	
	while (false == m_c_lock.acquire_lock()) {}
	//m_c_lock.acquire_lock();
	TRACE("CAutoLock 생성 [%s]\n", m_strUsage);
}        

inline CAutoLock::~CAutoLock() 
{ 
	m_c_lock.unlock(); 	
	TRACE("CAutoLock 제거 [%s]\n",m_strUsage);
	m_strUsage.Empty();
}



class CFileTransSock  
{
	friend unsigned __stdcall PThreadProc( void* pParam ); 
protected:	
	HWND m_hOwnerWnd;
	char m_szMyIPAddr [15];
	int  m_nSockIdIndex;

public:		
	CFileTransSock();
	CFileTransSock(HWND hOwnerWnd);	
	~CFileTransSock();
	
	BOOL SocketInit(CString strIP, BOOL bStartThread = TRUE);	
	BOOL SocketInit(BOOL bStartThread = TRUE);	
		
	static CFileTransSock& Instance()
	{
		CWinApp *pApp = AfxGetApp();
		CWnd *pMainWnd = pApp->GetMainWnd();
		static CFileTransSock obj(pMainWnd->GetSafeHwnd());
		return obj;
	};
		
	CMapStringToOb	mStrFileRecvDlgMap	;	// Recv Dialog MAP	
	CMapStringToOb  mStrFileSendDlgMap	;	// Send Dialog MAP	

	CRITICAL_SECTION m_CS; // 파일전송은 서버 역활을 하므로 동기화 필요함

	//BYTE			m_buffer[MAX_BUFF_SIZE];
	//BYTE*			m_pBuff;	
	//DWORD			m_iBuffLength;	///< 모아놓은(m_pBuff) 데이터의 크기	
	DWORD			m_nFileSize, m_nReceiveSize, m_nFilePoint,  m_nLength;

	HANDLE			_thread;	
	CMapStringToOb	mSockObjMap;    ///< SOCKET_OBJ Map		
	SOCKET			m_hSocket;
	HWND			m_pHWnd;
	int				Start();	
	int				Stop(bool wait);		
	int				GetOnePacketLength(char *pData, int iLength);
	BOOL			GetHeaderInfo(SOCKET_OBJ_FILE * pSockObj);
	void			SetParentWnd(HWND hWnd);
	int             Init(HWND p_hWnd_dlg);		
	unsigned		_run();	
	int				_do_recv(SOCKET_OBJ_FILE * sock);			
	BUFFER_OBJ_FILE *	GetBufferObj(int buflen);
	void			FreeBufferObj(BUFFER_OBJ_FILE *obj);
	SOCKET_OBJ_FILE *	GetSocketObj(SOCKET s, int listening);
	void			FreeSocketObj(SOCKET_OBJ_FILE *obj);
	THREAD_OBJ_FILE *	GetThreadObj();
	void			FreeThreadObj(THREAD_OBJ_FILE *thread);
	int				InsertSocketObj(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock);
	void			RemoveSocketObj(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock);
	SOCKET_OBJ_FILE *	FindSocketObj(THREAD_OBJ_FILE *thread, int index);
	void			EnqueueBufferObj(SOCKET_OBJ_FILE *sock, BUFFER_OBJ_FILE *obj, BOOL AtHead);
	BUFFER_OBJ_FILE *	DequeueBufferObj(SOCKET_OBJ_FILE *sock);
	int				ReceivePendingData(SOCKET_OBJ_FILE *sockobj);
	int				SendPendingData(SOCKET_OBJ_FILE *sock);	
	int				HandleIo(THREAD_OBJ_FILE *thread, SOCKET_OBJ_FILE *sock);
	void			RenumberThreadArray(THREAD_OBJ_FILE *thread);	
	THREAD_OBJ_FILE *	AssignToFreeThread(SOCKET_OBJ_FILE *sock);
	CString			GetCurrentTimeByString(BOOL bOnlyTime = TRUE);
	CSockObjMapInfoFile*	SocketConnect(UINT nSocketPort, char* szSocketAddress); 
	//CCriticalSectionLocalScope m_criSecLocalScope;

	void ProcessWhenConnClosed	(SOCKET_OBJ_FILE * pSockObj) ;
	BOOL InformFolderSelect		(SOCKET_OBJ_FILE * pSockObj); 
	BOOL DenyTransferFile		(SOCKET_OBJ_FILE* pSockObj );	
	BOOL CancelSendFile			(CSockObjMapInfoFile * pSpckObjMap);
	BOOL GetInformedFile		(SOCKET_OBJ_FILE * pSockObj, char* szFilePath) ;
	BOOL ProcessRecvData		(SOCKET_OBJ_FILE * pSockObj) ;		
	BOOL InformFileSend			(CSockObjMapInfoFile * pSpckObjMap, char* szToId, CString& strToIP, char* szFilePath, int nFileCnt);
	
};
	



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILETRANSSOCK_H__E614E97D_88F9_40B3_84AD_B2284598C8AE__INCLUDED_)
