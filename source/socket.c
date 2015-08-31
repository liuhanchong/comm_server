/*
 * socket.c
 *
 *  Created on: 2015年8月8日
 *      Author: liuhanchong
 */

#include "../header/socket.h"

int Create(int nDomain, int nType, int nProt, int nPort, const char *pIp)
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/socket.ini", 200) != 1)
	{
		ERROR_DESC("Create", ERR_RDINI);
		return 0;
	}

	serverSocket.nAccOutTime = GetInt(&ini, "ACCOVERTIME", "AccOutTime", 9); /*线程未使用时间超时*/
	serverSocket.nAccOutTimeThreadLoopSpace = GetInt(&ini, "ACCOVERTIME", "AccOutTimeThreadLoopSpace", 49); /*超时访问线程时候的判断间隔*/
	serverSocket.nMaxAcceptSocketNumber = GetInt(&ini, "SOCKETNUMBER", "MaxAcceptSocketNumber", 99);
	serverSocket.nMaxBufferLength = GetInt(&ini, "BUFFER", "MaxBufferLength", 1023);
	serverSocket.nAccThreadLoopSpace = GetInt(&ini, "ACCSOCKET", "AccThreadLoopSpace", 0);
	int nMaxAioQueueLength = GetUInt(&ini, "AIO", "MaxAioQueueLength", 99);
	int nAioLoopSpace = GetUInt(&ini, "AIO", "AioLoopSpace", 0);

	ReleaseIni(&ini);

	if (InitQueue(&serverSocket.socketList, serverSocket.nMaxAcceptSocketNumber, 0) == 0)
	{
		ERROR_DESC("Create", ERR_INITQUEUE);
		return 0;
	}

	serverSocket.nServerSocket = -1;
	serverSocket.pAccThread = NULL;
	serverSocket.pOutTimeThread = NULL;

	/*必须是TCP/IP协议的套接字创建*/
	if (!pIp || nDomain != AF_INET || nType != SOCK_STREAM)
	{
		ERROR_DESC("Create", ERR_TYPETRAN);
		return -1;
	}

	/*创建套接字*/
	int nSocket = socket(nDomain, nType, nProt);
	if (nSocket == -1)
	{
		ERROR_DESC("Create", ERR_SOCKET);
		ERROR_CODEDESC("Create", errno);
		return -1;
	}

	/*设置套接字选项*/
	int nReuse = 1;
	if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(nReuse)) == -1)
	{
		ERROR_DESC("Create", ERR_MODSOCK);
		ERROR_CODEDESC("Create", errno);
	}

	/*绑定套接字*/
	struct sockaddr_in sock_addr;
	int nLen = sizeof(struct sockaddr_in);
	sock_addr.sin_family = nDomain;
	sock_addr.sin_port = htons(nPort);
	sock_addr.sin_addr.s_addr = inet_addr(pIp);
	serverSocket.nServerSocket = nSocket;

	int nRet = bind(nSocket, (struct sockaddr *)&sock_addr, nLen);
	if (nRet == -1)
	{
		Close(nSocket);
		ERROR_CODEDESC("Create", errno);
		ERROR_DESC("Create", ERR_BIND);
		return -1;
	}

	if (CreateAio(&serverSocket.aio, nMaxAioQueueLength, nAioLoopSpace) == 0)
	{
		Close(nSocket);
		ERROR_CODEDESC("Create", errno);
		return -1;
	}

	return nSocket;
}

int Listen(int nSocket, int nQueSize)
{
	if (nSocket < 0 || nQueSize <= 0)
	{
		ERROR_DESC("Listen", ERR_TYPETRAN);
		return 0;
	}

	if (listen(nSocket, nQueSize) == 0)
	{
		return 1;
	}

	ERROR_DESC("Listen", ERR_LISTEN);
	return 0;
}

int Accept(int nSocket)
{
	if (nSocket < 0)
	{
		ERROR_DESC("Accept", ERR_TYPETRAN);
		return 0;
	}

	/*初始化DATA模块*/
	if (InitData() == 0)
	{
		ERROR_DESC("Accept", ERR_DATA);
		return 0;
	}

	serverSocket.pAccThread = CreateThread(AcceptSocket, NULL, 1, 1, serverSocket.nAccThreadLoopSpace);
	if (!serverSocket.pAccThread)
	{
		ERROR_DESC("Accept-1", ERR_CRETHREAD);
		return 0;
	}

	serverSocket.pOutTimeThread = CreateThread(FreeOutTimeSocket, NULL, 1, 1, serverSocket.nAccOutTimeThreadLoopSpace);
	if (!serverSocket.pOutTimeThread)
	{
		ERROR_DESC("Accept-2", ERR_CRETHREAD);
		return 0;
	}

	return 1;
}

int Close(int nSocket)
{
	if (nSocket < 0)
	{
		ERROR_DESC("Close", ERR_TYPETRAN);
		return 0;
	}

	if (serverSocket.pAccThread)
	{
		ReleaseThread(serverSocket.pAccThread);
	}

	if (serverSocket.pOutTimeThread)
	{
		ReleaseThread(serverSocket.pOutTimeThread);
	}

	/*释放data模块*/
	if (ReleaseData() == 0)
	{
		ERROR_DESC("Close", ERR_DATARE);
	}

	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	void *pData = NULL;
	BeginTraveData(&serverSocket.socketList, nIndex, pNode, pData);
		ReleaseSocketNode((SocketNode *)pData);
	EndTraveData();

	if (ReleaseAio(&serverSocket.aio) == 0)
	{
		ERROR_DESC("Close-Epoll", ERR_CLOEPOLL);
	}

	ReleaseQueue(&serverSocket.socketList);

	if (close(nSocket) != 0)
	{
		ERROR_DESC("Close-Socket", ERR_CLOSE_S);
		return 0;
	}

	return 1;
}

void ReleaseSocketNode(SocketNode *pNode)
{
	if (pNode)
	{
		if (close(pNode->nClientSocket) != 0)
		{
			ERROR_DESC("ReleaseSocketNode", ERR_CLOSE_S);
		}

		if (pNode->data.pData)
		{
			free(pNode->data.pData);
			pNode->data.pData = NULL;
		}

		free(pNode);
		pNode = NULL;
	}
}

void *AcceptSocket(void *pData)
{
	//接收异步执行
	int nClientSocket = 0;
	int nSerSocket = 0;
	struct sockaddr_in sock_addr;
	int nLen = 0;
	nSerSocket = serverSocket.nServerSocket;
	nLen = sizeof(struct sockaddr_in);
	nClientSocket = accept(nSerSocket, (struct sockaddr *)&sock_addr, (socklen_t *)&nLen);

	/*异步读取数据*/
	if (nClientSocket >= 0)
	{
		//将客户端socket设置为non_blocked
		if (fcntl(nClientSocket, F_SETFL, O_NONBLOCK) == -1)
		{
			ERROR_DESC("AcceptSocket", ERR_MODSOCK);
		}

		SocketNode *pScInfor = (SocketNode *)malloc(sizeof(SocketNode));
		if (pScInfor)
		{
			pScInfor->sock_addr = sock_addr;
			pScInfor->nClientSocket = nClientSocket;
			pScInfor->tmAccDateTime = time(NULL);
			pScInfor->nBuffLength = serverSocket.nMaxBufferLength;

			/*设置数据信息*/
			pScInfor->data.pData = malloc(pScInfor->nBuffLength);
			pScInfor->data.nDataSize = 0;/*当前buffer中存在的数据长度*/
			pScInfor->data.nSocket = pScInfor->nClientSocket;

			/*异步读取数据*/
			if (pScInfor->data.pData)
			{
				if (AdditionEvent(serverSocket.aio.nAioId, pScInfor->nClientSocket, EVFILT_READ, (void *)pScInfor) == 0)
				{
					ERROR_DESC("AcceptSocket", ERR_AIOREAD);
				}
				else
				{
					LockQueue((&serverSocket.socketList));
					Insert(&serverSocket.socketList, (void *)pScInfor, 0);
					UnlockQueue((&serverSocket.socketList));
				}
			}
			else
			{
				ERROR_DESC("AcceptSocket", ERR_CREAIOCB);
				free(pScInfor);
				pScInfor = NULL;
			}
		}
		else
		{
			ERROR_DESC("AcceptSocket", ERR_CRESOCKET);
		}
	}
	else
	{
		ERROR_DESC("AcceptSocket", ERR_ACCSOCKET);
		ERROR_CODEDESC("AcceptSocket", errno);
	}

	return NULL;
}

void *FreeOutTimeSocket(void *pData)
{
	/*遍历队列列表*/
	int nIndex = 0;
	QueueNode *pNode = NULL;
	SocketNode *pSocketNode = NULL;
	time_t tmCurTime = 0;

	LockQueue((&serverSocket.socketList));

	BeginTraveData(&serverSocket.socketList, nIndex, pNode, pData);
		pSocketNode = (SocketNode *)pData;
		tmCurTime = time(NULL);
		if (((tmCurTime - pSocketNode->tmAccDateTime) >= serverSocket.nAccOutTime))
		{
			ReleaseSocketNode(pSocketNode);
			DeleteForNode(&serverSocket.socketList, pNode);
		}
	EndTraveData();

	UnlockQueue((&serverSocket.socketList));

	return NULL;
}
