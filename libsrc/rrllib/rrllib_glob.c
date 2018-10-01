#include <stdio.h>
#include <stdarg.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

STATIC BOOL         intGlobInitFlg = RC_FALSE;
STATIC RrllibGlobCb intGlobCb;

FT_PUBLIC RT_RESULT rrllib_globGetLogLvl()
{
    return intGlobCb.logLvl;
}

FT_PUBLIC BOOL rrllib_globGetInitFlg()
{
    return intGlobInitFlg; 
}

FT_PUBLIC RT_RESULT rrllib_globSetLogFunc(UINT lvl, RrllibLogFunc logFunc)
{
    if(rrllib_globGetInitFlg() != RC_TRUE){
        return RRLERR_GLOB_CB_NOT_INIT;
    }

    if(lvl > RRL_DBG){
        return RRLERR_INVALID_LOG_LEVEL;
    }

    intGlobCb.logLvl = lvl;
    intGlobCb.logFunc = logFunc;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_globSetDispFunc(RrllibDispFunc dispFunc)
{
    intGlobCb.dispFunc = dispFunc;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_globLogPrnt(UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...)
{
    va_list ap;

    if(intGlobCb.logFunc == NULL){
        return RC_OK;
    }

    va_start(ap, fmt);

    intGlobCb.tmpBufLen = vsnprintf(intGlobCb.tmpBuf, RRLLIB_MAX_TMP_BUF_LEN, fmt, ap);

    intGlobCb.logFunc(lvl, file, line, intGlobCb.tmpBuf);

    va_end(ap);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_globDispPrnt(CONST CHAR *fmt,...)
{
    va_list ap;

    va_start(ap, fmt);

    if(intGlobCb.dispFunc == NULL){
        vfprintf(stderr,fmt, ap);
        goto goto_dispEnd;
    }

    intGlobCb.tmpBufLen = vsnprintf(intGlobCb.tmpBuf, RRLLIB_MAX_TMP_BUF_LEN, fmt, ap);

    intGlobCb.dispFunc(intGlobCb.tmpBuf);

goto_dispEnd:
    va_end(ap);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_globInit()
{
    if(intGlobInitFlg == RC_TRUE){
        return RC_OK;
    }

    intGlobCb.logLvl = 0;
    intGlobCb.tmpBufLen = 0;
    intGlobCb.logFunc = NULL;
    intGlobCb.dispFunc = NULL;

    intGlobInitFlg = RC_TRUE;

    return RC_OK;
}

