#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"

void ftp_reply(session_t *sess,int status,const char *text);
static void do_user(session_t *sess);
static void do_pass(session_t *sess);

void handle_child(session_t *sess)
{
        int ret;
        /* 欢迎信息 */
        //当有用户连接成功后,返回220信息
        //writen(sess->ctrl_fd, "220 miniftpd 0.1\r\n", strlen( "220 miniftpd 0.1\r\n"));
        ftp_reply(sess, FTP_GREET, "miniftpd 0.1");
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
                printf("cmdline=[%s]\n",sess->cmdline);
                str_split(sess->cmdline,sess->cmd,sess->arg,' ');
                printf("cmd=[%s]\n",sess->cmd);
                printf("arg=[%s]\n", sess->arg);

                //将命令转换为大写
                str_upper(sess->cmd);
                if(strcmp("USER",sess->cmd) == 0)
                {
                        do_user(sess);
                }else if(strcmp("PASS", sess->cmd) == 0)
                {
                        do_pass(sess);
                }


        }
}

static void do_user(session_t *sess)
{
        struct passwd *pw = getpwnam(sess->arg);
        if(pw == NULL)
        {
                //用户不存在
                ftp_reply(sess,FTP_LOGINERR , "Login incorrect");
                return ;
        }
        sess->uid = pw->pw_uid;

        ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
}
static void do_pass(session_t *sess)
{
        struct passwd *pw = getpwuid(sess->uid);
        if(pw == NULL)
        {
                //用户不存在
                ftp_reply(sess,FTP_LOGINERR , "Login incorrect");
                return ;
        }
        //nobody 进程不能访问影子文件
        struct spwd *sp = getspnam(pw->pw_name);
        if(sp == NULL)
        {
                ftp_reply(sess, FTP_LOGINERR, "Login incorrect");
                return ;
        }

        //对明文密码进行加密
        char * encrypted_pass = crypt(sess->arg,sp->sp_pwdp);
        if(strcmp(encrypted_pass,sp->sp_pwdp) != 0)
        {
                ftp_reply(sess, FTP_LOGINERR, "Login incorrect");
                return ;
        }

        ftp_reply(sess,FTP_LOGINOK,"Login successful");
        /* 更改顺序有讲究,不能倒置，倒置后可能没有权限更改gid */
        if(setegid(pw->pw_gid) < 0)
        {
                ERR_EXIT("do_pass:setegid");
        }
        if(seteuid(pw->pw_uid) < 0)
        {
                ERR_EXIT("do_pass:seteuid");
        }
        if(chdir(pw->pw_dir) < 0)
        {
                ERR_EXIT("do_pass:chdir");
        }


}
void ftp_reply(session_t *sess, int status,const char *text)
{
        char buf[1024] = {0};
        sprintf(buf, "%d %s\r\n",status,text);
        writen(sess->ctrl_fd,buf,strlen(buf));

}
