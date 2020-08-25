#include"connection_pool.h"
#include"connection.h"
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
//初始化连接池
void connectionpool_init(connectionpool_t* pool,char* name,char*passwd,char* db,int max_sz){
  //初始化对连接池的描述
  pool->name = name;
  pool->passwd = passwd;
  pool->db = db;
  pool->max_size = max_sz;
  pool->cur_size = 0;
  pool->head = pool->tail = NULL;//初始化队头队尾相等（即队为空的意思）
  pthread_cond_init(&(pool->cond),NULL);
  pthread_mutex_init(&(pool->mutex),NULL);
}



//存放任务给连接池
void connection_pool_push(connectionpool_t* pool,const char*sql){
  pthread_mutex_lock(&(pool->mutex));
  if(pool->head!=NULL){
    //有空闲队列
    //取一个连接节点 执行任务
    node_t *tmp = pool->head;
    pool->head = tmp->next;
    //怕任务处理时间过长，影响其他线程，在取完连接后应该解锁
    pthread_mutex_unlock(&(pool->mutex));
    update(tmp->conn,sql); 
    pthread_mutex_lock(&(pool->mutex));
    //归还节点：
    tmp->next = pool->head; 
    pool->head = tmp;
  }
  else if(pool->cur_size<pool->max_size){
    //未达到连接池上限，可以创建连接一个新的连接节点使用
    node_t* tmp = (node_t*) malloc(sizeof(node_t)); 
    tmp->conn = connect(pool->name,pool->passwd,pool->db);
    
    //先用该连接执行sql语句
    pthread_mutex_unlock(&(pool->mutex));
    update(tmp->conn,sql); 
    pthread_mutex_lock(&(pool->mutex));
    
    //使用完毕该连接放入连接池中
    tmp->next = pool->head;
    pool->head = tmp;
    pool->cur_size++;//连接池连接个数++
    //归还以后就有空闲的了 唤醒一下等待队列中未拿到连接的线程
    pthread_cond_signal(&(pool->cond));
  }
  else{
    //不可创建连接，只能等待
    while(pool->head==NULL){
      //当无连接又无法创建连接时，一直等待有空闲连接
      pthread_cond_wait(&(pool->cond),&(pool->mutex));
    }
    //当等到空闲连接
    node_t* tmp = pool->head;
    pool->head = tmp->next;
    pthread_mutex_unlock(&(pool->mutex));
    update(tmp->conn,sql); 
    pthread_mutex_lock(&(pool->mutex));
    //归还节点：
    tmp->next = pool->head; 
    pool->head = tmp;
  }
  pthread_mutex_unlock(&(pool->mutex));
}
