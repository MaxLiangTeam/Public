#pragma once

#include <afxmt.h>
#include <WinSock2.h>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum MSG_ERROR
{
	MSG_SUCCESS,
	MSG_LOGIN_FAILD,
	MSG_BSC_FAILD,
	MSG_IOEXP_FAILD,
	MSG_Other
};

typedef struct _tagTelnetData
{
	CString strCommand;//Telnet下的命令,如“PRINT V RNLCT 0-:170;”
	CString strContent;//Telnet命令返回内容

	_tagTelnetData ()
	{
		//
	}
}TelnetData, *LPTelnetData;

//const	unsigned char IAC		= 255;
//const	unsigned char DO		= 253;
//const	unsigned char DONT		= 254;
//const	unsigned char WILL		= 251;
//const	unsigned char WONT		= 252;
//const	unsigned char SB		= 250;
//const	unsigned char SE		= 240;
//const	unsigned char IS		= '0';
//const	unsigned char SEND		= '1';
//const	unsigned char INFO		= '2';
//const	unsigned char VAR		= '0';
//const	unsigned char VALUE		= '1';
//const	unsigned char ESC		= '2';
//const	unsigned char USERVAR	= '3';

class CSTelnet
{
public:
	CSTelnet(void);

	virtual ~CSTelnet(void);
public:
	//登录
	bool Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode);

	//登出,并且释放资源
	void LogOut();

	//将消息放到队列中
	void PushMsg(CString strMsg);

	//发送消息,会等待所有消息发送完毕才返回
	bool Send();

	//亮灯
	void SetSendEvent();

	//经过原始socket发送信息
	void DoSendMsg(CString strMsg);

	vector<TelnetData> GetTelnetData();

	//根据telnet命令返回需要的信息
	CString GetTelnetMsgDate(CString strMsg);

	//void SetLog(Log* pLog);
	//void StartRecv();

	bool ConnectServer(const char* pszIp, int nPort);
	int Writen(const void* pBuf, int nLen);
	int Readen(void* pBuf, int nLen);
	
private:
	//连接到服务器
	bool ConnectServ();

	//发送登录和头信息
	bool SendLoginInfo(int nMode);

	//------------登录的时候交互信息相关函数-------------------
	bool GetLine(const CByteArray &bytes, int nBytes, int &ndx);
	void ProcessOptions();
	void RespondToOptions();
	void ArrangeReply(CString strOption);
	//------------登录的时候交互信息相关函数-------------------

	void Analyse(CString strCmd, CString strInfo);
	
	public:
	//Log*		m_pLog;

	SOCKET m_hSocket;	//客户端socket	
	CString m_strLogFile;

private:
	CString m_strHost;
	int		m_nPort;
	CString m_strAccount;
	CString m_strPassword;

	CByteArray	m_bBuf;
	CString		m_strNormalText;
	CString		m_strLine;
	CStringList m_strListOptions;
	CString		m_strResp;

	vector<CString>		m_vecSendMsg;		//发送消息容器
	vector<TelnetData>	m_vecTelnetData;	//Telnet执行结果

	int			m_nLoginTime;	//登录次数
	CEvent		m_hEvent;		//红绿灯
	MSG_ERROR	m_msgError;		//错误类型
};
