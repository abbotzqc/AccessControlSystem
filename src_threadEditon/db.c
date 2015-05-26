#include<stdio.h>
#include"db.h"
#include"msgqueue.h"
#include<errno.h>
#include<strings.h>
#include<stdlib.h>
#include"getdate.h"
#include<string.h>

extern unsigned char door_num;
extern unsigned char *SoftWare_Editon;

static int get_reason_id(MYSQL *conn,char *reason)
{
    int ret;
    unsigned int value;
   // MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char str_sql[512];
    //bzero(str_sql, sizeof(str_sql));
    sprintf(str_sql, "select id from %s where reason=\"%s\"", "public_reason_table" ,reason);
    ret = mysql_query(conn, str_sql);
    if(ret)
    {
        M_print_mysql(conn);
        return -1;//执行失败
    }
    result = mysql_store_result(conn);
    if(result == NULL)
    {
        M_print_mysql(conn);
        //return;
        mysql_free_result(result);
        return -1;            //没有相应原因的id,或者执行失败
    }
    else
    {
        row = mysql_fetch_row(result);
        if(row == NULL)
        {
            M_print_mysql(conn);
            mysql_free_result(result);
            return -1;
        }
        value = atoi(row[0]);
        mysql_free_result(result);
        return value;              //这一行的第一列就是所需要的
    }
}

void *pthread_db(void *arg)
{
    M_print_time_str("pthread database START !");
	if(arg == NULL)
	{
		M_print_str("no parms passed in");
		exit(0);
	}
	
	s_db_parm parm;
    parm = *(s_db_parm *)arg;
	int ret;
    char str_sql[512]={};
    char datetime[20]={};
	s_dbMSG db_msgbuf;

	//MYSQL conn_init;
    MYSQL *conn;
    MYSQL_RES *result;
    //MYSQL_ROW row;
    unsigned int deal_reason_id, forbid_reason_id;

    s_logMSG logmsg;
    s_recordMSG recordmsg;
    s_stateMSG statemsg;
    s_sqlMSG sqlmsg;
    //char deal_type[10];
    char *deal_type;
    //char deal_result[10];
    char *deal_result;
    mysql_thread_init();
    //conn = mysql_init(conn);
    if((conn = mysql_init(conn)) == NULL)
    {
        //M_print_mysql(conn);
        mysql_thread_end();
        exit(0);
    }
    //mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"your_prog_name");
	if((conn=mysql_real_connect(conn, "localhost", parm.user, parm.password, parm.db_schema, 0, NULL, 0)) == NULL)
    {
        M_print_mysql(conn);
	    if(mysql_real_connect(conn, "localhost", parm.user, parm.password, NULL, 0, NULL, 0) == NULL)
        {
            M_print_mysql(conn);
            bzero(str_sql, sizeof(str_sql));
            sprintf(str_sql, "create database %s", parm.db_schema);
            if(mysql_query(conn, str_sql) != 0)             //创建库 没有被服务器接受
            {
               M_print_mysql(conn);
               mysql_thread_end();
               exit(0);
            }
        }
    }
    M_print_str("pthread db connect mysql OK !");
    int i;
    char str_table[N_TABLE][24] = {"public_card_table","public_cardinfo_table","public_reason_table"};
    sprintf(str_table[3], "door%d_limit_table", parm.door_num);
    sprintf(str_table[4], "door%d_record_table", parm.door_num);
    sprintf(str_table[5], "door%d_log_table", parm.door_num);
    sprintf(str_table[6], "door%d_state_table", parm.door_num);

    char *str_field[N_TABLE] = {};
    str_field[4] = RECORD_FIELD;
    str_field[5] = LOG_FIELD;
    str_field[6] = STATE_FIELD;
    //查看各表是否存在,公用表不存在的话退出，私有表不存在创建
    for(i=0;i<4;i++)                //信号量限制一下这个库的读写？防止另一个进程互相干扰？
    {
        bzero(str_sql, sizeof(str_sql));
        sprintf(str_sql, "select * from %s", str_table[i]);
        ret = mysql_query(conn, str_sql);
        if(ret != 0)
        {
            M_print_mysql(conn);
            mysql_thread_end();
            exit(0);
        }
        result = mysql_store_result(conn);
        if(result == NULL)  //表不存在退出
        {
            M_print_mysql(conn);
            mysql_free_result(result);
            mysql_thread_end();
            exit(0);
        }
        mysql_free_result(result);
    }
    for(i=4;i<N_TABLE;i++)
    {
        bzero(str_sql,sizeof(str_sql));
        sprintf(str_sql, "create table %s (%s)", str_table[i], str_field[i]);
        ret = mysql_query(conn, str_sql);
        if(ret != 0)
        {//说明存在 或者没有被服务器接受
            M_print_mysql(conn);
        }
    }
    getdate_unknow_t(datetime);
    bzero(str_sql,sizeof(str_sql));
    sprintf(str_sql, "update door%d_state_table set 门编号=%d, 系统启动时间='%s', 门状态='', 门状态改变时间='',软件版本='%s' where id=1", door_num, door_num, datetime, SoftWare_Editon);
    ret = mysql_query(conn, str_sql);
    M_print_str("pthread db checked all table OK !");
    
	while(1)
	{
   		ret = msgrcv(parm.id_dbmsg, &db_msgbuf, sizeof(db_msgbuf)-sizeof(long), 0, 0);
        if(ret < 0)
        {
            M_print_err(errno);
            continue;
        }
        //插入数据库
        switch(db_msgbuf.mtype)
        {
            case OPT_STATE_TABLE:
            memcpy(&statemsg, db_msgbuf.buf, sizeof(statemsg));
            bzero(str_sql,sizeof(str_sql));
            getdate_know_t(statemsg.t, datetime);
            sprintf(str_sql, "update door%d_state_table set 门编号=%d, 门状态='%s', 门状态改变时间='%s' where 门编号=%d", door_num, door_num, (statemsg.state==OPENDOOR?"开":"关"), datetime, door_num);
            mysql_query(conn, str_sql);
            break;
            case OPT_LOG_TABLE:
            memcpy(&logmsg, db_msgbuf.buf, sizeof(logmsg));
            switch(logmsg.deal_type)
            {
                case 'f':
                //strcpy(deal_type, "FORMAT");
                deal_type = "FORMAT";
                break;
                case 'u':
                //strcpy(deal_type, "UPDATE");
                deal_type = "UPDATE";
                break;
                default:
                //bzero(deal_type, sizeof(deal_type));
                deal_type = "";
                break;
            }
            switch(logmsg.deal_result)
            {
                case 's':
                //strcpy(deal_result, "SUCCESS");
                deal_result = "SUCCESS";
                break;
                case 'f':
                //strcpy(deal_result, "FAIL");
                deal_result = "FAIL";
                break;
                default:
                //bzero(deal_result, sizeof(deal_result));
                deal_result = "";
                break;
            }
            ret = get_reason_id(conn,logmsg.deal_reason);
            if(ret >0)
            {
                deal_reason_id = ret;
            }
            getdate_know_t(logmsg.time, datetime);
            bzero(str_sql,sizeof(str_sql));
            sprintf(str_sql, "insert into %s values(NULL,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,\"%s\")", str_table[LOG_TABLE], logmsg.card_no, logmsg.head_no, deal_type, logmsg.recv_buf, logmsg.snd_buf, deal_result, deal_reason_id, datetime);
            mysql_query(conn, str_sql);
            break;
            case OPT_RECORD_TABLE:
            memcpy(&recordmsg, db_msgbuf.buf, sizeof(recordmsg));
            ret = get_reason_id(conn,recordmsg.forbid_reason);
            if(ret > 0)
            {
                forbid_reason_id = ret;
            }
            getdate_know_t(recordmsg.time, datetime);
            bzero(str_sql,sizeof(str_sql));
            sprintf(str_sql, "insert into %s values(NULL,%d,%d,\"%c\",%d,\"%s\")",str_table[RECORD_TABEL], recordmsg.card_no, recordmsg.head_no, recordmsg.letgo_flag, forbid_reason_id, datetime);
            mysql_query(conn, str_sql);
            break;
            case OPT_OTHER_TABLE:
            memcpy(&sqlmsg, db_msgbuf.buf, sizeof(sqlmsg));
            mysql_query(conn, sqlmsg.str_sql);
            break;
        }
        
	}

    mysql_thread_end();
	exit(0);
}
//开门时间插入有问题，现在的开门时间是传给io线程的时间，实际开门时间应该有io线程修改

//void main()
//{
//    return;
//}
