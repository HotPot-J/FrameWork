#ifndef __GLOBAL_H__
#define __GLOBAL_H__
//定义一些全局变量 让其他文件可以访问
extern char **g_os_argv; //记录操作系统命令行参数的起始位置
extern int g_envmenlen; //环境变量的空间大小
extern char *g_envmen; //环境变量新地址   
#endif //__GLOBAL_H___
