#pragma once

#include <map>
#include <vector>
using namespace std;

class CCDDDecode
{
public:
	CCDDDecode(void);
public:
	virtual ~CCDDDecode(void);
	static int Decode(CString strCmd,CString& strData,map<CString,int>& mapHead,vector<vector<CString>>& vecValues,map<CString,CString>& mapPrm);
	static int DecodeColumnData(CString strCmd,CString& strData,vector<CString>& vecValues);
	static int GetMsgFromFile(CString strCmd,CString& strData,const char* strFile);
};
