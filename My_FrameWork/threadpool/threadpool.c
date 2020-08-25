#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include"threadpool.h"

 //初始化线程池
void threadpool_init(threadpool_t* pool,int max_thread){
  pthread_cond_init(&(pool->_cond),NULL); //初始化条件变量
  pthread_mutex_init(&(pool->_mutex),NULL);//初始化互斥锁
  pool->head = NULL;             //初始任务队列头尾节点
  pool->tail = NULL;              
  pool->max_thread = max_thread; //初始化最大线程个数
  pool->counter = 0;             //初始化当前线程个数
  pool->idle = 0;                //初始化空闲线程个数
  pool->quit = 0;               //缺省不需要销毁线程池
}


//处理任务的函数
void *routine(void* arg){
    //新的线程是要在这里工作的
    threadpool_t* pool = (threadpool_t*) arg;
    while(1){
        //pool也是一个临界资源
        pthread_mutex_lock(&(pool->_mutex));
        ++(pool->idle); //刚进来的时候没任务所有空闲线程个数多了一个

        while(pool->head==NULL&&pool->quit==0){
             //任务队列中没有任务且销毁标志位为0 则在等待队列中等待被唤醒
             pthread_cond_wait(&(pool->_cond),&(pool->_mutex));//该接口之所
      //以需要传递mutex就是因为，此时该线程需要阻塞在对列中等待，不需要拿着/
      //互斥锁不放，使得其他线程无法访问线程池,但是当条件满足被唤醒，又会自动上锁，去访问临界资源。
        }
        //走到这里说明来任务或者需要销毁线程池了
        //空闲线程个数一定需要--
        --(pool->idle);
        if(pool->head!=NULL){
            task_node* tmp = pool->head;
            pool->head = tmp->next; //出队
            //此时以及拿出来节点了 就可以解锁了 防止任务函数过长
            //影响其他线程去操作线程池中的数据
            pthread_mutex_unlock(&(pool->_mutex));
            tmp->callback(tmp->args); //真正执行任务
            free(tmp); //释放空间防止内存泄漏
            pthread_mutex_lock(&(pool->_mutex));
        }
        if(pool->quit==1&&pool->head==NULL){
          //如果退出标志位为1且没有任务可做 就销毁
            pool->counter--;  //线程个数-1
            if(pool->counter==0){
              printf("唤醒destroy中的wait\n");
              //当线程池中最后一个线程退出了，再唤醒在destory中阻塞
              //在最后的wait，使得整个destroy完成
              pthread_cond_signal(&(pool->_cond));
            }
            pthread_mutex_unlock(&(pool->_mutex));//解锁 
            break; //跳出循环后该线程任务结束，由于在创建之初以及进行线程分离
                 //所以系统会回收该线程资源
        }
        pthread_mutex_unlock(&(pool->_mutex));
    
    } 
}


 //往线程池添加任务
void threadpool_add_task(threadpool_t* pool,void *(task)(void*),void* arg){
         //先生产包装一个任务
         task_node* newnode = (task_node*)malloc(sizeof(task_node));
         newnode->callback = task;
         newnode->args = arg;
         newnode->next = NULL;
 
    /*接下来需要对任务队列进行操作 由于生产者和消费者会从任务队列
     * 中增减数据，而且互斥锁也是全局的，只有一个，多线程对其内部数据访问也会造成线程安全问题，我们需要用互斥锁保护
    */
         pthread_mutex_lock(&(pool->_mutex)); //给任务队列上锁

         //进行入队操作（可以单独封装一个队列，这里我直接实现吧）
         if(pool->head==NULL){
           //队列为空
           pool->head = newnode;
         }
         else{pool->tail->next = newnode;}

         //入队后，需要唤醒消费者来消费，即让线程池的线程
         //来处理该任务，我们要判断当前线程池线程个数情况
         if(pool->idle>0){
           //此时线程池有空闲线程 拿出来用就完了
           pthread_cond_signal(&(pool->_cond));
           --(pool->idle);
         }
         else if(pool->counter<pool->max_thread){
           //如果没有空闲的线程 如果线程池中的线程数量小于最大
           //线程个数，直接创建一个线程让他去完成该任务
            pthread_t tid;
            pthread_create(&tid,NULL,routine,(void*)pool);
            pool->counter++; //当前线程池新增一个线程
            //让刚创建出来的线程分离避免内存泄漏
            pthread_detach(tid);
         }
         //如果没空闲线程，也不够创建新线程的条件，就只能等待有了
         //再说。
         
         pthread_mutex_unlock(&(pool->_mutex));//使用完队列解锁
}

 //销毁线程池
void threadpool_destroy(threadpool_t* pool){
  if(pool->quit==1){
    //销毁标志位已经是1了就不用继续了 
    return;
  }
  pthread_mutex_lock(&(pool->_mutex));
  pool->quit = 1; //销毁标志位置1
  //去通知空闲线程队列中阻塞的线程 该销毁了（唤醒等待队列中的线程）
  if(pool->counter>0){ //当线程池中有线程
    if(pool->idle>0){ //并且有空闲线程（说明等待队列中有等待着的线程）
      pthread_cond_broadcast(&pool->_cond);//该接口和signal的区别就是唤醒队列中所用的等待线程
      //当唤醒了以后在routine函数中，线程从wait中被唤醒，继续执行下面的
      //逻辑 最终会退出while(1)，从而结束掉线程
     }
      
       //但是此时并不一定所有线程都被销毁了（有些线程可能在执行任务中）
       //不会到退出的逻辑，直到任务执行完毕，所以我们应该让调用destroy的线程阻塞在此
       //否则，该函数一旦结束，调用这个任务的进程就结束了（一般都是在
       //进程快结束的时候调用destroy接口），当进程结束，未完成工作的线程
       //也就结束了，会造成问题
         while(pool->counter>0) //当线程池中未清理完 当前调用destroy的线程一直阻塞等待线程池中的线程退出完毕(要明确，线程池中的线程都是在他们自己的
//routine函数的while(1)结束后就退出了 与调用destroy的线程无关,当对后一个
//线程退出前，让pool->counter--为0后会唤醒调用destroy的线程，让其退出掉
         pthread_cond_wait(&(pool->_cond),&(pool->_mutex));

    } 
    pthread_mutex_unlock(&(pool->_mutex));
    
    pthread_cond_destroy(&(pool->_cond));
    pthread_mutex_destroy(&(pool->_mutex));
}
                                     
