#pragma once

class CTinyEncode
{
public:
	CTinyEncode(void);
	virtual ~CTinyEncode(void);

public:	
	bool Decode(CString strSor,CString &strDest);//����	
	bool Encode(CString strSor,CString &strDest);//����

private:
	unsigned char m_szKey[256];
};


class CMDIdentify : public CTinyEncode
{
public:
	CMDIdentify(void);
	virtual ~CMDIdentify(void);

private:
	CString GetModulePath();//��ȡִ�г�������·��
	CString LoadCapMaxData(CString strPath = "");
	CString ReadReg();

public:
	//��ȡ�ļ���SQL�ű���Ϣ,����SQL���,0-�ɹ�,1-�ļ�������,2-����ʧ��,3-������֤ʧ��
	int GetIdentifyData(CString strPath,CString strSQL);
};