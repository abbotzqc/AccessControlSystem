/*************************************************************************
	> File Name: rs485.c
	> Author: 
	> Mail: 
	> Created Time: Wed 18 Mar 2015 05:30:17 PM CST
 ************************************************************************/
#include"rs485.h"
#include"comm_proto.h"
#include"msgqueue.h"
#include<stdio.h>
#include"serial_init.h"
#include<errno.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<mysql/mysql.h>
#include<time.h>
#include"getdate.h"
#include"db.h"
#include"print.h"

#define RS485_SND //system("echo 1 > /sys/class/misc/io_ctl/gpio_state")
#define RS485_RECV //system("echo 0 > /sys/class/misc/io_ctl/gpio_state")

void *pthread_rs485(void *arg)
{
    if(arg == NULL)	
    {
        M_print_str("no parms passed in !");
        exit(0);
    }
    int fd, ret, retlen, i;
    fd_set rdflags;
    struct timeval wait;
    char buf[N_buf] = {};           //receive buf
    char sndbuf[N_buf] = {};
    char *pbuf = NULL;
    unsigned int recvlen = 0;
    unsigned char seq = 1;
    unsigned int frm_min_len;

    char str_sql[512];

    s_dbMSG msgbuf;
    s_logMSG logmsg;
    s_recordMSG recordmsg;
    s_sqlMSG sqlmsg;
    s_ioMSG iomsg;
    
    unsigned int update_data;
    unsigned int interval;          //时段
    unsigned char format_flag;
    unsigned char ele10_flag;
    unsigned char illegal_flag;
    s_card_datazone card_datazone;

    MYSQL conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    s_elementdata elementdata;
    s_sndfrminfo sndfrminfo;	
    s_recvfrminfo recvfrminfo;
    s_recvretreq recvretreq;	
	s_recvretset recvretset;
    unsigned int card_id, card_no;
   
    s_rs485_parm *parm = (s_rs485_parm *)arg;
    M_print_time_num_str("pthread rs485 START !", "485no", parm->no);
    
    //链接数据库
	if(mysql_init(&conn) == NULL)
    {
        M_print_num_mysql(&conn, "485no", parm->no);
        exit(0);
    }
	if(mysql_real_connect(&conn, "localhost", parm->user, parm->password, parm->db_schema, 0, NULL, 0) == NULL)
    {
        M_print_num_mysql(&conn, "485no", parm->no);
        exit(0);
    }
    M_print_num_str("pthread rs485 connect mysql OK !", "485no", parm->no);

    //开串口
    sndfrminfo.send_addr = parm->send_addr;
    sndfrminfo.recv_addr = parm->recv_addr;
    frm_min_len = FRM_MIN_LEN << (parm->codetype^0x01);

    if(parm->dir == NULL)
    {
        M_print_num_str("no RS485 dir passed in", "485no", parm->no);
        exit(0);
    }
    /* // it is unuseful now
    recvfrminfo.send_addr = parm->send_addr;
    recvfrminfo.recv_addr = parm->recv_addr;
    */
    fd = open(parm->dir, O_RDWR|O_NOCTTY|O_NDELAY|O_NONBLOCK, 0666);
    if(fd < 0)
    {
        M_print_num_err(errno, "485no", parm->no);
        exit(0);
    }
    serial_init(fd, frm_min_len);
    M_print_num_str("pthread rs485 open tty OK !", "485no", parm->no);
    pbuf = buf;
    FD_ZERO(&rdflags);
    FD_SET(fd, &rdflags);
    while(1)
    {
        ret = select(fd+1, &rdflags, NULL, NULL, NULL);
        if(!(ret > 0))
        {
            M_print_num_err(errno, "485no", parm->no);
            FD_ZERO(&rdflags);
            FD_SET(fd, &rdflags);
            continue;
        }
        M_print_time_num_str("something in serial receive buf", "485no", parm->no);
        ret = read(fd, pbuf, sizeof(buf)-(pbuf-buf));
        if(!(ret > 0))
        {
            M_print_num_err(errno, "485no", parm->no);
            continue;
        }
        pbuf += ret;
        recvlen = pbuf-buf;
        if(recvlen < frm_min_len)
        {
            D_M_PRINT(pbuf-buf, "too few for reading from serial receive buf");
            continue;
        }
        recvfrminfo.afn = AFN_HEARTBEAT;
        //?? recvfrminfo.seq = seq; 
        pbuf = buf;
        ret = analyzeFRM(&recvfrminfo, &recvretreq, &pbuf, sizeof(buf), recvlen, parm->codetype);
        if(ret != 1)	
        {
            D_M_PRINT(ret, "analyzeFRM not find correct match context");
            D_M_PRINT(strlen(buf), buf);
            continue;
        }
        for(i=0; i<recvretreq.elementdata_num; i++)
        {
            switch(recvretreq.elementdata[i].element)
            {
                case ELE0: //心跳帧
                break;
                case ELE8:
                card_id = 0;
                card_no = get_card_no(recvretreq.elementdata[i].elementdatacontext);
                //获取卡号后去查询,不能在主函数中登陆打开数据库，每个线程都应该开启自己跟数据库的链接，这样才能对应两个客户端程序被服务器处理
                sprintf(str_sql, "select * from public_card_table where card_no=%d",card_no);
                ret = mysql_query(&conn, str_sql);
                if(ret != 0)
                {
                    M_print_num_mysql(&conn, "485no", parm->no);
                }
                else
                {
                    res = mysql_store_result(&conn);
                    if(res == NULL)
                    {
                        M_print_num_mysql(&conn, "485no", parm->no);
                    }
                    else
                    {
                        if((row = mysql_fetch_row(res)) == NULL)
                        {
                            M_print_num_mysql(&conn, "485no", parm->no);
                        }
                        else
                        {
                            card_id = atoi(row[0]);
                        }
                    }
                    mysql_free_result(res);
                }
                if(card_id == 0)     //卡号未登记
                {   
                    //传递数据库消息
                    recordmsg.forbid_reason = STR_REASON1;
                    recordmsg.letgo_flag = 'N';
                    logmsg.deal_reason = STR_REASON1;
                    logmsg.deal_type = 'n';
                    logmsg.deal_result = 'n';
                    //card_datazone.abnormal_flag = CARD_STA_NOR;
                    //card_datazone.deal_reason = REASON1;
                    construct_elementdata10(ELE9_BEEP_ALERT, &elementdata);
                }
                else
                {
                    if(get_key_type(recvretreq.elementdata[i].elementdatacontext) == 1)
                    {
                        //卡片注册过，查看是否初始化格式化过
                        sprintf(str_sql, "select format_flag from public_cardinfo_table where card_id=%d",card_id);
                        ret = mysql_query(&conn, str_sql);
                        if(ret != 0)
                        {
                            M_print_num_mysql(&conn, "485no", parm->no);
                        }
                        else
                        {
                            res = mysql_store_result(&conn);
                            if(res == NULL)
                            {
                                M_print_num_mysql(&conn, "485no", parm->no);
                            }
                            else
                            {
                                if((row = mysql_fetch_row(res)) == NULL)
                                {
                                    M_print_num_mysql(&conn, "485no", parm->no);
                                }
                                else
                                {
                                    format_flag = atoi(row[0]);
                                }
                            }
                            mysql_free_result(res);
                        }
                        //发送格式化指令
                        card_datazone.flow_num = 1;
                        (format_flag == 1) ? (card_datazone.abnormal_flag = CARD_STA_UNNOR) : (card_datazone.abnormal_flag = CARD_STA_NOR);
                        card_datazone.device_type = DEV_TYP_POS;
                        card_datazone.device_no = parm->recv_addr;
                        getdate_BCD(card_datazone.time);
                        card_datazone.deal_reason = REASON8;
                        construct_elementdata9(&card_datazone, ELE9_FORMAT, &elementdata);
                        sndfrminfo.afn = AFN_SETSINGPARM;
                        (seq == 255) ? (sndfrminfo.seq = seq, seq = 0) : (sndfrminfo.seq = seq++);
                        sndfrminfo.elementdata_num = 1;
                        sndfrminfo.elementdata = &elementdata;
                        if((retlen = constructFRM(&sndfrminfo, sndbuf, parm->codetype)) == 0)
                        {
                            M_print_num_str("constructFRM fail", "485no", parm->no);
                            break;
                        }
                        ret = 0;
                        while(ret < retlen)
                        {
                            ret += write(fd, sndbuf+ret, retlen-ret);
                            if(ret < 0)
                            {
                                M_print_num_err(errno, "485no", parm->no);
                                break;
                            }
                        }
                        //等待读取执行返回结果
                        FD_SET(fd, &rdflags);
                        wait.tv_sec = 0;
                        wait.tv_usec = 5000;                   //500ms
                        ret = select(fd+1, &rdflags, NULL, NULL, &wait);
                        if(ret == 0)       //超时
                        {
                            //格式化无回应 记录结果
                            recordmsg.forbid_reason = STR_REASON8;
                            recordmsg.letgo_flag = 'N';
                            logmsg.deal_reason = STR_REASON8;
                            logmsg.deal_type = 'f';
                            logmsg.deal_result = 'n';
                            //card_datazone.abnormal_flag = CARD_STA_NOR;
                            //card_datazone.deal_reason = REASON8;
                            ele10_flag = ELE9_BEEP_ALERT;
                        }
                        else
                        {
                            //格式化有回应
                            read(fd, pbuf, sizeof(buf)-(pbuf-buf));
                            recvfrminfo.afn = AFN_SETSINGPARM;
                            (seq == 255) ? (recvfrminfo.seq = seq, seq = 0) : (recvfrminfo.seq = seq++);
                            recvlen = pbuf-buf;
                            pbuf = buf;
                            ret = analyzeFRM(&recvfrminfo, &recvretset, &pbuf, sizeof(buf), recvlen, parm->codetype);
                            if(recvretset.result == 1)
                            {
                                //格式化成功
                                iomsg.mtype = 1;
                                iomsg.io_switch = OPENDOOR;
                                iomsg.t = time(NULL);
                                msgsnd(parm->id_iomsg, &iomsg, sizeof(iomsg)-sizeof(long), 0);    //开门

                                recordmsg.forbid_reason = STR_REASON0;
                                recordmsg.letgo_flag = 'Y';
                                logmsg.deal_reason = STR_REASON0;
                                logmsg.deal_type = 'f';
                                logmsg.deal_result = 's';
                                //card_datazone.abnormal_flag = CARD_STA_NOR;
                                //card_datazone.deal_reason = REASON0;
                                ele10_flag = ELE9_BEEP_OK;
                                //更新格式化区 
                                msgbuf.mtype = OPT_OTHER_TABLE;
                                if(format_flag == 1) //格式化过
                                {
                                    sprintf(sqlmsg.str_sql, "update public_cardinfo_table set format_flag='%c' update_data=%d where card_id=%d", 'Y', 0, card_id);
                                }
                                else
                                {
                                    sprintf(sqlmsg.str_sql, "update public_cardinfo_table set format_flag='%c' update_data=%d where card_id=%d", 'Y', 1, card_id);
                                }
                                memcpy(msgbuf.buf, sqlmsg.str_sql, sizeof(sqlmsg.str_sql));
                                msgsnd(parm->id_dbmsg, &msgbuf, sizeof(msgbuf)-sizeof(long),0); 
                            }
                            else//格式化失败
                            {	
                                recordmsg.forbid_reason = STR_REASON8;
                                recordmsg.letgo_flag = 'N';
                                logmsg.deal_reason = STR_REASON8;
                                logmsg.deal_type = 'f';
                                logmsg.deal_result = 'f';
                                //card_datazone.abnormal_flag = CARD_STA_NOR;
                                //card_datazone.deal_reason = REASON0;
                                ele10_flag = ELE9_BEEP_ALERT;
                            }
                        }
                        construct_elementdata10(ele10_flag, &elementdata);
                    }
                    else //2本系统码
                    {
                        //验证数据区内容,即流水号
                        sprintf(str_sql, "select update_data from public_cardinfo_table where card_id=%d",card_id);
                        ret = mysql_query(&conn, str_sql);
                        if(ret != 0)
                        {
                            M_print_num_mysql(&conn, "485no", parm->no);
                        }
                        else
                        {
                            res = mysql_store_result(&conn);
                            if(res == NULL)
                            {
                                M_print_num_mysql(&conn, "485no", parm->no);
                            }
                            else
                            {
                                if((row = mysql_fetch_row(res)) == NULL)
                                {
                                    M_print_num_mysql(&conn, "485no", parm->no);
                                }
                                else
                                {
                                    update_data = atoi(row[0]);
                                }
                            }
                            mysql_free_result(res);
                        }
                        if(!(update_data < get_flow_data(recvretreq.elementdata[i].elementdatacontext))) //不小于
                        {
                            //数据区内容错误
                            recordmsg.forbid_reason = STR_REASON1;
                            recordmsg.letgo_flag = 'N';
                            logmsg.deal_reason = STR_REASON1;
                            logmsg.deal_type = 'n';
                            logmsg.deal_result = 'n';
                            construct_elementdata10(ELE9_BEEP_ALERT, &elementdata);
                        }
                        else
                        {
                            sprintf(str_sql, "select interval_flag from door%d_limit_table where card_id=%d and access_flag='Y' and ", parm->send_addr, card_id);
                            ret = mysql_query(&conn, str_sql);
                            if(ret != 0)
                            {
                                M_print_num_mysql(&conn, "485no", parm->no);
                            }
                            else
                            {
                                res = mysql_store_result(&conn);
                                if(res == NULL)
                                {
                                    M_print_num_mysql(&conn, "485no", parm->no);
                                }
                                else
                                {
                                    if((row = mysql_fetch_row(res)) == NULL)
                                    {
                                        M_print_num_mysql(&conn, "485no", parm->no);
                                        interval = 0;
                                    }
                                    else
                                    {
                                        interval = atoi(row[0]);
                                    }
                                }
                                mysql_free_result(res);
                            }
                            if(interval & (1<<getdate_hour()))
                            {
                                //有时段权限
                                //查黑户表
                                sprintf(str_sql, "select id from public_illegal_table where card_id=%d", card_id);
                                ret = mysql_query(&conn, str_sql);
                                if(ret != 0)
                                {
                                    M_print_num_mysql(&conn, "485no", parm->no);
                                }
                                else
                                {
                                    res = mysql_store_result(&conn);
                                    if(res == NULL)
                                    {
                                        M_print_num_mysql(&conn, "485no", parm->no);
                                    }
                                    else
                                    {
                                        if(mysql_num_rows(res) != 0)
                                        {
                                            illegal_flag = 1; 
                                        }
                                        else
                                        {
                                            illegal_flag = 0;
                                        }
                                    }
                                    mysql_free_result(res);
                                }
                                if(illegal_flag == 0)
                                {
                                    //不是黑户
                                    //查卡片异常标志
                                    if(get_sta_data(recvretreq.elementdata[i].elementdatacontext) != 1)
                                    {
                                        //无异常
                                        //更新数据区
                                        card_datazone.deal_reason = REASON0;
                                        card_datazone.abnormal_flag = CARD_STA_NOR;
                                        card_datazone.flow_num = update_data+1;    //用卡数据区的流水号，不变
                                        ele10_flag = ELE9_UPDATE; 
                                        card_datazone.device_type = DEV_TYP_POS;
                                        card_datazone.device_no = parm->recv_addr;
                                        getdate_BCD(card_datazone.time);
                                        construct_elementdata9(&card_datazone, ele10_flag, &elementdata);
                                        sndfrminfo.afn = AFN_SETSINGPARM;
                                        (seq == 255) ? (sndfrminfo.seq = seq, seq = 0) : (sndfrminfo.seq = seq++);
                                        sndfrminfo.elementdata_num = 1;
                                        sndfrminfo.elementdata = &elementdata;
                                        if((retlen = constructFRM(&sndfrminfo, sndbuf, parm->codetype)) == 0)
                                        {
                                            M_print_num_str("constructFRM fail", "485no", parm->no);
                                        }
                                        ret = 0;
                                        while(ret < retlen)
                                        {
                                            ret += write(fd, sndbuf+ret, retlen-ret);
                                            if(ret < 0)
                                            {
                                                M_print_num_err(errno, "485no", parm->no);
                                                break;
                                            }
                                        }
                                        FD_SET(fd, &rdflags);
                                        wait.tv_sec = 0;
                                        wait.tv_usec = 5000;                   //500ms
                                        ret = select(fd+1, &rdflags, NULL, NULL, &wait);
                                        if(ret == 0)       //超时
                                        {
                                            //更新无回应 记录结果
                                            recordmsg.forbid_reason = STR_REASON9;
                                            logmsg.deal_reason = STR_REASON9;
                                            recordmsg.letgo_flag = 'N';
                                            logmsg.deal_type = 'u';
                                            logmsg.deal_result = 'n';
                                            ele10_flag = ELE9_BEEP_ALERT;
                                        }
                                        else
                                        {
                                            //更新有回应
                                            read(fd, pbuf, sizeof(buf)-(pbuf-buf));
                                            recvfrminfo.afn = AFN_SETSINGPARM;
                                            (seq == 255) ? (recvfrminfo.seq = seq, seq = 0) : (recvfrminfo.seq = seq++);
                                            recvlen = pbuf-buf;
                                            pbuf = buf;
                                            ret = analyzeFRM(&recvfrminfo, &recvretset, &pbuf, sizeof(buf), recvlen, parm->codetype);
                                            if(recvretset.result == 1)
                                            {
                                                //更新成功
                                                iomsg.mtype = 1;
                                                iomsg.io_switch = OPENDOOR;
                                                iomsg.t = time(NULL);
                                                msgsnd(parm->id_iomsg, &iomsg, sizeof(iomsg)-sizeof(long), 0);    //开门
                                                recordmsg.forbid_reason = STR_REASON0;
                                                recordmsg.letgo_flag = 'Y';
                                                logmsg.deal_reason = STR_REASON0;
                                                logmsg.deal_type = 'u';
                                                logmsg.deal_result = 's';
                                                ele10_flag = ELE9_BEEP_OK;
                                                //更新数据库数据区 
                                                msgbuf.mtype = OPT_OTHER_TABLE;
                                                sprintf(sqlmsg.str_sql, "update public_cardinfo_table set update_data=%d where card_id=%d", update_data+1, card_id);
                                                memcpy(msgbuf.buf, sqlmsg.str_sql, sizeof(sqlmsg.str_sql));
                                                msgsnd(parm->id_dbmsg, &msgbuf, sizeof(msgbuf)-sizeof(long),0); 
                                            }
                                            else//更新失败
                                            {	
                                                recordmsg.forbid_reason = STR_REASON9;
                                                recordmsg.letgo_flag = 'N';
                                                logmsg.deal_reason = STR_REASON9;
                                                logmsg.deal_type = 'u';
                                                logmsg.deal_result = 'f';
                                                ele10_flag = ELE9_BEEP_ALERT;
                                            }
                                        }
                                        card_datazone.device_type = DEV_TYP_POS;
                                        card_datazone.device_no = parm->recv_addr;
                                        getdate_BCD(card_datazone.time);
                                        construct_elementdata10(ele10_flag, &elementdata);
                                    }
                                    else
                                    {
                                        //有异常
                                        recordmsg.forbid_reason = STR_REASON5;
                                        logmsg.deal_reason = STR_REASON5;
                                        card_datazone.deal_reason = REASON5;
                                        recordmsg.letgo_flag = 'N';
                                        logmsg.deal_type = 'u';
                                        logmsg.deal_result = 'n';
                                        card_datazone.abnormal_flag = CARD_STA_UNNOR;
                                        card_datazone.flow_num = update_data;    //用卡数据区的流水号，不变
                                        ele10_flag = ELE9_UPDATE|ELE9_BEEP_ALERT;
                                        card_datazone.device_type = DEV_TYP_POS;
                                        card_datazone.device_no = parm->recv_addr;
                                        getdate_BCD(card_datazone.time);
                                        construct_elementdata9(&card_datazone, ele10_flag, &elementdata);
                                    }
                                }
                                else
                                {
                                    //是黑户
                                    recordmsg.forbid_reason = STR_REASON4;
                                    logmsg.deal_reason = STR_REASON4;
                                    card_datazone.deal_reason = REASON4;
                                    recordmsg.letgo_flag = 'N';
                                    logmsg.deal_type = 'u';
                                    logmsg.deal_result = 'n';
                                    card_datazone.abnormal_flag = CARD_STA_NOR;
                                    card_datazone.flow_num = update_data;    //用卡数据区的流水号，不变
                                    ele10_flag = ELE9_UPDATE|ELE9_BEEP_ALERT;
                                    card_datazone.device_type = DEV_TYP_POS;
                                    card_datazone.device_no = parm->recv_addr;
                                    getdate_BCD(card_datazone.time);
                                    construct_elementdata9(&card_datazone, ele10_flag, &elementdata);
                                }
                            }
                            else
                            {
                                //没有时段权限
                                if(interval == 0) //此门没权限
                                {
                                    recordmsg.forbid_reason = STR_REASON2;
                                    logmsg.deal_reason = STR_REASON2;
                                    card_datazone.deal_reason = REASON2;   
                                }
                                else
                                {
                                    recordmsg.forbid_reason = STR_REASON3;
                                    logmsg.deal_reason = STR_REASON3;
                                    card_datazone.deal_reason = REASON3;   
                                }
                                recordmsg.letgo_flag = 'N';
                                logmsg.deal_type = 'u';
                                logmsg.deal_result = 'n';
                                card_datazone.abnormal_flag = CARD_STA_NOR;
                                card_datazone.flow_num = get_flow_data(recvretreq.elementdata[i].elementdatacontext);    //用卡数据区的流水号，不变
                                ele10_flag = ELE9_UPDATE|ELE9_BEEP_ALERT;
                                card_datazone.device_type = DEV_TYP_POS;
                                card_datazone.device_no = parm->recv_addr;
                                getdate_BCD(card_datazone.time);
                                construct_elementdata9(&card_datazone, ele10_flag, &elementdata);
                            }
                            
                        }
                    }
                }
                //提示刷卡读头蜂鸣或者同时更新数据区
                sndfrminfo.afn = AFN_SETSINGPARM;
                (seq == 255) ? (sndfrminfo.seq = seq, seq = 0) : (sndfrminfo.seq = seq++);
                sndfrminfo.elementdata_num = 1;
                sndfrminfo.elementdata = &elementdata;
                if((retlen = constructFRM(&sndfrminfo, sndbuf, parm->codetype)) == 0)
                {
                    M_print_num_str("constructFRM fail", "485no", parm->no);
                    break;
                }
                ret = 0;
                while(ret < retlen)
                {
                    ret += write(fd, sndbuf+ret, retlen-ret);
                    if(ret < 0)
                    {
                        M_print_num_err(errno, "485no", parm->no);
                        break;
                    }
                }
                //插入消息队列
                msgbuf.mtype = OPT_RECORD_TABLE;
                recordmsg.card_no = card_no;
                time(&recordmsg.time);
                //recordmsg.forbid_reason = REASON1;
                recordmsg.head_no = parm->recv_addr;
                //recordmsg.letgo_flag = 'N';
                bzero(msgbuf.buf, sizeof(msgbuf.buf));
                memcpy(msgbuf.buf, &recordmsg, sizeof(recordmsg));
                msgsnd(parm->id_dbmsg, &msgbuf, sizeof(msgbuf)-sizeof(long),0);
                msgbuf.mtype = OPT_LOG_TABLE;
                logmsg.card_no = card_no;
                time(&logmsg.time);
                //logmsg.deal_reason = REASON1;
                logmsg.head_no = parm->recv_addr;
                //logmsg.deal_type = 'n';
                //logmsg.deal_result = 'n';
                bzero(logmsg.recv_buf, sizeof(logmsg.recv_buf));
                memcpy(logmsg.recv_buf, recvretreq.elementdata[i].elementdatacontext, sizeof(recvretreq.elementdata[i].elementdatacontext)/sizeof(recvretreq.elementdata[i].elementdatacontext[0]));
                bzero(logmsg.snd_buf, sizeof(logmsg.snd_buf));
                bzero(msgbuf.buf, sizeof(msgbuf.buf));
                memcpy(msgbuf.buf, &logmsg, sizeof(logmsg));
                msgsnd(parm->id_dbmsg, &msgbuf, sizeof(msgbuf)-sizeof(long),0);
                break;
            }//end switch loog
        }//end for loop
        analyzeFRM_release(&recvretreq);
    }
    exit(0);	
}


/*
void main()
{}
*/
