/*---------------------------------------------------------------------------
�ļ�    : 
����    : ��Ϊ�����ļ�
����    : wsl
�汾    : V1.0
ʱ��    : 2012-02-28
2012-06-30 ����Decode
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "CDDDecode.h"
#include "Public.h"
#include <algorithm> 

CCDDDecode::CCDDDecode(void)
{
}

CCDDDecode::~CCDDDecode(void)
{
}

//-1���ݿ��ܲ����� ���ֶ�����������������
//0�ɹ�
//1ָ��ִ��ʧ��
//2�Ҳ����ַ�
//3�����е���������ͷ��������

int CCDDDecode::Decode(CString strCmd,CString& strData,map<CString,int>& mapHead,vector<vector<CString>>& vecValues,map<CString,CString>& mapPrm)
{
	//ͨ�ý���-------------------------------------------
	//modify by maxliang 20130726 �����¼[DSP LICUSAGE: TYPE=Current;] ����ʧ��
	CString strCmdFlag = strCmd + "%%";;
	int nTatol(0);	
	while (!strData.IsEmpty())
	{
		//��ȡָ���
		CString strPack = cutstr(strData,strCmdFlag,"---    END");
		if(strPack.IsEmpty())
		{
			break;
		}
		//�ж�ָ���Ƿ�ִ�гɹ�
		if(!skipstr(strPack,"ִ�гɹ�"))
		{
			return 1;//ִ��ʧ��
		}

		int nStart(0),nEnd(0);
		if(!isLine(strPack,'-',nStart,nEnd))
		{
			return 2;//�Ҳ����ؼ��ַ�
		}
		strPack.Delete(0,nEnd+1);
		strPack.TrimLeft('\r\n');
		strPack.TrimLeft();
		strPack.TrimLeft('\r\n');
		strPack.TrimLeft();

		//��ȡ��ͷ����
		vector<int> vecClo;//��ǰ����
		int nPos = strPack.Find("\r\n\r\n");
		if (nPos == -1)
		{
			return 2;//���û�п������޷�ȷ����ͷ
		}
		CString strTemp = strPack.Left(nPos);
		strTemp.TrimRight();
		int nClo(0);

		while (!strTemp.IsEmpty())
		{
			CString strName = cutstr(strTemp,"  ");//����
			if (strName.IsEmpty())
			{
				break;
			}
			//int nClo = (int)mapHead.size();
			map<CString,CString>::iterator it = mapPrm.find(strName);
			nClo++;
			if (it == mapPrm.end())
			{
				continue;
			}

			//��������
			mapHead.insert(make_pair(strName,nClo));
			vecClo.push_back(nClo);//�е�λ��
			strTemp.TrimLeft();// ȥ����ߵĿո�

		}

		strPack.Delete(0,nPos + 4);
		nPos = strPack.Find("(�������");
		if(nPos == -1)
		{
			nPos = strPack.Find("���к����������");
		}
		if (nPos != -1)
		{
			strTemp = strPack.Mid(nPos);
			strPack.Delete(nPos,strTemp.GetLength());
		}
		strPack.Delete(strPack.GetLength()-4,4);

		//����Ŀǰ����кŽ���ֵ��ʼ��
		//vector<CString> vecValue;

		//һ��һ��ȡ������
		while (!strPack.IsEmpty())
		{
			CString strLine = cutstr(strPack,'\r\n');
			if (strLine.IsEmpty())
			{
				break;
			}
			strLine.TrimRight();
			vector<CString> vecValue;

			for(int i = 0; i < nClo;i++)
			{
				strLine.TrimLeft();
				CString strValue = cutstr(strLine,"  ");//ÿ��������ֵ 1 2  3 4
				if (vecClo.end() != find(vecClo.begin(),vecClo.end(),i))
				{
					vecValue.push_back(strValue);
				}
			}
			vecValues.push_back(vecValue);//�����е�ֵ
		}

		//�ж��Ƿ����һ��
		CString strRet = cutstr(strTemp,"���н���ܸ��� =","\n");
		if (!strRet.IsEmpty())
		{
			strRet.Trim();
			if (atoi(strRet) == (int)vecValues.size())
			{
				return 0;//��������
			}
		}
	}
	return -1;//���ݿ��ܲ����� ���ֶ�����������������
}

int CCDDDecode::DecodeColumnData(CString strCmd,CString& strData,vector<CString>& vecValues)
{
	//ͨ�ý���-------------------------------------------
	CString strCmdFlag = strCmd + "%%";;
	int nTatol(0);	
	while (!strData.IsEmpty())
	{
		//��ȡָ���
		CString strPack = cutstr(strData,strCmdFlag,"---    END");
		if(strPack.IsEmpty())
		{
			break;
		}
		//�ж�ָ���Ƿ�ִ�гɹ�
		if(!skipstr(strPack,"ִ�гɹ�") && !skipstr(strPack,"�������ֳɹ�"))
		{
			return 1;//ִ��ʧ��
		}

		while (!strPack.IsEmpty())
		{
			//��ȡָ���
			CString strColumnData = cutstr(strPack,"-----\n","(������� =");
			if(strColumnData.IsEmpty())
			{
				break;;
			}
			vecValues.push_back(strColumnData);
		}

		if (vecValues.size() != 0)
			return 0;
	}

	return -1;//���ݿ��ܲ����� ���ֶ�����������������
}

int CCDDDecode::GetMsgFromFile(CString strCmd,CString& strData,const char* strFile)
{
	//ָ�벻��Ϊ��
	if (strFile == NULL)
	{
		return -1;
	}

	strCmd.TrimRight(';');
	if (strCmd.Find(':') == -1)
	{
		strCmd += ":";
	}
	int nCmdLen = strCmd.GetLength();

	strData  = strstr(strFile,strCmd);
	if(strData.IsEmpty())
	{
		return -1;
	}

	return 1;
}