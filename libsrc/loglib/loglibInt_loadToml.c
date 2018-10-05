#ifndef LOGLIB_TOML_CONFIG_DISABLE

#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>

#include "toml.h"

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "loglibInt.h"
#include "loglibInt.x"

#define TOML_CFG_GET_RAW_NUM(_tbl, _key, _rt_int, _datType, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    S_64 d_dat = 0;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_in((_tbl),(_key));\
    if(d_tmp != NULL){\
        if(toml_rtoi(d_tmp, &d_dat) == 0){\
            (_rt_int) = (_datType)d_dat;\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

#define TOML_CFG_GET_RAW_NUM_AT(_array, _idx,  _rt_int, _datType, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    S_64 d_dat = 0;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_at((_array),(_idx));\
    if(d_tmp != NULL){\
        if(toml_rtoi(d_tmp, &d_dat) == 0){\
            (_rt_int) = (_datType)d_dat;\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

#define TOML_CFG_GET_RAW_STR_PTR(_tbl, _key, _rt_str, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    CHAR *d_dat = NULL;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_in((_tbl),(_key));\
    if(d_tmp != NULL){\
        if(toml_rtos(d_tmp, &d_dat) == 0){\
            (_rt_str) = d_dat;\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

#define TOML_CFG_GET_RAW_STR(_tbl, _key, _rt_str, _maxLen, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    UINT d_datLen = 0;\
    CHAR *d_dat = NULL;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_in((_tbl),(_key));\
    if(d_tmp != NULL){\
        if(toml_rtos(d_tmp, &d_dat) == 0){\
            d_datLen = comlib_strGetLen(d_dat);\
            if(d_datLen >= (_maxLen)){\
                d_datLen = (_maxLen)-1;\
            }\
            comlib_strNCpy((_rt_str), d_dat, d_datLen);\
            (_rt_str)[d_datLen] = '\0';\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

#define TOML_CFG_GET_RAW_STR_AT_PTR(_array, _idx, _rt_str, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    CHAR *d_dat = NULL;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_at((_array),(_idx));\
    if(d_tmp != NULL){\
        if(toml_rtos(d_tmp, &d_dat) == 0){\
            (_rt_str) = d_dat;\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

#define TOML_CFG_GET_RAW_STR_AT(_array, _idx, _rt_str, _maxLen, _rt_ret){\
    CONST CHAR *d_tmp = NULL;\
    UINT d_datLen = 0;\
    CHAR *d_dat = NULL;\
    (_rt_ret) = RC_NOK;\
    d_tmp = toml_raw_at((_array),(_idx));\
    if(d_tmp != NULL){\
        if(toml_rtos(d_tmp, &d_dat) == 0){\
            d_datLen = comlib_strGetLen(d_dat);\
            if(d_datLen >= (_maxLen)){\
                d_datLen = (_maxLen)-1;\
            }\
            comlib_strNCpy((_rt_str), d_dat, d_datLen);\
            (_rt_str)[d_datLen] = '\0';\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

FT_PRIVATE RT_RESULT         load_apnd               (toml_table_t *apndTbl, LoglibIntApndCfg **rt_apndCfg);
FT_PRIVATE RT_RESULT         load_log                (toml_table_t *logTbl);
FT_PRIVATE RT_RESULT         load_logCfg             (toml_table_t *logCfg);

FT_PRIVATE RT_RESULT load_apnd(toml_table_t *apndTbl, LoglibIntApndCfg **rt_apndCfg)
{
    SINT ret = 0;
    UINT apndType = 0;
    CHAR *dat = NULL;
    LoglibIntApndCfg *apndCfg = NULL;

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "type", dat, ret);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Appender type not exist\n");
        return LOGERR_APND_TYPE_NOT_EXSIT;
    }
    else {
        if(comlib_strCaseCmp(dat, "file") == 0){
            apndType = LOGLIB_APND_TYPE_FILE;
        }
        else if(comlib_strCaseCmp(dat, "syslog") == 0){
            apndType = LOGLIB_APND_TYPE_SYSLOG;
        }
        else if(comlib_strCaseCmp(dat, "stdout") == 0){
            apndType = LOGLIB_APND_TYPE_STDOUT;
        }
        else {
            LOG_LOG(LOG_INT_ERR,"Invalid appender type(%s)\n",dat);
            return LOGERR_APND_INVALID_TYPE;
        }
    }

    apndCfg = comlib_memMalloc(sizeof(LoglibIntApndCfg));

    apndCfg->lnkNode.data = apndCfg;
    apndCfg->type = apndType;

    LOGLIB_INIT_APND_CFG(&apndCfg->apndCfg);

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "name", apndCfg->name, ret);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"append name not exist\n");
        return LOGERR_MAND_APND_DATA_NOT_EXIST;
    }

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "display_level", dat, ret);
    if(ret == RC_OK){
        if(comlib_strCaseCmp((CHAR*)dat,"true") == 0){
            apndCfg->apndCfg.dispBit |= LOGLIB_DISP_LVL_BIT;
        }
    }

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "display_file", dat, ret);
    if(ret == RC_OK){
        if(comlib_strCaseCmp((CHAR*)dat,"true") == 0){
            apndCfg->apndCfg.dispBit |= LOGLIB_DISP_FILE_BIT;
        }
    }

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "display_line", dat, ret);
    if(ret == RC_OK){
        if(comlib_strCaseCmp((CHAR*)dat,"true") == 0){
            apndCfg->apndCfg.dispBit |= LOGLIB_DISP_LINE_BIT;
        }
    }

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "display_time", dat, ret);
    if(ret == RC_OK){
        if(comlib_strCaseCmp((CHAR*)dat,"true") == 0){
            apndCfg->apndCfg.dispBit |= LOGLIB_DISP_TIME_BIT;
        }
    }

    TOML_CFG_GET_RAW_STR_PTR(apndTbl, "log_level", dat, ret);
    if(ret == RC_OK){
        UINT logLvl = 0;

        logLvl = comlib_utilAtoi((CHAR*)dat, comlib_strGetLen((CHAR*)dat));

        if(logLvl == 1){
            apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_ERR_LOG_BIT;
        }
        else if(logLvl == 2){
            apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_NOTY_LOG_BIT;
        }
        else if(logLvl == 3){
            apndCfg->apndCfg.logLvlBit |= LOGLIB_APND_DISP_DBG_LOG_BIT;
        }
    }

    switch(apndType){
        case LOGLIB_APND_TYPE_FILE:
            {
                TOML_CFG_GET_RAW_STR_PTR(apndTbl, "log_path", dat, ret);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"Log path not exist\n");
                    return LOGERR_MAND_APND_DATA_NOT_EXIST;
                }

                apndCfg->apndCfg.u.file.logPath = (CHAR*)dat;

                TOML_CFG_GET_RAW_STR_PTR(apndTbl, "log_name", dat, ret);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"Log name not exist\n");
                    return LOGERR_MAND_APND_DATA_NOT_EXIST;
                }

                apndCfg->apndCfg.u.file.name= (CHAR*)dat;

                TOML_CFG_GET_RAW_NUM(apndTbl, "max_log_size", 
                        apndCfg->apndCfg.u.file.maxLogSize,
                        U_32, ret);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"max log size not exist\n");
                    return LOGERR_MAND_APND_DATA_NOT_EXIST;
                }
            }
            break;
        case LOGLIB_APND_TYPE_SYSLOG:
            {
                UINT fac = 0;

                TOML_CFG_GET_RAW_STR_PTR(apndTbl, "facility", dat, ret);
                if(ret == RC_OK){
                    if(comlib_strCaseCmp((CHAR*)dat,"LOG_KERN") == 0)             { fac = LOG_KERN; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_USER") == 0)        { fac = LOG_USER; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_MAIL") == 0)        { fac = LOG_MAIL; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_DAEMON") == 0)      { fac = LOG_DAEMON; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_AUTH") == 0)        { fac = LOG_AUTH; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_SYSLOG") == 0)      { fac = LOG_SYSLOG; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LPR") == 0)         { fac = LOG_LPR; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_NEWS") == 0)        { fac = LOG_NEWS; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_UUCP") == 0)        { fac = LOG_UUCP; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_CRON") == 0)        { fac = LOG_CRON; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_AUTHPRIV") == 0)    { fac = LOG_AUTHPRIV; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_FTP") == 0)         { fac = LOG_FTP; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL0") == 0)      { fac = LOG_LOCAL0; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL1") == 0)      { fac = LOG_LOCAL1; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL2") == 0)      { fac = LOG_LOCAL2; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL3") == 0)      { fac = LOG_LOCAL3; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL4") == 0)      { fac = LOG_LOCAL4; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL5") == 0)      { fac = LOG_LOCAL5; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL6") == 0)      { fac = LOG_LOCAL6; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"LOG_LOCAL7") == 0)      { fac = LOG_LOCAL7; }
                    else {
                        LOG_LOG(LOG_INT_ERR,"Invalid fac(%s)\n",dat);
                        return LOGERR_INVALID_SYSLOG_FAC_TYPE;
                    }

                    apndCfg->apndCfg.u.syslog.fac = fac;
                }/* end of if(ret == RC_OK) */
            }
            break;
        case LOGLIB_APND_TYPE_STDOUT:
            {
                UINT stdType = 0;
                TOML_CFG_GET_RAW_STR_PTR(apndTbl, "std_type", dat, ret);
                if(ret == RC_OK){
                    if(comlib_strCaseCmp((CHAR*)dat,"stderr") == 0)             { stdType = LOG_KERN; }
                    else if(comlib_strCaseCmp((CHAR*)dat,"stdout") == 0)        { stdType = LOG_USER; }
                    else {
                        LOG_LOG(LOG_INT_ERR,"Invalid stdout type(%s)\n", dat);
                        return LOGERR_INVALID_STDOUT_TYPE;
                    }

                    apndCfg->apndCfg.u.stdout.type = stdType;
                }
            }
            break;
        default:
            {
                LOG_LOG(LOG_INT_ERR,"Invalid appender type(%d)\n",apndType);
                return LOGERR_APND_INVALID_TYPE;
            }
            break;
    };/* end of switch(apndType) */

    (*rt_apndCfg) = apndCfg;

    return RC_OK;
}

FT_PRIVATE RT_RESULT load_log(toml_table_t *logTbl)
{
    SINT ret = RC_OK;
    UINT i = 0;
    CHAR *name = NULL;
    CONST CHAR *rawDat = NULL;
    CHAR *dat = NULL;
    toml_table_t *apndTbl = NULL;
    toml_array_t *apndArray = NULL;
    LoglibCfg cfg;
    ComlibLnkNode *lnkNode = NULL;
    LoglibIntApndCfg *apndCfg = NULL;
    ComlibLnkLst apnderLL;

    LOGLIB_INIT_CFG(&cfg);

    cfg.dfltApndType = LOGLIB_APND_TYPE_NONE;

    ret = comlib_lnkLstInit(&apnderLL, ~0);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Appender linked list init failed(ret=%d)\n",ret);
        return LOGERR_APND_BUCKET_INIT_FAILED;
    }

    rawDat = toml_raw_in(logTbl,"name");
    if(rawDat != NULL){
        if(toml_rtos(rawDat, &dat) != 0){
            return RC_OK;
        }

        name = dat;
    }

    cfg.wrType = LOGLIB_WR_TYPE_DIR;

    TOML_CFG_GET_RAW_NUM(logTbl, "log_level", cfg.dfltLogLvl, U_32, ret);
    if(ret != RC_OK){
        cfg.dfltLogLvl = LOGLIB_LVL_ERR;
    }

    rawDat = toml_raw_in(logTbl,"write_thread_flag");
    if(rawDat != NULL){
        if(toml_rtos(rawDat, &dat) != 0){
            return RC_OK;
        }

        ret = comlib_strCaseCmp(dat, "true"); 
        if(ret == 0){
            cfg.wrType = LOGLIB_WR_TYPE_THRD;
        }
        else {
            cfg.wrType = LOGLIB_WR_TYPE_DIR;
        }
    }

    /* appender */
    apndArray = toml_array_in(logTbl, "APPENDER");
    if(apndArray != NULL){
        for(i=0;;i++){
            apndTbl = toml_table_at(apndArray, i);
            if(apndTbl == NULL){
                break;
            }

            ret = load_apnd(apndTbl, &apndCfg);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"Appender load failed(ret=%d)\n",ret);
                return ret;
            }

            ret = comlib_lnkLstInsertTail(&apnderLL, &apndCfg->lnkNode);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"new Appender  create failed(ret=%d)\n",ret);
                return LOGERR_APNDER_INSERT_FAILED;
            }
        }/* end of for(i=0;;i++) */
    }/* end of if(apndArray != NULL) */

    /* load */
    ret = loglib_apiInit(name, &cfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Loglib init failed(ret=%d)\n",ret);
        return ret;
    }

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&apnderLL);
        if(lnkNode == NULL){
            break;
        }

        apndCfg = lnkNode->data;

        ret = loglib_apiRegApnd(name, apndCfg->name, apndCfg->type, &apndCfg->apndCfg);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Appender regist failed(ret=%d)\n",ret);
            return ret;
        }

        ret = loglibInt_loadDstryApndCfg(apndCfg);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Appender destory failed(ret=%d)\n",ret);
        }

        comlib_memFree(apndCfg);
    }/* end of while(1) */

    return RC_OK;
}

FT_PRIVATE RT_RESULT load_logCfg(toml_table_t *logCfg)
{
    UINT i = 0;
    toml_table_t *logTbl = NULL;
    toml_array_t *logArray = NULL;

    logArray = toml_array_in(logCfg, "LOG");
    if(logArray == NULL){
        LOG_LOG(LOG_INT_ERR,"LOG array not exist\n");
        return RC_OK;
    }

    for(i=0;;i++){
        logTbl = toml_table_at(logArray, i);
        if(logTbl == NULL){
            break;
        }

        load_log(logTbl);
    }/* end of for(i=0;;i++) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_loadToml(CONST CHAR *cfgFile, CONST CHAR *cfgName)
{
    SINT ret = RC_OK;
    UINT i = 0;
    FILE *fp = NULL;
    CONST CHAR *rawDat = NULL;
    CHAR *dat = NULL;
    CHAR  errbuf[200];
    toml_table_t *conf = NULL;
    toml_table_t *logCfg = NULL;
    toml_array_t *logCfgArray = NULL;

    /* open file and parse */
    fp = fopen(cfgFile, "r");
    if (fp == NULL){
        LOG_LOG(LOG_INT_ERR,"TOML open failed(%d:%s)\n",errno, strerror(errno));
        return LOGERR_TOML_PARSING_FAILED;
    }

    conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    if (0 == conf) {
        LOG_LOG(LOG_INT_ERR, "toml parsing failed(%s)\n", errbuf);
        return LOGERR_TOML_PARSING_FAILED;
    }

    /* cloes file */
    fclose(fp);

    logCfgArray = toml_array_in(conf, "LOG_CFG");
    if(logCfgArray == NULL){
        LOG_LOG(LOG_INT_ERR, "LOG_CFG NOT EXIST\n");
        toml_free(conf);
        return LOGERR_TOML_PARSING_FAILED;
    }

    for(i=0;;i++){
        logCfg = toml_table_at(logCfgArray,i);
        if(logCfg == NULL){
            break;
        }

        rawDat = toml_raw_in(logCfg,"name");
        if((rawDat != NULL) && cfgName != NULL){
            if(toml_rtos(rawDat, &dat) != 0){
                continue;
            }

            ret = comlib_strCaseCmp(cfgName, dat); 
            if(ret != 0){
                continue;
            }
        }/* end of if((rawDat != NULL) && cfgName != NULL) */
        else if((rawDat != NULL) && cfgName == NULL){
            continue;
        }
        else if((rawDat == NULL) && cfgName != NULL){
            continue;
        }

        ret = load_logCfg(logCfg);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Log config laod failed(ret=%d)\n",ret);
            toml_free(conf);
            return LOGERR_TOML_PARSING_FAILED;
        }
    }/* end of for(i=0;;i++) */

    toml_free(conf);

    return RC_OK;
}

#endif /* LOGLIB_TOML_CONFIG_DISABLE */
