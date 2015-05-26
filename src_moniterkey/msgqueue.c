#include"msgqueue.h"
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//
//#ifndef PRINTF_H
//#define PRINTF_H
//#include<time.h>
//#define M_print_err(errno) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)
//#define M_print_str(str)   do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}while(0)
//#endif
//
////#define DEBUG
//#ifdef DEBUG 
//#define D_M_PRINT(integret, str) printf("debug usage %s : %d fail -- integret:%d  str:%s\n", __func__, __LINE__, integret, str)
//#else
//#define D_M_PRINT(integret, str)
//#endif
//
#ifndef MSGPATH_H
#define MSGPATH_H
#define MSGPATH   "./parm"
#endif

/*
 *  参数：msginfo
 *	return : -1 ---- fail  0 ---- success
 * */
int msgqueue_create(s_msginfo *msginfo)
{
    char *buf;
    buf = (char*)malloc(strlen(MSGPATH)+strlen("mkdir "));
    sprintf(buf, "mkdir %s", MSGPATH);
    system(buf);
    free(buf);
    buf = NULL;
	msginfo->key = ftok(MSGPATH, msginfo->path_id);
	if(msginfo->key < 0)
	{
		M_print_err(errno);
		return -1;
	}
	msginfo->id_msg = msgget(msginfo->key, IPC_CREAT|0666);
	if(msginfo->id_msg < 0)
	{
		M_print_err(errno);
		return -1;
	}
	return 0;
}

