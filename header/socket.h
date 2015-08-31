/*
 * socket.h
 *
 *  Created on: 2015年8月8日
 *      Author: liuhanchong
 */

#ifndef HEADER_SOCKET_H_
#define HEADER_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include "thread.h"
#include "queue.h"
#include "error.h"
#include "ini.h"
#include "data.h"
#include "aio_x.h"
#include "dbcore.h"

typedef struct SocketNode
{
	struct sockaddr_in sock_addr;
	int nClientSocket;
	time_t tmAccDateTime;
	DataNode data;
	int nBuffLength;/*初始化的buffer长度*/
} SocketNode;

typedef struct Socket
{
	List socketList;
	int nAccThreadLoopSpace;
	int nAccOutTime;
	int nAccOutTimeThreadLoopSpace;/*线程未使用的判断间隔*/
	int nServerSocket;
	Thread *pAccThread;
	Thread *pOutTimeThread;
	int nMaxAcceptSocketNumber;
	int nMaxBufferLength;
	Aio_X aio;//保存aio操作信息
} Socket;

Socket serverSocket;

/*接口*/
int Create(int nDomain, int nType, int nProtocol, int nPort, const char *pIp);
int Listen(int nSocket, int nQueSize);
int Accept(int nSocket);
int Close(int nSocket);
void ReleaseSocketNode(SocketNode *pNode);

/*私有*/
void *AcceptSocket(void *pData);
void *FreeOutTimeSocket(void *pData);

#endif /* HEADER_SOCKET_H_ */
