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
#include <pwd.h>
//#include <uuid/uuid.h>
#include <shadow.h>
#include <crypt.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define ERR_EXIT(m)                             \
        do                                      \
        {                                       \
                perror(m);                      \
                exit(EXIT_FAILURE);             \
        }                                       \
        while(0)


#define MAX_COMMAND_LINE 1024
#define MAX_COMMAND 32
#define MAX_ARG 1024

#define MINIFTPD_CONF "miniftpd.conf"

#endif  /* _COMMON_H_ */
