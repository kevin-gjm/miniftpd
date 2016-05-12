#include "common.h"
#include "sysutil.h"
#include "session.h"
int main(int argc, char *argv[])
{
        if(getuid() != 0)
        {
                fprintf(stderr,"miniftpd must be started as root\n");
                ERR_EXIT("must be root");
        }

        int listenfd = tcp_server(NULL,5188);
        int conn;
        pid_t pid;
        session_t sess =
                {
                        /* 控制连接  */
                        -1,"","","",
                        /* 父子进程通道 */
                        -1,-1,
                };

        while(1)
        {
                conn = accept_timeout(listenfd, NULL, 0);
                if(conn == -1)
                { ERR_EXIT("accept_time"); }

                pid = fork();
                if(pid == -1)
                        ERR_EXIT("fork");
                if(pid == 0)
                {
                        //children
                        close(listenfd);
                        fprintf(stdout,"children\n");
                        sess.ctrl_fd = conn;
                        begin_session(&sess);
                }else
                {
                        //father
                        close(conn);
                        fprintf(stdout,"father\n");
                }
        }
        return 0;
}
