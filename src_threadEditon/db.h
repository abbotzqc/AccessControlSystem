//
//#ifndef PRINTF_H
//#define PRINTF_H
//#include<time.h>
//#define M_print_err(errno) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)
//#define M_print_str(str)   do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}while(0)
//#endif
//
//#define M_print_mysql(conn) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %u__%s\n", __func__, __LINE__, mysql_errno(conn),mysql_error(conn));}while(0)
//
////#define DEBUG
//#ifdef DEBUG 
//#define D_M_PRINT(integret, str) printf("debug usage %s : %d fail -- integret:%d  str:%s\n", __func__, __LINE__, integret, str)
//#else
//#define D_M_PRINT(integret, str)
//#endif
//

#include"print.h"

#ifndef DB_H
#define DB_H
#include<mysql/mysql.h>

typedef struct
{
	char user[12];
	char password[24];
	char db_schema[24];    //库名
	int id_dbmsg;                //消息队列ID
    unsigned int door_num;       //不是send_addr，而是门编号
}s_db_parm;


#define STATE_TABLE 6
#define LOG_TABLE 5
#define RECORD_TABEL 4

#define N_TABLE 7
/*
#define RECORD_FIELD "id int unsigned auto_increment primary key not NULL, card_id int unsigned , head_no tinyint unsigned not NULL, letgo_flag tinyint not NULL, forbid_reason tinyint unsigned, time datetime not NULL, foreign key(card_id) references on public_card_table(id), foreign key(forbid_reason) references on public_reason_table(id)"
#define LOG_FIELD "id int unsigned auto_increment primary key not NULL, card_id int unsigned , head_no tinyint unsigned not NULL, deal_type tinyint unsigned not NULL, recv_buf varchar(255), snd_buf varchar(255), deal_result tinyint not NULL, deal_reason tinyint unsigned, time datetime not NULL, foreign key(card_id) references on public_card_table(id), foreign key(deal_reason) references on public_reason_table(id)"
*/
#define RECORD_FIELD "id int unsigned auto_increment primary key not NULL, card_no int unsigned , pos_no tinyint unsigned not NULL, letgo_flag char(1), forbid_reason_id tinyint unsigned, time datetime not NULL"//, foreign key(forbid_reason) references on public_reason_table(id)"
#define LOG_FIELD "id int unsigned auto_increment primary key not NULL, card_no int unsigned , pos_no tinyint unsigned not NULL, deal_type varchar(10), recv_buf varchar(255), snd_buf varchar(255), deal_result varchar(10), deal_reason_id tinyint unsigned, time datetime not NULL"//, foreign key(deal_reason) references on public_reason_table(id)"
 
#define STATE_FIELD "门编号 tinyint unsigned auto_increment primary key not NULL, 系统启动时间 datetime, 门状态 varchar(45), 门状态改变时间 datetime"

void *pthread_db(void *arg);

#endif


