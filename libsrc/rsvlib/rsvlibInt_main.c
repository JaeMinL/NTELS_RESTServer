#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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

#if 0
FT_PUBLIC VOID rsvlibInt_mainLogPrnt(UINT lvl, CHAR *file, UINT line, CHAR *str)
{
    LoglibCb *loglibCb = NULL;

    loglibCb = rsvlibInt_globGetLoglibCb();
    if(lvl == RRL_ERR){
        if(loglibCb != NULL){
            loglib_apiLogWrite(loglibCb, LOGLIB_LVL_ERR, file, line, str);
        }
        else {
            fprintf(stderr,str);

        }
    }
    else if(lvl == RRL_NOTY){
        if(loglibCb != NULL){
            loglib_apiLogWrite(loglibCb, LOGLIB_LVL_NOTY, file, line, str);
        }
        else {
            fprintf(stderr,str);

        }
    }
    else if(lvl == RRL_DBG){
        if(loglibCb != NULL){
            loglib_apiLogWrite(loglibCb, LOGLIB_LVL_DBG, file, line, str);
        }
        else {
            fprintf(stderr,str);

        }
    }
}

FT_PUBLIC RT_RESULT rsvlibInt_mainDispPrnt(CHAR *str)
{
    RSV_LOG(RSV_DBG,"%s",str);
    return RC_OK;
}
#endif

FT_PUBLIC RT_RESULT rsvlibInt_mainStop(UINT id)
{
    SINT ret = RC_OK;
    RsvlibIntCb *rsvlibIntCb = NULL;

    /* exist check */
    rsvlibIntCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibIntCb == NULL){
        RSV_LOG(RSV_ERR,"rsvlibInt not exist\n");
        return RC_NOK;
    }

    thrlib_mutxLock(&rsvlibIntCb->runMutx);

    if(rsvlibIntCb->runFlg != RC_TRUE){
        thrlib_mutxUnlock(&rsvlibIntCb->runMutx);
        return RC_OK;
    }

    ret = rsvlibInt_svrStop(&rsvlibIntCb->svrThrdMainCb);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Rsvlib server stop failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&rsvlibIntCb->runMutx);
        return RC_NOK;
    }

    RSV_LOG(RSV_ERR,"RSV SERVER STOP SUCCESS(id=%d)\n", id);
    rsvlibIntCb->runFlg = RC_FALSE;

    thrlib_mutxUnlock(&rsvlibIntCb->runMutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlibInt_mainRun(UINT id)
{
    SINT ret = RC_OK;
    RsvlibIntCb *rsvlibIntCb = NULL;

    /* exist check */
    rsvlibIntCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibIntCb == NULL){
        RSV_LOG(RSV_ERR,"rsvlibInt not exist\n");
        return RSVERR_RSVLIB_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&rsvlibIntCb->runMutx);

    rsvlibIntCb->runFlg = RC_TRUE;

    ret = rsvlibInt_svrStart(&rsvlibIntCb->svrThrdMainCb, rsvlibIntCb->genCfg.svrCfg.sslCfg.cert, 
                          rsvlibIntCb->genCfg.svrCfg.sslCfg.key,
                          rsvlibIntCb->genCfg.svrCfg.thrdCnt, 
                          rsvlibIntCb->genCfg.svrCfg.port);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"svr start failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&rsvlibIntCb->runMutx);
        return RSVERR_SVR_START_FAILED;
    }

    thrlib_mutxUnlock(&rsvlibIntCb->runMutx);

    return RC_OK;
}

