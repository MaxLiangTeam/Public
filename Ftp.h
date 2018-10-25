#pragma once

#include <wininet.h>
//#include <afxinet.h>

class CFtp
{
public:
	CFtp(void);
public:
	virtual ~CFtp(void);

	bool Login(const char* szIP,const WORD uPort,const char* szUser,const char* szPWD);
	void Logout(void);
	bool SetCurDir(const char* szCurDir);
	bool FindFile(const char* szFtpFile);
	bool DownloadFile(const char* szFtpFile,const char* szLocalDir);
	//bool UploadFile(const char* szSrcFile,const char* szDestFile);
	
	CString GetLastError(void) {return m_strError;}

private:
	HINTERNET m_hInetSession;
	HINTERNET m_hFtpConn;
	CString m_strError;
	//CString m_strIP;
	//CString m_strUser;
	//CString m_strPWD;
	//WORD    m_uPort;
};
