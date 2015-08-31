/*
 * ini.h
 *
 *  Created on: 2015年7月22日
 *      Author: liuhanchong
 */

#ifndef HEADER_INI_H_
#define HEADER_INI_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <wchar.h>
#include "error.h"

//#define WCHAR_INI
#define MAXTEXTLENGTH 1024

#ifdef WCHAR_INI
	typedef wchar_t CHAR;
	#define STRLEN wcslen
	#define STRCMP strcmp
//	#define ATOI _wtoi
//	#define ATOF _wtof
	#define MEMSET(pValue, nSet, nSize) memset(pValue, nSet, (nSize * sizeof(CHAR)))
	#define MEMCPY(pDes, pSrc, nSize) memcpy(pDes, pSrc, (nSize * sizeof(CHAR)))
	#define MALLOC(nSize) malloc((nSize * sizeof(CHAR)))
	#define FREE free
	#define FGETS fgetws
#else
	typedef char CHAR;
	#define STRLEN strlen
	#define STRCMP strcmp
    #define ATOI atoi
    #define ATOF atof
    #define MEMSET(pValue, nSet, nSize) memset(pValue, nSet, (nSize * sizeof(CHAR)))
    #define MEMCPY(pDes, pSrc, nSize) memcpy(pDes, pSrc, (nSize * sizeof(CHAR)))
    #define MALLOC(nSize) malloc(nSize * sizeof(CHAR))
    #define FREE free
	#define FGETS fgets
#endif

typedef struct Ini
{
	FILE *pFile;
	CHAR *pPath;
	CHAR *pText;
	int nRowMaxLength;
} Ini;

/*接口*/
int InitIni(Ini *pIni, CHAR *chPath, int nRowMaxLength);
const CHAR *GetPathName(Ini *pIni);
CHAR  *GetString(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR *pDef);
int GetInt(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault);
int GetUInt(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault);
double GetDouble(Ini *pIni, CHAR *pSection, CHAR *pKey, double dDefault);
void GetArray(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR **pArray, int nLen);
int GetBool(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault);
void ReleaseIni(Ini *pIni);

/*私有*/
void FindValue(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR *pValue, int nSize);
int FindSection(CHAR *pSection,  CHAR *pValue);
int FindKey(CHAR *pKey, CHAR *pValue);
CHAR *GetLine(CHAR *pText, int nSize, FILE *pFile);
void ClearSpace(CHAR *pText, int nSize);
int FindChar(CHAR ch, CHAR *pText);

#endif
