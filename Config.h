#pragma once
#include "..\Public\ado.h"

class XCString  
{
public:
	XCString():begFlag(1),endFlag(2){}
	XCString(const CString& str):m_str(str),begFlag(1),endFlag(2)
	{}
	XCString(const char* str):m_str(str),begFlag(1),endFlag(2)
	{}
	CString& operator=(const CString& str)
	{
		return m_str = str;
	}
	CString& operator=(const char* str)
	{
		return m_str = str;
	}
	operator CString()
	{
		return m_str;
	}
	operator const char*()
	{
		return m_str.GetBuffer();
	}
	void Empty()
	{
		m_str.Empty();
	}
public:
	int begFlag ;
	CString m_str;
	int endFlag;
};

class CConfig
{
public:
	CConfig(void);
public:
	virtual ~CConfig(void);

private:
	CString m_strPath;
	bool    m_bUpdate;

public:
	void Reset(); 
	bool UpdateFile(void);
	bool LoadFile(CString strPath);
	void GetValue(const CString& strSection,const CString& strKey,int& nValue, int nDefault,const CString& strDes,bool bLine=false);
	void GetValue(const CString& strSection,const CString& strKey,CString& strValue,CString strDefault,const CString& strDes,bool bLine=false);
	bool LoadParameter(const CString& strPath);
	bool LoadDbConfig(const CString& pszDbAddr, const CString& pszDbName, const CString& pszDbUser, const CString& pszDbPassword, const CString& strAppName);
	void FullConfigInfo();
	//����ȡ��Ӧ�ó���ID��windows����������ִ���ļ��İ汾��Ϣ��������������IP���µ���[SYS_APPLICATION_INFO]�м�¼�������¼������������¼�¼
	bool SaveProgramInfoIntoDB(CADODatabase& adoPoll, int nId, const CString& strCurServiceName, const CString& strVersion, const CString& strHostAddr);
	void GetLocalHostIpAddr(CString& strHostAddr, const CString& strDBAddress, u_short port);
	void SetServiceName(CString strName)
	{
		m_ServiceName = strName;
	}

	/************�����ļ���Ա����**********************************/
	//���ݿ�����
	CString m_strDBAddrL;	//���ݿ��ַ����IP+PORT
	CString m_strDBNameL;	//���ݿ���
	CString m_strDBUserL;//���ݿ��û���
	CString m_strDBPwdL;//���ݿ�����

	//LTE��������
	CString m_strDBAddrC;	//���ݿ��ַ����IP+PORT
	CString m_strDBNameC;	//���ݿ���
	CString m_strDBUserC;//���ݿ��û���
	CString m_strDBPwdC;//���ݿ�����

	//GSM-ERIC��������
	CString m_strDBAddr2G;	//���ݿ��ַ����IP+PORT
	CString m_strDBName2G;	//���ݿ���
	CString m_strDBUser2G;//���ݿ��û���
	CString m_strDBPwd2G;//���ݿ�����

	//GSM-HW��������
	CString m_strDBAddrHW2G;	//���ݿ��ַ����IP+PORT
	CString m_strDBNameHW2G;	//���ݿ���
	CString m_strDBUserHW2G;//���ݿ��û���
	CString m_strDBPwdHW2G;//���ݿ�����

	//�������ݿ�
	CString m_strDBAddrE;	//���ݿ��ַ����IP+PORT
	CString m_strDBNameE;	//���ݿ���
	CString m_strDBUserE;//���ݿ��û���
	CString m_strDBPwdE;//���ݿ�����

	//�������ݿ�����
	CString m_strDBAddrM;	//���ݿ��ַ����IP+PORT
	CString m_strDBNameM;	//���ݿ���
	CString m_strDBUserM;//���ݿ��û���
	CString m_strDBPwdM;//���ݿ�����


	//ϵͳ����
	int		m_nLogSave;	//Log��־����ʱ�䣨��λ���죩
	int		m_nLogLevel; //0�ر���־  1����  2��Ϣ  3Debug
	int     m_nSTSCycle; //STS����
	int     m_nRecvTimeout;
	int     m_nEnter; //���������Ƿ���س�
	int     m_nThreadNum;
	int     m_nAdjustCycle;//��������
	int     m_bCentreConfig;//���������Ƿ���Ч
	int     m_nStartMin;//����ʱ��
	int		m_nAdjTime;//����ʱ��
	int		m_nSendCmdThreadNum;//�·�ָ���߳���
	int     m_nAdjEnbNum;//������վ�����

	CString m_nAdjustTime;//����ʱ��
	CString m_nStartTime;//����ʱ��

	CString m_strAppName;//Ӧ����
	int m_strDBID; //��վ���е�Ӧ��ID
	CString m_ServiceName; //������
};
