/*************************************************************************
	> File Name: comm_proto.c
	> Author: 
	> Mail: 
	> Created Time: Wed 18 Mar 2015 10:14:14 AM CST
    > Description : Communication protocal
 ************************************************************************/
#include"comm_proto.h"
#include<stdio.h>
#include<regex.h>
#include<sys/types.h>
#include"CRCcheck.h"
#include"my_regexe.h"
#include<string.h>
#include<stdlib.h>

int byte[ELEMAX] = {
   BYTE_ELE0,
   BYTE_ELE1,
   BYTE_ELE2,
   BYTE_ELE3,
   BYTE_ELE4,
   BYTE_ELE5,
   BYTE_ELE6,
   BYTE_ELE7,
   BYTE_ELE8,
   BYTE_ELE9,
   BYTE_ELE10
};

/*
* 功能：构造某元素的一组数据
* 参数：data----卡片数据区数据结构体   type----写卡类型    elementdata----元素数据结构体
* 返回值：-1----失败
* */

int construct_elementdata9(s_card_datazone *data, unsigned char type, s_elementdata *elementdata)
{
    unsigned short a;
    elementdata->element = ELE9;
    elementdata->elementdatacontext[0] = type;
    char *databuf;
    databuf = elementdata->elementdatacontext+1;
    memcpy(databuf, &data->flow_num, sizeof(data->flow_num));
    *(databuf+4) = data->abnormal_flag<<4;
    *(databuf+4) &= 0x80;
    *(databuf+5) = 0;
    if(data->device_no > 0xfff)
    {
        M_print_str("device number overflow");
        return -1;
    }
    a = data->device_type+data->device_no;
    *(databuf+6) = (a&0xff00)>>8;
    *(databuf+7) = a&0xff;
    memcpy(databuf+8, data->time, sizeof(data->time)/sizeof(data->time[0]));
    *(databuf+13) = data->deal_reason;
    *(databuf+14) = 0x86;
    *(databuf+15) = crc8((unsigned char*)databuf, 15, 0);
    return 1;
}
/*
 * 参数：beep --- 蜂鸣次数 ELE9_BEEP_OK/ELE9_BEEP_ALERT   elementdata---同上
 * */
int construct_elementdata10(unsigned char beep ,s_elementdata *elementdata)
{
    elementdata->element = ELE10;
    elementdata->elementdatacontext[0] = beep;
    return 1;
}
//
///*
//* 功能： 构造卡片数据取内容
//* 参数：data----卡片数据区数据结构体    databuf----接收构造出的16字节数据区数据
//* 返回值：-1----失败
//* */
//int construct_datazone(s_card_datazone *data, unsigned char databuf[])
//{
//    unsigned short a;
//    memcpy(databuf, &data->flow_num, sizeof(data->flow_num));
//    *(databuf+4) = data->abnormal_flag<<4;
//    *(databuf+4) &= 0x80;
//    *(databuf+5) = 0;
//    if(data->device_no > 0xfff)
//    {
//        M_print_str("device number overflow");
//        return -1;
//    }
//    a = data->device_type+data->device_no;
//    memcpy(databuf+6, &a, sizeof(data->device_no));
//    memcpy(databuf+8, data->time, sizeof(data->time)/sizeof(data->time[0]));
//    memcpy(databuf+13, &data->deal_reason, sizeof(data->deal_reason));
//    *(databuf+14) = 0x86;
//    *(databuf+15) = crc8(databuf, 15, 0);
//    return 1;
//}

unsigned int get_card_no(char *databuf)
{
    char buf[4] = {};
    memcpy(buf, databuf, 4);
    return (buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3]);
}
//获得卡密码
unsigned char get_key_type(char *databuf)
{
    return databuf[4];
}
//获得流水号
unsigned int get_flow_data(char *databuf)
{
    char buf[4] = {};
    memcpy(buf, databuf+5, 4);
    return (buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3]);
}
//获得卡异常标志位数
unsigned char get_sta_data(char *databuf)
{
    return databuf[9];
}




static int colulate_ele_len(unsigned char element)
{
    int len;
    switch(element)
    {
        case ELE0:
        len = BYTE_ELE0;
        break;
        case ELE1:
        len = BYTE_ELE1;
        break;
        case ELE2:
        len = BYTE_ELE2;
        break;
        case ELE4:
        len = BYTE_ELE4;
        break;
        case ELE5:
        len = BYTE_ELE5;
        break;
        case ELE6:
        len = BYTE_ELE6;
        break;
        case ELE7:
        len = BYTE_ELE7;
        break;
        case ELE8:
        len = BYTE_ELE8;
        break;
        case ELE9:
        len = BYTE_ELE9;
        break;
        case ELE10:
        len = BYTE_ELE10;
        default:
        return -1;
        break;
    }
    return len;
}
/*
* 功能：构造数据
* 参数：sndfrminfo----需要发送的数据结构体   sndbuf----发送数据数组缓冲区    codetype----采用的数据格式
* 返回值：>0----返回数据长度    0----错误
* 改进：可以malloc数据区长内存，再添加一个释放函数，让调用者用完sndbuf结果后调用释放函数将其释放
* */
unsigned int constructFRM(const s_sndfrminfo *sndfrminfo, char *sndbuf, unsigned char codetype)// s_elementdata *elementdata, unsigned int elementdata_num)
{
    char databuf[268];                 //512+12        255--数据区最长长度（hex），12--固定区域长度
    int i, len;
    char *plen, *buf;
    //unsigned char element;
    //int ele_grp_len;//元素组长度
    //int ele_grp_cnt;//元素组个数
    plen = NULL;
    buf = databuf;

    *buf++ = FRM_HEAD;//头
    *buf++ = (sndfrminfo->send_addr&0xff00)>>8;
    *buf++ = sndfrminfo->send_addr&0xff;
    *buf++ = (sndfrminfo->recv_addr&0xff00)>>8;
    *buf++ = sndfrminfo->recv_addr&0xff;
    *buf++ = FRM_HEAD;//头

    //ele_grp_len = 0;
    //ele_grp_cnt = sndfrminfo->elementdata_num;//元素组个数
    len = 0 ;//数据长度

    switch(sndfrminfo->afn)
    {
        case AFN_TEST:
        break;
        case AFN_SETSINGPARM:
        *buf++ = 0x41;                  //控制域，有待用函数取代
        *buf++ = sndfrminfo->seq;
        plen = buf++;
        *buf++ = AFN_SETSINGPARM;
        for(i = 0; i < sndfrminfo->elementdata_num; i++)        //求数据长度
        {
            //element = sndfrminfo->elementdata[i].element;
            //ele_grp_len = colulate_ele_len(element);
            //len += ele_grp_len;
            //复制数据
            //*buf++ = element;
            //memcpy(buf, sndfrminfo->elementdata[i].elementdatacontext, ele_grp_len);
            //buf += colulate_ele_len(ele_grp_len); 
            len += byte[sndfrminfo->elementdata[i].element];
            *buf++ = sndfrminfo->elementdata[i].element;
            memcpy(buf, sndfrminfo->elementdata[i].elementdatacontext, byte[sndfrminfo->elementdata[i].element]);
            buf += byte[sndfrminfo->elementdata[i].element];
        }
        *plen = len;
        break;
        case AFN_REQPARM:
        *buf++ = 0x43;                  //控制域，有待用函数取代
        *buf++ = sndfrminfo->seq;
        plen = buf++;
        *buf++ = AFN_REQPARM;
        for(i = 0; i < sndfrminfo->elementdata_num; i++)        //求数据长度
        {
            //复制数据
            len += byte[sndfrminfo->elementdata[i].element];
            *buf++ = sndfrminfo->elementdata[i].element;
            memcpy(buf, sndfrminfo->elementdata[i].elementdatacontext, byte[sndfrminfo->elementdata[i].element]);
            buf += byte[sndfrminfo->elementdata[i].element];
        }
        *plen = len;
        break;
        case AFN_HEARTBEAT:
        *buf++ = 0xc0;                  //控制域，有待用函数取代
        *buf++ = sndfrminfo->seq;
        plen = buf++;
        *buf++ = AFN_HEARTBEAT;
        for(i = 0; i < sndfrminfo->elementdata_num; i++)        //求数据长度
        {
            //复制数据
            len += byte[sndfrminfo->elementdata[i].element];
            *buf++ = sndfrminfo->elementdata[i].element;
            memcpy(buf, sndfrminfo->elementdata[i].elementdatacontext, byte[sndfrminfo->elementdata[i].element]);
            buf += byte[sndfrminfo->elementdata[i].element];
        }
        *plen = len;
        break;
        default:
        return -1;
        break;
    }
    *buf++ = FRM_TAIL;
    *buf++ = crc8((unsigned char*)databuf, buf-databuf, 0);
    if(codetype == CODETYP_HEX)
    {
        memcpy(sndbuf, databuf, buf-databuf);
        return (buf-databuf);
    }
    else
    {
        for(i=0; i<buf-databuf; i++)
        {
            sprintf(sndbuf+(i<<1), "%02x", *(databuf+i));
        }
        return ((buf-databuf)<<1);
    }
}

/*
* parm : buf ---- the string who used to pick out from 用于找出所需信息的字符串,是hex或者ascii
* 参数：recvfrminfo----需要验证的信息     recvretarg----接收元素用的    buf----指向接收缓冲区  bufsize----缓冲区总长   lenbuf----接收缓冲区中数据的长度
*
* return : -1 ---- find false 找着了但是不对   0 ---- find NULL 没找着  1 ---- find   buf ---- point
* to the end of pattern
*
* 功能：解析接收的数据，讲数据提取出来，并且结束后把指向接收缓冲区起始地址的指针指向下一次接收数据需要放到缓冲区的地址位置,如果是要返回recvretreq类型的参数，则会在堆上申请分配内存，记得用完后调用analyzeFRM_release释放内存
*
* */

int analyzeFRM(s_recvfrminfo *recvfrminfo, void *recvretarg, char **buf, unsigned int bufsize, unsigned int lenbuf, unsigned char codetype)
{
    unsigned int len, i;
    char *endptr;

    s_reginfo reginfo;
    regmatch_t match[NMATCH+1];
    reginfo.pmatch = match;

	int frm_len;
    char recvbuf[((N_BUFDATA+FRM_BASE_LEN)<<1)+1] = {};//将hex数据转成ascii数据用的存储缓冲区
    char *bufdata;
    D_M_PRINT(recvfrminfo->afn, "analyzeFRM in");

    switch(recvfrminfo->afn)
    {
        case AFN_SETSINGPARM:
        reginfo.pattern = REG_SETSINGPARM_RPL;
        break;
        case AFN_REQPARM:
        reginfo.pattern = REG_REQ_RPL;
        break;
		case AFN_HEARTBEAT:
		//??
		reginfo.pattern = REG_HEARTBEAT;
		break;
        default:
        return -1;
        break;
    }
    reginfo.nmatch = NMATCH;
    reginfo.cflag = 0;

    e_regFRM_ret ret;

    D_M_PRINT(codetype, "begin to find");

    if(codetype == CODETYP_ASCII)
    {
        char str[4]={};
        reginfo.regbuf = *buf;
        
        ret = regFRM(&reginfo);

        switch(ret)
        {
            case REG_RET_FAIL://没有匹配项
			*buf = *buf+lenbuf;
            return 0;
            break;
            case REG_RET_SUCCESS:
        	frm_len = reginfo.pmatch[0].rm_eo - reginfo.pmatch[0].rm_so;      //匹配到的整个帧长
	        //提取seq与本地进行seq比较
			memcpy(str, *buf+reginfo.pmatch[PMATCH_SEQ].rm_so, reginfo.pmatch[PMATCH_SEQ].rm_eo-reginfo.pmatch[PMATCH_SEQ].rm_so);
			if(recvfrminfo->afn != AFN_HEARTBEAT)        //??
			{
				if(recvfrminfo->seq != strtoul(str, &endptr, 16))
				{
                    D_M_PRINT(0, "seq is unequal");
					//找到了，但不是本次通讯的，应该从缓冲区舍弃
                    bzero(*buf, reginfo.pmatch[0].rm_eo>>codetype);
					memmove(*buf, *buf+reginfo.pmatch[0].rm_eo, bufsize-reginfo.pmatch[0].rm_eo);
	                *buf += (lenbuf-(reginfo.pmatch[0].rm_eo>>codetype));
					return -1;
				}
			}
            //验证crc
            bzero(str, sizeof(str)/sizeof(str[0]));
            memcpy(str, *buf+reginfo.pmatch[PMATCH_LEN].rm_so, reginfo.pmatch[PMATCH_LEN].rm_eo-reginfo.pmatch[PMATCH_LEN].rm_so);
            len = strtoul(str, &endptr, 16);
            for(i=0; i<(lenbuf>>1); i++) //ascii形式转换位hex形式
            {
                bzero(str, sizeof(str)/sizeof(str[0]));
                memcpy(str, *buf+(i<<1), 2);
                recvbuf[i] = strtoul(str, &endptr, 16);
            }
            bufdata = recvbuf + ((reginfo.pmatch[PMATCH_DATA].rm_so)>>1);
            bzero(str, sizeof(str)/sizeof(str[0]));
            memcpy(str, *buf+reginfo.pmatch[PMATCH_CRC].rm_so, reginfo.pmatch[PMATCH_CRC].rm_eo-reginfo.pmatch[PMATCH_CRC].rm_so);
            if(crc8((unsigned char*)recvbuf, (frm_len>>1)-1, 0) != strtoul(str, &endptr, 16))
            {
                D_M_PRINT((int)strtoul(str, &endptr, 16), "is receive crc number, so crc wrong");
                bzero(*buf, reginfo.pmatch[0].rm_eo>>codetype);
				memmove(*buf, *buf+reginfo.pmatch[0].rm_eo, bufsize-reginfo.pmatch[0].rm_eo);
                D_M_PRINT(strlen(*buf), *buf);
	            *buf += (lenbuf-(reginfo.pmatch[0].rm_eo>>codetype));
                return -1;
            }
            break;
            default:
            return -1;
            break;
        }
    }
    else //CODETYP_HEX
    {
        for(i=0; i<lenbuf; i++)
        {
            sprintf(recvbuf+(i<<1),"%02x",*(*buf+i));
        }
        reginfo.regbuf = recvbuf;
        ret = regFRM(&reginfo);
        switch(ret)
        {
            case REG_RET_FAIL://没有匹配项
			*buf = *buf+lenbuf;
            return 0;
            break;
            case REG_RET_SUCCESS:
        	frm_len = (reginfo.pmatch[0].rm_eo - reginfo.pmatch[0].rm_so)>>1;
			if(recvfrminfo->afn != AFN_HEARTBEAT)        //??
			{
				if(recvfrminfo->seq != *(*buf+(reginfo.pmatch[PMATCH_SEQ].rm_so>>1)))
				{
                    D_M_PRINT(0, "seq is unequal");
					//找到了，但不是本次通讯的，应该从缓冲区舍弃
                    bzero(*buf, reginfo.pmatch[0].rm_eo>>codetype);
					memmove(*buf, *buf+((reginfo.pmatch[0].rm_eo)>>1) , bufsize-((reginfo.pmatch[0].rm_eo)>>1));
	                *buf += (lenbuf-(reginfo.pmatch[0].rm_eo>>codetype));
					return -1;
				}
			}
            len = *(*buf+(reginfo.pmatch[PMATCH_LEN].rm_so>>1));
            if(crc8((unsigned char*)*buf, frm_len-1, 0) != *(*buf+(reginfo.pmatch[PMATCH_CRC].rm_so>>1)))
            {
                D_M_PRINT(*(*buf+(reginfo.pmatch[PMATCH_CRC].rm_so>>1)), "is receive crc number");
                bzero(*buf, reginfo.pmatch[0].rm_eo>>codetype);
				memmove(*buf, *buf+((reginfo.pmatch[0].rm_eo)>>1), bufsize-((reginfo.pmatch[0].rm_eo)>>1));
	            *buf += (lenbuf-(reginfo.pmatch[0].rm_eo>>codetype));
                return -1;
            }
            break;
            default:
            return -1;
            break;
        }
    }
    switch(recvfrminfo->afn)//数据区内容填进参数中
    {
        case AFN_SETSINGPARM:
        {
            s_recvretset *recvretset = (s_recvretset *)recvretarg;
            if(len == 0)
            {
                recvretset->result = 1;
                recvretset->errornum = 0;
            }
            else
            {
                recvretset->result = 0;
                if(codetype == CODETYP_HEX)
                {
                    recvretset->errornum = *(*buf+(reginfo.pmatch[PMATCH_DATA].rm_so>>1));
                }
                else
                {
                    recvretset->errornum = *(bufdata+(reginfo.pmatch[PMATCH_DATA].rm_so>>1));
                }
            }
            break;
        }
        case AFN_REQPARM:
        case AFN_HEARTBEAT:
        {
            s_recvretreq *recvretreq = (s_recvretreq *)recvretarg;
            unsigned int p, ele_no;//元素编号 
            if(codetype == CODETYP_HEX)
            {
                for(i=0,p=(reginfo.pmatch[PMATCH_DATA].rm_so>>1); p<(reginfo.pmatch[PMATCH_DATA].rm_eo>>1); i++)      //p为相对于buf的偏移
                {
                    ele_no = *(*buf+p);
                    p++;
                    p += byte[ele_no];
                }
                recvretreq->elementdata_num=i;
                recvretreq->elementdata = (s_elementdata*)malloc(sizeof(s_elementdata)*(i+1));
                for(i=0,p=(reginfo.pmatch[PMATCH_DATA].rm_so>>1); p<(reginfo.pmatch[PMATCH_DATA].rm_eo>>1); i++)      //p为相对于buf的偏移
                {
                    ele_no = *(*buf+p);
                    recvretreq->elementdata[i].element = ele_no;
                    p++;
                    //bzero(recvretreq->elementdata[i].elementdatacontext, BYTE_MAX);
                    memcpy(recvretreq->elementdata[i].elementdatacontext, *buf+p, byte[ele_no]);
                    p += byte[ele_no];
                }
            }
            else
            {
                for(i=0,p=0; p<(reginfo.pmatch[PMATCH_DATA].rm_eo-reginfo.pmatch[PMATCH_DATA].rm_so)>>1; i++)      //p为相对于bufdata的偏移
                {
                    ele_no = *(bufdata+p);
                    p++;
                    p += byte[ele_no];
                }
                recvretreq->elementdata_num=i;
                recvretreq->elementdata = (s_elementdata*)malloc(sizeof(s_elementdata)*(i+1));
                for(i=0,p=0; p<(reginfo.pmatch[PMATCH_DATA].rm_eo-reginfo.pmatch[PMATCH_DATA].rm_so)>>1; i++)      //p为相对于bufdata的偏移
                {
                    ele_no = *(bufdata+p);
                    recvretreq->elementdata[i].element = ele_no;
                    p++;
                    //bzero(recvretreq->elementdata[i].elementdatacontext, BYTE_MAX);
                    memcpy(recvretreq->elementdata[i].elementdatacontext, bufdata+p, byte[ele_no]);
                    p += byte[ele_no];
                }
            }
            break;
        }
        default:
        return -1;
        break;
    }
    bzero(*buf, reginfo.pmatch[0].rm_eo>>codetype);
	memmove(*buf, *buf+((reginfo.pmatch[0].rm_eo>>codetype)), bufsize-(reginfo.pmatch[0].rm_eo>>codetype));
	*buf += (lenbuf-(reginfo.pmatch[0].rm_eo>>codetype));
    return 1;
}

void analyzeFRM_release(s_recvretreq *p)
{
    free(p->elementdata);
    return;
}

/*
void main()
{
    
    return;
}
*/
