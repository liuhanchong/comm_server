/*
 * queue.h
 *
 *  Created on: 2015年7月23日
 *      Author: liuhanchong
 */

#ifndef HEADER_QUEUE_H_
#define HEADER_QUEUE_H_

#include <memory.h>
#include <pthread.h>
#include "error.h"
#include "thread.h"

typedef struct QueueNode
{
	void *pData;
	int nPrio;
	struct QueueNode *pNext;
	struct QueueNode *pPre;
} QueueNode;

/*循环链表*/
typedef struct List
{
	int nCurQueueLen;
	int nMaxQueueLen;
	QueueNode *pQueueHead;
	pthread_mutex_t thMutex;
	int nOpenPrio;
} List;

/*遍历节点*/
#define BeginTraveNode(list, nIndex, pNode) \
			nIndex = 0; \
			while (nIndex < GetCurQueueLen(list)) \
			{ \
				pNode = GetNodeForIndex(list, nIndex); \
				if (pNode) \
				{ \

#define EndTraveNode() \
				} \
				nIndex++; \
			} \

/*遍历数据*/
#define BeginTraveData(list, nIndex, pNode, pData) \
			BeginTraveNode(list, nIndex, pNode) \
				pData = pNode->pData; \
				if (pData) \
				{ \

#define EndTraveData() \
				} \
			EndTraveNode() \

/*锁住链表*/
#define LockQueue(list) \
{ \
	if (!list) \
	{ \
		ERROR_DESC("LockQueue", ERR_ARGNULL); \
	} \
	\
	pthread_testcancel(); \
	pthread_cleanup_push(ReleaseResource, (void *)&list->thMutex); \
	pthread_testcancel(); \
	pthread_mutex_lock(&list->thMutex); \

/*解锁链表*/
#define UnlockQueue(list) \
	if (!list) \
	{ \
		ERROR_DESC("UnlockQueue", ERR_ARGNULL); \
	} \
	\
	pthread_mutex_unlock(&list->thMutex); \
	pthread_cleanup_pop(0); \
} \

/*接口*/
int InitQueue(List *list, int nMaxLen, int nOpenPrio);
int ReleaseQueue(List *list);
int GetCurQueueLen(List *list);
void SetMaxQueueLen(List *list, int nMaxLen);
int GetMaxQueueLen(List *list);
int Empty(List *list);
int Insert(List *list, void *pData, int nPrio);
int DeleteForNode(List *list, QueueNode *pData);
int DeleteForIndex(List *list, int nIndex);
QueueNode *GetHead(List *list);
void *GetDataForIndex(List *list, int nIndex);
void *GetDataForNode(QueueNode *pNode);
QueueNode *GetNodeForIndex(List *list, int nIndex);
int FindNodeIndex(List *list, const QueueNode *pData);
int FindDataIndex(List *list, const void *pData);
int ModifyData(List *list, int nIndex, void *pData);

#endif /* HEADER_QUEUE_H_ */
