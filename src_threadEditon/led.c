/*************************************************************************
	> File Name: led.c
	> Author: 
	> Mail: 
	> Created Time: Thu 30 Apr 2015 02:01:52 PM CST
 ************************************************************************/

#include<stdio.h>
#include"led.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<pthread.h>
#include"print.h"
#include<errno.h>
#include<string.h>
#define IOCTL_ON    ioctl(fd, LightON, parm->led_no)
#define IOCTL_OFF   ioctl(fd, LightOFF, parm->led_no)

void *pthread_led(void *arg)
{
    s_ledparm *parm = (s_ledparm*)arg;
    int fd;
    if((fd = open(parm->leddir, O_RDWR|O_NONBLOCK)) < 0)
    {
        M_print_err(errno);
        return NULL;
    }
    
    switch(parm->blingtype)
    {
        case LightON:
        ioctl(fd, LightON, parm->led_no);
        break;
        case LightOFF:
        ioctl(fd, LightOFF, parm->led_no);
        break;
        case BlingSlow:
        while(1)
        {
            IOCTL_ON;
            usleep(1000000);
            pthread_testcancel();
            IOCTL_OFF;
            pthread_testcancel();
            usleep(1000000);
        }
        break;
        case BlingFast:
        while(1)
        {
            IOCTL_ON;
            usleep(200000);
            pthread_testcancel();
            IOCTL_OFF;
            pthread_testcancel();
            usleep(200000);
        }
        break;
    }
    return NULL;
}
