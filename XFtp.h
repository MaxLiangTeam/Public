#pragma once
#include <afxinet.h>

class CXFtp
{
public:

	//��ʼ��CInternetSession�����õȴ�ʱ�䣬��λ:����
	CXFtp(int nWaitTime = 2);

	~CXFtp(void);

public:

	//��¼��ftp
	BOOL Login(CString strFtpAddress,CString strUser,CString strPassword,int nPort = 21,BOOL bPassive=FALSE);

	//�ǳ�ftp
	void Logout();

	//����ftp��Ŀ¼
	BOOL SetFtpDirectory(LPCTSTR pstrRemoteDirectory);

	//ģ��ƥ�����������а���strFileName���ļ���ĳ��Ŀ¼
	BOOL DownFile(CString strFileName,CString strLocalDirectory,int nWaiteMin);

	//ɾ��Ftp�ϵ��ļ�
	BOOL RemoveFile(CString strFileName);

	bool UpLoadFile(const CString &strSrcFile,const CString &strDestFileName);
private:
	//����Զ���ļ�,����·���������
	BOOL DownLoadFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile);

	//�����ļ�,ģ��ƥ��
	bool FindFtpFile(CString strFileName);

private:
	//CInternetSession����
	CInternetSession *m_pInetsession;

	//CFtpConnection ����
	CFtpConnection *m_pFtpConnection;

	//�˿�
	unsigned int m_nPort;

	//��ַ
	CString m_strAddress;

	//�û���
	CString m_strUserName;

	//����
	CString m_strPassword;

	//��ʼ����ʱ��
	CTime m_tTime;

	//ftp�ȴ������ļ���ʱ��,����
	int m_nWaitTime;

public:
	//Զ���ļ���
	CString m_strRemoteFileName;

	//���ػ������ļ�����·��
	CString m_strLocalFilePath;

	//��ǰftp·��
	CString m_strCurrentPath;

	//��־�ļ�
	CString m_strLogFile;
};
