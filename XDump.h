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
	
	//��ȡִ�г�������·������β��б��
	CString GetModulePath();

	//��ȡϵͳ��ǰ������ʱ�䣬��ʽ�磺//20090629170602
	CString GetCurrentDateTime();

private:
	CString m_strName;
};
