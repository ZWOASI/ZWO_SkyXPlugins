#ifndef XML_CONFIG_H
#define XML_CONFIG_H
#include "tinyxml.h"

#define REG_BINARY                  ( 3 )   // Free form binary
#define REG_DWORD                   ( 4 )   // 32-bit number
#define REG_STRING                  ( 5 )   // string
#define REG_LONG                   ( 6 )   // long number
#define REG_DOUBLE                 ( 7 )
#define REG_BOOL                   ( 8 )
#define REG_INT                   ( 9 )


struct xmlHandle{
	TiXmlDocument* doc;
	TiXmlNode* pItemElem;
};
void clearSpace(char* pszInput);

bool XMLOpenKey(const char* filepath, const char* pszKeyName, xmlHandle* pXmlHandle);

bool XMLCreateKey(const char* filepath, const char* pszKeyName, xmlHandle* pXmlHandle, const char* pszAttr = 0);

void XMLSetValueEx(xmlHandle XmlHandle, const char* pszItemName,  const int* pAttr, int iType, unsigned char* pVal, int iSize);

void XMLCloseKey(xmlHandle* XmlHandle);

bool XMLQueryValueEx(xmlHandle XmlHandle,  const char* pszItemName, int* pAttr, int* iType, unsigned char* pVal, int* iSize);


#endif