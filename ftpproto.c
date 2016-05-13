#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"

void handle_child(session_t *sess)
{
        int ret;
        /* 欢迎信息 */
        //当有用户连接成功后,返回220信息
        writen(sess->ctrl_fd, "220 miniftpd 0.1\r\n", strlen( "220 miniftpd 0.1\r\n"));
        while(1)
        {
                memset(sess->cmdline,0,sizeof(sess->cmdline));
                memset(sess->cmd,0,sizeof(sess->cmd));
                memset(sess->arg,0,sizeof(sess->arg));
                ret = readline(sess->ctrl_fd, sess->cmdline,MAX_COMMAND_LINE);
                //ftp协议规定每一条指令后面都跟着\r\n
                //解析命令和参数
                //处理FTP命令与参数

                if(ret == -1)//失败
                { ERR_EXIT("handle_child:readline"); }
                else if(ret ==0)//客户端断开连接
                {
                        exit(EXIT_SUCCESS);
                }
                //del the \r\n
                //printf("before:%s:after\n",sess->cmdline);
                str_trim_crlf(sess->cmdline);
                //printf("cmdline=[%s]\n",sess->cmdline);
                str_split(sess->cmdline,sess->cmd,sess->arg,' ');
                //printf("cmd=[%s]\n",sess->cmd);
                //printf("arg=[%s]\n", sess->arg);

                //将命令转换为大写
                str_upper(sess->cmd);


        }
}
