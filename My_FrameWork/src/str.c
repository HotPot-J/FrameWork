#include<stdio.h>
#include<string.h>
#include<str.h>
//自定义字符串处理函数

//去除字符串左空格
void ltrim(char* str){
  if(str==NULL||(*str)!=' '){return;}
  char* prev = str,*rear=str;
  while(*rear){
    if(*rear!=' '){
      break;
    }
    ++rear;
  }
  while(*rear){
    *(prev++) = *(rear++);
  }
  *prev = '\0';
}

//去除字符串右空格
void rtrim(char* str){
    if(str==NULL){return;}
    int len = strlen(str);
    if(len==0){return ;}
    for(int i = len-1;i>=0;++i){
      if(str[i]==' '){
        str[i] = '\0';
      }
      else{
        break;
      }
    }   
}
