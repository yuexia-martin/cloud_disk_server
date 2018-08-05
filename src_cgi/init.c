#include "init.h"

// extern redisContext *redis_conn;

//全局变量 MYSQL连接
MYSQL *mysql_conn;

//全局变量 redis 连接
redisContext *redis_conn;

const char *config_path="./conf/cfg.json";

//初始化
void  init(){

    mysql_initialise();
    redis_initialise();
}


//数据库初始化
int mysql_initialise(){

    mysql_conn=msql_conn("root","mhqhdubuntumm","cloud_disk");

    if(NULL==mysql_conn){
        LOG(LOG_MODULE, LOG_PROC, "连接mysql数据库失败\n");
        return -1;
    }
    
    execute(mysql_conn,"set names utf8");

    return 0;

}

//redis 初始化
int redis_initialise(){

    char *ip_str = malloc(128);
    char *port_str = malloc(128);

    get_cfg_value(config_path, "redis","ip",ip_str);
    get_cfg_value(config_path, "redis","port",port_str);



    redis_conn = rop_connectdb_nopwd(ip_str,port_str);

    if(NULL == redis_conn)
    {
        LOG(LOG_MODULE,LOG_PROC, "连接redis缓存服务器失败\n");
        return -1;
    }

    return 1;

}

