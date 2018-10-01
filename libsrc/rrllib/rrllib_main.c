#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PUBLIC RT_RESULT rrllib_mainInit(RrllibCb *rrlCb, BOOL caseFlg)
{
    GEN_CHK_ERR_RET(rrlCb == NULL,
                RRL_LOG(RRL_ERR,"RRLLIB is NULL\n"),
                RRLERR_RRLCB_IS_NULL);

    rrlCb->caseFlg = caseFlg;
    rrlCb->resPathLst = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_mainDstry(RrllibCb *rrlCb)
{
    SINT ret = 0;

    if(rrlCb->resPathLst == NULL){
        return RC_OK;
    }

    ret = rrllib_pathDstryPathLst(rrlCb->resPathLst);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Path list destory failed(ret=%d)\n",ret);
        return ret;
    }

    rrlCb->resPathLst = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_mainPrnt(RrllibCb *rrlCb, RrllibPrntUsrArgFunc func)
{
    SINT ret = RC_OK;

    if(rrlCb->resPathLst == NULL){
        return RC_OK;
    }

    ret = rrllib_pathPrntPathLst(rrlCb->resPathLst, 0, func);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Path list print failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

