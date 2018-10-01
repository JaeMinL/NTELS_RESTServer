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

/* lower handler */

FT_PUBLIC RT_RESULT smulibInt_lhSndRegReq(SmulibIntSmdInfo *smdInfo, UINT pid, CHAR *name, UINT nameLen)
{
    SINT ret = RC_OK;
    CHAR regReqBuf[sizeof(SmdMsgComHdr) + sizeof(SmdMsgRegReq)];
    SmdMsgComHdr *comHdr = NULL;
    SmdMsgRegReq *regReq = NULL;

    comHdr = (SmdMsgComHdr*)regReqBuf;

    comHdr->cmdCode = SMD_MSG_CODE_REG_REQ;

    comHdr->msgLen = sizeof(SmdMsgRegReq);

    regReq = (SmdMsgRegReq*)&regReqBuf[sizeof(SmdMsgComHdr)];

    regReq->pid = pid;
    comlib_strNCpy(regReq->name, name, nameLen);
    regReq->nameLen = nameLen;

    ret = rlylib_apiSndFixMsgToHost(&smdInfo->rlylibCb, "SMD", (CHAR*)&regReqBuf, sizeof(regReqBuf));
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Message send failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_lhRegCfmHdlr(SmulibIntSmdInfo *smdInfo, CHAR *host, SmdMsgRegCfm *regCfm)
{
    SINT ret = RC_OK;
    SmulibIntTmrInfo *tmrInfo = NULL;

    tmrInfo = smulibInt_mainGetTmrInfo();

    if(regCfm->rsltCode == SMD_MSG_RSLT_CODE_SUCC){
        UINT evnt = 0;
        SMU_LOG(SMU_NOTY,"REGISTRATION SUCCESS\n");

        smdInfo->status = SMULIB_SMD_STA_OPEN;
        smdInfo->hbSndWaitCnt = 0;

        GET_TMRNODE_STATE(&smdInfo->tmrNode, evnt, ret);
        evnt = evnt;

        if(ret == COM_TIMER_ACT){
            ret = comlib_timerTblCancelTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode);
            if(ret != RC_OK){
                SMU_LOG(SMU_ERR,"Timer cancel failed(ret=%d)\n",ret);
            }
        }

        /* start heart beat timer */
        ret = comlib_timerTblStartTm(&tmrInfo->tmrTbl, &smdInfo->tmrNode, SMULIB_TMR_EVNT_HB_SND_TMOUT, smdInfo->hbSndTmOut);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Timer start failed(ret=%d)\n",ret);
            return RC_NOK;
        }
    }
    else {
        SMU_LOG(SMU_ERR,"Registration failed(rsltCode=%d)\n",regCfm->rsltCode);
        exit(1);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_lhHbCfmHdlr(SmulibIntSmdInfo *smdInfo, CHAR *host, SmdMsgHBCfm *hbCfm)
{
    SINT ret = RC_OK;
    SmulibIntLocInfo *locInfo = NULL;

    locInfo = smulibInt_mainGetLocInfo();

    if(hbCfm->rsltCode == SMD_MSG_RSLT_CODE_SUCC){
        SMU_LOG(SMU_NOTY,"RECEIVE HBEAT CFM\n");

        smdInfo->hbSndWaitCnt = 0;
    }
    else if(hbCfm->rsltCode == SMD_MSG_RSLT_CODE_REREG){
        smdInfo->hbSndWaitCnt = 0;

        smdInfo->status = SMULIB_SMD_STA_CLOSE;

        /* send  */
        ret = smulibInt_lhSndRegReq(smdInfo, locInfo->pid, locInfo->name, locInfo->nameLen);
        if(ret != RC_OK){
            SMU_LOG(SMU_ERR,"Reg request failed(ret=%d)\n",ret);
            return RC_NOK;
        }
    }
    else {
        SMU_LOG(SMU_ERR,"Registration failed(rsltCode=%d)\n",hbCfm->rsltCode);
        exit(1);
    }
    return RC_OK;
}


FT_PUBLIC RT_RESULT smulibInt_lhSndHBeatReq(SmulibIntSmdInfo *smdInfo, UINT pid)
{
    SINT ret = RC_OK;
    SmdMsgComHdr *comHdr = NULL;
    SmdMsgHBReq *hbReq = NULL;
    CHAR hbReqBuf[sizeof(SmdMsgComHdr) + sizeof(SmdMsgHBReq)];

    comHdr  = (SmdMsgComHdr*)hbReqBuf;

    comHdr->cmdCode = SMD_MSG_CODE_HB_REQ;

    comHdr->msgLen = sizeof(SmdMsgHBReq);

    hbReq = (SmdMsgHBReq*)&hbReqBuf[sizeof(SmdMsgComHdr)];

    hbReq->pid = pid;

    ret = rlylib_apiSndFixMsgToHost(&smdInfo->rlylibCb, "SMD", (CHAR*)&hbReqBuf, sizeof(hbReqBuf));
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Heart beat message send failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_lhSndTermReq(SmulibIntSmdInfo *smdInfo, UINT pid)
{
    SINT ret = RC_OK;
    SmdMsgComHdr *comHdr = NULL;
    SmdMsgTermReq *termReq = NULL;
    CHAR termReqBuf[sizeof(SmdMsgComHdr) + sizeof(SmdMsgTermReq)];

    comHdr  = (SmdMsgComHdr*)termReqBuf;

    comHdr->cmdCode = SMD_MSG_CODE_TERM_REQ;

    comHdr->msgLen = sizeof(SmdMsgTermReq);

    termReq = (SmdMsgTermReq*)&termReqBuf[sizeof(SmdMsgComHdr)];

    termReq->pid = pid;

    ret = rlylib_apiSndFixMsgToHost(&smdInfo->rlylibCb, "SMD", (CHAR*)&termReqBuf, sizeof(termReqBuf));
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Termination message send failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT smulibInt_lhRcvHdlr(SmulibIntMainCb *mainCb, UINT *rt_loopCnt)
{
    SINT ret = RC_OK;
    CHAR host[RLYLIB_MAX_HOST_LEN];
    UINT hostId = 0;
    CHAR *rcvBuf = NULL;
    UINT rcvBufLen = 0;
    UINT loopCnt = 0;
    SmdMsgComHdr *comHdr = NULL;
    RlylibCb *rlylibCb = NULL;

    rlylibCb = &mainCb->smdInfo.rlylibCb;

    while(1){
        ret = rlylib_apiRcvDynMsgFromAny(rlylibCb, host, &hostId, &rcvBuf, &rcvBufLen);
        if(ret == RLYERR_MSG_NOT_EXIST){
            (*rt_loopCnt)  = loopCnt;
            return RC_OK;
        }
        else if(ret != RC_OK){
            SMU_LOG(SMU_ERR, "Message receive failed(ret=%d)\n",ret);
            (*rt_loopCnt)  = loopCnt;
            return RC_NOK;
        }

        loopCnt++;

        comHdr = (SmdMsgComHdr*)rcvBuf;

        switch(comHdr->cmdCode){
            case SMD_MSG_CODE_REG_CFM:
                {
                    ret = smulibInt_lhRegCfmHdlr(&mainCb->smdInfo, host, (SmdMsgRegCfm*)&rcvBuf[sizeof(SmdMsgComHdr)]);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"RegCfm handling failed(ret=%d)\n",ret);
                        goto goto_rcvRetErr;
                    }
                }
                break;
            case SMD_MSG_CODE_HB_CFM:
                {
                    ret = smulibInt_lhHbCfmHdlr(&mainCb->smdInfo, host, (SmdMsgHBCfm*)&rcvBuf[sizeof(SmdMsgComHdr)]);
                    if(ret != RC_OK){
                        SMU_LOG(SMU_ERR,"Heartbeat Cfm handling failed(ret=%d)\n",ret);
                        goto goto_rcvRetErr;
                    }
                }
                break;
            case SMD_MSG_CODE_TERM_CFM:
                {
                    SMU_LOG(SMU_NOTY,"RECEIVE TERM CFM\n");
                    exit(1);
                }
                break;
            default:
                {
                    SMU_LOG(SMU_ERR,"Invalid command code(%d)\n",comHdr->cmdCode);
                }
                break;
        };

goto_rcvRetErr:
        comlib_memFree(rcvBuf);
    }/* end of while(1) */

    (*rt_loopCnt)  = loopCnt;
    return RC_OK;
}

