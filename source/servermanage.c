/*
 * servermanage.c
 *
 *  Created on: 2015年8月10日
 *      Author: liuhanchong
 */

#include "../header/servermanage.h"

int Start(int nSize, char **aArgArray)
{
	if (nSize < 1 && nSize > 2)
	{
		ERROR_DESC("Start", ERR_SERARGNUMBER);
		return 0;
	}

	//启动服务器
	if (nSize == 1 || strcmp(aArgArray[1], "start") == 0)
	{
		if (CreateShareMemory() == 1)
		{
			if (pShareMemory->nStart != 1)
			{
				if (StartServer() == 1)
				{
					return 1;
				}
			}
		}

		ERROR_DESC("Start", ERR_SERSTA);
		return 0;
	}
	//重启服务器
	else if (strcmp(aArgArray[1], "restart") == 0)
	{
		if (CreateShareMemory() == 1)
		{
			if (pShareMemory->nStart == 1)
			{
				if (RestartServer() == 1)
				{
					return 1;
				}
			}
			else
			{
				if (StartServer() == 1)
				{
					return 1;
				}
			}
		}
	}
	//停止服务器
	else if (strcmp(aArgArray[1], "stop") == 0)
	{
		if (CreateShareMemory() == 1)
		{
			if (pShareMemory->nStart == 1)
			{
				kill(pShareMemory->proId, SIGTERM);//关闭服务器

				if (ReleaseShareMemory() == 1)
				{
					return 1;
				}
				else
				{
					ERROR_DESC("Start", ERR_RESHME);
				}
			}
		}

		ERROR_DESC("Start", ERR_SERWISTA);
		return 0;
	}

	ERROR_DESC("Start", ERR_SERARG);
	return 0;
}

void ProcessMessage()
{
	while (1);
}

int StartServer()
{
	Ini ini;
	if (InitIni(&ini, (CHAR *)"/Users/liuhanchong/Documents/workspace/comm_server/ini/servermanage.ini", 200) != 1)
	{
		ERROR_DESC("StartServer", ERR_RDINI);
		return 0;
	}

	CHAR *pIp = malloc(ini.nRowMaxLength);
	if (!pIp)
	{
		ReleaseIni(&ini);
		
		ERROR_DESC("StartServer-1", ERR_MALLOC);
		return 0;
	}

	strcpy(pIp, GetString(&ini, "SERVER", "IP", "127.0.0.1"));
	int nPort = GetInt(&ini, "SERVER", "PORT", 6001);
	int nListenNumber = GetInt(&ini, "SERVER", "LISNUMBER", 10);

	ReleaseIni(&ini);

	nServerSocket = Create(AF_INET, SOCK_STREAM, 0, nPort, (const char *)pIp);
	free(pIp);
	pIp = NULL;
	if (nServerSocket != -1)
	{
		if (Listen(nServerSocket, nListenNumber) != 0)
		{
			if (Accept(nServerSocket) != 0)
			{
				pShareMemory->proId = getpid();
				pShareMemory->nStart = 1;
				if (signal(SIGTERM, StopServerSignal) != (void *)-1)//激活终止信号
				{
					ProcessMessage();
				}
				return 1;
			}
		}
	}

	ERROR_DESC("StartServer", ERR_STARTSER);
	return 0;
}

int StopServer()
{
	if (nServerSocket >= 0)
	{
		if (Close(nServerSocket) == 1)
		{
			return 1;
		}
	}

	ERROR_DESC("StopServer", ERR_STOPSER);
	return 0;
}

int RestartServer()
{
	system("./comm_server stop");
	sleep(2);
	system("./comm_server start");
	exit(0);
	return 1;
}

int CreateShareMemory()
{
	key_t key = 1905;
	size_t size = sizeof(ShareMemory);
	int nFlag = 0644 | IPC_CREAT;

	nShareMemoryId = shmget(key, size, nFlag);
	if (nShareMemoryId != -1)
	{
		if ((pShareMemory = shmat(nShareMemoryId, NULL, 0)) != (void *)-1)
		{
			return 1;
		}
	}

	ERROR_DESC("CreateShareMemory", ERR_CRESHME);
	return 0;
}

int ReleaseShareMemory()
{
	if (shmdt(pShareMemory) == 0)
	{
		if (shmctl(nShareMemoryId, IPC_RMID, 0) == 0)
		{
			return 1;
		}
	}

	ERROR_DESC("ReleaseShareMemory", ERR_RESHME);
	return 0;
}

void StopServerSignal(int nSignalType)
{
	StopServer();
	exit(0);
}


