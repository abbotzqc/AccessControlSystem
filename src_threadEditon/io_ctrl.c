/*************************************************************************
	> File Name: io_ctrl.c
	> Author: 
	> Mail: 
	> Created Time: Wed 18 Mar 2015 05:25:48 PM CST
 ************************************************************************/
#include"io_ctrl.h"
#include<stdio.h>
#include<errno.h>
#include"msgqueue.h"
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include <time.h>
#include<unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

//#define AGAIN  lseek(fd_in, 0, SEEK_SET)

/*
* 功能：创建并且打开gpio控制目录
* 备注：这个函数有问题
* */

int get_gpioval_pfile(FILE **pfile_val, s_ioctrl *parm)
{
	int gpio_no;
	char dir[36];
    char gpiodir[24];
	FILE *pfile;
	//gpio1-24
	gpio_no = (atoi(parm->gpio+4)<<5) + atoi(parm->gpio+6);
	sprintf(gpiodir,"%s%d", GPIODIR, gpio_no);        //eg:/sys/class/gpio/gpio24
    //判断是否存在，不存在创建
	if(opendir(gpiodir) == NULL)  //if(access(dir, F_OK|R_OK|W_OK) == 0)
	{
		M_print_err(errno);
		return 0;
	}
	else  //创建
	{
		//export gpio24
		pfile = fopen(EXPORTDIR, "r+");
		if(pfile == NULL)
		{
			M_print_err(errno);
			return -1;
		}
		if(fprintf(pfile, "%d", gpio_no) < 0)
        {
            M_print_err(errno);
        }
        fflush(pfile);
		fclose(pfile);
	}
	//open  sys/class/gpio/gpio24
	sprintf(dir, "%s%s", gpiodir, GPIO_DIRE);
	pfile = fopen(dir, "r+");
	if(pfile == NULL)
	{
		M_print_err(errno);
		return -1;
	}
	if(fprintf(pfile, "%s", parm->direction) < 0)
    {
        M_print_err(errno);
    }
    fflush(pfile);
    fclose(pfile);
	sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
	pfile = fopen(dir, "r+");
	if(pfile == NULL)
	{
		M_print_err(errno);
		return -1;
	}
	else
	{
		*pfile_val = pfile;
	}
	return 0;
}

/*
* 功能：创建gpio目录，初始化gpio
*
* */
int gpio_init(char *gpiodir, s_ioctrl *parm)
{
	int gpio_no;
	char dir[24]={};
   // char gpiodir[sizeof(GPIODIR)+4]={};
	FILE *pfile;
	//gpio1-24
	gpio_no = (atoi(parm->gpio+4)<<5) + atoi(parm->gpio+6);
	sprintf(gpiodir,"%s%d", GPIODIR, gpio_no);        //eg:/sys/class/gpio/gpio24
    //判断是否存在，不存在创建
	if(opendir(gpiodir) == NULL)  //if(access(dir, F_OK|R_OK|W_OK) == 0)
	{
		//export gpio24
		pfile = fopen(EXPORTDIR, "w");
		if(pfile == NULL)
		{
			M_print_err(errno);
			return -1;
		}
        usleep(10);
		if(fprintf(pfile, "%d", gpio_no) < 0)
        {
            M_print_err(errno);
        }
        fflush(pfile);
		fclose(pfile);
	}
	//open  sys/class/gpio/gpio24
	sprintf(dir, "%s%s", gpiodir, GPIO_DIRE);
	pfile = fopen(dir, "w");              //用rw不行
	if(pfile == NULL)
	{
		M_print_err(errno);
		return -1;
	}
	if(fprintf(pfile, "%s", parm->direction) < 0)
    {
        M_print_err(errno);
    }
    fflush(pfile);
    fclose(pfile);
	return 0;
}

void *pthread_io(void *arg)
{
    M_print_time_str("pthread io START !");
	if(arg == NULL)	
	{
		M_print_str("no parms passed in");
        exit(0);
	}
    int cnt;	
    char dir[36]={};
    char gpiodir[24]={};
	int ret;
    char val = 0;
    char val_old = 0;
    char lock = 0;               //0 -- 未上锁    1 -- 上锁了
    time_t nowtime;
	s_ioMSG io_msgbuf;
	s_dbMSG db_msgbuf;
    s_stateMSG statemsg;
	s_ioctrl ioctrlparm;

    int fd_in, fd_out;

	s_io_parm *parm = (s_io_parm *)arg;

	if(parm->gpio_in == NULL || parm->gpio_out == NULL)
	{
		M_print_str("no gpio dir passed in");
		exit(0);
	}

	ioctrlparm.gpio = parm->gpio_in;
	ioctrlparm.direction = IN;
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		exit(0);
	}
    bzero(dir, sizeof(dir));
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);

    if((fd_in = open(dir, O_RDWR)) < 0)
    {
        M_print_err(errno);
	    exit(0);
    }

	ioctrlparm.gpio = parm->gpio_out;
	ioctrlparm.direction = OUT;
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		exit(0);
	}
    bzero(dir, sizeof(dir));
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
    
    if((fd_out = open(dir, O_RDWR)) < 0)
    {
        M_print_err(errno);
	    exit(0);
    }
    M_print_str("pthread io open io pin OK !");
    //先读取一下门状态
    AGAIN;
    if(read(fd_in, &val, 1) < 0)
    {
        M_print_err(errno);
    }
    //上锁
    if(val == READY_CLOSE)
    {
        AGAIN;
        if((write(fd_out, "1", CLOSE)) < 0)
        {//上锁失败
            M_print_err(errno);
            lock = 0;
        }
        else     //上锁成功 
        {
            M_print_time_str("pthread io close door");
            lock = 1;                      //记录上一个状态为上锁成功
            //insert db close 
            db_msgbuf.mtype = OPT_STATE_TABLE;
            statemsg.state = CLOSEDOOR;
            statemsg.t = time(NULL);
            memcpy(db_msgbuf.buf, &statemsg, sizeof(statemsg));
            ret = msgsnd(parm->id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
            if(ret < 0)
            {
                M_print_err(errno);
            }
        }
    }
    else     //门未关着
    {
        lock = 0;
        //insert db close 
        db_msgbuf.mtype = OPT_STATE_TABLE;
        statemsg.state = OPENDOOR;
        statemsg.t = time(NULL);
        memcpy(db_msgbuf.buf, &statemsg, sizeof(statemsg));
        ret = msgsnd(parm->id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
        if(ret < 0)
        {
            M_print_err(errno);
        }
    }
    val_old = val;
    while(1)
	{
		//close door
        AGAIN;
        if(read(fd_in, &val, 1) > 0)
        {
            if(val == READY_CLOSE && lock == 0 && val_old != READY_CLOSE)
            {
                AGAIN;
                if(write(fd_out, "1", CLOSE) < 0)
                {
                    val_old = 0;
                    M_print_err(errno);
                    continue;
                }
                M_print_time_str("pthread io close door");
                val_old = 1;
                //insert db close 
                db_msgbuf.mtype = OPT_STATE_TABLE;
                statemsg.state = CLOSEDOOR;
                statemsg.t = time(NULL);
                memcpy(db_msgbuf.buf, &statemsg, sizeof(statemsg));
                ret = msgsnd(parm->id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
                if(ret < 0)
                {
                    M_print_err(errno);
                    //return;
                }
            }
            val_old = val;
        }
        else
        {
            M_print_err(errno);
            continue;
        }
		ret = msgrcv(parm->id_iomsg, &io_msgbuf, sizeof(io_msgbuf)-sizeof(long), 0, IPC_NOWAIT);  //block or noblock
		if(ret < 0)
		{
            if(errno == ENOMSG);
            else
            {
			    M_print_err(errno);
            }
			//exit(0);
		}
        else
        {
            if(ret > 0)//有消息
            {
                nowtime = time(NULL);
                if(difftime(nowtime, io_msgbuf.t) > 30)
                {
                    continue;
                }
                //open door
                AGAIN;
                if(write(fd_out, "1", OPEN) < 0)
                {
                    M_print_err(errno);
                    continue;
                }
                lock = 0;
                M_print_time_str("pthread io open door");
                //insert db open
                db_msgbuf.mtype = OPT_STATE_TABLE;
                statemsg.state = OPENDOOR;
                statemsg.t = time(NULL);
                memcpy(db_msgbuf.buf, &statemsg, sizeof(statemsg));
                ret = msgsnd(parm->id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
                if(ret < 0)
                {
                    M_print_err(errno);
                    //return;
                }
                //close door
                val_old = READY_CLOSE;
                for(cnt=30;cnt>0;cnt--)
                {
                    sleep(1);
                    AGAIN;
                    if(read(fd_in, &val, 1) > 0)
                    {
                        if(val == READY_CLOSE && lock == 0 && val_old == 0)     // 说明门吸已经合上了，可以上锁了，并且已经动作关门上锁了
                        {
                            AGAIN;
                            if(write(fd_out, "1", CLOSE) < 0)
                            {
                                lock = 0;
                                M_print_err(errno);
                                continue;
                            }
                            lock = 1;
                            //insert db close 
                            db_msgbuf.mtype = OPT_STATE_TABLE;
                            statemsg.state = CLOSEDOOR;
                            statemsg.t = time(NULL);
                            memcpy(db_msgbuf.buf, &statemsg, sizeof(statemsg));
                            ret = msgsnd(parm->id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
                            if(ret < 0)
                            {
                                M_print_err(errno);
                                //return;
                            }
                            break;
                        }
                    }
                    else
                    {
                        M_print_err(errno);
                    }
                    val_old = val;
                }
            }
        }
	}
	exit(0);
}
//
//void main()
//{
//	return;
//}
//
