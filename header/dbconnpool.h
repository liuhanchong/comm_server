/*
 * dbconnpool.h
 *
 *  Created on: 2015年8月17日
 *      Author: liuhanchong
 */

#ifndef HEADER_DBCONNPOOL_H_
#define HEADER_DBCONNPOOL_H_

#include "dbcore.h"
#include "queue.h"
#include "ini.h"
#include "error.h"
#include "thread.h"

typedef struct DBConnNode
{
	MYSQL *pMySql;
	time_t tmAccessTime;
	int nConnection;//是否正在使用
} DBConnNode;

typedef struct DBConnPool
{
	List dbConnList;
	Thread *pFreeOvertimeConn;

	int nMaxConnNumber;
	int nCoreConnNumber;

	int nAccOverTime;/*连接未使用时间超时*/
	int nAccConnLoopSpace;/*连接未使用的判断间隔*/
} DBConnPool;

static DBConnPool connPool;

/*接口*/
int CreateDBConnPool();
int ReleaseDBConnPool();
DBConnNode *GetFreeDBConn();/*获取一个空闲连接*/
void ReleaseDBConnNode(DBConnNode *pNode);/*释放连接池节点*/

/*私有*/
void *FreeDBConn_Acc(void *pData);/*未访问超时连接*/
int CreateMulDBConn(int nNumber);
int InsertDBConn(CHAR *pHost, CHAR *pUser, CHAR *pPasswd, CHAR *pDB, CHAR *pUnixSocket, unsigned long lClientFlag, unsigned int nPort);

#endif /* HEADER_DBCONNPOOL_H_ */
