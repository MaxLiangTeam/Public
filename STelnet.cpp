#include "StdAfx.h"
#include "STelnet.h"
#include "Kernel.h"

CSTelnet::CSTelnet(void)
{
	//��ʼ��socket
	WSADATA wsaData = {0};
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData );

	m_bBuf.SetSize(1024);
	m_nPort = 23;
	m_nLoginTime = 0;
	m_msgError = MSG_Other;
}

CSTelnet::~CSTelnet(void)
{
	//�ر�SOCK
	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;
	WSACleanup();

	m_vecTelnetData.clear();
	m_vecSendMsg.clear();
}

bool CSTelnet::Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode)
{
	bool bRet = false;

	m_strHost = strHost;
	m_nPort = nPort;
	m_strAccount = strAccount;
	m_strPassword = strPassword;

	try
	{
		//����SOCK�����ӷ�����
		if (!ConnectServ())
		{
			return false;
		}

		//��¼ʧ��
		if (!SendLoginInfo(nMode))
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

bool CSTelnet::ConnectServ()
{
	CString strLog;
	//����socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		strLog.Format("socket()--Faild[%u]-->",GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	//�����ַ�Ͷ˿�
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_nPort);
	addr.sin_addr.s_addr = inet_addr(m_strHost);

	//���ӵ�������
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		strLog.Format("connect()--Faild[%u]-->",GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	
	int nNetTimeout =  30*1000;//10�룬
	//���ý��ճ�ʱ
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
	
	return true;
}

bool CSTelnet::ConnectServer(const char* pszIp, int nPort)
{
	m_strHost = pszIp;
	m_nPort = nPort;
	return ConnectServ();
}

bool CSTelnet::GetLine(const CByteArray &bytes, int nBytes, int &ndx)
{
	bool bLine = false;

	if (nBytes<=0)
		return true;

	while (false==bLine && ndx<nBytes)
	{
		unsigned char ch = (char)(bytes.GetAt(ndx));

		switch (ch)
		{
		case '\r':// ignore
			{
				//erased by linzc 130718
				//m_strLine += "\r\n"; //"CR";
			}
			break;
		case '\n':// end-of-line
			{
				//added by linzc 130718
				m_strLine += "\r\n";
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

	return bLine;
}

void CSTelnet::ProcessOptions()
{
	try
	{
		CString strTemp,strLog;
		CString strOption;
		unsigned char ch;
		int ndx;
		int ldx;
		BOOL bScanDone = FALSE;

		strTemp = m_strLine;
		//strLog.Format("[CTelnet::ERICProcessOptions] m_strLine [%x] \n",m_strLine);
		//g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

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
						//g_pLog->WriteLog("[CTelnet::ERICProcessOptions] WONT",m_strLogFile,LOG_INFO);
					}
					break;
				case IAC:
					{
						m_strNormalText	= strTemp.Left(ndx);
						strTemp	= strTemp.Mid(ndx + 1);
						//g_pLog->WriteLog("[CTelnet::ERICProcessOptions] IAC",m_strLogFile,LOG_INFO);
					}
					break;
				case SB:
					{
						m_strNormalText = strTemp.Left(ndx);
						ldx = strTemp.Find(SE);
						strOption = strTemp.Mid(ndx, ldx);
						m_strListOptions.AddTail(strOption);
						strTemp	= strTemp.Mid(ldx);
						//g_pLog->WriteLog("[CTelnet::ERICProcessOptions] SB",m_strLogFile,LOG_INFO);
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

		RespondToOptions();
	}
	catch (...)
	{
	}
}

void CSTelnet::RespondToOptions()
{
	try
	{
		CString strOption,strLog;

		while (!m_strListOptions.IsEmpty())
		{
			strOption = m_strListOptions.RemoveHead();
			ArrangeReply(strOption);
		}
		//strLog.Format("[CTelnet::ERICRespondToOptions] send [%x] \n",strOption);
		//g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		//����
		int nSendLen = send(m_hSocket, m_strResp, (int)m_strResp.GetLength(), 0);

		m_strResp.Empty();
	}
	catch (...)
	{
	}
}

void CSTelnet::ArrangeReply(CString strOption)
{
	try
	{
		CString strLog;
		unsigned char Verb;
		unsigned char Option;
		unsigned char Modifier;
		unsigned char ch;
		BOOL bDefined = FALSE;

		Verb = strOption.GetAt(1);
		Option = strOption.GetAt(2);
		//strLog.Format("[CTelnet::ERICArrangeReply] Verb [%02x] ,Verb [%02x] \n",Verb,Option);
		//g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
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

void CSTelnet::Analyse(CString strCmd, CString strInfo)
{
	TelnetData sctTemp;

	if (strInfo.GetLength() > 0)
	{
		//sctTemp.strCommand = strInfo.Mid(0, strInfo.Find("\r")+1);
		//sctTemp.strContent = strInfo.Mid(strInfo.Find("\r")+2, strInfo.ReverseFind('\r')-strInfo.Find("\r")-2);
		sctTemp.strCommand = strCmd;
		sctTemp.strContent = strInfo;

		sctTemp.strCommand.Trim();
		sctTemp.strContent.Trim();

		if (""!=sctTemp.strCommand && ""!=sctTemp.strContent)//����"����"��"����"ͬʱ��Ϊ��ʱ��������
			m_vecTelnetData.push_back(sctTemp);
	}
}

void CSTelnet::PushMsg( CString strMsg )
{
	m_vecSendMsg.push_back(strMsg);
}



void CSTelnet::SetSendEvent()
{
	m_hEvent.SetEvent();
}

void CSTelnet::DoSendMsg(CString strMsg)
{
	CString strLog;
	strLog.Format("send command %s",strMsg);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	//added by linzc 130220 ���Ϊ04 �򲻼�\r\n
	if (strMsg.Find(04) == -1)
		strMsg += "\r\n";
	int nLen = strMsg.GetLength();
	send(m_hSocket,strMsg.GetBuffer(0),nLen,0);
	strMsg.ReleaseBuffer();
}

bool CSTelnet::SendLoginInfo(int nMode)
{
	bool bRet = false;
	bool bLoginSuccess = false;

	int nRecvTimes(0);

	while(nRecvTimes < 100)
	{
		nRecvTimes++;

		//-------------------------���ջظ�,�ѷ��ص���Ϣ��ŵ�m_strNormalText��-------------------MSG_WAITALL
		DWORD d1 = GetTickCount();
		int nBytes = recv(m_hSocket, (char *)m_bBuf.GetData(), (int)m_bBuf.GetSize(),0);
		DWORD d2 = GetTickCount();
		
		if (-1 == nBytes)
		{
			if (10060 != WSAGetLastError())//�������ӷ���һ��ʱ���û����ȷ�𸴻����ӵ�����û�з�Ӧ�����ӳ���ʧ�ܡ� 
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
		ProcessOptions();
		m_strLine.Empty();
		m_strResp.Empty();
		OutputDebugString(m_strNormalText);
		//-------------------------���ջظ�,�ѷ��ص���Ϣ��ŵ�m_strNormalText��-------------------
		if (-1 != m_strNormalText.Find("Login ok"))
		{
			bLoginSuccess = true;
		}
		g_pLog->WriteLog(m_strNormalText,m_strLogFile,LOG_INFO);

		CString strtemp = m_strNormalText.Right(7);
		strtemp.Trim();
		strtemp.Replace("\r","");
		if (-1!=m_strNormalText.Find(">") || -1 != m_strNormalText.Find("<") || -1!=m_strNormalText.Find("Microsoft Corp.") || -1!=m_strNormalText.Find("Last login"))//��¼��ϳɹ�
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
		else if (m_nLoginTime >= 3)//��������loginʧ��
		{
			return false;
		}
		else if (("login:"==strtemp || -1!=m_strNormalText.Find("login name:")) 
			&& (-1==m_strNormalText.Find("Last login:")))//��Ҫ�����˺�
		{
			DoSendMsg(m_strAccount);
		}
		else if (-1 !=m_strNormalText.Find("username:"))//��Ҫ�����˺�
		{
			DoSendMsg(m_strAccount);
		}
		else if (-1 != m_strNormalText.Find("assword:"))//��Ҫ��������
		{
			DoSendMsg(m_strPassword);
			m_nLoginTime++;
		}
		else if (-1 != m_strNormalText.Find("incorrect"))//��¼ʧ��
		{
			m_nLoginTime++;
		}
		else if (-1 != m_strNormalText.Find("Domain:"))
		{
			DoSendMsg("");//���ͻس�
		}
		
		//����ʱ�䳬��9s
		if ((d2-d1)/1000>(30/*g_pConfig->nTimeOut*/))
		{
			return false;
		}
	}

	return bRet;
}

bool CSTelnet::Send()
{
	CString strLog;
	bool bRet = true;
	m_vecTelnetData.clear();//��������
	if ((m_hSocket==INVALID_SOCKET) || (m_hSocket == 0))
	{
		strLog.Format("Send() m_hSocket is invalid");
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_DEBUG);
		return false;
	}

	try
	{
		int nPos = 0;

		for (vector<CString>::iterator iter = m_vecSendMsg.begin();iter != m_vecSendMsg.end();iter++)
		{
			nPos++;
			if (m_hSocket != INVALID_SOCKET)
				DoSendMsg(*iter);

			if (*iter == "exit;")
			{//������˳����ֱ�ӷ��� added by linzc for ��Щ�豸��exit;�������ǳ�ʱ
				closesocket(m_hSocket);
				m_hSocket = 0;
				return true;
			}

			CString strTotalRecv;
			char szBuff[1024+1];//1k
			memset(szBuff,'\0',1024+1);

			long lT1 = GetTickCount();
			long lT2 = 0;
			int nRecvLen = recv(m_hSocket,szBuff,1024,0);
			while(nRecvLen > 0)
			{
				CString strRecv = szBuff;
				strRecv.Remove(3);//added by linzc to remove ascii 3 "ext"
				strTotalRecv += strRecv;

				int nLen = strTotalRecv.GetLength();
				//�������,����С��1024��Ϊ��,����������ҵ��ַ���\n>,�ʹ���������
				if (strTotalRecv.Find(">") != -1 || strTotalRecv.Find("<") != -1 || 
					strTotalRecv.Find("\nEND>") != -1 || strTotalRecv.Find("\nEND<") != -1)/* || (-1!=strJudge.Find("COMMAND SESSION SUSPENDED")&&-1!=strJudge.Find("END")))*/
				{
					g_pLog->WriteLog(strTotalRecv,m_strLogFile,LOG_DEBUG);

					if (-1 != strTotalRecv.Find("INFO: Connection failed."))//telnet Bsc faild
					{
					}
					else
					{					
						if (strTotalRecv.Find("%") != -1)
							strTotalRecv.Replace("%", "%%");
						Analyse(*iter, strTotalRecv);//��������

						strTotalRecv.Empty();
					}
					break;
				}

				Sleep(10);
				memset(szBuff,'\0',1024+1);
				nRecvLen = recv(m_hSocket,szBuff,1024,0);

				lT2 = GetTickCount();
				if ((*iter).Find(04) != -1)
				{
					if ((lT2 - lT1)/1000 >= g_pConfig->m_nRecvTimeout) 
						break;//����funbusy�ͷŷ���ʱ�ӣ����˳���׼�����뷢��\r����
				}

				if ((lT2 - lT1)/1000>g_pConfig->m_nRecvTimeout) //����ָ��ʱ��
				{
					//m_pLog->Error("recv command %s timeout(%ds), recvd msg:%s",
					//	*iter, g_pConfig->m_nRecvTimeout, strTotalRecv);
					strLog.Format("recv command %s timeout(%ds), recvd msg:%s",
						*iter, g_pConfig->m_nRecvTimeout, strTotalRecv);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
					strTotalRecv.Empty();
					closesocket(m_hSocket);
					m_hSocket = 0;
					return false;
				}
			}
			if (nRecvLen == -1)//�Ͽ�����
			{
				//m_pLog->Error("recv command %s return -1, msg return:%s",
				//	*iter, strTotalRecv);
				strLog.Format("recv command %s return -1, msg return:%s",
					*iter, strTotalRecv);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
				if (10060 != WSAGetLastError())//�������ӷ���һ��ʱ���û����ȷ�𸴻����ӵ�����û�з�Ӧ�����ӳ���ʧ�ܡ� 
					continue;
			}
#ifndef DEBUG
			if ((*iter).Find("RXESE") != -1)
				Sleep(500);//RXESE��ֺ����������ʱ����ø�0.5���ٷ���Ŀǰֻ����Ҫ�ӷֺŵĵط�������Ҫ�õ�m_vecSendMsg
#endif
		}
		m_vecSendMsg.clear();
	}
	catch (exception& e)
	{
		strLog.Format("send exception %s", e.what());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
	catch (...)
	{
		strLog.Format("telnet send exception ...");
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}

	return bRet;
}

vector<TelnetData> CSTelnet::GetTelnetData()
{
	return m_vecTelnetData;
}

CString CSTelnet::GetTelnetMsgDate( CString strMsg )
{
	CString strReturn;
	for (vector<TelnetData>::iterator iter = m_vecTelnetData.begin();iter != m_vecTelnetData.end();iter++)
	{
		if (strMsg == (*iter).strCommand)
		{
			strReturn = (*iter).strContent;
			break;
		}
	}

	return strReturn;
}

void CSTelnet::LogOut()
{
	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;

	Sleep(10);

	m_vecTelnetData.clear();
	m_vecSendMsg.clear();
	m_nLoginTime = 0;
}


//void CSTelnet::StartRecv()
//{
//	if(g_pXThreadPool)
//		g_pXThreadPool->ProcessJob(NULL,this);//���������߳�
//}

int CSTelnet::Writen(const void* pBuf, int nLen)
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
int CSTelnet::Readen(void* pBuf, int nLen)
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
			//��ʱ,ֱ�ӷ���
			break;
		}
		nLeft -= nRead;
		ptr += nRead;
	}
	return nLen - nLeft;
}