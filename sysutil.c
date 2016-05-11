#include "sysutil.h"
#include <netdb.h>

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
