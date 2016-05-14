#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"


void begin_session(session_t *sess)
{
        int sockfds[2];

        if(socketpair(PF_UNIX ,SOCK_STREAM  ,0  ,sockfds) < 0)
                ERR_EXIT("bigin_session:socketpair");

        pid_t pid ;
        pid = fork();
        if(pid < 0)
                ERR_EXIT("begin_session:fork");

        if(pid ==0 )
        {
                // 服务进程:ftp 协议相关的细节
                close(sockfds[0]);
                sess->child_fd = sockfds[1];
                handle_child(sess);
        }else{

                //nobody进程

                /* 将父进程改为nobody进程 */

                struct passwd *pw =  getpwnam("nobody");
                if(pw == NULL)
                        return ;
                /* 更改顺序有讲究,不能倒置，倒置后可能没有权限更改gid */
                if(setegid(pw->pw_gid) < 0)
                {
                        ERR_EXIT("begin_session:setegid");
                }
                if(seteuid(pw->pw_uid) < 0)
                {
                        ERR_EXIT("begin_session:seteuid");
                }

                close(sockfds[1]);
                sess->parent_fd = sockfds[0];
                handle_parent(sess);
        }
}
