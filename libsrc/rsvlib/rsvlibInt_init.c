#include <unistd.h>
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

FT_PUBLIC RT_RESULT rsvlibInt_init(UINT id, RsvlibGenCfg *genCfg)
{
    SINT ret = 0;
    RsvlibIntCb *rsvlibIntCb = NULL;

    ret = rsvlibInt_globInit();
    if(ret != RC_OK){
        fprintf(stderr,"Rsvlib init failed(ret=%d)\n",ret);
        return RSVERR_GLOB_CB_INIT_FAILED;
    }

    /* exist check */
    rsvlibIntCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibIntCb != NULL){
        RSV_LOG(RSV_ERR,"rsvlibInt already exist\n");
        return RSVERR_RSVLIB_CB_EXIST;
    }

    rsvlibIntCb = comlib_memMalloc(sizeof(RsvlibIntCb));
    if(rsvlibIntCb == NULL){
        RSV_LOG(RSV_ERR,"rsvlibInt alloc afiled\n");
        return RSVERR_ALLOC_FAILED;
    }

    thrlib_mutxInit(&rsvlibIntCb->runMutx);

    rsvlibIntCb->runFlg = RC_FALSE;

    comlib_memMemcpy(&rsvlibIntCb->genCfg, genCfg, sizeof(RsvlibGenCfg));

    rsvlibIntCb->svrThrdMainCb.dm = NULL;

    thrlib_mutxInit(&rsvlibIntCb->svrThrdMainCb.sesMutx);

    ret = comlib_lnkLstInit(&rsvlibIntCb->svrThrdMainCb.actvSvrSes, ~0);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Active session list init failed(ret=%d)\n",ret);
        comlib_memFree(rsvlibIntCb);
        return RSVERR_LNKLST_INIT_FAILED;
    }

    /* init svr url */
    ret = rrllib_mainInit(&rsvlibIntCb->svrThrdMainCb.rrlCb, RC_FALSE);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Rsvlib rule init failed(ret=%d)\n",ret);
        comlib_memFree(rsvlibIntCb);
        return RSVERR_RULE_INIT_FAILED;
    }

    ret = rsvlibInt_globSetRsvlibIntCb(id, rsvlibIntCb);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"rsvlibInt init failed(ret=%d, id=%d)\n",ret, id);
        comlib_memFree(rsvlibIntCb);
        return RSVERR_RSVLIB_CB_SET_FAILED;
    }

    return RC_OK;
}

