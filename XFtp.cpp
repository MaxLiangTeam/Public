#include "StdAfx.h"
#include "XFtp.h"
#include "Include.h"
#include "Kernel.h"

CXFtp::CXFtp(int nWaitTime/* = 2*/)
{
	try
	{
		m_nWaitTime = nWaitTime;
		m_pFtpConnection = NULL;
		m_pInetsession = NULL;
		m_pInetsession = new CInternetSession("session",0,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL, INTERNET_FLAG_DONT_CACHE);

		//���ó�ʱ
		m_pInetsession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,60*1000); 
		m_pInetsession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT,60*1000); 
		m_pInetsession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,60*1000);
		m_pInetsession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 3);// 3������
	}
	catch (...)
	{
		CString strError;
		strError.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strError);
	}
	
}

CXFtp::~CXFtp(void)
{
	try
	{
		if(m_pFtpConnection != NULL)
		{
			m_pFtpConnection->Close();
			delete m_pFtpConnection;
		}
		delete m_pInetsession;	
	}
	catch (...)
	{
		CString strError;
		strError.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strError);
	}
}

BOOL CXFtp::Login(CString strFtpAddress,CString strUser,CString strPassword,int nPort /*= 21*/ ,BOOL bPassive/*=FALSE*/)
{
	BOOL bRet = FALSE;

	try
	{
		m_strAddress	=	strFtpAddress;
		m_strUserName	=	strUser;
		m_strPassword	=	strPassword;
		m_nPort			=	nPort;

		if (m_pInetsession != NULL)
		{
			m_pFtpConnection = m_pInetsession->GetFtpConnection(m_strAddress,m_strUserName,m_strPassword,bPassive);

			if (m_pFtpConnection != NULL)
			{
				bRet = TRUE;
			}
		}

	}
	catch (...)
	{
		m_pFtpConnection = NULL;

		CString strError;
		strError.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strError);
	}

	return bRet;
}

void CXFtp::Logout()
{
	try
	{
		if(m_pFtpConnection != NULL)
		{
			m_pFtpConnection->Close();
			//delete m_pFtpConnection;
			//m_pFtpConnection = NULL;
		}
		if (m_pInetsession != NULL)
		{
			m_pInetsession->Close();
		}
	}
	catch (...)
	{
	}
}

BOOL CXFtp::DownLoadFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile)
{
	BOOL bRet = FALSE;

	try
	{
		if (NULL != m_pFtpConnection)
		{
			//bRet = m_pFtpConnection->GetFile(pstrRemoteFile,pstrLocalFile,FALSE);
			bRet = m_pFtpConnection->GetFile(pstrRemoteFile,pstrLocalFile,FALSE,FILE_ATTRIBUTE_NORMAL ,FTP_TRANSFER_TYPE_BINARY);

			if (bRet)
			{
			    CString strLog;
                strLog.Format("�����ļ�[%s]�ɹ�",pstrRemoteFile);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
			} 
			else
			{
				CString strLog;
				strLog.Format("�����ļ�[%s]ʧ��",pstrRemoteFile);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
			}
		}
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
	}

	return bRet;
}

BOOL CXFtp::DownFile(CString strFileName,CString strLocalDirectory,int nWaiteMin)
{
	BOOL bRet(FALSE);
	try
	{
		if (NULL == m_pFtpConnection)
		{
			return FALSE;
		}

		//��ʼ���ص�ʱ��
		m_tTime = CTime::GetCurrentTime();
		m_nWaitTime = nWaiteMin;

		//���������ļ���
		CreateDir(strLocalDirectory);

		if (FindFtpFile(strFileName + ".zip"))
		{
			m_strLocalFilePath = strLocalDirectory + "\\" + m_strRemoteFileName;//���浽���ص��ļ���
			if (m_strRemoteFileName.Find(".zip") != -1)
			{
				Sleep(10*1000);
			}

			CString strLog;
			strLog.Format("�ҵ��ļ�-%s",m_strRemoteFileName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);

			bRet = DownLoadFile(m_strRemoteFileName,m_strLocalFilePath);
		}
		else
		{
			m_strRemoteFileName = strFileName + ".zip";

			CString strLog;
            strLog.Format("�Ҳ����ļ�-%s(.zip��.txt),�������������ļ�-%s",strFileName,m_strRemoteFileName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
			
			m_strLocalFilePath = strLocalDirectory + "\\" + m_strRemoteFileName;//���浽���ص��ļ���
			bRet = DownLoadFile(m_strRemoteFileName,m_strLocalFilePath);
		}
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
	}
	return bRet;
}

BOOL CXFtp::RemoveFile(CString strFileName)
{
	BOOL bRet = FALSE;

	try
	{
		if (NULL == m_pFtpConnection)
		{
			return FALSE;
		}

		//if (FindFtpFile(strFileName))
		//{
			bRet = m_pFtpConnection->Remove(strFileName);
		//}
	}
	catch (...)
	{
	}

	return bRet;
}

BOOL CXFtp::SetFtpDirectory( LPCTSTR pstrRemoteDirectory )
{
	BOOL bRet = FALSE;

	m_strCurrentPath = pstrRemoteDirectory;

	try
	{
		if (NULL != m_pFtpConnection)
		{
			bRet = m_pFtpConnection->SetCurrentDirectory(pstrRemoteDirectory);
		}
	}
	catch (...)
	{
		CString strError;
		strError.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strError);
	}

	return bRet;
}

bool CXFtp::FindFtpFile(CString strFileName)
{
	bool bRet = false;

	try
	{
		bool bExist = false;//�ǹ�ƥ�䵽�ļ���
		while(!bExist)
		{
			CFtpFileFind mFtpFind(m_pFtpConnection);//ֻ��ͬʱ����һ��CFtpFileFind
			BOOL bFind = mFtpFind.FindFile("*");

			//OutputDebugString("FindFile\n");

			while(bFind)
			{
				bFind = mFtpFind.FindNextFile();

				if (mFtpFind.IsDots() || mFtpFind.IsDirectory())
				{
					continue;//. , .. 
				}

				//������ļ�
				CString strTemp = mFtpFind.GetFileName();//�ļ���

				//�ܹ�ƥ����
				if (-1 != strTemp.Find(strFileName))
				{
					m_strRemoteFileName =  mFtpFind.GetFileName();

					bExist = true;
					bRet = true;
					break;
				}
			}

			//�ر�ftp����
			mFtpFind.Close();
		
			CTime now = CTime::GetCurrentTime();//��ǰʱ��
			CTimeSpan   tmSpan = now - m_tTime;

			//һ�α����������ҵ����߳�ʱ
			if ((bExist) || (tmSpan.GetMinutes() > m_nWaitTime))
			{
				break;
			}
			if (tmSpan.GetMinutes() > m_nWaitTime-1)
			{
				strFileName.Replace(".zip",".txt");
				CString strLog;
				strLog.Format("��ʱǰһ������%s",strFileName);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
			}
			
			//�ȴ�1s������һ�α���
			Sleep(1000*5);
		}
	}
	catch (...)
	{
		CString strError;
		strError.Format("ERROR:%d,%s,%d",GetLastError(),__FILE__,__LINE__);
		TRACE(strError);
	}
	
	return bRet;
}

bool CXFtp::UpLoadFile( const CString &strSrcFile,const CString &strDestFileName )
{
	bool bRet = false;

	try
	{
		if (NULL == m_pFtpConnection)
		{
			return false;
		}

		if(m_pFtpConnection->PutFile(strSrcFile,strDestFileName))
		{
			bRet = true;
		}
	}
	catch (...)
	{
	}

	return bRet;
}
