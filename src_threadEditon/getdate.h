#ifndef GETDATE_H
#define GETDATE_H

//数组大小要不小于20
void getdate_BCD(char str_time[]);
void getdate_unknow_t(char str_time[]);
void getdate_know_t(time_t t, char str_time[]);
unsigned int getdate_hour();
#endif
