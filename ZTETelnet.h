#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")


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


const	unsigned char IAC		= 255;
const	unsigned char DO		= 253;
const	unsigned char DONT		= 254;
const	unsigned char WILL		= 251;
const	unsigned char WONT		= 252;
const	unsigned char SB		= 250;
const	unsigned char SE		= 240;
const	unsigned char IS		= '0';
const	unsigned char SEND		= '1';
const	unsigned char INFO		= '2';
const	unsigned char VAR		= '0';
const	unsigned char VALUE		= '1';
const	unsigned char ESC		= '2';
const	unsigned char USERVAR	= '3';


class CSTelnet
{
public:
	CSTelnet(void);

	virtual ~CSTelnet(void);
public:
	//登录
	bool Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode = 0); // nMode-0 直连，1-跳转

	//登出,并且释放资源
	void LogOut();

	//发送消息,会等待所有消息发送完毕才返回
	bool Send(CString strSendMsg);

	//根据telnet命令返回需要的信息
	CString GetTelnetRetMsg();

	// failed: (总数:1 成功:0 失败:1) 返回 false,  success: (总数:1 成功:1 失败:0), 返回 true	 
	bool GetSyncResult();

	void SetLogPath(const CString &strLogPath);
	void SetNormalText(CString strText)
	{
		m_strNormalText = strText;
	}
	void GetAnalyzeRet(vector<CString>& vTitle, vector<vector<CString>>& vValues);

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

	// 发送指令
	void DoSendMsg(CString strMsg);

	// 过滤特殊字符
	CString BufferFilter(char* pRecBuffer);	
	void AnalyseMsg(CString strData);

private:
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
	MSG_ERROR	m_msgError;		//错误类型

	vector<CString> m_vTitles;
	vector<vector<CString>> m_vValues;
	bool m_bSyncResult;
};
