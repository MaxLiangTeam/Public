#include "StdAfx.h"
#include "ZTETelnet.h"

const int nBufferSize = 10240;

CSTelnet::CSTelnet(void)
{
	//初始化socket
	WSADATA wsaData = {0};
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData );

	m_bBuf.SetSize(1024);
	m_nPort = 23;
	m_msgError = MSG_Other;

	m_bSyncResult = true;
}

CSTelnet::~CSTelnet(void)
{
	//关闭SOCK
	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;
	WSACleanup();
}

bool CSTelnet::Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode/* = 0*/)
{
	bool bRet = false;

	m_strHost = strHost;
	m_nPort = nPort;
	m_strAccount = strAccount;
	m_strPassword = strPassword;

	m_strNormalText = "";

	try
	{
		//创建SOCK，连接服务器
		if (!ConnectServ())
		{
			g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO, "[CSTelnet::Login] 登陆失败退出程序");
			return false;
		}

		//登录失败
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
	//创建socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		if (g_pLog)
		{
			g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO,"socket()--Faild[%u]-->",GetLastError());
		}
		return false;
	}

	//构造地址和端口
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_nPort);
	addr.sin_addr.s_addr = inet_addr(m_strHost);

	//连接到服务器
	int nResult = connect(m_hSocket,(sockaddr*)&addr,sizeof(sockaddr));	
	if(nResult == SOCKET_ERROR)
	{
		OutputDebugString("connect ERROR");
		if (g_pLog)
		{
			g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO,"connect()--Faild[%u]-->",GetLastError());
		}
		return false;
	}

	
	int nNetTimeout = 30*1000;//10秒，
	//设置接收超时
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
	
	return true;
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

bool CSTelnet::Send(CString strSendMsg)
{	
	m_bSyncResult = true;
	bool bRet = true;
	if ((m_hSocket==INVALID_SOCKET) || (m_hSocket == 0))
	{
		g_pLog->WriteLogFormat(m_strLogFile,LOG_ERROR,"Send() m_hSocket is invalid");
		return false;
	}

	try
	{
		if (m_hSocket != INVALID_SOCKET)
		{
			DoSendMsg(strSendMsg);			
		}

		if (strSendMsg.Find("logout") != -1)
		{
			closesocket(m_hSocket);
			m_hSocket = 0;
			return true;
		}

		if (strSendMsg.Find("GETCPURATIO") != -1)
		{
			Sleep(1500);
		}

		CString strTotalRecv = "";
		char szBuff[nBufferSize];//10k
		memset(szBuff, '\0', sizeof(szBuff));

		long lT1 = GetTickCount();
		long lT2 = 0;
		int nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
		while(nRecvLen > 0)
		{
			CString strRecv = szBuff;
			strTotalRecv += strRecv;
			
			// 当接收到末尾为 $> 表示结束
			CString strRight = strTotalRecv.Right(2);
			if ("$>" == strRight)
			{
				break;
			}

			Sleep(10);
			memset(szBuff, '\0', sizeof(szBuff));
			nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);

			lT2 = GetTickCount();

			if ((lT2 - lT1)/1000 > 30) //超过指定时延
			{
				g_pLog->WriteLogFormat(m_strLogFile, LOG_ERROR, "recv command %s timeout(%ds), recvd msg:%s", strSendMsg, 30, strTotalRecv);
				strTotalRecv.Empty();
				closesocket(m_hSocket);
				m_hSocket = 0;
				return false;
			}

			if (nRecvLen == -1)//断开连接
			{
				g_pLog->WriteLogFormat(m_strLogFile, LOG_ERROR, "recv command %s return -1, msg return:%s",
					strSendMsg, strTotalRecv);
				if (10060 != WSAGetLastError())//由于连接方在一段时间后没有正确答复或连接的主机没有反应，连接尝试失败。 
					continue;
			}
		}
		
		g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::Send] 退出接收, 进行解码");

		if (strSendMsg.Find("GETCPURATIO") != -1)
		{
			int nPos = strTotalRecv.Find("CPU占有率");
			int nLen = strTotalRecv.GetLength();		

			g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::Send] CPU占有率nPos:%d, nLen:%d", nPos, nLen);

			m_strNormalText = strTotalRecv.Right(nLen - nPos);
		}
		else if (strSendMsg.Find("SYNC:POS") != -1)
		{//  发送同步指令, 等待接收异步返回信息
			g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::Send] 发送同步指令, 等待接收异步修改进度信息.");

			memset(szBuff, '\0', sizeof(szBuff));
			long lT1 = GetTickCount();
			long lT2 = 0;
			int nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
			while(nRecvLen > 0)
			{
				CString strRecv = szBuff;
				strTotalRecv += strRecv;

				// 当接收到末尾为 "数据同步  (总数:1 成功:1 失败:0)全部成功[100]" 表示结束
				int nPos = 0;
				nPos = strTotalRecv.Find("数据同步", nPos);
				if (nPos != -1)
				{
					nPos = strTotalRecv.Find("总数", nPos);
					if (nPos != -1)
					{
						nPos = strTotalRecv.Find("成功", nPos);
						if (nPos != -1)
						{
							nPos = strTotalRecv.Find("失败", nPos);
							if (nPos != -1)
							{
								int nLeft = 0;
								nLeft = strTotalRecv.Find(":", nPos);
								int nRight = 0;
								nRight = strTotalRecv.Find(")", nLeft);
								CString strRet = strTotalRecv.Mid(nLeft+1, nRight-nLeft-1);								
								if (strRet != "0")
								{
									m_bSyncResult = false;
								}
								break;
							}							
						}
					}
				}

				Sleep(500);
				memset(szBuff, '\0', sizeof(szBuff));
				nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);

				lT2 = GetTickCount();

				if ((lT2 - lT1)/1000 > 30) //超过指定时延
				{
					g_pLog->WriteLogFormat(m_strLogFile, LOG_ERROR, "recv command %s timeout(%ds), recvd msg:%s", strSendMsg, 30, strTotalRecv);
					break;
				}
			}

			m_strNormalText = BufferFilter(strTotalRecv.GetBuffer(0));
		}
		else if (strSendMsg.Find("RELEASE MUTEXRIGHT") != -1)
		{
			memset(szBuff, '\0', sizeof(szBuff));
			long lT1 = GetTickCount();
			long lT2 = 0;
			int nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
			while(nRecvLen > 0)
			{
				CString strRecv = szBuff;
				strTotalRecv += strRecv;

				// 当接收到末尾为 "数据同步  (总数:1 成功:1 失败:0)全部成功[100]" 表示结束
				if (strTotalRecv.Find("释放互斥权限成功") != -1)
				{
					break;
				}

				Sleep(500);
				memset(szBuff, '\0', sizeof(szBuff));
				nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);

				lT2 = GetTickCount();

				if ((lT2 - lT1)/1000 > 30) //超过指定时延
				{
					g_pLog->WriteLogFormat(m_strLogFile, LOG_ERROR, "recv command %s timeout(%ds), recvd msg:%s", strSendMsg, 30, strTotalRecv);
					break;
				}
			}

			m_strNormalText = BufferFilter(strTotalRecv.GetBuffer(0));
		}
		else
		{
			m_strNormalText = BufferFilter(strTotalRecv.GetBuffer(0));

			// 字符串开头就是命令串+$>  可能是重复, 过滤
			if (m_strNormalText.Find(strSendMsg) == 0)
			{
				m_strNormalText = m_strNormalText.Right(m_strNormalText.GetLength() - strSendMsg.GetLength());
			}
		}
		
		g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::Send] 解码完成, 打印输出内容");
		g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "%s", m_strNormalText);
	}
	catch (exception& e)
	{
		g_pLog->WriteLogFormat(m_strLogFile,LOG_ERROR,"send exception %s", e.what());
	}
	catch (...)
	{
		g_pLog->WriteLogFormat(m_strLogFile,LOG_ERROR,"telnet send exception ...");
	}

	return bRet;
}

void CSTelnet::LogOut()
{
	Send("logout;");

	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;

	Sleep(10);
}

void CSTelnet::SetLogPath( const CString &strLogPath )
{
	m_strLogFile = strLogPath;
}

bool CSTelnet::SendLoginInfo( int nMode )
{
	g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO,"[CSTelnet::SendLoginInfo]------->");

	bool bRet = false;
	bool bLoginSuccess = false;
	int nRecvTimes(0);

	while(nRecvTimes < 100)
	{
		nRecvTimes++;

		//-------------------------接收回复,把返回的信息存放到m_strNormalText中------------------
		DWORD d1 = GetTickCount();
		int nBytes = recv(m_hSocket, (char *)m_bBuf.GetData(), (int)m_bBuf.GetSize(),0);
		DWORD d2 = GetTickCount();

		if (-1 == nBytes)
		{
			if (10060 != WSAGetLastError())//由于连接方在一段时间后没有正确答复或连接的主机没有反应，连接尝试失败。 
			{
				g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO,"[CSTelnet::SendLoginInfo],Faild[10060]");
				return false;
			}
		}

		int ndx = 0;
		GetLine(m_bBuf, nBytes, ndx);
		ProcessOptions();
		m_strLine.Empty();
		m_strResp.Empty();

		if (m_strNormalText.GetLength() > 0)
		{
			//g_pLog->WriteLogFormat(m_strLogFile, LOG_SOCKET, m_strNormalText);
		}
		
		if (m_strNormalText.Find("username") != -1)
		{
			//g_pLog->WriteLogFormat(m_strLogFile, LOG_SOCKET, m_strNormalText);
			m_strNormalText.Empty();

			// 发送 telnet 名
			DoSendMsg(m_strAccount);
		}
		
		if (m_strNormalText.Find("password") != -1)
		{
			//g_pLog->WriteLogFormat(m_strLogFile, LOG_SOCKET, m_strNormalText);
			m_strNormalText.Empty();

			// 发送 telnet 密码
			DoSendMsg(m_strPassword);
		}

		if (m_strNormalText.Find("$>") != -1)
		{
			g_pLog->WriteLogFormat(m_strLogFile,LOG_INFO,"connect()--Succees-->");
			return true;
		}

		//接收时间超过9s
		if ((d2-d1)/1000>(30/*g_pConfig->nTimeOut*/))
		{
			return false;
		}
	}

	return true;
}

void CSTelnet::DoSendMsg( CString strMsg )
{
	g_pLog->WriteLogFormat(m_strLogFile,LOG_SOCKET,"send command %s",strMsg);
	//added by linzc 130220 如果为04 则不加\r\n
	if (strMsg.Find(04) == -1)
		strMsg += "\r\n";
	int nLen = strMsg.GetLength();
	send(m_hSocket,strMsg.GetBuffer(0),nLen,0);
	strMsg.ReleaseBuffer();
}

void CSTelnet::ProcessOptions()
{
	try
	{
		CString strTemp;
		CString strOption;
		unsigned char ch;
		int ndx;
		int ldx;
		BOOL bScanDone = FALSE;

		strTemp = m_strLine;

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
					}
					break;
				case IAC:
					{
						m_strNormalText	= strTemp.Left(ndx);
						strTemp	= strTemp.Mid(ndx + 1);
					}
					break;
				case SB:
					{
						m_strNormalText = strTemp.Left(ndx);
						ldx = strTemp.Find(SE);
						strOption = strTemp.Mid(ndx, ldx);
						m_strListOptions.AddTail(strOption);
						strTemp	= strTemp.Mid(ldx);
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
		CString strOption;

		while (!m_strListOptions.IsEmpty())
		{
			strOption = m_strListOptions.RemoveHead();
			ArrangeReply(strOption);
		}

		//发送
		int nSendLen = send(m_hSocket, m_strResp, (int)m_strResp.GetLength(), 0);

		m_strResp.Empty();
	}
	catch (...)
	{
	}
}

void CSTelnet::ArrangeReply( CString strOption )
{
	try
	{
		unsigned char Verb;
		unsigned char Option;
		unsigned char Modifier;
		unsigned char ch;
		BOOL bDefined = FALSE;

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

CString CSTelnet::BufferFilter( char* pRecBuffer )
{
	g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] 过滤特殊字符开始");
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
		{// 退格
			continue;
		}

		// \x1b[2K\x1b[D 跳过 3 个字符
		if (next4 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next4] == 27)
		{
			n = next3;
			continue;
		}

		// \x1b[D\x1b[D 跳过 2 个字符
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] == 27)
		{
			n = next2;
			continue;
		}

		// \x1b[D$>D 跳过 2 个字符
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] != 27)
		{
			n = next2;
			continue;
		}

		strReceive.AppendChar(pRecBuffer[n]);
	}

	strReceive.Replace("请按任意键继续(Ctrl+C 中断显示). . .", NULL);

	g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] 过滤特殊字符结束");
	return strReceive;
}

void CSTelnet::AnalyseMsg( CString strData )
{
	g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::AnalyseMsg] 解析指令开始");

	// 分割字段
	CString strBuffer = strData;	
	if (strBuffer.GetLength() > 0)
	{
		/*
			以找到以下内容为开始:
			结果
			------
		*/

		bool bShortLine = false;
		bool bResult = false;
		bool bExit = false;
		CString strLine = "";
		while (strBuffer.GetLength() > 0)
		{
			int nPos = strBuffer.FindOneOf("\r\n");
			strLine = strBuffer.Mid(0, nPos+2);
			strBuffer.Delete(0, nPos+2);

			if (strLine.Find("结果") != -1 || strLine.Find("CPU占有率") != -1)
			{
				bResult = true;
				continue;
			}

			if (strLine.Find("------") != -1)
			{
				bShortLine = true;
				continue;
			}

			if (bResult && bShortLine)
			{
				// 开始标志
				m_vTitles = mySplitString(strLine, ' ');

				while (strBuffer.GetLength() > 0)
				{
					int nPos = strBuffer.FindOneOf("\r\n");
					if (nPos == -1)
					{// 最后的回显没有 回车换行符号
						if (strBuffer.Find("$>") != -1)
						{
							bExit = true;
							break;
						}
					}
					if (nPos == 0)
					{// 表示该行为空行，换行
						strBuffer.Delete(0, nPos+2);
						continue;
					}

					strLine = strBuffer.Mid(0, nPos+2);
					strBuffer.Delete(0, nPos+2);
					if (strLine.Find("----------------------------") != -1)
					{
						continue;
					}
					if (strLine.Find("$>") != -1)
					{
						bExit = true;
						break;
					}
					if (strLine.Find("------") != -1)
					{
						continue;
					}
					vector<CString> vString = mySplitString(strLine, ' ');
					m_vValues.push_back(vString);
				}
			}

			if (bExit)
				break;
		}
	}

	if (strData.Find("CPU占有率") != -1)
	{
		m_vValues.clear();
		m_vValues.push_back(m_vTitles);
		m_vTitles.clear();
		m_vTitles.push_back("CPU占有率");
	}
	g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::AnalyseMsg] 解析指令结束");
}

CString CSTelnet::GetTelnetRetMsg()
{
	return m_strNormalText;
}

void CSTelnet::GetAnalyzeRet( vector<CString>& vTitle, vector<vector<CString>>& vValues )
{
	vTitle.clear();
	vValues.clear();
	m_vTitles.clear();
	m_vValues.clear();

	AnalyseMsg(m_strNormalText);

	vTitle = m_vTitles;
	vValues = m_vValues;
}

bool CSTelnet::GetSyncResult()
{
	return m_bSyncResult;
}
