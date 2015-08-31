/*
 * aio_x.c
 *
 *  Created on: 2015年8月20日
 *      Author: liuhanchong
 */

#include "../header/aio_x.h"
#include "../header/data.h"
#include "../header/socket.h"

extern struct Data data;
extern struct Socket serverSocket;

int CreateAio(Aio_X *pAio, int nMaxAioQueueLength, int nLoopSpace)
{
	if (!pAio || nMaxAioQueueLength < 1)
	{
		ERROR_DESC("CreateAio", ERR_ARGNULL);
		return 0;
	}

	pAio->nAioId = kqueue();
	if (pAio->nAioId == -1)
	{
		ERROR_DESC("CreateAio", ERR_CREEPOLL);
		return 0;
	}	

	pAio->nMaxAioQueueLength = nMaxAioQueueLength;
	pAio->pEvnetQueue = malloc(nMaxAioQueueLength * sizeof(struct kevent));
	if (!pAio->pEvnetQueue)
	{
		ReleaseAio(pAio);
		ERROR_DESC("CreateAio", ERR_CLOEPOLL);
		return 0;
	}

	pAio->pProAioThread = CreateThread(ProcessAio, NULL, 1, 1, nLoopSpace);
	if (!pAio->pProAioThread)
	{
		ReleaseAio(pAio);
		ERROR_DESC("CreateAio", ERR_CRETHREAD);
		return 0;
	}

	return 1;
}

int ControlAio(int nQueueId, struct kevent *event)
{
	if (kevent(nQueueId, event, 1, NULL, 0, NULL) == -1)
	{
		ERROR_CODEDESC("ControlAio", errno);
		return 0;
	}
	return 1;
}

int RemoveEvent(int nQueueId, int nFd, int nFilter)
{
	struct kevent event = GetEvent(nFd, nFilter, EV_DELETE, NULL);
	return ControlAio(nQueueId, &event);
}

int AdditionEvent(int nQueueId, int nFd, int nFilter, void *pData)
{
	struct kevent event = GetEvent(nFd, nFilter, EV_ADD, pData);
	return ControlAio(nQueueId, &event);	
}

int ModifyEvent(int nQueueId, int nFd, int nFilter, void *pData)
{
	return AdditionEvent(nQueueId, nFd, nFilter, pData);
}

struct kevent GetEvent(int fd, int nFilter, int nFlags, void *pData)
{
	struct kevent event;
	EV_SET(&event, fd, nFilter, nFlags, 0, 0, pData);
	return event;
}

int ReleaseAio(Aio_X *pAio)
{
	if (!pAio)
	{
		ERROR_DESC("ReleaseAio", ERR_ARGNULL);
		return 0;
	}

	if (pAio->pProAioThread)
	{
		ReleaseThread(pAio->pProAioThread);
	}

	if (pAio->pEvnetQueue)
	{
		free(pAio->pEvnetQueue);
		pAio->pEvnetQueue = NULL;
	}

	if (close(pAio->nAioId) != 0)
	{
		ERROR_DESC("ReleaseAio", ERR_CLOEPOLL);
		return 0;
	}
	return 1;
}

void *ProcessAio(void *pData)
{
	if (!serverSocket.aio.pEvnetQueue || serverSocket.aio.nMaxAioQueueLength < 1)
	{
		ERROR_DESC("ProcessAio", ERR_TYPETRAN);
		return NULL;
	}

	struct timespec time = {.tv_sec = 5, .tv_nsec = 0};
	int nQueLen = kevent(serverSocket.aio.nAioId, NULL, 0, serverSocket.aio.pEvnetQueue, serverSocket.aio.nMaxAioQueueLength, &time);
	if (nQueLen == -1)
	{
		ERROR_CODEDESC("ProcessAio", errno);
	}

	struct kevent event;
	for (int i = 0; i < nQueLen; i++)
	{
		event = serverSocket.aio.pEvnetQueue[i];

		//读取数据
		if (event.flags & EVFILT_READ)
		{
			Read(&event);
		}
		//写数据
		else if (event.flags & EVFILT_WRITE)
		{
			Write(&event);
		}
		//错误数据
		else if (event.flags & EV_ERROR)
		{
			if (RemoveEvent(serverSocket.aio.nAioId, event.ident, EV_ERROR) == 0)
			{
				ERROR_CODEDESC("ProcessAio", errno);
			}
			ERROR_DESC("ProcessAio", (char *)event.data);
		}
		else
		{
		}
	}

	return NULL;
}

int Read(struct kevent *event)
{
	int nFd = 0;
	int nReturn = 0;
	SocketNode *pSocketNode = NULL;
	nFd = event->ident;
	pSocketNode = (SocketNode *)event->udata;
	if (!pSocketNode)
	{
		return 0;
	}

	/*获取节点信息*/
	LockQueue((&serverSocket.socketList));

	if (FindDataIndex(&serverSocket.socketList, (const void *)pSocketNode) >= 0)
	{
		memset(pSocketNode->data.pData, 0, pSocketNode->nBuffLength);
		pSocketNode->data.nDataSize = recv(nFd, pSocketNode->data.pData, pSocketNode->nBuffLength, 0);
		if (pSocketNode->data.nDataSize == -1)
		{
			if (RemoveEvent(serverSocket.aio.nAioId, nFd, EVFILT_READ) == 0)
			{
				ERROR_DESC("Read", ERR_LISEPOLL);
			}
			ERROR_CODEDESC("Read", errno);
		}
		else if (pSocketNode->data.nDataSize == 0)//对方关闭了套接字
		{
			if (RemoveEvent(serverSocket.aio.nAioId, nFd, EVFILT_READ) == 0)
			{
				ERROR_DESC("Read", ERR_LISEPOLL);
			}
		}
		else
		{
			pSocketNode->tmAccDateTime = time(NULL);

			//将获取到的数据保存
			DataNode *pDataNode = (DataNode *)malloc(sizeof(DataNode));
			if (pDataNode)
			{
				char *pTmp = (char *)malloc(pSocketNode->data.nDataSize + 1);
				if (pTmp)
				{
					/*保存数据*/
					memcpy(pTmp, pSocketNode->data.pData, pSocketNode->data.nDataSize);
					pTmp[pSocketNode->data.nDataSize] = '\0';
					pDataNode->pData = pTmp;
					pDataNode->nSocket = pSocketNode->nClientSocket;
					pDataNode->nDataSize = pSocketNode->data.nDataSize;

					/*插入数据处理队列*/
					LockQueue((&data.recvDataList));
					Insert(&data.recvDataList, pDataNode, 0);
					UnlockQueue((&data.recvDataList));

					nReturn = 1;
				}
				else
				{
					free(pDataNode);
					pDataNode = NULL;
					ERROR_DESC("Read-1", ERR_MALLOC);
				}
			}
			else
			{
				ERROR_DESC("Read-2", ERR_MALLOC);
			}
		}
	}

	UnlockQueue((&serverSocket.socketList));

	return nReturn;
}

int Write(struct kevent *event)
{
	return 1;
}
