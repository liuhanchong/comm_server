/*
 * threadpool.h
 *
 *  Created on: 2015年8月3日
 *      Author: liuhanchong
 */

#ifndef HEADER_THREADPOOL_H_
#define HEADER_THREADPOOL_H_

#include "thread.h"
#include "queue.h"
#include "ini.h"
#include "error.h"

typedef struct WorkNode
{
	Thread *pWorkThread;
	time_t tmAccessTime;
	time_t tmExeTime;
} WorkNode;

typedef struct ThreadPool
{
	List workList;
	Thread *pDynAddThread;
	Thread *pFreeOvertimeThread;
	Thread *pExecuteOvertimeThread;
	int *pTaskQueueLength;

	int nMaxThreadNumber;
	int nCoreThreadNumber;

	int nAccOverTime;/*线程未使用时间超时*/
	int nAccThreadLoopSpace;/*线程未使用的判断间隔*/

	int nAddThreadNumber;/*增加线程时候增加的个数*/
	int nAddThreadLoopSpace;/*增加线程时候的判断间隔*/

	int nExeThreadOverTime;/*执行线程的时间超时*/
	int nExeThreadLoopSpace;/*执行线程的判断间隔*/
} ThreadPool;

static ThreadPool queue;

/*接口*/
int CreateThreadPool();
int ReleaseThreadPool();
int GetFreeThreadNumber();/*获取空闲线程个数*/
WorkNode *GetFreeThread();/*获取一个空闲线程*/
void ReleaseThreadNode(WorkNode *pNode);/*释放线程节点*/
int ExecuteTask(void *(*Fun)(void *), void *pData);/*执行线程*/
void SetTaskQueueLength(int *pTaskQueueLength);/*设置任务队列的长度，其是动态变化的*/

/*私有*/
void *AddThread_Dyn(void *pData);/*动态的添加线程*/
void *FreeThread_Acc(void *pData);/*未访问超时线程*/
void *FreeThread_Exe(void *pData);/*执行超时线程*/
int CreateMulThread(int nNumber);
int InsertThread();
void *DefaultThreadFun(void *pData);

#endif /* HEADER_THREADPOOL_H_ */
