#ifndef __NETCONNECTPOOL_H__
#define __NETCONNECTPOOL_H_

typedef struct connection{
  int fd;//已连接套接字
  /*events是该连接的状态，在epoll_wait监控到事件发生fd就绪后，根据状态来回调rdhandler/wehandler函数
   * 而读写回调函数都在net.c中实现，并且这两个回调函数对应的业务函数也在net.c中实现（run）
   * */
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
