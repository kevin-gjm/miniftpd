#ifndef _COMMON_H_
#define _COMMON_H_

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define ERR_EXIT(m)                             \
        do                                      \
        {                                       \
                perror(m);                      \
                exit(EXIT_FAILURE);             \
        }                                       \
        while(0)


#endif  /* _COMMON_H_ */
