/*
 * ini.c
 *
 *  Created on: 2015年7月22日
 *      Author: liuhanchong
 */

#include "../header/ini.h"

int InitIni(Ini *pIni, CHAR *chPath, int nRowMaxLength)
{
	int nPathLen = STRLEN(chPath);
	if (!chPath || !pIni || nRowMaxLength < 1 || nPathLen < 1)
	{
		ERROR_DESC("InitIni", ERR_ARGNULL);
		return 0;
	}

	FILE *pFile = fopen(chPath, "r+");
	if (!pFile)
	{
		ERROR_CODEDESC("InitIni", errno);
		return 0;
	}
	pIni->pFile = pFile;
	
	pIni->pPath = (CHAR *)MALLOC(nPathLen);
	if (!pIni->pPath)
	{
		return 0;
	}
	MEMCPY(pIni->pPath, chPath, nPathLen);

#ifdef WCHAR_INI
	pIni->pText = (CHAR *)MALLOC(nRowMaxLength / 2.0);
#else
	pIni->pText = (CHAR *)MALLOC(nRowMaxLength);
#endif
	if (!pIni->pText)
	{
		FREE(pIni->pPath);
		return 0;
	}
	
	pIni->nRowMaxLength = nRowMaxLength;

	return 1;
}

const CHAR *GetPathName(Ini *pIni)
{
	if (pIni)
	{
		return pIni->pPath;
	}

	return NULL;
}

CHAR *GetString(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR *pDef)
{
	if (!pIni || !pSection || !pKey || !pDef)
	{
		ERROR_DESC("GetString", ERR_ARGNULL);
		return NULL;
	}

	CHAR *pValue = (CHAR *)MALLOC(pIni->nRowMaxLength);
	if (pValue)
	{
		FindValue(pIni, pSection, pKey, pValue, pIni->nRowMaxLength);
	}

	if (STRLEN(pValue) > 0)
	{
		int nSize = STRLEN(pValue);
		MEMCPY(pIni->pText, pValue, nSize);
		pIni->pText[nSize] = '\0';
	}
	else
	{
		MEMSET(pIni->pText, 0, pIni->nRowMaxLength);
		MEMCPY(pIni->pText, pDef, STRLEN(pDef));
	}

	FREE(pValue);
	pValue = NULL;

	return pIni->pText;
}

int GetInt(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ERROR_DESC("GetInt", ERR_ARGNULL);
		return 0;
	}

	GetString(pIni, pSection, pKey, (CHAR *)"");

	if (STRLEN(pIni->pText) == 0)
	{
		return nDefault;
	}

	return ATOI(pIni->pText);
}

int GetUInt(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ERROR_DESC("GetUInt", ERR_ARGNULL);
		return 0;
	}

	int nValue = GetInt(pIni, pSection, pKey, nDefault);

	return (nValue > 0) ? nValue : 0;
}

double GetDouble(Ini *pIni, CHAR *pSection, CHAR *pKey, double dDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ERROR_DESC("GetInt", ERR_ARGNULL);
		return 0;
	}

	GetString(pIni, pSection, pKey, (CHAR *)"");

	return (STRLEN(pIni->pText) == 0) ? dDefault : ATOF(pIni->pText);
}

void GetArray(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR **pArray, int nLen)
{

}

int GetBool(Ini *pIni, CHAR *pSection, CHAR *pKey, int nDefault)
{
	if (!pIni || !pSection || !pKey)
	{
		ERROR_DESC("GetBool", ERR_ARGNULL);
		return 0;
	}

	return ((GetInt(pIni, pSection, pKey, ((nDefault > 0) ? 1 : 0)) > 0) ? 1 : 0);
}

void ReleaseIni(Ini *pIni)
{
	if (pIni)
	{
		fclose(pIni->pFile);
		FREE(pIni->pPath);
		FREE(pIni->pText);
		memset(pIni, 0, sizeof(Ini));
	}
}

void FindValue(Ini *pIni, CHAR *pSection, CHAR *pKey, CHAR *pValue, int nSize)
{
	if (!pIni || !pSection || !pKey || !pValue || nSize <= 0)
	{
		ERROR_DESC("FindValue", ERR_ARGNULL);
		return;
	}

	FILE *pFile = pIni->pFile;
	if (!pFile)
	{
		return;
	}

	if (fseek(pFile, 0, SEEK_SET) != 0)
	{
		return;
	}

	CHAR *pTmp = (CHAR *)MALLOC(pIni->nRowMaxLength);
	if (!pTmp)
	{
		return;
	}

	while (GetLine(pTmp, pIni->nRowMaxLength, pFile))
	{
		if (FindSection(pSection, pTmp))
		{
			while (GetLine(pTmp, pIni->nRowMaxLength, pFile))
			{
				/*找到下一个段，证明在当前段没找到*/
				if ((FindChar('[', pTmp) >= 0) && (FindChar(']', pTmp) >= 0))
				{
					break;
				}

				if (FindKey(pKey, pTmp))
				{
					/*分析数值*/
					int nIndex = FindChar('=', pTmp) + 1;
					MEMCPY(pValue, (pTmp + nIndex), (STRLEN(pTmp) - nIndex));
					pValue[STRLEN(pTmp) - nIndex] = '\0';
					break;
				}
			}
		}
	}

	FREE(pTmp);
	pTmp = NULL;
}

int FindSection(CHAR *pSection, CHAR *pValue)
{
	if (!pSection || !pValue)
	{
		ERROR_DESC("FindSection", ERR_ARGNULL);
		return 0;
	}

	int nBegin = -1;
	int nEnd = -1;
	nBegin = FindChar('[', pValue) + 1;
	nEnd = FindChar(']', pValue);

	int nSize = nEnd - nBegin;
	if (nBegin >= 0 && nEnd >= nBegin && (nSize == STRLEN(pSection)))
	{
		for (int i = 0; i < nSize; i++)
		{
			if (pValue[i + nBegin] != pSection[i])
			{
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

int FindKey(CHAR *pKey, CHAR *pValue)
{
	if (!pKey || !pValue)
	{
		ERROR_DESC("FindKey", ERR_ARGNULL);
		return 0;
	}

	int nIndex = FindChar('=', pValue);
	if (nIndex == STRLEN(pKey))
	{
		for (int i = 0; i < nIndex; i++)
		{
			if (pValue[i] != pKey[i])
			{
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

CHAR *GetLine(CHAR *pText, int nSize, FILE *pFile)
{
	if (!pText || nSize <= 0 || !pFile)
	{
		ERROR_DESC("GetLine", ERR_ARGNULL);
		return 0;
	}

	CHAR *pRet = FGETS(pText, nSize, pFile);
	if (!pRet)
	{
		return 0;
	}

	//删除\r\n
	int nIndex = FindChar('\r', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	nIndex = FindChar('\n', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	//找到;号标志
	nIndex = FindChar(';', pText);
	if (nIndex >= 0)
	{
		pText[nIndex] = '\0';
	}

	//清楚左右的空格
	int nLen = STRLEN(pText);
	ClearSpace(pText, nLen);

	//清除等号左右的空格
	nIndex = FindChar('=', pText);
	if (nIndex >= 0 && nIndex < nLen)
	{
		pText[nIndex] = ' ';

		ClearSpace(pText, nIndex + 1);

		nLen = STRLEN(pText);
		pText[nLen] = '=';

		ClearSpace(pText + nLen + 1, nSize - nLen);
	}

	return pRet;
}

void ClearSpace(CHAR *pText, int nSize)
{
	if (!pText)
	{
		ERROR_DESC("ClearSpace", ERR_ARGNULL);
		return;
	}

	int nBegin = -1;
	int nEnd = -1;
	for (int i = 0; i < nSize; i++)
	{
		if (pText[i] != ' ')
		{
			if (nBegin == -1)
			{
				nBegin = i;
			}

			nEnd = i;
		}
	}

	if (nBegin >= 0 && nEnd >= nBegin)
	{
		int nSize = nEnd - nBegin + 1;
		for (int i = 0; i < nSize; i++)
		{
			pText[i] = pText[i + nBegin];
		}

		pText[nEnd - nBegin + 1] = '\0';
	}
	else
	{
		pText[0] = '\0';
	}
}

int FindChar(CHAR ch, CHAR *pText)
{
	if (!pText)
	{
		ERROR_DESC("FindChar", ERR_ARGNULL);
		return -1;
	}
	
	CHAR *pIndex = pText;
	while (*pIndex != '\0' && *(pIndex) != ch)
	{
		pIndex++;
	}
	
	return (*pIndex == ch) ? (pIndex - pText) : -1;
}
