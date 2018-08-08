#include "init.h"

/*检查服务器是否已经存在文件 
    文件存在 返回文件所属用户id
    文件不存在 0
*/
extern int check_file_exists(char *file_md5);
redisContext * redis_conn;
//计算某个文件是否已经存在(利用md5进行验证)
int main()
{



    char *query_result=malloc(1024);
    char *query=malloc(1024);
    char *token=malloc(513);
    
    char *file_md5=malloc(512);
    char *filename=malloc(512);

    int ret;
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


        if(-1 == uid)
           {

             LOG(LOG_MODULE, LOG_PROC, "权限鉴定失败,请您先登录后再访问\n");

             return_json(ERROR,"msg","权限不足,拒绝访问.请您先登录后再访问!");
             break;
           }


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

            ret=0;

            MEMSET_ARRAY_ZERO(query_result);
            MEMSET_ARRAY_ZERO(query);
            MEMSET_ARRAY_ZERO(token);
            MEMSET_ARRAY_ZERO(file_md5);
            MEMSET_ARRAY_ZERO(filename);


            do{


               
                 //解析json
                cJSON * json_object= cJSON_Parse(receivData);//从 给定的json字符串中得到cjson对象

                if(!json_object){

                    LOG(LOG_MODULE, LOG_PROC, "JSON 解析错误\n");
                    return_json(ERROR,"msg","参数有误!");
                    break;
                }


                //从json对象中获取值
                cJSON   * filename_json=cJSON_GetObjectItem(json_object,"filename");
                cJSON   * md5_json=cJSON_GetObjectItem(json_object,"md5");

                if(!filename_json || !md5_json){

                    LOG(LOG_MODULE, LOG_PROC, "文件名或md5值为空,参数有误\n");

                    return_json(ERROR,"msg","参数有误!");
                    break;
                }

                //把值转换成char
                char * filename_tmp=cJSON_Print(filename_json);
                
                char * md5=cJSON_Print(md5_json);

               

                //查询文件是否存在于服务器
                ret = check_file_exists(md5);

                if(ret > 0){

                      //去掉双引号
                      strncpy(file_md5,md5+1,strlen(md5)-2);

                   
                      //去掉双引号
                      strncpy(filename,filename_tmp+1,strlen(filename_tmp)-2);

                      LOG(LOG_MODULE, LOG_PROC,"file_md5:%s md5:%s\n",file_md5,md5);
                      //这里需要把这个文件也加入到当前上传的这个用户里面 
                      ret = insert_user_file_list_table(uid,file_md5,filename);

                    if(-1 != ret){
                         return_json(SUCCESS,"msg","秒传成功");
                     }else{
                       LOG(LOG_MODULE, LOG_PROC, "插入数据库失败\n");
                     }

                }else{
                    return_json(ERROR,"msg","文件无法秒传");
                }

           }while(0);




			free(receivData);
        }

    }

    return 0;
}




/*检查服务器是否已经存在文件 
    文件存在 返回文件所属用户id
    文件不存在 0
*/
int check_file_exists(char *file_md5)
{
    char query[1024]={0};
    char query_result[1024]={0};

    //组装sql 查询文件md5是否存在
    sprintf(query,"SELECT\
                        uid\
                    FROM\
                        user_file_list\
                    LEFT JOIN file_info ON user_file_list.file_id = file_info.file_id\
                    WHERE\
                    file_info.md5 = %s\
                    limit 1",file_md5);

    //查询服务器是否已经拥有该文件
    process_result_one(mysql_conn,query,query_result);

    LOG(LOG_MODULE, LOG_PROC, "文件uid:%s\n",query_result);

    return atoi(query_result);
}