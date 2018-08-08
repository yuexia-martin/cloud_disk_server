#include "init.h"
redisContext * redis_conn;
int main()
{

    char *query_result=malloc(1024);
    char *query=malloc(1024);
    char *token=malloc(513);
    char *md5_token=malloc(33);
    int ret;

    init();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        long len;

         printf("Content-type: text/html;charset=utf-8\r\n\r\n");


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
            // printf("%s\n",receivData);

            //处理数据
            ret=0;

            MEMSET_ARRAY_ZERO(query_result);
            MEMSET_ARRAY_ZERO(query);
            MEMSET_ARRAY_ZERO(token);
            MEMSET_ARRAY_ZERO(md5_token);

          
           
           do{

                 //解析json
                cJSON * json_object= cJSON_Parse(receivData);//从 给定的json字符串中得到cjson对象

                if(!json_object){

                    LOG(LOG_MODULE, LOG_PROC, "JSON 解析错误\n");
                    break;
                }

                //从json对象中获取值
                cJSON   * username_json=cJSON_GetObjectItem(json_object,"username");
                cJSON   * password_json=cJSON_GetObjectItem(json_object,"password");

                if(!username_json || !password_json){

                    LOG(LOG_MODULE, LOG_PROC, "用户名或密码字段不存在\n");

                    return_json(ERROR,"msg","非法的客户端程序!");
                    break;
                }

                //把值转换成char
                char * username=cJSON_Print(username_json);
                char md5Str[33]={0};
                unsigned char * password=cJSON_Print(password_json);



                //使用这个函数对某个数据块直接运算，输出的MD5Str 32字节+\0
                MD5ToHexString(password ,strlen(password),md5Str);


                //组装sql 查询账户密码是否正确
                sprintf(query,"select id from user where name =%s and password=\'%s\'",username,md5Str);


                //查询用户名是否存在
                process_result_one(mysql_conn,query,query_result);

                if(strlen(query_result)>0){

                    //登录成功 生成token
                    genRandomString(32,token);

                    //把token编码成md5
                    MD5ToHexString(token ,sizeof(token), md5_token);

                    //把编码加上前缀
                    sprintf(token,"token_%s",md5_token);


                    //并把token写入redis(以后客户端访问必须带token才能访问)
                     ret = rop_set_string(redis_conn,token,query_result);

                    

                    if(-1 == ret){
                        LOG(LOG_MODULE, LOG_PROC, "设置redis登录token失败\n");
                        break;
                    }



                    // 设置过期时间(一天过期)
                    ret = rop_set_key_expire(redis_conn, token, 43200);


                    if(-1 == ret){
                        LOG(LOG_MODULE, LOG_PROC, "设置过期时间失败\n");
                    }


                    return_json(SUCCESS,"token",md5_token);
                }else{
                    return_json(ERROR,"msg","登录失败 用户名或密码错误!");
                }




           }while(0);



			free(receivData);
        }

    }

    return 0;
}

