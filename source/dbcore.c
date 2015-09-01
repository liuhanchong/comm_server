/*
 * dbcore.c
 *
 *  Created on: 2015年8月17日
 *      Author: liuhanchong
 */

#include "../header/dbcore.h"

MYSQL *OpenDB(CHAR *pHost, CHAR *pUser, CHAR *pPasswd, CHAR *pDB, CHAR *pUnixSocket, unsigned long lClientFlag, unsigned int nPort)
{
	MYSQL *pMySql = mysql_init(NULL);
	if (!pMySql)
	{
		ERROR_CODEDESC("OpenDB", errno);
		ERROR_DESC("OpenDB", ERR_DBINIT);
		return NULL;
	}

	if (!mysql_real_connect(pMySql, pHost, pUser, pPasswd, pDB, nPort, pUnixSocket, lClientFlag))
	{
		CloseDB(pMySql);
		ERROR_DESC("OpenDB", mysql_error(pMySql));
		ERROR_DESC("OpenDB", ERR_DBCONN);
		return NULL;
	}

	if (mysql_autocommit(pMySql, 0) != 0)
	{
		CloseDB(pMySql);
		ERROR_DESC("OpenDB", mysql_error(pMySql));
		ERROR_DESC("OpenDB", ERR_DBAUTOCOMM);
		return NULL;
	}

	return pMySql;
}

int ExecuteSelect(MYSQL *pMySql, char *sSql)
{
	if (!sSql || !pMySql)
	{
		ERROR_DESC("ExecuteSelect", ERR_ARGNULL);
		return 0;
	}

	if (mysql_query(pMySql, sSql) == 0)
	{
		return 1;
	}

	ERROR_DESC("ExecuteSelect", mysql_error(pMySql));
	return 0;
}

int ExecuteModify(MYSQL *pMySql, char *sSql)
{
	if (!sSql || !pMySql)
	{
		ERROR_DESC("ExecuteModify", ERR_ARGNULL);
		return 0;
	}

	if (mysql_query(pMySql, sSql) != 0)
	{
		ERROR_DESC("ExecuteModify-1", mysql_error(pMySql));
		return 0;
	}

	if (mysql_commit(pMySql) != 0)
	{
		ERROR_DESC("ExecuteModify-2", mysql_error(pMySql));
		return 0;
	}

	return 1;
}

int ExecuteModifyEx(MYSQL *pMySql, char **sSqlArray, int nSize)
{
	if (!sSqlArray || !pMySql || nSize <= 0)
	{
		ERROR_DESC("ExecuteModifyEx", ERR_ARGNULL);
		return 0;
	}

	for (int i = 0; i < nSize; i++)
	{
		if (mysql_query(pMySql, sSqlArray[i]) != 0)
		{
			ERROR_DESC("ExecuteModifyEx", mysql_error(pMySql));

			if (mysql_rollback(pMySql) != 0)
			{
				ERROR_DESC("ExecuteModifyEx", mysql_error(pMySql));
			}
			return 0;
		}
	}

	if (mysql_commit(pMySql) != 0)
	{
		ERROR_DESC("ExecuteModifyEx", mysql_error(pMySql));
		return 0;
	}

	return 1;
}

unsigned long GetAffectRow(MYSQL *pMySql)
{
	if (!pMySql)
	{
		ERROR_DESC("GetEffectRow", ERR_ARGNULL);
		return 0;
	}

	return mysql_affected_rows(pMySql);
}

unsigned long GetRecordCount(MYSQL_RES *pResult)
{
	if (!pResult)
	{
		ERROR_DESC("GetRecordCount", ERR_ARGNULL);
		return 0;
	}

	return mysql_num_rows(pResult);
}

MYSQL_RES *GetRecordResult(MYSQL *pMySql)
{
	if (!pMySql)
	{
		ERROR_DESC("GetRecordResult", ERR_ARGNULL);
		return NULL;
	}

	MYSQL_RES *pResult = mysql_store_result(pMySql);
	if (!pResult)
	{
		ERROR_DESC("GetRecordResult", mysql_error(pMySql));
	}
	return pResult;
}

void ReleaseRecordResult(MYSQL_RES *pResult)
{
	if (!pResult)
	{
		ERROR_DESC("ReleaseRecordResult", ERR_ARGNULL);
	}
	else
	{
		mysql_free_result(pResult);
	}
}

char *GetStringValue(MYSQL_RES *pResult, char *pField)
{
	if (!pResult || !pField)
	{
		ERROR_DESC("GetStringValue", ERR_ARGNULL);
		return NULL;
	}

	unsigned int nColumns = mysql_num_fields(pResult);
	MYSQL_FIELD *pFields = mysql_fetch_fields(pResult);

	char *pValue = NULL;
	MYSQL_ROW rowResult = pResult->current_row;
	if (rowResult && pFields)
	{
		for (unsigned int i = 0; i < nColumns; i++)
		{
			if (strcmp(pFields[i].name, pField) == 0)
			{
				pValue = rowResult[i];
				break;
			}
		}
	}

	return pValue;
}

int GetIntValue(MYSQL_RES *pResult, char *pField)
{
	char *pValue = GetStringValue(pResult, pField);
	if (pValue)
	{
		return atoi(pValue);
	}

	return 0;
}

double GetDoubleValue(MYSQL_RES *pResult, char *pField)
{
	char *pValue = GetStringValue(pResult, pField);
	if (pValue)
	{
		return atof(pValue);
	}

	return 0;
}

float GetFloatValue(MYSQL_RES *pResult, char *pField)
{
	char *pValue = GetStringValue(pResult, pField);
	if (pValue)
	{
		return (float)atof(pValue);
	}

	return 0;
}

int IsEOF(MYSQL_RES *pResult)
{
	if (!pResult)
	{
		ERROR_DESC("IsEOF", ERR_ARGNULL);
		return 1;
	}

	return (mysql_eof(pResult) == 0) ? 0 : 1;
}

int IsActive(MYSQL *pMySql)
{
	if (!pMySql)
	{
		ERROR_DESC("IsActive", ERR_ARGNULL);
		return 0;
	}

	if (mysql_ping(pMySql) != 0)
	{
		ERROR_DESC("IsActive", mysql_error(pMySql));
		return 0;
	}

	return 1;
}

int ResetConnection(MYSQL *pMySql)
{
	if (!pMySql)
	{
		ERROR_DESC("ResetConnection", ERR_ARGNULL);
		return 0;
	}

//	if (mysql_reset_connection(pMySql) != 0)
	{
		ERROR_DESC("ResetConnection", mysql_error(pMySql));
		return 0;
	}

	return 1;
}

int OffsetRecordResult(MYSQL_RES *pResult, int nOffset)
{
	if (!pResult || nOffset < 0)
	{
		ERROR_DESC("OffsetRecordResult", ERR_ARGNULL);
		return 0;
	}

	mysql_data_seek(pResult, nOffset);

	return 1;
}

MYSQL_ROW GetRowResult(MYSQL_RES *pResult)
{
	if (!pResult)
	{
		ERROR_DESC("GetRowResult", ERR_ARGNULL);
		return NULL;
	}

	return mysql_fetch_row(pResult);
}

const char *GetExecuteSqlResultInfor(MYSQL *pMySql)
{
	return mysql_info(pMySql);
}

int CloseDB(MYSQL *pMySql)
{
	if (!pMySql)
	{
		ERROR_DESC("CloseDB", ERR_ARGNULL);
		return 0;
	}

	mysql_close(pMySql);
	return 1;
}


