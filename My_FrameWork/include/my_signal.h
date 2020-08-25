#ifndef __SIGNAL_H__
#define __SIGNAL_H___
#include<signal.h>
typedef struct{
  int signo;   //信号的值 比如SIGINT（2）
  const char* signame;//信号的名字比如"SIGINT"
  char* name; //命令的名字
  void (*handler)(int signo,siginfo_t* info,void* ucontext);//信号> 处理函数 
}signal_t;

//回调函数
void sig_handler(int signo,siginfo_t* info,void* ucontext);

//初始化信号处理      
int init_signal();        
   
#endif //__SIGNAL_H__
