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
       

         printf("Content-type: text/html;charset=utf-8\r\n\r\n");


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

             //解析json
            cJSON * json_object= cJSON_Parse(receivData);//从 给定的json字符串中得到cjson对象

            //从json对象中获取值
            cJSON   * action_json=cJSON_GetObjectItem(json_object,"action");

            char * action=cJSON_Print(action_json);


            LOG(LOG_MODULE, LOG_PROC, "LOG:%s\n",action);

            //如果是获取文件列表
            if(strcmp(action,"\"get_list\"")==0){

                //循环获取这个用户的文件列表
                 get_file_list(uid);
            }

          


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


    MYSQL_ROW row;
    uint i;

    
    cJSON * root =  cJSON_CreateArray();
    
    //获取列属性
    MYSQL_FIELD *fields;

    //从结果集中抓取列属性
    fields = mysql_fetch_fields(res_set);


    // mysql_fetch_row从使用mysql_store_result得到的结果结构中提取一行，并把它放到一个行结构中。当数据用完或发生错误时返回NULL.
    while ((row = mysql_fetch_row(res_set)) != NULL)
    {
        //每次都重新开辟空间,这样添加到的才会不是同一个地址，才能不添加到一起去
        cJSON * result =  cJSON_CreateObject();

        //mysql_num_fields获取结果中列的个数
        for(i = 0; i < mysql_num_fields(res_set); i++)
        {
            //把键值对添加到result对象中
            cJSON_AddItemToObject(result,fields[i].name, cJSON_CreateString(row[i]));
        }

        //把对象加入到根节点
        cJSON_AddItemToArray(root,result);
    }

     printf("%s\n", cJSON_Print(root));
}
