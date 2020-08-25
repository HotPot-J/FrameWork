//信号处理文件

#include<stdio.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<string.h>
#include<my_signal.h>
/*
//信号结构体(定义再头文件my_signal.h中)
typedef struct{
  int signo;   //信号的值 比如SIGINT（2）
  const char* signame;//信号的名字比如"SIGINT"
  char* name; //命令的名字
  void (*handler)(int signo,siginfo_t* info,void* ucontext);//信号处理函数
}signal_t;
*/

void sig_handler(int signo,siginfo_t* info,void* ucontext){
  printf("recv %d\n",signo);
}

//
signal_t sig_arr[] = {

  {SIGHUP, "SIGHUP", NULL,sig_handler},//终止进程 终端线路挂断
  {SIGINT, "SIGINT", NULL,sig_handler},//SIGINT 程序终止信号（ctl+c）
  {SIGQUIT,"SIGQUIT",NULL,sig_handler},//和SIGINT类似 会产生core文件 ctrl+/
  {SIGCHLD, "SIGCHLD", NULL,sig_handler},//子进程退出时会给父进程发送这样一个信号
  {SIGTERM,"SIGTERM",NULL,sig_handler},//程序结束信号，与SIGKILL不同的是该信号可以被阻塞和处理（SIGKILL不能被阻塞、处理和忽略）
  {SIGSYS,"SIGSYS",NULL,NULL},//非法的系统调用,该信号不用处理
  //......
  {0,NULL,NULL,NULL} //表明改结构体数组结束
};

//信号初始化
int init_signal(){
   signal_t* sig = NULL;
   struct sigaction act;

   for(sig = sig_arr;sig->signo!=0;++sig){
      
       //如果回调函数不为空，再做处理
       memset(&act,0x00,sizeof(struct sigaction));
       if(sig->handler==NULL){
         act.sa_sigaction = (void(*)(int,siginfo_t*,void*))(SIG_IGN)  ;
       }
       else{
       act.sa_sigaction = sig_handler;
       act.sa_flags = SA_SIGINFO;
       }
    //sa_mask表示当前进程在处理信号时，也有可能接收到新的信号，将新的信号放在sa_mask中
    //将sa_mask清空
      sigemptyset(&act.sa_mask);
      sigaction(sig->signo,&act,NULL);

  }
   return 1;
}







