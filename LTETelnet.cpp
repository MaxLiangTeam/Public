#include "StdAfx.h"
#include "LTETelnet.h"
#include "Kernel.h"

const int nBufferSize = 10240;

CLTETelnet::CLTETelnet(void)
{
	//��ʼ��socket
	WSADATA wsaData = {0};
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData );

	m_bBuf.SetSize(1024);
	m_nPort = 23;
	m_msgError = MSG_Other;

	m_bSyncResult = true;
}

CLTETelnet::~CLTETelnet(void)
{
	//�ر�SOCK
	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;
	WSACleanup();
}

bool CLTETelnet::Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode/* = 0*/)
{
	bool bRet = false;

	m_strHost = strHost;
	m_nPort = nPort;
	m_strAccount = strAccount;
	m_strPassword = strPassword;

	m_strNormalText = "";

	try
	{
		//����SOCK�����ӷ�����
		if (!ConnectServ())
		{
			CString strLog;
			strLog.Format("[%s][CSTelnet::Login] ��½ʧ���˳�����\n",m_strThreadName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
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

bool CLTETelnet::ConnectServ()
{
	//����socket
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if (m_hSocket == INVALID_SOCKET)
	{
		CString strLog;
		strLog.Format("[%s]socket()--Faild[%u]-->",m_strThreadName,GetLastError());
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
		CString strLog;
		strLog.Format("[%s]connect()--Faild[%u]-->",m_strThreadName,GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	
	int nNetTimeout = 30*1000;//10�룬
	//���ý��ճ�ʱ
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
	
	return true;
}


bool CLTETelnet::GetLine(const CByteArray &bytes, int nBytes, int &ndx)
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

bool CLTETelnet::Send(CString strSendMsg)
{	
	m_bSyncResult = true;
	bool bRet = true;
	if ((m_hSocket==INVALID_SOCKET) || (m_hSocket == 0))
	{
		CString strLog;
		strLog.Format("[%s]Send() m_hSocket is invalid",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
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
			
			// �����յ�ĩβΪ $> ��ʾ����
			CString strRight = strTotalRecv.Right(2);
			if ("$>" == strRight)
			{
				break;
			}

			Sleep(10);
			memset(szBuff, '\0', sizeof(szBuff));
			nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);

			lT2 = GetTickCount();

			if ((lT2 - lT1)/1000 > 30) //����ָ��ʱ��
			{
				CString strLog;
				strLog.Format("[%s]recv command %s timeout(%ds), recvd msg:%s",m_strThreadName, strSendMsg, 30, strTotalRecv);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);

				strTotalRecv.Empty();
				closesocket(m_hSocket);
				m_hSocket = 0;
				return false;
			}

			if (nRecvLen == -1)//�Ͽ�����
			{
				CString strLog;
				strLog.Format("[%s]recv command %s return -1, msg return:%s",m_strThreadName, strSendMsg, strTotalRecv);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);

				if (10060 != WSAGetLastError())//�������ӷ���һ��ʱ���û����ȷ�𸴻����ӵ�����û�з�Ӧ�����ӳ���ʧ�ܡ� 
					continue;
			}
		}
		
		CString strLog;
		strLog.Format("[%s][CSTelnet::Send] �˳�����, ���н���",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		if (strSendMsg.Find("GETCPURATIO") != -1)
		{
			int nPos = strTotalRecv.Find("CPUռ����");
			int nLen = strTotalRecv.GetLength();		

			strLog.Format("[%s][CSTelnet::Send] CPUռ����nPos:%d, nLen:%d",m_strThreadName, nPos, nLen);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

			m_strNormalText = strTotalRecv.Right(nLen - nPos);
		}
		else if (strSendMsg.Find("SYNC:POS") != -1)
		{
			//  ����ͬ��ָ��, �ȴ������첽������Ϣ
			strLog.Format("[%s][CSTelnet::Send] ����ͬ��ָ��, �ȴ������첽�޸Ľ�����Ϣ.",m_strThreadName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

			memset(szBuff, '\0', sizeof(szBuff));
			long lT1 = GetTickCount();
			long lT2 = 0;
			int nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);
			while(nRecvLen > 0)
			{
				CString strRecv = szBuff;
				strTotalRecv += strRecv;

				// �����յ�ĩβΪ "����ͬ��  (����:1 �ɹ�:1 ʧ��:0)ȫ���ɹ�[100]" ��ʾ����
				int nPos = 0;
				nPos = strTotalRecv.Find("����ͬ��", nPos);
				if (nPos != -1)
				{
					nPos = strTotalRecv.Find("����", nPos);
					if (nPos != -1)
					{
						nPos = strTotalRecv.Find("�ɹ�", nPos);
						if (nPos != -1)
						{
							nPos = strTotalRecv.Find("ʧ��", nPos);
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

				if ((lT2 - lT1)/1000 > 30) //����ָ��ʱ��
				{
					strLog.Format("[%s]recv command %s timeout(%ds), recvd msg:%s",m_strThreadName, strSendMsg, 30, strTotalRecv);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
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

				// �����յ�ĩβΪ "����ͬ��  (����:1 �ɹ�:1 ʧ��:0)ȫ���ɹ�[100]" ��ʾ����
				if (strTotalRecv.Find("�ͷŻ���Ȩ�޳ɹ�") != -1)
				{
					break;
				}

				Sleep(500);
				memset(szBuff, '\0', sizeof(szBuff));
				nRecvLen = recv(m_hSocket, szBuff, sizeof(szBuff), 0);

				lT2 = GetTickCount();

				if ((lT2 - lT1)/1000 > 30) //����ָ��ʱ��
				{
					strLog.Format("[%s]recv command %s timeout(%ds), recvd msg:%s",m_strThreadName, strSendMsg, 30, strTotalRecv);
					g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
					break;
				}
			}

			m_strNormalText = BufferFilter(strTotalRecv.GetBuffer(0));
		}
		else
		{
			m_strNormalText = BufferFilter(strTotalRecv.GetBuffer(0));

			// �ַ�����ͷ�������+$>  �������ظ�, ����
			if (m_strNormalText.Find(strSendMsg) == 0)
			{
				m_strNormalText = m_strNormalText.Right(m_strNormalText.GetLength() - strSendMsg.GetLength());
			}
		}
		
		strLog.Format("[%s][CSTelnet::Send] �������, ��ӡ�������%s",m_strThreadName,m_strNormalText);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	}
	catch (exception& e)
	{
		CString strLog;
		strLog.Format("[%s]send exception %s",m_strThreadName, e.what());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[%s]telnet send exception ...",m_strThreadName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}

	return bRet;
}

void CLTETelnet::Logout()
{
	Send("logout;");

	if ((m_hSocket != INVALID_SOCKET)&&(m_hSocket))
		closesocket(m_hSocket);
	m_hSocket = 0;

	Sleep(10);
}

void CLTETelnet::SetLogPath( const CString &strLogPath )
{
	m_strLogFile = strLogPath;
}

bool CLTETelnet::SendLoginInfo( int nMode )
{
	CString strLog;
	strLog.Format("[%s][CSTelnet::SendLoginInfo]------->",m_strThreadName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	bool bRet = false;
	bool bLoginSuccess = false;
	int nRecvTimes(0);

	while(nRecvTimes < 100)
	{
		nRecvTimes++;

		//-------------------------���ջظ�,�ѷ��ص���Ϣ��ŵ�m_strNormalText��------------------
		DWORD d1 = GetTickCount();
		int nBytes = recv(m_hSocket, (char *)m_bBuf.GetData(), (int)m_bBuf.GetSize(),0);
		DWORD d2 = GetTickCount();

		if (-1 == nBytes)
		{
			if (10060 != WSAGetLastError())//�������ӷ���һ��ʱ���û����ȷ�𸴻����ӵ�����û�з�Ӧ�����ӳ���ʧ�ܡ� 
			{
				strLog.Format("[%s][CSTelnet::SendLoginInfo],Faild[10060]",m_strThreadName);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
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

			// ���� telnet ��
			DoSendMsg(m_strAccount);
		}
		
		if (m_strNormalText.Find("password") != -1)
		{
			//g_pLog->WriteLogFormat(m_strLogFile, LOG_SOCKET, m_strNormalText);
			m_strNormalText.Empty();

			// ���� telnet ����
			DoSendMsg(m_strPassword);
		}

		if (m_strNormalText.Find("$>") != -1)
		{
			strLog.Format("[%s]connect()--Succees-->",m_strThreadName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			return true;
		}

		//����ʱ�䳬��9s
		if ((d2-d1)/1000>(30/*g_pConfig->nTimeOut*/))
		{
			return false;
		}
	}

	return true;
}

void CLTETelnet::DoSendMsg( CString strMsg )
{
	CString strLog;
	strLog.Format("[%s]send command %s",m_strThreadName,strMsg);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	//added by linzc 130220 ���Ϊ04 �򲻼�\r\n
	if (strMsg.Find(04) == -1)
		strMsg += "\r\n";
	int nLen = strMsg.GetLength();
	send(m_hSocket,strMsg.GetBuffer(0),nLen,0);
	strMsg.ReleaseBuffer();
}

void CLTETelnet::ProcessOptions()
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

void CLTETelnet::RespondToOptions()
{
	try
	{
		CString strOption;

		while (!m_strListOptions.IsEmpty())
		{
			strOption = m_strListOptions.RemoveHead();
			ArrangeReply(strOption);
		}

		//����
		int nSendLen = send(m_hSocket, m_strResp, (int)m_strResp.GetLength(), 0);

		m_strResp.Empty();
	}
	catch (...)
	{
	}
}

void CLTETelnet::ArrangeReply( CString strOption )
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

CString CLTETelnet::BufferFilter( char* pRecBuffer )
{
	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] ���������ַ���ʼ");

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
		{// �˸�
			continue;
		}

		// \x1b[2K\x1b[D ���� 3 ���ַ�
		if (next4 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next4] == 27)
		{
			n = next3;
			continue;
		}

		// \x1b[D\x1b[D ���� 2 ���ַ�
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] == 27)
		{
			n = next2;
			continue;
		}

		// \x1b[D$>D ���� 2 ���ַ�
		if (next3 < nLen && pRecBuffer[n] == 27 && pRecBuffer[next1] == 91 && pRecBuffer[next3] != 27)
		{
			n = next2;
			continue;
		}

		strReceive.AppendChar(pRecBuffer[n]);
	}

	strReceive.Replace("�밴���������(Ctrl+C �ж���ʾ). . .", NULL);

	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::BufferFilter] ���������ַ�����");
	return strReceive;
}

void CLTETelnet::AnalyseMsg( CString strData )
{
	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::AnalyseMsg] ����ָ�ʼ");
	CString strLog;
	strLog.Format("[%s][CSTelnet::AnalyseMsg] ����ָ�ʼ",m_strThreadName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	// �ָ��ֶ�
	CString strBuffer = strData;	
	if (strBuffer.GetLength() > 0)
	{
		/*
			���ҵ���������Ϊ��ʼ:
			���
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

			if (strLine.Find("���") != -1 || strLine.Find("CPUռ����") != -1)
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
				// ��ʼ��־
				//m_vTitles = mySplitString(strLine, ' ');

				while (strBuffer.GetLength() > 0)
				{
					int nPos = strBuffer.FindOneOf("\r\n");
					if (nPos == -1)
					{// ���Ļ���û�� �س����з���
						if (strBuffer.Find("$>") != -1)
						{
							bExit = true;
							break;
						}
					}
					if (nPos == 0)
					{// ��ʾ����Ϊ���У�����
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
					//vector<CString> vString = mySplitString(strLine, ' ');
					vector<CString> vString;
					m_vValues.push_back(vString);
				}
			}

			if (bExit)
				break;
		}
	}

	if (strData.Find("CPUռ����") != -1)
	{
		m_vValues.clear();
		m_vValues.push_back(m_vTitles);
		m_vTitles.clear();
		m_vTitles.push_back("CPUռ����");
	}
	//g_pLog->WriteLogFormat(m_strLogFile, LOG_INFO, "[CSTelnet::AnalyseMsg] ����ָ�����");
	strLog.Format("[%s][CSTelnet::AnalyseMsg] ����ָ�����",m_strThreadName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
}

CString CLTETelnet::GetTelnetRetMsg()
{
	return m_strNormalText;
}

void CLTETelnet::GetAnalyzeRet( vector<CString>& vTitle, vector<vector<CString>>& vValues )
{
	vTitle.clear();
	vValues.clear();
	m_vTitles.clear();
	m_vValues.clear();

	AnalyseMsg(m_strNormalText);

	vTitle = m_vTitles;
	vValues = m_vValues;
}

bool CLTETelnet::GetSyncResult()
{
	return m_bSyncResult;
}
