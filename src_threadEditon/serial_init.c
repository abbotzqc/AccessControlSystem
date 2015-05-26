#include"serial_init.h"
#include<termios.h>
#include<unistd.h>
#include<stdio.h>
#include"print.h"
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
void serial_init(int fd, int vmin)
{
	struct termios options;//termios函数族提供一个常规的终端接口，用于控制非同步通信端口。

	tcgetattr(fd, &options);//取得当前串口的属性，并付给collect_fd这个设备

	//options.c_cflag |= (CLOCAL | CREAD);//clocal表示忽略modem控制线，cread表示打开接收者
    options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSIZE;//清空原有字符长度（csize表示原有字符长度掩码）
	options.c_cflag &= ~CRTSCTS;//不启用RTS/CTS（硬件）流控制
	options.c_cflag |= CS8;//设置字符长度掩码
	options.c_cflag &= ~CSTOPB;//设置停止位为1个（cstopb表示设置两个停止位）
    //options.c_cflag &= ~ (INLCR | ICRNL | IGNCR);

	//options.c_iflag &= ~ (IXON | IXOFF | IXANY);
    options.c_iflag |= IGNPAR;//忽略帧错误和奇偶校验错
    options.c_iflag &= ~INPCK;

	options.c_oflag = 0;//设置输出模式标志 
    //options.c_oflag &= ~(ONLCR | OCRNL);
	
    //options.c_lflag = 0;//设置本地模式标志
    options.c_lflag &= ~(ICANON|ECHO);//|ECHOE|ISIG);
    
   
	cfsetispeed(&options, B115200);//设置输入波特率
	cfsetospeed(&options, B115200);//设置输出波特率

    options.c_cc[VTIME] = 200;
    options.c_cc[VMIN] = 0; //vmin;

    tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd, TCSANOW, &options) != 0)//把上面设置号的属性赋值给collect_fd这个设备，tcsanow表示马上生效
    {
        M_print_str("set options for serial fail");
    }
	return;
}
