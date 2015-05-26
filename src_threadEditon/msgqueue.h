#include"print.h"

#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#ifndef MSGPATH_H
#define MSGPATH_H
#define MSGPATH "./parm"
#endif

typedef struct           //构建消息队列需要的参数
{
	key_t key;
	int id_msg;
	//unsigned char path_msg[sizeof(MSGPATH)];               //前两个参数是赋值传出去的，后一个是传进来的
    unsigned char path_id;
}s_msginfo;
//
//传递给数据库线程的参数类型
#define LETGO 'Y'
#define FORBIDGO 'N'
typedef struct
{
    unsigned int card_no;
    time_t time;
    char *forbid_reason;
    unsigned char head_no;          
    unsigned char letgo_flag;         //'Y'          'N'
}s_recordMSG;
typedef struct
{
    unsigned int card_no;
    time_t time;
    char recv_buf[64];
    char snd_buf[64];
    char *deal_reason;
    unsigned char head_no;
    unsigned char deal_type;          //'f'format    'u'update   'n'null 
    unsigned char deal_result;        //'s'sucess     'f'fail
}s_logMSG;
typedef struct
{
    char state;                       //门开或者门关
    time_t t;
}s_stateMSG;
typedef struct
{
    char str_sql[256];
}s_sqlMSG;
//typedef struct
//{
//	long mtype;
//    unsigned int card_no;
//    //unsigned char time[20];
//    time_t time;                          //rs485将t传过来，由db线程转化正具体格式时
//    unsigned char head_no;
//    //char *reason;              //指向常量所以用指针 讲两个日志内容一起传到消息队列的话，reason就要分两个了
//    //record table
//    char *forbid_reason;
//    unsigned char letgo_flag;
//    //log table
//    unsigned char recv_buf[64];           //hex 数据
//    unsigned char snd_buf[64];
//    unsigned char deal_type;
//    unsigned char deal_result;
//    char *deal_reason;
//}s_dbMSG;
//#define OPT_RECORD_TABLE 0
//#define OPT_LOG_TABLE 1
//#define OPT_TWO_TABLE 2

//传递给数据库线程的消息队列结构体
typedef struct
{
    long mtype;
    char buf[512];
}s_dbMSG;
//s_dbMSG中mtype取值
#define OPT_LOG_TABLE 1
#define OPT_RECORD_TABLE 2
#define OPT_OTHER_TABLE 3
#define OPT_STATE_TABLE 4


//传递给io线程的消息队列结构体
typedef struct
{
	long mtype;                  //值为1
	unsigned char io_switch;
    time_t t;                    //当io线程取得消息是对比时间，如果时间差了规定半分钟则视为无线消息
}s_ioMSG;
#define OPENDOOR 1
#define CLOSEDOOR 2

int msgqueue_create(s_msginfo *msginfo);
#endif
