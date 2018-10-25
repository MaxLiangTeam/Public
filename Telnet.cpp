/*---------------------------------------------------------------------------
ÎÄ¼þ    : 
ÃèÊö    : 
×÷Õß    : wsl
°æ±¾    : V1.0
Ê±¼ä    : 2012-01-13
2012-01-13 ¸Ä°æ
2012-02-27 ÔÙ´ÎÐÞ¸Ä
2012-09-26 ÐÞ¸ÄÅÐ¶Ï½áÊø·ûº¯Êý
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "Telnet.h"
#include "Kernel.h"

struct check_end_tab 
{
	int (CTelnet::*p)(CString& strCmd,CString& strPack,CString& strData);// º¯ÊýÖ¸Õë
} check_end[] = {
	&CTelnet::CheckEnd0,//LST²éÑ¯Ö¸Áî
	&CTelnet::CheckEnd1,//µ÷ÕûÖ¸Áî
	&CTelnet::CheckEnd2,//DSP
	&CTelnet::CheckEnd3,//µÇÂ¼»Ø¸´
	&CTelnet::CheckEnd4,//
	&CTelnet::CheckEnd5,//MMLµÇÂ½×¢²áÍøÔª
	&CTelnet::CheckEnd6,//MML×¢ÏúÍøÔª
	&CTelnet::CheckEnd7,//µÇ³ö»Ø¸´
	&CTelnet::CheckEnd8//¼ì²â23¶Ë¿Ú
};
bool CTelnet::m_bInit = false;

CTelnet::CTelnet(void)
:m_nTimeout(30)
{
	m_hSocket = INVALID_SOCKET;
	if (!m_bInit)
	{
		m_bInit = true;
		WSADATA wsaData = {0};
		WORD wVersionRequested = MAKEWORD( 2, 2 );
		WSAStartup( wVersionRequested, &wsaData);
	}

	m_bBuf.SetSize(1024);
}

CTelnet::~CTelnet(void)
{
	m_bExit = true;
	if (m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
	}
	m_hSocket = INVALID_SOCKET;
}

//¼ì²éÊÇ·ñÊÇ½ÓÊÕÍêÕûÊý¾Ý°ü±êÖ¾
int CTelnet::CheckEnd0(CString& strCmd,CString& strPack,CString& strData)
{
	if (strData.Find("¸üÐÂ") != -1)
	{
		return 0;//½ÓÊÕ½áÊø±êÖ¾
	}
	return -1;
}

//µ÷ÕûÖ¸Áî½áÊø¼ì²â
int CTelnet::CheckEnd1(CString& strCmd,CString& strPack,CString& strData)
{
	CString strFlag,strDataInfo;
	strDataInfo = BufferFilter(strData.GetBuffer(0));
	if (strCmd.Find("KILL MUTEXRIGHT")!= -1)
	{
		strFlag = "ÊÍ·Å»¥³âÈ¨ÏÞ³É¹¦";
	}
	else if (strCmd.Find("APPLY MUTEXRIGHT")!= -1)
	{
		strFlag = "ÉêÇë»¥³âÈ¨ÏÞ³É¹¦";
	}
	else if (strCmd.Find("RELEASE MUTEXRIGHT")!= -1)
	{
		strFlag = "ÊÍ·Å»¥³âÈ¨ÏÞ³É¹¦";
	}
	else if (strCmd.Find("UPDATE:MOC=")!= -1)
	{
		strFlag = "¶ÔÏó±»¸üÐÂ";
	}
	else if (strCmd.Find("SYNC:POS")!= -1)
	{
		strFlag = "È«²¿³É¹¦";
	}
	if(strDataInfo.Find("½á¹û") != -1)
	{
		int nPos = strDataInfo.Find("½á¹û");
		strDataInfo.Delete(0,nPos);
		CString strRight = strDataInfo.Right(2);
		if (strDataInfo.Find(strFlag) != -1 && strRight.Find("$>") != -1)
		{
			return 0;//½ÓÊÕ½áÊø±êÖ¾
		}
		else if (strDataInfo.Find("²¿·Ö³É¹¦") != -1 && strRight.Find("$>") != -1 && strCmd.Find("SYNC:POS")!= -1)
		{
			return 0;
		}
		else if (strDataInfo.Find("ÃüÁîÖ´ÐÐÊ§°Ü") != -1 && strRight.Find("$>") != -1 && strCmd.Find("SYNC:POS")!= -1)
		{
			return -2;
		}
	}

	return -1;
}

//DSPÖ¸Áî½áÊø¼ì²â
int CTelnet::CheckEnd2(CString& strCmd,CString& strPack,CString& strData)
{
	if((strPack.Find("¹²ÓÐ") != -1 || strPack.Find("(½á¹û¸öÊý") != -1) && strPack.Find("END") != -1)
	{
		if (strData.Find(strCmd) != -1)
		{
			return 0;//½ÓÊÕ½áÊø±êÖ¾
		}
	}
	return -1;
}

//µÇÂ¼½áÊø¼ì²â
int CTelnet::CheckEnd3(CString& strCmd,CString& strPack,CString& strData)
{
	if(strPack.Find("END") != -1 && strCmd.Find("LGI:OP") != -1 && strData.Find("%%LGI:OP") != -1)
	{
		return 0;
	}
	return -1;
}

//EXPORT½áÊø·ûÅÐ¶Ï
int CTelnet::CheckEnd4(CString& strCmd,CString& strPack,CString& strData)
{
	CString strEnd = strPack.Right(1024);
	if(strEnd.Find("×´Ì¬  =  ³É¹¦") != -1 || strEnd.Find("ÎÄ¼þÒÑ±»´æ´¢µ½") != -1)
	{
		if (strData.Find("%%EXP CFGMML") != -1)
		{
			return 0;
		}
	}
	return -1;
}

//MML×¢²áÍøÔª
int CTelnet::CheckEnd5(CString& strCmd,CString& strPack,CString& strData)
{
	if(strPack.Find("%%REG NE:NAME") != -1 && strData.Find("END") != -1)
	{
		return 0;//½ÓÊÕ½áÊø±êÖ¾
	}
	return -1;
}

//MML×¢ÏúÍøÔª
int CTelnet::CheckEnd6(CString& strCmd,CString& strPack,CString& strData)
{
	if(strPack.Find("%%UNREG NE:NAME") != -1 && strData.Find("END") != -1)
	{
		return 0;//½ÓÊÕ½áÊø±êÖ¾
	}
	return -1;
}

//µÇ³ö
int CTelnet::CheckEnd7(CString& strCmd,CString& strPack,CString& strData)
{
	if(strPack.Find("%%LGO:OP") != -1  && strData.Find("END") != -1)
	{
		return 0;//½ÓÊÕ½áÊø±êÖ¾
	}
	return -1;
}

int CTelnet::CheckEnd8(CString& strCmd,CString& strPack,CString& strData)
{
	if(strPack.Find(">") != -1 || strPack.Find("<") != -1)
	{
		return 0;//½ÓÊÕ½áÊø±êÖ¾
	}
	return -1;
}

bool CTelnet::CheckLoginMsg(CString& strCmd)
{
	CString strPWDFlag = "PWD=\"";
	CString strEMSFlag = "\",DN=EMS";
	CString strNoEMSFlag = "\";";
	CString strCmdTemp = strCmd;

	int nPWDpos = strCmdTemp.Find(strPWDFlag);
	if (nPWDpos == std::string::npos)
	{
		return false;
	}

	int nEMSpos = strCmdTemp.Find(strEMSFlag,nPWDpos);
	int nNoEMSpos = strCmdTemp.Find(strNoEMSFlag,nPWDpos);

	if(nEMSpos != std::string::npos)
	{
		//É¾³ýpwdÈ»ºóÓÃ*Ìæ»»
		strCmdTemp.Delete(nPWDpos + strPWDFlag.GetLength(),nEMSpos - (nPWDpos + strPWDFlag.GetLength()));
		strCmdTemp.Insert(nPWDpos + strPWDFlag.GetLength(),"******");
	}
	else if(nNoEMSpos != std::string::npos)
	{
		//É¾³ýpwdÈ»ºóÓÃ*Ìæ»»
		strCmdTemp.Delete(nPWDpos + strPWDFlag.GetLength(),nNoEMSpos - (nPWDpos + strPWDFlag.GetLength()));
		strCmdTemp.Insert(nPWDpos + strPWDFlag.GetLength(),"******");
	}
	else
	{
		return false;
	}

	if(strCmdTemp.IsEmpty()) return false;

	strCmd = strCmdTemp;
	return true;
}

int CTelnet::Send(CString strCmd,CString& strData,enCheckEnd enCheck,bool bIsSend)
{
	CString strLog;
	if (!bIsSend)
	{
		strLog.Format("[%s][CTelnet::Send] ²»·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		strData = "RETCODE = 0";
		return strData.GetLength();
	}

	//Èç¹ûÎªwindows,·¢ËÍÖ¸ÁîÐèÒª¼Ó»Ø³µ
	//if(g_bIsTest)g_pConfig->m_nEnter = 0;
	CString newCmd = strCmd + (g_pConfig->m_nEnter ? "\r\n" : "");
	int nLen = send(m_hSocket,newCmd.GetBuffer(),newCmd.GetLength(),0);

	if (nLen <= 0)
	{
		strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ·¢ËÍÊ§°Ü",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return -1;//·¢ËÍÊ§°Ü
	}
	strLog.Format("[%s][CTelnet::Send] ·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
	CheckLoginMsg(strLog);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	
	//Çå¿Õ½ÓÊÕÊý¾Ý
	strData.Empty();

	//ÉèÖÃ½ÓÊÕ³¬Ê±Ê±¼ä
	int nTimeout(m_nTimeout);
	if (enCheck != CHECK_EXEC)
	{
		nTimeout = 5;
	}
	//Èç¹ûÊÇSET GTRXADMSTAT: TimeoutÊ±¼äÉèÎª1·ÖÖÓ
	if (newCmd.Find("SYNC:POS=") != -1)
	{
		nTimeout = 600;
	}

	time_t t = time(NULL);
	while (!m_bExit)
	{
		//¿ªÊ¼½ÓÊÕ
		char szBuff[1024+1] = {0};//1k
		int nRecvLen = recv(m_hSocket,szBuff,1024,0);
		if (nRecvLen < 0)
		{
			if (strData.IsEmpty())
			{
				if (time(NULL) - t >= nTimeout)
				{
					strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕÎª¿Õ,³¬Ê±Ê±¼ä=%d s",m_strThreadName,strCmd,time(NULL) - t);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					return -2;
				}
				continue;
			}
			else
			{
				if (time(NULL) - t < nTimeout)continue;

				if (0 != (this->*check_end[enCheck].p)(strCmd,strData,strData))
				{
					strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ²»ÍêÕû,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,nTimeout,strData);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					return -2;//-2 Îª½ÓÊÕÊ§°Ü
				}
				break;//½ÓÊÕÍêÕû
			}
		}
		CString strRecv(BufferFilter(&szBuff[0]));
		//BufferFilter(strRecv.GetString(0));
		strRecv.Replace("\r","");
		strData += strRecv;
		
		//ÅÐ¶ÏÊÇ·ñ½ÓÊÜÍê±Ï
		int nRetCmd = (this->*check_end[enCheck].p)(strCmd,strRecv,strData);
		if (-1 != nRetCmd)
		{
			if (-2 == nRetCmd)
			{
				return -3;
			}
			break;//½ÓÊÕÍê±Ï
		}

		//add by maxliang 20130904 ¼ì²éÊÇ·ñ³¬Ê±
		if (time(NULL) - t >= nTimeout)
		{
			strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,nTimeout,strData);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return -2; 
		}
		Sleep(1);
	}
	CString strDataInfo = BufferFilter(strData.GetBuffer(0));
	strLog.Format("[%s][CTelnet::Recv] [%s]",m_strThreadName,strDataInfo);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	return strDataInfo.GetLength();
}

int CTelnet::Send(CString strCmd,CString& strData,bool bIsSend)
{
	CString strLog;
	if (!bIsSend)
	{
		strLog.Format("[%s][CTelnet::Send] ²»·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		strData = "RETCODE = 0";
		return strData.GetLength();
	}

	//Èç¹ûÎªwindows,·¢ËÍÖ¸ÁîÐèÒª¼Ó»Ø³µ
	//if(g_bIsTest)g_pConfig->m_nEnter = 0;
	CString newCmd = strCmd + (g_pConfig->m_nEnter ? "\r\n" : "");
	int nLen = send(m_hSocket,newCmd.GetBuffer(),newCmd.GetLength(),0);

	if (nLen <= 0)
	{
		strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ·¢ËÍÊ§°Ü",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return -1;//·¢ËÍÊ§°Ü
	}
	strLog.Format("[%s][CTelnet::Send] ·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
	CheckLoginMsg(strLog);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	//Çå¿Õ½ÓÊÕÊý¾Ý
	strData.Empty();
	//ÉèÖÃ½ÓÊÕ³¬Ê±Ê±¼ä
	int nTimeout(m_nTimeout);
	if (newCmd.Find("SYNC:POS=") != -1)
	{
		nTimeout = 600;
	}

	time_t t = time(NULL);
	char szBuff[1024+1];//1k
	bool bReMark(false);
	CString strTemp;
	int nRecvLen = recv(m_hSocket,szBuff,1024,0);
	while (nRecvLen > 0)
	{
		CString strRecv = szBuff;
		//strRecv.Replace("\n","");
		//strTemp += strRecv;
		//g_pLog->WriteLog(strRecv,m_strLogFile,LOG_INFO);
		if(strRecv.Find("½á¹û") != -1 )
		{
			//strRecv.Replace("[D[D","");
			int nPos = strRecv.Find("½á¹û");
			strRecv.Delete(0,nPos);
			bReMark = true;
		}
		if (bReMark)
		{
			strData += strRecv;
		}
		
		int nLen = strData.GetLength();
		// µ±½ÓÊÕµ½Ä©Î²Îª $> ±íÊ¾½áÊø
		CString strRight = strData.Right(2);
		//½ÓÊÜÍê±Ï,³¤¶ÈÐ¡ÓÚ1024²»Îª¿Õ,ÇÒÔÚ×îºóÃæÕÒµ½×Ö·û´®\n>,¾Í´ú±í½ÓÊÕÍê±Ï
		if (strRight.Find("$>") != -1 && strData.Find("½á¹û") != -1  /*&& strData.Find(strCmd) != -1*/)
		{
			bool bRet(false);
			if (strCmd.Find("SYNC:POS") != -1)
			{
				//  ·¢ËÍÍ¬²½Ö¸Áî, µÈ´ý½ÓÊÕÒì²½·µ»ØÐÅÏ¢
				strLog.Format("[%s][Telnet::Send] ·¢ËÍÍ¬²½Ö¸Áî, µÈ´ý½ÓÊÕÒì²½ÐÞ¸Ä½ø¶ÈÐÅÏ¢.",m_strThreadName);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
				while(nRecvLen > 0)
				{
					if (strData.Find("ÃüÁîÖ´ÐÐÊ§°Ü") != -1)
					{
						return -3;
					}
					//strData += BufferReplace(strRecv);
					//g_pLog->WriteLog(strData,m_strLogFile,LOG_INFO);
					if (strData.Find("È«²¿³É¹¦") != -1)
					{
						bRet = true;
						break;
					}

					//// µ±½ÓÊÕµ½Ä©Î²Îª "Êý¾ÝÍ¬²½  (×ÜÊý:1 ³É¹¦:1 Ê§°Ü:0)È«²¿³É¹¦[100]" ±íÊ¾½áÊø
					//int nPos = 0;
					//nPos = strData.Find("Êý¾ÝÍ¬²½", nPos);
					//if (nPos != -1)
					//{
					//	nPos = strData.Find("×ÜÊý", nPos);
					//	if (nPos != -1)
					//	{
					//		nPos = strData.Find("³É¹¦", nPos);
					//		if (nPos != -1)
					//		{
					//			nPos = strData.Find("Ê§°Ü", nPos);
					//			if (nPos != -1)
					//			{
					//				int nLeft = 0;
					//				nLeft = strData.Find(":", nPos);
					//				int nRight = 0;
					//				nRight = strData.Find(")", nLeft);
					//				CString strRet = strData.Mid(nLeft+1, nRight-nLeft-1);								
					//				if (strRet != "0")
					//				{
					//					return -2;
					//				}
					//				bRet = true;
					//				break;
					//			}							
					//		}
					//	}
					//}

					Sleep(10);
					memset(szBuff, '\0', sizeof(szBuff));
					nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
					CString strRecv = szBuff;
					strData += BufferReplace(strRecv);
					//strData += BufferFilter(strRecv.GetBuffer(0));

					if (time(NULL) - t >= nTimeout)
					{
						strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,nTimeout,strData);
						g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
						return -2; 
					}
				}
			}
			else if((strData.Find("³É¹¦") != -1 ||strData.Find("¸üÐÂ") != -1))
			{
				bRet = true;
			}
			else
			{
				//¼ÌÐø½ÓÊÕÖ±µ½³¬Ê±
				Sleep(10);
				memset(szBuff,'\0',1024+1);
				nRecvLen = recv(m_hSocket,szBuff,1024,0);

				if (time(NULL) - t >= nTimeout)
				{
					strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					return -2; 
				}

				continue;
			}

			if(bRet)
			{
				strLog.Format("[%s][CTelnet::Send][%s] Ö´ÐÐ³É¹¦",m_strThreadName,strData);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
				return strData.GetLength();
			}
			else
			{
				strLog.Format("[%s][CTelnet::Send][%s] Ö´ÐÐÊ§°Ü",m_strThreadName,strData);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
				return -2;
			}
		}

		Sleep(10);
		memset(szBuff,'\0',1024+1);
		nRecvLen = recv(m_hSocket,szBuff,1024,0);

		if (time(NULL) - t >= nTimeout)
		{
			strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return -2; 
		}
	}
	if (nRecvLen == -1)
	{
		strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕÎª¿Õ",m_strThreadName,strCmd);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return -2;
	}
	g_pLog->WriteLog(strData,m_strLogFile,LOG_INFO);
	return strData.GetLength();
}

int CTelnet::SendCMD(CString strCmd,CString& strData,bool bIsSend)
{
	CString strLog;
	if (!bIsSend)
	{
		strLog.Format("[%s][CTelnet::Send] ²»·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		strData = "RETCODE = 0";
		return strData.GetLength();
	}

	//Èç¹ûÎªwindows,·¢ËÍÖ¸ÁîÐèÒª¼Ó»Ø³µ
	//if(g_bIsTest)g_pConfig->m_nEnter = 0;
	CString newCmd = strCmd + (g_pConfig->m_nEnter ? "\r\n" : "");
	int nLen = send(m_hSocket,newCmd.GetBuffer(),newCmd.GetLength(),0);

	if (nLen <= 0)
	{
		strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ·¢ËÍÊ§°Ü",m_strThreadName,strCmd);
		CheckLoginMsg(strLog);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return -1;//·¢ËÍÊ§°Ü
	}
	strLog.Format("[%s][CTelnet::Send] ·¢ËÍÖ¸Áî=[%s]\n",m_strThreadName,strCmd);
	CheckLoginMsg(strLog);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	//Çå¿Õ½ÓÊÕÊý¾Ý
	strData.Empty();
	//ÉèÖÃ½ÓÊÕ³¬Ê±Ê±¼ä
	int nTimeout(m_nTimeout);
	if (newCmd.Find("SYNC:POS=") != -1)
	{
		nTimeout = 600;
	}

	time_t t = time(NULL);
	char szBuff[1024+1];//1k
	bool bReMark(false);
	CString strTemp;
	int nRecvLen = recv(m_hSocket,szBuff,1024,0);
	while (nRecvLen > 0)
	{
		CString strRecv = szBuff;
		strData += BufferFilter(strRecv.GetBuffer(0));

		if (strData.Right(2) == "$>")
		{
			bool bRet(false);
			//strData.Replace("\r\n","");
			CString strCheck(strData);
			strCheck.Remove('\n');

/*			if (strCmd.Find("RELEASE MUTEXRIGHT") != -1)
			{
				if (strData.Find("ÊÍ·Å»¥³âÈ¨ÏÞ³É¹¦") != -1)
				{
					bRet = true;
					break;
				}

				Sleep(500);
				memset(szBuff, '\0', sizeof(szBuff));
				nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
				CString strRecv = szBuff;
				strData += BufferFilter(strRecv.GetBuffer(0));

				if (time(NULL) - t >= nTimeout)
				{
					strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					return -2; 
				}
			}
			else */if (strCmd.Find("SYNC:POS") != -1)
			{
				//  ·¢ËÍÍ¬²½Ö¸Áî, µÈ´ý½ÓÊÕÒì²½·µ»ØÐÅÏ¢
				strLog.Format("[%s][Telnet::Send] ·¢ËÍÍ¬²½Ö¸Áî, µÈ´ý½ÓÊÕÒì²½ÐÞ¸Ä½ø¶ÈÐÅÏ¢.",m_strThreadName);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
				while(nRecvLen > 0)
				{
					if (strData.Find("ÃüÁîÖ´ÐÐÊ§°Ü") != -1)
					{
						return -3;
					}
					// µ±½ÓÊÕµ½Ä©Î²Îª "Êý¾ÝÍ¬²½  (×ÜÊý:1 ³É¹¦:1 Ê§°Ü:0)È«²¿³É¹¦[100]" ±íÊ¾½áÊø
					int nPos = 0;
					nPos = strData.Find("Êý¾ÝÍ¬²½", nPos);
					if (nPos != -1)
					{
						nPos = strData.Find("×ÜÊý", nPos);
						if (nPos != -1)
						{
							nPos = strData.Find("³É¹¦", nPos);
							if (nPos != -1)
							{
								nPos = strData.Find("Ê§°Ü", nPos);
								if (nPos != -1)
								{
									int nLeft = 0;
									nLeft = strData.Find(":", nPos);
									int nRight = 0;
									nRight = strData.Find(")", nLeft);
									CString strRet = strData.Mid(nLeft+1, nRight-nLeft-1);								
									if (strRet != "0")
									{
										return -2;
									}
									bRet = true;
									break;
								}							
							}
						}
					}

					Sleep(500);
					memset(szBuff, '\0', sizeof(szBuff));
					nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
					CString strRecv = szBuff;
					strData += BufferFilter(strRecv.GetBuffer(0));

					if (time(NULL) - t >= nTimeout)
					{
						strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
						g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
						return -2; 
					}
				}
			}
			else if(strCheck.Find(strCmd) != -1 && (strData.Find("³É¹¦") != -1 ||strData.Find("¸üÐÂ") != -1))
			{
				bRet = true;
			}
			else
			{
				//¼ÌÐø½ÓÊÕÖ±µ½³¬Ê±
				Sleep(10);
				memset(szBuff,'\0',1024+1);
				nRecvLen = recv(m_hSocket,szBuff,1024,0);

				if (time(NULL) - t >= nTimeout)
				{
					strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					return -2; 
				}

				continue;
			}

			if(bRet)
			{
				strLog.Format("[%s][CTelnet::Send][%s] Ö´ÐÐ³É¹¦",m_strThreadName,strData);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
				return strData.GetLength();
			}
			else
			{
				strLog.Format("[%s][CTelnet::Send][%s] Ö´ÐÐÊ§°Ü",m_strThreadName,strData);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
				return -2;
			}
		}
		
		Sleep(10);
		memset(szBuff,'\0',1024+1);
		nRecvLen = recv(m_hSocket,szBuff,1024,0);

		if (time(NULL) - t >= nTimeout)
		{
			strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕ³¬Ê±,³¬Ê±Ê±¼ä=%d s\n%s",m_strThreadName,strCmd,m_nTimeout,strData);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return -2; 
		}
	}
	if (nRecvLen == -1)
	{
		strLog.Format("[%s][CTelnet::Send] Ö¸Áî=[%s] ½ÓÊÕÎª¿Õ",m_strThreadName,strCmd);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return -2;
	}
	g_pLog->WriteLog(strData,m_strLogFile,LOG_INFO);
	return strData.GetLength();
}

bool CTelnet::Login(CString strIP,int nPort,CString strUser,CString strPassword,int nLoginType)
{
	CString strLog;
	//´´½¨socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		strLog = "[CTelnet::Login] socket(AF_INET,SOCK_STREAM,IPPROTO_IP) Error!\n";
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	//¹¹ÔìµØÖ·ºÍ¶Ë¿Ú
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strIP);

	//Á¬½Óµ½·þÎñÆ÷
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		CString strLog;
		strLog.Format("[%s][CTelnet::Login] Connect [%s:%d] Error\n",m_strThreadName,strIP,nPort);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	m_bExit = false;
	int nNetTimeout =  30*1000;//30Ãë£¬
	//ÉèÖÃ½ÓÊÕ³¬Ê±
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));

	return SendLoginInfo(strUser, strPassword);

}


bool CTelnet::MMLLogin(CString strIP,int nPort, CString strName,
					   CString strUser,CString strPassword,int nLoginType)
{
	CString strLog;
	//´´½¨socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		strLog = "[CTelnet::MMLLogin] socket(AF_INET,SOCK_STREAM,IPPROTO_IP) Error!\n";
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	//¹¹ÔìµØÖ·ºÍ¶Ë¿Ú
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strIP);

	//Á¬½Óµ½·þÎñÆ÷
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		CString strLog;
		strLog.Format("[%s][CTelnet::MMLLogin] Connect [%s:%d] Error\n",m_strThreadName,strIP,nPort);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	m_bExit = false;
	int nNetTimeout =  30*1000;//30Ãë£¬
	//ÉèÖÃ½ÓÊÕ³¬Ê±
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));

	//¹¹ÔìµÇÂ¼×Ö·û´®
	CString strLogin;
	strLogin.Format("LGI:OP=\"%s\",PWD=\"%s\",DN=EMS;",strUser,strPassword);

	CString strRecv("");
	bool bRes(false);
	int nRet = Send(strLogin,strRecv,CHECK_LOGIN);
	if (-1 != strRecv.Find("RETCODE = 0") && -1 != strRecv.Find("LGI:OP"))
	{
		strLog.Format("[%s][CTelnet::MMLLogin] µÇÂ¼M2000³É¹¦\n",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		bRes = true;
	}

	if(!bRes)
	{
		//µÇÂ¼Ê§°Ü£¬Ôò¹Ø±ÕSOCKET
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		strLog.Format("[%s][CTelnet::MMLLogin] µÇÂ¼M2000Ê§°Ü-->[%s:%d]\n%s\n",m_strThreadName,strIP,nPort,strLogin);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	//×¢²áÍøÔª
	CString strRegNe;
	strRegNe.Format("REG NE:NAME=\"%s\";",strName);
	strRecv = "";
	nRet = Send(strRegNe,strRecv,CHECK_MMLREG);
	if (-1 != strRecv.Find("RETCODE = 0"))
	{
		strLog.Format("[%s][CTelnet::MMLLogin] ×¢²áÍøÔª³É¹¦\n",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		return true;//×¢²áÍøÔª³É¹¦
	}

	//µÇÂ¼Ê§°Ü£¬Ôò¹Ø±ÕSOCKET
	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
	strLog.Format("[%s][CTelnet::MMLLogin] ×¢²áÍøÔªÊ§°Ü-->[%s:%d]\n%s\n",m_strThreadName,strIP,nPort,strLogin);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	return false;
}

//Ê¹ÓÃ23¶Ë¿Ú½øÐÐtelnetµÇÂ½
bool CTelnet::Login(CString& strIP,const CString& strUser,const CString& strPassword)
{
	CString strLog;
	//´´½¨socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		strLog = "[CTelnet::Login] socket(AF_INET,SOCK_STREAM,IPPROTO_IP) Error!\n";
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	//¹¹ÔìµØÖ·ºÍ¶Ë¿Ú
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(23);
	addr.sin_addr.s_addr = inet_addr(strIP);

	//Á¬½Óµ½·þÎñÆ÷
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		CString strLog;
		strLog.Format("[%s][CTelnet::Login] Connect [%s:%d] Error\n",m_strThreadName,strIP,23);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	m_bExit = false;
	int nNetTimeout =  30*1000;//30Ãë£¬
	//ÉèÖÃ½ÓÊÕ³¬Ê±
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));

	return SendLoginInfo(strUser, strPassword);
}

bool CTelnet::SendMsg(const CString& strMsg)
{
    CString strLog;
	CString newCmd = strMsg + (g_pConfig->m_nEnter ? "\r\n" : "");

	int nLen = send(m_hSocket,newCmd.GetBuffer(0),newCmd.GetLength(),0);
	if (nLen <= 0)
	{
		strLog.Format("[%s][CTelnet::SendLoginInfo] ÏûÏ¢=[%s] ·¢ËÍÊ§°Ü",m_strThreadName,newCmd);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;//·¢ËÍÊ§°Ü
	}
	strLog.Format("[%s][CTelnet::SendLoginInfo] ÏûÏ¢=[%s] ·¢ËÍ",m_strThreadName,newCmd);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	return true;
}

bool CTelnet::SendLoginInfo(const CString& strUser,const CString& strPassword)
{
	bool bRet = false;
	bool bLoginSuccess = false;

	int nRecvTimes(0);
	DWORD d1 = GetTickCount();
	while(nRecvTimes < 100)
	{
		nRecvTimes++;

		//-------------------------½ÓÊÕ»Ø¸´,°Ñ·µ»ØµÄÐÅÏ¢´æ·Åµ½m_strNormalTextÖÐ-------------------MSG_WAITALL

		char szBuf[1024] = {0};
		int nBytes = recv(m_hSocket, szBuf, sizeof(szBuf),0);


		if ( nBytes <= 0 )
		{
			DWORD d2 = GetTickCount();
			if (d2 - d1 > 110*1000)      //ÔÊÐí110ÃëµÄÁ¬½ÓÊ±¼ä
			{
				return false;
			}
			if (WSAETIMEDOUT != WSAGetLastError())//ÓÉÓÚÁ¬½Ó·½ÔÚÒ»¶ÎÊ±¼äºóÃ»ÓÐÕýÈ·´ð¸´»òÁ¬½ÓµÄÖ÷»úÃ»ÓÐ·´Ó¦£¬Á¬½Ó³¢ÊÔÊ§°Ü¡£ 
			{
				return false;
			}
		}	

		//-------------------------½ÓÊÕ»Ø¸´,°Ñ·µ»ØµÄÐÅÏ¢´æ·Åµ½m_strNormalTextÖÐ-------------------
		CString strMessage(szBuf);
		ProcessOptions(strMessage);
		if (-1 != strMessage.Find("Login ok"))
		{
			bLoginSuccess = true;
		}
		g_pLog->WriteLog(strMessage,m_strLogFile,LOG_INFO);


		CString strtemp = strMessage.Right(7);
		strtemp.Trim();
		strtemp.Replace("\r","");
		if (-1!=strMessage.Find(">") || -1 != strMessage.Find("<") || -1!=strMessage.Find("Microsoft Corp.") || -1!=strMessage.Find("Last login"))//µÇÂ¼Íê±Ï³É¹¦
		{
			if (-1!=strMessage.Find(">") || -1 != strMessage.Find("<"))
			{
				return true;
			}
		}
		else if (-1 !=strMessage.Find("username:"))//ÐèÒª·¢ËÍÕËºÅ
		{
			SendMsg(strUser);
		}
		else if (("login:"==strtemp || -1!=strMessage.Find("login name:")) 
			&& (-1==strMessage.Find("Last login:")))//ÐèÒª·¢ËÍÕËºÅ
		{
			SendMsg(strUser);
		}
		else if (-1 != strMessage.Find("password:")|| -1 != strMessage.Find("Password:"))//ÐèÒª·¢ËÍÃÜÂë
		{
			SendMsg(strPassword);
		}
		else if (-1 != strMessage.Find("incorrect"))//µÇÂ¼Ê§°Ü
		{
			return false;
		}
		else if (-1 != strMessage.Find("Domain:"))
		{
			SendMsg("");//·¢ËÍ»Ø³µ
		}
	}
	return false;
}

bool CTelnet::Logout()
{
	m_bExit = true;
	if (m_hSocket != INVALID_SOCKET)
	{
		send(m_hSocket,"LGO:;",(int)strlen("LGO:;"),0);
		CString strLog;
		strLog.Format("[%s][CTelnet::Logout] µÇ³öOMMB\n",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		return true;
	}
	return false;
}

bool CTelnet::MMLLogout(CString strName,CString strUser)
{
	CString strRecv("");
	CString strLog;
	CString strUnReg;
	CString strLogOut;

	if (m_hSocket != INVALID_SOCKET)
	{
		//×¢ÏúÍøÔª
		strUnReg.Format("UNREG NE:NAME=\"%s\";",strName);

		int nRet = Send(strUnReg,strRecv,CHECK_MMLUNREG);
		if (-1 == strRecv.Find("RETCODE = 0"))
		{
			strLog.Format("[%s][CTelnet::MMLLogout] ×¢ÏúÍøÔªÊ§°Ü Name:[%s]\n",m_strThreadName,strName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			return false;
		}

		//µÇ³öM2000
		strRecv = "";
		strLogOut.Format("LGO:OP=\"%s\";",strUser);
		nRet = Send(strLogOut,strRecv,CHECK_LOGOUT);
		if(-1 == strRecv.Find("RETCODE = 0"))
		{
			strLog.Format("[%s][CTelnet::MMLLogout] µÇ³öM2000Ê§°Ü %s\n",m_strThreadName,strLogOut);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			return false;
		}

		strLog.Format("[%s][CTelnet::MMLLogout] µÇ³öM2000\n",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		return true;
	}
	return false;
}

CString CTelnet::ProcessOptions(CString& strmsg)
{
	int ndx;
	int ldx;
	unsigned char ch;
	BOOL OMMBanDone = FALSE;
	CString strOption;
	CStringList strListOptions;

	CString strtemp = strmsg;
	strmsg.Empty();

	while(!strtemp.IsEmpty() && !OMMBanDone)
	{
		ndx = strtemp.Find(IAC);
		if (-1 != ndx)
		{
			strmsg += strtemp.Left(ndx);
			ch = strtemp.GetAt(ndx + 1);
			switch (ch)
			{
			case DO:
			case DONT:
			case WILL:
			case WONT:
				{
					strOption = strtemp.Mid(ndx, 3);
					strtemp	= strtemp.Mid(ndx + 3);
					strmsg	= strtemp.Left(ndx);
					strListOptions.AddTail(strOption);
				}
				break;
			case IAC:
				{
					strmsg	= strtemp.Left(ndx);
					strtemp	= strtemp.Mid(ndx + 1);
				}
				break;
			case SB:
				{
					strmsg = strtemp.Left(ndx);
					ldx = strtemp.Find(SE);
					strOption = strtemp.Mid(ndx, ldx);
					strListOptions.AddTail(strOption);
					strtemp	= strtemp.Mid(ldx);
				}
				break;
			}
		}
		else
		{
			strmsg = strtemp;
			OMMBanDone = TRUE;
		}
	} 

	//´ð¸´
	strtemp.Empty();
	while (!strListOptions.IsEmpty())
	{
		strOption = strListOptions.RemoveHead();
		strtemp += ArrangeReply(strOption);
	}
	return strtemp;
}

CString CTelnet::ArrangeReply(CString& strOption)
{
	unsigned char Verb;
	unsigned char Option;
	unsigned char Modifier;
	unsigned char ch;
	BOOL bDefined = FALSE;
	CString strResp;

	Verb = strOption.GetAt(1);
	Option = strOption.GetAt(2);

	switch (Option)
	{
	case 1:	// Echo
	case 3: // Suppress Go-Ahead
		{
			bDefined = TRUE;
		}
		break;
	default:
		break;
	}

	strResp += IAC;

	if(TRUE == bDefined)
	{
		switch (Verb)
		{
		case DO:
			{
				ch = WILL;
				strResp += ch;
				strResp += Option;
			}
			break;
		case DONT:
			{
				ch = WONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		case WILL:
			{
				ch = DO;
				strResp += ch;
				strResp += Option;
			}
			break;
		case WONT:
			{
				ch = DONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		case SB:
			{
				Modifier = strOption.GetAt(3);
				if(SEND == Modifier)
				{
					ch = SB;
					strResp += ch;
					strResp += Option;
					strResp += IS;
					strResp += IAC;
					strResp += SE;
				}
			}
			break;
		default:
			break;
		}
	}
	else
	{
		switch (Verb)
		{
		case DO:
			{
				ch = WONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		case DONT:
			{
				ch = WONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		case WILL:
			{
				ch = DONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		case WONT:
			{
				ch = DONT;
				strResp += ch;
				strResp += Option;
			}
			break;
		default:
			break;
		}
	}
	return strResp;
}

bool CTelnet::ConnectServer(CString strIP, int nPort)
{
	CString strLog;
	//´´½¨socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		g_pLog->WriteLog("socket(AF_INET,SOCK_STREAM,IPPROTO_IP) Error!",m_strLogFile,LOG_ERROR);
		return false;
	}

	//¹¹ÔìµØÖ·ºÍ¶Ë¿Ú
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strIP);

	//Á¬½Óµ½·þÎñÆ÷
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		strLog.Format("[%s][CTelnet::ConnectServer] Connect [%s:%d] Error\n",m_strThreadName,strIP,nPort);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	int nNetTimeout =  30*1000;//10Ãë£¬
	//ÉèÖÃ½ÓÊÕ³¬Ê±
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));

	return true;
}

int CTelnet::Writen(const void* pBuf, int nLen)
{
	int nLeft;
	int nWritten;
	const char *ptr = (const char*)pBuf;
	nLeft = nLen;
	while(nLeft > 0)
	{
		nWritten = ::send(m_hSocket, ptr, nLeft, 0);
		if (nWritten <= 0) 
		{
			break;
		}
		nLeft -= nWritten;
		ptr += nWritten;
	}
	return nLen - nLeft;
}

int CTelnet::Readen(void* pBuf, int nLen)
{
	int nLeft;
	int nRead;
	char *ptr = (char*)pBuf;
	nLeft = nLen;
	while(nLeft > 0)
	{
		nRead = recv(m_hSocket, ptr, nLeft, 0);
		if (nRead <= 0)
		{
			//³¬Ê±,Ö±½Ó·µ»Ø
			break;
		}
		nLeft -= nRead;
		ptr += nRead;
	}
	return nLen - nLeft;
}

bool CTelnet::ERICLogin(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode)
{
	bool bRet = false;
	try
	{
		//´´½¨SOCK£¬Á¬½Ó·þÎñÆ÷
		if (!ConnectERICServ(strHost,nPort))
		{
			return false;
		}

		//µÇÂ¼Ê§°Ü
		if (!SendERICLoginInfo(strAccount,strPassword,nMode))
		{
			return false;
		}

		bRet = true;
	}
	catch (...)
	{

	}

	return bRet;
}

bool CTelnet::ConnectERICServ(CString strHost,int nPort)
{
	CString strLog;
	//´´½¨socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		strLog = "[CTelnet::ConnectERICServ] socket(AF_INET,SOCK_STREAM,IPPROTO_IP) Error!\n";
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	g_pLog->WriteLog("[CTelnet::ConnectERICServ] socket(AF_INET,SOCK_STREAM,IPPROTO_IP) ³É¹¦",m_strLogFile,LOG_INFO);
	//¹¹ÔìµØÖ·ºÍ¶Ë¿Ú
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strHost);

	//Á¬½Óµ½·þÎñÆ÷
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		strLog.Format("[%s][CTelnet::ConnectERICServ] Connect [%s:%d] Error\n",m_strThreadName,strHost,nPort);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	strLog.Format("[%s][CTelnet::ConnectERICServ] Connect [%s:%d] ³É¹¦\n",m_strThreadName,strHost,nPort);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	int nNetTimeout =  30*1000;//10Ãë£¬
	//ÉèÖÃ½ÓÊÕ³¬Ê±
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));

	return true;
}

bool CTelnet::SendERICLoginInfo(CString strAccount,CString strPassword,int nMode)
{
	bool bRet = false;
	bool bLoginSuccess = false;

	int nRecvTimes(0);

	while(nRecvTimes < 100)
	{
		nRecvTimes++;

		//-------------------------½ÓÊÕ»Ø¸´,°Ñ·µ»ØµÄÐÅÏ¢´æ·Åµ½m_strNormalTextÖÐ-------------------MSG_WAITALL
		DWORD d1 = GetTickCount();
		int nBytes = recv(m_hSocket, (char *)m_bBuf.GetData(), (int)m_bBuf.GetSize(),0);
		DWORD d2 = GetTickCount();

		if (-1 == nBytes)
		{
			if (10060 != WSAGetLastError())//ÓÉÓÚÁ¬½Ó·½ÔÚÒ»¶ÎÊ±¼äºóÃ»ÓÐÕýÈ·´ð¸´»òÁ¬½ÓµÄÖ÷»úÃ»ÓÐ·´Ó¦£¬Á¬½Ó³¢ÊÔÊ§°Ü¡£ 
			{
				return false;
			}
		}	

		int ndx = 0;
		//modified by linzc 130718 
		GetLine(m_bBuf, nBytes, ndx);
		//while(TRUE != GetLine(m_bBuf, nBytes, ndx))
		//{
		//	Sleep(1);
		//}
		//if(g_bIsTest)
		//{
		//	m_strLine.Empty();
		//	m_strLine += IAC;
		//	m_strLine += DO;
		//	m_strLine += MAXTEST;
		//}
		ERICProcessOptions();
		m_strLine.Empty();
		m_strResp.Empty();
		OutputDebugString(m_strNormalText);
		//-------------------------½ÓÊÕ»Ø¸´,°Ñ·µ»ØµÄÐÅÏ¢´æ·Åµ½m_strNormalTextÖÐ-------------------

		if (-1 != m_strNormalText.Find("Login ok"))
		{
			bLoginSuccess = true;
		}
		g_pLog->WriteLog(m_strNormalText,m_strLogFile,LOG_INFO);

		CString strtemp = m_strNormalText.Right(7);
		strtemp.Trim();
		strtemp.Replace("\r","");
		if (-1!=m_strNormalText.Find(">") || -1 != m_strNormalText.Find("<") || -1!=m_strNormalText.Find("Microsoft Corp.") || -1!=m_strNormalText.Find("Last login"))//µÇÂ¼Íê±Ï³É¹¦
		{
			if(0 == nMode)
			{
				if (-1!=m_strNormalText.Find(">") || -1 != m_strNormalText.Find("<"))
				{
					return true;
				}
			}
			else
			{
				if (bLoginSuccess)
				{
					return true;
				}
				else
				{
					m_nLoginTime++;
				}
			}
		}
		else if (m_nLoginTime >= 3)//³¢ÊÔÈý´ÎloginÊ§°Ü
		{
			return false;
		}
		else if (("login:"==strtemp || -1!=m_strNormalText.Find("login name:")) 
			&& (-1==m_strNormalText.Find("Last login:")))//ÐèÒª·¢ËÍÕËºÅ
		{
			SendMsg(strAccount);
		}
		else if (-1 != m_strNormalText.Find("assword:"))//ÐèÒª·¢ËÍÃÜÂë
		{
			SendMsg(strPassword);
			m_nLoginTime++;
		}
		else if (-1 != m_strNormalText.Find("incorrect"))//µÇÂ¼Ê§°Ü
		{
			m_nLoginTime++;
		}
		else if (-1 != m_strNormalText.Find("Domain:"))
		{
			SendMsg("");//·¢ËÍ»Ø³µ
		}

		//½ÓÊÕÊ±¼ä³¬¹ý9s
		if ((d2-d1)/1000>(30/*g_pConfig->nTimeOut*/))
		{
			return false;
		}
	}

	return bRet;
}


bool CTelnet::GetLine(const CByteArray &bytes, int nBytes, int &ndx)
{
	bool bLine = false;

	try
	{
		if (nBytes<=0)
			return true;

		while (false==bLine && ndx<nBytes)
		{
			unsigned char ch = (char)(bytes.GetAt(ndx));

			switch (ch)
			{
			case '\r':// ignore
				{
					m_strLine += "\r\n"; //"CR";
				}
				break;
			case '\n':// end-of-line
				{
				}
				break;
			default:// other....
				{
					m_strLine += ch;
				}
				break;
			} 

			++ndx;

			if (ndx == nBytes)
				bLine = true;
		}
	}
	catch (...)
	{
	}

	return bLine;
}

void CTelnet::ERICProcessOptions()
{
	try
	{
		CString strLog;
		CString strTemp;
		CString strOption;
		unsigned char ch;
		int ndx;
		int ldx;
		BOOL bScanDone = FALSE;

		strTemp = m_strLine;
		strLog.Format("[%s][CTelnet::ERICProcessOptions] m_strLine [%x] \n",m_strThreadName,m_strLine);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		while(!strTemp.IsEmpty() && TRUE!=bScanDone)
		{
			ndx = strTemp.Find(IAC);
			if (-1 != ndx)
			{
				m_strNormalText += strTemp.Left(ndx);
				ch = strTemp.GetAt(ndx + 1);
				switch (ch)
				{
				case DO:
				case DONT:
				case WILL:
				case WONT:
					{
						strOption = strTemp.Mid(ndx, 3);
						strTemp	= strTemp.Mid(ndx + 3);
						m_strNormalText	= strTemp.Left(ndx);
						m_strListOptions.AddTail(strOption);
						g_pLog->WriteLog("[CTelnet::ERICProcessOptions] WONT",m_strLogFile,LOG_INFO);
					}
					break;
				case IAC:
					{
						m_strNormalText	= strTemp.Left(ndx);
						strTemp	= strTemp.Mid(ndx + 1);
						g_pLog->WriteLog("[CTelnet::ERICProcessOptions] IAC",m_strLogFile,LOG_INFO);
					}
					break;
				case SB:
					{
						m_strNormalText = strTemp.Left(ndx);
						ldx = strTemp.Find(SE);
						strOption = strTemp.Mid(ndx, ldx);
						m_strListOptions.AddTail(strOption);
						strTemp	= strTemp.Mid(ldx);
						g_pLog->WriteLog("[CTelnet::ERICProcessOptions] SB",m_strLogFile,LOG_INFO);
					}
					break;
				}
			}
			else
			{
				m_strNormalText = strTemp;
				bScanDone = TRUE;
			}
		} 

		ERICRespondToOptions();
	}
	catch (...)
	{
	}
}

void CTelnet::ERICRespondToOptions()
{
	try
	{
		CString strOption,strLog;

		while (!m_strListOptions.IsEmpty())
		{
			strOption = m_strListOptions.RemoveHead();
			ERICArrangeReply(strOption);
		}

		strLog.Format("[%s][CTelnet::ERICRespondToOptions] send [%x] \n",m_strThreadName,strOption);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		//·¢ËÍ
		int nSendLen = send(m_hSocket, m_strResp, (int)m_strResp.GetLength(), 0);

		m_strResp.Empty();
	}
	catch (...)
	{
	}
}

void CTelnet::ERICArrangeReply(CString strOption)
{
	CString strLog;
	try
	{
		unsigned char Verb;
		unsigned char Option;
		unsigned char Modifier;
		unsigned char ch;
		BOOL bDefined = FALSE;

		Verb = strOption.GetAt(1);
		Option = strOption.GetAt(2);
		strLog.Format("[%s][CTelnet::ERICArrangeReply] Verb [%02x] ,Verb [%02x] \n",m_strThreadName,Verb,Option);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);

		switch (Option)
		{
		case 1:	// Echo
		case 3: // Suppress Go-Ahead
			{
				bDefined = TRUE;
			}
			break;
		default:
			break;
		}

		m_strResp += IAC;

		if(TRUE == bDefined)
		{
			switch (Verb)
			{
			case DO:
				{
					ch = WILL;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case DONT:
				{
					ch = WONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case WILL:
				{
					ch = DO;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case WONT:
				{
					ch = DONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case SB:
				{
					Modifier = strOption.GetAt(3);
					if(SEND == Modifier)
					{
						ch = SB;
						m_strResp += ch;
						m_strResp += Option;
						m_strResp += IS;
						m_strResp += IAC;
						m_strResp += SE;
					}
				}
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Verb)
			{
			case DO:
				{
					ch = WONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case DONT:
				{
					ch = WONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case WILL:
				{
					ch = DONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			case WONT:
				{
					ch = DONT;
					m_strResp += ch;
					m_strResp += Option;
				}
				break;
			default:
				break;
			}
		}
	}
	catch (...)
	{
	}
}

CString CTelnet::BufferFilter( char* pRecBuffer )
{
	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] ¹ýÂËÌØÊâ×Ö·û¿ªÊ¼");

	CString strReceive;
	BOOL bStartFlag = false;
	BOOL bEndFlag = false;
	size_t nLen = strlen(pRecBuffer);

	for (size_t n = 0; n < nLen; n++)
	{
		size_t next1 = n + 1; //91
		size_t next2 = n + 2; //50
		size_t next3 = n + 3; //75
		size_t next4 = n + 4; //27

		if (pRecBuffer[n] == '\x08')
		{// ÍË¸ñ
			continue;
		}

		// \x1b[2K\x1b[D Ìø¹ý 3 ¸ö×Ö·û
		if (next4 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next4] == 27)
		{
			n = next3;
			continue;
		}

		// \x1b[D\x1b[D Ìø¹ý 2 ¸ö×Ö·û
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] == 27)
		{
			n = next2;
			continue;
		}

		// \x1b[D$>D Ìø¹ý 2 ¸ö×Ö·û
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] != 27)
		{
			n = next2;
			continue;
		}

		strReceive.AppendChar(pRecBuffer[n]);
	}

	strReceive.Replace("Çë°´ÈÎÒâ¼ü¼ÌÐø(Ctrl+C ÖÐ¶ÏÏÔÊ¾). . .", NULL);

	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] ¹ýÂËÌØÊâ×Ö·û½áÊø");
	return strReceive;
}

CString CTelnet::BufferReplace(CString & strData)
{
	CString strReceive(strData);
	//Ìæ»»[2K[D
	strReceive.Replace("[2K[D","");
	//Ìæ»»[D[D
	strReceive.Replace("[D[D","");
	//Ìæ»»[D
	strReceive.Replace("[D","");
	return strReceive;
}