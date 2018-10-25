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
	//将获取的应用程序ID、windows服务名、可执行文件的版本信息、本机服务器的IP更新到表[SYS_APPLICATION_INFO]中记录，如果记录不存在则插入新记录
	bool SaveProgramInfoIntoDB(CADODatabase& adoPoll, int nId, const CString& strCurServiceName, const CString& strVersion, const CString& strHostAddr);
	void GetLocalHostIpAddr(CString& strHostAddr, const CString& strDBAddress, u_short port);
	void SetServiceName(CString strName)
	{
		m_ServiceName = strName;
	}

	/************配置文件成员变量**********************************/
	//数据库配置
	CString m_strDBAddrL;	//数据库地址，即IP+PORT
	CString m_strDBNameL;	//数据库名
	CString m_strDBUserL;//数据库用户名
	CString m_strDBPwdL;//数据库密码

	//LTE数据中心
	CString m_strDBAddrC;	//数据库地址，即IP+PORT
	CString m_strDBNameC;	//数据库名
	CString m_strDBUserC;//数据库用户名
	CString m_strDBPwdC;//数据库密码

	//GSM-ERIC数据中心
	CString m_strDBAddr2G;	//数据库地址，即IP+PORT
	CString m_strDBName2G;	//数据库名
	CString m_strDBUser2G;//数据库用户名
	CString m_strDBPwd2G;//数据库密码

	//GSM-HW数据中心
	CString m_strDBAddrHW2G;	//数据库地址，即IP+PORT
	CString m_strDBNameHW2G;	//数据库名
	CString m_strDBUserHW2G;//数据库用户名
	CString m_strDBPwdHW2G;//数据库密码

	//易优数据库
	CString m_strDBAddrE;	//数据库地址，即IP+PORT
	CString m_strDBNameE;	//数据库名
	CString m_strDBUserE;//数据库用户名
	CString m_strDBPwdE;//数据库密码

	//易优数据库主库
	CString m_strDBAddrM;	//数据库地址，即IP+PORT
	CString m_strDBNameM;	//数据库名
	CString m_strDBUserM;//数据库用户名
	CString m_strDBPwdM;//数据库密码


	//系统配置
	int		m_nLogSave;	//Log日志保存时间（单位：天）
	int		m_nLogLevel; //0关闭日志  1错误  2消息  3Debug
	int     m_nSTSCycle; //STS周期
	int     m_nRecvTimeout;
	int     m_nEnter; //发送命令是否带回车
	int     m_nThreadNum;
	int     m_nAdjustCycle;//调整周期
	int     m_bCentreConfig;//集中配置是否生效
	int     m_nStartMin;//启动时间
	int		m_nAdjTime;//调整时间
	int		m_nSendCmdThreadNum;//下发指令线程数
	int     m_nAdjEnbNum;//调整的站最大数

	CString m_nAdjustTime;//启动时间
	CString m_nStartTime;//调整时间

	CString m_strAppName;//应用名
	int m_strDBID; //主站库中的应用ID
	CString m_ServiceName; //服务名
};
