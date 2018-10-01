#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"

#include "smd_user.h"
#include "smd_user.x"
#include "smulib.h"
#include "smulib.x"
#include "smulibInt.h"
#include "smulibInt.x"

STATIC BOOL intGlobInitFlg = RC_FALSE;
STATIC SmulibIntGlobCb g_intGlobCb;

FT_PUBLIC RT_RESULT smulibInt_globGetInitFlg()
{
    if(intGlobInitFlg == RC_TRUE){
        return RC_OK;
    }

    return RC_NOK;
}
FT_PUBLIC SmulibIntMainBlkInfo* smulibInt_globGetMainBlkInfo()
{
    return &g_intGlobCb.mainBlkInfo;
}

FT_PUBLIC ThrlibTq* smulibInt_globGetSndTq()
{
    return &g_intGlobCb.sndTq;

}

FT_PUBLIC SmulibIntThrdBlkInfo *smulibInt_globGetThrdBlkInfo()
{
    return &g_intGlobCb.thrdBlkInfo;
}

FT_PUBLIC ULONG smulibInt_globGetCurTick()
{
    return g_intGlobCb.curTick;
}

FT_PUBLIC VOID smulibInt_globUpdCurTick(ULONG curTick)
{
    g_intGlobCb.curTick = curTick;
}

FT_PUBLIC SmulibIntSigInfo* smulibInt_globGetSigInfo()
{
    return &g_intGlobCb.sigInfo;
}

FT_PUBLIC RT_RESULT smulibInt_globSetLogFunc(UINT lvl, SmulibLogFunc logFunc)
{
    if(intGlobInitFlg == RC_FALSE){
        return SMUERR_GLOB_CB_NOT_INIT;
    }

    if(lvl > SMU_DBG){
        return SMUERR_INVALID_LOG_LEVEL;
    }

    thrlib_mutxLock(&g_intGlobCb.logInfo.mutx);

    g_intGlobCb.logInfo.logLvl = lvl;
    g_intGlobCb.logInfo.logFunc = logFunc;

    thrlib_mutxUnlock(&g_intGlobCb.logInfo.mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_globInit()
{
    SINT ret = RC_OK;

    if(intGlobInitFlg == RC_TRUE){
        return RC_OK;
    }

    g_intGlobCb.curTick = 0;
    g_intGlobCb.mainBlkInfo.lstUpdTick = 0;
    g_intGlobCb.mainBlkInfo.status = SMU_MAIN_BLK_STA_ACT;

    ret = comlib_lnkLstInit(&g_intGlobCb.thrdBlkInfo.blkLL, ~0);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block init failed(ret=%d)\n",ret);
        return SMUERR_THRD_BLK_LNKLST_INIT_FAILED;
    }

    thrlib_mutxInit(&g_intGlobCb.logInfo.mutx);
    g_intGlobCb.logInfo.logLvl = 0;
    g_intGlobCb.logInfo.logBufLen = 0;
    g_intGlobCb.logInfo.logFunc = NULL;

    ret = comlib_hashTblInit(&g_intGlobCb.sigInfo.sigHT, 8192, RC_FALSE, COM_HASH_TYPE_UINT, NULL);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Signal hash table init failed(ret=%d)\n",ret);
        return SMUERR_SIG_HASHTBL_INIT_FAILED;
    }

    thrlib_mutxInit(&g_intGlobCb.sigInfo.mutx);

    ret = thrlib_tqInit(&g_intGlobCb.sndTq, THR_TQ_LOCK_TYPE_LOCKED, 8192);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread queue init failed(ret=%d)\n",ret);
        return SMUERR_SNDQ_INIT_FAILED;
    }

    intGlobInitFlg = RC_TRUE;

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_globKeepalive()
{
    g_intGlobCb.mainBlkInfo.lstUpdTick = g_intGlobCb.curTick;

    return RC_OK;
}

FT_PUBLIC UINT smulibInt_globGetLogLvl()
{
    SmulibIntLogInfo *logInfo = NULL;

    logInfo = &g_intGlobCb.logInfo;

    return logInfo->logLvl;
}

FT_PUBLIC RT_RESULT smulibInt_globSetLogLvl(UINT logLvl)
{
    SmulibIntLogInfo *logInfo = NULL;

    if((logLvl < SMU_ERR) || (logLvl > SMU_DBG)){
        return SMUERR_INVALID_LOG_LEVEL;
    }

    logInfo = &g_intGlobCb.logInfo;

    logInfo->logLvl = logLvl;

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_globLogPrnt(UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...)
{
    va_list ap;
    SmulibIntLogInfo *logInfo = NULL;

    logInfo = &g_intGlobCb.logInfo;

    /* lock */
    thrlib_mutxLock(&logInfo->mutx);

    if(logInfo->logFunc == NULL){
        thrlib_mutxUnlock(&logInfo->mutx);
        return RC_OK;
    }

    va_start(ap, fmt);

    logInfo->logBufLen = vsnprintf(logInfo->logBuf, SMULIB_MAX_LOG_BUF_LEN, fmt, ap);
    if(logInfo->logBufLen >= SMULIB_MAX_LOG_BUF_LEN){
        logInfo->logBufLen = SMULIB_MAX_LOG_BUF_LEN-1;
        logInfo->logBuf[SMULIB_MAX_LOG_BUF_LEN-1] = '\0';
    }

    logInfo->logFunc(lvl, file, line, logInfo->logBuf);

    va_end(ap);

    thrlib_mutxUnlock(&logInfo->mutx);

    return RC_OK;
}

