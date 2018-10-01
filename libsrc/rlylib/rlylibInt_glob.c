#include <stdio.h>
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
#include "rlylibInt.h"
#include "rlylibInt.x"

STATIC BOOL                        intGlobInitFlg = RC_FALSE;
STATIC RlylibIntGlobCb             intGlobCb;

FT_PUBLIC RT_RESULT rlylibInt_globGetInitFlg()
{
    return intGlobInitFlg;
}

/* lock free */
FT_PUBLIC RT_RESULT rlylibInt_globGetLogLvl()
{
    return intGlobCb.logLvl;
}

FT_PUBLIC ULONG rlylibInt_globGetCurTick()
{
    return intGlobCb.tmr.tick;
}

FT_PUBLIC RT_RESULT rlylibInt_globUpdCurTick()
{
    thrlib_mutxLock(&intGlobCb.mutx);

    EXEC_TMR_TICK((&intGlobCb.tmr));

    thrlib_mutxUnlock(&intGlobCb.mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_globLogPrnt(UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...)
{
    va_list ap;

    /* lock */
    thrlib_mutxLock(&intGlobCb.mutx);

    if(intGlobCb.logFunc == NULL){
        thrlib_mutxUnlock(&intGlobCb.mutx);
        return RC_OK;
    }

    va_start(ap, fmt);

    intGlobCb.logBufLen = vsnprintf(intGlobCb.logBuf, RLYLIB_MAX_LOG_BUF_LEN, fmt, ap);

    intGlobCb.logFunc(lvl, file, line, intGlobCb.logBuf);

    va_end(ap);

    thrlib_mutxUnlock(&intGlobCb.mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_globSetLogFunc(UINT lvl, RlylibLogFunc logFunc)
{
    if(rlylibInt_globGetInitFlg() != RC_TRUE){
        return RLYERR_GLOB_CB_NOT_INIT;
    }

    if(lvl > RLY_DBG){
        return RLYERR_INVALID_LOG_LEVEL;
    }

    thrlib_mutxLock(&intGlobCb.mutx);

    intGlobCb.logLvl = lvl;
    intGlobCb.logFunc = logFunc;

    thrlib_mutxUnlock(&intGlobCb.mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_globInit()
{
    SINT ret = RC_OK;

    if(intGlobInitFlg == RC_TRUE){
        return RC_OK;
    }

    thrlib_mutxInit(&intGlobCb.mutx);

    intGlobCb.logLvl = RLY_NONE;
    intGlobCb.logBufLen = 0;
    intGlobCb.logFunc = NULL;

    /* init timer */
    ret = comlib_timerInit(&intGlobCb.tmr, COM_TIMER_TYPE_100M);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Timer init failed(ret=%d)\n",ret);
        return RLYERR_TMR_INIT_FAILED;
    }

    rlylibInt_globUpdCurTick();

    intGlobInitFlg = RC_TRUE;

    return RC_OK;
}
