#include "StdAfx.h"
#include "TinyEncode.h"
#include <atlbase.h>

CTinyEncode::CTinyEncode(void)
{
	unsigned char szValue[256] =
	{
		1, 3, 5, 15, 17, 51, 85, 255, 26, 46, 114, 150, 161, 248, 19, 53, 
		95, 225, 56, 72, 216, 115, 149, 164, 247, 2, 6, 10, 30, 34, 102, 170, 
		229, 52, 92, 228, 55, 89, 235, 38, 106, 190, 217, 112, 144, 171, 230, 49, 
		83, 245, 4, 12, 20, 60, 68, 204, 79, 209, 104, 184, 211, 110, 178, 205, 
		76, 212, 103, 169, 224, 59, 77, 215, 98, 166, 241, 8, 24, 40, 120, 136, 
		131, 158, 185, 208, 107, 189, 220, 127, 129, 152, 179, 206, 73, 219, 118, 154, 
		181, 196, 87, 249, 16, 48, 80, 240, 11, 29, 39, 105, 187, 214, 97, 163, 
		254, 25, 43, 125, 135, 146, 173, 236, 47, 113, 147, 174, 233, 32, 96, 160, 
		251, 22, 58, 78, 210, 109, 183, 194, 93, 231, 50, 86, 250, 21, 63, 65, 
		195, 94, 226, 61, 71, 201, 64, 192, 91, 237, 44, 116, 156, 191, 218, 117, 
		159, 186, 213, 100, 172, 239, 42, 126, 130, 157, 188, 223, 122, 142, 137, 128, 
		155, 182, 193, 88, 232, 35, 101, 175, 234, 37, 111, 177, 200, 67, 197, 84, 
		252, 31, 33, 99, 165, 244, 7, 9, 27, 45, 119, 153, 176, 203, 70, 202, 
		69, 207, 74, 222, 121, 139, 134, 145, 168, 227, 62, 66, 198, 81, 243, 14, 
		18, 54, 90, 238, 41, 123, 141, 140, 143, 138, 133, 148, 167, 242, 13, 23, 
		57, 75, 221, 124, 132, 151, 162, 253, 28, 36, 108, 180, 199, 82, 246, 1
	};

	memcpy(m_szKey, szValue, 256);
}

CTinyEncode::~CTinyEncode(void)
{
}

bool CTinyEncode::Decode( CString strSor,CString &strDest )
{
	strDest.Empty();

	int nIndex = 0;
	int nSorLen = strSor.GetLength();
	if (0 != nSorLen%2)
	{
		return false;
	}

	unsigned char szTemp[3] = {0};
	int nValue = 0;
	CString strTemp;
	
	while (nIndex < nSorLen/2)
	{	
		szTemp[0] = strSor.GetAt(nIndex*2);
		szTemp[1] = strSor.GetAt(nIndex*2+1);
		strTemp.Format("%c%c",szTemp[0],szTemp[1]);

		nValue = strtoul(strTemp,NULL,16);
		nValue = nValue<0 ? (256+nValue) : nValue; 
		nValue ^= m_szKey[nIndex%256];

		strTemp.Format("%c",nValue);
		strDest += strTemp;

		++nIndex;
	}

	return true;
}

bool CTinyEncode::Encode( CString strSor,CString &strDest )
{
	strDest.Empty();

	int nIndex = 0;
	int nSorLen = strSor.GetLength();

	int nValue = 0;
	CString strTemp;
	
	while (nIndex < nSorLen)
	{	
		nValue = strSor[nIndex];
		nValue = nValue<0 ? (256+nValue) : nValue; 
		nValue ^= m_szKey[nIndex%256];

		strTemp.Format("%02X",nValue);
		strDest += strTemp;

		++nIndex;
	}

	return true;
}

CString CMDIdentify::GetModulePath()
{
	char cFile[MAX_PATH] = {0};  
	GetModuleFileName(NULL, cFile, sizeof(cFile)); 

	CString strTemp = cFile;
	int nPos = strTemp.ReverseFind('\\');
	CString strPath = strTemp.Left(nPos+1);

	return strPath;
}

CString CMDIdentify::LoadCapMaxData( CString strPath /* = ""*/)
{
	if ("" == strPath)
	{
		strPath = GetModulePath() + "CAPMAX.dat";
	}

	CString strReturn = "";

	CFile mfile;
	if (mfile.Open(strPath,CFile::modeRead|CFile::typeBinary))
	{
		char *pBuf = (char *)malloc(mfile.GetLength()+1);
		memset(pBuf,'\0',mfile.GetLength()+1);

		mfile.Read(pBuf,(UINT)mfile.GetLength());
		strReturn = pBuf;
		free(pBuf);

		mfile.Close();
	}

	return strReturn;
}

int CMDIdentify::GetIdentifyData(CString strPath,CString strSQL)
{
	CString strSor = LoadCapMaxData(strPath);
	if (strSor.IsEmpty())
	{
		return 1;
	}

	CString strData;
	if (!Decode(strSor,strData))
	{
		return 2;
	}

	int nStart = strData.Find("SN=");
	int nEnd(0);
	CString strSN;

	if (-1 != nStart)
	{
		nEnd = strData.Find(";;",nStart);
		if (-1 != nEnd)
		{
			strSN = strData.Mid(nStart+3,nEnd-nStart-3);
		}
	}

	CString strSNReal = ReadReg();
	if (strSNReal.IsEmpty() || strSNReal!=strSN)
	{
		return 3;
	}

	nStart = strData.Find("SQL=");
	if (-1 != nStart)
	{
		nEnd = strData.Find(";;",nStart);
		if (-1 != nEnd)
		{
			strSQL = strData.Mid(nStart+4,nEnd-nStart-4);
		}
	}

	strSQL += ";";

	return 0;
}


CString CMDIdentify::ReadReg()
{
	CString strRet;
	CRegKey reg;

	DWORD count = 128;
	char szTemp[MAX_PATH] = {0};

	CString strKey = "SOFTWARE\\BetterComm";
	if(reg.Open(HKEY_LOCAL_MACHINE,strKey) == ERROR_SUCCESS)
	{	
		reg.QueryValue(szTemp,"ServerSN",&count);

		reg.Close();
		strRet = szTemp;
	}

	return strRet;
}

CMDIdentify::CMDIdentify( void )
{

}

CMDIdentify::~CMDIdentify( void )
{

}
