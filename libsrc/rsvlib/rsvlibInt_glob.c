#include <unistd.h>
#include <stdarg.h>
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
#include "rrllib.h"
#include "rrllib.x"

#include "rsvlib.h"
#include "rsvlib.x"
#include "rsvlibInt.h"
#include "rsvlibInt.x"

STATIC RsvlibIntGlobCb g_globCb;

STATIC BOOL g_initFlg = RC_FALSE;

#if 0
FT_PUBLIC RT_RESULT rsvlibInt_globInitLoglibCb(CHAR *logName, CHAR *logPath, LoglibCfg *cfg)
{
    LoglibCfg logCfg;
    SINT ret = RC_OK;

    LOGLIB_GLOB_INIT();

    if(logPath != NULL){
        ret = loglib_apiLoadCfg(&g_globCb.loglibCb, logPath, logName);
    }
    else {
        LOGLIB_INIT_CFG(&logCfg);

        logCfg.dfltLogLvl = LOGLIB_LVL_ERR;

        ret = loglib_apiInitLoglibCb(&g_globCb.loglibCb, &logCfg);
    }
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Loglib init failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    g_globCb.loglibInitFlg = RC_TRUE;
    return RC_OK;
}
#endif

#if 0
FT_PUBLIC LoglibCb* rsvlibInt_globGetLoglibCb()
{
    if(g_globCb.loglibInitFlg == RC_TRUE){
        return &g_globCb.loglibCb;
    }
    else {
        return NULL;
    }
}
#endif

FT_PUBLIC RT_RESULT rsvlibInt_globSetRsvlibIntCb(UINT id, RsvlibIntCb *rsvlibIntCb)
{
    if(id > RSV_MAX_SVR_CNT){
        RSV_LOG(RSV_ERR,"Invalid id(%d)\n",id);
        return RC_NOK;
    }

    if(g_globCb.rsvlibCb[id] != NULL){
        RSV_LOG(RSV_ERR,"RsvlibInt control block alerady exist(%d)\n", id);
        return RC_NOK;
    }

    g_globCb.rsvlibCb[id] = rsvlibIntCb;

    return RC_OK;
}

FT_PUBLIC RsvlibIntCb* rsvlibInt_globGetRsvlibIntCb(UINT id)
{
    if(id > RSV_MAX_SVR_CNT){
        RSV_LOG(RSV_ERR,"Invalid id(%d)\n",id);
        return NULL;
    }
    return g_globCb.rsvlibCb[id];
}

FT_PUBLIC RT_RESULT rsvlibInt_globInit()
{
#if 0
    SINT ret = RC_OK;
#endif
    UINT i = 0;
#if 0
    LoglibCfg logCfg;
#endif

    if(g_initFlg == RC_TRUE){
        return RC_OK;
    }

#if 0 
    g_globCb.loglibInitFlg = RC_FALSE;
#endif

    for(i=0;i<RSV_MAX_SVR_CNT;i++){
        g_globCb.rsvlibCb[i] = NULL;
    }

    g_globCb.rsvlibCbCnt = 0;

    /* init rsvlibInt rule library */
    rrllib_globInit();

#if 0
    rrllib_globSetLogFunc(RRL_ERR, rsvlibInt_mainLogPrnt);
    rrllib_globSetDispFunc(rsvlibInt_mainDispPrnt);
#endif

    /* set log */
    thrlib_mutxInit(&g_globCb.mutx);

    g_globCb.logLvl = RSV_NONE;
    g_globCb.logBufLen = 0;
    g_globCb.logFunc = NULL;
#if 0
    LOGLIB_INIT_CFG(&logCfg);

    ret = rsvlibInt_globInitLoglibCb("RSV", NULL, &logCfg);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Log init failed(ret=%d)\n",ret);
        return RC_NOK;
    }
#endif

    g_initFlg = RC_TRUE;

    return RC_OK;
}

/* lock free */
FT_PUBLIC RT_RESULT rsvlibInt_globGetLogLvl()
{
    return g_globCb.logLvl;
}

FT_PUBLIC RT_RESULT rsvlibInt_globSetLogFunc(UINT lvl, RsvlibLogFunc logFunc)
{
    if(g_initFlg != RC_TRUE){
        return RSVERR_GLOB_CB_NOT_INIT;
    }

    if(lvl > RSV_DBG){
        return RSVERR_INVALID_LOG_LEVEL;
    }

    thrlib_mutxLock(&g_globCb.mutx);

    g_globCb.logLvl = lvl;
    g_globCb.logFunc = logFunc;

    thrlib_mutxUnlock(&g_globCb.mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlibInt_globLogPrnt(UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...)
{
    va_list ap;

    /* lock */
    thrlib_mutxLock(&g_globCb.mutx);

    if(g_globCb.logFunc == NULL){
        thrlib_mutxUnlock(&g_globCb.mutx);
        return RC_OK;
    }

    va_start(ap, fmt);

    g_globCb.logBufLen = vsnprintf(g_globCb.logBuf, RSV_MAX_LOG_BUF_LEN, fmt, ap);

    g_globCb.logFunc(lvl, file, line, g_globCb.logBuf);

    va_end(ap);

    thrlib_mutxUnlock(&g_globCb.mutx);

    return RC_OK;
}

