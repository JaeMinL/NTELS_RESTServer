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

FT_PRIVATE VOID       thrd_clnThrd       (VOID *args);

FT_PRIVATE VOID thrd_clnThrd(VOID *args)
{
    SINT ret = RC_OK;
    CHAR *rcvDat = NULL;
    LoglibIntThrdCb *thrdCb = NULL;

    thrdCb = (LoglibIntThrdCb*)args;

    while(1){
        ret = thrlib_tqPop(&thrdCb->rcvTq, (VOID*)&rcvDat);
        if(ret == THRERR_TQ_EMPTY){
            break;
        }
        else if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Thread queue pop failed(ret=%d)\n",ret);
            break;
        }
        comlib_memFree(rcvDat);

    }/* end of while(1) */

    ret = loglibInt_apndDelAll(&thrdCb->apndInfo);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append delete failed(ret=%d)\n",ret);
    }

    ret = thrlib_tqDstry(&thrdCb->rcvTq);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Thread queue destory failed(ret=%d)\n",ret);
    }

    thrlib_mutxDstry(&thrdCb->mutx);
    thrlib_condDstry(&thrdCb->cond);

    comlib_memFree(thrdCb);

    return ;
}

FT_PUBLIC RT_RESULT loglibInt_thrdLogWrite(LoglibIntApndInfo *apndInfo, UINT lvl, CONST CHAR *fName, UINT fNameLen, 
                                           UINT line, CONST CHAR *logBuf, UINT logBufLen, 
                                           struct tm *curTms)
{
    SINT ret = RC_OK;
    UINT hdrLen = 0;
    ComlibLnkNode *lnkNode = NULL;
    LoglibIntApndCb *apndCb = NULL;
    UINT dispBufLen = 0;
    CHAR *dispBuf = NULL;

    dispBuf = apndInfo->logBuf;

    COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        apndCb = lnkNode->data;

        if(apndCb->apndType == LOGLIB_APND_TYPE_USR){
            UINT usrLvl = 0;
            CHAR *usrFName = NULL;
            CHAR usrLine = 0;

            if((apndCb->dispBit & LOGLIB_DISP_FILE_BIT)){
                usrFName = (CHAR*)fName;
            }
            if((apndCb->dispBit & LOGLIB_DISP_LINE_BIT)){
                usrLine = line;
            }
            if((apndCb->dispBit & LOGLIB_DISP_LVL_BIT)){
                usrLvl = lvl;
            }

            apndCb->u.usr.usrLogFunc(curTms, usrLvl, (CHAR*)usrFName, usrLine, (CHAR*)logBuf);
        }
        else {
            dispBufLen = 0;

            ret = loglibInt_msicPrntHdr(apndCb, lvl, curTms, (CHAR*)fName, fNameLen, line, 
                                        (CHAR*)dispBuf, (LOGLIB_LOG_MAX_BUF_LEN), 
                                        &hdrLen);

            dispBufLen += hdrLen;

            dispBufLen += snprintf(&dispBuf[dispBufLen], LOGLIB_LOG_MAX_BUF_LEN - dispBufLen,"%.*s", logBufLen, logBuf);

            ret = loglibInt_apndWrite(apndCb, lvl, dispBuf, curTms);
            if(ret != RC_OK){
                return ret;
            }
        }/* end of else */

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            break;
        }
    }/* end of while(1) */


    return RC_OK;
}

FT_PUBLIC VOID loglibInt_thrdMain(VOID *args)
{
    SINT ret = RC_OK;
    struct timespec wait;
    LoglibIntThrdCb *thrdCb = NULL;
    CHAR *rcvDat = NULL;
    CHAR *cur = NULL;
    UINT lvl = 0;
    LoglibIntApndInfo *apndInfo = NULL;
    LoglibIntThrdMsgHdr *msgHdr = NULL;
    LoglibIntThrdMsgAvpHdr *avpHdr = NULL;
    UINT fNameLen = 0;
    CHAR *fName = NULL;
    UINT line = 0;
    UINT logBufLen = 0;
    CHAR *logBuf = NULL;

    thrdCb = (LoglibIntThrdCb*)args;

    /* detach thread */
    thrlib_thrdDtch(thrdCb->tid);

    /* pthread only */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

    thrlib_mutxLock(&thrdCb->mutx);

    /* initialization */
    apndInfo = &thrdCb->apndInfo;

    thrlib_mutxUnlock(&thrdCb->mutx);

    /* cancel enable */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    thrlib_thrdSetCnclType(THR_THRD_CNCL_DEFERRED, NULL);

    THRLIB_THRDCLNUP_PUSH(thrd_clnThrd, thrdCb);

    while(1){
        thrlib_mutxLock(&thrdCb->mutx);

        ret = thrlib_tqPop(&thrdCb->rcvTq, (VOID*)&rcvDat);
        if(ret == THRERR_TQ_EMPTY){
            ret = comlib_timerGetTime(&wait);

            wait.tv_nsec += 1000000; /* 1 millisecond */
            if(wait.tv_nsec >= COM_TIMER_TICK_SEC){
                wait.tv_nsec -= COM_TIMER_TICK_SEC;
                wait.tv_sec += 1;
            }

            thrlib_condTmWait(&thrdCb->cond, &thrdCb->mutx, &wait);

            thrlib_mutxUnlock(&thrdCb->mutx);
            continue;
        }
        else if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Log data pop failed(ret=%d)\n",ret);
            thrlib_mutxUnlock(&thrdCb->mutx);
            continue;
        }

        thrlib_mutxUnlock(&thrdCb->mutx);

        /* data parsing process */
        msgHdr = (LoglibIntThrdMsgHdr *)rcvDat;
        cur = (CHAR*)&rcvDat[sizeof(LoglibIntThrdMsgHdr)];

        /* level */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        lvl = *(UINT*)cur;

        cur += avpHdr->msgLen;

        /* fName */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);

        fName = cur;
        fNameLen = avpHdr->msgLen;
        fNameLen--; /* /0 */

        cur += avpHdr->msgLen;

        /* line */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        line = *(UINT*)cur;

        cur += avpHdr->msgLen;

        /* log */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        logBuf = cur;
        logBufLen = avpHdr->msgLen;
        logBufLen--; /* /0 */

        /* message process */
        thrlib_mutxLock(&thrdCb->mutx);

        ret = loglibInt_thrdLogWrite(apndInfo, lvl, fName, fNameLen, 
                                     line, logBuf, logBufLen, 
                                     &msgHdr->msgTms);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Log write failed(ret=%d)\n",ret);
        }

        thrlib_mutxUnlock(&thrdCb->mutx);

        comlib_memFree(rcvDat);
    }/* end of while(1) */

    THRLIB_THRDCLNUP_POP(0);

    return ;
}

