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

SmulibIntMainCb g_mainCb;

FT_PRIVATE VOID main_clnThrd(VOID *args);

FT_PRIVATE VOID main_clnThrd(VOID *args)
{
    return;
}

FT_PUBLIC VOID smulibInt_mainRlyLog(UINT lvl, CHAR *file, UINT line, CHAR *str)
{
    return;
    if(lvl == RLY_ERR){
        smulibInt_globLogPrnt(SMU_ERR, file, line, str);
    }
    else if (lvl == RLY_NOTY){
        smulibInt_globLogPrnt(SMU_NOTY, file, line, str);
    }
    else if (lvl == RLY_DBG){
        smulibInt_globLogPrnt(SMU_DBG, file, line, str);
    }
}

FT_PUBLIC SmulibIntLocInfo* smulibInt_mainGetLocInfo()
{
    return &g_mainCb.locInfo;
}

FT_PUBLIC SmulibIntSmdInfo* smulibInt_mainGetSmdInfo()
{
    return &g_mainCb.smdInfo;
}


FT_PUBLIC SmulibIntTmrInfo* smulibInt_mainGetTmrInfo()
{
    return &g_mainCb.tmrInfo;
}

FT_PUBLIC RT_RESULT smulibInt_mainExit()
{
    exit(1);
}

FT_PUBLIC RT_RESULT smulibInt_mainTermProc(BOOL frceDnFlg/* force down flag */)
{
    SINT ret = RC_OK;
    UINT evnt = 0;
    SmulibIntSmdInfo *smdInfo = NULL;
    SmulibIntLocInfo *locInfo = NULL;
    SmulibIntTmrInfo *tmrInfo = NULL;

    if(frceDnFlg == RC_TRUE){
        SMU_LOG(SMU_NOTY, "Process force down\n");
        exit(1);
    }

    smdInfo = smulibInt_mainGetSmdInfo();
    locInfo = smulibInt_mainGetLocInfo();
    tmrInfo = smulibInt_mainGetTmrInfo();

    if(smdInfo->status == SMULIB_SMD_STA_CLOSE){
        SMU_LOG(SMU_NOTY, "Process force down(process close state)\n");
        exit(1);
    }

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
            return RC_NOK;
        }
    }

    /* start timer */
    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &locInfo->tmrNode, SMULIB_TMR_EVNT_FREEZE_TMOUT, smdInfo->freezeTmOut);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"heartbeat request timer start failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC VOID smulibInt_mainThrdMain(VOID *arg)
{
    SINT ret = 0;
    UINT loopCnt = 0;
    UINT rcvLoopCnt = 0;
    SmulibIntCfg *cfg = NULL;
    SmulibIntTmrInfo *tmrInfo = NULL;
    SmulibIntMainBlkInfo *mainBlkInfo = NULL;

    cfg = arg;

    thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

    if(cfg->smdAddrFlg == RC_TRUE){
        ret = smulibInt_init(&g_mainCb, cfg->name ,&cfg->smdAddr, cfg->rcvTq, &cfg->optArg);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"init failed(ret=%d)\n", ret);
            exit(1);
            /* return; */
        }
    }
    else {
        ret = smulibInt_init(&g_mainCb, cfg->name ,NULL, cfg->rcvTq, &cfg->optArg);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"init failed(ret=%d)\n", ret);
            exit(1);
        }
    }

    comlib_memFree(cfg);

    thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    thrlib_thrdSetCnclType(THR_THRD_CNCL_DEFERRED, NULL);

    THRLIB_THRDCLNUP_PUSH(main_clnThrd, &g_mainCb);

    tmrInfo = &g_mainCb.tmrInfo;

    mainBlkInfo = smulibInt_globGetMainBlkInfo();

    while(1){
        EXEC_TMR_TICK(&tmrInfo->tmr);
        smulibInt_globUpdCurTick(tmrInfo->tmr.tick);

        ret = smulibInt_lhRcvHdlr(&g_mainCb, &rcvLoopCnt);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Receive handler failed(ret=%d)\n",ret);
        }
        loopCnt += rcvLoopCnt;

        ret = smulibInt_uhRcvHdlr(&g_mainCb, &rcvLoopCnt);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Application receive handler failed(ret=%d)\n",ret);
        }
        loopCnt += rcvLoopCnt;

        ret = smulibInt_uhChkMainTick(tmrInfo->tmr.tick, mainBlkInfo, &g_mainCb.locInfo);

        /* check thread */
        ret = smulibInt_thBlkChkThrd();

        comlib_timerTblHandler(&tmrInfo->tmrTbl);

        if(loopCnt == 0){
        usleep(2000);
        }
        else {
            loopCnt = 0;
        }
    }

    THRLIB_THRDCLNUP_POP(0);

    return;
}
