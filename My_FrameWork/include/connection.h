#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include<mysql/mysql.h>
//连接数据库
MYSQL* connect(char* name,char* passwd,char*db );

//插入、删除
int update(MYSQL* my,const char* sql);

//查询
MYSQL_RES *query(MYSQL* my,char sql);

#endif //__CONNECTION_H__
