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

#include"print.h"

#ifndef IO_CTRL_H
#define IO_CTRL_H
#define BASEDIR "/sys/class/gpio/"
#define EXPORTDIR "/sys/class/gpio/export"
#define GPIODIR "/sys/class/gpio/gpio"
#define GPIO_DIRE "/direction"
#define GPIO_VAL "/value"

#define IN "in"
#define OUT "out"

#define READY_CLOSE 1
#define CLOSE 0
#define OPEN 1

typedef struct
{
	unsigned short send_addr;
	unsigned short recv_addr;
	int id_iomsg;
	int id_dbmsg;
	char gpio_in[12];           //eg:gpio1-24    信号输入管脚
	char gpio_out[12];          //信号输出管脚
}s_io_parm;

typedef struct                  //设置io控制参数
{	
	char *gpio;
	char *direction;
	char *flow;
}s_ioctrl;

int get_gpioval_pfile(FILE **pfile_val, s_ioctrl *parm);
int gpio_init(char *gpiodir, s_ioctrl *parm);
void *pthread_io(void *arg);
#endif
