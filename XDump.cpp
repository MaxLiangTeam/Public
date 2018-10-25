#include "StdAfx.h"
#include "XDump.h"
#include <imagehlp.h>

#pragma comment(lib,"Version.lib ")
#pragma comment(lib,"dbghelp.lib ")

CXDump *g_pCXdump = NULL;

long __stdcall _ProcessException(_EXCEPTION_POINTERS *pExceptionInfo)
{
	OutputDebugString("��ʼִ��  _ProcessException");

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
	
	//�����ļ�
	HANDLE hFile = CreateFile(strPath,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(hFile != INVALID_HANDLE_VALUE) 
	{
		//�����쳣��Ϣ�ṹ��
		_MINIDUMP_EXCEPTION_INFORMATION   ExInfo;

		ExInfo.ThreadId = GetCurrentThreadId();
		ExInfo.ExceptionPointers = pExceptionInfo;
		ExInfo.ClientPointers = NULL;    

		//����MiniDumpWriteDumpд��Ϣ
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
	strMSG.Format("[%s] Ӧ�ó���������Ҫ��ֹ����������ȷ������\n\n�ڴ� [0x%08X],��־ [0x%08X],������ [0x%08X] ��\n\n�뽫�����ļ����͸������Ա��\n%s",
		g_pCXdump->m_strName,
		pExceptionInfo->ExceptionRecord->ExceptionAddress,
		pExceptionInfo->ExceptionRecord->ExceptionFlags,
		pExceptionInfo->ExceptionRecord->ExceptionCode,strPath);

	::MessageBox(NULL,strMSG,"Ӧ�ó������",MB_ICONERROR|MB_OKCANCEL);

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

	::SetUnhandledExceptionFilter(_ProcessException);//�����쳣������
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

//��ȡִ�г�������·������β��б��
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

//��ȡϵͳ��ǰ������ʱ�䣬��ʽ�磺//20090629170602
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