#pragma once

class CTinyEncode
{
public:
	CTinyEncode(void);
	virtual ~CTinyEncode(void);

public:	
	bool Decode(CString strSor,CString &strDest);//解密	
	bool Encode(CString strSor,CString &strDest);//加密

private:
	unsigned char m_szKey[256];
};


class CMDIdentify : public CTinyEncode
{
public:
	CMDIdentify(void);
	virtual ~CMDIdentify(void);

private:
	CString GetModulePath();//获取执行程序所在路径
	CString LoadCapMaxData(CString strPath = "");
	CString ReadReg();

public:
	//获取文件内SQL脚本信息,返回SQL语句,0-成功,1-文件不存在,2-解码失败,3-机器验证失败
	int GetIdentifyData(CString strPath,CString strSQL);
};