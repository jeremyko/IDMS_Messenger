
#if !defined(AFX_CDESWRAPPER_H__6E8F509D_19BF_4C67_99B5_7E383F40FD77__INCLUDED_)
#define AFX_CDESWRAPPER_H__6E8F509D_19BF_4C67_99B5_7E383F40FD77__INCLUDED_

class CDesWrapper : CObject  
{
public:
	CString GetLastError();
	CString GetEncrypt(CString strData);
	void    GetDecrypt(CString strData, CString& strDecrypt);
	CString GetKey();
	void	SetKey(CString strKey);
	CDesWrapper();
	virtual ~CDesWrapper();

protected:
	BYTE	ConvertHexaToByte(char cData);
	BYTE	ConvertByteToHexa(BYTE byData);
	void	ConvertHexaToByte(char* szHexa, int nHexaSize, BYTE*& pByte, int& nByteSize);
	void	ConvertByteToHexa(BYTE* pByte, int nByteSize, char*& szHexa, int& nHexaSize);
	CString m_strKey;
	CString m_strLastError;
};

#endif // !defined(AFX_TEST_H__6E8F509D_19BF_4C67_99B5_7E383F40FD77__INCLUDED_)
