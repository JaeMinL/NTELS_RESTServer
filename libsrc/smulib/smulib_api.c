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

FT_PUBLIC RT_RESULT smulib_apiInit(CHAR *name, TrnlibTransAddr *smdAddr, SmulibOptArg *optArg)
{
    SINT ret = 0;
    ThrlibThrdId tid = 0;
    SmulibIntCfg *cfg = NULL;
    ThrlibTq *sndTq = NULL;

    ret = smulibInt_globGetInitFlg();
    if(ret == RC_OK){
        return RC_OK;
    }

    ret = smulibInt_globInit();
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Global data init failed(ret=%d)\n",ret);
        return ret;
    }

    if(optArg != NULL){
        if(optArg->logFunc != NULL){
            smulibInt_globSetLogFunc(optArg->logLvl, optArg->logFunc);
        }
    }/* end of if(optArg != NULL) */

    sndTq = smulibInt_globGetSndTq();
    if(sndTq == NULL){
        SMU_LOG(SMU_ERR,"Send queue not exist\n");
        return SMUERR_SNDQ_INIT_FAILED;
    }

    cfg = comlib_memMalloc(sizeof(SmulibIntCfg));

    ret = comlib_strGetLen(name);
    if(ret == 0){
        SMU_LOG(SMU_ERR,"name does not exist\n");
        return SMUERR_NAME_NOT_EXIST;
    }
    if(ret >= SMD_PROC_NAME_MAX_LEN){
        SMU_LOG(SMU_ERR,"name is too long(len=%d, max=%d)\n",ret, SMD_PROC_NAME_MAX_LEN);
        return SMUERR_NAME_TOO_LONG;
    }

    cfg->rcvTq = sndTq;
    comlib_strCpy(cfg->name, name);

    if(smdAddr != NULL){
        cfg->smdAddrFlg = RC_TRUE;
        comlib_memMemcpy(&cfg->smdAddr, smdAddr, sizeof(TrnlibTransAddr));
    }
    else {
        cfg->smdAddrFlg = RC_FALSE;
    }

    if(optArg != NULL){
        comlib_memMemcpy(&cfg->optArg, optArg, sizeof(SmulibOptArg));
    }
    else {
        cfg->optArg.rlyThrdCnt = SMULIB_DFLT_RLY_THRD_CNT;
        cfg->optArg.freezeTmOut = SMULIB_DFLT_FREEZE_TM_OUT;
        cfg->optArg.hbSndTmOut = SMULIB_DFLT_HB_SND_TM_OUT;
        cfg->optArg.regSndTmOut = SMULIB_DFLT_REG_SND_TM_OUT;
        cfg->optArg.tmOutFunc = NULL;
    }

    ret = thrlib_thrdCrte(&tid, NULL, smulibInt_mainThrdMain, cfg);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"SMULIB main thread create failed(ret=%d)\n",ret);

        return SMUERR_THRD_CRTE_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulib_apiSetLogFunc(UINT lvl, SmulibLogFunc logFunc)
{
    SINT ret = RC_OK;

    ret = smulibInt_globSetLogFunc(lvl, logFunc);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Log function setting failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC UINT smulib_apiGetLogLvl()
{
    return smulibInt_globGetLogLvl();
}

FT_PUBLIC RT_RESULT smulib_apiSetLogLvl(UINT logLvl)
{
    SINT ret = RC_OK; 

    ret = smulibInt_globSetLogLvl(logLvl);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Log level setting failed(ret=%d)\n",ret);

        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulib_apiKeepalive()
{
    SINT ret = RC_OK;

    ret = smulibInt_globKeepalive();

    return ret;
}

FT_PUBLIC RT_RESULT smulib_apiSigReg(UINT sigNo, SmulibSigFunc func, VOID *usrArg)
{
    SINT ret = 0;

    ret = smulibInt_sigReg(sigNo, func, usrArg);

    return ret;
}

FT_PUBLIC RT_RESULT smulib_apiRegThrdBlk(SmulibThrdCb *thrdCb, SmulibTmOutFunc tmOutFunc, VOID *arg, BOOL killPrcFlg, 
                                         SmulibThrdBlkCfg *thrdBlkCfg)
{
    SINT ret = RC_OK;
    SmulibIntThrdBlkCb *thrdBlkCb = NULL;
    SmulibIntThrdBlkInfo *thrdBlkInfo = NULL;

    thrdBlkInfo = smulibInt_globGetThrdBlkInfo();

    ret = smulibInt_thBlkInit(thrdBlkInfo, tmOutFunc, arg, killPrcFlg, thrdBlkCfg, &thrdBlkCb);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block create failed(ret=%d)\n",ret);
        return ret;
    }

    thrdCb->main = (SmulibThrdBlkCb)thrdBlkCb;

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulib_apiDeregThrdBlk(SmulibThrdCb *thrdCb)
{
    SINT ret = RC_OK;
    SmulibIntThrdBlkCb *thrdBlkCb = NULL;
    SmulibIntThrdBlkInfo *thrdBlkInfo = NULL;

    thrdBlkInfo = smulibInt_globGetThrdBlkInfo();

    thrdBlkCb = (SmulibIntThrdBlkCb*)thrdCb->main;

    ret = smulibInt_thBlkDstry(thrdBlkInfo, thrdBlkCb);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block keepalive(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulib_apiKeepaliveThrdBlk(SmulibThrdCb *thrdCb)
{
    SINT ret = RC_OK;
    SmulibIntThrdBlkCb *thrdBlkCb = NULL;

    thrdBlkCb = (SmulibIntThrdBlkCb*)thrdCb->main;

    ret = smulibInt_thBlkKeepalive(thrdBlkCb);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Thread block keepalive(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

