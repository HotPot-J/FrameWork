#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list.h>
//初始化链表
list_t *list_init(){
  list_t* p = (list_t*)malloc(sizeof(list_t));
  //初始化结点为空
  memset(p,0x00,sizeof(list_t)); 
  p->head = NULL; 
  p->len = 0;
  return p;
}

//往链表中插入数据
int list_insert(list_t* list,const char* key,const char* value){
    
  //初始化即将插入的结点
    list_node* newnode =NULL;
    
    newnode = (list_node*)malloc(sizeof(list_node));
    memset(newnode,0x00,sizeof(list_node));
    
    newnode->name = (char*)malloc(strlen(key)+1); 
    memset(newnode->name,0,strlen(key)+1);
    strncpy(newnode->name,key,strlen(key));

    newnode->value = (char*)malloc(strlen(value)+1); 
    memset(newnode->value,0,strlen(value)+1);
    strncpy(newnode->value,value,strlen(value));
    
    newnode->next = NULL;
    //头插
    if(list->head==NULL){
      list->head = newnode;
    }
    else{
      newnode->next = list->head;
      list->head = newnode;
    }
    ++list->len;
    return 1;
}

//销毁链表
void list_destroy(list_t* list){
    list_node* cur = NULL;
    cur = list->head;
    while(cur){
      list_node* _next = cur->next;
      free(cur);
      cur = _next;
    } 
}

