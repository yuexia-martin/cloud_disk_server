#include "init.h"


extern void get_file_list(int uid);

//获取文件列表
int main()
{
     int uid=-1;

    init();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        long len;
       

        printf("Content-type: text/html\r\n\r\n");


         //鉴权
        uid = check_user();

        if( contentLength == NULL )
        {
            len = 0;
        }
        else
        {
            len = atol(contentLength); //字符串转整型
        }

        if (len <= 0)
        {
            LOG(LOG_MODULE, LOG_PROC, "len = 0, No data from standard input\n");
        }
        else 
        {
			char *receivData = (char*)malloc(len);
			if(!receivData)
			{
				LOG(LOG_MODULE, LOG_PROC, "Not enough memory to malloc\n");
				exit(1);
			}
			size_t readSize = fread(receivData,1,len,stdin);
			if(readSize != len)
			{
				LOG(LOG_MODULE, LOG_PROC, "Not enough bytes receive on standard input\n");
				exit(1);
			}
			//处理数据

           //循环获取这个用户的文件列表
            get_file_list(uid);


            //返回json




			free(receivData);
        }

    }

    return 0;
}


void get_file_list(int uid){

    char query[1024]={0};
    char query_result[1024]={0};

    //组装sql 查询文件md5是否存在
    sprintf(query,"\
        SELECT\
            id,\
            uid,\
            user_file_list.file_id,\
            filename,\
            user_file_list.createtime,\
            shared_status,\
            pv,\
            md5,\
            size,\
            type,\
            count\
        FROM\
            user_file_list\
        LEFT JOIN file_info ON file_info.file_id = user_file_list.file_id\
        WHERE\
            uid = %d\
        ",uid);


    // 查询
    mysql_query(mysql_conn, query);

    //生成结果集
    MYSQL_RES *res_set = mysql_store_result(mysql_conn); 

 
    //处理结果集
    process_result_test(mysql_conn,res_set);


}
