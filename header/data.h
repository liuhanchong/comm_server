/*
 * data.h
 *
 *  Created on: 2015年8月9日
 *      Author: liuhanchong
 */

#ifndef HEADER_DATA_H_
#define HEADER_DATA_H_

#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "queue.h"
#include "thread.h"
#include "threadpool.h"
#include "dbconnpool.h"

typedef struct DataNode
{
	int nSocket;
	void *pData;
	int nDataSize;
} DataNode;

typedef struct Data
{
	List recvDataList;
	List sendDataList;
	int nMaxListLen;
	Thread *pProThread;
	int nProDataLoopSpace;
} Data;

Data data;

/*接口*/
int InitData();
int ReleaseData();
void ReleaseDataNode(DataNode *pNode);

/*私有*/
void *ProcessRecvData(void *pData);
void *ProcessSendData(void *pData);

/*测试*/
void *TestData(void *pData);

#endif /* HEADER_DATA_H_ */
