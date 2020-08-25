#子makefile模板

#一、先定义一些常用的变量

CC=gcc -g
#源文件（.c）   wildcard的作用是过滤筛选 比如当前目录下右add.c sub.c a.h那么
# wildcard* .c 筛选的结果就是：add.c sub.c
SRCS=$(wildcard *.c)

#将点c文件换成.o文件 在放进OBJS变量中
#add.c sub.c => add.o sub.o
OBJS=$(SRCS:.c=.o)

# gcc -MM 会自动生成依赖关系 生成的这种依赖文件我们通常定义为.d文件
#将SRCS中的.c文件文件换成.d 再放进DEPS中
#add.c sub.c => add.d sub.d
DEPS=$(SRCS:.c=.d)

#定义可执行程序存放的变量 (生成可执行程序的位置)
#addprefix函数 加前缀函数，把前缀<prefix>加到<names>中的每个单词前。
#这里使用的含义是，将当前文件目录作为前缀添加到$(BIN)这个参数中，而这个$(BIN)参数是每个子目录的路径 
#假设 $(BIN) 是main 那么该句的效果是：生成/home/jxy/project/frameWork/main
BIN :=$(addprefix $(BUILD_ROOT)/,$(BIN))

#SHARE特指链接生成动态链接库对象时的编译选项
#SHARE=--share
#-----------------------------暂时省略SHARE这个自定义变量

#CFLAG，表示在编译时加入的选项参数
#-wall(编译后显示警告信息) -g(调试信息) -fPIC(编译动态链接库式加入到选项)
#-I./inc -I选项指定从哪个目录寻找头文件，在这里指定后，在源文件中可使用<>包含头文件这里的./inc即为指定头文件目录
#CFLAG=
#----------------------------暂时省略CFLAG这个变量

#LFLAG表示链接生成可执行程序时所要链接的所有库的选项参数
#-L./lib :-L指示动态/静态链接库所在目录，这里./lib即所在目录
#-l : 指示动态/静态库名字（注意：这里的库名字不包括lib前缀和.a或.so后缀）
#$(SHARE):SHARE变量对应的动态库连接选项--share
LFLAG=-L/usr/lib64/mysql -lmysqlclient -lpthread

#生成存放.o文件夹（目录）的路径
LINK_OBJ_DIR=$(BUILD_ROOT)/link_obj
#生成存放依赖文件.d文件夹(目录)的路径
DEP_DIR=$(BUILD_ROOT)/dep

#利用shell命令创建LINK_OBJ_DIR 和DEP_DIR文件
#临时目录，存放.o和.d文件的目录 -p即递归创建目录，若上级目录不存在则自动创建
$(shell mkdir -p $(LINK_OBJ_DIR))
$(shell mkdir -p $(DEP_DIR))
#main.o => /home/jxy/project/frameWork/link_obj/main.o
OBJS:=$(addprefix $(LINK_OBJ_DIR)/,$(OBJS))
DEPS:=$(addprefix $(DEP_DIR)/,$(DEPS))

#将上面创建的.o文件夹中的.o文件筛选出来
LINK_OBJ=$(wildcard $(LINK_OBJ_DIR)/*.o)
LINK_OBJ += $(OBJS)

all: $(DEPS) $(OBJS) $(BIN)
#BIN可执行文件依赖于.o文件 并连接动态库 $(LFLAG)
$(BIN) : $(LINK_OBJ)
	$(CC) -o $@ $^ $(LFLAG)
# -I 选项是指定include头文件的路径
# .o文件依赖于.c文件
$(LINK_OBJ_DIR)/%.o : %.c
	$(CC) -c $^ -o $@ -I$(INCLUDE_PATH) 
#将.o文件的路径输入到准备存放.d文件中 再使用 gcc -MM 选项添加依赖关系追加
#到准备存放.d的文件中
$(DEP_DIR)/%.d : %.c
	echo -n $(LINK_OBJ_DIR)/ > $@  
	$(CC) -I$(INCLUDE_PATH) -MM $^ >> $@
