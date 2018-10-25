/*---------------------------------------------------------------------------
描述    : FTP下载类
作者    : wsl
版本    : 
时间    : 2013-04-25
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "Ftp.h"

CFtp::CFtp(void)
:m_hInetSession(NULL),m_hFtpConn(NULL)
{
}

CFtp::~CFtp(void)
{
	Logout();
}

bool CFtp::Login(const char* szIP,const WORD uPort,const char* szUser,const char* szPWD)
{
	m_hInetSession = InternetOpen("session",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
	if (m_hInetSession == NULL)
	{
		m_strError.Format("InternetOpen failure!");
		return false;
	}
	m_hFtpConn = InternetConnect(m_hInetSession,szIP,uPort,szUser,szPWD,INTERNET_SERVICE_FTP,INTERNET_FLAG_PASSIVE,0);
	if (m_hFtpConn == NULL)
	{
		m_strError.Format("InternetConnect failure!");
		return false;
	}
	return true;
}

void CFtp::Logout(void)
{
	if (m_hFtpConn != NULL)
	{
		InternetCloseHandle(m_hFtpConn);
		m_hFtpConn = NULL;
	}
	if (m_hInetSession != NULL)
	{
		InternetCloseHandle(m_hInetSession);
		m_hInetSession = NULL;
	}
}

bool CFtp::SetCurDir(const char* szCurDir)
{
	if(!FtpSetCurrentDirectory(m_hFtpConn,szCurDir))
	{
		m_strError.Format("FtpSetCurrentDirectory [%s] failure!",szCurDir);
		return false;
	}
	return true;
}

bool CFtp::FindFile(const char* szFtpFile)
{
	bool bFind = false;
	WIN32_FIND_DATA findData;
	HINTERNET hFind = FtpFindFirstFile(m_hFtpConn,_T("*"),&findData,0,0);

	if (!hFind)
	{
		m_strError.Format("FtpFindFirstFile failure!");
		if (GetLastError()  == ERROR_NO_MORE_FILES)
		{
			m_strError += "maybe directory is none!";
		}
		return bFind;
	}
	do
	{
		if(findData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			CString strFile = findData.cFileName;
			if(strFile.Find(szFtpFile) != -1)
			{
				bFind = true;
				break;
			}
		}
	}while(InternetFindNextFile(hFind,&findData));
	InternetCloseHandle(hFind);

	if (!bFind)
	{
		m_strError.Format("Not find [%s]!",szFtpFile);
		return false;
	}

	return true;
}

bool CFtp::DownloadFile(const char* szFtpFile,const char* szLocalDir)
{
	//查找文件
	if (!FindFile(szFtpFile))
	{
		return false;
	}
	
	//开始下载文件
	CString strLocalFile;
	strLocalFile.Format("%s\\%s",szLocalDir,szFtpFile);
	if(!FtpGetFile(m_hFtpConn,szFtpFile,strLocalFile,FALSE,FILE_ATTRIBUTE_NORMAL,FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_NO_CACHE_WRITE,0))
	{
		m_strError.Format("Dowdload file [%s] to [%s] failure!",szFtpFile,szLocalDir);
		return false;
	}
	return true;
}
