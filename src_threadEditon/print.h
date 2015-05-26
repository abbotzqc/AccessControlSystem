
#ifndef PRINTF_H
#define PRINTF_H
#include<time.h>
#include<stdio.h>

#define M_print_time_err(errno) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)

#define M_print_err(errno) \
        do{printf("usage %s : line%d fail -- %s\n", __func__, __LINE__, strerror(errno));}while(0)

#define M_print_time_str(str) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d -- %s\n", __func__, __LINE__, str);}while(0)

#define M_print_str(str) \
        do{printf("usage %s : line%d -- %s\n", __func__, __LINE__, str);}while(0)

#define M_print_time_mysql(conn) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d fail -- %u__%s\n", __func__, __LINE__, mysql_errno(conn),mysql_error(conn));}while(0)

#define M_print_mysql(conn) \
        do{printf("usage %s : line%d fail -- %u__%s\n", __func__, __LINE__, mysql_errno(conn),mysql_error(conn));}while(0)

////
#define M_print_time_num_err(errno, num_name, num) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d fail -- %s_%d_%s\n", __func__, __LINE__, num_name, num, strerror(errno));}while(0)

#define M_print_num_err(errno, num_name, num) \
        do{printf("usage %s : line%d fail -- %s_%d_%s\n", __func__, __LINE__, num_name, num, strerror(errno));}while(0)

#define M_print_time_num_str(str, num_name, num) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d -- %s_%d_%s\n", __func__, __LINE__, num_name, num, str);}while(0)

#define M_print_num_str(str, num_name, num) \
        do{printf("usage %s : line%d -- %s_%d_%s\n", __func__, __LINE__, num_name, num, str);}while(0)

#define M_print_time_num_mysql(conn, num_name, num) \
        do{time_t t;time(&t);printf("%s ~~~~ ",asctime(gmtime(&t)));printf("usage %s : line%d fail -- %s_%d_%u__%s\n", __func__, __LINE__, num_name, num, mysql_errno(conn),mysql_error(conn));}while(0)

#define M_print_num_mysql(conn, num_name, num) \
        do{printf("usage %s : line%d fail -- %s_%d_%u__%s\n", __func__, __LINE__, num_name, num, mysql_errno(conn),mysql_error(conn));}while(0)
#endif

//#define DEBUG
#ifdef DEBUG 
#define D_M_PRINT(integret, str) printf("debug usage %s : line%d -- integret:%d  str:%s\n", __func__, __LINE__, integret, str)
#else
#define D_M_PRINT(integret, str)
#endif
