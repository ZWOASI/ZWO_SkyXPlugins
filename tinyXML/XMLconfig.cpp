
#include "tinyxml.h"
#include "XMLconfig.h"
#ifdef _WINDOWS
#include <shlobj.h>
#include <io.h>
#include <direct.h>
#define OutputDbgPrint printf
#else
#include <sys/types.h>
#include<unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define OutputDbgPrint printf
#endif

//bool bModified = false;

typedef char szFolderName[256];

/***************************************************************************
功能：分解键值. 比如software\ASI185MC -> software + ASI185MC
参数：	pFolderName 不为空指针时才复制
返回值：分解后的数量
***************************************************************************/
int getFolderName(const char* pszFullPath, szFolderName* pFolderName)
{
	int iSubLen = 0, iCount = 0;
	const char* psz0, *psz1;
	psz0 = psz1 = pszFullPath;
	char szTemp[256];
	while (*psz1)
	{
		if(*psz1 == '\\' || *psz1 == '/')
		{
			 if(iSubLen > 0)
			 {
				strncpy(szTemp, psz0, iSubLen);
				szTemp[iSubLen] = 0;
				if(pFolderName)
					strcpy(pFolderName[iCount], szTemp);
				iCount++;
			}			
			iSubLen = 0;
			psz0 = psz1 + 1;
			 
		}
		else
			iSubLen++;
		psz1++;
	}
	if(iSubLen > 0)
	{
		strncpy(szTemp, psz0, iSubLen);
		szTemp[iSubLen] = 0;
		if(pFolderName)
			strcpy(pFolderName[iCount], szTemp);
		iCount++;
	}

	return iCount;
}

//szFolderName szDir = {0};

/***************************************************************************
功能：初始化xml文件的保存路径
参数：	
返回值
***************************************************************************/
bool initConfigSaveDir(szFolderName pszDir)
{
//	if(szDir[0])
//		return true;


#ifdef _WINDOWS
	TCHAR m_lpszDefaultDir[MAX_PATH]={0};
	TCHAR wszDocument[MAX_PATH]={0};
	char szDocument[MAX_PATH]={0};
	memset(m_lpszDefaultDir,0,_MAX_PATH);

	LPITEMIDLIST pidl=NULL;
	SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl);
	if (pidl && SHGetPathFromIDList(pidl, wszDocument))
	{
		GetShortPathName(wszDocument,m_lpszDefaultDir,_MAX_PATH);

		int iLength ;  
		//获取字节长度   
		iLength = WideCharToMultiByte(CP_ACP, 0, wszDocument, -1, NULL, 0, NULL, NULL);  
		//将tchar值赋给_char    
		WideCharToMultiByte(CP_ACP, 0, wszDocument, -1, szDocument, iLength, NULL, NULL);
		sprintf(pszDir, "%s\\ZWO\\", szDocument);
		if(access(pszDir, 0) < 0)//如果路径不存在会写文件失败, 所以这里要先新建路径
		{
			if(mkdir(pszDir) < 0)//S_IRWXU|S_IRWXG|S_IRWXO
				return false;
		}
		return true;
	}
	
	return false;

#else
	char *home = 0;
	home = getenv("HOME");
	if (!home) {
		struct passwd *pw = getpwuid(getuid());
		home = pw->pw_dir;
	}
	if(!home)
	{
		OutputDbgPrint("get home path err\n");
		return false;
	}
	
#ifdef _MAC
	sprintf(pszDir, "%s/Library/Application Support/ZWO/", home);
#else
	#ifdef  ANDROID
		sprintf(pszDir, "/data/data/com.zwoasi.smartcontroller/files/");
    #else
		sprintf(pszDir, "%s/.ZWO/", home);
	#endif
#endif
	if(access(pszDir, F_OK) < 0)//如果路径不存在会写文件失败, 所以这里要先新建路径
	{
		if(mkdir(pszDir, S_IRWXU) < 0)//S_IRWXU|S_IRWXG|S_IRWXO
		{
			OutputDbgPrint("mkdir err\n");
			return false;
		}
	
		char mode[] = "0777";//666的话没法写入		
		int iMod;
		iMod = strtol(mode, 0, 8);
		if(chmod (pszDir,iMod) < 0)
			OutputDbgPrint( "chmod error: %s\n", pszDir);
	}
	return true;

#endif

}
/***************************************************************************
功能：清除空格,用下划线代替

返回值：
***************************************************************************/

static const char replace_char[] = " !\"#$%&'()*+,:;<=>?@[]^`{|}~";

const void replaceChar(char *charac)
{
	int sz = sizeof(replace_char)/sizeof(char);
	for(int i = 0; i < sz; i++)
	{
		if(*charac == replace_char[i])
		{
			*charac = '_';
			break;
		}
	}
}


void clearSpace(char* pszInput)
{
	if(pszInput && strlen(pszInput) > 0)
	{		
		int len = strlen(pszInput);
		for(int i = 0; i < len; i++)
		{
		//	if(*pszInput == ' ')			
		//		*pszInput = '_';
			replaceChar(pszInput);
			pszInput++;			
		}		
	}
}
//TiXmlDocument* doc = 0;
/***************************************************************************
功能：打开键
参数：	
pszItemAttr， 键名 如<software>/<config>/ASI185MC
pElem， 指向该键的Node指针
返回值：
***************************************************************************/
//bool XMLOpenKey(const char* filename, const char* pszKeyName, TiXmlNode** pElem)
bool XMLOpenKey(const char* filename, const char* pszKeyName, xmlHandle* pXmlHandle)
{
	TiXmlNode *pElem = 0;
	bool bFind = false;
	szFolderName* pFolder = 0;
	int i = 0, iCount = 0;
	TiXmlNode*  itemRoot = 0;

	szFolderName szDir = {0};

	if(!initConfigSaveDir(szDir))
		return false;
		
	char szfilepath[256];
	sprintf(szfilepath, "%s%s", szDir, filename);


/*	if(pXmlHandle->doc)//先释放 20180420
	{
		delete pXmlHandle->doc;
		pXmlHandle->doc = 0;
	}*/
	TiXmlDocument* doc = new TiXmlDocument(szfilepath);  
	bool loadOkay = doc->LoadFile();  
	if (!loadOkay) {      
		OutputDbgPrint( "Could not load test file %s. Error='%s'. Exiting.\n", szfilepath,doc->ErrorDesc() );  		
		goto END_OPEN;
	}

	char szItem[256];
	strcpy(szItem, pszKeyName);
	clearSpace(szItem);
	
	iCount = getFolderName(szItem, 0);//先得到目录级数
	if(iCount > 0)
	{
		 pFolder = new szFolderName[iCount];//分配内存用
		 getFolderName(szItem, pFolder);//得到文件(夹)名
	}
	else
	{
	//	OutputDbgPrint( "getFolderName err\n"); 
		bFind = true;
		pElem = doc;//根路径
		goto END_OPEN;
	}
	
	itemRoot = doc;	
	for(i = 0; i < iCount; i++)//从根开始寻找目标子项, 如<software>/<config>
	{
		itemRoot = itemRoot->FirstChild( pFolder[i] );//名称相同的子项
		if(!itemRoot)
		{
			OutputDbgPrint( "find child err\n"); 
			goto END_OPEN;
		}
	}
	bFind = true;
	pElem = itemRoot;
	//	TiXmlElement* root = doc->RootElement();  


END_OPEN:
	if(!bFind)//没找到相符的则释放掉
	{
		if(doc)
			delete doc;
		doc = 0;
	}
	if(pFolder)
		delete[] pFolder;

//	bModified = false;

	pXmlHandle->doc = doc;
	pXmlHandle->pItemElem = pElem;

	return bFind;
}


/*
日期
*/
static char* GetDate()
{      	
	static char buf[64];
	int year, mon , day, hour, min, sec;

#ifdef _WINDOWS
	SYSTEMTIME stime;
	GetLocalTime(&stime);
	year = stime.wYear;    
	mon = stime.wMonth;    
	day = stime.wDay;    
	hour = stime.wHour;
	min = stime.wMinute;  
	sec = stime.wSecond;  

#else
	time_t nowtime;    
    struct tm *timeinfo;    
    time( &nowtime );    
    timeinfo = localtime( &nowtime );

    year = timeinfo->tm_year + 1900;    
    mon = timeinfo->tm_mon + 1;    
    day = timeinfo->tm_mday;    
    hour = timeinfo->tm_hour;  
    min = timeinfo->tm_min;  
    sec = timeinfo->tm_sec;  
#endif

	sprintf(buf, "%04d%02d%02d_%02d%02d%02d",year,mon,day,hour,min,sec); 
	return buf;
}

/***************************************************************************
功能：新建键
参数：	
pszItemAttr， 键名 如<software>/<config>/ASI185MC
pElem， 指向该键的Node指针
返回值：
***************************************************************************/
//bool XMLCreateKey(const char* filename, const char* pszKeyName, TiXmlNode** pElem)
bool XMLCreateKey(const char* filename, const char* pszKeyName, xmlHandle* pXmlHandle, const char* pszAttr)
{
	TiXmlNode *pElem = 0;
	szFolderName szDir = {0};
	if(!initConfigSaveDir(szDir))
		return false;

/*	if(pXmlHandle->doc)//先释放
	{
		delete pXmlHandle->doc;
		pXmlHandle->doc = 0;
	}*/

	char szItem[256];
	strcpy(szItem, pszKeyName);
	clearSpace(szItem);

	int iCount = getFolderName(szItem, 0);
	szFolderName* pFolder = 0;
	if(iCount > 0)
	{
		pFolder = new szFolderName[iCount];
		getFolderName(szItem, pFolder);
	}
	else
		return false;

	char szfilepath[256];
	sprintf(szfilepath, "%s%s", szDir, filename);
	TiXmlDocument* doc = new TiXmlDocument(szfilepath);
	bool loadOkay = doc->LoadFile();
	if (!loadOkay) {      //文件不存在
		OutputDbgPrint( "Could not load test file %s. Error='%s'. Create new.\n", szfilepath,doc->ErrorDesc() );  	
		TiXmlDeclaration *dec = new TiXmlDeclaration("1.0","UTF-8","");  //新建文件声明
		doc->LinkEndChild(dec);	
	}
	
	TiXmlNode* itemRoot = doc, *nodeTemp = 0;
	int i = 0;	
	for(i = 0; i < iCount; i++)
	{		
		nodeTemp = itemRoot;
		itemRoot = itemRoot->FirstChild( pFolder[i] );//转到名称符合的子项
		if(!itemRoot)//没找到则新建子项
		{
			TiXmlNode* pNewNode = new TiXmlElement(pFolder[i]);
			((TiXmlElement*)pNewNode)->SetAttribute("date", GetDate());
			if(i == iCount - 1 && pszAttr)//最后的项才添加attr
				((TiXmlElement*)pNewNode)->SetAttribute("attr", pszAttr);
			itemRoot = nodeTemp->LinkEndChild(pNewNode);				
		}
	}
	pElem = itemRoot;


	pXmlHandle->doc = doc;
	pXmlHandle->pItemElem = pElem;

	if(pFolder)
	{
		delete[] pFolder;
	}
	return true;

}
/***************************************************************************
功能：设置值
参数：	
TiXmlNode* pItemElem, 该item的指针
char* pszItemName,  项名
char* pszAttr, 属性
int iType, 类型
unsigned char* pVal, 内容的指针
int iSize, 内容长度
返回值：
***************************************************************************/
//void XMLSetValueEx(TiXmlNode* pItemElem, const char* pszItemName,  const int* pAttr, int iType, unsigned char* pVal, int iSize)//给Item下设置一个子element项
void XMLSetValueEx(xmlHandle XmlHandle, const char* pszItemName,  const int* pAttr, int iType, unsigned char* pVal, int iSize)//给Item下设置一个子element项
{
	TiXmlNode* pItemElem = XmlHandle.pItemElem;
	if (!XmlHandle.doc || !pItemElem)
		return ;
	if(iSize <= 0)
		return;
	bool bFind = false;

	char* szBuf = 0;
	szBuf = new char[2*iSize + 1];//a->0x61 显示为61
	memset(szBuf, 0, 2*iSize + 1);
	char szTemp[3] = {0};
	for(int i = 0; i < iSize; i++)
	{
		sprintf(szTemp, "%02x", pVal[i]);//内存的值用16进制保存,否则如果内存里有0的话就结束了
		strcat(szBuf, szTemp);
	}

	char szElemVal[256];
	strcpy(szElemVal, pszItemName);
	clearSpace(szElemVal);
	
	char szType[8];
	sprintf(szType, "%d", iType);
	for( TiXmlNode*  child = pItemElem->FirstChild(); child; child = pItemElem->IterateChildren(child) )//遍历查找是否已经存在名称相同的element
	{
		TiXmlElement* element = child->ToElement();
		if(!element)
		{
			OutputDbgPrint("content error, remove item\n");
			pItemElem->RemoveChild(child);//20180912内容错误则删除,否则保存时会崩溃,后面会新建
			break;
		}
		
		const char *attr = element->Attribute("type");
		const char* text = element->GetText();
		if(!attr || !text)
		{
			OutputDbgPrint("content error, remove item\n");
			pItemElem->RemoveChild(child);//内容错误则删除,否则保存时会崩溃,后面会新建
			break;
		}
			
		if(!strcmp(child->Value(), szElemVal))//名称相同
		{
			child->FirstChild()->SetValue(szBuf);//设置elememt的第一个子项的值,因为只有一个text子项,所以就是设置text
			((TiXmlElement*)child)->SetAttribute("type", szType);//记录类型属性
			if(pAttr)
			{
				char szAttr[256];
				sprintf(szAttr, "%d", *pAttr);
				((TiXmlElement*)child)->SetAttribute("attr", szAttr);//记录可选属性
			}
		
			bFind = true;
			break;

		}
	}
	if(!bFind)//如果没有同名的则新建一个并插入到item中
	{
		TiXmlNode* NewNode = new TiXmlElement(szElemVal);
		((TiXmlElement*)NewNode)->SetAttribute("type", szType);
		if(pAttr)
		{
			char szAttr[256];
			sprintf(szAttr, "%d", *pAttr);
			((TiXmlElement*)NewNode)->SetAttribute("attr", szAttr);//记录可选属性
		}
		pItemElem->LinkEndChild(NewNode)->LinkEndChild(new TiXmlText(szBuf));//InsertEndChild(TiXmlText(szBuf)的话还要多个拷贝的步骤
	}
//	bModified = true;
	if(szBuf)
		delete[] szBuf;
}
/***************************************************************************
功能：关闭. 释放资源
参数：	

返回值：
***************************************************************************/
void XMLCloseKey(xmlHandle* XmlHandle)
{
	//if(bModified)//如果有修改才保存
	if(!XmlHandle->doc)
		return;
	
	if(!XmlHandle->doc->SaveFile())			
		OutputDbgPrint("Reg Save failed\n");
	
	if(XmlHandle->doc)
		delete XmlHandle->doc;
	XmlHandle->doc = 0;
}
/***************************************************************************
功能：查询值
参数：	
TiXmlNode* pItemElem, 该item的指针
char* pszItemName,  项名
char* pszAttr, 属性
int* iType, 类型
unsigned char* pVal, 内容的指针
int* iSize, 内容长度
返回值：
***************************************************************************/
//bool XMLQueryValueEx(TiXmlNode* pItemElem,  const char* pszItemName, int* pAttr, int* iType, unsigned char* pVal, int* iSize)
bool XMLQueryValueEx(xmlHandle XmlHandle,  const char* pszItemName, int* pAttr, int* iType, unsigned char* pVal, int* iSize)
{
	TiXmlNode* pItemElem = XmlHandle.pItemElem;
	bool bOK = false;

	if (!XmlHandle.doc || !pItemElem)
		return false;

	char szElemVal[256];
	strcpy(szElemVal, pszItemName);
	clearSpace(szElemVal);

	for( TiXmlNode*  child = pItemElem->FirstChild(); child; child = pItemElem->IterateChildren(child) )//遍历查找是否已经存在名称相同的element
	{
		if(!strcmp(child->Value(), szElemVal))//element名称相同
		{
			TiXmlElement* element = child->ToElement();
			if(!element)
				break;
			
			const char *attr = element->Attribute("type");
			if(pAttr)
			{
				const char* attr2= element->Attribute("attr");
				if(attr2)
					*pAttr = atoi(attr2);
			}
			const char* text = element->GetText();  
			if(attr && text)
			{
				*iType = atoi(attr);

				*iSize = strlen(text)/2;
				if(pVal)
				{
					char szTemp[3];
					long lTemp;
					for(int i = 0; i < *iSize; i++)
					{
						memcpy(szTemp, text + i*2, 2);
						szTemp[2] = 0;
						lTemp = strtol(szTemp, NULL, 16);//16进制转10进制
						pVal[i] = lTemp;
					}
				}

				bOK = true;
			}		
			break;
		}
	}
	return bOK;
}


