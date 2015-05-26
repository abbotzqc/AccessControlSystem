/*************************************************************************
	> File Name: monitor_key.c
	> Author: 
	> Mail: 
	> Created Time: Mon 13 Apr 2015 03:25:37 PM CST
 ************************************************************************/

#include<stdio.h>
#include"monitor_key.h"
#include<errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/time.h>
#include<time.h>
#include"msgqueue.h"
#include"io_ctrl.h"

#ifndef PRINTF_H
#define PRINTF_H
#include<time.h>
#define M_print_err(errno) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)
#define M_print_str(str)   do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}while(0)
#endif

//#define DEBUG
#ifdef DEBUG 
#define D_M_PRINT(integret, str) printf("debug usage %s : %d fail -- integret:%d  str:%s\n", __func__, __LINE__, integret, str)
#else
#define D_M_PRINT(integret, str)
#endif


typedef struct
{
    unsigned int dummy1;
    unsigned int dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int value;
    unsigned int dummy3;
    unsigned int dummy4;
    unsigned int dummy5;
    unsigned int dummy6;
}s_inputdata;

#define KEY5 257
#define KEY4 256
#define KEYMAXNUM 2
#define Key4state KEY4-256
#define Key5state KEY5-256
#define KEY_DOWN 0
#define KEY_UP 1

int main(int agrc, const char *argv[])
{
    int fd, ret;
    fd_set rdflag;
    unsigned char keystate[KEYMAXNUM] = {};
    unsigned char bufstate[KEYMAXNUM] = {};
    s_inputdata event;
    s_msginfo msginfo_key5,msginfo_key4;
    s_ioMSG iomsg_key5, iomsg_key4;
    msginfo_key5.path_id = 'i';
    if(msgqueue_create(&msginfo_key5) != 0)
    {
        M_print_str("msgqueue_create fail");
    }
    msginfo_key4.path_id = 'j';
    msgqueue_create(&msginfo_key4);


    fd = open(getenv("KEYPAD_DEV"), O_RDONLY);//|O_NONBLOCK);
    if(fd < 0)
    {
        M_print_err(errno);
        return 1;
    }
    FD_ZERO(&rdflag);
    FD_SET(fd, &rdflag);
    while(1)
    {
        ret = select(fd+1, &rdflag, NULL, NULL, 0);
        if(!(ret > 0))
        {
            M_print_err(errno);
            FD_ZERO(&rdflag);
            FD_SET(fd, &rdflag);
            continue;
        }
        ret = read(fd, &event, sizeof(event));
        if(ret != sizeof(event))
        {
            continue;
        }
        switch(event.code)
        {
            case KEY5:
            //给进程发信号或者发消息
            if(event.value == KEY_DOWN)
            {
                iomsg_key5.mtype = 1;
                iomsg_key5.io_switch = OPENDOOR;
                iomsg_key5.t = time(NULL);
                msgsnd(msginfo_key5.id_msg, &iomsg_key5, sizeof(iomsg_key5)-sizeof(long), 0);
                D_M_PRINT(0, "send msg");
            }
            else //KEY_UP
            {}
            D_M_PRINT(event.value, "");
            break;
            case KEY4:
            //给进程发信号
            if(event.value == KEY_DOWN)
            {}
            else //KEY_UP
            {}
            D_M_PRINT(event.value, "");
            break;
            default:
            break;
        }
    }
    return 1;
}
