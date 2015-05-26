/*************************************************************************
	> File Name: netreport.c
	> Author: 
	> Mail: 
	> Created Time: Wed 11 Mar 2015 03:41:58 PM CST
 ************************************************************************/

#include"net_report.h"
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<errno.h>
#include<string.h>
#include<sys/time.h>
#include<sys/select.h>


#define N 128

void *pthread_netreport(void *arg)
{
    M_print_str("pthread netreport START !");
    struct sockaddr_in ip_addr;
    s_netinfo netinfo;
    if(arg == NULL)
    {
        printf("usage: pthread_netreport has no ip address information\n");
        return NULL;
    }
    netinfo = *(s_netinfo *)arg;
    free(arg);

    ip_addr.sin_family = AF_INET;
    ip_addr.sin_port = htons(netinfo.port);
    ip_addr.sin_addr.s_addr = inet_addr(netinfo.ip);

    int socket_fd, flag, ret;
    fd_set read_flag, write_flag;
    int reuse_addr=1;
    char buf[N];

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("usage: pthread_netreport socket fail -- ");
        return NULL;
    }
    flag = fcntl(socket_fd, F_GETFL, 0);
    if(netinfo.keepalive == 0)
    {
        
    }
    else
    {
        
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&netinfo.keepalive, sizeof(netinfo.keepalive));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, (void *)&netinfo.keepidle, sizeof(netinfo.keepidle));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&netinfo.keepintval, sizeof(netinfo.keepintval));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, (void *)&netinfo.keepcnt, sizeof(netinfo.keepcnt));
    
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse_addr, sizeof(reuse_addr));

    while(1)
    {
        if((connect(socket_fd, (struct sockaddr *)&ip_addr, sizeof(ip_addr))) < 0)
        {
            printf("usage: pthread_netreport connect fail -- %s\n", strerror(errno));
        }
        fcntl(socket_fd, F_GETFL, flag|O_NONBLOCK);
        FD_ZERO(&write_flag);
        FD_SET(socket_fd, &write_flag);
        while(1)
        {
            ret = select(socket_fd+1, &write_flag, NULL, NULL, &netinfo.waitd);
            if(ret > 0)
            {
                if(write(socket_fd, buf, sizeof(buf)) < 0)
                {
                     
                }
            }
            else    //链接断了
            {

            }
        }
    }
}
