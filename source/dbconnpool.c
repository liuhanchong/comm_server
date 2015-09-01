/*
 * dbconnpool.c
 *
 *  Created on: 2015年8月17日
 *      Author: liuhanchong
 */

#include "../header/dbconnpool.h"

int CreateDBConnPool()
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/dbconnpool.ini", 200) != 1)
	{
		ERROR_DESC("CreateDBConnPool", ERR_RDINI);
		return 0;
	}

	connPool.nMaxConnNumber = GetInt(&ini, "POOLNUMBER", "MaxConnNumber", 99);
	connPool.nCoreConnNumber = GetInt(&ini, "POOLNUMBER", "CoreConnNumber", 29);

	connPool.nAccOverTime = GetInt(&ini, "ACCOVERTIME", "AccOverTime", 1700);/*连接未使用时间超时*/
	connPool.nAccConnLoopSpace = GetInt(&ini, "ACCOVERTIME", "AccConnLoopSpace", 500);/*超时访问连接时候的判断间隔*/

	ReleaseIni(&ini);

	if (InitQueue(&connPool.dbConnList, connPool.nMaxConnNumber, 0) == 0)
	{
		ERROR_DESC("CreateDBConnPool", ERR_INITQUEUE);
		return 0;
	}

	if (CreateMulDBConn(connPool.nCoreConnNumber) == 0)
	{
		ERROR_DESC("CreateDBConnPool", ERR_CREPOOL);
		return 0;
	}

	connPool.pFreeOvertimeConn = CreateThread(FreeDBConn_Acc, NULL, 1, 1, connPool.nAccConnLoopSpace);
	if (!connPool.pFreeOvertimeConn)
	{
		ERROR_DESC("CreateDBConnPool", ERR_CRETHREAD);
		return 0;
	}

	return 1;
}

int ReleaseDBConnPool()
{
	if (connPool.pFreeOvertimeConn)
	{
		ReleaseThread(connPool.pFreeOvertimeConn);
	}

	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	BeginTraveData(&connPool.dbConnList, nIndex, pNode, pData);
		ReleaseDBConnNode((DBConnNode *)pData);
	EndTraveData();

	ReleaseQueue(&connPool.dbConnList);

	return 1;
}

DBConnNode *GetFreeDBConn()
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	DBConnNode *pDBConnNode = NULL;
	BeginTraveData(&connPool.dbConnList, nIndex, pNode, pData);
		pDBConnNode = (DBConnNode *)pData;
		if (pDBConnNode->nConnection == 0)
		{
			pDBConnNode->nConnection = 1;//状态为已连接
			pDBConnNode->tmAccessTime = time(NULL);
			return pDBConnNode;
		}
	EndTraveData();

	return NULL;
}

int ReleaseAccessDBConn(DBConnNode *pDBConnNode)
{
	pDBConnNode->nConnection = 0;//状态为已连接
	pDBConnNode->tmAccessTime = time(NULL);
	return 1;
}

void ReleaseDBConnNode(DBConnNode *pNode)
{
	if (pNode)
	{
		if (pNode->pMySql)
		{
			CloseDB(pNode->pMySql);
		}

		free(pNode);
		pNode = NULL;
	}
}

void *FreeDBConn_Acc(void *pData)
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	DBConnNode *pDBConnNode = NULL;
	time_t tmCurTime = 0;

	LockQueue((&connPool.dbConnList));

	BeginTraveData(&connPool.dbConnList, nIndex, pNode, pData);
		pDBConnNode = (DBConnNode *)pData;
		tmCurTime = time(NULL);
		if ((pDBConnNode->nConnection == 0) && ((tmCurTime - pDBConnNode->tmAccessTime) >= connPool.nAccOverTime))
		{
			/*当删除的线程超过核心线程数，不用再删除*/
			if (GetCurQueueLen(&connPool.dbConnList) <= connPool.nCoreConnNumber)
			{
				break;
			}

			ReleaseDBConnNode(pDBConnNode);
			DeleteForNode(&connPool.dbConnList, pNode);
		}
	EndTraveData();

	UnlockQueue((&connPool.dbConnList));

	return NULL;
}

int CreateMulDBConn(int nNumber)
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/db.ini", 200) != 1)
	{
		ERROR_DESC("CreateMulDBConn", ERR_RDINI);
		return 0;
	}

	CHAR *pHost = malloc(ini.nRowMaxLength);
	CHAR *pUser = malloc(ini.nRowMaxLength);
	CHAR *pPasswd = malloc(ini.nRowMaxLength);
	CHAR *pDB = malloc(ini.nRowMaxLength);
	CHAR *pUnixSocket = malloc(ini.nRowMaxLength);
	if (!pHost || !pUser || !pUnixSocket || !pPasswd || !pDB)
	{
		free(pHost);
		pHost = NULL;

		free(pUser);
		pUser = NULL;

		free(pPasswd);
		pPasswd = NULL;

		free(pDB);
		pDB = NULL;

		free(pUnixSocket);
		pUnixSocket = NULL;

		ReleaseIni(&ini);

		ERROR_DESC("CreateMulDBConn", ERR_MALLOC);
		return 0;
	}

	strcpy(pHost, GetString(&ini, "MYSQLDB", "Host", "127.0.0.1"));
	strcpy(pUser, GetString(&ini, "MYSQLDB", "User", "root"));
	strcpy(pPasswd, GetString(&ini, "MYSQLDB", "Passwd", ""));
	strcpy(pDB, GetString(&ini, "MYSQLDB", "DB", "test"));
	strcpy(pUnixSocket, GetString(&ini, "MYSQLDB", "UnixSocket", ""));
	unsigned long lClientFlag = GetInt(&ini, "MYSQLDB", "ClientFlag", 0);
	unsigned int nPort = GetInt(&ini, "MYSQLDB", "Port", 3306);

	ReleaseIni(&ini);

	while ((nNumber--) > 0)
	{
		if (InsertDBConn(pHost, pUser, pPasswd, pDB, pUnixSocket, lClientFlag, nPort) == 0)
		{
			ERROR_DESC("CreateMulDBConn", ERR_CRETHREAD);
		}
	}

	free(pHost);
	pHost = NULL;

	free(pUser);
	pUser = NULL;

	free(pPasswd);
	pPasswd = NULL;

	free(pDB);
	pDB = NULL;

	free(pUnixSocket);
	pUnixSocket = NULL;

	return 1;
}

int InsertDBConn(CHAR *pHost, CHAR *pUser, CHAR *pPasswd, CHAR *pDB, CHAR *pUnixSocket, unsigned long lClientFlag, unsigned int nPort)
{
	if (GetCurQueueLen(&connPool.dbConnList) >= GetMaxQueueLen(&connPool.dbConnList))
	{
		ERROR_DESC("InsertDBConn", ERR_OUTMAXLEN);
		return 0;
	}

	DBConnNode *pTmp = (DBConnNode *)malloc(sizeof(DBConnNode));
	if (!pTmp)
	{
		ERROR_DESC("InsertDBConn", ERR_MALLOC);
		return 0;
	}

	MYSQL *pMySql = OpenDB(pHost, pUser, pPasswd, pDB, pUnixSocket, lClientFlag, nPort);
	if (!pMySql)
	{
		free(pTmp);
		pTmp = NULL;
		ERROR_DESC("InsertDBConn", ERR_DBOPEN);
		return 0;
	}

	pTmp->pMySql = pMySql;
	pTmp->tmAccessTime = time(NULL);
	pTmp->nConnection = 0;

	int nRet = Insert(&connPool.dbConnList, (void *)pTmp, 0);
	if (!nRet)
	{
		ReleaseDBConnNode(pTmp);
		pTmp = NULL;
		ERROR_DESC("InsertDBConn", ERR_INSELE);
		return 0;
	}

	return 1;
}
