#pragma once

//v 7.0.1.10152

typedef int (__stdcall *pNEMonitor)(LPCTSTR, const unsigned, LPCTSTR,const unsigned, const unsigned,LPCTSTR);
//typedef int (__stdcall *pNEMonitor)(LPCTSTR, const unsigned, LPCTSTR,const unsigned, const unsigned,const unsigned);
typedef int (__stdcall *pSYSAuthentication)(LPCTSTR, const unsigned, LPCTSTR);
typedef int (__stdcall *pNEAuthentication)(LPCTSTR, const unsigned, LPCTSTR, LPCTSTR);
typedef int (__stdcall *pSearchDB2String)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, int, char**);

class CDLLMgr
{
public:
	CDLLMgr(void);
public:
	virtual ~CDLLMgr(void);
	virtual bool Load(CString strDll);
	virtual void Free();
protected:
	HINSTANCE m_hDLL;
};

class CIdentify : public CDLLMgr
{
public:
	CIdentify(void);
public:
	virtual ~CIdentify(void);
	virtual bool Load(CString strDll);
private:
	CString m_strErr;
public:
	CString GetLastErr() {return m_strErr;}
public:
	static pSYSAuthentication SYSAuthentication;
	static pNEMonitor NEMonitor;
	static pNEAuthentication NEAuthentication;
	static pSearchDB2String SearchDB2String;
};