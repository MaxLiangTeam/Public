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
	CString strCommand;//Telnet�µ�����,�硰PRINT V RNLCT 0-:170;��
	CString strContent;//Telnet���������

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
	//��¼
	bool Login(CString strHost,int nPort,CString strAccount,CString strPassword,int nMode = 0); // nMode-0 ֱ����1-��ת

	//�ǳ�,�����ͷ���Դ
	void LogOut();

	//������Ϣ,��ȴ�������Ϣ������ϲŷ���
	bool Send(CString strSendMsg);

	//����telnet�������Ҫ����Ϣ
	CString GetTelnetRetMsg();

	// failed: (����:1 �ɹ�:0 ʧ��:1) ���� false,  success: (����:1 �ɹ�:1 ʧ��:0), ���� true	 
	bool GetSyncResult();

	void SetLogPath(const CString &strLogPath);
	void SetNormalText(CString strText)
	{
		m_strNormalText = strText;
	}
	void GetAnalyzeRet(vector<CString>& vTitle, vector<vector<CString>>& vValues);

private:
	//���ӵ�������
	bool ConnectServ();
	//���͵�¼��ͷ��Ϣ
	bool SendLoginInfo(int nMode);

	//------------��¼��ʱ�򽻻���Ϣ��غ���-------------------
	bool GetLine(const CByteArray &bytes, int nBytes, int &ndx);
	void ProcessOptions();
	void RespondToOptions();
	void ArrangeReply(CString strOption);
	//------------��¼��ʱ�򽻻���Ϣ��غ���-------------------

	// ����ָ��
	void DoSendMsg(CString strMsg);

	// ���������ַ�
	CString BufferFilter(char* pRecBuffer);	
	void AnalyseMsg(CString strData);

private:
	SOCKET m_hSocket;	//�ͻ���socket	
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
	MSG_ERROR	m_msgError;		//��������

	vector<CString> m_vTitles;
	vector<vector<CString>> m_vValues;
	bool m_bSyncResult;
};
