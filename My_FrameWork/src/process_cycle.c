//进程相关
#define _GNU_SOURCE
#include <sched.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<config.h>
#include<fun.h>
#include<process_cycle.h>
//主进程创建子进程，然后
void master_process_cyle(){ 
  //先对一些不必要的信号进行阻塞（屏蔽）
  sigset_t set;
  sigemptyset(&set);//清空位图（信号）
  //将set中对应信号的位置置1
  sigaddset(&set,SIGHUP);//终止进程 终端挂断
  sigaddset(&set,SIGINT);//终止进程 ctrl+c
  sigaddset(&set,SIGQUIT);//终止进程 会产生core文档
  sigaddset(&set,SIGABRT);
  sigaddset(&set,SIGBUS);//非法地址，包括内存对齐出错
  sigaddset(&set,SIGUSR1);

  sigaddset(&set,SIGUSR2);
  sigaddset(&set,SIGSEGV);
  sigaddset(&set,SIGPIPE);
  sigaddset(&set,SIGALRM);
  
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGCHLD);
  sigaddset(&set,SIGTTIN);
  sigaddset(&set,SIGURG);
  sigaddset(&set,SIGWINCH);
  sigaddset(&set,SIGIO);
  
  //在创建子进程之前我们屏蔽上述信号，避免在创建子进程期间被打断
  sigprocmask(SIG_BLOCK,&set,NULL);

  //从配置文件获取需要创建几个工作进程(默认为1个)
  int worker_process_num = get_int_default("WorkProc",1);

 start_work_process(worker_process_num,-1);//-1表示新创建子进程,子进程在这个函数中就不会出来了
  
  //由于父进程接下来需要接收一些信号来处理一些事情，比如在子进程退
  //出的时候接收SIGCHLD信号来调用wait回收子进程资源
  sigemptyset(&set);//将位图置空
  
  //设置主进程的名字
  set_proctitle("framework:master");   
  while(1){
    //sigsuspend详解见文档
    sigsuspend(&set); //暂时将我们先前屏蔽的信号暂停，并阻塞在这里当该函数返回后又会恢复成原来的屏蔽状态,所以在此期间任何信号对本进程有效
    
    printf("sigsuspend return\n");
  }
  
}
//num表示需要创建子进程个数，type表示新创建子进程
void start_work_process(int num,int type){
    int i = 0;
    for(int i = 0;i<num;++i){
      spawn_process(i,"frameworker:worker_process",type);//封装子进程（起名字等）
    }
}

//inum：第几个子进程 procname：子进程名字
//创建子进程
void spawn_process(int inum,char* procname,int type){
     pid_t pid = fork();
     switch(pid){
       case -1:
            perror("fork\n"),exit(1);
     
       case 0:    //子进程
            worker_process_cycle(inum,procname);//子进程在该循环中工作
       default: //父进程 直接返回 到start_work_process函数去创建
                //下一个子进程，直到创建完毕，返回至mast_process_cycle()执行接下来的逻辑
          break;
     }
}


//子进程初始化
static void worker_init(int inum){
  //由于子进程会继承父进程的信号屏蔽，所以我们要解除信号屏蔽，使工
  //作进程可以正常接收信号处理业务
   sigset_t set;
   sigemptyset(&set); //清空位图
   sigprocmask(SIG_SETMASK,&set,NULL);//SIG_SETMASK为设置新的位图，而我们这里将set置空，则达到了取消屏蔽的目的 
   //设置CPU亲和
   cpu_set_t cpu;
   CPU_ZERO(&cpu); //初始化置空
   //模取cpu个数 达到分配cpu的目的
   int CpuSize = get_int_default("CpuSize",1);//从配置文件获取CpuSize（cpu个数）
   CPU_SET(inum%CpuSize,&cpu);
  // printf("inum: %d,cpu:%d",inum,inum%CPU_SETSIZE);
   sched_setaffinity(0,sizeof(cpu),&cpu);//绑定当前进程到inum%CPU号cpu上

}


//子进程处理工作
void worker_process_cycle(int inum,char* procname){
     worker_init(inum);//子进程的初始化
     set_proctitle(procname);//设置子进程名字
     while(1){
         //阻塞在epoll_wait()
         sleep(1);
         printf("worker process %d\n",inum);
     }
}




