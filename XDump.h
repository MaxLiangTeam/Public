#pragma once

class CXDump
{
	friend long __stdcall _ProcessException(_EXCEPTION_POINTERS *pExceptionInfo);
public:
	CXDump(void);

	virtual ~CXDump(void);
public:

	void Start(CString strName = "");

private:
	CString GetFileVersion(char*  FileName);

	CString GetFileVersion();
	
	//获取执行程序所在路径、结尾带斜杠
	CString GetModulePath();

	//获取系统当前日期与时间，格式如：//20090629170602
	CString GetCurrentDateTime();

private:
	CString m_strName;
};
