/*************************************************************************
	> File Name: main.c
	> Author:zhaoqiancheng
	> Mail: 
	> Created Time: Mon 09 Mar 2015 11:27:42 AM CST
 ************************************************************************/

#include<stdio.h>
#include<mysql/mysql.h>
#include<pthread.h>
//#include<ccl/ccl.h>
#include "msgqueue.h"
//#include"net_report.h"              
#include"rs485.h"
#include"io_ctrl.h"
#include"db.h"
#include<stdlib.h>
#include"print.h"
#include"ReadConf.h"
#include"led.h"
#include"net_report.h"
#include"getdate.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<errno.h>
#include<string.h>


#define N 128

#define GET_CONF_ITEM(str) get_conf_item(&conf, str)

#define SOFTWARE_EDITON "v15.5.3"

unsigned char door_num;
char *SoftWare_Editon = SOFTWARE_EDITON;

/************************************************************************
* parse : argv[1]--configure file path
*
* */
s_msginfo msginfo_io, msginfo_db;
s_io_parm parm_io;
s_db_parm parm_db;
s_rs485_parm parm_rs485_one, parm_rs485_two;
s_netinfo parm_net;
s_ledparm parm_led;
int main(int argc, const char *argv[])
{
    M_print_time_str("AccessControlSys START !===================================================");
    if(argc == 1)
    {
        M_print_str("no configure file passed in !");
        return 1;
    }

    s_conf conf;
    unsigned short send_addr;
    unsigned char rs485_flag1 = 0; 
    unsigned char rs485_flag2 = 0;
    // 
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

    //线程id
	pthread_t id_rs485_one, id_rs485_two, id_db, id_led;
    //创建消息队列：数据库消息队列 io消息队列
    msginfo_db.path_id = 'd';
    if(msgqueue_create(&msginfo_db) != 0)
    {
        M_print_str("db msgqueue_create fail");
        return 1;
    }
    msginfo_io.path_id = 'i';
    if(msgqueue_create(&msginfo_io) != 0)
    {
        M_print_str("io msgqueue_create fail");
        return 1;
    }
    
    read_conf(&conf, argv[1]);
    
//    mysql_library_init(0,NULL,NULL);
//    
    GET_CONF_ITEM("leddir");
    strcpy(parm_led.leddir, conf.parm);
    GET_CONF_ITEM("ledno");
    parm_led.led_no = atoi(conf.parm);
    parm_led.blingtype = BlingFast; 
	pthread_create(&id_led, NULL, &pthread_led, &parm_led);

    get_conf_item(&conf, "zone-no");
    send_addr = (1<<13)|(atoi(conf.parm)<<8);
    GET_CONF_ITEM("dev-no");
    send_addr |= atoi(conf.parm);
    
    door_num = atoi(conf.parm);        
    parm_db.door_num = atoi(conf.parm);        //门编号，不是门地址
    parm_db.id_dbmsg = msginfo_db.id_msg;

    parm_io.send_addr = send_addr;
    parm_io.id_iomsg = msginfo_io.id_msg;
    parm_io.id_dbmsg = msginfo_db.id_msg;
    
    parm_rs485_one.send_addr = send_addr;
    parm_rs485_one.id_iomsg = msginfo_io.id_msg;
    parm_rs485_one.id_dbmsg = msginfo_db.id_msg;
    parm_rs485_one.no = 1;

    parm_rs485_two.send_addr = send_addr;
    parm_rs485_two.id_iomsg = msginfo_io.id_msg;
    parm_rs485_two.id_dbmsg = msginfo_db.id_msg;
    parm_rs485_two.no = 2;
   
    GET_CONF_ITEM("db-user");
    strncpy(parm_db.user, conf.parm, sizeof(parm_db.user));
    GET_CONF_ITEM("db-password");
    strncpy(parm_db.password, conf.parm, sizeof(parm_db.password));
    GET_CONF_ITEM("db-schema");
    strncpy(parm_db.db_schema, conf.parm, sizeof(parm_db.db_schema));
    
    strncpy(parm_rs485_one.user, parm_db.user, sizeof(parm_rs485_one.user));
    strncpy(parm_rs485_one.password, parm_db.password, sizeof(parm_rs485_one.password));
    strncpy(parm_rs485_one.db_schema, parm_db.db_schema, sizeof(parm_rs485_one.db_schema));
    strncpy(parm_rs485_two.user, parm_db.user, sizeof(parm_rs485_two.user));
    strncpy(parm_rs485_two.password, parm_db.password, sizeof(parm_rs485_two.password));
    strncpy(parm_rs485_two.db_schema, parm_db.db_schema, sizeof(parm_rs485_two.db_schema));

    GET_CONF_ITEM("gpio-feedbackpin");
    strncpy(parm_io.gpio_in, conf.parm, sizeof(parm_io.gpio_in));
    GET_CONF_ITEM("gpio-controlpin");
    strncpy(parm_io.gpio_out, conf.parm, sizeof(parm_io.gpio_out));

    GET_CONF_ITEM("rs485-dir-1");
    if(strlen(conf.parm) != 0)
    {
        rs485_flag1 = 1;
        strncpy(parm_rs485_one.dir, conf.parm, sizeof(parm_rs485_one.dir));
        GET_CONF_ITEM("codetype-1");//hex -- 1
        if(!strcmp(conf.parm, "HEX"))
        {
            parm_rs485_one.codetype = 1;
        }
        else
        {
            parm_rs485_one.codetype = 0;
        }
    }
    GET_CONF_ITEM("rs485-dir-2");
    if(strlen(conf.parm) != 0)
    {
        rs485_flag2 = 1;
        strncpy(parm_rs485_two.dir, conf.parm, sizeof(parm_rs485_two.dir));
        GET_CONF_ITEM("codetype-2");
        if(!strcmp(conf.parm, "HEX"))
        {
            parm_rs485_two.codetype = 1;
        }
        else
        {
            parm_rs485_two.codetype = 0;
        }
    }
    
    GET_CONF_ITEM("port");
    parm_net.port = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("ip");
    strncpy(parm_net.ip, conf.parm, sizeof(parm_net.ip));
    GET_CONF_ITEM("keepidle");
    parm_net.keepidle = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("keepcnt");
    parm_net.keepcnt = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("keepinterval");
    parm_net.keepintval = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("keepalive");
    parm_net.keepalive = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("usec");
    parm_net.waitd.tv_usec = strtol(conf.parm, NULL, 10);
    GET_CONF_ITEM("sec");
    parm_net.waitd.tv_sec = strtol(conf.parm, NULL, 10);

    release_conf(&conf);
    
    pthread_create(&id_db, NULL, &pthread_db, &parm_db);
    pthread_detach(id_db);
    sleep(1);
    //pthread_create(&id_io, NULL, &pthread_io, &parm_io);
    //pthread_detach(id_io);
    if(rs485_flag1)
    {
        pthread_create(&id_rs485_one, NULL, &pthread_rs485, &parm_rs485_one);
        pthread_detach(id_rs485_one);
    }
    if(rs485_flag2)
    {
        sleep(1);
        pthread_create(&id_rs485_two, NULL, &pthread_rs485, &parm_rs485_two);
        pthread_detach(id_rs485_two);
    }
    pthread_cancel(id_led);
    pthread_join(id_led, NULL);
    parm_led.blingtype = BlingSlow; 
	pthread_create(&id_led, NULL, &pthread_led, &parm_led);

    //链接服务器端
    M_print_str("pthread netreport START !");
    struct sockaddr_in ip_addr;

    ip_addr.sin_family = AF_INET;
    ip_addr.sin_port = htons(parm_net.port);
    ip_addr.sin_addr.s_addr = inet_addr(parm_net.ip);

    int socket_fd, flag;
    fd_set read_flag, write_flag;
    int reuse_addr=1;
    char buf[N];

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("usage: pthread_netreport socket fail -- ");
        return 0;
    }
    flag = fcntl(socket_fd, F_GETFL, 0);
    if(parm_net.keepalive > 0 || parm_net.keepidle > 0 || parm_net.keepintval > 0 || parm_net.keepcnt > 0)
    {
        parm_net.keepalive = 1;
        parm_net.keepidle = 60;
        parm_net.keepintval = 5;
        parm_net.keepcnt = 3;
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&parm_net.keepalive, sizeof(parm_net.keepalive));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, (void *)&parm_net.keepidle, sizeof(parm_net.keepidle));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&parm_net.keepintval, sizeof(parm_net.keepintval));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, (void *)&parm_net.keepcnt, sizeof(parm_net.keepcnt));
    
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse_addr, sizeof(reuse_addr));

    fcntl(socket_fd, F_GETFL, flag|O_NONBLOCK);
    if((connect(socket_fd, (struct sockaddr *)&ip_addr, sizeof(ip_addr))) < 0)
    {
        M_print_err(errno);
    }
    else
    {
        FD_ZERO(&write_flag);
        FD_SET(socket_fd, &write_flag);
        if(select(socket_fd+1, &write_flag, NULL, NULL, 0) > 0)
        {
            getdate_unknow_t(buf);
            if(write(socket_fd, buf, strlen(buf)) < 0)
            {
                M_print_err(errno);
            }
        }
        else    //链接断了
        {
            fcntl(socket_fd, F_GETFL, flag|O_NONBLOCK);
            if((connect(socket_fd, (struct sockaddr *)&ip_addr, sizeof(ip_addr))) < 0)
            {
                M_print_err(errno);
            }
        }
    }
    

    //pthread_io(&parm_io);
    M_print_time_str("pthread io START !");
   

	if(parm_io.gpio_in == NULL || parm_io.gpio_out == NULL)
	{
		M_print_str("no gpio dir passed in");
		return 0;
	}

	ioctrlparm.gpio = parm_io.gpio_in;
	ioctrlparm.direction = IN;
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		return 0;
	}
    bzero(dir, sizeof(dir));
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);

    if((fd_in = open(dir, O_RDWR)) < 0)
    {
        M_print_err(errno);
	    return 0;
    }

	ioctrlparm.gpio = parm_io.gpio_out;
	ioctrlparm.direction = OUT;
	ret = gpio_init(gpiodir, &ioctrlparm);
	if(ret < 0)
	{
		M_print_str("gpio init fail");
		return 0;
	}
    bzero(dir, sizeof(dir));
    sprintf(dir, "%s%s", gpiodir, GPIO_VAL);
    
    if((fd_out = open(dir, O_RDWR)) < 0)
    {
        M_print_err(errno);
	    return 0;
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
            ret = msgsnd(parm_io.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
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
        ret = msgsnd(parm_io.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
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
                ret = msgsnd(parm_io.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
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
		ret = msgrcv(parm_io.id_iomsg, &io_msgbuf, sizeof(io_msgbuf)-sizeof(long), 0, IPC_NOWAIT);  //block or noblock
		if(ret < 0)
		{
            if(errno == ENOMSG);
            else
            {
			    M_print_err(errno);
            }
			//return 0;
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
                ret = msgsnd(parm_io.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
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
                            ret = msgsnd(parm_io.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0);
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

        FD_ZERO(&write_flag);
        FD_SET(socket_fd, &write_flag);
        if(select(socket_fd+1, &write_flag, NULL, NULL, 0) > 0)
        {
            getdate_unknow_t(buf);
            if(write(socket_fd, buf, strlen(buf)) < 0)
            {
                M_print_err(errno);
            }
        }
        else    //链接断了
        {
            fcntl(socket_fd, F_GETFL, flag|O_NONBLOCK);
            if((connect(socket_fd, (struct sockaddr *)&ip_addr, sizeof(ip_addr))) < 0)
            {
                M_print_err(errno);
            }
        }
	}

    
    //pthread_join(id_heartbeat, NULL);
    //M_print_time_str("pthread heartbeat exit !");
    //pthread_join(id_db, NULL);
    //M_print_time_str("pthread db exit !");
    //pthread_join(id_io, NULL);
    //M_print_time_str("pthread io exit !");
    //if(rs485_flag1)
    //{
    //    pthread_join(id_rs485_one, NULL);
    //    M_print_time_str("pthread rs485_one exit !");
    //}
    //if(rs485_flag2)
    //{
    //    pthread_join(id_rs485_two, NULL);
    //    M_print_time_str("pthread rs485_two exit !");
    //}
    pthread_join(id_led, NULL);
    //mysql_library_end();
    return 1;
}

