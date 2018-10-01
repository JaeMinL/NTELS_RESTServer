#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

FT_PUBLIC RT_RESULT smulibInt_timerAppTmOutHdlr(SmulibIntLocInfo *locInfo)
{
    SINT ret = RC_OK;
    UINT evnt = 0;
    SmulibIntSmdInfo *smdInfo = NULL;
    SmulibIntTmrInfo *tmrInfo = NULL;

    smdInfo = smulibInt_mainGetSmdInfo();
    tmrInfo = smulibInt_mainGetTmrInfo();

    SMU_LOG(SMU_ERR,"Application timeout occure\n");
    ret = smulibInt_lhSndTermReq(smdInfo, locInfo->pid);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Termination request send failed(ret=%d)\n",ret);
        exit(1);
    }

    /* cancel heartbeat timeout */
    GET_TMRNODE_STATE(&smdInfo->tmrNode, evnt, ret);
    evnt = evnt;

    if(ret == COM_TIMER_ACT){
        ret = comlib_timerTblCancelTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"SMD Timer cancel failed(ret=%d)\n",ret);
        }
    }

    /* start timer */
    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &locInfo->tmrNode, SMULIB_TMR_EVNT_FREEZE_TMOUT, 30);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"heartbeat request timer start failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_timerEvnt(UINT evnt, VOID *arg)
{
    SINT ret = 0;

    switch(evnt) {
        case SMULIB_TMR_EVNT_REG_SND_TMOUT:
            {
                SmulibIntSmdInfo *smdInfo = NULL;
                SmulibIntLocInfo *locInfo = NULL;
                SmulibIntTmrInfo *tmrInfo = NULL;

                locInfo = smulibInt_mainGetLocInfo();
                tmrInfo = smulibInt_mainGetTmrInfo();
                smdInfo = arg;

                SMU_LOG(SMU_NOTY,"REG SEND TIMEOUT OCCURE\n");

                ret = smulibInt_lhSndRegReq(smdInfo, locInfo->pid, locInfo->name, locInfo->nameLen);
                if(ret != RC_OK){
                    SMU_LOG(SMU_ERR,"Reg request send failed\n");
                    break;
                }

                ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode, SMULIB_TMR_EVNT_REG_SND_TMOUT, 
                                             smdInfo->regSndTmOut);
                if(ret != RC_OK){
                    SMU_LOG(SMU_ERR,"reg request timer start failed(ret=%d)\n",ret);
                    break;
                }
            }
            break;
        case SMULIB_TMR_EVNT_HB_SND_TMOUT:
            {
                SmulibIntLocInfo *locInfo = NULL;
                SmulibIntSmdInfo *smdInfo = NULL;
                SmulibIntTmrInfo *tmrInfo = NULL;

                locInfo = smulibInt_mainGetLocInfo();
                tmrInfo = smulibInt_mainGetTmrInfo();

                smdInfo = arg;

                SMU_LOG(SMU_NOTY,"HBEAT SEND TIMEOUT OCCURE\n");

                if(smdInfo->hbSndWaitCnt >= 3){
                    SMU_LOG(SMU_NOTY,"CHANGE CLOSE STATUS(waitCnt=%d)\n", smdInfo->hbSndWaitCnt);
                    smdInfo->status = SMULIB_SMD_STA_CLOSE;
                    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode, SMULIB_TMR_EVNT_REG_SND_TMOUT, 
                                                 smdInfo->regSndTmOut);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"reg request timer start failed(ret=%d)\n",ret);
                        break;
                    }
                }
                else {
                    ret = smulibInt_lhSndHBeatReq(smdInfo, locInfo->pid);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"Heart beat send failed(ret=%d)\n",ret);
                    }

                    smdInfo->hbSndWaitCnt++;

                    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode, SMULIB_TMR_EVNT_HB_SND_TMOUT, 
                                                 smdInfo->hbSndTmOut);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"heartbeat request timer start failed(ret=%d)\n",ret);
                        break;
                    }
                }
            }
            break;
        case SMULIB_TMR_EVNT_APP_HB_TMOUT:
            {
                SMU_LOG(SMU_NOTY,"APP HBEAT SEND TIMEOUT OCCURE\n");

                ret = smulibInt_mainTermProc(RC_FALSE);
                if(ret != RC_OK){
                    SMU_LOG(SMU_ERR,"Termination failed(ret=%d)\n",ret);
                    break;
                }
            }
            break;
        case SMULIB_TMR_EVNT_FREEZE_TMOUT:
            {
                SMU_LOG(SMU_NOTY,"FREEZE TIMEOUT OCCURE\n");
                exit(1);
            }
            break;
        default:
            SMU_LOG(SMU_ERR,"Invalid timer event id(id=%d)\n",evnt);
            break;
    };

    return RC_OK;
}
