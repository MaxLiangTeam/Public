#include "stdafx.h"

#include "telnet.h"

#include <stdio.h>
#include <string.h>


//判断指定会话是否有效
static int telnet_SessionIsOpen(const telnet_Session pTelnetSession)
{
	if(pTelnetSession == NULL)
		return 0;

	if(pTelnetSession->m_szLineMark == NULL)
		return 0;

	if(pTelnetSession->m_TelnetSocket == NULL)
		return 0;

	return 1;
}
//一直读取数据直到出现指定字符串szMark
static int telnet_RecvUntilMark(SOCKET TelnetSocket, const char* szMark, char* pcRecvBuf, const int iRecvBufLen)
{
	const char* pMatch = szMark;
	char cReceive = '\0';
	int iReceived = 0;

	if(TelnetSocket == INVALID_SOCKET)
		return -1;

	while(1){
		if(recv(TelnetSocket, &cReceive, 1, 0) != 1)
			return -1;

		//pcRecvBuf为空或iRecvBufLen为0时不记录读取内容
		if(pcRecvBuf != NULL && iReceived < iRecvBufLen)
			pcRecvBuf[iReceived++] = cReceive;

		if(cReceive == *pMatch)
			pMatch++;
		else
			pMatch = szMark;

		if(*pMatch == '\0')
			break;
	}

	//返回读取到的字符个数
	return iReceived;
}

int telnet_WSAStartup()
{
	WSADATA  Ws;

	if(WSAStartup(MAKEWORD(2,2), &Ws) != 0){
		return -1;
	}

	return 0;
}

int telnet_WSACleanup()
{
	WSACleanup();

	return 0;
}

int telnet_CloseSession(telnet_Session pTelnetSession)
{
	if(pTelnetSession == NULL)
		return -1;

	if(pTelnetSession->m_TelnetSocket != INVALID_SOCKET)
		closesocket(pTelnetSession->m_TelnetSocket);
	
	free(pTelnetSession);
	
	pTelnetSession = NULL;

	return 0;
}

telnet_Session telnet_OpenSessionWithLogin(const char* szLoginMark, const char* szLineMark, const char* szIp, const int iPort, const char* szUserName, const char* szPassWord)
{
	struct sockaddr_in ServerAddr;
	unsigned char strRecvBuf[3] = {0};
	telnet_Session pTelnetSession = NULL;
	
	//分配telnet_Session_t空间
	pTelnetSession = (telnet_Session_t*)malloc(sizeof(telnet_Session_t));
	if(pTelnetSession == NULL)
		return NULL;

	memset(pTelnetSession, 0, sizeof(telnet_Session_t));

	//若szLineMark无效，直接返回
	if(szLineMark == NULL || strlen(szLineMark) >= TELNET_MARK_LEN){
		free(pTelnetSession);
		return NULL;
	}

	strcpy(pTelnetSession->m_szLineMark, szLineMark);

	//创建socket，连接至telnet服务器
	pTelnetSession->m_TelnetSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(pTelnetSession->m_TelnetSocket == INVALID_SOCKET){
		free(pTelnetSession);
		return NULL;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(szIp);
	ServerAddr.sin_port = htons(iPort);
	memset(ServerAddr.sin_zero, 0x00, 8);

	if(connect(pTelnetSession->m_TelnetSocket,(struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR){
		telnet_CloseSession(pTelnetSession);
		return NULL;
	}

	strcpy(pTelnetSession->m_szIp, szIp);
	pTelnetSession->m_iPort = iPort;

	//与服务器进行协商
	while(1){
		if(recv(pTelnetSession->m_TelnetSocket, (char*)strRecvBuf, 1, 0) != 1){
			telnet_CloseSession(pTelnetSession);
			return NULL;
		}

		if(strRecvBuf[0] != 255)
			break;

		if(recv(pTelnetSession->m_TelnetSocket, (char*)(strRecvBuf+1), 2, 0) != 2){
			telnet_CloseSession(pTelnetSession);
			return NULL;
		}
		
		/*所有服务器要求客户端开启或关闭的选项均关闭
		  所有服务器希望自己开启的选项均开启
		  所有服务器希望自己关闭的选项均关闭*/
		switch(strRecvBuf[1]){
			case 251:	//WILL
				strRecvBuf[1] = 253;	//DO
				break;

			case 252:	//WONT
				strRecvBuf[1] = 254;	//DONT
				break;

			case 253:	//DO
			case 254:	//DONT
				strRecvBuf[1] = 252;	//WONT
				break;

			default:
				telnet_CloseSession(pTelnetSession);
				return NULL;
		}

		if(send(pTelnetSession->m_TelnetSocket, (char*)strRecvBuf, 3, 0) != 3){
			telnet_CloseSession(pTelnetSession);
			return NULL;
		}
	}

	//存在一个隐患，在协商最后，会多读取1个字符。通常读取的是欢迎信息，不影响之后RecvUntilMark判断

	//若szLoginMark为空，则不进行登录操作
	if(szLoginMark != NULL){
		if(telnet_RecvUntilMark(pTelnetSession->m_TelnetSocket, szLoginMark, NULL, 0) != 0){
			telnet_CloseSession(pTelnetSession);
			return NULL;
		}
		
		if(szUserName != NULL){
			if(send(pTelnetSession->m_TelnetSocket, szUserName, strlen(szUserName), 0) != strlen(szUserName)){
				telnet_CloseSession(pTelnetSession);
				return NULL;
			}

			if(send(pTelnetSession->m_TelnetSocket, "\r\n", 2, 0) != 2){
				telnet_CloseSession(pTelnetSession);
				return NULL;
			}
		}
		else{
			telnet_CloseSession(pTelnetSession);
			return NULL;
		}
		
		if(szPassWord != NULL){
			if(send(pTelnetSession->m_TelnetSocket, szPassWord, strlen(szPassWord), 0) != strlen(szPassWord)){
				telnet_CloseSession(pTelnetSession);
				return NULL;
			}

			if(send(pTelnetSession->m_TelnetSocket, "\r\n", 2, 0) != 2){
				telnet_CloseSession(pTelnetSession);
				return NULL;
			}
		}	
	}
	
	//读取服务器响应，直到出现命令行提示符
	if(telnet_RecvUntilMark(pTelnetSession->m_TelnetSocket, pTelnetSession->m_szLineMark, NULL, 0) != 0){
		telnet_CloseSession(pTelnetSession);
		return NULL;
	}

	return pTelnetSession;
}

int telnet_SendCmdBasic(const telnet_Session pTelnetSession, const char* pcCmd, const int iCmdLen, char* pcRecvBuf, const int iRecvBufLen)
{
	int iReceived = 0;

	if(!telnet_SessionIsOpen(pTelnetSession))
		return -1;

	//发送命令
	if(send(pTelnetSession->m_TelnetSocket, pcCmd, iCmdLen, 0) != iCmdLen)
		return -1;

	
	//读取服务器响应，遇到命令行提示符认为命令执行完成
	iReceived = telnet_RecvUntilMark(pTelnetSession->m_TelnetSocket, pTelnetSession->m_szLineMark, pcRecvBuf, iRecvBufLen);
	if(iReceived < 0)
		return -1;

	return iReceived;
	
}

int telnet_ComfirmLastCmdResult(const telnet_Session pTelnetSession)
{
	const char* szComfirmCmd = "echo $?\r\n";
	char strRecvBuf[256] = {0};
	int iResult = -1;
	
	//发送确认命令echo $?
	if(telnet_SendCmdBasic(pTelnetSession, szComfirmCmd, strlen(szComfirmCmd), strRecvBuf, 256) < 0)
		return -1;
	
	//读取返回值
	if(sscanf(strRecvBuf,"echo $?\r\n%d\r\n", &iResult) != 1)
		return -1;

	if(iResult != 0)
		return -1;

	return 0;
}

int telnet_SendCmdWithComfirm(const telnet_Session pTelnetSession, const char* pcCmd, const int iCmdLen)
{
	//发送命令
	if(telnet_SendCmd(pTelnetSession, pcCmd, iCmdLen) != 0)
		return -1;
	
	//确认结果
	if(telnet_ComfirmLastCmdResult(pTelnetSession) != 0)
		return -1;

	return 0;
}

int telnet_SendCmdFromFile(const telnet_Session pTelnetSession, const char* szFilePath)
{
	FILE* pfile;
	char strReadBuf[TELNET_CMD_LEN_IN_FILE+2] = {0};
	int iStrLen = 0;

	pfile = fopen(szFilePath, "r");
	if(pfile == NULL)
		return -1;

	//按行读取命令
	while(fgets(strReadBuf, TELNET_CMD_LEN_IN_FILE, pfile)){	//单行命令不能超过CMD_LEN_IN_FILE个字节
		//修改换行符格式
		iStrLen = strlen(strReadBuf);
		if(strReadBuf[iStrLen-1] == '\n'){	//将\n修改为\r\n
			strReadBuf[iStrLen-1] = '\r';
			strReadBuf[iStrLen] = '\n';
		}
		else{	//对于文件最后一行命令，添加\r\n
			strReadBuf[iStrLen] = '\r';
			strReadBuf[iStrLen+1] = '\n';
		}
		
		//发送命令并确认结果
		if(telnet_SendCmdWithComfirm(pTelnetSession, strReadBuf, iStrLen+1) != 0){
			if(pfile)
				fclose(pfile);
			return -1;
		}
	}
	
	if(pfile)
		fclose(pfile);

	return 0;
}