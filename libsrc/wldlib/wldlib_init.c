#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

FT_PRIVATE RT_RESULT init_initFullMathLst(WldlibFullMatchLst *fullMatchLst);

FT_PRIVATE RT_RESULT init_initFullMathLst(WldlibFullMatchLst *fullMatchLst)
{
    SINT ret = RC_OK;

    ret = comlib_hashTblInit(&fullMatchLst->fullMatchHT, 1024, RC_FALSE, COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Full match hash table init failed(ret=%d)\n",ret);
        return WLDERR_HASHTBL_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&fullMatchLst->fullMatchLL, ~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Full match linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_init(WldlibCb *wldlibCb)
{
    SINT ret = 0;
    wldlibCb->blkLst = NULL;

    ret = comlib_lnkLstInit(&wldlibCb->freeBlkLst, ~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Free wildcard block linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&wldlibCb->freeWldChr, ~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Free wildcard charactor linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&wldlibCb->freeFullMatchBktLst, ~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Free full match block linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    ret = init_initFullMathLst(&wldlibCb->fullMatchLst);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Full match list init failed(ret=%d)\n",ret);
        return ret;
    }

    wldlibCb->blkLst = NULL;

    return RC_OK;
}

