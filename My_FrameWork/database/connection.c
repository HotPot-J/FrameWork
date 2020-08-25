#include<stdio.h>
#include"connection.h"
//连接数据库
MYSQL* connect(char* name,char* passwd,char*db ){
  MYSQL* my = NULL;
  my = mysql_init(NULL); //初始化一个操作句柄
  if(mysql_real_connect(my,"localhost",name,passwd,db,0,NULL,0)==NULL){
        printf("mysql_real_connect error: %s\n",mysql_error(my));
        return NULL;
  }
  mysql_set_character_set(my,"utf8");
  return my;
}

//插入、删除
int update(MYSQL* my,const char* sql){
  int ret = mysql_query(my,sql);
  if(ret!=0){
    printf("query [%s] fail ,error:%s\n",sql,mysql_error(my));
    return 0;
  }
  return 1;
}

//查询
MYSQL_RES *query(MYSQL* my,char sql){
  return NULL;
}

