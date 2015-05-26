#include"print.h"

#define return_if_fail(errno) {printf("usage %s : %d fail -- %s\n", __func__, __LINE__, strerror(errno));}
#define return_str(str) {printf("usage %s : %d fail -- %s\n", __func__, __LINE__, str);}
#define SAFE_FREE(p)    if(p != NULL) {regfree(p); p = NULL;}
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
#ifndef MY_REGEXE_H
#define MY_REGEXE_H

#include<regex.h>
#include<sys/types.h>

#define N_PATTERN 512

//#include<regex.h>
typedef enum
{
    REG_RET_FAIL,
    REG_RET_SUCCESS,
}e_regFRM_ret;

typedef struct
{
    char *regbuf;
    char *pattern;
    //regex_t reg;
    int nmatch;
    regmatch_t *pmatch;
    int cflag;
}s_reginfo;

e_regFRM_ret regFRM(s_reginfo *reginfo);
#endif
