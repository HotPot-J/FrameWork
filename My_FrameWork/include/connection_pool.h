#ifndef __CONNECTION_POOL_H__
#define __CONNECTION_POOL_H__
#include"connection.h"
#include<mysql/mysql.h>
#include<pthread.h>
//描述一个连接
typedef struct node{
   MYSQL* conn;
   struct node* next;

}node_t;

//管理连接池的结构体
typedef struct connectionpool{
    char* name;//用户名称
    char* passwd;//数据库密码
    char* db;
    int max_size;//连接池连接上限
    int cur_size;//当前连接池大小
    node_t* head;//队头
    node_t *tail;//队尾
    //由于有多个线程请求连接，所以我们需要进行加锁保护和条件变量实现同步
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}connectionpool_t;

//初始化连接池
void connectionpool_init(connectionpool_t* pool,char* name,char*passwd,char* db,int max_sz);

void connection_pool_push(connectionpool_t* pool,const char*sql);
#endif //__CONNECTION_POOL_H__
