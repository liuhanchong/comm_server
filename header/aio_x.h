/*
 * aio_x.h
 *
 *  Created on: 2015年8月20日
 *      Author: liuhanchong
 */

#ifndef HEADER_AIO_X_H_
#define HEADER_AIO_X_H_

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <memory.h>
#include "error.h"
#include "thread.h"
#include "queue.h"

typedef struct Aio_X
{
	struct kevent *pEvnetQueue;
	int nMaxAioQueueLength;
	int nAioId;/*保存epoll/kqueue事件句柄*/
	Thread *pProAioThread;
} Aio_X;

/*接口*/
int CreateAio(Aio_X *pAio, int nMaxAioQueueLength, int nLoopSpace);
int ControlAio(int nQueueId, struct kevent *event);
int RemoveEvent(int nQueueId, int nFd, int nFilter);
int AdditionEvent(int nQueueId, int nFd, int nFilter, void *pData);
int ModifyEvent(int nQueueId, int nFd, int nFilter, void *pData);
struct kevent GetEvent(int fd, int nFilter, int nFlags, void *pData);
int ReleaseAio(Aio_X *pAio);

/*私有*/
void *ProcessAio(void *pData);
int Read(struct kevent *event);
int Write(struct kevent *event);

#endif /* HEADER_AIO_X_H_ */
