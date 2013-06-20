// Dec.cpp: implementation of the CDec class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DesWrapper.h"

#define KEY					_T("IdmsSicc")
#define KEY_SIZE			8

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDesWrapper::CDesWrapper()
{
	m_strKey		= _T("");
	m_strLastError	= _T("");
}

CDesWrapper::~CDesWrapper()
{

}

void CDesWrapper::SetKey(CString strKey)
{
	while( strKey.GetLength() < 8 )
		strKey += " ";

	m_strKey = strKey.Left(8);
}

CString CDesWrapper::GetKey()
{
	return m_strKey;
}

CString CDesWrapper::GetEncrypt(CString strData)
{
	int i = 0;
	CString	strEncrypt;
	TCHAR	tcKey[KEY_SIZE + 1];
	int		nOrgSize = 0;
	BYTE*	pEncByte = NULL;
	int		nEncSize = 0;
	char*	pHexa = NULL;
	int		nHexaSize = 0;

	// Key 생성
	memset(tcKey, '\0', KEY_SIZE + 1);
	if( m_strKey != _T("") )
	{
		BYTE*	pKeyByte;
		int		nKeySize;
		char*	pHexData;
		int		nHexSize;

		for( i = 0; i < m_strKey.GetLength(); i++ )
		{
			if( i > KEY_SIZE )		break;
			tcKey[i] = m_strKey.GetAt(i);
		}

		pKeyByte = NULL;
		if( GSEncryptX_EncryptDes(KEY, (LPBYTE)tcKey, KEY_SIZE, (LPBYTE*)&pKeyByte, &nKeySize) != GSENCRYPTX_SUCCESS )
		{
			//m_strLastError = GSEncryptX_GetLastError();
			return _T("");
		}

		pHexData = NULL;
		ConvertByteToHexa(pKeyByte, nKeySize, pHexData, nHexSize);
		for( i = 0; i < nHexSize; i++ )
		{
			if( (i % 2) == 1 )
				continue;
			tcKey[i / 2] = pHexData[i];
		}
		delete[] pKeyByte;
		delete[] pHexData;
	}
	else
		memcpy(tcKey, KEY, KEY_SIZE);

	nOrgSize = strData.GetLength();

	if( GSEncryptX_EncryptDes(tcKey, (LPBYTE)(LPCTSTR)strData, nOrgSize, (LPBYTE*)&pEncByte, &nEncSize) != GSENCRYPTX_SUCCESS )
	{
		//m_strLastError = GSEncryptX_GetLastError();
		return _T("");
	}

	pHexa = NULL;
	ConvertByteToHexa(pEncByte, nEncSize, pHexa, nHexaSize);
	if( pHexa == NULL || nHexaSize == 0 )
		return _T("");

	strEncrypt.Format("%s", pHexa);
	delete[] pHexa;
	delete[] pEncByte;

	return strEncrypt;
}

void CDesWrapper::GetDecrypt(CString strData, CString& strDecrypt)
{	
	int i = 0;
	TCHAR	tcKey[KEY_SIZE + 1];
	BYTE*	pDecByte = NULL;
	int		nDecSize = 0;
	BYTE*	pByte = NULL;
	int		nByteSize = 0;

	// Key 생성
	memset(tcKey, '\0', KEY_SIZE + 1);
	if( m_strKey != _T("") )
	{
		BYTE*	pKeyByte;
		int		nKeySize;
		char*	pHexData;
		int		nHexSize;

		for(  i = 0; i < m_strKey.GetLength(); i++ )
		{
			if( i > KEY_SIZE )		break;
			tcKey[i] = m_strKey.GetAt(i);
		}

		pKeyByte = NULL;
		if( GSEncryptX_EncryptDes(KEY, (LPBYTE)tcKey, KEY_SIZE, (LPBYTE*)&pKeyByte, &nKeySize) != GSENCRYPTX_SUCCESS )
		{
			//m_strLastError = GSEncryptX_GetLastError();
			//return _T("");
			return;
		}

		pHexData = NULL;
		ConvertByteToHexa(pKeyByte, nKeySize, pHexData, nHexSize);
		for( i = 0; i < nHexSize; i++ )
		{
			if( (i % 2) == 1 )
				continue;
			tcKey[i / 2] = pHexData[i];
		}
		
		delete[] pKeyByte;
		delete[] pHexData;
	}
	else
		memcpy(tcKey, KEY, KEY_SIZE);

	pByte = NULL;
	ConvertHexaToByte((LPSTR)(LPCSTR)strData, strData.GetLength(), pByte, nByteSize);
	if( pByte == NULL || nByteSize == 0 )
		//return _T("");
		return;

	if( GSEncryptX_DecryptDes(tcKey, (LPBYTE)pByte, nByteSize, (LPBYTE*)&pDecByte, &nDecSize) != GSENCRYPTX_SUCCESS )
	{
		//m_strLastError = GSEncryptX_GetLastError();
		//return _T("");
		return;
	}

	strDecrypt.Format("%s", pDecByte);
	delete[] pByte;
	delete[] pDecByte;	
}

void CDesWrapper::ConvertByteToHexa(BYTE *pByte, int nByteSize, char*& szHexa, int& nHexaSize)
{
	BYTE byTemp;

	nHexaSize = 0;
	if( szHexa != NULL )
		delete[] szHexa;

	if( pByte == NULL )
	{
		m_strLastError = _T("변환될 데이터의 메모리가 할당되지 않았습니다.");
		return;
	}
	if( nByteSize <= 0 )
	{
		m_strLastError = _T("변환될 데이터의 크기가 지정되지 않았습니다.");
		return;
	}

	szHexa = new char[nByteSize * 2 + 1];
	memset(szHexa, '\0', nByteSize * 2 + 1);

	for( int i = 0; i < nByteSize; i++ )
	{
		byTemp = pByte[i] >> 4;
		szHexa[nHexaSize++] = ConvertByteToHexa(byTemp);

		byTemp = pByte[i] << 4;
		byTemp = byTemp >> 4;
		szHexa[nHexaSize++] = ConvertByteToHexa(byTemp);
	}
}

void CDesWrapper::ConvertHexaToByte(char *szHexa, int nHexaSize, BYTE*& pByte, int& nByteSize)
{
	nByteSize = 0;
	if( pByte != NULL )
		delete[] pByte;

	if( szHexa == NULL )
	{
		m_strLastError = _T("변환될 데이터의 메모리가 할당되지 않았습니다.");
		return;
	}
	if( nHexaSize <= 0 )
	{
		m_strLastError = _T("변환될 데이터의 크기가 지정되지 않았습니다.");
		return;
	}
	if( nHexaSize % 2 )
	{
		m_strLastError = _T("변환될 데이터의 크기가 잘못되었습니다");
		return;
	}

	pByte = new BYTE[nHexaSize / 2];
	memset(pByte, NULL, nHexaSize / 2);

	for( int i = 0; i < nHexaSize; i+=2 )
	{
		pByte[nByteSize++] = (ConvertHexaToByte(szHexa[i]) << 4) | ConvertHexaToByte(szHexa[i+1]);
	}
}

CString CDesWrapper::GetLastError()
{
	return m_strLastError;
}

BYTE CDesWrapper::ConvertByteToHexa(BYTE byData)
{
	if( byData <= 9 )
		return (byData + '0');

	return (byData + 'A' - 10);
}

BYTE CDesWrapper::ConvertHexaToByte(char cData)
{
	if( cData <= '9' )
		return (cData - '0');

	return (cData - 'A' + 10);
}

//DEL void CDesWrapper::ExclusiveOr(BYTE *pInputData, BYTE *pExcusiveData, int nSize, BYTE *&pOutputData)
//DEL {
//DEL 	if( pInputData == NULL || pExcusiveData == NULL || nSize <= 0 )
//DEL 		return;
//DEL 
//DEL 	if( pOutputData != NULL )
//DEL 		delete[] pOutputData;
//DEL 
//DEL 	pOutputData = new BYTE[nSize];
//DEL 
//DEL 	for( int i = 0; i < nSize; i++ )
//DEL 		pOutputData[i] = pInputData[i] ^ pExcusiveData[i];
//DEL }
