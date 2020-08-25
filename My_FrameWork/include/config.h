#ifndef __CONFIG_H__
#define __CONFIG_H__

//加载，在程序启动时，就应该将所有的配置信息加载到内存
//conf_name是配置文件的名字
void loda(const char* conf_name);

//读取字符串类型的配置项
const char *get_string(const char* key);

//读取整数类型的配置项(给其一个缺省值 def)
int get_int_default(const char* key,const int def);

#endif //__CONFIG_H__
