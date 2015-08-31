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
	while ((nNumber--) > 0)
	{
		if (InsertDBConn() == 0)
		{
			ERROR_DESC("CreateMulDBConn", ERR_CRETHREAD);
		}
	}
	return 1;
}

int InsertDBConn()
{
	return 1;
}
