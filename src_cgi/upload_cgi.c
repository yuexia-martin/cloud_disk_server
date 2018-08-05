#include "init.h"

extern char **environ;


#define SIZE 1024000000

// extern char* memstr(char* full_data, int full_data_len, char* substr);



redisContext * redis_conn;

int main ()
{

    char **initialEnv = environ;
    int count = 0;
    
    //管道
    int child_fd[2];
    int pid;
    int status=0;
    int uid=-1;

    //文件长度
    int file_size=0;

    char * buf=NULL;
    char filename[1024]={0};
    char file_id[1024]={0};

    //文件md5
    char *file_md5=malloc(512);
    //文件后缀名
    char file_suffix[513]={0};

    //查询语句
    char *query=malloc(1024);

    //接收返回值
    char *query_result=malloc(1024);


    buf=malloc(SIZE);

    if(-1 == pipe(child_fd)){

        LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "创建管道失败\n");
        exit(0);

    }

    init();


   

    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;
        char tmp[10240]={0};

        printf("Content-type: text/html\r\n\r\n");
    


    do{
         
            //鉴权
           uid = check_user();

           if(-1 == uid)
           {
             LOG(LOG_MODULE, LOG_PROC, "权限鉴定失败,请您先登录后再访问\n");
             return_json(ERROR,"msg","权限不足,拒绝访问.请您先登录后再访问!");
             break;
           }

          
            if(NULL==buf){
                printf("初始化失败\n");
                break;
            }

            LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "接收到新连接\n");

            char path[1024]={"/tmp/"};


            int ret=0;
            int fd=0;

            file_size=0;

            char explore_content[1024]={0};
            char *explore_end=NULL;

            
            char *filename_begin=0;
            char *filename_end=0;

            char *file_content_begin=NULL;
            char *file_content_end=NULL;

            char *content=NULL;


            memset(buf,0,SIZE);
            memset(filename,0,sizeof(filename));
            memset(explore_content,0,sizeof(explore_content));
            memset(file_id,0,sizeof(file_id));
            memset(file_md5,0,sizeof(file_md5));
            memset(file_suffix,0,sizeof(file_suffix));
            memset(query_result,0,sizeof(query_result));
            memset(query,0,sizeof(query));
             
            // //获取长度
            if (contentLength != NULL) {
                len = strtol(contentLength, NULL, 10);
            }
            else {
                len = 0;
            }

            if (len <= 0) {
            // printf("No data from standard input.<p>\n");
            }
            else {
                int i, ch;

          
                for (i = 0; i < len; i++) {
                        if ((ch = getchar()) < 0) {
                            LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "upload 读取字节流失败\n");
                            break;
                         }

                        buf[i]=ch;
                }
               
            }

            // 找到分隔符
            explore_end=strstr(buf,"\r\n");
            strncpy(explore_content,buf,explore_end - buf);


            //获取文件名
            filename_begin=strstr(buf,"filename=\"")+ strlen("filename=\"");

            filename_end=strstr(filename_begin,"\"");

            strncpy(filename,filename_begin,filename_end-filename_begin);

            LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "get filename:%s\n",filename);


            //获取文件内容
            file_content_begin=strstr(buf,"filename=\"");
            
            file_content_begin=strstr(file_content_begin,"\r\n\r\n")+4;

            file_content_end=memstr(file_content_begin,len,explore_content)-2;

           


            //取得文件的保存路径
            strcat(path,filename);


            ret=access(path,F_OK);

            if(ret!=0){
               
                fd=open(path,O_WRONLY|O_CREAT,0666);

            }else{
                
                fd=open(path,O_WRONLY,0666);

            }


        if(-1==fd){

                LOG(LOG_MODULE, LOG_PROC, "无法打开文件或创建文件:%d %s\n",fd,path);
               
        }else{

            ret=write(fd,file_content_begin,file_content_end - file_content_begin);


            if(-1 == ret){
                
                LOG(LOG_MODULE, LOG_PROC, "写入文件错误\n",fd,path);

            }else{

               
                

                //写文件信息到数据库

                  pid=fork();

                    //开启进程 上传到dfs 并获得file_id 
                    if(pid==0){

                        close(STDOUT_FILENO);

                        //复制文件描述符 为标准输入
                        dup2(child_fd[1],STDOUT_FILENO);

                        execlp("fdfs_upload_file","../mycode/update","/etc/fdfs/client.conf.sample",path,NULL);

                        //子进程退出
                        exit(0);

                    }else{

                        ret=read(child_fd[0],&file_id,sizeof(file_id));

                        if(-1 == ret){
                            LOG(LOG_MODULE, LOG_PROC,"子进程返回错误 ret:%d  file_id:%s",ret,file_id);
                        }else{
                            LOG(LOG_MODULE, LOG_PROC,"file_id:%s",file_id);
                        }

                      
                        wait(&status);
                    }

                    LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "okok\n");

                    //获取文件md5
                    Compute_file_md5((char *)&path,file_md5);

                    // file_md5[0]='x';
                    // file_md5[1]='y';
                     LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "001:%c\n",*file_md5);
                    



                    LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "file MD5:%s\npath:%s\n",file_md5,path);

                    //获取文件大小

                    file_size = file_content_end - file_content_begin;

                    get_extension(filename,(char *)&file_suffix);

                    LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "文件后缀名:%s\n",file_suffix);

                    if(strlen(file_id)>0 && strlen(file_md5)>0 && file_size > 0){
                         
                         //组装sql 查询账户密码是否正确
                        sprintf(query,"insert into file_info values('%s','%s',%d,'%s',1,NOW())",file_id,file_md5,file_size,file_suffix);

                         
                         //插入数据
                        ret = execute(mysql_conn,query);

                        if(-1 != ret){

                            //写入用户文件表 user_file_list
                            insert_user_file_list_table(uid,file_md5,filename);

                             return_json(SUCCESS,"msg","文件上传成功!");
                        }else{
                             LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "文件上传 数据库语句执行失败:%s\n",query);
                             return_json(ERROR,"msg","文件上传失败!");
                        }
                    }else{
                        
                         LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "文件参数错误\n");
                    }

                
            }
        }
       

        close(fd);




        }while(0);

       

       

    } /* while */

    free(buf);

    return 0;
}

