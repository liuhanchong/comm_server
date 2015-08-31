/*
 * data.c
 *
 *  Created on: 2015年8月9日
 *      Author: liuhanchong
 */

#include "../header/data.h"

int InitData()
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/data.ini", 200) != 1)
	{
		ERROR_DESC("InitData", ERR_RDINI);
		return 0;
	}

	data.nMaxListLen = GetInt(&ini, "DATANUMBER", "MaxListLen", 999); /*最大处理数据数量*/
	data.nProDataLoopSpace = GetInt(&ini, "DATANUMBER", "ProDataLoopSpace", 1); /*处理数据时间间隔*/

	ReleaseIni(&ini);

	if (InitQueue(&data.recvDataList, data.nMaxListLen, 0) == 0)
	{
		ERROR_DESC("InitData", ERR_INITQUEUE);
		return 0;
	}

	data.pProThread = CreateThread(ProcessRecvData, NULL, 1, 1, data.nProDataLoopSpace);
	if (!data.pProThread)
	{
		ERROR_DESC("InitData", ERR_CRETHREAD);
		return 0;
	}

	if (CreateThreadPool() == 0)
	{
		ERROR_DESC("InitData", ERR_CREPOOL);
		return 0;
	}

	/*设置动态队列*/
	SetTaskQueueLength(&data.recvDataList.nCurQueueLen);

	return 1;
}

int ReleaseData()
{
	if (data.pProThread)
	{
		ReleaseThread(data.pProThread);
	}

	if (ReleaseThreadPool() == 0)
	{
		ERROR_DESC("ReleaseData", ERR_REPOOL);
	}

	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	BeginTraveData(&data.recvDataList, nIndex, pNode, pData);
		ReleaseDataNode((DataNode *)pData);
	EndTraveData();

	if (ReleaseQueue(&data.recvDataList) == 0)
	{
		ERROR_DESC("ReleaseData", ERR_REQUEUE);
	}

	return 1;
}

void ReleaseDataNode(DataNode *pNode)
{
	if (pNode)
	{
		if (pNode->pData)
		{
			free(pNode->pData);
			pNode->pData = NULL;
		}

		free(pNode);
		pNode = NULL;
	}
}

void *ProcessRecvData(void *pData)
{
	LockQueue((&data.recvDataList));

	QueueNode *pQueueNode = (QueueNode *)GetNodeForIndex(&data.recvDataList, 0);
	if (pQueueNode)
	{
		/*此处分配的datanode内存空间需要执行的线程函数进行销毁*/
		if (ExecuteTask(TestData, pQueueNode->pData) == 1)
		{
			DeleteForNode(&data.recvDataList, pQueueNode);
		}
	}

	UnlockQueue((&data.recvDataList));

	return NULL;
}

void *TestData(void *pData)
{
	DataNode *pDataNode = (DataNode *)pData;
	if (pDataNode)
	{
		if(pDataNode->pData)
		{
			printf("INFOR-socket:%d data:%s\n", pDataNode->nSocket, (char *)pDataNode->pData);
		}

		ReleaseDataNode(pDataNode);
		pDataNode = NULL;
	}

	return NULL;
}

