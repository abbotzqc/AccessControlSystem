#ifndef READCONF_H
#define READCONF_H

#include<regex.h>
#include<sys/types.h>
#include"print.h"
#include <sys/types.h>
#include<unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
typedef struct
{
    char *buf;
    int fd;
    char *parm;
    char flag_free;
}s_conf;

int read_conf(s_conf *conf, const char *filename);
int get_conf_item(s_conf *conf, const char *item);
int release_conf(s_conf *conf);
#endif
