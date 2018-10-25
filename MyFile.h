#pragma once

class CMyFile
{
public:
	CMyFile(void);
public:
	virtual ~CMyFile(void);

	int Open(LPCTSTR lpstrPath,UINT nOpenFlag);
	int Write(LPCTSTR lpstrData,UINT nLen);
	int Write(LPCTSTR lpstrPath,UINT nOpenFlag,LPCTSTR lpstrData,UINT nLen);
	static int Remove(LPCTSTR lpstrPath);

	char* str() {return m_pData;}
	UINT  len() {return m_nLen;}
private:
	CString m_strPath;
	char* m_pData;
	UINT  m_nLen;
};
