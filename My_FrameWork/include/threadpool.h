#ifndef  __THREADPOOL_H__
#define  __THREADPOOL_H__

#include<pthread.h>

//任务队列节点 
typedef struct{ 
    void* (*callback)(void* args);//回调函数
    void *args; 
    struct node *next;

}task_node;

//线程池结构体
typedef struct{
  pthread_cond_t _cond;//条件变量
  pthread_mutex_t _mutex;//互斥锁
  task_node* head;//任务队列队头
  task_node* tail;//任务队列队尾
  int max_thread;//线程池最多允许创建几个线程个数
  int counter;   //线程池当前的线程个数
  int idle;      //空闲线程的个数
  int quit;      //如果为1表示需要销毁线程池

}threadpool_t;

//初始化线程池
void threadpool_init(threadpool_t* pool,int max_thread);

//往线程池添加任务
void threadpool_add_task(threadpool_t* pool,void*(*task)(void*),void*arg);

//销毁线程池
void threadpool_destroy(threadpool_t* pool);

#endif //__THREADPOOL_H__
