#设置当前路径 shell是函数，可以用该格式调用Linux下的命令
#该命令执行完的结果为：/home/jxy/project/frameWork
#export在shell中是配置环境变量的命令 在shell脚本中其也相当于把我们定
#义的当前路径定义为全局的，其他makefile中也可以使用
export BUILD_ROOT=$(shell pwd)
#头文件路径
export INCLUDE_PATH=$(BUILD_ROOT)/include
#编译路径（每一个需要编译的子目录的makefile的路径）
export BUILD_PATH = $(BUILD_ROOT)/database\
					$(BUILD_ROOT)/src\
					$(BUILD_ROOT)/database\
					$(BUILD_ROOT)/net\
					$(BUILD_ROOT)/threadpool

# @的意思是执行时不显示这些命令
# make -C 指定要编译那个目录中的makefile
all:
	 @for dir in $(BUILD_PATH);\
		do\
				make -C $$dir;\
	  done 

clean:
			rm -rf $(BUILD_ROOT)/link_obj
			rm -rf $(BUILD_ROOT)/dep
			rm -rf main
