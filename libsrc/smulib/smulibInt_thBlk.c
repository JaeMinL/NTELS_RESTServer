#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"

#include "smd_user.h"
#include "smd_user.x"
#include "smulib.h"
#include "smulib.x"
#include "smulibInt.h"
#include "smulibInt.x"

FT_PUBLIC RT_RESULT smulibInt_thBlkInit(SmulibIntThrdBlkInfo *thrdBlkInfo, SmulibTmOutFunc tmOutFunc, VOID *args,
                                        BOOL killPrcFlg, SmulibThrdBlkCfg *thrdBlkCfg, SmulibIntThrdBlkCb **rt_thrdBlkCb)
{
    SINT ret = RC_OK;
    SmulibIntThrdBlkCb *thrdBlkCb = NULL;

    if((killPrcFlg != RC_TRUE) && (killPrcFlg != RC_FALSE)){
        SMU_LOG(SMU_ERR,"Invalid kill flag(%d)\n",killPrcFlg);
        return SMUERR_INVALID_KILL_FLAG;
    }

    thrdBlkCb = comlib_memMalloc(sizeof(SmulibIntThrdBlkCb));

    thrlib_mutxInit(&thrdBlkCb->mutx);
    thrdBlkCb->tid = thrlib_thrdSelf();
    thrdBlkCb->tmOutFunc = tmOutFunc;
    thrdBlkCb->killPrcFlg = killPrcFlg;

    if(thrdBlkCfg == NULL){
        thrdBlkCb->thrdTmOut = SMULIB_DFLT_THRD_TM_OUT;
    }
    else {
        thrdBlkCb->thrdTmOut = thrdBlkCfg->thrdTmOut;
    }
    thrdBlkCb->usrArg = args;
    thrdBlkCb->appCtrl.lstUpdTick= smulibInt_globGetCurTick();
    thrdBlkCb->lnkNode.data = thrdBlkCb;

    thrlib_mutxLock(&thrdBlkInfo->mutx);

    ret = comlib_lnkLstInsertTail(&thrdBlkInfo->blkLL, &thrdBlkCb->lnkNode);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block create failed(ret=%d)\n",ret);
        comlib_memFree(thrdBlkCb);
        return SMUERR_THRD_BLK_CRTE_FAILED;
    }

    (*rt_thrdBlkCb) = thrdBlkCb;

    thrlib_mutxUnlock(&thrdBlkInfo->mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_thBlkDstry(SmulibIntThrdBlkInfo *thrdBlkInfo, SmulibIntThrdBlkCb *thrdBlkCb)
{
    SINT ret = RC_OK;

    thrlib_mutxLock(&thrdBlkInfo->mutx);

    ret = comlib_lnkLstDel(&thrdBlkInfo->blkLL, &thrdBlkCb->lnkNode);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block destory failed(ret=%d)\n",ret);
        return SMUERR_THRD_BLK_DEL_FAILED;
    }

    thrlib_mutxUnlock(&thrdBlkInfo->mutx);

    comlib_memFree(thrdBlkCb);

    return RC_OK;
}


FT_PUBLIC RT_RESULT smulibInt_thBlkKeepalive(SmulibIntThrdBlkCb *thrdBlkCb)
{
    thrdBlkCb->appCtrl.lstUpdTick = smulibInt_globGetCurTick();

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_thBlkChkThrd()
{
    SINT ret = RC_OK;
    ULONG curTick = 0;
    ULONG lstUpdTick = 0;
    ULONG diff = 0;
    ComlibLnkNode *lnkNode = NULL;
    SmulibIntThrdBlkCb *thrdBlkCb = NULL;
    SmulibIntThrdBlkInfo *thrdBlkInfo = NULL;

    curTick = smulibInt_globGetCurTick();

    thrdBlkInfo = smulibInt_globGetThrdBlkInfo();

    thrlib_mutxLock(&thrdBlkInfo->mutx);

    COM_GET_LNKLST_FIRST(&thrdBlkInfo->blkLL, lnkNode);
    if(lnkNode == NULL){
        thrlib_mutxUnlock(&thrdBlkInfo->mutx);
        return RC_OK;
    }

    while(1){
        thrdBlkCb = lnkNode->data;

        lstUpdTick = thrdBlkCb->appCtrl.lstUpdTick;
        CHECK_TICK_DIFF(lstUpdTick, curTick, diff);
        if(diff > thrdBlkCb->thrdTmOut){
            /* time out occure */
            thrlib_thrdCancel(thrdBlkCb->tid);

            if(thrdBlkCb->tmOutFunc != NULL){
                thrlib_mutxLock(&thrdBlkCb->mutx);
                thrdBlkCb->tmOutFunc(thrdBlkCb->usrArg);
                thrlib_mutxUnlock(&thrdBlkCb->mutx);
            }

            ret = comlib_lnkLstDel(&thrdBlkInfo->blkLL, &thrdBlkCb->lnkNode);
            if(ret != RC_OK){
                SMU_LOG(SMU_ERR,"Thread block destory failed(ret=%d)\n",ret);
                return SMUERR_THRD_BLK_DEL_FAILED;
            }

            if(thrdBlkCb->killPrcFlg == RC_TRUE){
                ret = smulibInt_mainTermProc(RC_FALSE);
                if(ret != RC_OK){
                    SMU_LOG(SMU_ERR,"Term proc failed(ret=%d)\n",ret);
                    exit(1);
                }
            }

            comlib_memFree(thrdBlkCb);
        }/* end of if(diff > 30) */

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            break;
        }
    }/* end of while(1) */

    thrlib_mutxUnlock(&thrdBlkInfo->mutx);

    return RC_OK;
}
