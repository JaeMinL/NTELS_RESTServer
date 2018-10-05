#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>

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

FT_PRIVATE RT_RESULT loglibInt_apndSetApndPrevBit(LoglibIntApndCb *apndCb);

FT_PRIVATE RT_RESULT loglibInt_apndSetApndPrevBit(LoglibIntApndCb *apndCb)
{
    apndCb->prevDispBit = 0;

    apndCb->prevDispBit = apndCb->dispBit;

    return RC_OK;
}

FT_PUBLIC U_32 loglibInt_apndGetDispBit(LoglibIntApndCb *apndCb)
{
    return apndCb->dispBit;
}

FT_PUBLIC RT_RESULT loglibInt_apndSetPrevDispBit(LoglibIntApndCb *apndCb)
{
    return loglibInt_apndSetDispBit(apndCb, apndCb->prevDispBit);
}

FT_PUBLIC RT_RESULT loglibInt_apndSetDispBit(LoglibIntApndCb *apndCb, U_32 bit)
{

    loglibInt_apndSetApndPrevBit(apndCb);
    
    apndCb->dispBit = bit;

    return RC_OK;
}


FT_PUBLIC RT_RESULT loglibInt_apndFind(LoglibIntApndInfo *apndInfo, CHAR *name, UINT nameLen, LoglibIntApndCb **rt_apndCb)
{
    LoglibIntApndCb *apndCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }

    while(1){
        apndCb = lnkNode->data;

        if(apndCb->nameLen == nameLen){
            if(comlib_strNCmp(apndCb->name, name, nameLen) == 0){
                break;
            }
        }

        COM_GET_NEXT_NODE(lnkNode)
        if(lnkNode == NULL){
            return RC_NOK;
        }
    }/* end of while(1) */

    if(rt_apndCb != NULL){
        (*rt_apndCb) = apndCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndDel(LoglibIntApndInfo *apndInfo, LoglibIntApndCb *apndCb)
{
    SINT ret = RC_OK;

    ret = comlib_lnkLstDel(&apndInfo->apndLL, &apndCb->lnkNode);
    if(ret != RC_OK){
        LOG_LOG(LOG_ERR,"Append link list delete failed(ret=%d)\n",ret);
        return LOGERR_APND_DEL_FAILED;
    }

    comlib_memFree(apndCb);

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndDelAll(LoglibIntApndInfo *apndInfo)
{
    SINT ret = RC_OK;
    LoglibIntApndCb *apndCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        apndCb = lnkNode->data;

        ret = loglibInt_apndDel(apndInfo, apndCb);
        if(ret != RC_OK){
            LOG_LOG(LOG_ERR,"Append delete failed(ret=%d)\n",ret);
        }

        COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
        if(lnkNode == NULL){
            return RC_OK;
        }
    };

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndInit(UINT type, CHAR *name, LoglibApndCfg *apndCfg, LoglibIntApndCb **rt_apndCb)
{
    UINT nameLen = 0;
    UINT dispEnv = 0;
    LoglibIntApndCb *apndCb = NULL;

    GEN_CHK_ERR_RET((type != LOGLIB_APND_TYPE_STDOUT) &&
                    (type != LOGLIB_APND_TYPE_SYSLOG) &&
                    (type != LOGLIB_APND_TYPE_FILE) &&
                    (type != LOGLIB_APND_TYPE_USR),
                    LOG_LOG(LOG_ERR,"Invalid Append type(type=%d)\n",type),
                    LOGERR_INVALID_APND_TYPE);

    nameLen = comlib_strGetLen(name);
    if(nameLen >= LOGLIB_APND_NAME_MAX_LEN){
        LOG_LOG(LOG_ERR,"Invalid append name lenght(%d)\n", nameLen);
        return LOGERR_INVALID_APND_NAME_LEN;
    }

    apndCb = comlib_memMalloc(sizeof(LoglibIntApndCb));

    apndCb->lnkNode.data = apndCb;
    comlib_strNCpy(apndCb->name, name, nameLen);
    apndCb->name[nameLen] = '\0';
    apndCb->nameLen = nameLen;

    apndCb->apndType = type;

    if(apndCfg == NULL){
        apndCb->dispBit = LOGLIB_DISP_LVL_BIT | LOGLIB_DISP_FILE_BIT | LOGLIB_DISP_LINE_BIT | LOGLIB_DISP_TIME_BIT;
        apndCb->logLvlBit= LOGLIB_APND_DISP_ALL_LOG_BIT;
    }
    else {
        apndCb->dispBit = apndCfg->dispBit;
        apndCb->logLvlBit = apndCfg->logLvlBit;
    }

    /* check global evn */
    dispEnv = loglibInt_globGetDispEnv();
    if((dispEnv & LOGLIB_DISP_LVL_BIT) == 0){
        apndCb->dispBit &= ~LOGLIB_DISP_LVL_BIT;
    }
    if((dispEnv & LOGLIB_DISP_FILE_BIT) == 0){
        apndCb->dispBit &= ~LOGLIB_DISP_FILE_BIT;
    }
    if((dispEnv & LOGLIB_DISP_LINE_BIT) == 0){
        apndCb->dispBit &= ~LOGLIB_DISP_LINE_BIT;
    }
    if((dispEnv & LOGLIB_DISP_TIME_BIT) == 0){
        apndCb->dispBit &= ~LOGLIB_DISP_TIME_BIT;
    }

    switch(type){
        case LOGLIB_APND_TYPE_STDOUT:
            {
                if(apndCfg == NULL){
                    apndCb->u.stdout.type = LOGLIB_STDOUT_TYPE_ERR;
                }
                else {
                    apndCb->u.stdout.type = apndCfg->u.stdout.type;
                }
            }
            break;
        case LOGLIB_APND_TYPE_SYSLOG:
            {
                if(apndCfg == NULL){
                    apndCb->u.syslog.fac = LOG_LOCAL0;
                }
                else {
                    apndCb->u.syslog.fac = apndCfg->u.syslog.fac;
                }
            }
            break;
        case LOGLIB_APND_TYPE_FILE:
            {
                if(apndCfg == NULL){
                    comlib_memFree(apndCb);
                    return LOGERR_APND_CFG_NOT_EXIST;
                }
                apndCb->u.file.fp = NULL;

                apndCb->u.file.logPathLen = comlib_strGetLen(apndCfg->u.file.logPath);
                if( apndCb->u.file.logPathLen >=  LOGLIB_LOG_PATH_MAX_LEN){
                    LOG_LOG(LOG_ERR,"Invalid log path length(len=%d, max=%d)\n",
                            apndCb->u.file.logPathLen ,LOGLIB_LOG_PATH_MAX_LEN);
                    comlib_memFree(apndCb);
                    return LOGERR_INVLAID_LOG_PATH_LEN;
                }
                comlib_strCpy(apndCb->u.file.logPath, apndCfg->u.file.logPath);

                apndCb->u.file.nameLen = comlib_strGetLen(apndCfg->u.file.name);
                if( apndCb->u.file.nameLen >=  LOGLIB_APND_NAME_MAX_LEN){
                    LOG_LOG(LOG_ERR,"Invalid log append name length(len=%d, max=%d)\n",
                            apndCb->u.file.nameLen ,LOGLIB_APND_NAME_MAX_LEN);
                    comlib_memFree(apndCb);
                    return LOGERR_INVLAID_LOG_APND_NAME_LEN;
                }
                comlib_strCpy(apndCb->u.file.name, apndCfg->u.file.name);

                apndCb->u.file.maxLogSize = apndCfg->u.file.maxLogSize;
                apndCb->u.file.logSize = 0;
            }
            break;
        case LOGLIB_APND_TYPE_USR:
            {
                if(apndCfg == NULL){
                    comlib_memFree(apndCb);
                    return LOGERR_APND_CFG_NOT_EXIST;
                }

                apndCb->u.usr.usrLogFunc = apndCfg->u.usr.usrLogFunc;
            }
            break;
        default:
            {
                LOG_LOG(LOG_ERR,"Invalid append type(%d)\n",type);
                comlib_memFree(apndCb);
                return RC_NOK;
            }
            break;
    };

    (*rt_apndCb) = apndCb;

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndMakeFile(LoglibIntApndCb *apndCb)
{
    SINT ret = RC_OK;
    SINT i = 0;
    CHAR tmp[LOGLIB_LOG_PATH_MAX_LEN];
    struct stat fStat;

    if(apndCb->u.file.fp != NULL){
        fclose(apndCb->u.file.fp);
        apndCb->u.file.fp = NULL;
    }

    ret = stat(apndCb->u.file.fullLogPath, &fStat);
    if(ret == 0){ /* exist */
        if(fStat.st_size > apndCb->u.file.maxLogSize){
            for(i=24;i>=0;i--){
                /* if exist */
                if(i != 0){
                    comlib_strSNPrnt(apndCb->u.file.fullLogPath, LOGLIB_LOG_PATH_MAX_LEN,
                            "%s.%d", apndCb->u.file.fullLogPath, i);
                }

                ret = access(apndCb->u.file.fullLogPath,F_OK);
                if(ret != 0){
                    apndCb->u.file.fullLogPath[apndCb->u.file.fullLogPathLen] = '\0';
                    continue;
                }

                /* change */
                comlib_strSNPrnt(tmp, LOGLIB_LOG_PATH_MAX_LEN,
                        "%.*s.%d", 
                        apndCb->u.file.fullLogPathLen, 
                        apndCb->u.file.fullLogPath,
                        i+1);

                rename(apndCb->u.file.fullLogPath, tmp);

                apndCb->u.file.fullLogPath[apndCb->u.file.fullLogPathLen] = '\0';
            }/* end of for(i=24;i>=0;i--) */

            apndCb->u.file.logSize = 0;
        }/* if(fStat.st_size > loglibCb->maxLogSize) */
        else {
            apndCb->u.file.logSize = fStat.st_size;
        }
    }/* end of if(ret == 0) */

    apndCb->u.file.fp = fopen(apndCb->u.file.fullLogPath,"a");
    if(apndCb->u.file.fp == NULL){
        LOG_LOG(LOG_ERR,"Log file open failed(path=%.*s, error=%d:%s)\n", 
                apndCb->u.file.fullLogPathLen,
                apndCb->u.file.fullLogPath,
                errno, strerror(errno));
        return LOGERR_LOG_FILE_OPEN_FAILED; 
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndMake(LoglibIntApndCb *apndCb)
{
    SINT ret = RC_OK;
    TIMET curTm = 0;
    struct tm *curTms = NULL;

    curTm = time(NULL);
    curTms = localtime((CONST TIMET*)&curTm);

    comlib_memMemcpy(&apndCb->u.file.lastTms, curTms, sizeof(struct tm));

    apndCb->u.file.fullLogPathLen = comlib_strSNPrnt(apndCb->u.file.fullLogPath, 
            LOGLIB_LOG_PATH_MAX_LEN,
            "%.*s",apndCb->u.file.logPathLen, apndCb->u.file.logPath);
    apndCb->u.file.fullLogPathLen += comlib_strSNPrnt(&apndCb->u.file.fullLogPath[apndCb->u.file.fullLogPathLen], 
            LOGLIB_LOG_PATH_MAX_LEN - apndCb->u.file.fullLogPathLen, 
                                  "/%04d-%02d-%02d", (curTms->tm_year+1900), curTms->tm_mon+1, curTms->tm_mday);

    ret = comlib_fileFrceDir(apndCb->u.file.fullLogPath, 0755);
    if(ret != RC_OK){
        if(ret != COMERR_FILE_EXIST){
            LOG_LOG(LOG_ERR,"mkdir failed(path=%s, error=%d:%s)\n",apndCb->u.file.fullLogPath, errno, strerror(errno));
            return LOGERR_LOG_DIR_CRTE_FAILED;
        }
    }

    apndCb->u.file.fullLogPathLen += comlib_strSNPrnt(&apndCb->u.file.fullLogPath[apndCb->u.file.fullLogPathLen], 
            LOGLIB_LOG_PATH_MAX_LEN - apndCb->u.file.fullLogPathLen, 
                                  "/%.*s%02d.log",apndCb->u.file.nameLen, apndCb->u.file.name, curTms->tm_hour);

    ret = loglibInt_apndMakeFile(apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_ERR,"File make failed(path=%.*s,ret=%d)\n",
                apndCb->u.file.fullLogPathLen, 
                apndCb->u.file.fullLogPath, ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_apndWrite(LoglibIntApndCb *apndCb, CONST UINT lvl, CONST CHAR *logBuf, struct tm *curTms)
{
    SINT ret = RC_OK;
    CHAR strTm[100];

    if(apndCb->logLvlBit != 0){
        if((lvl == LOGLIB_LVL_ERR) &&
           ((apndCb->logLvlBit & LOGLIB_APND_DISP_ERR_LOG_BIT) == 0)){
            return RC_OK;
        }
        else if((lvl == LOGLIB_LVL_NOTY) &&
                ((apndCb->logLvlBit & LOGLIB_APND_DISP_NOTY_LOG_BIT) == 0)){
            return RC_OK;
        }
        else if((lvl == LOGLIB_LVL_DBG) &&
                ((apndCb->logLvlBit & LOGLIB_APND_DISP_DBG_LOG_BIT) == 0)){
            return RC_OK;
        }
    }/* end of if(apndCb->logLvlBit != 0) */

    switch (apndCb->apndType){
        case LOGLIB_APND_TYPE_FILE:
            {
                /* check */
                if((apndCb->u.file.fp == NULL) ||
                   (apndCb->u.file.lastTms.tm_year != curTms->tm_year) ||
                   (apndCb->u.file.lastTms.tm_mon != curTms->tm_mon) ||
                   (apndCb->u.file.lastTms.tm_mday != curTms->tm_mday) ||
                   (apndCb->u.file.lastTms.tm_hour != curTms->tm_hour) ||
                   (apndCb->u.file.logSize >= apndCb->u.file.maxLogSize)){
                    /* make new file */
                    ret = loglibInt_apndMake(apndCb);
                    if(ret != RC_OK){
                        LOG_LOG(LOG_ERR,"File Make failed(ret=%d)\n",ret);
                    }
                }

                strftime(strTm, 100, "%H:%M:%S",curTms);

                if(apndCb->u.file.fp != NULL){
                    fputs(logBuf, apndCb->u.file.fp);

                    fflush(apndCb->u.file.fp);
                }
                else {
                    fputs(logBuf, stderr);
                }

                apndCb->u.file.logSize += comlib_strGetLen(logBuf);
            }
            break;
        case LOGLIB_APND_TYPE_STDOUT:
            {
                if(apndCb->u.stdout.type == LOGLIB_STDOUT_TYPE_ERR){
                    fputs(logBuf, stderr);
                }
                else {
                    fputs(logBuf, stdout);
                }
            }
            break;
        case LOGLIB_APND_TYPE_SYSLOG:
            {
                UINT pri = 0;

                switch(lvl){
                    case LOGLIB_LVL_ERR:   pri = LOG_ERR;    /* syslog error */
                    case LOGLIB_LVL_NOTY:  pri = LOG_INFO;   /* syslog info */
                    case LOGLIB_LVL_DBG:   pri = LOG_DEBUG;  /* syslog debug */
                }
                syslog(pri | apndCb->u.syslog.fac,"%s", logBuf);
            }
            break;
    };/* switch (apndCb->type) */

    return RC_OK;
}

