#pragma once

#include <afxmt.h>
#include <WinSock2.h>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>

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
const	unsigned char MAXTEST	= '25';

enum enCheckEnd
{
	CHECK_LST	= 0,// LST查询指令
	CHECK_EXEC	= 1,// 调整指令
	CHECK_DSP	= 2,// DSP指令
	CHECK_LOGIN	= 3,//登录
    CHECK_EXPORT = 4, //生成信息文件指令
	CHECK_MMLREG = 5, //MML北向连接登陆注册网元
	CHECK_MMLUNREG = 6, //MML北向连接注销网元
	CHECK_LOGOUT = 7, //登出
	CHECK_STAT = 8 //通过telnet23端口接入， 校验结束符
};

class CTelnet
{
public:
	virtual ~CTelnet(void);
public:
	CTelnet(void);
	bool IsLogin() {return m_hSocket != INVALID_SOCKET ? true : false;}
	bool Login(CString strIP,int nPort,CString strUser,CString strPassword,int nLoginType=0);
	bool MMLLogin(CString strIP,int nPort, CString strName,CString strUser,CString strPassword,int nLoginType = 0);
	bool Logout(void);
	bool MMLLogout(CString strName,CString strUser);
	bool Login(CString& strIP,const CString& strUser,const CString& strPassword);
	bool SendLoginInfo(const CString& strUser,const CString& strPassword);
	bool ConnectServer(CString strIP, int nPort);//连接到服务器
	int Writen(const void* pBuf, int nLen);
	int Readen(void* pBuf, int nLen);

	//单个发送指令
	int Send(CString strCmd,CString& strData,enCheckEnd enCheck = CHECK_LST,bool bIsSend = true);
	int Send(CString strCmd,CString& strData,bool bIsSend = true);
	int SendCMD(CString strCmd,CString& strData,bool bIsSend = true);
	bool SendMsg(const CString& strMsg);

	//------------登录的时候交互信息相关函数-------------------
	bool GetLine(const CByteArray &bytes, int nBytes, int &ndx);
	CString ProcessOptions(CString& strmsg);
	CString ArrangeReply(CString& strOption);
	void ERICProcessOptions();
	void ERICRespondToOptions();
	void ERICArrangeReply(CString strOption);
	//------------登录的时候交互信息相关函数-------------------
	// 过滤特殊字符
	CString BufferFilter(char* pRecBuffer);
	CString BufferReplace(CString & strData);

	//------------------ERIC登录方式------------------------
	//登录
	bool ERICLogin(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode);
	//连接到服务器
	bool ConnectERICServ(CString strHost,int nPort);
	//发送登录和头信息
	bool SendERICLoginInfo(CString strAccount,CString strPassword,int nMode);

	//检查是否是接收完整数据包标志
	int CheckEnd0(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd1(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd2(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd3(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd4(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd5(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd6(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd7(CString& strCmd,CString& strPack,CString& strData);
	int CheckEnd8(CString& strCmd,CString& strPack,CString& strData);
	//检查是否是登录信息消息
	bool CheckLoginMsg(CString& strCmd);
	//设置接收超时
	void SetTimeout(int nTimeout) {m_nTimeout = nTimeout;}


public:
	SOCKET m_hSocket;
	CString m_strLogFile;
	CString m_strThreadName;
private:
	CString		m_strResp;
	CString		m_strLine;
	CString		m_strNormalText;
	CStringList m_strListOptions;
	CByteArray	m_bBuf;
	static bool m_bInit;
	bool   m_bExit;
	int    m_nTimeout;
	int		m_nLoginTime;	//登录次数
};
