/**
 * @file util_cgi.c
 * @brief  cgi后台通用接口
 * @author Mike
 * @version 1.0
 * @date 2017年1月13日23:38:31
 */
 
#include "init.h"

/**
 * @brief  去掉一个字符串两边的空白字符
 *
 * @param inbuf确保inbuf可修改
 *
 * @returns   
 *      0 成功
 *      -1 失败
 */
int trim_space(char *inbuf)
{
    int i = 0;
    int j = strlen(inbuf) - 1;

    char *str = inbuf;

    int count = 0;

    if (str == NULL ) 
	{
        //LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "inbuf   == NULL\n");
        return -1;
    }


    while (isspace(str[i]) && str[i] != '\0') 
	{
        i++;
    }

    while (isspace(str[j]) && j > i) 
	{
        j--;
    }

    count = j - i + 1;

    strncpy(inbuf, str + i, count);

    inbuf[count] = '\0';

    return 0;
}

/**
 * @brief  在字符串full_data中查找字符串substr第一次出现的位置
 *
 * @param full_data 	源字符串首地址
 * @param full_data_len 源字符串长度
 * @param substr        匹配字符串首地址
 *
 * @returns   
 *      成功: 匹配字符串首地址
 *      失败：NULL
 */
char* memstr(char* full_data, int full_data_len, char* substr) 
{ 
	//异常处理
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) 
	{ 
        return NULL; 
    } 

    if (*substr == '\0')
	{ 
        return NULL; 
    } 

	//匹配子串的长度
    int sublen = strlen(substr); 

    int i; 
    char* cur = full_data; 
    int last_possible = full_data_len - sublen + 1; //减去匹配子串后的长度
    for (i = 0; i < last_possible; i++) 
	{ 
        if (*cur == *substr) 
		{ 
            if (memcmp(cur, substr, sublen) == 0) 
			{ 
                //found  
                return cur; 
            } 
        }
		
        cur++; 
    } 

    return NULL; 
} 

/**
 * @brief  解析url query 类似 abc=123&bbb=456 字符串
 *          传入一个key,得到相应的value
 * @returns
 *          0 成功, -1 失败
 */
int query_parse_key_value(const char *query, const char *key, char *value, int *value_len_p)
{
    char *temp = NULL;
    char *end = NULL;
    int value_len =0;


    //找到是否有key
    temp = strstr(query, key);
    if (temp == NULL)
    {
        //LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "Can not find key %s in query\n", key);

        return -1;
    }

    temp += strlen(key);//=
    temp++;//value


    //get value
    end = temp;

    while ('\0' != *end && '#' != *end && '&' != *end )
    {
        end++;
    }

    value_len = end-temp;

    strncpy(value, temp, value_len);
    value[value_len] ='\0';

    if (value_len_p != NULL)
    {
        *value_len_p = value_len;
    }

    return 0;
}

//通过文件名file_name， 得到文件后缀字符串, 保存在suffix 如果非法文件后缀,返回"null"
int get_file_suffix(const char *file_name, char *suffix)
{
    const char *p = file_name;
    int len = 0;
    const char *q=NULL;
    const char *k= NULL;

    if (p == NULL)
    {
        return -1;
    }

    q = p;


    while (*q != '\0')
    {
        q++;
    }

    k = q;
    while (*k != '.' && k != p)
    {
        k--;
    }

    if (*k == '.')
    {
        k++;
        len = q - k;

        if (len != 0)
        {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        }
        else
        {
            strncpy(suffix, "null", 5);
        }
    }
    else
    {
        strncpy(suffix, "null", 5);
    }

    return 0;
}

//字符串strSrc中的字串strFind，替换为strReplace
void str_replace(char* strSrc, char* strFind, char* strReplace)
{
    while (*strSrc != '\0')
    {
        if (*strSrc == *strFind)
        {
            if (strncmp(strSrc, strFind, strlen(strFind)) == 0)
            {
                int i = 0;
                char *q = NULL;
                char *p = NULL;
                char *repl = NULL;
                int lastLen = 0;

                i = strlen(strFind);
                q = strSrc+i;
                p = q;//p、q均指向剩余字符串的首地址
                repl = strReplace;

                while (*q++ != '\0')
                    lastLen++;
                char* temp = (char *)malloc(lastLen+1); //临时开辟一段内存保存剩下的字符串,防止内存覆盖
                int k = 0;
                for (k = 0; k < lastLen; k++)
                {
                    *(temp+k) = *(p+k);
                }
                *(temp+lastLen) = '\0';
                while (*repl != '\0')
                {
                    *strSrc++ = *repl++;
                }
                p = strSrc;
                char* pTemp = temp;//回收动态开辟内存
                while (*pTemp != '\0')
                {
                    *p++ = *pTemp++;
                }
                free(temp);
                *p = '\0';
            }
            else
                strSrc++;
        }
        else
            strSrc++;
    }
}

//返回前端情况，NULL代表失败, 返回的指针不为空，则需要free
char * return_status(char *status_num)
{
    char *out = NULL;
    cJSON *root = cJSON_CreateObject();  //创建json项目
    cJSON_AddStringToObject(root, "code", status_num);// {"code":"000"}
    out = cJSON_Print(root);//cJSON to string(char *)

    cJSON_Delete(root);


    return out;
}

//验证登陆token，成功返回0，失败-1
int verify_token(char *token,char *userId)
{
    int ret = 0;
    redisContext * redis_conn = NULL;
    char key[128] = "token_";
	strcat(key,token);

    //redis 服务器ip、端口
    char redis_ip[30] = {0};
    char redis_port[10] = {0};

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "redis connected error\n");
        ret = -1;
        goto END;
    }

    //获取user对应的value
    ret = rop_hash_get(redis_conn,key,"userId", userId);
    if(ret == 0)
    {
		//	
    }

END:
    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }


    return ret;
}


void return_json(int code,char * key,char *value)
{

    cJSON * result =  cJSON_CreateObject();

    cJSON_AddItemToObject(result, "status", cJSON_CreateNumber(code));
    cJSON_AddItemToObject(result, key, cJSON_CreateString(value));

    printf("%s\n", cJSON_Print(result));

}


//生成随机字符串
int genRandomString(int length,char* ouput)
{
    int flag, i;
    srand((unsigned)time(NULL));
    for (i = 0; i < length - 1; i++)
    {
        flag = rand() % 3;
        switch (flag)
        {
        case 0:
            ouput[i] = 'A' + rand() % 26;
            break;
        case 1:
            ouput[i] = 'a' + rand() % 26;
            break;
        case 2:
            ouput[i] = '0' + rand() % 10;
            break;
        default:
            ouput[i] = 'x';
            break;
        }
    }
    return 0;
}


//验证用户是否是合法用户
int check_user(){

     char *token = getenv("HTTP_TOKEN");
     int  length = 0;
     char token_md5[512]={0};
     char result[512]={0};


     sprintf(token_md5,"token_%s",token);



     //访问redis 鉴权
     rop_get_string(redis_conn,token_md5,result);

     //如果根本没有带token访问
     if(NULL == token)
     {
        return -1;
     }

     length=strlen(token);

     if(length <= 0){
        return -1;
     }

     return atoi(result);
}


int Compute_file_md5(char *file_path, char *md5_str)
{
    LOG(LOG_MODULE, LOG_PROC, "进入Compute_file_md5函数\n");
    int i;
    int fd;
    int ret;
    unsigned char data[READ_DATA_SIZE];
    unsigned char md5_value[MD5_SIZE];
    MD5_CTX md5;
 
    fd = open(file_path, O_RDONLY);
    if (-1 == fd)
    {
        perror("open");
        return -1;
    }
 
    // init md5
    MD5Init(&md5);

LOG(LOG_MODULE, LOG_PROC, " 001 info md5:%s\n",md5);
 
    while (1)
    {
        ret = read(fd, data, READ_DATA_SIZE);
        if (-1 == ret)
        {
            perror("read");
            return -1;
        }
 
        MD5Update(&md5, data, ret);
 
        if (0 == ret || ret < READ_DATA_SIZE)
        {
            break;
        }
    }
 
    close(fd);
 LOG(LOG_MODULE, LOG_PROC, "002  info md5:%s\n",md5);
    MD5Final(&md5, md5_value);
 LOG(LOG_MODULE, LOG_PROC, "003 info md5:%s\n",md5);
    for(i = 0; i < MD5_SIZE; i++)
    {
        snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
    }
     LOG(LOG_MODULE, LOG_PROC, " info md5:%s\n",md5);

    md5_str[MD5_STR_LEN] = '\0'; // add end
 
    return 0;
}




void get_extension(const char *file_name,char *extension)
{
    int i=0,length;
    length=strlen(file_name);
        while(file_name[i])
    {
        if(file_name[i]=='.')
        break;
        i++;
    }
    if(i<length)
    strcpy(extension,file_name+i+1);
    else
    strcpy(extension,"\0");
}

//把文件信息插入到user_file_list表中
int insert_user_file_list_table(int uid,char file_md5[33],char *filename)
{
    char query[1024]={0};




    //组装sql 查询账户密码是否正确
    sprintf(query,"INSERT INTO user_file_list VALUES(NULL,%d,(SELECT file_info.file_id FROM file_info where md5='%s' limit 1),'%s',NOW(),0,0)",uid,file_md5,filename);

     LOG(LOG_MODULE, LOG_PROC, "info file_md5%s\n",file_md5);


    //插入数据
    return execute(mysql_conn,query);
}




