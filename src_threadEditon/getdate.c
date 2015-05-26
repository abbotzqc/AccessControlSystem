/*************************************************************************
	> File Name: getdate.c
	> Author: 
	> Mail: 
	> Created Time: Tue 31 Mar 2015 12:38:36 PM CST
 ************************************************************************/

#include<stdio.h>
#include<sys/time.h>
#include<time.h>
#include<string.h>
void getdate_BCD(char str_time[])
{
    time_t t;
    struct tm *tm;
    time(&t);
    tm = localtime(&t);
    sprintf(str_time, "%02d%02d%02d%02d%02d", tm->tm_year+1900-2000, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min);
    return;
}

void getdate_unknow_t(char str_time[])
{
    time_t t;
    struct tm *tm;
    time(&t);
    tm = localtime(&t);
    sprintf(str_time, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return;
}

void getdate_know_t(time_t t, char str_time[])
{
    struct tm *tm;
//    time(&t);
    tm = localtime(&t);
    sprintf(str_time, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return;
}

unsigned int getdate_hour()
{
    time_t t;
    struct tm *tm;
    time(&t);
    tm = localtime(&t);
    return tm->tm_hour;
}
/*
void main()
{
    char str[10];
    getdate(str);
    printf("%s  %d\n",str,strlen(str));
    return;
}*/
