/*
 * queue.c
 *
 *  Created on: 2015年7月23日
 *      Author: liuhanchong
 */

#include "../header/queue.h"

int InitQueue(List *list, int nMaxLen, int nOpenPrio)
{
	if (!list || nMaxLen <= 0)
	{
		ERROR_DESC("InitQueue", ERR_ARGNULL);
		return 0;
	}

	if (pthread_mutex_init(&list->thMutex, NULL) == 0)
	{
		list->nCurQueueLen = 0;
		list->pQueueHead = NULL;
		list->nMaxQueueLen = nMaxLen;
		list->nOpenPrio = nOpenPrio;
		return 1;
	}

	ERROR_DESC("InitQueue", ERR_INITQUEUE);
	return 0;
}

int ReleaseQueue(List *list)
{
	if (!list)
	{
		ERROR_DESC("ReleaseQueue", ERR_ARGNULL);
		return 0;
	}

	while (!Empty(list))
	{
		DeleteForNode(list, list->pQueueHead);
	}

	if (pthread_mutex_destroy(&list->thMutex) != 0)
	{
		 ERROR_CODE("ReleaseQueue", errno);
		return 0;
	}

	memset(list, 0, sizeof(sizeof(List)));
	return 1;
}

int GetCurQueueLen(List *list)
{
	if (!list)
	{
		ERROR_DESC("GetCurQueueLen", ERR_ARGNULL);
		return 0;
	}
	return list->nCurQueueLen;
}

int GetMaxQueueLen(List *list)
{
	if (!list)
	{
		ERROR_DESC("GetMaxQueueLen", ERR_ARGNULL);
		return 0;
	}
	return list->nMaxQueueLen;
}

void SetMaxQueueLen(List *list, int nMaxLen)
{
	if (!list || nMaxLen <= 0)
	{
		ERROR_DESC("SetMaxQueueLen", ERR_ARGNULL);
		return;
	}
	list->nMaxQueueLen = nMaxLen;
}

int Empty(List *list)
{
	if (!list)
	{
		ERROR_DESC("Empty", ERR_ARGNULL);
		return 1;
	}
	return ((list->nCurQueueLen == 0) ? 1 : 0);
}

int Insert(List *list, void *pData, int nPrio)
{
	if (!list)
	{
		ERROR_DESC("Insert", ERR_ARGNULL);
		return 0;
	}

	if (list->nCurQueueLen >= list->nMaxQueueLen)
	{
		ERROR_DESC("Insert", ERR_OUTMAXLEN);
		return 0;
	}

	QueueNode *pNode = (QueueNode *)malloc(sizeof(QueueNode));
	if (!pNode)
	{
		ERROR_DESC("Insert", ERR_MALLOC);
		return 0;
	}

	/*填充结构体*/
	pNode->pData = pData;
	pNode->pNext = NULL;
	pNode->pPre = NULL;
	pNode->nPrio = nPrio;

	/*第一次插入元素*/
	if (Empty(list))
	{
		list->pQueueHead = pNode;
		pNode->pNext = pNode;
		pNode->pPre = pNode;
	}
	/*链表存在元素*/
	else
	{
		//找到头指针
		QueueNode *pHead = list->pQueueHead;
		if (!pHead)
		{
			free(pNode);
			pNode = NULL;
			ERROR_DESC("Insert", ERR_INSELE);
			return 0;
		}
		
		//优先级队列
		if (list->nOpenPrio == 1)
		{
			while (pHead->pNext != list->pQueueHead)
			{
				if (pNode->nPrio < pHead->nPrio)
				{
					break;
				}
				
				pHead = pHead->pNext;
			}
			
			pNode->pPre = pHead;
			pNode->pNext = pHead->pNext;
			pHead->pNext->pPre = pNode;
			pHead->pNext = pNode;
		}
		//非优先级队列
		else
		{
			pNode->pPre = pHead->pPre;
			pHead->pPre->pNext = pNode;
			pHead->pPre = pNode;
			pNode->pNext = pHead;
		}
	}

	list->nCurQueueLen++;

	return 1;
}

int DeleteForNode(List *list, QueueNode *pData)
{
	if (!list || !pData || Empty(list))
	{
		ERROR_DESC("DeleteForNode", ERR_ARGNULL);
		return 0;
	}

	if (pData->pNext && pData->pPre)
	{
		pData->pNext->pPre = pData->pPre;
		pData->pPre->pNext = pData->pNext;
		
		//删除第一个元素改变头指针
		if (pData == list->pQueueHead)
		{
			list->pQueueHead = pData->pNext;
		}
		
		free(pData);
		pData = NULL;
		
		list->nCurQueueLen--;

		return 1;
	}

	ERROR_DESC("DeleteForNode", ERR_DELNODE);
	return 0;
}

int DeleteForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ERROR_DESC("DeleteForIndex", ERR_ARGNULL);
		return 0;
	}

	QueueNode *pData = GetNodeForIndex(list, nIndex);
	if (DeleteForNode(list, pData))
	{
		return 1;
	}

	ERROR_DESC("DeleteForIndex", ERR_DELNODE);
	return 0;
}

QueueNode *GetHead(List *list)
{
	if (!list)
	{
		ERROR_DESC("GetHead", ERR_ARGNULL);
		return NULL;
	}

	return GetNodeForIndex(list, 0);
}

void *GetDataForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ERROR_DESC("GetDataForIndex", ERR_ARGNULL);
		return NULL;
	}

	QueueNode *pTmp = GetNodeForIndex(list, nIndex);
	if (!pTmp)
	{
		return NULL;
	}

	return pTmp->pData;
}

void *GetDataForNode(QueueNode *pNode)
{
	if (!pNode)
	{
		ERROR_DESC("GetDataForNode", ERR_ARGNULL);
		return NULL;
	}

	return pNode->pData;
}

QueueNode *GetNodeForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ERROR_DESC("GetNodeForIndex", ERR_ARGNULL);
		return NULL;
	}

	if (Empty(list))
	{
		return NULL;
	}

	QueueNode *pData = list->pQueueHead;
	while (nIndex > 0)
	{
		if (pData->pNext != list->pQueueHead)
		{
			pData = pData->pNext;
		}
		nIndex--;
	}

	return (nIndex == 0) ? pData : NULL;
}

int FindNodeIndex(List *list, const QueueNode *pData)
{
	if (!list || !pData || Empty(list))
	{
		ERROR_DESC("FindNode", ERR_ARGNULL);
		return -1;
	}

	int nIndex = 0;
	QueueNode *pTmp = list->pQueueHead;
	do
	{
		if (pTmp == pData)
		{
			return nIndex;
		}

		pTmp = pTmp->pNext;
		nIndex++;
	} while (pTmp != list->pQueueHead);

	return -1;
}

int FindDataIndex(List *list, const void *pData)
{
	if (!list || !pData || Empty(list))
	{
		ERROR_DESC("FindData", ERR_ARGNULL);
		return -1;
	}

	int nIndex = 0;
	QueueNode *pTmp = list->pQueueHead;
	do
	{
		if (pTmp->pData == pData)
		{
			return nIndex;
		}

		pTmp = pTmp->pNext;
		nIndex++;
	} while (pTmp != list->pQueueHead);

	return -1;
}

int ModifyData(List *list, int nIndex, void *pData)
{
	if (!list || nIndex < 0)
	{
		ERROR_DESC("ModifyData", ERR_ARGNULL);
		return 0;
	}

	QueueNode *pTmp = GetNodeForIndex(list, nIndex);
	if (pTmp)
	{
		pTmp->pData = pData;
		return 1;
	}

	ERROR_DESC("ModifyNode", ERR_EDDATA);
	return 0;
}


