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

#include"print.h"
#ifndef COMM_PROTO_H
#define COMM_PROTO_H

#define FRM_HEAD 0x68
#define FRM_TAIL 0x06

#define AFN_TEST        0x01
//#define AFN_SETCOMPPARM 
#define AFN_SETSINGPARM 0x02
#define AFN_REQPARM     0x03
#define AFN_HEARTBEAT   0x04
/*
#define C_DIR_DOWN      0
#define C_DIR_UP        1
#define C_DIR_BIT       7

#define C_PRM_ACTIVE    1
#define C_PRM_NEGTIVE   0
#define C_PRM_BIT       6

#define C_DIV_SING      0
#define C_DIV_MULT      1      
#define C_DIV_BIT       5

#define C_RLY_INITIAL   0
#define C_PLY_TRANS     1
#define C_PLY_BIT       4

//C_FUN_ACTIVE_DOWN
#define C_FUN_ACTIVE_DOWN_SND_TEST       000
#define C_FUN_ACTIVE_DOWN_SND_SETPARM    001
#define C_FUN_ACTIVE_DOWN_VERT_SETPARM   001
#define C_FUN_ACTIVE_DOWN_REQSING       010
#define C_FUN_ACTIVE_DOWN_RPLSING       010
#define C_FUN_ACTIVE_DOWN_REQMULT       011
#define C_FUN_ACTIVE_DOWN_RPLMULT       011
//C_FUN_ACTIVE_UP
#define C_FUN_ACTIVE_UP_SND_HEARTBEAT  000
//C_FUN_NEGTIVE_UP
#define C_FUN_NEGTIVE_UP_VERT           000
#define C_FUN_NEGTIVE_UP_VERT_DENY      001
#define C_FUN_NEGTIVE_UP_RPL            100
#define C_FUN_NEGTIVE_UP_RPL_NULL       101
//C_FUN_NEGTIVE_DOWN
//
#define C_FUN_BIT       0
*/
//frame type
#define TYPE_LINKTEST    0
#define TYPE_SETSINGPARM 1
#define TYPE_REQSING     2
#define TYPE_REQMULT     3
#define TYPE_HEATBEAT    4

//regex
#define PMATCH_SND_ADDR 1
#define PMATCH_RECV_ADDR 2
#define PMATCH_SEQ 3
#define PMATCH_LEN 4
//#define PMATCH_AFN 5
#define PMATCH_DATA 5
#define PMATCH_CRC 6

#define NMATCH  8

#define REG_LINKTEST_RPL "68([0-9a-fA-F]{4})([0-9a-fA-F]{4})6888([0-9a-fA-F]{2})([0-9a-fA-F]{2})01([0-9a-fA-F]{1,})06([0-9a-fA-F]{2})"
#define REG_SETSINGPARM_RPL "68([0-9a-fA-F]{4})([0-9a-fA-F]{4})6880([0-9a-fA-F]{2})(00)02()06([0-9a-fA-F]{2})|68([0-9a-fA-F]{2})([0-9a-fA-F][1-9a-fA-F])6881([0-9a-fA-F]{2})(01)02([0-9a-fA-F]{2})06([0-9a-fA-F]{2})"
#define REG_REQ_RPL "68([0-9a-fA-F]{4})([0-9a-fA-F]{4})6888([0-9a-fA-F]{2})([0-9a-fA-F]{2})03([0-9a-fA-F]{2,})06([0-9a-fA-F]{2})"
//??
#define REG_HEARTBEAT "68([0-9a-fA-F]{4})([0-9a-fA-F]{4})68c0([0-9a-fA-F]{2})([0-9a-fA-F]{2})04([0-9a-fA-F]{2,})06([0-9a-fA-F]{2})"

#define CODETYP_HEX 1
#define CODETYP_ASCII 0
//element data length of byte
typedef enum
{
    ELE0=0,ELE1,ELE2,ELE3,ELE4,ELE5,ELE6,ELE7,ELE8,ELE9,ELE10,ELEMAX
}e_ele_nem;


//#define ELE0    0
//#define ELE1    1
//#define ELE2    2
//#define ELE4    4
//#define ELE5    5
//#define ELE6    6
//#define ELE7    7
//#define ELE8    8
//#define ELE9    9
//#define ELE10   10

#define BYTE_ELE0   1
#define BYTE_ELE1   6
#define BYTE_ELE2   6
#define BYTE_ELE3   0
#define BYTE_ELE4   6
#define BYTE_ELE5   2
#define BYTE_ELE6   2
#define BYTE_ELE7   7
#define BYTE_ELE8   21
#define BYTE_ELE9   17
#define BYTE_ELE10  1

#define BYTE_MAX    21


#define FRM_MIN_LEN 12                     //hex type frame length 
#define FRM_BASE_LEN 10                    //帧固定区域长度

#define N_BUFDATA (BYTE_MAX*ELEMAX)

//macro for reason
#define STR_REASON0 "通行"
#define STR_REASON1 "卡号未登记"
#define STR_REASON2 "此门无通行权限"
#define STR_REASON3 "此卡此时段无权通行"
#define STR_REASON4  "卡号在黑名单内"
#define STR_REASON5  "卡片有异常标志"
#define STR_REASON6  "卡片通行流水号超范围"
#define STR_REASON7  "卡片通行流水号小于本机记录"
#define STR_REASON8  "格式化失败"
#define STR_REASON9  "更新数据失败"
//#define REASON  ""

typedef enum
{REASON0=0, REASON1 , REASON2 , REASON3 , REASON4 , REASON5 , REASON6 , REASON7 , REASON8 , REASON9}e_reason;


/*
typedef union
{
    char all[BYTE_MAX];
    char ele0_undef[BYTE_ELE0]; 
    char ele1_Akey[BYTE_ELE1];
    char ele2_Akey[BYTE_ELE2];
    char ele4_localkey[BYTE_ELE4];
    char ele5_parm[BYTE_ELE5];
    char ele6_addr[BYTE_ELE6];
    char ele7_time[BYTE_ELE7];
    char ele8_read[BYTE_ELE8];
    char ele9_write[BYTE_ELE9];
    char ele10_letgo;
}u_elementdatacontext;*/

typedef struct
{
    unsigned char element;
    //char[BYTE_MAX];
    //u_elementdatacontext *elementdatacontext;
    char elementdatacontext[BYTE_MAX];
}s_elementdata;

typedef struct
{
    unsigned short send_addr;
    unsigned short recv_addr;
    unsigned char direction;
    unsigned char afn;
    unsigned char seq;
    unsigned int elementdata_num;
    s_elementdata *elementdata;         //数组
}s_sndfrminfo;

typedef struct
{
    unsigned short send_addr;
    unsigned short recv_addr;
    unsigned char direction;
    unsigned char afn;
    unsigned char seq;
}s_recvfrminfo;

//typedef struct
//{
//    unsigned short send_addr;
//    unsigned short recv_addr;
//    unsigned char direction;
//    unsigned char afn;
//    unsigned char seq;
//    s_elementdata *elementdata;         //数组
//    unsigned int elementdata_num;
//}s_recvfrminfo;

typedef struct
{
    unsigned int elementdata_num;      //数组个数，也就是元素组个数，由帧解析函数填充   ?这样穿参数是否合法,在解析函数中开辟内存，再写一个释放函数释放内存
    s_elementdata *elementdata;         //数组
}s_recvretreq;

typedef struct
{
    unsigned char result;              //1 -- ok          0 -- sth wrong
    unsigned char errornum;
}s_recvretset;

typedef struct
{
    unsigned int flow_num;
    unsigned char abnormal_flag;
    unsigned char device_type;           //00100000 刷卡读头
    unsigned short device_no;
    char time[5];
    unsigned char deal_reason;
}s_card_datazone;

#define DEV_TYP_POS 0x20            //设备类型：刷卡读头
#define CARD_STA_NOR 0              //卡正常
#define CARD_STA_UNNOR 1            //不正常

#define ELE9_BEEP_OK 0x00           //蜂鸣一声
#define ELE9_BEEP_ALERT 0x80        //蜂鸣三声 报警
#define ELE9_FORMAT 2
#define ELE9_UPDATE 1

int construct_elementdata10(unsigned char beep ,s_elementdata *elementdata);
int construct_elementdata9(s_card_datazone *data, unsigned char type, s_elementdata *elementdata);
unsigned int get_card_no(char *databuf);
unsigned char get_key_type(char *databuf);
unsigned int get_flow_data(char *databuf);
unsigned char get_sta_data(char *databuf);
//void constructFRM(const s_sndfrminfo *sndfrminfo, char *buf, s_elementdata *elementdata, unsigned int elementdata_num);
unsigned int constructFRM(const s_sndfrminfo *sndfrminfo, char *buf, unsigned char codetype);
int analyzeFRM(s_recvfrminfo *recvfrminfo, void *recvretarg, char **buf, unsigned int bufsize, unsigned int lenbuf, unsigned char codetype);
void analyzeFRM_release(s_recvretreq *p);
#endif
