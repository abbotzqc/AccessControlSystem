/*************************************************************************
	> File Name: ReadConf.c
	> Author: 
	> Mail: 
	> Created Time: Wed 22 Apr 2015 01:28:34 PM CST
 ************************************************************************/

#include<stdio.h>
#include"ReadConf.h"


/*
 * 功能：读取conf配置文件
 * */



int read_conf(s_conf *conf, const char *filename)
{
    int len;
    conf->flag_free = 0;         //0 --- parm not need free
    conf->parm = NULL;
    if(filename == NULL)
    {
        M_print_str("filename NULL, check the direction is correct !");
        return -1;
    }
    if((conf->fd = open(filename, O_RDONLY, 0666)) == 0)
    {
        M_print_str("open conf file fail, check the direction is correct !");
        return -1;
    }
    lseek(conf->fd, 0, SEEK_END);
    len = lseek(conf->fd, 0, SEEK_CUR);
    lseek(conf->fd, 0, SEEK_SET);
    conf->buf = (char*)malloc(sizeof(char)*(len+1));
    memset(conf->buf, 0, len+1);
    read(conf->fd, conf->buf, len+1);
    return 1;
}

/*
* 功能：获得某个项目参数值
* */
int get_conf_item(s_conf *conf, const char *item)
{
    regex_t reg;
    int nmatch = 2;
    regmatch_t pmatch[3];
    char *pattern;
    if(conf->flag_free == 1)
    {
        free(conf->parm);
        conf->flag_free = 0;
    }
    pattern = (char*)malloc(sizeof(char)*(strlen(item)+strlen(" =.*\"(.*)\""))+1);
    memset(pattern, 0, strlen(item)+strlen(" =.*\"(.*)\"")+1);
//    memcpy(pattern, item, strlen(item));
//    memcpy(pattern+strlen(item), " =.*\"(.*)\"", strlen(" =.*\"(.*)\""));
    sprintf(pattern, "%s =.*\"(.*)\"", item);
    if(regcomp(&reg, pattern, REG_EXTENDED|REG_ICASE|REG_NEWLINE) < 0)
    {
        M_print_err(errno);
        free(pattern);
        return -1;
    }
    if(regexec(&reg, conf->buf, nmatch, pmatch, 0) == REG_NOMATCH)
    {
        M_print_err(errno);
        free(pattern);
        regfree(&reg);
        return -1;
    }
    if(pmatch[1].rm_so == -1 || pmatch[1].rm_eo == -1)
    {
        M_print_str("pmatch[1].rm -1");
        free(pattern);
        regfree(&reg);
        return -1;
    }
    //regfree(&reg);
    free(pattern);
    
    conf->parm = (char*)malloc(sizeof(char)*(pmatch[1].rm_eo-pmatch[1].rm_so+1));
    memset(conf->parm, 0, pmatch[1].rm_eo-pmatch[1].rm_so+1);
    conf->flag_free = 1;
    strncpy(conf->parm, conf->buf+pmatch[1].rm_so, pmatch[1].rm_eo-pmatch[1].rm_so);
    return 1;
}

/*
* 功能：释放资源
* */
int release_conf(s_conf *conf)
{
    close(conf->fd);
    free(conf->buf);
    if(conf->flag_free == 1)
    {
        free(conf->parm);
        conf->parm = NULL;
    }
    conf->fd = 0;
    conf->buf = NULL;
    return 1;
}
