//
//#ifndef PRINTF_H
//#define PRINTF_H
//#include<time.h>
//#define M_print_err(errno) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)
//#define M_print_str(str)   do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}while(0)
//#endif
//

#include"print.h"

#ifndef NET_REPORT_H
#define NET_REPORT_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct
{
    int port;
    char ip[12];
    int keepidle;
    int keepcnt;
    int keepintval;
    int keepalive;
    struct timeval waitd;
}s_netinfo;

void *pthread_netreport(void *arg);

//#include"netreport.h"
#endif
