#include <unistd.h>

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

FT_PUBLIC RT_RESULT rlylibInt_initMainCb(RlylibIntMainCb *intMainCb, UINT rlyType, CHAR *locHost, RlylibOptArg *optArg)
{
    SINT ret = RC_OK;
    UINT i = 0;
    RlylibIntHostThrdCb *hostThrdCb = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"relay library control block is null\n"),
                    RLYERR_NULL);

    if(rlyType != RLYLIB_TYPE_CLIENT &&
       rlyType != RLYLIB_TYPE_SERVER &&
       rlyType != RLYLIB_TYPE_BOTH){
        RLY_LOG(RLY_ERR,"Invalid relay distribution type(%d)\n", rlyType);
        return RLYERR_INVALID_RLY_TYPE;
    }

    intMainCb->rlyType  = rlyType;

    comlib_strCpy(intMainCb->locHost, locHost);

    if(optArg == NULL){
        intMainCb->hostMainCb.maxHostCnt = RLYLIB_DFLT_MAX_HOST_CNT; 
        intMainCb->rlmMainCb.maxRlmCnt = RLYLIB_DFLT_MAX_RLM_CNT; 
        intMainCb->acptType = RLYLIB_ACPT_TYPE_ALL; /* default */
        intMainCb->thrdCnt = RLYLIB_DFLT_THRD_CNT; /* single thread */
        intMainCb->condWaitFlg = RC_FALSE; 
        intMainCb->waitTm = 0; 

        intMainCb->dfltHostOptArg.rlyMode = RLYLIB_CONN_RLY_MODE_AS;
        intMainCb->dfltHostOptArg.msgDropIndFlg = RC_FALSE;
        intMainCb->dfltHostOptArg.freeTmEnb = RC_TRUE;

        comlib_memMemset(&intMainCb->dfltHostOptArg.tmrOptArg, 0x0, sizeof(RlylibTmrOptArg));
    }/* if(optArg == NULL) */
    else {
        if(optArg->maxHostCnt != 0){
            intMainCb->hostMainCb.maxHostCnt = optArg->maxHostCnt;
        }
        else {
            intMainCb->hostMainCb.maxHostCnt = RLYLIB_DFLT_MAX_HOST_CNT; 
        }

        if(optArg->maxRlmCnt != 0){
            intMainCb->rlmMainCb.maxRlmCnt = optArg->maxRlmCnt;
        }
        else {
            intMainCb->rlmMainCb.maxRlmCnt = RLYLIB_DFLT_MAX_RLM_CNT; 
        }

        if(optArg->condWaitFlg == RC_TRUE){
            intMainCb->condWaitFlg = RC_TRUE; 
            if(optArg->waitTm != 0){
                intMainCb->waitTm = optArg->waitTm; 
            }
            else {
                intMainCb->waitTm = 0; 
            }
        }

        if(optArg->acptType != 0){
            if(optArg->acptType != RLYLIB_ACPT_TYPE_ALL &&
               optArg->acptType != RLYLIB_ACPT_TYPE_CHK_LST ){
                RLY_LOG(RLY_ERR,"Invalid Accept type(%d)\n",optArg->acptType);
                comlib_memFree(intMainCb);
                return RLYERR_INVALID_ACPT_TYPE;
            }

            intMainCb->acptType = optArg->acptType;
        }/* if(optArg->acptType != 0) */

        if(optArg->thrdCnt != 0){
            intMainCb->thrdCnt = optArg->thrdCnt;
        }/* if(optArg->thrdCnt != 0) */
        else {
            intMainCb->thrdCnt = 1;
        }

        comlib_memMemcpy(&intMainCb->dfltHostOptArg, &optArg->dfltHostOptArg, sizeof(RlylibHostOptArg));
    }/* end of else */

    intMainCb->priHostId = 0; /* primary host id */
    intMainCb->lastRcvHostId = 0;
    intMainCb->lastSndHostId = 0;

    ret = comlib_lnkLstInit (&intMainCb->hostLnkLst, intMainCb->hostMainCb.maxHostCnt);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Conn linted list init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_CONN_LNK_LST_INIT_FAILED;
    }

    ret = thrlib_condInit(&intMainCb->rdCond);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"cond init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_COND_INIT_FAILED;
    }

    ret = rlylibInt_syncMutxInit(&intMainCb->mainMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"main mutex init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_MUTX_INIT_FAILED;
    }

    ret = comlib_hashTblInit(&intMainCb->hostHt, intMainCb->hostMainCb.maxHostCnt, RC_FALSE, COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host host hash table init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_CONN_HASH_INIT_FAILED;
    }

    if(rlyType != RLYLIB_TYPE_CLIENT){
        ret = thrlib_mutxInit(&intMainCb->acptThrdCb.mutx);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Accept mutex init failed(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_MUTX_INIT_FAILED;
        }

        ret = comlib_lnkLstInit (&intMainCb->acptThrdCb.acptLnkLst, ~0);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Acpt linked list init failed(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_ACPT_LNK_LST_INIT_FAILED;
        }

        ret = comlib_lnkLstInit(&intMainCb->acptThrdCb.actConnLnkLst, ~0);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Act connection linked list init failed(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_ACT_CONN_LNK_LST_INIT_FAILED;
        }

        ret = comlib_lnkLstInit(&intMainCb->acptThrdCb.freeConnLnkLst, ~0);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Free connection linked list init failed(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_FREE_CONN_LNK_LST_INIT_FAILED;
        }

        ret = rlylibInt_msgInitMsgBufInfo(&intMainCb->msgBufInfo, RLYLIB_DFLT_MAIN_MSG_BUF_CNT);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"buffer infor init failed(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_BUF_INFO_INIT_FAILED;
        }


        intMainCb->acptThrdCb.tmpConnCnt = 0;
        intMainCb->acptThrdCb.termFlg = RC_FALSE;
        intMainCb->acptThrdCb.termSig = RC_FALSE;

        intMainCb->acptThrdCb.rdSockFdSet = trnlib_sockAllocSockFdSet();
        if(intMainCb->acptThrdCb.rdSockFdSet == NULL){
            RLY_LOG(RLY_ERR,"Read fd set failed\n");
            thrlib_thrdExit(0);
        }

        intMainCb->acptThrdCb.wrSockFdSet = trnlib_sockAllocSockFdSet();
        if(intMainCb->acptThrdCb.wrSockFdSet == NULL){
            RLY_LOG(RLY_ERR,"Write fd set failed\n");
            thrlib_thrdExit(0);
        }

        intMainCb->acptThrdCb.exSockFdSet = trnlib_sockAllocSockFdSet();
        if(intMainCb->acptThrdCb.rdSockFdSet == NULL){
            RLY_LOG(RLY_ERR,"Exption fd set failed\n");
            thrlib_thrdExit(0);
        }

        /* create thread */
        ret = thrlib_thrdCrte(&intMainCb->acptThrdCb.acptTid,NULL, rlylibInt_acptMainThrd, intMainCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Accept thread create faeild(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_ACPT_THRD_CRTE_FAILED;
        }
    }

    intMainCb->hostCb = comlib_memMalloc(sizeof(RlylibIntHostCb*) * intMainCb->hostMainCb.maxHostCnt);
    if(intMainCb->hostCb == NULL){
        RLY_LOG(RLY_ERR,"Host control blaock alloc failed\n");
        return RLYERR_HOST_CB_ALLOC_FAILED;
    }
    comlib_memMemset(intMainCb->hostCb, 0x0, sizeof(RlylibIntHostCb*) * intMainCb->hostMainCb.maxHostCnt);

    ret = thrlib_mutxInit(&intMainCb->rlmMainCb.mutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"realm mutex init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_MUTX_INIT_FAILED;

    }

    intMainCb->rlmMainCb.rlmCnt = 0;

    intMainCb->rlmCb = comlib_memMalloc(sizeof(RlylibIntRlmCb*) * intMainCb->rlmMainCb.maxRlmCnt);
    if(intMainCb->hostCb == NULL){
        RLY_LOG(RLY_ERR,"Host control blaock alloc failed\n");
        return RLYERR_RLM_CB_ALLOC_FAILED;
    }
    comlib_memMemset(intMainCb->rlmCb, 0x0, sizeof(RlylibIntRlmCb*) * intMainCb->rlmMainCb.maxRlmCnt);

    ret = comlib_hashTblInit(&intMainCb->rlmHt, intMainCb->rlmMainCb.maxRlmCnt, RC_FALSE, COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Realm hash table init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_RLM_HASH_INIT_FAILED;
    }

    ret = thrlib_tqInit(&intMainCb->freeMsgChkQ, THR_TQ_LOCK_TYPE_LOCK_FREE, RLYLIB_DFLT_FREE_CHK_Q_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"free message chunk queue init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_THRD_Q_INIT_FAILED;
    }

    intMainCb->rlmMainCb.main = intMainCb;

    ret = comlib_lnkLstInit(&intMainCb->rlmMainCb.rlmLnkLst, ~0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Act connection linked list init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_HOST_LNK_LST_INIT_FAILED;
    }

    ret = thrlib_mutxInit(&intMainCb->hostMainCb.mutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"host mutex init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_MUTX_INIT_FAILED;

    }

    ret = thrlib_condInit(&intMainCb->hostMainCb.cond);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"host cond init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_COND_INIT_FAILED;
    }

    intMainCb->hostMainCb.hostCnt = 0;
    intMainCb->hostMainCb.sleepCnt = 0;
    intMainCb->hostMainCb.termFlg = RC_FALSE;
    intMainCb->hostMainCb.actThrdCnt = intMainCb->thrdCnt;

    intMainCb->hostMainCb.main = intMainCb;

    ret = comlib_lnkLstInit(&intMainCb->hostMainCb.hostLnkLst, ~0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Act connection linked list init failed(ret=%d)\n",ret);
        comlib_memFree(intMainCb);
        return RLYERR_HOST_LNK_LST_INIT_FAILED;
    }


    for(i=0;i<intMainCb->thrdCnt;i++){
        hostThrdCb = comlib_memMalloc(sizeof(RlylibIntHostThrdCb));

        hostThrdCb->tid = 0;
        hostThrdCb->procCnt = 0;
        hostThrdCb->tmrUpdFlg = RC_FALSE;
        hostThrdCb->main = &intMainCb->hostMainCb;

        if((i == 0) &&
           (rlyType == RLYLIB_TYPE_CLIENT)){
            hostThrdCb->tmrUpdFlg = RC_TRUE;
        }

        ret = thrlib_thrdCrte(&hostThrdCb->tid, NULL, rlylibInt_hostThrdMain, hostThrdCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Host thread pool create faeild(ret=%d)\n",ret);
            comlib_memFree(intMainCb);
            return RLYERR_CONN_THRD_POOL_INIT_FAILED;
        }

        hostThrdCb = NULL;
    }/* end of for(i=0;i<intMainCb->maxThrdCnt;i++) */

    return RC_OK;
}
