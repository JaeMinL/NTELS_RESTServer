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
#include "rrllib.h"
#include "rrllib.x"

#include "rsvlib.h"
#include "rsvlib.x"
#include "rsvlibInt.h"
#include "rsvlibInt.x"

STATIC RsvlibIntGlobCb g_globCb;

STATIC BOOL g_initFlg = RC_FALSE;

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
    UINT i = 0;

    if(g_initFlg == RC_TRUE){
        return RC_OK;
    }

    for(i=0;i<RSV_MAX_SVR_CNT;i++){
        g_globCb.rsvlibCb[i] = NULL;
    }

    g_globCb.rsvlibCbCnt = 0;

    /* init rsvlibInt rule library */
    rrllib_globInit();

    /* set log */
    thrlib_mutxInit(&g_globCb.mutx);

    g_globCb.logLvl = RSV_NONE;
    g_globCb.logBufLen = 0;
    g_globCb.logFunc = NULL;

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

