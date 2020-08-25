#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<global.h>
#include<fun.h>  //该文件函数声明在fun.h中
extern char** environ; //保存指向环境变量空间起始地址指针的指针
extern int g_envmenlen; //环境变量长度
extern char *g_envmen;//新开辟的环境变量起始地址
extern char** g_os_argv;//保存指向命令行参数起始位置的指针的指针
//初始化进程名字 并给环境变量搬家  
void init_proctitle(){
   //求出环境变量空间大小
  int i = 0; 
  for(i = 0;environ[i]!=NULL;++i){
      g_envmenlen += strlen(environ[i])+1;
  }

  //为转移环境变量而开辟新空间
  g_envmen = (char*)malloc(sizeof(char)*g_envmenlen);
  memset(g_envmen,0x00,g_envmenlen);
  char* tmp = g_envmen;
  for(i = 0; environ[i]!=NULL;++i){
    int len = strlen(environ[i])+1;
    strcpy(tmp,environ[i]);
    environ[i] = tmp; //将原始的environ[i]也指向tmp（因为系统还是调用environ来寻找环境变量i）
    tmp+=len;//指向下一个环境变量的起始位置
  }
}

//设置进程标题
void set_proctitle(const char* name){
    int title_len = strlen(name)+1;
    int arg_len = 0;
    int i = 0;
    //求命令行参数空间大小
    for(i = 0;g_os_argv[i]!=NULL;++i){
       title_len+=(strlen(g_os_argv[i])+1);
    }
    //如果所设置的进程标题过长(即使加上环境变量的地址空间也不够) 则设置失败
    if(title_len>(arg_len+g_envmenlen)){
        printf("set proc title_len too long!\n");
        return;
    }
    //设置 但只设置进程名 不设置其他
    g_os_argv[1] = NULL;
    char* tmp = g_os_argv[0];
    strcpy(tmp,name);

    //将放置命令行参数的位置清空
    tmp+=title_len;
    memset(tmp,0x00,(arg_len+g_envmenlen)-title_len);
}
