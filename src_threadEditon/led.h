#ifndef LED_H
#define LED_H

typedef struct
{
    char leddir[10];
    char blingtype;            //闪烁方式
    char led_no;               //传入ioctl中led的编号
}s_ledparm;

#define LightON 1
#define LightOFF 0
#define BlingSlow 2
#define BlingFast 3
#define ON_LED 1
#define OFF_LED 0

void *pthread_led(void *arg);

#endif
