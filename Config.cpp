/*---------------------------------------------------------------------------
文件    : 
描述    : 华为载波自动调整系统配置文件
作者    : wsl
版本    : V1.0
时间    : 2012-02-22
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "Config.h"
#include <exception>
#include "AppParameter.h"
#include "Include.h"

CConfig::CConfig(void)
:m_bUpdate(false),m_bCentreConfig(1),m_nAdjustCycle(60),m_strDBID(0)
{
}

CConfig::~CConfig(void)
{
}

void CConfig::Reset()
{
	m_strDBAddrL.Empty();
	m_strDBNameL.Empty();
	m_strDBUserL.Empty();
	m_strDBPwdL.Empty();
	m_strDBAddrC.Empty();
	m_strDBNameC.Empty();
	m_strDBUserC.Empty();
	m_strDBPwdC.Empty();
	m_strDBAddr2G.Empty();
	m_strDBName2G.Empty();
	m_strDBUser2G.Empty();
	m_strDBPwd2G.Empty();
	m_strDBAddrHW2G.Empty();
	m_strDBNameHW2G.Empty();
	m_strDBUserHW2G.Empty();
	m_strDBPwdHW2G.Empty();
	m_strDBAddrE.Empty();
	m_strDBNameE.Empty();
	m_strDBUserE.Empty();
	m_strDBPwdE.Empty();
	m_strAppName.Empty();

	m_nSendCmdThreadNum = 1;
	m_nAdjEnbNum = 10000;
	m_nLogSave = m_nLogLevel = m_nSTSCycle = m_nRecvTimeout = m_nEnter = m_nThreadNum = 0;
	m_nStartMin = 55; m_nAdjTime = 10;
	m_nAdjustCycle = 60;
}

void CConfig::GetValue(const CString& strSection,const CString& strKey,int& nValue,int nDefault,const CString& strDes,bool bLine)
{
	//用读内存值更新默认值
	if (m_bUpdate)
	{
		nDefault = nValue;
	}

	nValue = GetPrivateProfileInt(strSection, strKey, -1, m_strPath);
	if (-1 == nValue)
	{
		nValue = nDefault;
		CString strValue;
		strValue.Format("%d%s",nValue, bLine ? "\r\n" : "");
		WritePrivateProfileString(strSection, strKey, strValue, m_strPath);

		//write description
		if (!strDes.IsEmpty())
		{
			CFile mFile;

			CString strData;								//文件内容
			CString strAppTemp = "[" + strSection + "]";	//找APP位置
			CString strKeyTemp = strKey + "=";			//找Key位置
			CString strDescrition = "#" + strDes + "\r\n";	//描述的字符串
			CString strIsExist = strDes + strKeyTemp;		//找描述字符串的位置

			int nStart(0),nEnd(0);
			if(0 != mFile.Open(m_strPath,CFile::modeReadWrite))
			{
				int nLen = (int)mFile.GetLength();//文件大小
				char *pBuff = (char *)malloc(nLen+1);
				ZeroMemory(pBuff,nLen+1);

				mFile.Read(pBuff,nLen);//读文件
				strData = pBuff;
				free(pBuff);

				nStart = strData.Find(strIsExist);
				if (-1 == nStart)//判断注释是不是已经存在,不存在才写
				{
					nStart = strData.Find(strAppTemp);
					if (-1 != nStart)
					{
						nStart = strData.Find(strKeyTemp,nStart);
						if (-1 != nStart)
						{
							strData.Insert(nStart,strDescrition);
							mFile.SeekToBegin();
							mFile.Write(strData,strData.GetLength());
						}
					}	
				}		
				mFile.Close();
				mFile.Flush();
			}
		}
	}
}

void CConfig::GetValue(const CString& strSection,const CString& strKey,CString& strValue,CString strDefault,const CString& strDes,bool bLine)
{
	//用读内存值更新默认值
	if (m_bUpdate)
	{
		strDefault = strValue;
	}

	CString strTemp;
	DWORD dw = GetPrivateProfileString(strSection, strKey, "", strTemp.GetBuffer(100), 100, m_strPath);
	strTemp.ReleaseBuffer();

	if ("" == strTemp)
	{
		strTemp = strDefault + (bLine ? "\r\n" : "");
		WritePrivateProfileString(strSection, strKey, strTemp, m_strPath);
		strTemp = strDefault;

		//write description
		if (!strDes.IsEmpty())
		{
			CFile mFile;

			CString strData;								//文件内容
			CString strAppTemp = "[" + strSection + "]";	//找APP位置
			CString strKeyTemp = strKey + "=";			//找Key位置
			CString strDescrition = "#" + strDes + "\r\n";	//描述的字符串
			CString strIsExist = strDescrition + strKeyTemp;		//找描述字符串的位置

			int nStart(0),nEnd(0);
			if(0 != mFile.Open(m_strPath,CFile::modeReadWrite))
			{
				int nLen = (int)mFile.GetLength();//文件大小
				char *pBuff = (char *)malloc(nLen+1);
				ZeroMemory(pBuff,nLen+1);

				mFile.Read(pBuff,nLen);//读文件
				strData = pBuff;
				free(pBuff);

				nStart = strData.Find(strIsExist);
				if (-1 == nStart)//判断注释是不是已经存在,不存在才写
				{
					nStart = strData.Find(strAppTemp);
					if (-1 != nStart)
					{
						nStart = strData.Find(strKeyTemp,nStart);
						if (-1 != nStart)
						{
							strData.Insert(nStart,strDescrition);
							mFile.SeekToBegin();
							mFile.Write(strData,strData.GetLength());
						}
					}	
				}		
				mFile.Close();
				mFile.Flush();
			}
		}
	}
	strValue = strTemp;
}

bool CConfig::UpdateFile()
{
	try
	{
		//删除旧的文件
		CFile::Remove(m_strPath);

		//更新配置文件
		m_bUpdate = true;

		//重新加载文件
		return LoadFile(m_strPath);
	}
	catch (...)
	{
	}
	return false;
}

//配置文件INI
bool CConfig::LoadFile(CString strPath)
{
	try
	{
		//配置文件名
		m_strPath = strPath;
	
		CFile file;
		if(!file.Open(m_strPath,CFile::modeRead))
		{		
			if(!file.Open(m_strPath,CFile::modeCreate|CFile::modeWrite))
			{
				return false;
			}
		}
		file.Close();

		//本地数据库---------------------------------------------------------------------------------------------------
		GetValue("iSON-LOCAL", "DBIP",m_strDBAddrL,"127.0.0.1","本地库IP地址");
		GetValue("iSON-LOCAL", "DBName",m_strDBNameL,"iSON_LOCAL","本地库名称");
		GetValue("iSON-LOCAL", "DBUser",m_strDBUserL,"sa","本地库登录用户");
		GetValue("iSON-LOCAL", "DBPassword",m_strDBPwdL,"123456","本地库用登录密码",true);

		//数据中心
		GetValue("iSON-DC", "DBIP",m_strDBAddrC,"127.0.0.1","数据中心库IP地址");
		GetValue("iSON-DC", "DBName",m_strDBNameC,"iSON_DC","数据中心库名称");
		GetValue("iSON-DC", "DBUser",m_strDBUserC,"sa","数据中心库登录用户");
		GetValue("iSON-DC", "DBPassword",m_strDBPwdC,"123456","数据中心库用登录密码",true);

		//2GERIC数据中心
		GetValue("iSON-2GERIC-DC", "DBIP",m_strDBAddr2G,"127.0.0.1","2G数据中心库IP地址");
		GetValue("iSON-2GERIC-DC", "DBName",m_strDBName2G,"iSON_DC","2G数据中心库名称");
		GetValue("iSON-2GERIC-DC", "DBUser",m_strDBUser2G,"sa","2G数据中心库登录用户");
		GetValue("iSON-2GERIC-DC", "DBPassword",m_strDBPwd2G,"123456","2G数据中心库用登录密码",true);

		//2GHW数据中心
		GetValue("iSON-2GHW-DC", "DBIP",m_strDBAddrHW2G,"127.0.0.1","2GHW数据中心库IP地址");
		GetValue("iSON-2GHW-DC", "DBName",m_strDBNameHW2G,"iSON_DC","2GHW数据中心库名称");
		GetValue("iSON-2GHW-DC", "DBUser",m_strDBUserHW2G,"sa","2GHW数据中心库登录用户");
		GetValue("iSON-2GHW-DC", "DBPassword",m_strDBPwdHW2G,"123456","2GHW数据中心库用登录密码",true);

		//易优数据库
		GetValue("iSON-EOES", "DBIP",m_strDBAddrE,"127.0.0.1","易优数据库IP地址");
		GetValue("iSON-EOES", "DBName",m_strDBNameE,"EOES_DATA","易优数据库名称");
		GetValue("iSON-EOES", "DBUser",m_strDBUserE,"sa","易优数据库登录用户");
		GetValue("iSON-EOES", "DBPassword",m_strDBPwdE,"123456","易优数据库用登录密码",true);

		//易优数据库主库
		GetValue("iSON-EOES-M", "DBIP",m_strDBAddrM,"127.0.0.1","易优数据库主库IP地址");
		GetValue("iSON-EOES-M", "DBName",m_strDBNameM,"EOES_MASTER","易优数据库主库名称");
		GetValue("iSON-EOES-M", "DBUser",m_strDBUserM,"sa","易优数据库主库登录用户");
		GetValue("iSON-EOES-M", "DBPassword",m_strDBPwdM,"123456","易优数据库主库用登录密码",true);


		//系统-------------------------------------------------------------------------------------------------
		GetValue("System", "STSCycle", m_nSTSCycle,15, "统计生成周期(默认15分)");
		GetValue("System", "RecvTimeout", m_nRecvTimeout,30, "TELNET接收超时(单位:秒)");
		GetValue("System", "Enter", m_nEnter,1, "#TELNET发送命令是否加回车(0－不加 1-加回车)");
		GetValue("System", "ThreadNum", m_nThreadNum,50, "线程池启动的线程数(默认50)");
		GetValue("System", "StartMin", m_nStartMin,15, "启动分钟(单位:分)");
		GetValue("System", "AdjTime", m_nAdjTime,50, "调整时间(默认50)");
		GetValue("System", "TaskStartTime", m_nStartTime, "02:20", "多定漏定方案启动时间(默认02:20,格式:两位时:两位分)");
		GetValue("System", "TaskAdjust", m_nAdjustTime, "02:50", "多定漏定方案调整时间(默认02:50,格式:两位时:两位分)");
		GetValue("System", "CentreConfig", m_bCentreConfig,1, "集中配置开关(0-关 1开)");
		GetValue("System", "AdjEnbNum", m_nAdjEnbNum,10000, "调整基站最大数(默认:10000)");
		GetValue("System", "SendCmdThreadNum", m_nSendCmdThreadNum,1, "下发指令线程数(默认1)",true);

		//日志----------------------------------------------------------------------------------------------------
		GetValue("Log", "LogSave",m_nLogSave,7,"日志保存天数");
		GetValue("Log", "LogLevel",m_nLogLevel,3,"日志等级(0-关 1-错误 2-信息 3-调试)",true);
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool CConfig::LoadParameter(const CString& strPath)
{

	m_strPath = strPath;
	//CString m_strPath = GetModulePath() + "iSON_TRX_HW_GSM_PY.ini";
	CFile file;
	if(!file.Open(m_strPath,CFile::modeRead))
	{		
		if(!file.Open(m_strPath,CFile::modeCreate|CFile::modeWrite))
		{
			throw std::exception("can't open config file");
		}
	}
	file.Close();

	CString strDbAdder,strDbName,strDbUser,strDbPassword;
	GetValue("main_config", "DBHost",strDbAdder,"127.0.0.1","main config database IP");
	GetValue("main_config", "DBName",strDbName,"iSON_main_config","main config database name");
	GetValue("main_config", "DBUserName",strDbUser,"sa","main config database user name");
	GetValue("main_config", "DBPassword",strDbPassword,"123456","main config Database Password");
	GetValue("main_config", "ApplicationName",m_strAppName,"ApplicationName","application name");
	if (!LoadDbConfig(strDbAdder, strDbName, strDbUser, strDbPassword, m_strAppName))
	{
		return false;
	}

	if (!CAppParameter::GetInstance()->LoadParameter(m_strDBAddrL.GetBuffer(), m_strDBNameL.GetBuffer(),
		m_strDBUserL.GetBuffer(), m_strDBPwdL.GetBuffer(), "SYS_HW2G_PARAMETER"))
	{
		return false;
	}
	FullConfigInfo();
	return true;
}

bool CConfig::LoadDbConfig(const CString& pszDbAddr, const CString& pszDbName, const CString& pszDbUser, const CString& pszDbPassword, const CString& strAppName)
{
	//连接本地数据库
	CString strLogs;
	CADODatabase mainConfig;
	mainConfig.SetTimeout(10);
	mainConfig.SetMax(1);
	if(!mainConfig.Init(pszDbAddr, pszDbName, pszDbUser, pszDbPassword))
	{
		throw std::exception("Init main_config db faild");
	}
	CString strSql;
	strSql.Format("select a.* from SYS_DATABASE_BasicInfo a, SYS_DATABASE_Relate b "
		" where a.ApplicationName = b.MatchApplicationName and b.ApplicationName = '%s' "
		" union select * from SYS_DATABASE_BasicInfo where ApplicationName = '%s'", strAppName, strAppName);

	int nInitFlag = 0;
	CADORecordset Rs(&mainConfig);
	if(Rs.Open(strSql.GetBuffer()))
	{
		while (!Rs.IsEOF())
		{	

			CString strDbType;
			Rs.GetFieldValue("ApplicationType",strDbType);
			if ("PARAM_ADJ" == strDbType)
			{
				Rs.GetFieldValue("IP",m_strDBAddrL);
				Rs.GetFieldValue("DataBaseName",m_strDBNameL);
				Rs.GetFieldValue("UserName",m_strDBUserL);
				Rs.GetFieldValue("Pwd",m_strDBPwdL);
				Rs.GetFieldValue("ID",m_strDBID);
				nInitFlag |= 0x1;
			}
			else if ("DC" == strDbType)
			{
				Rs.GetFieldValue("IP",m_strDBAddrC);
				Rs.GetFieldValue("DataBaseName",m_strDBNameC);
				Rs.GetFieldValue("UserName",m_strDBUserC);
				Rs.GetFieldValue("Pwd",m_strDBPwdC);
				nInitFlag |= 0x2;
			}
			else if ("EOES" == strDbType)
			{
				Rs.GetFieldValue("IP",m_strDBAddrE);
				Rs.GetFieldValue("DataBaseName",m_strDBNameE);
				Rs.GetFieldValue("UserName",m_strDBUserE);
				Rs.GetFieldValue("Pwd",m_strDBPwdE);
				nInitFlag |= 0x4;
			}
			else if ("MEOES" == strDbType)
			{
				Rs.GetFieldValue("IP",m_strDBAddrM);
				Rs.GetFieldValue("DataBaseName",m_strDBNameM);
				Rs.GetFieldValue("UserName",m_strDBUserM);
				Rs.GetFieldValue("Pwd",m_strDBPwdM);
				nInitFlag |= 0x8;
			}
			Rs.MoveNext();
		}
		Rs.Close();
	}
	if (nInitFlag != 0xF)
	{
		CString strErrorMsg;
		strErrorMsg.Format("Not Init All DB,[PARAM_ADJ-DC-EOES-MEOES]-[%d-%d-%d-%d]", 
			nInitFlag&0x1, nInitFlag&0x2, nInitFlag&0x4, nInitFlag&0x8);
		throw std::exception(strErrorMsg.GetBuffer());
	}

	//后台版本登记------------------------------------------------------------------------------------------------------
	//获取服务器所在的IP地址
	CString strHostAddr;
	GetLocalHostIpAddr(strHostAddr, pszDbAddr, 1433);

	//获取可执行文件的本版信息
	char cFilePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, cFilePath, sizeof(cFilePath));
	CString strVersion;
	CUtils::GetFileVersion(cFilePath, strVersion);

	//将这些信息保存到主站数据库中:如果信息已经存在就更新信息，否则新增
	if(!SaveProgramInfoIntoDB(mainConfig, m_strDBID, m_ServiceName, strVersion, strHostAddr))
	{
		return false;
	}
	
	return true;
}

void CConfig::FullConfigInfo()
{
	////鉴权配置---------------------------------------------------------------------------------------------------
	//m_strIDIP = GETPARAM("IdentifyCfg", "IdentifyIP","127.0.0.1");
	//m_nIDPort = GETPARAM("IdentifyCfg", "IdentifyPort", 6001);

	////短信监控配置------------------------------------------------------------------------------------------------------
	//m_strSMSIP = GETPARAM("SMSMonitor", "SMSIP","127.0.0.1");
	//m_nSMSPort = GETPARAM("SMSMonitor", "SMSPort",m_nSMSPort);

	//系统-------------------------------------------------------------------------------------------------
	m_nSTSCycle = GETPARAM("System", "STSCycle",15);
	m_nRecvTimeout  = GETPARAM("System", "RecvTimeout",30);
	m_nEnter  = GETPARAM("System", "Enter",1);
	m_nThreadNum  = GETPARAM("System", "ThreadNum",50);
}

/*
 @brief 将获取的应用程序ID、windows服务名、可执行文件的版本信息、本机服务器的IP更新到表[SYS_APPLICATION_INFO]中记录，如果记录不存在则插入新记录
 @param adoPoll : <IN>
 @param nId : <IN> 应用程序ID
 @param strCurServiceName : <IN> windows服务名
 @param strVersion : <IN> 可执行文件的版本信息
 @param strHostAddr : <IN> 本机服务器的IP
 @return bool
*/
bool CConfig::SaveProgramInfoIntoDB(CADODatabase& adoPoll, int nId, const CString& strCurServiceName, const CString& strVersion, const CString& strHostAddr)
{
	CADORecordset Rs(&adoPoll);
	CString strSQL;
	strSQL.Format("if exists (select * from SYS_APPLICATION_INFO where APP_ID = %d)\r\n\
				  begin\r\n\
				  update SYS_APPLICATION_INFO set SERVICE_NAME = '%s', APP_VERSION = '%s', IP = '%s' where APP_ID = %d \r\n\
				  end\r\n\
				  else\r\n\
				  begin\r\n\
				  insert into SYS_APPLICATION_INFO(APP_ID, SERVICE_NAME, APP_VERSION, IP) values(%d, '%s', '%s', '%s')\r\n\
				  end;\r\n",m_strDBID,strCurServiceName,strVersion,strHostAddr,m_strDBID,m_strDBID,strCurServiceName,strVersion,strHostAddr);

	if(!adoPoll.Execute((LPCTSTR)strSQL))
	{
		return false;
	}

	return true;
}

/*
 @brief : 取得本机的IP地址（通过套接字连接到一个数据库，再从这个套接字中取得本机的IP地址）
 @param strHostAddr : <OUT>保存取得的IP地址
 @param strDBAddress : <IN>要连接的数据库的地址
 @param port : <IN>端口（数据库的端口1433）
 @return void
*/
void CConfig::GetLocalHostIpAddr(CString& strHostAddr, const CString& strDBAddress, u_short port)
{
	//创建套接字
	SOCKET mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(strDBAddress);
	sockAddr.sin_port = htons(port);
	//连接到一个数据库
	connect(mySocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
	sockaddr_in hostAddr;
	int hostAddrLen = sizeof(hostAddr);
	//从这个已连接的套接字中获取本机的IP地址
	getsockname(mySocket, (sockaddr*)&hostAddr, &hostAddrLen);
	char * cHostAddr = inet_ntoa(hostAddr.sin_addr);
	//关闭套接字
	closesocket(mySocket);

	//获取本机的IP地址
	strHostAddr.Empty();
	strHostAddr = cHostAddr;
}
