#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<config.h>
#include<list.h>
#include<str.h>
static list_t* head = NULL;

//加载，在程序启动时，就应该将所有的配置信息加载到内存
//conf_name是配置文件的名字
void loda(const char* conf_name){
  if(conf_name==NULL){
     //判空
     return;
  }
  FILE* fp = fopen(conf_name,"r");
  if(fp==NULL){
    return;
  }

  //初始化链表
  head = list_init();

  //规定配置文件一行最多500个有效字符
  char linebuf[501];
  while(!feof(fp)){
    //先将缓冲区刷新 置空
    memset(linebuf,0x00,sizeof(linebuf));
    if(fgets(linebuf,500,fp)==NULL) //从配置文件中读取一行数据
        continue;
    if(linebuf[0]=='\0') //为空行
      continue;
    //开头是空格 换行 回车(\r) 注释(#) tab(\t) '['块名称 都不可以
    if(linebuf[0]==' '||linebuf[0]=='\n'||linebuf[0]=='\r'||
              linebuf[0]=='#'||linebuf[0]=='\t'||linebuf[0]=='[')
      continue;
     //如果最后一个字符是\n(10) \r(13) 空格(32)等都替换成\0
     while(linebuf[strlen(linebuf)-1]==10||
        linebuf[strlen(linebuf)-1]==13||
        linebuf[strlen(linebuf)-1]==32){
          linebuf[strlen(linebuf)-1] = '\0';
     }
        //有可能全有可能当前行全部都是无效字符 全部被清理
        if(linebuf[0]=='\0'){continue;}
        
        //配置文件中的有效信息都是key=value的格式 所以需要找到=的位置
        char* ptmp = NULL; 
        ptmp = strchr(linebuf,'=');
        if(ptmp!=NULL){
          //为了防止用户写的 addr = 192.168.211.129等号左右两两边有空格 我们在这里进行专门的处理
          char name[501] = {};
          char value[501] = {}; 
          strncpy(name,linebuf,(int)(ptmp-linebuf));  
          strcpy(value,ptmp+1); 
          
          //清楚左右空格
          ltrim(name);
          rtrim(name);
          ltrim(value);
          rtrim(value);
          list_insert(head,name,value);
        }
  }
  fclose(fp);
}

//读取字符串类型的配置项
const char *get_string(const char* key){
      list_node* tmp = head->head;
      while(tmp!=NULL){
        if(strcmp( tmp->name,key)==0){
          return tmp->value;
        }
        tmp = tmp->next;
      }
    return NULL;
}

//读取整数类型的配置项(给其一个缺省值 def)
int get_int_default(const char* key,const int def){
      int ret = def;
      list_node* tmp = head->head;
      while(tmp!=NULL){
        if(strcmp( tmp->name,key)==0){
          return atoi(tmp->value);
        }
        tmp = tmp->next;
      }
    return ret;
}
