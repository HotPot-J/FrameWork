#ifndef __NETCONNECTPOOL_H__
#define __NETCONNECTPOOL_H__

typedef struct connection{
  int fd;//已连接套接字
  int events;//当前状态，EPOLLIN(可读事件) EPOLLOUT(可写事件)
  void (*rdhandler)(struct connection*c);//读回调函数
  void (*wrhandler)(struct connection*c);//写回调函数
  char buffer[2000]; //收发缓冲区
  struct connection *data; //相当于链表的next指针
}connection_t;

//初始化侦听套接字
 int listen_init();
//epoll初始化
int epoll_init();

//从epoll获取就绪事件(并阻塞监控)
void epoll_process();


#endif //__NETCONNECTPOOL_H__
