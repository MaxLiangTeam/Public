/*---------------------------------------------------------------------------
�ļ�    : 
����    : ��Ϊ�ز��Զ�����ϵͳ�����ļ�
����    : wsl
�汾    : V1.0
ʱ��    : 2012-02-22
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
	//�ö��ڴ�ֵ����Ĭ��ֵ
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

			CString strData;								//�ļ�����
			CString strAppTemp = "[" + strSection + "]";	//��APPλ��
			CString strKeyTemp = strKey + "=";			//��Keyλ��
			CString strDescrition = "#" + strDes + "\r\n";	//�������ַ���
			CString strIsExist = strDes + strKeyTemp;		//�������ַ�����λ��

			int nStart(0),nEnd(0);
			if(0 != mFile.Open(m_strPath,CFile::modeReadWrite))
			{
				int nLen = (int)mFile.GetLength();//�ļ���С
				char *pBuff = (char *)malloc(nLen+1);
				ZeroMemory(pBuff,nLen+1);

				mFile.Read(pBuff,nLen);//���ļ�
				strData = pBuff;
				free(pBuff);

				nStart = strData.Find(strIsExist);
				if (-1 == nStart)//�ж�ע���ǲ����Ѿ�����,�����ڲ�д
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
	//�ö��ڴ�ֵ����Ĭ��ֵ
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

			CString strData;								//�ļ�����
			CString strAppTemp = "[" + strSection + "]";	//��APPλ��
			CString strKeyTemp = strKey + "=";			//��Keyλ��
			CString strDescrition = "#" + strDes + "\r\n";	//�������ַ���
			CString strIsExist = strDescrition + strKeyTemp;		//�������ַ�����λ��

			int nStart(0),nEnd(0);
			if(0 != mFile.Open(m_strPath,CFile::modeReadWrite))
			{
				int nLen = (int)mFile.GetLength();//�ļ���С
				char *pBuff = (char *)malloc(nLen+1);
				ZeroMemory(pBuff,nLen+1);

				mFile.Read(pBuff,nLen);//���ļ�
				strData = pBuff;
				free(pBuff);

				nStart = strData.Find(strIsExist);
				if (-1 == nStart)//�ж�ע���ǲ����Ѿ�����,�����ڲ�д
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
		//ɾ���ɵ��ļ�
		CFile::Remove(m_strPath);

		//���������ļ�
		m_bUpdate = true;

		//���¼����ļ�
		return LoadFile(m_strPath);
	}
	catch (...)
	{
	}
	return false;
}

//�����ļ�INI
bool CConfig::LoadFile(CString strPath)
{
	try
	{
		//�����ļ���
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

		//�������ݿ�---------------------------------------------------------------------------------------------------
		GetValue("iSON-LOCAL", "DBIP",m_strDBAddrL,"127.0.0.1","���ؿ�IP��ַ");
		GetValue("iSON-LOCAL", "DBName",m_strDBNameL,"iSON_LOCAL","���ؿ�����");
		GetValue("iSON-LOCAL", "DBUser",m_strDBUserL,"sa","���ؿ��¼�û�");
		GetValue("iSON-LOCAL", "DBPassword",m_strDBPwdL,"123456","���ؿ��õ�¼����",true);

		//��������
		GetValue("iSON-DC", "DBIP",m_strDBAddrC,"127.0.0.1","�������Ŀ�IP��ַ");
		GetValue("iSON-DC", "DBName",m_strDBNameC,"iSON_DC","�������Ŀ�����");
		GetValue("iSON-DC", "DBUser",m_strDBUserC,"sa","�������Ŀ��¼�û�");
		GetValue("iSON-DC", "DBPassword",m_strDBPwdC,"123456","�������Ŀ��õ�¼����",true);

		//2GERIC��������
		GetValue("iSON-2GERIC-DC", "DBIP",m_strDBAddr2G,"127.0.0.1","2G�������Ŀ�IP��ַ");
		GetValue("iSON-2GERIC-DC", "DBName",m_strDBName2G,"iSON_DC","2G�������Ŀ�����");
		GetValue("iSON-2GERIC-DC", "DBUser",m_strDBUser2G,"sa","2G�������Ŀ��¼�û�");
		GetValue("iSON-2GERIC-DC", "DBPassword",m_strDBPwd2G,"123456","2G�������Ŀ��õ�¼����",true);

		//2GHW��������
		GetValue("iSON-2GHW-DC", "DBIP",m_strDBAddrHW2G,"127.0.0.1","2GHW�������Ŀ�IP��ַ");
		GetValue("iSON-2GHW-DC", "DBName",m_strDBNameHW2G,"iSON_DC","2GHW�������Ŀ�����");
		GetValue("iSON-2GHW-DC", "DBUser",m_strDBUserHW2G,"sa","2GHW�������Ŀ��¼�û�");
		GetValue("iSON-2GHW-DC", "DBPassword",m_strDBPwdHW2G,"123456","2GHW�������Ŀ��õ�¼����",true);

		//�������ݿ�
		GetValue("iSON-EOES", "DBIP",m_strDBAddrE,"127.0.0.1","�������ݿ�IP��ַ");
		GetValue("iSON-EOES", "DBName",m_strDBNameE,"EOES_DATA","�������ݿ�����");
		GetValue("iSON-EOES", "DBUser",m_strDBUserE,"sa","�������ݿ��¼�û�");
		GetValue("iSON-EOES", "DBPassword",m_strDBPwdE,"123456","�������ݿ��õ�¼����",true);

		//�������ݿ�����
		GetValue("iSON-EOES-M", "DBIP",m_strDBAddrM,"127.0.0.1","�������ݿ�����IP��ַ");
		GetValue("iSON-EOES-M", "DBName",m_strDBNameM,"EOES_MASTER","�������ݿ���������");
		GetValue("iSON-EOES-M", "DBUser",m_strDBUserM,"sa","�������ݿ������¼�û�");
		GetValue("iSON-EOES-M", "DBPassword",m_strDBPwdM,"123456","�������ݿ������õ�¼����",true);


		//ϵͳ-------------------------------------------------------------------------------------------------
		GetValue("System", "STSCycle", m_nSTSCycle,15, "ͳ����������(Ĭ��15��)");
		GetValue("System", "RecvTimeout", m_nRecvTimeout,30, "TELNET���ճ�ʱ(��λ:��)");
		GetValue("System", "Enter", m_nEnter,1, "#TELNET���������Ƿ�ӻس�(0������ 1-�ӻس�)");
		GetValue("System", "ThreadNum", m_nThreadNum,50, "�̳߳��������߳���(Ĭ��50)");
		GetValue("System", "StartMin", m_nStartMin,15, "��������(��λ:��)");
		GetValue("System", "AdjTime", m_nAdjTime,50, "����ʱ��(Ĭ��50)");
		GetValue("System", "TaskStartTime", m_nStartTime, "02:20", "�ඨ©����������ʱ��(Ĭ��02:20,��ʽ:��λʱ:��λ��)");
		GetValue("System", "TaskAdjust", m_nAdjustTime, "02:50", "�ඨ©����������ʱ��(Ĭ��02:50,��ʽ:��λʱ:��λ��)");
		GetValue("System", "CentreConfig", m_bCentreConfig,1, "�������ÿ���(0-�� 1��)");
		GetValue("System", "AdjEnbNum", m_nAdjEnbNum,10000, "������վ�����(Ĭ��:10000)");
		GetValue("System", "SendCmdThreadNum", m_nSendCmdThreadNum,1, "�·�ָ���߳���(Ĭ��1)",true);

		//��־----------------------------------------------------------------------------------------------------
		GetValue("Log", "LogSave",m_nLogSave,7,"��־��������");
		GetValue("Log", "LogLevel",m_nLogLevel,3,"��־�ȼ�(0-�� 1-���� 2-��Ϣ 3-����)",true);
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
	//���ӱ������ݿ�
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

	//��̨�汾�Ǽ�------------------------------------------------------------------------------------------------------
	//��ȡ���������ڵ�IP��ַ
	CString strHostAddr;
	GetLocalHostIpAddr(strHostAddr, pszDbAddr, 1433);

	//��ȡ��ִ���ļ��ı�����Ϣ
	char cFilePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, cFilePath, sizeof(cFilePath));
	CString strVersion;
	CUtils::GetFileVersion(cFilePath, strVersion);

	//����Щ��Ϣ���浽��վ���ݿ���:�����Ϣ�Ѿ����ھ͸�����Ϣ����������
	if(!SaveProgramInfoIntoDB(mainConfig, m_strDBID, m_ServiceName, strVersion, strHostAddr))
	{
		return false;
	}
	
	return true;
}

void CConfig::FullConfigInfo()
{
	////��Ȩ����---------------------------------------------------------------------------------------------------
	//m_strIDIP = GETPARAM("IdentifyCfg", "IdentifyIP","127.0.0.1");
	//m_nIDPort = GETPARAM("IdentifyCfg", "IdentifyPort", 6001);

	////���ż������------------------------------------------------------------------------------------------------------
	//m_strSMSIP = GETPARAM("SMSMonitor", "SMSIP","127.0.0.1");
	//m_nSMSPort = GETPARAM("SMSMonitor", "SMSPort",m_nSMSPort);

	//ϵͳ-------------------------------------------------------------------------------------------------
	m_nSTSCycle = GETPARAM("System", "STSCycle",15);
	m_nRecvTimeout  = GETPARAM("System", "RecvTimeout",30);
	m_nEnter  = GETPARAM("System", "Enter",1);
	m_nThreadNum  = GETPARAM("System", "ThreadNum",50);
}

/*
 @brief ����ȡ��Ӧ�ó���ID��windows����������ִ���ļ��İ汾��Ϣ��������������IP���µ���[SYS_APPLICATION_INFO]�м�¼�������¼������������¼�¼
 @param adoPoll : <IN>
 @param nId : <IN> Ӧ�ó���ID
 @param strCurServiceName : <IN> windows������
 @param strVersion : <IN> ��ִ���ļ��İ汾��Ϣ
 @param strHostAddr : <IN> ������������IP
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
 @brief : ȡ�ñ�����IP��ַ��ͨ���׽������ӵ�һ�����ݿ⣬�ٴ�����׽�����ȡ�ñ�����IP��ַ��
 @param strHostAddr : <OUT>����ȡ�õ�IP��ַ
 @param strDBAddress : <IN>Ҫ���ӵ����ݿ�ĵ�ַ
 @param port : <IN>�˿ڣ����ݿ�Ķ˿�1433��
 @return void
*/
void CConfig::GetLocalHostIpAddr(CString& strHostAddr, const CString& strDBAddress, u_short port)
{
	//�����׽���
	SOCKET mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(strDBAddress);
	sockAddr.sin_port = htons(port);
	//���ӵ�һ�����ݿ�
	connect(mySocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
	sockaddr_in hostAddr;
	int hostAddrLen = sizeof(hostAddr);
	//����������ӵ��׽����л�ȡ������IP��ַ
	getsockname(mySocket, (sockaddr*)&hostAddr, &hostAddrLen);
	char * cHostAddr = inet_ntoa(hostAddr.sin_addr);
	//�ر��׽���
	closesocket(mySocket);

	//��ȡ������IP��ַ
	strHostAddr.Empty();
	strHostAddr = cHostAddr;
}
