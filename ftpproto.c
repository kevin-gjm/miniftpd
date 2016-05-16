#include "ftpproto.h"
#include "common.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"

//标准格式:520 XXXXXX.\r\n
void ftp_reply(session_t *sess,int status,const char *text);
//多行模式:211-XXXX:\r\n
void ftp_lreply(session_t *sess, int status,const char *text);
//文本形式:XXXXX\r\n
void ftp_treply(session_t *sess,const char* text);
//使用命令映射机制
static void do_user(session_t *sess);
static void do_pass(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_quit(session_t *sess);
static void do_port(session_t *sess);
static void do_pasv(session_t *sess);
static void do_type(session_t *sess);
static void do_stru(session_t *sess);
static void do_mode(session_t *sess);
static void do_retr(session_t *sess);
static void do_stor(session_t *sess);
static void do_appe(session_t *sess);
static void do_list(session_t *sess);
static void do_nlst(session_t *sess);
static void do_rest(session_t *sess);
static void do_abor(session_t *sess);
static void do_pwd(session_t *sess);
static void do_mkd(session_t *sess);
static void do_rmd(session_t *sess);
static void do_dele(session_t *sess);
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);
static void do_site(session_t *sess);
/* static void do_site_help(session_t* sess,char* arg); */
/* static void do_site_umask(session_t* sess,char* arg); */
/* static void do_site_chmod(session_t* sess,char* arg); */
static void do_syst(session_t *sess);
static void do_feat(session_t *sess);
static void do_size(session_t *sess);
static void do_stat(session_t *sess);
static void do_noop(session_t *sess);
static void do_help(session_t *sess);


static struct ftp_cmd_t
{
        const char* cmd;
        void (*cmd_handler)(session_t *sess);
}
        ctrl_cmds[] =
        {
                //不在表内则表示未识别的命令
                {"USER",    do_user },
                {"PASS",    do_pass },
                {"CWD",     do_cwd },
                {"XCWD",    do_cwd },
                {"CDUP", do_cdup },
                {"XCUP", do_cdup },
                {"QUIT",    do_quit },
                {"ACCT", NULL }, //能够认识，但是没能实现
                {"SMNT", NULL },
                {"REIN",NULL },
                /* 传输参数命令 */
                {"PORT", do_port },
                {"PASV", do_pasv },
                {"TYPE", do_type },
                {"STRU", do_stru },
                {"MODE", do_mode },
                /* 服务命令 */
                {"RETR", do_retr },
                {"STOR", do_stor },
                {"APPE",do_appe },
                {"LIST", do_list },
                {"NLST", do_nlst },
                {"REST",do_rest },
                {"ABOR", do_abor },
                {"\377\364\377\362ABOR", do_abor},
                {"PWD",     do_pwd },
                {"XPWD",    do_pwd },
                {"MKD",     do_mkd },
                {"XMKD", do_mkd },
                {"RMD", do_rmd },
                {"XRMD", do_rmd },
                {"DELE", do_dele },
                {"RNFR",    do_rnfr },
                {"RNTO",    do_rnto },
                {"SITE", do_site },
                {"SYST",    do_syst },
                {"FEAT",    do_feat },
                {"SIZE", do_size },
                {"STAT", do_stat },
                {"NOOP", do_noop },
                {"HELP", do_help },
                {"STOU", NULL },
                {"ALLO", NULL }
                /* 不需要空表项{NULL,NULL},以另一种结束方式表示  */
        };

void ftp_lreply(session_t *sess, int status,const char *text)
{
        char buf[1024] = {0};
        sprintf(buf, "%d-%s\r\n",status,text);
        writen(sess->ctrl_fd,buf,strlen(buf));

}
void ftp_reply(session_t *sess, int status,const char *text)
{
        char buf[1024] = {0};
        sprintf(buf, "%d %s\r\n",status,text);
        writen(sess->ctrl_fd,buf,strlen(buf));

}
void ftp_treply(session_t *sess,const char* text)
{
        char buf[1024] = {0};
        sprintf(buf, "%s\r\n",text);
        writen(sess->ctrl_fd,buf,strlen(buf));

}
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
/*
//使用命令映射机制消除if...else...这种结构
if(strcmp("USER",sess->cmd) == 0)
{
do_user(sess);
}else if(strcmp("PASS", sess->cmd) == 0)
{
do_pass(sess);
}
*/
                int i = 0;
                int size = sizeof(ctrl_cmds) / sizeof(ctrl_cmds[0]);
                for(i=0;i<size;i++)
                {
                        if(strcmp(ctrl_cmds[i].cmd,sess->cmd) ==0)
                        {
                                if(ctrl_cmds[i].cmd_handler != NULL)
                                {
                                        ctrl_cmds[i].cmd_handler(sess);
                                }else
                                {
                                        ftp_reply(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
                                }
                                break;
                        }
                }
                if(i == size)
                {
                        ftp_reply(sess,FTP_BADCMD,"Unknown command.");

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
static void do_cwd(session_t *sess){}
static void do_cdup(session_t *sess){}
static void do_quit(session_t *sess){}
static void do_port(session_t *sess){}
static void do_pasv(session_t *sess){}
static void do_type(session_t *sess){}
static void do_stru(session_t *sess){}
static void do_mode(session_t *sess){}
static void do_retr(session_t *sess){}
static void do_stor(session_t *sess){}
static void do_appe(session_t *sess){}
static void do_list(session_t *sess){}
static void do_nlst(session_t *sess){}
static void do_rest(session_t *sess){}
static void do_abor(session_t *sess){}
static void do_pwd(session_t *sess)
{
        char text[2048] = {0};
        char dir[1024+1] = {0};
        getcwd(dir, 1024);
        sprintf(text, "\"%s\"",dir);
        ftp_reply(sess,FTP_PWDOK,text);
}
static void do_mkd(session_t *sess){}
static void do_rmd(session_t *sess){}
static void do_dele(session_t *sess){}
static void do_rnfr(session_t *sess){}
static void do_rnto(session_t *sess){}
static void do_site(session_t *sess){}
/* static void do_site_help(session_t* sess,char* arg){} */
/* static void do_site_umask(session_t* sess,char* arg){} */
/* static void do_site_chmod(session_t* sess,char* arg){} */
static void do_syst(session_t *sess)
{
        ftp_reply(sess,FTP_SYSTOK,"UNIX Type: L8.");
}
static void do_feat(session_t *sess)
{
        //211-Features:
        //EPRT
        //EPSV
        //MDTM
        //PASV
        //REST STREAM
        //SIZE
        //TVFS
        //UTF8
        //211 END
        ftp_lreply(sess,FTP_FEAT, "Features:");
        ftp_treply(sess, " ERPT");
        ftp_treply(sess, " EPSV");
        ftp_treply(sess, " MDTM");
        ftp_treply(sess, " PASV");
        ftp_treply(sess, " REST STREAM");
        ftp_treply(sess, " SIZE");
        ftp_treply(sess, " TVFS");
        ftp_treply(sess, " UTF8");
        ftp_reply(sess, FTP_FEAT, "End");


}
static void do_size(session_t *sess){}
static void do_stat(session_t *sess){}
static void do_noop(session_t *sess){}
static void do_help(session_t *sess){}
