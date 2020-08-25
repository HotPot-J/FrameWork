#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
//系统调用IO接口三兄弟,下列三个头文件
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<fun.h> //该文件下函数定义声明在fun.h文件下

//创建守护进程
int  set_daemon(){
    //1.创建子进程1再将父进程退出掉让子进程被init领养
    pid_t pid = fork();
    if(pid){
      //让父进程退出
      exit(0);
    }
    //2.让子进程1重新建立会话组，脱离控制终端
    setsid();
    //3.由于子进程1成为会话组小组长，可以重新打开一个控制终端
    //我们创建子进程2，防止这种情况发生
    pid = fork();
    if(pid){
      //让子进程1退出
      exit(0);
    }
    //子进程2逻辑
    //4.设置运行目录为根目录
    chdir("/");
    //5.清除从父进程继承的文件创建掩模
    umask(0);
    //6.将标准输入输出重定向到黑洞
     int fd = open("/dev/null",O_RDWR);
     dup2(fd,0);
     dup2(fd,1);

     return 0;
}
