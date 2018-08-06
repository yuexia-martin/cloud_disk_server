
#要编译的工具代码
TOOLS=./common/make_log.c \
	  ./common/md5.c \
	  ./common/cJSON.c \
	  ./common/des.c \
	  ./common/redis_op.c \
	  ./common/base64.c \
	  ./common/deal_mysql.c\
	  ./common/util_cgi.c\
	  ./common/cfg.c


#引用的头文件路径
CPPLFAGS=-I./include					\
		 -I/usr/include/fastdfs			\
		 -I/usr/include/fastcommon		\
		 -I/usr/local/include/hiredis/  \
		 -I/usr/include/mysql/

SRC=./src_cgi/init.c\
    
    


#要编译的库
LIBS=-lfcgi -lfdfsclient -lfastcommon -lhiredis -lm -lmysqlclient


all:reg login upload md5 myfiles


reg:
	gcc -I./include ${TOOLS}  ${CPPLFAGS} ${SRC} ./src_cgi/reg_cgi.c -o ./bin_cgi/reg ${LIBS}

login:
	gcc -I./include ${TOOLS}  ${CPPLFAGS} ${SRC} ./src_cgi/login_cgi.c -o ./bin_cgi/login ${LIBS}

upload:
	gcc -I./include ${TOOLS}  ${CPPLFAGS} ${SRC} ./src_cgi/upload_cgi.c -o ./bin_cgi/upload ${LIBS}
	
md5:
	gcc -I./include ${TOOLS}  ${CPPLFAGS} ${SRC} ./src_cgi/md5_cgi.c -o ./bin_cgi/md5 ${LIBS}

myfiles:
	gcc -I./include ${TOOLS}  ${CPPLFAGS} ${SRC} ./src_cgi/myfiles_cgi.c -o ./bin_cgi/myfiles ${LIBS}

