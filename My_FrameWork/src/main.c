#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<list.h>
#include<config.h>
#include<global.h>
#include<fun.h>
#include<my_signal.h>
#include<process_cycle.h>
#include<connection.h>
#include<connection_pool.h>
#include<netconnectpool.h>
#include<threadpool.h>
extern char **environ;//内核定义的全局的指向环境变量位置，在这里声明一下

char **g_os_argv; //记录操作系统命令行参数的起始位置
int g_envmenlen; //环境变量的空间大小
char *g_envmen; //环境变量新地址

int main(int argc,char* argv[]){
  g_os_argv = argv;
  init_proctitle();//转移环境变量
 
  //自定义函数在声明<cinfig.h>中
  loda("my.conf");//加载配置文件  
  init_signal();//初始化信号处理，进程收到什么样的信号做什么事情

  //是否应该以守护进程启动，默认不守护
  if(get_int_default("Daemon",0)){
    set_daemon();
  } 
  master_process_cyle();//进入主循环（主线程）


  return 0;
}
