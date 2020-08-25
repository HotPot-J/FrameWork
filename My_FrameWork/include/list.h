#ifndef __LIST_H__
#define __LIST_H__
//结点
typedef struct listnode{
  char* name;
  char* value;
  struct listnode* next;
}list_node;
//链表
typedef struct list{
  list_node* head;
  int len;
}list_t;

//初始化链表
list_t *list_init();

//往链表中插入数据
int list_insert(list_t* head,const char* key,const char* value);

//销毁链表
void list_destroy(list_t* head);

#endif //__LIST_H__
