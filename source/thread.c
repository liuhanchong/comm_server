/*
 * thread.c
 *
 *  Created on: 2015年7月20日
 *      Author: liuhanchong
 */

#include "../header/thread.h"

Thread *CreateThread(void *(*Fun)(void *), void *pData, int nCancelMode, int nExecuteMode, int nLoopSecond)
{
	if (Fun == NULL)
	{
		ERROR_DESC("CreateThread", ERR_ARGNULL);
		return NULL;
	}

	int nLength = sizeof(Thread);
	Thread *pThread = (Thread *)malloc(nLength);
	if (!pThread)
	{
		ERROR_DESC("CreateThread", ERR_MALLOC);
		return NULL;
	}

	memset(pThread, 0, nLength);
	if (pthread_mutex_init(&pThread->thMutex, NULL) == 0 &&
			pthread_cond_init(&pThread->thCondition, NULL) == 0 && 
			pthread_attr_init(&pThread->thAttribute) == 0)
	{
		pThread->Fun = Fun;
		pThread->nExecute = 0;
		pThread->pData = pData;
		pThread->nCancelMode = nCancelMode;
		pThread->nLoopSecond = nLoopSecond;
		pThread->nExecuteMode = nExecuteMode;
		if (pthread_create(&pThread->thId, &pThread->thAttribute, DefaultExecuteMode, (void *)pThread) == 0)
		{
			return pThread;
		}
	}

	free(pThread);
	pThread = NULL;

	ERROR_DESC("CreateThread", ERR_CRETHREAD);
	return NULL;

}

int ReleaseThread(Thread *pThread)
{
	if (pThread == NULL)
	{
		ERROR_DESC("ReleaseThread", ERR_ARGNULL);
		return 0;
	}

	if (pthread_cancel(pThread->thId) == 0 &&
			pthread_join(pThread->thId, NULL) == 0 &&
			pthread_mutex_destroy(&pThread->thMutex) == 0 &&
			pthread_cond_destroy(&pThread->thCondition) == 0 &&
			pthread_attr_destroy(&pThread->thAttribute) == 0)
	{
		free(pThread);
		pThread = NULL;
		return 1;
	}

	free(pThread);
	pThread = NULL;

	ERROR_DESC("ReleaseThread", ERR_CLOTHREAD);
	return 0;
}

int PauseThread(Thread *pThread)
{
	if (pThread == NULL)
	{
		ERROR_DESC("PauseThread", ERR_ARGNULL);
		return 0;
	}

	pthread_mutex_lock(&pThread->thMutex);
	pThread->nExecute = 0;
	pthread_mutex_unlock(&pThread->thMutex);
	return 1;
}

int ResumeThread(Thread *pThread)
{
	if (pThread == NULL)
	{
		ERROR_DESC("ResumeThread", ERR_ARGNULL);
		return 0;
	}

	pthread_mutex_lock(&pThread->thMutex);
	pThread->nExecute = 1;
	pthread_cond_signal(&pThread->thCondition);
	pthread_mutex_unlock(&pThread->thMutex);
	return 1;
}

int IsResume(Thread *pThread)
{
	if (pThread != NULL)
	{
		return pThread->nExecute;
	}

	return 0;
}

void SetThreadDetach(Thread *pThread, int nDetach)
{
	if (pThread == NULL)
	{
		ERROR_DESC("SetThreadDetach", ERR_ARGNULL);
		return;
	}
  
  	int nFlag =  (nDetach == 1) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
	if (pthread_attr_setdetachstate(&pThread->thAttribute, nFlag) != 0)
	{
		ERROR_DESC("SetThreadDetach", ERR_DETACH);
		return;
	}
}

void SetThreadExecute(Thread *pThread, void *(*Fun)(void *), void *pData)
{
	if (pThread)
	{
		pThread->Fun = Fun;
		pThread->pData = pData;
	}
}

void *DefaultExecuteMode(void *pData)
{
	Thread *pThread = (Thread *)pData;
	if (pThread == NULL)
	{
		ERROR_DESC("DefaultExecuteMode", ERR_TYPETRAN);
		return NULL;
	}

	if (SetCancelMode(pThread->nCancelMode) == 0)
	{
		ERROR_DESC("DefaultExecuteMode", ERR_CANCELTHREAD);
		return NULL;
	}

	if (pThread->Fun == NULL)
	{
		ERROR_DESC("DefaultExecuteMode", ERR_PRONULL);
		return NULL;
	}

	if (pThread->nExecuteMode == 1)
	{
		STARTTHREAD();
		if (pThread->Fun)
		{
			pThread->Fun(pThread->pData);
		}
		ENDTHREAD(pThread->nLoopSecond);
	}
	else if (pThread->nExecuteMode == 2)
	{
		SUSPENDTHREAD(pThread);
		if (pThread->Fun)
		{
			pThread->Fun(pThread->pData);
		}
		RELEASETHREAD(pThread);
	}

	return NULL;
}

int SetCancelMode(int nMode)
{
	if (nMode)
	{
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0 &&
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0)
		{
			return 1;
		}
	}
	else
	{
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) == 0)
		{
			return 1;
		}
	}

	ERROR_DESC("SetCancelMode", ERR_CANCELTHREAD);
	return 0;
}

void ReleaseResource(void *pData)
{
	pthread_mutex_t *pMutex = (pthread_mutex_t *)pData;
	if (!pMutex)
	{
		ERROR_DESC("ReleaseResource", ERR_TYPETRAN);
		return;
	}

	int nCode = pthread_mutex_unlock(pMutex);
	if (nCode != 0)
	{
		ERROR_CODE("ReleaseResource", nCode);
		return;
	}
}

