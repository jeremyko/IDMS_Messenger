// GSEncryptX.h: interface for the GSEncryptX
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GSENCRYPTX_H__7F6A9700_F55F_489C_8B8E_861477C7A3D3__INCLUDED_)
#define AFX_GSENCRYPTX_H__7F6A9700_F55F_489C_8B8E_861477C7A3D3__INCLUDED_

#ifdef GSENCRYPTX_EXPORTS
#define GSENCRYPTX_API __declspec(dllexport)
#else
#define GSENCRYPTX_API __declspec(dllimport)
#endif

// Err MACRO
#define GSENCRYPTX_SUCCESS          0
#define GSENCRYPTX_SYSTEMERR		-100
#define GSENCRYPTX_APPERR			-200

// DLL Function
GSENCRYPTX_API CString GSEncryptX_GetLastError();
GSENCRYPTX_API int GSEncryptX_EncryptDes(PCHAR szPassWord, LPBYTE szSrc, int nSize, LPBYTE* pszDest, int* pSize);
GSENCRYPTX_API int GSEncryptX_DecryptDes(PCHAR szPassWord, LPBYTE szSrc, int nSize, LPBYTE* pszDest, int* pSize);

#endif // AFX_GSENCRYPTX_H__7F6A9700_F55F_489C_8B8E_861477C7A3D3__INCLUDED_