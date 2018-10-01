#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

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

typedef struct ApiDispHdrCtrl      ApiDispHdrCtrl;
typedef struct ApiApndCfg          ApiApndCfg;

typedef RT_RESULT (*ApiApndInfoFunc)  (LoglibIntApndInfo *, VOID *arg); /* apndInfoFunc(apndInfo, arg) */

struct ApiDispHdrCtrl{
    UINT              apndNameLen;
    CHAR              *apndName;
    U_32              dispBit;
};

struct ApiApndCfg{
    UINT              nameLen;
    CHAR              *name;
    UINT              type;
    UINT              logLvlBit;
    LoglibApndCfg     *apndCfg;
};

FT_PRIVATE RT_RESULT     api_getCallApndInfoFunc     (LoglibIntMainCb *mainCb, ApiApndInfoFunc func, VOID *arg);
FT_PRIVATE RT_RESULT     api_setDispHdr              (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_SetDispHdrToPrev        (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_getDispHdr              (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_setAllDispHdr           (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_setAllDispHdrToPrev     (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_regApnd                 (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_deregApnd               (LoglibIntApndInfo *apndInfo, VOID *arg);
FT_PRIVATE RT_RESULT     api_setApndLogLvl           (LoglibIntApndInfo *apndInfo, VOID *arg);

FT_PRIVATE RT_RESULT api_setDispHdr(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiDispHdrCtrl *dispHdrCtrl = NULL;
    LoglibIntApndCb *apndCb = NULL;

    dispHdrCtrl = arg;

    /* find */
    ret = loglibInt_apndFind(apndInfo, dispHdrCtrl->apndName, dispHdrCtrl->apndNameLen, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append not exist(name=%.*s)\n", dispHdrCtrl->apndNameLen, dispHdrCtrl->apndName);
        return LOGERR_APND_NOT_EXIST;
    }
    else {
        loglibInt_apndSetDispBit(apndCb, dispHdrCtrl->dispBit);
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_SetDispHdrToPrev(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiDispHdrCtrl *dispHdrCtrl = NULL;
    LoglibIntApndCb *apndCb = NULL;

    dispHdrCtrl = arg;

    ret = loglibInt_apndFind(apndInfo, dispHdrCtrl->apndName, dispHdrCtrl->apndNameLen, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append not exist(name=%.*s)\n", dispHdrCtrl->apndNameLen, dispHdrCtrl->apndName);
        return LOGERR_APND_NOT_EXIST;
    }
    else {
        loglibInt_apndSetPrevDispBit(apndCb);
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_getDispHdr(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiDispHdrCtrl *dispHdrCtrl = NULL;
    LoglibIntApndCb *apndCb = NULL;

    dispHdrCtrl = arg;

    /* find */
    ret = loglibInt_apndFind(apndInfo, dispHdrCtrl->apndName, dispHdrCtrl->apndNameLen, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append not exist(name=%.*s)\n", dispHdrCtrl->apndNameLen, dispHdrCtrl->apndName);
        return LOGERR_APND_NOT_EXIST;
    }
    else {
        dispHdrCtrl->dispBit = loglibInt_apndGetDispBit(apndCb);

    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_setAllDispHdr(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    ComlibLnkNode *lnkNode = NULL;
    LoglibIntApndCb *apndCb = NULL;
    ApiDispHdrCtrl *dispHdrCtrl = NULL;

    dispHdrCtrl = arg;

    COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
    if(lnkNode != NULL){
        while(1){
            apndCb = lnkNode->data;

            loglibInt_apndSetDispBit(apndCb, dispHdrCtrl->dispBit);

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }/* end of if(lnkNode != NULL) */

    return RC_OK;
}


FT_PRIVATE RT_RESULT api_getCallApndInfoFunc(LoglibIntMainCb *mainCb, ApiApndInfoFunc func, VOID *arg)
{
    SINT ret = RC_OK;
    LoglibIntApndInfo *apndInfo = NULL;

    if(mainCb->wrType == LOGLIB_WR_TYPE_DIR){
        thrlib_mutxLock(&mainCb->mutx);

        apndInfo = &mainCb->u.dir.apndInfo;

        ret = func(apndInfo, arg);
        if(ret != RC_OK){
            thrlib_mutxUnlock(&mainCb->mutx);
            return ret;
        }

        thrlib_mutxUnlock(&mainCb->mutx);
    }
    else { /* thread */
        LoglibIntThrdCb *thrdCb = NULL;

        thrlib_mutxLock(&mainCb->mutx);

        thrdCb = mainCb->u.thrd.thrdCb;

        thrlib_mutxLock(&thrdCb->mutx);

        apndInfo = &thrdCb->apndInfo;

        ret = func(apndInfo, arg);
        if(ret != RC_OK){
            thrlib_mutxUnlock(&thrdCb->mutx);
            thrlib_mutxUnlock(&mainCb->mutx);
            return ret;
        }

        thrlib_mutxUnlock(&thrdCb->mutx);
        thrlib_mutxUnlock(&mainCb->mutx);
    }
    return RC_OK;
}

FT_PRIVATE RT_RESULT api_setAllDispHdrToPrev(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    ComlibLnkNode *lnkNode = NULL;
    LoglibIntApndCb *apndCb = NULL;

    COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
    if(lnkNode != NULL){
        while(1){
            apndCb = lnkNode->data;

            loglibInt_apndSetPrevDispBit(apndCb);

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }/* end of if(lnkNode != NULL) */

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_regApnd(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiApndCfg *apndCfg = NULL;
    LoglibIntApndCb *apndCb = NULL;

    apndCfg = arg;

    /* find */
    ret = loglibInt_apndFind(apndInfo, apndCfg->name, apndCfg->nameLen, NULL);
    if(ret == RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append already exist(name=%.*s)\n", apndCfg->nameLen, apndCfg->name);
        return LOGERR_APND_ALREADY_EXIST;
    }

    ret = loglibInt_apndInit(apndCfg->type, apndCfg->name, apndCfg->apndCfg, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Log append reg failed(ret=%d)\n",ret);
        return ret;
    }

    ret = comlib_lnkLstInsertTail(&apndInfo->apndLL, &apndCb->lnkNode);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Log append insert failed(ret=%d)\n",ret);
        return LOGERR_APND_INSERT_FAILED;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_deregApnd(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiApndCfg *apndCfg = NULL;
    LoglibIntApndCb *apndCb = NULL;

    apndCfg = arg;

    /* find */
    ret = loglibInt_apndFind(apndInfo, apndCfg->name, apndCfg->nameLen, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append not exist(name=%.*s)\n", apndCfg->nameLen, apndCfg->name);
        return LOGERR_APND_NOT_EXIST;
    }

    ret = loglibInt_apndDel(apndInfo, apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Log append delete failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT api_setApndLogLvl(LoglibIntApndInfo *apndInfo, VOID *arg)
{
    SINT ret = RC_OK;
    ApiApndCfg *apndCfg = NULL;
    LoglibIntApndCb *apndCb = NULL;

    apndCfg = arg;

    /* find */
    ret = loglibInt_apndFind(apndInfo, apndCfg->name, apndCfg->nameLen, &apndCb);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append not exist(name=%.*s)\n", apndCfg->nameLen, apndCfg->name);
        return LOGERR_APND_NOT_EXIST;
    }

    apndCb->logLvlBit = apndCfg->logLvlBit;

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiGlobInit(UINT bit)
{
    loglibInt_globSetDispEnv(bit);

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiLoadCfg(LoglibCb *loglibCb, CHAR *cfgFile, CHAR *name)
{
    SINT ret = RC_OK;

    ret = loglibInt_cfgLoadCfg(loglibCb, cfgFile, name);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Config load failed(ret=%d, cfg=%s)\n",ret, cfgFile);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiInitLoglibCb(LoglibCb *loglibCb, LoglibCfg *cfg)
{
    SINT ret = RC_OK;

    loglibCb->mainCb = comlib_memMalloc(sizeof(LoglibIntMainCb));

    if(cfg == NULL){
        loglibCb->logLvl = LOGLIB_LVL_ERR;
    }
    else {
        loglibCb->logLvl = cfg->dfltLogLvl;
    }

    ret = loglibInt_init(loglibCb->mainCb, cfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Loglib init failed(ret=%d\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiDstryLoglibCb(LoglibCb *loglibCb)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;

    mainCb = loglibCb->mainCb;

    thrlib_mutxLock(&mainCb->mutx);

    if(mainCb->wrType == LOGLIB_WR_TYPE_DIR){
        ret = loglibInt_apndDelAll(&mainCb->u.dir.apndInfo);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Append delete failed(ret=%d)\n",ret);
            return ret;
        }
    }
    else {
        ret = thrlib_thrdCancel(mainCb->u.thrd.thrdCb->tid);
    }

    thrlib_mutxUnlock(&mainCb->mutx);

    thrlib_mutxDstry(&mainCb->mutx);

    comlib_memFree(mainCb);

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiSetDispHdr(LoglibCb *loglibCb, CHAR *apndName, U_32 dispBit)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;
    ApiDispHdrCtrl dispHdrCtrl;

    mainCb = loglibCb->mainCb;

    dispHdrCtrl.apndNameLen = comlib_strGetLen(apndName);
    dispHdrCtrl.apndName = apndName;
    dispHdrCtrl.dispBit = dispBit;

    ret = api_getCallApndInfoFunc(mainCb, api_setDispHdr, &dispHdrCtrl);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiSetDispHdrToPrev(LoglibCb *loglibCb, CHAR *apndName)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;
    ApiDispHdrCtrl dispHdrCtrl;

    mainCb = loglibCb->mainCb;

    dispHdrCtrl.apndNameLen = comlib_strGetLen(apndName);
    dispHdrCtrl.apndName = apndName;
    dispHdrCtrl.dispBit = 0;

    ret = api_getCallApndInfoFunc(mainCb, api_SetDispHdrToPrev, &dispHdrCtrl);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiGetDispHdr(LoglibCb *loglibCb, CHAR *apndName, U_32 *rt_dispBit)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;
    ApiDispHdrCtrl dispHdrCtrl;

    mainCb = loglibCb->mainCb;

    dispHdrCtrl.apndName = apndName;
    dispHdrCtrl.apndNameLen = comlib_strGetLen(apndName);
    dispHdrCtrl.dispBit = 0;

    ret = api_getCallApndInfoFunc(mainCb, api_getDispHdr, &dispHdrCtrl);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    (*rt_dispBit) = dispHdrCtrl.dispBit;

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiSetAllDispHdr(LoglibCb *loglibCb, U_32 dispBit)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;
    ApiDispHdrCtrl dispHdrCtrl;

    dispHdrCtrl.apndNameLen = 0;
    dispHdrCtrl.apndName = NULL;
    dispHdrCtrl.dispBit = dispBit;

    mainCb = loglibCb->mainCb;

    ret = api_getCallApndInfoFunc(mainCb, api_setAllDispHdr, &dispHdrCtrl);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiSetAllDispHdrToPrev(LoglibCb *loglibCb)
{
    SINT ret = RC_OK;
    LoglibIntMainCb *mainCb = NULL;

    mainCb = loglibCb->mainCb;

    ret = api_getCallApndInfoFunc(mainCb, api_setAllDispHdrToPrev, NULL);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC U_32 loglib_apiGetDfltDispHdr(LoglibCb *loglibCb)
{
    return loglibInt_globGetDispEnv();
}

FT_PUBLIC RT_RESULT loglib_apiSetLogLvl(LoglibCb *loglibCb, UINT lvl)
{
    GEN_CHK_ERR_RET((lvl != LOGLIB_LVL_NONE) && (lvl != LOGLIB_LVL_ERR) &&
                    (lvl != LOGLIB_LVL_NOTY) && (lvl != LOGLIB_LVL_DBG),
                    LOG_LOG(LOG_INT_ERR,"Invalid Log level(lvl=%d)\n",lvl),
                    LOGERR_INVLAID_LOG_LVL);

    loglibCb->logLvl = lvl;

    return RC_OK;
}

FT_PUBLIC INLINE UINT loglib_apiGetLogLvl(LoglibCb *loglibCb)
{
    return loglibCb->logLvl;
}

FT_PUBLIC RT_RESULT loglib_apiRegApnd(LoglibCb *loglibCb, CHAR *name, UINT type, LoglibApndCfg *apndCfg)
{
    SINT ret = RC_OK;
    ApiApndCfg apiApndCfg;
    LoglibIntMainCb *mainCb = NULL;

    mainCb = loglibCb->mainCb;

    apiApndCfg.nameLen = comlib_strGetLen(name);
    apiApndCfg.name = name;
    apiApndCfg.apndCfg = apndCfg;
    apiApndCfg.logLvlBit = 0;
    apiApndCfg.type = type;

    ret = api_getCallApndInfoFunc(mainCb, api_regApnd, &apiApndCfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiDeregApnd(LoglibCb *loglibCb, CHAR *name)
{
    SINT ret = RC_OK;
    ApiApndCfg apiApndCfg;
    LoglibIntMainCb *mainCb = NULL;

    mainCb = loglibCb->mainCb;

    apiApndCfg.nameLen = comlib_strGetLen(name);
    apiApndCfg.name = name;
    apiApndCfg.apndCfg = NULL;
    apiApndCfg.logLvlBit = 0;
    apiApndCfg.type = 0;

    ret = api_getCallApndInfoFunc(mainCb, api_deregApnd, &apiApndCfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiSetApndLogLvl(LoglibCb *loglibCb, CHAR *name, U_32 logLvlBit)
{
    SINT ret = RC_OK;
    ApiApndCfg apiApndCfg;
    LoglibIntMainCb *mainCb = NULL;

    mainCb = loglibCb->mainCb;

    apiApndCfg.nameLen = comlib_strGetLen(name);
    apiApndCfg.name = name;
    apiApndCfg.apndCfg = NULL;
    apiApndCfg.logLvlBit = logLvlBit;
    apiApndCfg.type = 0;

    ret = api_getCallApndInfoFunc(mainCb, api_setApndLogLvl, &apiApndCfg);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Append information call failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglib_apiLogWrite(LoglibCb *loglibCb, UINT lvl, CONST CHAR *fName, UINT line, CONST CHAR *fmt, ...)
{
    SINT ret = RC_OK;
    UINT hdrLen = 0;
    UINT fNameLen = 0;
    TIMET curTm = 0;
    struct tm *curTms = NULL;
    va_list ap;
    LoglibIntMainCb *mainCb = NULL;
    UINT logBufLen = 0;
    CHAR *thrdMsg = NULL;
    CHAR *logBuf = NULL;

    if(lvl > loglibCb->logLvl) {
        thrlib_mutxUnlock(&mainCb->mutx);
        return RC_OK;
    }

    mainCb = (LoglibIntMainCb*)loglibCb->mainCb;

    thrlib_mutxLock(&mainCb->mutx);

    /* make string */
    if(mainCb->wrType == LOGLIB_WR_TYPE_DIR){
        logBuf = mainCb->u.dir.apndInfo.logBuf;
    }
    else {
        fNameLen = comlib_strGetLen(fName);
        thrdMsg = comlib_memMalloc(sizeof(LoglibIntThrdMsgHdr) +
                                   (sizeof(LoglibIntThrdMsgAvpHdr) + sizeof(UINT)) +
                                   (sizeof(LoglibIntThrdMsgAvpHdr) + fNameLen+1) +
                                   (sizeof(LoglibIntThrdMsgAvpHdr) + sizeof(UINT)) +
                                   (sizeof(LoglibIntThrdMsgAvpHdr) + LOGLIB_LOG_MAX_BUF_LEN));
    }

    curTm = time(NULL);
    curTms = localtime((CONST TIMET*)&curTm);

    /* write */
    if(mainCb->wrType == LOGLIB_WR_TYPE_DIR){
        ComlibLnkNode *lnkNode = NULL;
        LoglibIntApndInfo *apndInfo = NULL;
        LoglibIntApndCb *apndCb = NULL;

        apndInfo = &mainCb->u.dir.apndInfo;
        COM_GET_LNKLST_FIRST(&apndInfo->apndLL, lnkNode);
        if(lnkNode == NULL){
            thrlib_mutxUnlock(&mainCb->mutx);
            return RC_OK;
        }

        while(1){
            apndCb = lnkNode->data;

            if(apndCb->apndType == LOGLIB_APND_TYPE_USR){
                UINT usrLvl = 0;
                CONST CHAR *usrFName = NULL;
                CHAR usrLine = 0;

                if((apndCb->dispBit & LOGLIB_DISP_FILE_BIT)){
                    usrFName = fName;
                }
                if((apndCb->dispBit & LOGLIB_DISP_LINE_BIT)){
                    usrLine = line;
                }
                if((apndCb->dispBit & LOGLIB_DISP_LVL_BIT)){
                    usrLvl = lvl;
                }

                va_start(ap, fmt);

                logBufLen = 0;
                logBufLen += vsnprintf(&logBuf[logBufLen], LOGLIB_LOG_MAX_BUF_LEN - logBufLen, fmt, ap);
                if(LOGLIB_LOG_MAX_BUF_LEN <= logBufLen){
                    logBufLen = LOGLIB_LOG_MAX_BUF_LEN-1;
                    logBuf[LOGLIB_LOG_MAX_BUF_LEN] = '\0';
                }

                va_end(ap);

                apndCb->u.usr.usrLogFunc(curTms, usrLvl, usrFName, usrLine, logBuf);
            }/* if(apndCb->type == LOGLIB_APND_TYPE_USR) */
            else {
                /* make display buffer */
                va_start(ap, fmt);

                logBufLen = 0;

                ret = loglibInt_msicPrntHdr(apndCb, lvl, curTms, fName, comlib_strGetLen(fName), line, 
                                            &logBuf[logBufLen], (LOGLIB_LOG_MAX_BUF_LEN - logBufLen), 
                                            &hdrLen);

                logBufLen += hdrLen;

                logBufLen += vsnprintf(&logBuf[logBufLen], LOGLIB_LOG_MAX_BUF_LEN - logBufLen, fmt, ap);
                if(LOGLIB_LOG_MAX_BUF_LEN <= logBufLen){
                    logBufLen = LOGLIB_LOG_MAX_BUF_LEN-1;
                    logBuf[LOGLIB_LOG_MAX_BUF_LEN] = '\0';
                }

                va_end(ap);

                ret = loglibInt_apndWrite(apndCb, lvl, logBuf, curTms);
                if(ret != RC_OK){
                    thrlib_mutxUnlock(&mainCb->mutx);
                    return ret;
                }
            }/* end of else */

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }
    else { /* thread */
        CHAR *cur = NULL;
        LoglibIntThrdMsgHdr *msgHdr = NULL;
        LoglibIntThrdMsgAvpHdr *avpHdr = NULL;

        msgHdr = (LoglibIntThrdMsgHdr*)thrdMsg;

        /* make thread message */
        cur = &thrdMsg[sizeof(LoglibIntThrdMsgHdr)];

        /* level */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        avpHdr->cmdCode = LOGLIB_THRD_MSG_AVP_CODE_LVL;
        avpHdr->msgLen = sizeof(UINT);
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        comlib_memMemcpy(cur, &lvl, sizeof(UINT));
        cur += sizeof(UINT);

        /* File Name */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        avpHdr->cmdCode = LOGLIB_THRD_MSG_AVP_CODE_FNAME;
        avpHdr->msgLen = fNameLen+1;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        comlib_strNCpy(cur, fName, fNameLen);
        cur[fNameLen] = '\0';
        cur += (fNameLen + 1);

        /* line */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        avpHdr->cmdCode = LOGLIB_THRD_MSG_AVP_CODE_LINE;
        avpHdr->msgLen = sizeof(UINT);
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        comlib_memMemcpy(cur, &line, sizeof(UINT));
        cur += sizeof(UINT);

        /* data */
        avpHdr = (LoglibIntThrdMsgAvpHdr*)cur;
        avpHdr->cmdCode = LOGLIB_THRD_MSG_AVP_CODE_DATA;
        cur += sizeof(LoglibIntThrdMsgAvpHdr);
        logBuf = cur;

        /* make buffer */
        va_start(ap, fmt);

        logBufLen = snprintf(logBuf, LOGLIB_LOG_MAX_BUF_LEN, fmt, ap);
        if(LOGLIB_LOG_MAX_BUF_LEN <= logBufLen){
            logBufLen = LOGLIB_LOG_MAX_BUF_LEN-1;
            logBuf[LOGLIB_LOG_MAX_BUF_LEN] = '\0';
        }

        va_end(ap);

        logBuf[logBufLen] = '\0';
        logBufLen++; /* \0 */
        avpHdr->msgLen = logBufLen;
        cur += logBufLen;

        /* make header */
        msgHdr->cmdCode = LOGLIB_THRD_MSG_CODE_LOG;
        msgHdr->msgLen = cur - thrdMsg;
        comlib_memMemcpy(&msgHdr->msgTms, curTms, sizeof(struct tm));

        ret = thrlib_tqPush(mainCb->u.thrd.sndTq, (VOID*)msgHdr);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Log message push failed(ret=%d)\n",ret);
            comlib_memFree(thrdMsg);
            thrlib_mutxUnlock(&mainCb->mutx);
            return LOGERR_THRD_QUE_PUSH_FAILED;
        }

        thrlib_condSig(mainCb->u.thrd.cond);
    }

    thrlib_mutxUnlock(&mainCb->mutx);

    return RC_OK;
}

