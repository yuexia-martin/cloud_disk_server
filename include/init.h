#ifndef INIT_H
#define INIT_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>


#include <ctype.h>
#include <stdint.h>

#include <fcntl.h>



#include "make_log.h" //日志头文件
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include "redis_op.h"
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include "md5.h"



#define LOG_MODULE       "cgi"
#define LOG_PROC         "reg"



//成功
#define SUCCESS 1

//已经存在 禁止重复
#define ERROR_EXISTS 2

#define ERROR 3

#define ERROR_OTHER 10



#define MEMSET_ARRAY_ZERO(arg)    do{memset(arg,0,sizeof(arg));}while(0);


extern void init();

extern int mysql_initialise();

extern int redis_initialise();


//全局变量 MYSQL连接
extern MYSQL * mysql_conn;

//全局变量 redis 连接
redisContext *redis_conn;

extern const char *config_path;

#endif