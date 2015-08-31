/*
 * thread.h
 *
 *  Created on: 2015年7月20日
 *      Author: liuhanchong
 */

#ifndef HEADER_THREAD_H_
#define HEADER_THREAD_H_

#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include "error.h"

typedef struct Thread
{
	pthread_t thId;
	pthread_mutex_t thMutex;
	pthread_cond_t thCondition;
	int nExecute;/*线程是否正在执行*/
	void *pData;/*传递的执行函数的参数*/
	void *(*Fun)(void *);/*线程执行函数*/
	int nCancelMode;//取消模式
	int nLoopSecond;//多少秒循环一次
	int nExecuteMode;//执行模式
	pthread_attr_t thAttribute;//线程属性
} Thread;

#define SUSPENDTHREAD(pThread)  \
		{ \
			while (1) \
			{ \
				pthread_testcancel(); \
				pthread_cleanup_push(ReleaseResource, (&(pThread->thMutex))); \
				pthread_testcancel(); \
				pthread_mutex_lock((&(pThread->thMutex))); \
				while (pThread->nExecute == 0) \
				{ \
					pthread_testcancel(); \
					pthread_cond_wait((&(pThread->thCondition)), (&(pThread->thMutex))); \
					pthread_testcancel(); \
				} \

#define RELEASETHREAD(pThread) 	\
		 		pthread_mutex_unlock((&(pThread->thMutex))); \
		 		pthread_testcancel(); \
		 		PauseThread(pThread); \
		 		pthread_cleanup_pop(0); \
			} \
		} \

#define STARTTHREAD()  \
		{ \
			while (1) \
			{ \
				pthread_testcancel(); \

#define ENDTHREAD(SEC)  \
				pthread_testcancel(); \
				if (SEC > 0) \
				{ \
					pthread_testcancel(); \
					sleep(SEC); \
					pthread_testcancel(); \
				} \
				pthread_testcancel(); \
			} \
		} \

/*接口*/
Thread *CreateThread(void *(*Fun)(void *), void *pData, int nCancelMode, int nExecuteMode, int nLoopSecond);
int ReleaseThread(Thread *pThread);
int PauseThread(Thread *pThread);
int ResumeThread(Thread *pThread);
int IsResume(Thread *pThread);
void SetThreadDetach(Thread *pThread, int nDetach);
void SetThreadExecute(Thread *pThread, void *(*Fun)(void *), void *pData);

/*私有*/
void *DefaultExecuteMode(void *pData);
int SetCancelMode(int nMode);
void ReleaseResource(void *pData);

#endif /* HEADER_THREAD_H_ */
