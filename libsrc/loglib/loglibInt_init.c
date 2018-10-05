#include <stdio.h>
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

FT_PUBLIC RT_RESULT loglibInt_init(CHAR *name, LoglibCfg *cfg, LoglibIntMainCb **rt_mainCb)
{
    SINT ret = RC_OK;
    UINT nameLen = 0;
    LoglibCfg dfltCfg;
    LoglibIntMainCb *mainCb = NULL;
    LoglibIntApndCb *apndCb = NULL;
    LoglibCfg *cfgPtr = NULL;

    if(cfg == NULL){
        dfltCfg.wrType = LOGLIB_WR_TYPE_DIR; /* default : DIRECT */
        dfltCfg.dfltApndType = LOGLIB_APND_TYPE_STDOUT;
        dfltCfg.dfltApndCfg.u.stdout.type = LOGLIB_STDOUT_TYPE_ERR;
        dfltCfg.dfltLogLvl = LOGLIB_LVL_ERR;

        cfgPtr = &dfltCfg;
    }
    else {
        cfgPtr = cfg;

        GEN_CHK_ERR_RET((cfgPtr->wrType != LOGLIB_WR_TYPE_DIR) &&
                        (cfgPtr->wrType != LOGLIB_WR_TYPE_THRD),
                        LOG_LOG(LOG_INT_ERR,"Invalid write type(%d)\n",cfgPtr->wrType),
                        LOGERR_INVALID_WR_TYPE);

        if((cfgPtr->dfltApndType != LOGLIB_APND_TYPE_NONE) &&
           (cfgPtr->dfltApndType != LOGLIB_APND_TYPE_STDOUT) &&
           (cfgPtr->dfltApndType != LOGLIB_APND_TYPE_FILE) &&
           (cfgPtr->dfltApndType != LOGLIB_APND_TYPE_SYSLOG) &&
           (cfgPtr->dfltApndType != LOGLIB_APND_TYPE_RMT) &&
           (cfgPtr->dfltApndType != LOGLIB_APND_TYPE_USR)){
            return LOGERR_INVALID_APND_TYPE;
        }
    }

    mainCb = comlib_memMalloc(sizeof(LoglibIntMainCb));

    if(name != NULL){
        nameLen = comlib_strGetLen(name);
        if(nameLen >= (LOGLIB_LOG_NAME_MAX_LEN-1)){
            return LOGERR_LOG_NAME_LEN_TO_LONG;
        }

        comlib_strCpy(mainCb->name, name);

        comlib_strChgStrToUpper(name, nameLen, mainCb->name, nameLen);

        mainCb->name[nameLen] = '\0';

        mainCb->nameLen = nameLen;
    }
    else { /* default */
        mainCb->nameLen = 0;
    }

    mainCb->hNode.key.key = mainCb->name;
    mainCb->hNode.key.keyLen = mainCb->nameLen;
    mainCb->hNode.data = mainCb;

    mainCb->lnkNode.data = mainCb;

    mainCb->logLvl = cfgPtr->dfltLogLvl;

    thrlib_mutxInit(&mainCb->mutx);

    mainCb->wrType = cfgPtr->wrType;

    switch(cfgPtr->dfltApndType){
        case  LOGLIB_APND_TYPE_FILE:
            {
                LoglibApndCfg apndCfg;

                LOGLIB_INIT_APND_CFG(&apndCfg);

                apndCfg.u.file.logPath = cfgPtr->dfltApndCfg.u.file.logPath;
                apndCfg.u.file.name = cfgPtr->dfltApndCfg.u.file.name;
                apndCfg.u.file.maxLogSize = cfgPtr->dfltApndCfg.u.file.maxLogSize;

                ret = loglibInt_apndInit(cfgPtr->dfltApndType, "DEFAULT", &apndCfg, &apndCb);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"Loglib append init failed(ret=%d)\n",ret);
                    return ret;
                }

            }
            break;
        case  LOGLIB_APND_TYPE_STDOUT:
            {
                ret = loglibInt_apndInit(cfgPtr->dfltApndType, "DEFAULT", NULL, &apndCb);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"Loglib append init failed(ret=%d)\n",ret);
                    return ret;
                }
            }
            break;
        case  LOGLIB_APND_TYPE_SYSLOG:
            {
                ret = loglibInt_apndInit(cfgPtr->dfltApndType, "DEFAULT", NULL, &apndCb);
                if(ret != RC_OK){
                    LOG_LOG(LOG_INT_ERR,"Loglib append init failed(ret=%d)\n",ret);
                    return ret;
                }
            }
            break;
    };/* end of switch(cfgPtr.dfltFileIoType) */

    if(mainCb->wrType == LOGLIB_WR_TYPE_DIR){
        LoglibIntApndInfo *apndInfo =  NULL;

        apndInfo = &mainCb->u.dir.apndInfo;

        ret = comlib_lnkLstInit(&apndInfo->apndLL, ~0);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Append linked list init failed(ret=%d)\n",ret);
            return LOGERR_APND_LNLLST_INIT_FAIELD;
        }

        if(apndCb != NULL){
            ret = comlib_lnkLstInsertTail(&apndInfo->apndLL, &apndCb->lnkNode);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"Append insert failed(ret=%d)\n",ret);
                return LOGERR_APND_INSERT_FAILED;
            }
        }

    }
    else { /* thread  */
        LoglibIntThrdCb *thrdCb = NULL;
        LoglibIntApndInfo *apndInfo =  NULL;

        thrdCb = comlib_memMalloc(sizeof(LoglibIntThrdCb));

        apndInfo = &thrdCb->apndInfo;

        /* make thread control block*/
        thrlib_mutxInit(&thrdCb->mutx);

        thrlib_condInit(&thrdCb->cond);

        ret = thrlib_tqInit(&thrdCb->rcvTq, THR_TQ_LOCK_TYPE_LOCK_FREE, 8192);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Thread queue init failed(ret=%d)\n",ret);
            comlib_memFree(thrdCb);
            return LOGERR_THRD_QUE_INIT_FAILED;
        }

        ret = comlib_lnkLstInit(&apndInfo->apndLL, ~0);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"append linked list init failed(ret=%d)\n",ret);
            comlib_memFree(thrdCb);
            return ret;
        }

        if(apndCb != NULL){
            ret = comlib_lnkLstInsertTail(&apndInfo->apndLL, &apndCb->lnkNode);
            if(ret != RC_OK){
                LOG_LOG(LOG_INT_ERR,"Append insert failed(ret=%d)\n",ret);
                comlib_memFree(thrdCb);
                return LOGERR_APND_INSERT_FAILED;
            }
        }

        mainCb->u.thrd.sndTq = &thrdCb->rcvTq;
        mainCb->u.thrd.cond = &thrdCb->cond;
        mainCb->u.thrd.thrdCb = thrdCb;

        ret = thrlib_thrdCrte(&thrdCb->tid, NULL, loglibInt_thrdMain, thrdCb);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"log thread create failed(ret=%d)\n",ret);
            return LOGERR_THRD_CRTE_FAILED;
        }
    }/* end of else */

    (*rt_mainCb) = mainCb;

    return RC_OK;
}

