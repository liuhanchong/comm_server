#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_TYPETRAN ("强制转换数据失败") //数据转换失败
#define ERR_UNLOCK ("释放互斥锁失败") //释放互斥锁失败
#define ERR_CANCELTHREAD ("设置取消线程失败") //取消线程失败
#define ERR_ARGNULL ("传递的参数为空") //参数传递为空
#define ERR_CRETHREAD ("创建线程失败") //线程创建失败
#define ERR_MALLOC ("内存分配失败") //内存分配失败
#define ERR_CLOTHREAD ("线程关闭失败") //线程关闭失败
#define ERR_OUTMAXLEN ("超出队列的最大长度") //队列溢出失败
#define ERR_INSELE ("插入元素失败") //插入元素失败
#define ERR_SOCKET ("Socket初始化失败") //初始化socket失败
#define ERR_BIND ("Socket绑定失败") //绑定socket失败
#define ERR_LISTEN ("Socket监听失败") //监听socket失败
#define ERR_DATA ("数据处理初始化失败") //数据处理模块的初始化失败
#define ERR_DATARE ("数据处理释放失败") //数据处理模块的释放失败
#define ERR_CLOSE_S ("关闭Socket失败") //关闭socket失败
#define ERR_CRESOCKET ("创建Socket节点失败") //创建socket节点失败
#define ERR_ACCSOCKET ("接收Socket节点失败") //接收socket节点失败
#define ERR_CREAIOCB ("创建Aiocb结构失败") //创建结构失败
#define ERR_CREPOOL ("创建线程池失败") //创建线程池失败
#define ERR_INITQUEUE ("初始化队列失败") //初始化队列失败
#define ERR_AIOREAD ("异步读取失败") //异步读取失败
#define ERR_BUFF ("创建BUFFER失败") //创建BUFFER失败
#define ERR_PRONULL ("线程执行函数为空") //线程无法执行
#define ERR_DETACH ("分离线程失败") //分离线程失败
#define ERR_OPFILE ("打开文件失败") //打开文件失败
#define ERR_DESLOCK ("销毁互斥锁失败") //销毁互斥锁失败
#define ERR_DELNODE ("删除节点失败") //删除节点失败
#define ERR_EDNODE ("修改节点失败") //修改节点失败
#define ERR_EDDATA ("修改数据失败") //修改数据失败
#define ERR_RDINI ("读取配置文件失败") //配置文件失败
#define ERR_REPOOL ("销毁线程池失败") //销毁线程池失败
#define ERR_REQUEUE ("销毁队列失败") //销毁队列失败
#define ERR_SERARGNUMBER ("服务器参数个数错误") //服务器参数个数错误
#define ERR_SERARG ("服务器参数错误") //服务器参数错误
#define ERR_STARTSER ("启动服务器失败") //服务器启动错误
#define ERR_STOPSER ("停止服务器失败") //服务器停止错误
#define ERR_RESTARTSER ("重启服务器失败") //服务器重启错误
#define ERR_CRESHME ("创建共享内存失败") //创建共享内存失败
#define ERR_RESHME ("释放共享内存失败") //释放共享内存失败
#define ERR_SERWISTA ("服务器没有启动") //服务器没有启动
#define ERR_SERSTA ("服务器已经启动") //服务器已经启动
#define ERR_OPESER ("操作服务器失败") //操作服务器失败
#define ERR_SIGENA ("激活信号失败") //激活信号失败
#define ERR_LISEPOLL ("删除epoll监听事件失败") //删除epoll监听事件失败
#define ERR_CREEPOLL ("创建epoll失败") //创建epoll失败
#define ERR_CLOEPOLL ("关闭epoll失败") //关闭epoll失败
#define ERR_MODSOCK ("修改套接字属性失败") //修改套接字属性失败
#define ERR_DBINIT ("数据库初始化失败")
#define ERR_DBCONN ("数据库连接失败")
#define ERR_DBOPEN ("数据库打开失败")
#define ERR_DBCLOSE ("数据库关闭失败")
#define ERR_DBAUTOCOMM ("设置数据库自动提交失败")

/*相关的错误输出宏定义*/
#define ERROR_DESC(FUN, DESC) (printf("错误:Fun-%s, 描述-%s!\n", (FUN), (DESC)))
#define ERROR_CODE(FUN, CODE) (printf("错误:Fun-%s, 错误码-%d!\n", (FUN), (CODE)))
#define ERROR_CODEDESC(FUN, CODE) (printf("错误:Fun-%s, 错误码描述-%s!\n", (FUN), (strerror(CODE))))
#define ERR_SYS(MSG) (perror(MSG))

