/*
 * threadpool.c
 *
 *  Created on: 2015年8月3日
 *      Author: liuhanchong
 */

#include "../header/threadpool.h"

int CreateThreadPool()
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/threadpool.ini", 200) != 1)
	{
		ERROR_DESC("CreateThreadPool", ERR_RDINI);
		return 0;
	}

	queue.nMaxThreadNumber = GetInt(&ini, "POOLNUMBER", "MaxThreadNumber", 99);
	queue.nCoreThreadNumber = GetInt(&ini, "POOLNUMBER", "CoreThreadNumber", 29);

	queue.nAccOverTime = GetInt(&ini, "ACCOVERTIME", "AccOverTime", 1700);/*线程未使用时间超时*/
	queue.nAccThreadLoopSpace = GetInt(&ini, "ACCOVERTIME", "AccThreadLoopSpace", 500);/*超时访问线程时候的判断间隔*/

	queue.nAddThreadNumber = GetInt(&ini, "ADDTHREAD", "AddThreadNumber", 4);/*增加线程时候增加的个数*/
	queue.nAddThreadLoopSpace = GetInt(&ini, "ADDTHREAD", "AddThreadLoopSpace", 50);/*增加线程时候的判断间隔*/

	queue.nExeThreadOverTime = GetInt(&ini, "EXEOVERTIME", "ExeThreadOverTime", 170);/*执行线程的时间超时*/
	queue.nExeThreadLoopSpace = GetInt(&ini, "EXEOVERTIME", "ExeThreadLoopSpace", 9);/*执行线程的判断间隔*/

	ReleaseIni(&ini);

	queue.pTaskQueueLength = NULL;

	if (InitQueue(&queue.workList, queue.nMaxThreadNumber, 0) == 0)
	{
		ERROR_DESC("CreateThreadPool", ERR_INITQUEUE);
		return 0;
	}

	if (CreateMulThread(queue.nCoreThreadNumber) == 0)
	{
		ERROR_DESC("CreateThreadPool", ERR_CREPOOL);
		return 0;
	}

	queue.pDynAddThread = CreateThread(AddThread_Dyn, NULL, 1, 1, queue.nAddThreadLoopSpace);
	if (!queue.pDynAddThread)
	{
		ERROR_DESC("CreateThreadPool-1", ERR_CRETHREAD);
		return 0;
	}

	queue.pFreeOvertimeThread = CreateThread(FreeThread_Acc, NULL, 1, 1, queue.nAccThreadLoopSpace);
	if (!queue.pFreeOvertimeThread)
	{
		ERROR_DESC("CreateThreadPool-2", ERR_CRETHREAD);
		return 0;
	}

	queue.pExecuteOvertimeThread = CreateThread(FreeThread_Exe, NULL, 1, 1, queue.nExeThreadLoopSpace);
	if (!queue.pExecuteOvertimeThread)
	{
		ERROR_DESC("CreateThreadPool-3", ERR_CRETHREAD);
		return 0;
	}

	return 1;
}

int ReleaseThreadPool()
{
	if (queue.pDynAddThread)
	{
		ReleaseThread(queue.pDynAddThread);
	}

	if (queue.pFreeOvertimeThread)
	{
		ReleaseThread(queue.pFreeOvertimeThread);
	}

	if (queue.pExecuteOvertimeThread)
	{
		ReleaseThread(queue.pExecuteOvertimeThread);
	}

	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	BeginTraveData(&queue.workList, nIndex, pNode, pData);
		ReleaseThreadNode((WorkNode *)pData);
	EndTraveData();

	ReleaseQueue(&queue.workList);

	return 1;
}

int GetFreeThreadNumber()
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	WorkNode *pWorkNode = NULL;
	int nCount = 0;

	BeginTraveData(&queue.workList, nIndex, pNode, pData);
		pWorkNode = (WorkNode *)pData;
		if (IsResume(pWorkNode->pWorkThread) == 0)
		{
			nCount++;
		}
	EndTraveData();

	return nCount;
}

WorkNode *GetFreeThread()
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	WorkNode *pWorkNode = NULL;

	BeginTraveData(&queue.workList, nIndex, pNode, pData);
		pWorkNode = (WorkNode *)pData;
		if (IsResume(pWorkNode->pWorkThread) == 0)
		{
			pWorkNode->tmAccessTime = time(NULL);
			return pWorkNode;
		}
	EndTraveData();

	return NULL;
}

void ReleaseThreadNode(WorkNode *pNode)
{
	if (pNode)
	{
		if (pNode->pWorkThread)
		{
			ReleaseThread(pNode->pWorkThread);
		}

		free(pNode);
		pNode = NULL;
	}
}

int ExecuteTask(void *(*Fun)(void *), void *pData)
{
	WorkNode *pNode = GetFreeThread();
	if (pNode)
	{
		/*设置线程执行的函数和所需要的数据*/
		SetThreadExecute(pNode->pWorkThread, Fun, pData);

		if (ResumeThread(pNode->pWorkThread) == 1)
		{
			pNode->tmExeTime = time(NULL);
			return 1;
		}
	}

	return 0;
}

void SetTaskQueueLength(int *pTaskQueueLength)
{
	queue.pTaskQueueLength = pTaskQueueLength;
}

void *AddThread_Dyn(void *pData)
{
	LockQueue((&queue.workList));

	if (GetFreeThreadNumber() == 0)
	{
		if (queue.pTaskQueueLength != NULL)
		{
			if (*queue.pTaskQueueLength > 10)//目前设定为10个
			{
				CreateMulThread(queue.nAddThreadNumber);
			}
		}
	}

	UnlockQueue((&queue.workList));

	return NULL;
}

void *FreeThread_Acc(void *pData)
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	WorkNode *pWorkNode = NULL;
	time_t tmCurTime = 0;

	LockQueue((&queue.workList));

	BeginTraveData(&queue.workList, nIndex, pNode, pData);
		pWorkNode = (WorkNode *)pData;
		tmCurTime = time(NULL);
		if ((IsResume(pWorkNode->pWorkThread) == 0) && ((tmCurTime - pWorkNode->tmAccessTime) >= queue.nAccOverTime))
		{
			/*当删除的线程超过核心线程数，不用再删除*/
			if (GetCurQueueLen(&queue.workList) <= queue.nCoreThreadNumber)
			{
				break;
			}

			ReleaseThreadNode(pWorkNode);
			DeleteForNode(&queue.workList, pNode);
		}
	EndTraveData();

	UnlockQueue((&queue.workList));

	return NULL;
}

void *FreeThread_Exe(void *pData)
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	WorkNode *pWorkNode = NULL;
	time_t tmCurTime = 0;

	LockQueue((&queue.workList));

	BeginTraveData(&queue.workList, nIndex, pNode, pData);
		pWorkNode = (WorkNode *)pData;
		tmCurTime = time(NULL);
		if ((IsResume(pWorkNode->pWorkThread) == 0) && ((tmCurTime - pWorkNode->tmExeTime) >= queue.nExeThreadOverTime))
		{
			ReleaseThreadNode(pWorkNode);
			DeleteForNode(&queue.workList, pNode);
			InsertThread();
		}
	EndTraveData();

	UnlockQueue((&queue.workList));

	return NULL;
}

int CreateMulThread(int nNumber)
{
	while ((nNumber--) > 0)
	{
		if (InsertThread() == 0)
		{
			ERROR_DESC("CreateMulThread", ERR_CRETHREAD);
		}
	}
	return 1;
}

int InsertThread()
{
	if (GetCurQueueLen(&queue.workList) >= GetMaxQueueLen(&queue.workList))
	{
		ERROR_DESC("InsertThread", ERR_OUTMAXLEN);
		return 0;
	}

	WorkNode *pTmp = (WorkNode *)malloc(sizeof(WorkNode));
	if (!pTmp)
	{
		ERROR_DESC("InsertThread", ERR_MALLOC);
		return 0;
	}

	Thread *pWorkThread = CreateThread(DefaultThreadFun, pTmp, 1, 2, 0);
	if (!pWorkThread)
	{
		free(pTmp);
		pTmp = NULL;
		ERROR_DESC("InsertThread", ERR_CRETHREAD);
		return 0;
	}

	pTmp->pWorkThread = pWorkThread;
	pTmp->tmAccessTime = time(NULL);
	pTmp->tmExeTime = time(NULL);

	int nRet = Insert(&queue.workList, (void *)pTmp, 0);
	if (!nRet)
	{
		ReleaseThreadNode(pTmp);
		ERROR_DESC("InsertThread", ERR_INSELE);
		return 0;
	}

	return 1;
}

void *DefaultThreadFun(void *pData)
{
	return NULL;
}
