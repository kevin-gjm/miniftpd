#include "sysutil.h"
#include "common.h"

int getlocalip(char *ip)
{
        char host[100]={0};
        if(gethostname(host,sizeof(host)) <  0)
                return -1;
        struct hostent *hp;
        if((hp = gethostbyname(host)) == NULL)
                return -1;
        strcpy(ip,inet_ntoa(*(struct in_addr*)hp->h_addr));
        return 0;
}

/*
  activate_nonblock 设置I/O为非阻塞模式
  @fd:文件描述符
*/
void activate_nonblock(int fd)
{
        int ret;
        int flags = fcntl(fd,F_GETFL);
        if(flags == -1)
                ERR_EXIT("fcntl");

        flags |= O_NONBLOCK;
        ret = fcntl(fd,F_SETFL,flags);
        if(ret == -1)
                ERR_EXIT("fcntl");

}

/*
  deactivate_nonblock 设置为阻塞模式
  @fd: 文件描述符
*/

void deactivate_nonblock(int fd)
{
        int ret;
        int flags = fcntl(fd, F_GETFL);
        if(-1 == flags)
                ERR_EXIT("fcntl");
        flags &= ~O_NONBLOCK;
        ret = fcntl(fd, F_SETFL);
        if(-1 == ret)
                ERR_EXIT("fcntl");
}

/*
  read_timeout 读超时检测函数，不含读操作
  @fd:文件描述符
  @wait_seconds:等待超时秒数，如果为0表示不检测超时
  成功(未超时)返回0，失败返回－1，超时返回－1并且errno=ETIMEDOUT，－2未知错误
*/
int read_timeout(int fd,unsigned int wait_seconds)
{
        int ret = 0;
        if(wait_seconds > 0)
        {
                fd_set read_fdset;
                struct timeval timeout;

                FD_ZERO(&read_fdset);
                FD_SET(fd,&read_fdset);

                timeout.tv_sec = wait_seconds;
                timeout.tv_usec = 0;
                do
                {
                        ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);

                } while (ret < 0 && errno == EINTR);

                if (0 == ret) {
                        ret = -1;
                        errno = ETIMEDOUT;
                }
                else if (1 == ret) {
                        ret = 0;
                }else
                        ret = -2;
        }//end wait_seconds >0

        return ret;
}

/*
  write_timeout 读超时检测函数
  @fd:文件描述符
  @wait_seconds:等待超时秒数，0表示不检测超时
  成功(未成功)返回0，失败返回－1，超时返回－1并且errno ＝ ETIMEDOUT. -2 未知错误
*/

int write_timeout(int fd,unsigned int wait_seconds)
{
        int ret = 0;
        if(wait_seconds > 0)
        {
                fd_set write_fdset;
                struct timeval timeout;

                FD_ZERO(&write_fdset);
                FD_SET(fd,&write_fdset);

                timeout.tv_sec = timeout;
                timeout.tv_usec = 0;

                do
                {
                        ret =select(fd+1, NULL, NULL, &write_fdset, &timeout);
                } while (ret < 0 && errno == EINTR);

                if(0 == ret)
                {
                        ret = -1;
                }else if (ret == 1) {
                        ret = 0;
                }else
                        ret = -2;
        }//end wait_seconds >0

        return ret;
}

/*
  accept_timeout 带超时的accept
  @fd:套接字
  @addr:输出参数，返回对方地址
  @wait_seconds:等待超时秒数，0表示不检测超时，表示正常模式
  成功(未超时)返回已连接的套接字，超时返回-1并且errno = ETIMEDOUT
*/

int accept_time(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
        int ret;

        socklen_t addrlen =  sizeof(struct sockaddr_in);

        if (wait_seconds > 0)
        {
                fd_set accept_fdset;
                struct timeval timeout;

                FD_ZERO(&accept_fdset);
                FD_SET(fd,&accept_fdset);
                timeout.tv_sec = wait_seconds;
                timeout.tv_usec = 0;
                do
                {
                        ret = select(fd+1,&accept_fdset,NULL,NULL,&timeout);
                } while (ret < 0 && errno == EINTR);

                if(-1 == ret)
                {
                        return -1;
                }else if(ret == 0)
                {
                        errno = ETIMEDOUT;
                        return -1;
                }else{
                        return -2;
                }
        }//end wait_seconds > 0

        if(addr != NULL)
        {
                ret = accept(fd, (struct sockaddr*) addr, &addrlen);
        }else{
                ret = accept(fd, NULL, NULL);
        }

        return ret;
}

/*
  connect_timeout  connect的超时版本
  @fd:套接字
  @addr:要连接的对方地址
  @wait_seconds:等待超时秒数,如果0表示正常模式
  成功(未超时)返回0,失败返回－1且errno = ETIMEDOUT
*/

int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
        int ret;
        socklen_t addrlen = sizeof(struct sockaddr_in);

        if(wait_seconds > 0)
                activate_nonblock activate_nonblock(fd);
        ret = connect(fd, (struct sockaddr*)addr, addrlen);
        //返回0的情况一般是本地连接,其他情况EINPROGRESS一定会被设置
        if(ret < 0 && errno == EINPROGRESS )
        {
                fd_set connect_fdset;
                struct timeval timeout;

                FD_ZERO(&connect_fdset);
                FD_SET(fd,&connect_fdset);

                timeout.tv_sec = wait_seconds;
                timeout.tv_usec = 0;

                do
                {
                        ret = select(fd+1,NULL,&connect_fdset,NULL,&timeout);
                } while (ret < 0 && errno == EINTR);
                if(ret == 0)
                {
                        ret = -1;
                        errno = ETIMEDOUT;
                }else if(ret < 0)
                {
                        return -1;
                }else if(ret == 1)
                {
                        //正常建立连接和sockfd发生错误都会导致可写(错误导致可读可写)通过检测错误代码确定是否正常
                        int err;
                        socklen_t socklen = sizeof(err);
                        //getsockopt存在移植性的问题，平台不同返回值不同
                        int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
                        if(sockoptret == -1)
                        {
                                return -1;
                        }
                        if(err == 0)
                        {
                                //连接成功
                                ret = 0;
                        }else{
                                errno = err;
                                ret = -1;
                        }
                }//end ret ==1

        }//end (ret < 0 && errno == EINPROGRESS )
        if(wait_seconds > 0)
        {
                deactivate_nonblock(fd);
        }

        return ret;
}

/*
  readn  读取固定字节数
  @fd:文件描述符
  @buf:接收缓冲区
  @count:要读取的字节数
  成功返回count,失败返回-1,读到EOF返回<count
*/

ssize_t readn(int fd, void *buf,size_t count)
{
        size_t nleft = count;
        ssize_t nread;
        char *bufp = (char*)buf;

        while(nleft > 0)
        {
                if((nread = read(fd,bufp,nleft)) < 0)
                {
                        if(errno == EINTR)
                                continue;
                        return -1;
                }else if(nread == 0)
                        return count -nleft;
                bufp += nread;
                nleft -= nread;
        }

        return count;
}

/*
  writen  发送固定字节数
  @fd: 文件描述符
  @buf:发送缓冲区
  @count:要读取的字节数
  成功返回count,失败返回-1
*/

ssize_t writen(int fd,const char *buf,size_t count)
{
        size_t nleft = count;
        ssize_t nwritten;
        char *bufp = (char *) buf;

        while(nleft > 0 )
        {
                if((nwritten = write(fd, bufp, nleft)) < 0)
                {
                        if(errno == EINTR)
                                continue;
                        return -1;
                }else if(nwritten == 0)
                        continue;
                bufp += nwritten;
                nleft -= nwritten;
        }

        return count;

}
/*
  recv_peek 仅仅查看套接字缓冲区数据，但是不移除数据
  @sockfd:套接字
  @buf:接收缓冲区
  @len:长度
  成功返回>=0,失败返回－1
*/
ssize_t recv_peek(int sockfd,void *buf,size_t len)
{
        while(1)
        {
                int ret = recv(sockfd,buf,len,MSG_PEEK);
                if(ret == -1 && errno == EINTR)
                        continue;
                return ret;
        }
}
/*
  readline 按行读取数据,以\n作为一行的分隔符号
  @sockfd: 套接字
  @buf:接收缓冲区
  @maxline:每一行的最大长度
  成功返回>=0,失败返回-1
*/
ssize_t readline(int sockfd,void *buf,size_t maxline)
{
        int ret;
        int i;
        int nread;
        char *bufp = buf;
        int nleft = maxline;
        while(1)
        {
                ret = recv_peek(sockfd,bufp,nleft);
                if(ret < 0)
                {
                        ERR_EXIT("recv_peek");
                }else if(ret == 0)
                {
                        return ret;
                }
                nread = ret;

                for(i = 0;i < nread;i++)
                {
                        if(bufp[i]=='\n')
                        {
                                ret = readn(sockfd,bufp,i+1);
                                if(ret != i+1)
                                {
                                        ERR_EXIT("readn");
                                }
                                return ret;
                        }//end if(bufp[i]=='\n')
                        if(nread > nleft)
                                ERR_EXIT("read");
                }// end for

                ret = readn(sockfd,bufp,nread);
                nleft -= nread;
                bufp += nread;
        }//end while
        return -1;
}

void send_fd(int sockfd,int fd)
{
        int ret;
        struct msghdr msg;
        struct cmsghdr *p_cmsg;//辅助数据结构
        struct iovec vec;

        char cmsgbuf[CMSG_SPACE(sizeof(fd))];
        int *p_fds;
        char sendchar = 0;
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);

        p_cmsg =CMSG_FIRSTHDR(&msg);
        p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
        p_cmsg->cmsg_level = SOL_SOCKET;
        p_cmsg->cmsg_type = SCM_RIGHTS;

        p_fds = (int *) CMSG_DATA(p_cmsg);
        *p_fds = fd;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &vec;
        msg.msg_iovlen = 1;
        msg.msg_flags = 0;

        vec.iov_base = &sendchar;
        vec.iov_len = sizeof(sendchar);

        ret = sendmsg(sockfd,&msg,0);
        if(ret != 1)
        {
                ERR_EXIT("sendmsg");
        }
}

int recv_fd(const int sockfd)
{
        int ret;
        char recvchar;
        int recvfd;
        struct msghdr msg;
        struct cmsghdr *p_cmsg;
        struct iovec vec;
        char cmsgbuf[CMSG_SPACE(sizeof(recvchar))];
        int *p_fds;

        vec.iov_base = &recvchar;
        vec.iov_len = sizeof(recvchar);

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &vec;
        msg.msg_iovlen = 1;
        msg.msg_flags = 0;
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = dizeof(cmsgbuf);

        p_cmsg = CMSG_FIRSTHDR(&msg);
        if(p_cmsg == NULL)
                ERR_EXIT("CMSG_FIRSTHDR");
        p_fds = (int *) CMSG_DATA(p_cmsg);
        *p_fds = -1;
        ret = recvmsg(sockfd, &msg, 0);
        if(ret != 1)
                ERR_EXIT("recvmsg");

        p_cmsg = CMSG_FIRSTHDR(&msg);
        if(p_cmsg == NULL)
                ERR_EXIT("CMSG_FIRSTHDR");
        p_fds = (int *)CMSG_DATA(p_cmsg);
        recvfd = *p_fds;
        return recvfd;
}
