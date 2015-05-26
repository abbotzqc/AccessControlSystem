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
		pfile = fopen(EXPORTDIR, "w");//"r+");
		if(pfile == NULL)
		{
			M_print_err(errno);
			return -1;
		}
		fprintf(pfile, "%d", gpio_no);
		fclose(pfile);
	}
	//open  sys/class/gpio/gpio24
	sprintf(dir, "%s%s", gpiodir, GPIO_DIRE);
	pfile = fopen(dir, "rw");//"r+");
	if(pfile == NULL)
	{
		M_print_err(errno);
		return -1;
	}
	fprintf(pfile, "%s", parm->direction);
    fclose(pfile);
	sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
	pfile = fopen(dir, "rw");//"r+");
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
		fprintf(pfile, "%d", gpio_no);
		fclose(pfile);
	}
	//open  sys/class/gpio/gpio24
	sprintf(dir, "%s%s", gpiodir, GPIO_DIRE);
	pfile = fopen(dir, "rw");//"r+");//"w");
	if(pfile == NULL)
	{
		M_print_err(errno);
		return -1;
	}
	fprintf(pfile, "%s", parm->direction);
    fclose(pfile);
	return 0;
}

void *pthread_io(void *arg)
{
    M_print_time_str("pthread io START !");
	if(arg == NULL)	
	{
		M_print_str("no parms passed in");
		return NULL;
	}
    int cnt;	
	int gpio_in_no, gpio_out_no;
    char dir[36]={};
    char gpiodir[24]={};
	FILE *pfile_in = NULL;
	FILE *pfile_out = NULL;
	int ret;
    int val = 0;
    int val_old = 0;
    time_t nowtime;
	s_ioMSG io_msgbuf;
	s_dbMSG db_msgbuf;
    s_stateMSG statemsg;
	s_ioctrl ioctrlparm;

	s_io_parm *parm = (s_io_parm *)arg;

	if(parm->gpio_in == NULL || parm->gpio_out == NULL)
	{
		M_print_str("no gpio dir passed in");
		return NULL;
	}

	ioctrlparm.gpio = parm->gpio_in;
	ioctrlparm.direction = IN;
	//ret = get_gpioval_pfile(&pfile_in, &ioctrlparm);
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		return NULL;
	}
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
	pfile_in = fopen(dir, "rw");//"r+");
	if(pfile_in == NULL)
	{
		M_print_err(errno);
		return NULL;
	}

	ioctrlparm.gpio = parm->gpio_out;
	ioctrlparm.direction = OUT;
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		return NULL;
	}
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
	pfile_out = fopen(dir, "rw");//"r+");//"w");
	if(pfile_out == NULL)
	{
		M_print_err(errno);
		return NULL;
	}
    M_print_str("pthread io open io pin OK !");
    fscanf(pfile_in, "%d", &val);
    val_old = val;
    if(val == READY_CLOSE)
    {
        fprintf(pfile_out, "%d", CLOSE);
        M_print_time_str("pthread io close door");
        val_old = val;
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
    else
    {
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
    while(1)
	{
        val_old = val;
		//close door
		if((ret = fscanf(pfile_in, "%d", &val)) > 0)
        {
            if(val == READY_CLOSE && val_old != READY_CLOSE)
            {
                fprintf(pfile_out, "%d", CLOSE);
                M_print_time_str("pthread io close door");
                val_old = val;
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
        }
		ret = msgrcv(parm->id_iomsg, &io_msgbuf, sizeof(io_msgbuf)-sizeof(long), 0, IPC_NOWAIT);  //block or noblock
		if(ret < 0)
		{
            if(errno == ENOMSG);
            else
            {
			    M_print_err(errno);
            }
			//return NULL;
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
                fprintf(pfile_out, "%d", OPEN);
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
                for(cnt=30;cnt>0;cnt--)
                {
                    sleep(1);
                    val_old = val;
                    if((ret = fscanf(pfile_in, "%d", &val)) > 0)
                    {
                        if(val == READY_CLOSE && val_old != READY_CLOSE)             //val_old==READY_CLOSE 说明门吸已经合上了，可以上锁了，并且已经动作关门上锁了
                        {
                            fprintf(pfile_out, "%d", CLOSE);
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
                        //else
                        //{
                        //    continue;
                        //}
                    }
                }
            }
        }
        
	}
	return NULL;
}
//
//void main()
//{
//	return;
//}
//
