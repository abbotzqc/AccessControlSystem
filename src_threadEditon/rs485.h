//
//#ifndef PRINTF_H
//#define PRINTF_H
//#include<time.h>
//#define M_print_err(errno) do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)
//#define M_print_str(str)   do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}while(0)
//#endif
//

#include"print.h"

#ifndef RS485_H
#define RS485_H
#define N_buf 1024              //接收数据缓冲区大小    

typedef struct
{
	unsigned short send_addr;
    unsigned short recv_addr;          //head_no           改进：如果以后连多个pos机，应该从心跳帧中去获取
	int id_dbmsg;
    int id_iomsg;
	char dir[12];                 //485路径 /dev/ttyXXX
	unsigned char codetype;
    char user[11];
    char password[24];
    char db_schema[24];
    char no;                      //485编号
    //unsigned char pos_type;
}s_rs485_parm;

void *pthread_rs485(void *arg);
#endif




