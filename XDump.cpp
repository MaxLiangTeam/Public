#include "StdAfx.h"
#include "XDump.h"
#include <imagehlp.h>

#pragma comment(lib,"Version.lib ")
#pragma comment(lib,"dbghelp.lib ")

CXDump *g_pCXdump = NULL;

long __stdcall _ProcessException(_EXCEPTION_POINTERS *pExceptionInfo)
{
	OutputDebugString("开始执行  _ProcessException");

	long lRet = EXCEPTION_CONTINUE_SEARCH;

	CString strPath;

	if (g_pCXdump)
	{
		strPath = g_pCXdump->GetModulePath() + g_pCXdump->m_strName + "_" + g_pCXdump->GetCurrentDateTime() + "_" + g_pCXdump->GetFileVersion()+ ".dmp";
	}
	else
	{
		strPath = "Service_Loader_Dump_File.dmp";
	}
	
	//创建文件
	HANDLE hFile = CreateFile(strPath,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(hFile != INVALID_HANDLE_VALUE) 
	{
		//创建异常信息结构体
		_MINIDUMP_EXCEPTION_INFORMATION   ExInfo;

		ExInfo.ThreadId = GetCurrentThreadId();
		ExInfo.ExceptionPointers = pExceptionInfo;
		ExInfo.ClientPointers = NULL;    

		//调用MiniDumpWriteDump写信息
		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithFullMemory,
			&ExInfo,
			NULL,
			NULL);

		lRet = EXCEPTION_EXECUTE_HANDLER;
	}
	CloseHandle(hFile);

	CString strMSG;
	strMSG.Format("[%s] 应用程序发生错误，要终止程序，请点击“确定”。\n\n内存 [0x%08X],标志 [0x%08X],错误码 [0x%08X] 。\n\n请将以下文件发送给相关人员：\n%s",
		g_pCXdump->m_strName,
		pExceptionInfo->ExceptionRecord->ExceptionAddress,
		pExceptionInfo->ExceptionRecord->ExceptionFlags,
		pExceptionInfo->ExceptionRecord->ExceptionCode,strPath);

	::MessageBox(NULL,strMSG,"应用程序错误",MB_ICONERROR|MB_OKCANCEL);

	return lRet;
}


CXDump::CXDump(void)
{
	m_strName = "";
}

CXDump::~CXDump(void)
{
}

void CXDump::Start(CString strName/* = ""*/)
{
	m_strName = strName;
	if (m_strName.IsEmpty())
	{
		m_strName = "Dump";
	}

	g_pCXdump = this;

	::SetUnhandledExceptionFilter(_ProcessException);//设置异常处理函数
}


CString CXDump::GetFileVersion(char*  FileName)  
{    
	CString asVer = "";  

	try
	{
		int iVerInfoSize;  
		char *pBuf;  
		VS_FIXEDFILEINFO *pVsInfo;  
		unsigned int iFileInfoSize = sizeof( VS_FIXEDFILEINFO );  

		iVerInfoSize = GetFileVersionInfoSize(FileName,NULL);   

		if(iVerInfoSize != 0)  
		{     
			pBuf = new char[iVerInfoSize];  
			if(GetFileVersionInfo(FileName,0,iVerInfoSize,pBuf))     
			{     
				if(VerQueryValue(pBuf,"\\",(void**)&pVsInfo,&iFileInfoSize))     
				{     
					asVer.Format("%d_%d_%d_%d",HIWORD(pVsInfo->dwFileVersionMS),LOWORD(pVsInfo->dwFileVersionMS),HIWORD(pVsInfo->dwFileVersionLS),LOWORD(pVsInfo->dwFileVersionLS));  
				}     
			}     
			delete []pBuf;     
		}
	}
	catch(...)
	{

	}	

	return asVer;     
}

CString CXDump::GetFileVersion()
{
	char cFile[MAX_PATH];  
	GetModuleFileName(NULL, cFile, sizeof(cFile));

	return GetFileVersion(cFile);
}

//获取执行程序所在路径、结尾带斜杠
CString CXDump::GetModulePath()
{
	CString strPath;

	try
	{
		char cFile[MAX_PATH];  
		GetModuleFileName(NULL, cFile, sizeof(cFile)); 

		CString strTemp = cFile;
		int nPos = strTemp.ReverseFind('\\');
		strPath = strTemp.Left(nPos+1);
	}
	catch(...)
	{

	}

	return strPath;
}

//获取系统当前日期与时间，格式如：//20090629170602
CString CXDump::GetCurrentDateTime()
{
	CString strTime;

	try
	{
		COleDateTime currentTime = COleDateTime::GetCurrentTime();
		strTime = currentTime.Format("%Y%m%d%H%M%S");//20090629170602
	}
	catch(...)
	{
	}

	return strTime;
}