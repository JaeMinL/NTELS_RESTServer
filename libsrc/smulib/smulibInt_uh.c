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

FT_PUBLIC RT_RESULT smulibInt_uhChkMainTick(ULONG curTick, SmulibIntMainBlkInfo *mainBlkInfo, SmulibIntLocInfo *locInfo)
{
    SINT ret = RC_OK;

    if(((curTick - mainBlkInfo->lstUpdTick) >= 30) &&
        (mainBlkInfo->status == SMU_MAIN_BLK_STA_ACT)){
        /* time out */
        SMU_LOG(SMU_ERR,"Application timeout(tick=%lu, curTick=%lu)\n",mainBlkInfo->lstUpdTick, curTick);
        ret = smulibInt_timerAppTmOutHdlr(locInfo);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Application timeout handling failed(ret=%d)\n",ret);
            return RC_NOK;
        }

        mainBlkInfo->status = SMU_MAIN_BLK_STA_INACT;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_uhRcvHdlr(SmulibIntMainCb *mainCb, UINT *rt_loopCnt)
{
    SINT ret = RC_OK;
    UINT evnt = 0;
    SmulibIntMsgComHdr *comHdr = NULL;
    SmulibIntLocInfo *locInfo = NULL;
    SmulibIntTmrInfo *tmrInfo = NULL;
    UINT loopCnt = 0;

    locInfo = &mainCb->locInfo;
    tmrInfo = &mainCb->tmrInfo;

    while(1){
        ret = thrlib_tqPop(mainCb->rcvTq, (VOID*)&comHdr);
        if(ret == THRERR_TQ_EMPTY){
            (*rt_loopCnt) = loopCnt;
            return RC_OK;
        }
        else if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Application message receive failed(ret=%d)\n",ret);
            (*rt_loopCnt) = loopCnt;
            return RC_NOK;
        }

        loopCnt++;
        switch(comHdr->cmdCode){
            case SMULIB_MSG_CODE_HB:
                {
                    GET_TMRNODE_STATE(&locInfo->tmrNode, evnt, ret);
                    if(evnt == SMULIB_TMR_EVNT_FREEZE_TMOUT){
                        SMU_LOG(SMU_ERR,"Freeze wait state\n");
                        comlib_memFree(comHdr);
                        (*rt_loopCnt) = loopCnt;
                        return RC_OK;
                    }

                    if(ret == COM_TIMER_ACT){
                        ret = comlib_timerTblCancelTm(&tmrInfo->tmrTbl, &locInfo->tmrNode);
                        if(ret != RC_OK){
                            SMU_LOG(SMU_ERR,"Timer cancel faield(ret=%d)\n",ret);
                        }
                    }

                    /* timer restart */
                    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &locInfo->tmrNode, SMULIB_TMR_EVNT_APP_HB_TMOUT, 30);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"Application heartbeat timer start failed(ret=%d)\n",ret);
                        comlib_memFree(comHdr);
                        (*rt_loopCnt) = loopCnt;
                        return RC_NOK;
                    }

                    comlib_memFree(comHdr);
                }
                break;
            default:
                {
                    SMU_LOG(SMU_ERR,"Invalid command code(%d)\n",comHdr->cmdCode);
                    comlib_memFree(comHdr);
                    (*rt_loopCnt) = loopCnt;
                    return RC_NOK;
                }
                break;
        }


    }/* end of while(1) */

    (*rt_loopCnt) = loopCnt;

    return RC_OK;
}
