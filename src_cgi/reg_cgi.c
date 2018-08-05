#include "init.h"



int main()
{

    char *query_result=malloc(1024);
    char *query=malloc(1024);
    int ret;

   init();

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        long len;

        printf("Content-type: text/html\r\n\r\n");

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
          
            //获取参数
            
                //解析json
                cJSON * json_object= cJSON_Parse(receivData);//从 给定的json字符串中得到cjson对象

                //从json对象中获取值
                cJSON   * username_json=cJSON_GetObjectItem(json_object,"username");
                cJSON   * nickname_json=cJSON_GetObjectItem(json_object,"nickname");
                cJSON   * password_json=cJSON_GetObjectItem(json_object,"password");
                cJSON   * phone_json=cJSON_GetObjectItem(json_object,"phone");
                cJSON   * email_json=cJSON_GetObjectItem(json_object,"email");

                //把值转换成char
                char * username=cJSON_Print(username_json);
                char * nickname=cJSON_Print(nickname_json);
                char * phone=cJSON_Print(phone_json);
                char * email=cJSON_Print(email_json);

                char md5Str[33]={0};
                unsigned char * password=cJSON_Print(password_json);

                if(NULL == phone){
                    phone = "NULL";
                }

                if(NULL == email){
                    email = "NULL";
                }

               

                //使用这个函数对某个数据块直接运算，输出的MD5Str 32字节+\0
                 MD5ToHexString(password ,strlen(password),md5Str);


                //组装sql 查询用户名或昵称是否存在
                sprintf(query,"select name from user where name =%s",username);


                //查询用户名是否存在
                process_result_one(mysql_conn,query,query_result);

                //没有查询到用户,可以添加用户
               if(strlen(query_result)==0){

                    MEMSET_ARRAY_ZERO(query);

                    //组装insert语句
                    sprintf(query,"insert into user values(NULL,%s,%s,\"%s\",%s,NOW(),%s);",username,nickname,md5Str,phone,email);

                    //插入数据
                    ret = execute(mysql_conn,query);

                    if(-1==ret){

                         return_json(ERROR_OTHER,"msg","注册错误,请检查注册参数后重试或联系系统管理员!");

                         LOG(LOG_MODULE, LOG_PROC,query);

                    }else{
                        return_json(SUCCESS,"msg","注册成功!");
                    }


                    

               }else{

                    //用户已经存在
                    return_json(ERROR_EXISTS,"msg","用户名或昵称已经被注册!");   
               }

               


			free(receivData);
        }

    }

    return 0;
}

