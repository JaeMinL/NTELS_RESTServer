#include <unistd.h>
#include <string.h>
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

FT_PUBLIC CHAR* loglibInt_miscGetStrLvl(UINT lvl)
{
    switch(lvl){
        case LOGLIB_LVL_ERR:
            {
                return "ERR ";
            }
            break;
        case LOGLIB_LVL_NOTY:
            {
                return "NOTY";
            }
            break;
        case LOGLIB_LVL_DBG:
            {
                return "DBG ";
            }
            break;
        default:
            {
                return "UNKN";
            }
            break;
    };
}

FT_PUBLIC RT_RESULT loglibInt_msicPrntHdr(LoglibIntApndCb *apndCb, UINT lvl, struct tm *curTms, 
                                          CONST CHAR *fName, UINT fNameLen, UINT line, 
                                          CHAR *rt_logBuf, UINT maxLogBufLen, UINT *rt_logBufLen)
{
    CHAR strTm[100];
    UINT logBufLen = 0;
    CHAR *logBuf = NULL;

    logBuf = rt_logBuf;

    strftime(strTm, 100, "%H:%M:%S",curTms);

    if(apndCb->dispBit & LOGLIB_DISP_LVL_BIT){
        logBufLen += snprintf(&logBuf[logBufLen], maxLogBufLen - logBufLen, 
                              "[%s]",loglibInt_miscGetStrLvl(lvl));
        if(maxLogBufLen <= logBufLen){
            logBuf[maxLogBufLen] = '\0';
            logBufLen = maxLogBufLen-1;
        }
    }

    if(apndCb->dispBit & LOGLIB_DISP_TIME_BIT){
        logBufLen += snprintf(&logBuf[logBufLen], maxLogBufLen - logBufLen,
                              "[%s] ", strTm);
        if(maxLogBufLen <= logBufLen){
            logBuf[maxLogBufLen] = '\0';
            logBufLen = maxLogBufLen-1;
        }
    }

    if((apndCb->dispBit & LOGLIB_DISP_FILE_BIT) &&
       (apndCb->dispBit & LOGLIB_DISP_LINE_BIT)){
        CONST CHAR *fNamePtr = NULL;

        fNamePtr = (strrchr(fName, '/') ? strrchr(fName, '/') + 1 : fName);

        logBufLen += snprintf(&logBuf[logBufLen], maxLogBufLen - logBufLen, 
                              "[%s:%d] ", fNamePtr, line);
        if(maxLogBufLen <= logBufLen){
            logBuf[maxLogBufLen] = '\0';
            logBufLen = maxLogBufLen-1;
        }
    }
    else if(((apndCb->dispBit & LOGLIB_DISP_FILE_BIT) == 0) &&
            (apndCb->dispBit & LOGLIB_DISP_LINE_BIT)){
        logBufLen += snprintf(&logBuf[logBufLen], maxLogBufLen - logBufLen,
                              "[%d] ", line);
        if(maxLogBufLen <= logBufLen){
            logBuf[maxLogBufLen] = '\0';
            logBufLen = maxLogBufLen-1;
        }
    }
    else if((apndCb->dispBit & LOGLIB_DISP_FILE_BIT) &&
            ((apndCb->dispBit & LOGLIB_DISP_LINE_BIT) == 0)){
        CONST CHAR *fNamePtr = NULL;

        fNamePtr = (strrchr(fName, '/') ? strrchr(fName, '/') + 1 : fName);

        logBufLen += snprintf(&logBuf[logBufLen], maxLogBufLen - logBufLen,
                              "[%s] ", fNamePtr);
        if(maxLogBufLen <= logBufLen){
            logBuf[maxLogBufLen] = '\0';
            logBufLen = maxLogBufLen-1;
        }
    }

    (*rt_logBufLen) = logBufLen;
    return RC_OK;
}

