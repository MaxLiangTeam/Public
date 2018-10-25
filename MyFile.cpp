#include "StdAfx.h"
#include "MyFile.h"

CMyFile::CMyFile(void)
:m_pData(0),m_nLen(0)
{
}

CMyFile::~CMyFile(void)
{
	delete[] m_pData;
}

int CMyFile::Open(LPCTSTR lpstrPath,UINT nOpenFlag)
{
	int nRet(0);
	BOOL bOpen(FALSE);
	CFile file;

	//��ȡ�ļ�
	try
	{
		bOpen = file.Open(lpstrPath,CFile::modeRead | nOpenFlag);
		if (!bOpen)
		{
			return -1;
		}
		UINT nLen = (UINT)file.GetLength();
		m_pData = new char[nLen + 1];
		memset(m_pData,0,nLen + 1);
		m_nLen = file.Read(m_pData,nLen);
		if (nLen != m_nLen)
		{
			nRet = -2;
		}
	}
	catch (...)
	{	
		nRet = -3;
	}

	//�ر��ļ�
	if (bOpen)
	{
		try
		{
			file.Close();
		}
		catch (...)
		{
		}
	}

	return nRet;
}

//0�ɹ� 1��ʧ�� -1��������
int CMyFile::Write(LPCTSTR lpstrData,UINT nLen)
{
	int nRet(0);
	BOOL bOpen(FALSE);
	CFile file;

	try
	{
		bOpen = file.Open(m_strPath,CFile::modeWrite);
		if (!bOpen)
		{
			return -1;
		}

		if (m_pData != NULL && m_nLen > 0)
		{
			file.Write(m_pData,m_nLen);
		}
		if (lpstrData != NULL && nLen > 0)
		{
			file.Write(lpstrData,nLen);
		}
	}
	catch (...)
	{	
		nRet = -3;
	}

	//�ر��ļ�
	if (bOpen)
	{
		try
		{
			file.Close();
		}
		catch (...)
		{
			nRet = -3;
		}
	}

	return nRet;
}


//0�ɹ� 1��ʧ�� -1��������
int CMyFile::Write(LPCTSTR lpstrPath,UINT nOpenFlag,LPCTSTR lpstrData,UINT nLen)
{
	int nRet(0);
	BOOL bOpen(FALSE);
	CFile file;

	try
	{
		bOpen = file.Open(lpstrPath,CFile::modeWrite | nOpenFlag);
		if (!bOpen)
		{
			return -1;
		}

		if (lpstrData != NULL && nLen > 0)
		{
			file.Write(lpstrData,nLen);
		}
	}
	catch (...)
	{	
		nRet = -3;
	}

	//�ر��ļ�
	if (bOpen)
	{
		try
		{
			file.Close();
		}
		catch (...)
		{
			nRet = -3;
		}
	}

	return nRet;
}

int CMyFile::Remove(LPCTSTR lpstrData)
{
	try
	{
		CFile::Remove(lpstrData);
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}