#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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

#include "smd_user.h"
#include "smd_user.x"
#include "smulib.h"
#include "smulib.x"
#include "smulibInt.h"
#include "smulibInt.x"

FT_PUBLIC RT_RESULT smulibInt_init(SmulibIntMainCb *mainCb, CHAR *name, TrnlibTransAddr *trnAddr, 
                                   ThrlibTq *rcvTq, SmulibOptArg *optArg)
{
    SINT ret = RC_OK;
    CHAR *strAddr = NULL;
    pid_t pid = 0;
    SmulibIntLocInfo *locInfo = NULL;
    SmulibIntSmdInfo *smdInfo = NULL;
    SmulibIntTmrInfo *tmrInfo = NULL;
    RlylibOptArg rlyOptArg;
    TrnlibTransAddr tmpAddr;
    TrnlibTransAddr *dstAddr = NULL;

    locInfo = &mainCb->locInfo;
    smdInfo = &mainCb->smdInfo;
    tmrInfo = &mainCb->tmrInfo;

    if(trnAddr == NULL){
        CHAR strIp[1024];
        UINT port = 0;
        /* get env */
        strAddr = getenv("SMD_ADDR");
        if(strAddr == NULL){
            SMU_LOG(SMU_ERR,"SMD_ADDR env not exist\n");
            return SMUERR_SMD_ADDR_ENV_NOT_EXIST;
        }

        sscanf(strAddr,"%s %u",strIp, &port);

        tmpAddr.netAddr.afnum = TRNLIB_AFNUM_IPV4;
        ret = trnlib_utilPton(TRNLIB_AFNUM_IPV4, strIp, &tmpAddr.netAddr.u.ipv4NetAddr);
        if(ret != RC_OK){
            tmpAddr.netAddr.afnum= TRNLIB_AFNUM_IPV6;
            ret = trnlib_utilPton(TRNLIB_AFNUM_IPV6, strIp, tmpAddr.netAddr.u.ipv6NetAddr);

            if(ret != RC_OK){
                SMU_LOG(SMU_ERR,"SMD IP Parsing failed(ret=%d)\n",ret);
                return SMUERR_SMD_ADDR_PARSING_FAILED;
            }
        }

        tmpAddr.port = port;
        tmpAddr.proto = TRNLIB_TRANS_PROTO_TCP;

        dstAddr = &tmpAddr;
    }
    else {
        dstAddr = trnAddr;
    }

    ret = smulibInt_globGetInitFlg();
    if(ret != RC_OK){
        return SMUERR_GLOB_CB_INIT_FIRST;
    }

    mainCb->rcvTq = rcvTq; 

    ret = rlylib_apiInitGlob();

    rlylib_apiSetLogFunc(RLY_NOTY, smulibInt_mainRlyLog);

    pid = getpid();

    locInfo->pid = pid;
    comlib_strSNPrnt(locInfo->strPid, SMULIB_STR_PID_MAX_LEN, "%d", pid);

    RLYLIB_INIT_OPT_ARG(&rlyOptArg);

    if(optArg->rlyThrdCnt != 0){
        rlyOptArg.thrdCnt = optArg->rlyThrdCnt;
    }
    else {
        rlyOptArg.thrdCnt = 1;
    }

    ret = rlylib_apiInitRlylibCb(&smdInfo->rlylibCb, RLYLIB_TYPE_CLIENT, locInfo->strPid, &rlyOptArg);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"relay library init failed(ret=%d)\n",ret);
        return SMUERR_RLY_INIT_FAILED;
    }

    ret = comlib_timerInit(&tmrInfo->tmr, COM_TIMER_TYPE_100M);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Timer init failed(ret=%d)\n",ret);
        return SMUERR_TIMER_INIT_FAILED;
    }

    ret = comlib_timerTblInit(&tmrInfo->tmrTbl, &tmrInfo->tmr, smulibInt_timerEvnt);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Timer table init failed(ret=%d)\n",ret);
        return SMUERR_TIMER_TBL_INIT_FAILED;
    }

    comlib_strCpy(locInfo->name, name);
    locInfo->nameLen = comlib_strGetLen(name);
    locInfo->name[locInfo->nameLen] = '\0';

    locInfo->tmrNode.data = &locInfo;

    /* add host */
    ret = rlylib_apiAddHost(&smdInfo->rlylibCb, "SMD", RLYLIB_TYPE_CLIENT, 0, NULL, &smdInfo->spId);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"SMD host add faiiled(ret=%d)\n",ret);
        return SMUERR_RLY_HOST_ADD_FAILED;
    }

    /* add connection */
    ret = rlylib_apiAddConn(&smdInfo->rlylibCb, "SMD", dstAddr, 1);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Connection add failed(ret=%d)\n",ret);
        return SMUERR_RLY_CONN_ADD_FAILED;
    }

    smdInfo->hbSndTmOut = optArg->hbSndTmOut;
    smdInfo->freezeTmOut = optArg->freezeTmOut;
    smdInfo->regSndTmOut = optArg->regSndTmOut;
    smdInfo->status = SMULIB_SMD_STA_CLOSE;
    smdInfo->tmrNode.data = smdInfo;

    /* send reg message */
    ret = smulibInt_lhSndRegReq(smdInfo, locInfo->pid, locInfo->name, locInfo->nameLen);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Reg request send failed\n");
        return SMUERR_REG_REG_SND_FAILED;
    }

    ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode, SMULIB_TMR_EVNT_REG_SND_TMOUT, smdInfo->regSndTmOut);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"reg request timer start failed(ret=%d)\n",ret);
        return SMUERR_REG_REG_TMOUT_START_FAILED;
    }

    return RC_OK;
}

