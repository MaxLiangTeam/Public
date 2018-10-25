#include "StdAfx.h"
#include "Identify.h"

CDLLMgr::CDLLMgr(void)
:m_hDLL(NULL)
{
}

CDLLMgr::~CDLLMgr(void)
{
	this->Free();
}

bool CDLLMgr::Load(CString strDll)
{
	m_hDLL = LoadLibrary(strDll);
	return m_hDLL != NULL ? true : false;
}

void CDLLMgr::Free()
{
	if (m_hDLL)
	{
		FreeLibrary(m_hDLL);
	}
}

pSYSAuthentication CIdentify::SYSAuthentication = NULL;
pNEMonitor CIdentify::NEMonitor = NULL;
pNEAuthentication CIdentify::NEAuthentication = NULL;
pSearchDB2String CIdentify::SearchDB2String = NULL;

CIdentify::CIdentify(void)
{
}

CIdentify::~CIdentify(void)
{
}

bool CIdentify::Load(CString strDll)
{
	//º”‘ÿDLL
	m_hDLL = LoadLibrary(strDll);
	if (m_hDLL == NULL)
	{
		m_strErr.Format("Load [%s] failure!",strDll);
		return false;
	}

	CIdentify::SYSAuthentication = (pSYSAuthentication)GetProcAddress(m_hDLL,"SYSAuthentication");
	if (CIdentify::SYSAuthentication == NULL)
	{
		m_strErr.Format("Get [SYSAuthentication] from [%s] failure!",strDll);
		return false;
	}

	CIdentify::NEAuthentication = (pNEAuthentication)GetProcAddress(m_hDLL,"NEAuthentication");
	if (CIdentify::NEAuthentication == NULL)
	{
		m_strErr.Format("Get [NEAuthentication] from [%s] failure!",strDll);
		return false;
	}

	CIdentify::NEMonitor = (pNEMonitor)GetProcAddress(m_hDLL,"NEMonitor");
	if (CIdentify::NEMonitor == NULL)
	{
		m_strErr.Format("Get [NEMonitor] from [%s] failure!",strDll);
		return false;
	}

	CIdentify::SearchDB2String = (pSearchDB2String)GetProcAddress(m_hDLL,"SearchDB2String");
	if (CIdentify::SearchDB2String == NULL)
	{
		m_strErr.Format("Get [SearchDB2String] from [%s] failure!",strDll);
		return false;
	}

	return true;
}