#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_log.h" //日志头文件
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>

#define LOG_MODULE       "cgi"
#define LOG_PROC         "dealfile"
#define MEMSET_ARRAY_ZERO(arg)    do{memset(arg,0,sizeof(arg));}while(0);

int main()
{
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




			free(receivData);
        }

    }

    return 0;
}

