#pragma once
#include <afxinet.h>

class CXFtp
{
public:

	//初始化CInternetSession，设置等待时间，单位:分钟
	CXFtp(int nWaitTime = 2);

	~CXFtp(void);

public:

	//登录到ftp
	BOOL Login(CString strFtpAddress,CString strUser,CString strPassword,int nPort = 21,BOOL bPassive=FALSE);

	//登出ftp
	void Logout();

	//设置ftp的目录
	BOOL SetFtpDirectory(LPCTSTR pstrRemoteDirectory);

	//模糊匹配下载名字中包含strFileName的文件到某个目录
	BOOL DownFile(CString strFileName,CString strLocalDirectory,int nWaiteMin);

	//删除Ftp上的文件
	BOOL RemoveFile(CString strFileName);

	bool UpLoadFile(const CString &strSrcFile,const CString &strDestFileName);
private:
	//下载远程文件,本地路径必须存在
	BOOL DownLoadFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile);

	//查找文件,模糊匹配
	bool FindFtpFile(CString strFileName);

private:
	//CInternetSession连接
	CInternetSession *m_pInetsession;

	//CFtpConnection 连接
	CFtpConnection *m_pFtpConnection;

	//端口
	unsigned int m_nPort;

	//地址
	CString m_strAddress;

	//用户名
	CString m_strUserName;

	//密码
	CString m_strPassword;

	//开始查找时间
	CTime m_tTime;

	//ftp等待生成文件的时间,分钟
	int m_nWaitTime;

public:
	//远程文件名
	CString m_strRemoteFileName;

	//下载回来的文件完整路径
	CString m_strLocalFilePath;

	//当前ftp路径
	CString m_strCurrentPath;

	//日志文件
	CString m_strLogFile;
};
