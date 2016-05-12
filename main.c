#include "common.h"
#include "sysutil.h"
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
               }else
               {
                       //father
                       close(conn);
                       fprintf(stdout,"father\n");
               }
        }
        return 0;
}
