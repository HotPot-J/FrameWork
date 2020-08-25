#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include "threadpool.h"
#include"netconnectpool.h"

#define CONNSIZE 1024
int lfd = 0; //监听套接字
int epfd = 0;//epoll操作句柄
connection_t connections[CONNSIZE];//连接池
connection_t* free_connection = NULL; //连接池中空闲连接的头
threadpool_t  pool;//线程池
struct epoll_event evts[CONNSIZE];//epoll中就绪事件


void epoll_add_event(int fd,int rdev,int wrev,int evtype,connection_t* c);
void event_accept(connection_t* oc);
void wait_request_handler(connection_t* oc);
void write_request_handler(connection_t* oc);

//初始化监听套接字
int listen_init(){
  //1.AF_INET 网络协议使用ipv4 2.SOCK_STREAM流式套接字 TCP 3. 6：套接字所用类型TCP 
   lfd = socket(AF_INET,SOCK_STREAM,6);
  int op = 1;
  setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&op,sizeof(op));//允许重用本地地址和端口int 允许绑定已被使用的地址（或端口号）
  struct sockaddr_in addr;
  addr.sin_family = AF_INET; //ipv4
  addr.sin_port = htons(8080);//主机字节序到网络字节序 可以从配置文件读取
  addr.sin_addr.s_addr = htonl(INADDR_ANY);//指定0.0.0.0的地址，表示本地所有ip
  int ret = bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
  if(ret<0){
    //绑定地址失败
    perror("bind error\n");
  }
  //监听
  listen(lfd,SOMAXCONN); //SOMAXCONN侦听等待队列最多

  return lfd;
}

//从连接池中获取一个空闲连接 并将需要处理的文件描述符传递给它
connection_t* get_connection(int fd){
  //获取到第一个空间连接
  connection_t* cur = free_connection;
  //free_connection移动到下一个空闲连接当中去
  free_connection = cur->data;
  cur->fd = fd;
  return cur;
}

//关闭一个链接（即放回给连接池一个连接）
void close_connection(connection_t* c){
  close(c->fd);//关闭当前连接中的文件描述符
  c->fd = -1;//重新置为无效文件描述符
  c->data = free_connection; //相当于头插到连接池中（归还）
  free_connection = c;
}


//初始化epoll
int epoll_init(){
   //创建epoll操作句柄
   epfd = epoll_create1(EPOLL_CLOEXEC); 
   //初始化一个线程池让他去执行这些具体事件
   threadpool_init(&pool,2);//4是最大线程数量 可以通过配置文件读取

   //初始化一个连接池，供epoll从中添加事件监控起来
   connection_t* c = connections;//让一个临时变量指向该连接池，避免动全局变量
   connection_t* cur = NULL;//帮助连接池的data指针把每个元素串联起来
   int i = CONNSIZE;
   //我们从最后一个连接池节点开始遍历并初始化
   while(i){
       c[i-1].data = cur; 
       c[i-1].fd = -1; //初始化整个连接池的fd先置为无效
       cur = &connections[i-1]; //cur项前走一步
       --i;
   }
   //最终全局变量free_connection指向connections[0] (因为最初连接池全是空闲连接 那么空闲连接的头就是连接池第一个元素)
      free_connection = cur;
      c = get_connection(lfd);//从连接池中获取到一个连接,并将侦听套接字给该连接，准备进行侦听
      c->rdhandler = event_accept;//由于侦听套接字是处理连接的 永远是读事件
                                 //而event_accept就是其处理新到来连接的回调函数
  

  //将侦听套接字添加到epoll侦听事件中
                  //描述符，读，写 ,回调函数 连接池节点
     epoll_add_event(c->fd,1,0,EPOLL_CTL_ADD,c);
}

void event_accept(connection_t* oc){
     //获取连接 拿到新建连接的套接字描述符
     int newfd = accept(oc->fd,NULL,NULL);//不关心对端地址信息就NULL NULL
     
     //拿一个连接池中的连接回来并将新建描述符添加到该连接(分配连接池)
    connection_t* nc   = get_connection(newfd);
    //初始化该连接的读写回调函数
    nc->rdhandler = wait_request_handler; //读
    nc->wrhandler = write_request_handler;//写

    //添加进epoll监听事件
     epoll_add_event(nc->fd,1,0,EPOLL_CTL_ADD,nc);
}

void epoll_add_event(int fd,int rdev,int wrev,int evtype,connection_t* c){
   //对epoll中监控的事件进行操作
  //evtype是需要对事件集合做的事件类型分别：EPOLL_CTL_ADD(添加事件) 修改 删除
    // 操作epoll的函数: int epoll_ctl(int epfd,int op,int fd,struct epoll*event);
    //显然 我们第一步需要组织一个struct epoll_event结构体
     struct epoll_event ev;
    
     if(evtype==EPOLL_CTL_MOD){
       //修改
      //然而我们不能直接去修改（直接修改是无效的）我们需要将其删除，重新组织再进行添加 所有有了接下来的操作：
  //在实际中，我们在监听完一个文件描述符的读事件，必然要开始监听它的写事件
        //先删除该事件
        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev);
        //我们关心的不单单是产生事件的文件描述符，我们关心连接池中一个节点的所有信息：
         ev.data.ptr = c; //让event事件中data联合体中的指针指向当前连接节点
         ev.events = c->events; //让新组织的结构体状态等于当前连接池节点的状态（EPOLLIN/EPOLLOUT）
         if(c->events==EPOLLIN){
              //上一次关心读 修改后就关心写
              ev.events = EPOLLOUT;
              c->events = EPOLLOUT;
         }
         else if(c->events==EPOLLOUT){
              //上一次关心写 修改后就关心读
              ev.events = EPOLLIN;
              c->events = EPOLLIN;
         }
         //重新组织完毕，把该节点添加到epoll监控事件中
         epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
     }
     else if(evtype==EPOLL_CTL_ADD){
       //增加
       if(rdev==1){
          //当前添加的文件描述符事件为读
          ev.events = EPOLLIN;
          c->events = EPOLLIN;
       }
       else if(wrev){
          //写
          ev.events = EPOLLOUT;
          c->events = EPOLLOUT;
       }
        ev.data.ptr = c;
        epoll_ctl(epfd,evtype,fd,&ev);
     }
}

//线程池应该处理的回调函数（业务）
void *run(void* arg){
   connection_t* c = (connection_t*) arg; 
   //业务处理 小写转大写转换（看个效果而已）
   char* cur = c->buffer;
   while(*cur){
     if(*cur>='a'&&*cur<='z'){
       *cur = *cur-32;
     }
   }
   //接受完数据就应该改变当前文件描述符在epoll的监控状态(从监控他的读到监控他的写)
  epoll_add_event(c->fd,0,1,EPOLL_CTL_MOD,c);
}

void wait_request_handler(connection_t* c){
    int ret = read(c->fd,c->buffer,sizeof(c->buffer)-1);//阻塞接收数据
    if(ret<=0){
      //对端关闭
      //将其从epoll监听事件中删除
      epoll_ctl(epfd,EPOLL_CTL_DEL,c->fd,NULL);
      close_connection (c); //将该连接归还给连接池
    }
    //成功接收将该连接池节点扔给线程池去处理
    threadpool_add_task(&pool,run,c);
}
void write_request_handler(connection_t* c){
     write(c->fd,c->buffer,strlen(c->buffer));//发送数据
     //发送完毕后，应该转成epollin事件
     epoll_add_event(c->fd,1,0,EPOLL_CTL_MOD,c);
}


//获取epoll就绪事件

void epoll_process(){
  while(1){
    int ready = epoll_wait(epfd,evts,CONNSIZE,-1); //在就绪事件发生前阻塞在这里（epoll_wait的返回值就是就绪事件的个数）
    int i = 0;
    connection_t* c = NULL;//过去就绪事件中epoll_event中data联合体的ptr指针所指向的连接池节点
    for(i = 0;i<ready;++i){
        c = (connection_t*)evts[i].data.ptr;
        int revents = evts[i].events; //获取其关心的事件（EPOLLIN/EPOLLOUT）    
        if(revents==EPOLLIN){
            (c->rdhandler)(c); //调用连接池节点中保存的读回调函数
        }
        if(revents==EPOLLOUT){
            (c->wrhandler)(c); //调用连接池节点中保存的写回调函数
        }
    }
  }
}














