#ifndef __PROCESS_CYCLE_H__
#define __PROCESS_CYCLE_H__
//主进程循环
void master_process_cyle();

//创建num个工作线程
void start_work_process(int num,int type);

//正式创建子进程(fork())
void spawn_process(int num,char* procname,int type);

//子进程处理工作循环
void worker_process_cycle(int inum,char* procname);

//处理工作之前需要对子进程初始化（取消继承而来的信号屏蔽 绑定cpu）
static void  worker_init(int num);

#endif //__PROCESS_CYCLE_H__
