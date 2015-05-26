/*************************************************************************
	> File Name: my_regexe.c
	> Author: 
	> Mail: 
	> Created Time: Tue 24 Mar 2015 09:23:43 AM CST
 ************************************************************************/
#include<sys/types.h>
#include<regex.h>
#include"my_regexe.h"
#include<stdio.h>
#include<string.h>
#include<errno.h>

e_regFRM_ret regFRM(s_reginfo *reginfo)
{
    D_M_PRINT(0, "regFRM in");
    if((reginfo->regbuf == NULL) || (reginfo->pattern == NULL) || (reginfo->pmatch == NULL) || (reginfo->nmatch == 0))
    {
        return_str("something wrong with params");
        return REG_RET_FAIL;
    }
    
    int flag;
    int ret;
   // char errbuf[256];
   // regmatch_t nmatch, pmatch;
    regex_t reg;

    if(reginfo->cflag == 0)
    { 
        flag = REG_EXTENDED | REG_ICASE;
    }
    else
    {
            flag = reginfo->cflag;
        }
        ret = regcomp(&reg, reginfo->pattern, flag);
    if(ret < 0)
    {
        M_print_err(errno);
        //regerror(ret, &reg, errbuf, sizeof(errbuf));
        return REG_RET_FAIL;
    }
    D_M_PRINT(0, reginfo->regbuf);
    ret = regexec(&reg, reginfo->regbuf, reginfo->nmatch, reginfo->pmatch, 0);
    if(ret == REG_NOMATCH)
    {
        M_print_str("REG_NOMATCH");
        //regerror(ret, &reg, errbuf, sizeof(errbuf));
        regfree(&reg);
        return REG_RET_FAIL;
    }
    regfree(&reg);
    return REG_RET_SUCCESS;
}

