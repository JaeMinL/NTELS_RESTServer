#include <stdio.h>

#include <errno.h>
#include <string.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"
#include "rlylibInt.h"
#include "rlylibInt.x"

FT_PRIVATE RlylibIntRlmCb*      rlm_findRlmByRlm          (RlylibIntMainCb *intMainCb, CHAR *rlm, UINT rlmLen);
FT_PRIVATE RlylibIntRegHostBkt* rlm_findRegHostBktByHost  (RlylibIntRlmCb *rlmCb, CHAR *host, UINT hostLen);
FT_PRIVATE RlylibIntRegHostBkt* rlm_getActvRegHostBkt     (RlylibIntRlmCb *rlmCb, UINT stHostId);
FT_PRIVATE RT_RESULT            rlm_getActvHostByRule     (RlylibIntRlmCb *rlmCb, RlylibRlmKey *rlmKey, RlylibIntHostCb **rt_hostCb);
FT_PRIVATE RT_RESULT            rlm_getHashFromU32        (RlylibRlmKey *srcKey, U_32 hashKey, BOOL bitHashFlg, UINT *rt_val);
FT_PRIVATE RT_RESULT            rlm_getHashFromStr        (RlylibRlmKey *srcKey, U_32 hashKey, BOOL bitHashFlg , UINT *rt_val);

FT_PRIVATE RT_RESULT rlm_getHashFromU32(RlylibRlmKey *srcKey, U_32 hashKey, BOOL bitHashFlg, UINT *rt_val)
{
    if(bitHashFlg == RC_TRUE){
        *rt_val= (UINT)(*((UINT*)srcKey->key) & hashKey);
    }
    else{
        *rt_val= (UINT)(*((UINT*)srcKey->key) % hashKey);
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT rlm_getHashFromStr(RlylibRlmKey *srcKey, U_32 hashKey, BOOL bitHashFlg , UINT *rt_val)
{
    UINT    strIdx = 0;
    UINT    sum = 0;

    for(strIdx = 0; strIdx < srcKey->keyLen;strIdx++){
        sum = (U_32)((31 * sum) + ((UCHAR*)srcKey->key)[strIdx]);
    }

    if(bitHashFlg == RC_TRUE){
        *rt_val = (UINT)(sum & hashKey);
    }
    else {
        *rt_val = (UINT)(sum % hashKey);
    }

    return RC_OK;
}

FT_PRIVATE RlylibIntRlmCb* rlm_findRlmByRlm(RlylibIntMainCb *intMainCb, CHAR *rlm, UINT rlmLen)
{
    SINT ret = RC_OK;
    ComlibHashKey hKey;
    ComlibHashNode *hNode;

    hKey.key = rlm;
    hKey.keyLen = rlmLen;

    ret = comlib_hashTblFindHashNode(&intMainCb->rlmHt, &hKey, 0, &hNode);
    if(ret == RC_OK){
        return hNode->data;
    }

    return NULL;
}

FT_PRIVATE RlylibIntRegHostBkt* rlm_findRegHostBktByHost(RlylibIntRlmCb *rlmCb, CHAR *host, UINT hostLen)
{
    SINT ret = RC_OK;
    ComlibHashKey hKey;
    ComlibHashNode *hNode;

    hKey.key = host;
    hKey.keyLen = hostLen;

    ret = comlib_hashTblFindHashNode(&rlmCb->regHostHt, &hKey, 0, &hNode);
    if(ret == RC_OK){
        return hNode->data;
    }

    return NULL;
}

FT_PRIVATE RlylibIntRegHostBkt* rlm_getActvRegHostBkt(RlylibIntRlmCb *rlmCb, UINT stHostId)
{
    UINT i = 0;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntRegHostBkt *regHostBkt = NULL;

    regHostBkt = rlmCb->regHostBkt[stHostId];
    if(regHostBkt == NULL){
        COM_GET_LNKLST_FIRST(&rlmCb->regHostLst, lnkNode);
        if(lnkNode == NULL){
            RLY_LOG(RLY_ERR,"Registerd host not exist\n");
            return NULL;
        }
        regHostBkt = lnkNode->data;
    }

    for(i=0;i<rlmCb->regHostCnt;i++){
        if(regHostBkt->regHost->status == RLYLIB_HOST_STA_OPEN){
            break;
        }

        lnkNode = &regHostBkt->lnkNode;

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            COM_GET_LNKLST_FIRST(&rlmCb->regHostLst, lnkNode);
            if(lnkNode == NULL){
                RLY_LOG(RLY_ERR,"Open host not exist\n");
                return NULL;
            }
        }

        regHostBkt = lnkNode->data;
    }/* end of for(i=0;i<rlmCb->regHostCnt;i++) */

    if(i == rlmCb->regHostCnt){
        return NULL;
    }

    return regHostBkt;
}

/* thread unsafe */
FT_PRIVATE RT_RESULT rlm_getActvHostByRule(RlylibIntRlmCb *rlmCb, RlylibRlmKey *rlmKey, RlylibIntHostCb **rt_hostCb)
{
    SINT ret = RC_OK;
    UINT i = 0;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntRegHostBkt *regHostBkt = NULL;

    switch(rlmCb->rule){
        case RLYLIB_RLM_RULE_RR:
            {
                regHostBkt = rlm_getActvRegHostBkt(rlmCb, rlmCb->ruleData.u.rndRbin.nxtSndRegHostId);
                if(regHostBkt == NULL){
                    RLY_LOG(RLY_ERR,"Active host not exist\n");
                    return RLYERR_REG_HOST_NOT_EXIST;
                }

                *(rt_hostCb) = regHostBkt->regHost;

                lnkNode = &regHostBkt->lnkNode;
                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    COM_GET_LNKLST_FIRST(&rlmCb->regHostLst, lnkNode);
                }
                regHostBkt = lnkNode->data;

                rlmCb->ruleData.u.rndRbin.nxtSndRegHostId = regHostBkt->spId;
            }
            break;
        case RLYLIB_RLM_RULE_PS:
            {
                regHostBkt = rlmCb->regHostBkt[rlmCb->ruleData.u.priSec.priRegHostId];
                if((regHostBkt == NULL) || 
                   (regHostBkt->regHost->status != RLYLIB_HOST_STA_OPEN)){

                    regHostBkt = rlm_getActvRegHostBkt(rlmCb, rlmCb->ruleData.u.priSec.priRegHostId);
                    if(regHostBkt == NULL){
                        RLY_LOG(RLY_ERR,"Active host not exist\n");
                        return RLYERR_REG_HOST_NOT_EXIST;
                    }
                }

                *(rt_hostCb) = regHostBkt->regHost;
            }
            break;
        case RLYLIB_RLM_RULE_HASH:
            {
                U_32 hashData = 0;

                GEN_CHK_ERR_RET(rlmKey == NULL,
                                RLY_LOG(RLY_ERR,"Realm key not exist\n"),
                                RLYERR_INVALID_HASH_KEY_NOT_EXIST);

                if((rlmKey->keyLen > RLYLIB_RLM_HASH_MAX_KEY_LEN) ||
                   (rlmKey->keyLen == 0) ||
                   ((rlmCb->ruleData.u.hash.hashKeyType == RLYLIB_RLM_HASH_KEY_TYPE_U32) && (rlmKey->keyLen != 4))){
                    RLY_LOG(RLY_ERR,"Invalid Key length(%d)\n",rlmKey->keyLen);
                    return RLYERR_INVALID_HASH_KEY_LEN;
                }

                /* get hashing key */
                if(rlmCb->ruleData.u.hash.hashKeyType == RLYLIB_RLM_HASH_KEY_TYPE_U32){
                    if(rlmCb->ruleData.u.hash.bitMask == 0){
                        ret = rlm_getHashFromU32(rlmKey, rlmCb->regHostCnt, RC_FALSE, &hashData);
                    }
                    else{
                        ret = rlm_getHashFromU32(rlmKey, rlmCb->ruleData.u.hash.bitMask, RC_TRUE, &hashData);
                    }
                }
                else { /* string */
                    if(rlmCb->ruleData.u.hash.bitMask == 0){
                        ret = rlm_getHashFromStr(rlmKey, rlmCb->regHostCnt, RC_FALSE, &hashData);
                    }
                    else{
                        ret = rlm_getHashFromStr(rlmKey, rlmCb->ruleData.u.hash.bitMask, RC_TRUE, &hashData);
                    }
                }

                if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"Key hashing failed(ret=%d)\n",ret);
                    return RLYERR_INVALID_HASHING_FAILED;
                }

                for(i=0;i<rlmCb->regHostCnt;i++){
                    regHostBkt = rlmCb->ruleData.u.hash.regHostBkt[hashData];

                    if(regHostBkt->regHost->status == RLYLIB_HOST_STA_OPEN){
                        break;
                    }

                    hashData++;

                    if(hashData >= rlmCb->regHostCnt){
                        hashData = 0;
                    }
                }/* end of for(i=0;i<rlmCb->regHostCnt;i++) */

                if(i == rlmCb->regHostCnt){
                    RLY_LOG(RLY_ERR,"Can not find active host\n");
                    return RLYERR_REG_HOST_NOT_EXIST;
                }

                *(rt_hostCb) = regHostBkt->regHost;
            }
            break;
        case RLYLIB_RLM_RULE_DHT:
        default:
            {
                RLY_LOG(RLY_ERR,"Invalid rule type(%d)\n", rlmCb->rule);

                return RC_NOK;
            }
            break;
    }

    return RC_OK;
}


FT_PUBLIC RT_RESULT rlylibInt_rlmMakeRlmCb(RlylibIntMainCb *intMainCb, CHAR *rlm, UINT rule, RlylibRlmOptArg *rlmOptArg)
{
    SINT ret = RC_OK;
    UINT rlmId = 0;
    UINT i = 0;
    UINT maxRegHostCnt = 0;
    UINT hashKeyType = 0;
    UINT hashType = 0;
    RlylibIntRlmCb *rlmCb = NULL;

    if(rlmOptArg != NULL){
        maxRegHostCnt = rlmOptArg->maxRegHostCnt;
        hashKeyType = rlmOptArg->hashKeyType;
        hashType = rlmOptArg->hashType;
    }
    else {
        maxRegHostCnt =  RLYLIB_DFLT_MAX_REG_HOST_CNT;
        hashKeyType = RLYLIB_RLM_HASH_KEY_TYPE_STR;
        hashType = RLYLIB_RLM_HASH_TYPE_HASHING_ALL_HOST;
    }

    for(i=0;i<intMainCb->rlmMainCb.maxRlmCnt;i++){
        if(intMainCb->rlmCb[i] == NULL){
            break;
        }
    }

    if(i == intMainCb->rlmMainCb.maxRlmCnt){
        return RLYERR_RLM_IS_FULL;
    }
    else {
        rlmId = i;
    }

    rlmCb = comlib_memMalloc(sizeof(RlylibIntRlmCb));
    if(rlmCb == NULL){
        RLY_LOG(RLY_ERR,"Realm control block alloc failed(errno=%d:%s\n", errno, strerror(errno));
        return RLYERR_RLM_CB_ALLOC_FAILED;
    }

    /* init */
    rlmCb->spId = rlmId;

    ret = thrlib_mutxInit(&rlmCb->mutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"relay mutex init failed(ret=%d)\n",ret);
        comlib_memFree(rlmCb);
        return RLYERR_MUTX_INIT_FAILED;
    }

    comlib_strCpy(rlmCb->locRlm, rlm);

    /* check rule */
    if(rule != RLYLIB_RLM_RULE_RR &&
            rule != RLYLIB_RLM_RULE_PS &&
            rule != RLYLIB_RLM_RULE_HASH &&
            rule != RLYLIB_RLM_RULE_DHT){
        RLY_LOG(RLY_ERR,"Invalid realm rule(rule=%d)\n",rule);
        comlib_memFree(rlmCb);
        return RLYERR_INVALID_RLM_RULE;
    }

    rlmCb->rule = rule;

    switch(rule){
        case RLYLIB_RLM_RULE_RR:
            {
                rlmCb->ruleData.u.rndRbin.nxtSndRegHostId = 0;
            }
            break;

        case RLYLIB_RLM_RULE_PS:
            {
                rlmCb->ruleData.u.priSec.priRegHostId = 0;
            }
            break;
        case RLYLIB_RLM_RULE_HASH:
            {
                if((hashKeyType != RLYLIB_RLM_HASH_KEY_TYPE_U32) && 
                        (hashKeyType != RLYLIB_RLM_HASH_KEY_TYPE_STR)){
                    RLY_LOG(RLY_ERR,"Invalid hashKeyType(%d)\n",hashKeyType);
                    comlib_memFree(rlmCb);

                    return RLYERR_INVALID_HASH_KEY_TYPE;
                }

                if((hashType != RLYLIB_RLM_HASH_TYPE_HASHING_ACT_HOST) &&
                        hashType != RLYLIB_RLM_HASH_TYPE_HASHING_ALL_HOST){
                    RLY_LOG(RLY_ERR,"Invalid hashType(%d)\n",hashType);
                    comlib_memFree(rlmCb);

                    return RLYERR_INVALID_HASH_TYPE;
                }

                rlmCb->ruleData.u.hash.hashKeyType = hashKeyType;
                rlmCb->ruleData.u.hash.hashType = hashType;

                rlmCb->ruleData.u.hash.regHostBkt = comlib_memMalloc(sizeof(RlylibIntRegHostBkt*) * maxRegHostCnt);
                comlib_memMemset(rlmCb->ruleData.u.hash.regHostBkt, 0x0, sizeof(RlylibIntRegHostBkt*) * maxRegHostCnt);
            }
            break;
        default:
            {
            }
            break;
    };

    rlmCb->nxtRcvHostId = 0;
    rlmCb->maxRegHostCnt = maxRegHostCnt;
    rlmCb->regHostCnt = 0;
    rlmCb->regHostBkt = comlib_memMalloc(sizeof(RlylibIntRegHostBkt*) * maxRegHostCnt);
    comlib_memMemset(rlmCb->regHostBkt, 0x0, sizeof(RlylibIntRegHostBkt*) * maxRegHostCnt);

    ret = thrlib_tqInit(&rlmCb->sndTq, THR_TQ_LOCK_TYPE_LOCKED, RLYLIB_DFLT_MAX_TQ_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"realm send queue init failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }
    ret = thrlib_tqInit(&rlmCb->ctrlTq, THR_TQ_LOCK_TYPE_LOCKED, RLYLIB_DFLT_MAX_TQ_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"realm send queue init failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }

    ret = comlib_lnkLstInit(&rlmCb->regHostLst, ~0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Reg host linked listed list init failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }

    ret = comlib_hashTblInit(&rlmCb->regHostHt, maxRegHostCnt, RC_FALSE, COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Reg host hash table init failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }

    rlmCb->hKey.key = rlmCb->locRlm;
    rlmCb->hKey.keyLen = comlib_strGetLen(rlmCb->locRlm);

    rlmCb->thrdLoopLnkNode.prev = NULL;
    rlmCb->thrdLoopLnkNode.next = NULL;
    rlmCb->thrdLoopLnkNode.data = rlmCb;

    rlmCb->hNode.data = rlmCb;

    ret = comlib_hashTblInsertHashNode(&intMainCb->rlmHt, &rlmCb->hKey, &rlmCb->hNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host Hash table insert failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }

    ret = comlib_lnkLstInsertTail(&intMainCb->rlmMainCb.rlmLnkLst, &rlmCb->thrdLoopLnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"rlm main linked list insert failed(ret=%d)\n",ret);
        goto goto_freeRlmCb;
    }

    intMainCb->rlmMainCb.rlmCnt++;

    intMainCb->rlmCb[rlmId] = rlmCb;

    return RC_OK;

goto_freeRlmCb:
    comlib_memFree(rlmCb->regHostBkt);
    comlib_memFree(rlmCb);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmRcvFixMsgFromRlm(RlylibIntMainCb *intMainCb, CHAR *realm, CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen, CHAR *rt_host, UINT *rt_hostId)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT realmLen = 0;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntRlmCb *rlmCb = NULL;
    RlylibIntRegHostBkt *regHostBkt = NULL;
    RlylibIntHostCb *hostCb =  NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    realmLen = comlib_strGetLen(realm);

    /* mutex lock */
    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    /* find realm */
    rlmCb = rlm_findRlmByRlm(intMainCb, realm, realmLen);
    if(rlmCb == NULL){
        RLY_LOG(RLY_ERR,"Can not find realm(%s)\n",realm);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

        return RLYERR_RLM_NOT_EXIST;
    }

    regHostBkt = rlmCb->regHostBkt[rlmCb->nxtRcvHostId];
    if(regHostBkt == NULL){
        COM_GET_LNKLST_FIRST(&rlmCb->regHostLst, lnkNode);
        if(lnkNode == NULL){
            RLY_LOG(RLY_ERR,"Host not exist\n");
            return RLYERR_REG_HOST_NOT_EXIST;
        }

        regHostBkt = lnkNode->data;
    }
    else {
        lnkNode = &regHostBkt->lnkNode;
    }

    hostCb = regHostBkt->regHost;

    for(i=0;i<rlmCb->maxRegHostCnt;i++){
        ret = rlylibInt_hostRcvFixMsg(intMainCb, hostCb, rt_buf, maxBufLen, rt_bufLen);
        if((ret != RC_OK) && (ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
            RLY_LOG(RLY_ERR,"Rcv fix message error(ret=%d)\n",ret);
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

            return ret;
        }
        else if(ret == RLYERR_MSG_NOT_EXIST){
            /* get next */
            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                COM_GET_LNKLST_FIRST(&rlmCb->regHostLst,lnkNode);
                if(lnkNode == NULL){
                    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

                    return ret;
                }
            }

            regHostBkt = lnkNode->data;

            hostCb = regHostBkt->regHost;
        }
        else if((ret == RC_OK) || (ret == RLYERR_DROP_MSG)){
            if(rt_host != NULL){
                comlib_strCpy(rt_host, hostCb->host);
            }

            if(rt_hostId != NULL){
                *rt_hostId = hostCb->spId;
            }

            break;
        }
    }/* end of for(i=0;i<rlmCb->maxRegHostCnt;i++) */

    if(i == rlmCb->maxRegHostCnt){
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

        return RLYERR_MSG_NOT_EXIST;
    }

    lnkNode = &regHostBkt->lnkNode;

    COM_GET_NEXT_NODE(lnkNode);
    if(lnkNode == NULL){
        COM_GET_LNKLST_FIRST(&rlmCb->regHostLst,lnkNode);
    }
    regHostBkt = lnkNode->data;

    rlmCb->nxtRcvHostId = regHostBkt->spId;

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmSndFixMsgToRlm(RlylibIntMainCb *intMainCb, CHAR *realm, RlylibRlmKey *rlmKey, 
		CHAR *buf, UINT bufLen, CHAR *rt_host, UINT *rt_hostId)
{
    SINT ret = RC_OK;
    UINT realmLen = 0;
    RlylibIntRlmCb *rlmCb = NULL;
    RlylibIntHostCb *hostCb = NULL;
    RlylibIntMsgChk *msgChk = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    /* check buffer length */
    if(bufLen == 0){
        RLY_LOG(RLY_ERR,"Invalid buffer len(zero)\n");
        return RLYERR_INVALID_BUF_LEN;
    }

    /* send message */
    RLYLIB_MAKE_SND_FIX_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, buf, bufLen , msgChk, ret);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer make failed(ret=%d)\n",ret);\
            return ret;
    }

    realmLen = comlib_strGetLen(realm);

    /* mutex lock */
    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    /* find realm */
    rlmCb = rlm_findRlmByRlm(intMainCb, realm, realmLen);
    if(rlmCb == NULL){
        RLY_LOG(RLY_ERR,"Can not find realm(%s)\n",realm);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return RLYERR_RLM_NOT_EXIST;
    }

    ret = rlm_getActvHostByRule(rlmCb, rlmKey, &hostCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Can not find active realm(ret=%d)\n",ret);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }

        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return ret;
    }

    /* send message */
    ret = thrlib_tqPush(hostCb->sndTq, (VOID*)msgChk);
    if(ret != RC_OK){
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        RLY_LOG(RLY_ERR,"Message sned failed(ret=%d, realm=%s)\n",ret, realm);
    }

    if(rt_host != NULL){
        comlib_strCpy(rt_host, hostCb->host);
    }

    if(rt_hostId != NULL){
        *(rt_hostId) = hostCb->spId;
    }

    /* mutex unlock */
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmAddRlm(RlylibIntMainCb *intMainCb, CHAR *realm, UINT rule, RlylibRlmOptArg *rlmOptArg)
{
    SINT ret = RC_OK;

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    thrlib_mutxLock(&intMainCb->rlmMainCb.mutx);

    ret = rlylibInt_rlmMakeRlmCb(intMainCb, realm, rule, rlmOptArg);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Realm control blacok make failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&intMainCb->rlmMainCb.mutx);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

        return ret;
    }

    thrlib_mutxUnlock(&intMainCb->rlmMainCb.mutx);
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmRegHost(RlylibIntMainCb *intMainCb, CHAR *realm, CHAR *host, BOOL priFlg, RlylibRlmKey *rlmKey)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT realmLen = 0;
    UINT hostLen = 0;
    UINT bktId = 0;
    RlylibIntRegRlmBkt *regRlmBkt = NULL;
    RlylibIntRegHostBkt *regHostBkt = NULL;
    RlylibIntRlmCb *rlmCb = NULL;
    RlylibIntHostCb *hostCb = NULL;

    realmLen = comlib_strGetLen(realm);
    hostLen = comlib_strGetLen(host);

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
    thrlib_mutxLock(&intMainCb->rlmMainCb.mutx);
    thrlib_mutxLock(&intMainCb->hostMainCb.mutx);

    /* find realm */
    rlmCb = rlm_findRlmByRlm(intMainCb, realm, realmLen);
    if(rlmCb == NULL){
        RLY_LOG(RLY_ERR,"Realm not exist(ret=%d, realm=%s)\n",ret, realm);
        ret = RLYERR_RLM_NOT_EXIST;
        goto goto_errRet;
    }

    thrlib_mutxLock(&rlmCb->mutx);

    if((rlmCb->rule == RLYLIB_RLM_RULE_DHT) && 
            (rlmKey == NULL)){
        RLY_LOG(RLY_ERR,"Hashing key not exist\n");
        thrlib_mutxUnlock(&rlmCb->mutx);
        ret = RLYERR_INVALID_HASH_KEY_NOT_EXIST;
        goto goto_errRet;
    }

    if(rlmKey != NULL){
        if((rlmKey->keyLen > RLYLIB_RLM_HASH_MAX_KEY_LEN) ||
                (rlmKey->keyLen == 0)){
            RLY_LOG(RLY_ERR,"Invalid hashing key len(%d)\n", rlmKey->keyLen);
            thrlib_mutxUnlock(&rlmCb->mutx);
            ret = RLYERR_INVALID_HASH_KEY_LEN;
            goto goto_errRet;
        }

        switch(rlmCb->rule){
            case RLYLIB_RLM_RULE_HASH:
                {
                    if((rlmCb->ruleData.u.hash.hashKeyType == RLYLIB_RLM_HASH_KEY_TYPE_U32) &&
                            (rlmKey->keyLen > 4)){
                        RLY_LOG(RLY_ERR,"Invalid hashing key len(%d, keyType=U32)\n", rlmKey->keyLen);
                        thrlib_mutxUnlock(&rlmCb->mutx);
                        ret = RLYERR_INVALID_HASH_KEY_LEN;
                        goto goto_errRet;
                    }
                }
                break;
            case RLYLIB_RLM_RULE_DHT:
                {
                }
                break;
        }
    }/* end of if(rlmKey != NULL) */


    /* find host */
    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Host not exist(realm=%.*s, host=%.*s)\n",
                realmLen, realm, hostLen, host);
        thrlib_mutxUnlock(&rlmCb->mutx);
        ret = RLYERR_HOST_CB_NOT_EXIST;
        goto goto_errRet;
    }

    regHostBkt = rlm_findRegHostBktByHost(rlmCb, host, hostLen);
    if(regHostBkt != NULL){
        RLY_LOG(RLY_ERR,"registered Host already exist(realm=%.*s, host=%.*s)\n",
                realmLen, realm, hostLen, host);
        thrlib_mutxUnlock(&rlmCb->mutx);
        ret = RLYERR_REG_HOST_EXIST;
        goto goto_errRet;
    }

    for(i=0;i<rlmCb->maxRegHostCnt;i++){
        if(rlmCb->regHostBkt[i] == NULL){
            break;
        }
    }

    if(i == rlmCb->maxRegHostCnt){
        RLY_LOG(RLY_ERR,"registered Host is full(realm=%.*s, host=%.*s)\n",
                realmLen, realm, hostLen, host);
        thrlib_mutxUnlock(&rlmCb->mutx);
        ret = RLYERR_REG_HOST_FULL;
        goto goto_errRet;
    }

    bktId = i;

    /* make host bkt */
    regHostBkt = comlib_memMalloc(sizeof(RlylibIntRegHostBkt));

    regHostBkt->spId = bktId;

    regHostBkt->lnkNode.data = regHostBkt;
    regHostBkt->hNode.data =  regHostBkt;

    regHostBkt->regHost = hostCb;

    comlib_strCpy(regHostBkt->host, hostCb->host);

    regHostBkt->hKey.key = regHostBkt->host;
    regHostBkt->hKey.keyLen = comlib_strGetLen(regHostBkt->host);

    ret = comlib_lnkLstInsertTail(&rlmCb->regHostLst, &regHostBkt->lnkNode); 
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"host bucket list insert failed(ret=%d, realm=%.*s, host=%.*s)\n",ret,
                realmLen, realm, hostLen, host);
        comlib_memFree(regHostBkt);
        thrlib_mutxUnlock(&rlmCb->mutx);
        ret = RLYERR_LNK_LST_INSERT_FAEILD;
        goto goto_errRet;
    }

    ret = comlib_hashTblInsertHashNode(&rlmCb->regHostHt, &regHostBkt->hKey, &regHostBkt->hNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Hash table insert failed(ret=%d, realm=%.*s, host=%.*s)\n",ret,
                realmLen, realm, hostLen, host);

        ret = comlib_lnkLstDel(&rlmCb->regHostLst, &regHostBkt->lnkNode);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Reg host delete failed(ret=%d)\n",ret);
        }
        comlib_memFree(regHostBkt);
        thrlib_mutxUnlock(&rlmCb->mutx);

        ret = RLYERR_HASH_TBL_INSERT_FAEILD;
        goto goto_errRet;

    }

    if(rlmKey != NULL){
        comlib_memMemcpy(regHostBkt->rlmKeyData, rlmKey->key, rlmKey->keyLen);
        regHostBkt->rlmKey.keyLen = rlmKey->keyLen;
    }
    else {
        regHostBkt->rlmKey.keyLen = 0;
    }

    if(priFlg == RC_TRUE ||
            rlmCb->regHostCnt == 0){
        rlmCb->ruleData.u.priSec.priRegHostId = bktId;
    }

    if(rlmCb->rule == RLYLIB_RLM_RULE_HASH){
        rlmCb->ruleData.u.hash.regHostBkt[rlmCb->regHostCnt] = regHostBkt;
        regHostBkt->hashId =  rlmCb->regHostCnt;

        if(((rlmCb->regHostCnt+1) & (rlmCb->regHostCnt)) == 0){
            rlmCb->ruleData.u.hash.bitMask = rlmCb->regHostCnt;
        }
        else {
            rlmCb->ruleData.u.hash.bitMask = 0;
        }
    }
    else {
        regHostBkt->hashId =  0;
    }

    rlmCb->regHostCnt++;

    rlmCb->regHostBkt[i] = regHostBkt;

    thrlib_mutxUnlock(&rlmCb->mutx);

    /* reg host Cb */
    thrlib_mutxLock(&hostCb->mutx);

    regRlmBkt = comlib_memMalloc(sizeof(RlylibIntRegRlmBkt));

    regRlmBkt->regRlm = rlmCb;

    regRlmBkt->lnkNode.data = regRlmBkt;

    ret = comlib_lnkLstInsertTail(&hostCb->regRlmBktLst, &regRlmBkt->lnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"reg realm insert failed(ret=%d, realm=%.*s, host=%.*s)\n",ret,
                realmLen, realm, hostLen, host);

        ret = comlib_hashTblDelHashNode(&rlmCb->regHostHt, &regHostBkt->hNode);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Reg host hash table delete failed(ret=%d)\n",ret);
        }

        ret = comlib_lnkLstDel(&rlmCb->regHostLst, &regHostBkt->lnkNode);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Reg host linked list delete failed(ret=%d)\n",ret);
        }

        comlib_memFree(regHostBkt);
        comlib_memFree(regRlmBkt);

        rlmCb->regHostCnt--;

        thrlib_mutxUnlock(&hostCb->mutx);

        ret = RLYERR_LNK_LST_INSERT_FAEILD;

        goto goto_errRet;
    }

    hostCb->regRlmCnt++;

    thrlib_mutxUnlock(&hostCb->mutx);

    thrlib_mutxUnlock(&intMainCb->hostMainCb.mutx);
    thrlib_mutxUnlock(&intMainCb->rlmMainCb.mutx);
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;

goto_errRet:
    thrlib_mutxUnlock(&intMainCb->hostMainCb.mutx);
    thrlib_mutxUnlock(&intMainCb->rlmMainCb.mutx);
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmDeregHost(RlylibIntMainCb *intMainCb, CHAR *realm, CHAR *host)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_rlmDstryAll(RlylibIntMainCb *intMainCb)
{
    SINT ret = RC_OK;
    ComlibLnkNode *rlmLnkNode = NULL;
    ComlibLnkNode *hostLnkNode = NULL;
    ComlibLnkNode *rlmBktLnkNode = NULL;
    RlylibIntRlmCb *rlmCb = NULL;
    RlylibIntHostCb *hostCb = NULL;
    RlylibIntRegRlmBkt *regRlmBkt = NULL;
    RlylibIntRlmMainCb *rlmMainCb = NULL;
    RlylibIntRegHostBkt *regHostBkt = NULL;

    rlmMainCb = &intMainCb->rlmMainCb;

    thrlib_mutxLock(&intMainCb->rlmMainCb.mutx);

    while(1){
        rlmLnkNode = comlib_lnkLstGetFirst(&rlmMainCb->rlmLnkLst);
        if(rlmLnkNode == NULL){
            break;
        }

        rlmCb = rlmLnkNode->data;

        thrlib_mutxLock(&rlmCb->mutx);

        /* destroy all host */
        while(1){
            hostLnkNode = comlib_lnkLstGetFirst(&rlmCb->regHostLst);
            if(hostLnkNode == NULL) {
                break;
            }/* end of if(hostLnkNode != NULL) */
            regHostBkt = hostLnkNode->data;

            ret = comlib_hashTblDelHashNode(&rlmCb->regHostHt, &regHostBkt->hNode);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Realm bucket delete failed(ret=%d)\n",ret);
            }

            hostCb = regHostBkt->regHost;

            COM_GET_LNKLST_FIRST(&hostCb->regRlmBktLst, rlmBktLnkNode);
            if(rlmBktLnkNode == NULL){
                break;
            }
            while(1){
                regRlmBkt = rlmBktLnkNode->data;

                if(regRlmBkt->regRlm == rlmCb){
                    comlib_lnkLstDel(&hostCb->regRlmBktLst, rlmBktLnkNode);
                    comlib_memFree(regRlmBkt);
                    break;
                }

                COM_GET_NEXT_NODE(rlmBktLnkNode);
                if(rlmBktLnkNode == NULL){
                    break;
                }
            }/* end of while(1) */

            hostCb->regRlmCnt--;

            comlib_memFree(regHostBkt);
        }/* end of while(1) */

        ret = comlib_hashTblDstry(&rlmCb->regHostHt);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"RegHost hash table delete failed(ret=%d)\n",ret);
        }

        comlib_memFree(rlmCb->regHostBkt);

        thrlib_tqDstry(&rlmCb->sndTq);
        thrlib_tqDstry(&rlmCb->ctrlTq);

        thrlib_mutxUnlock(&rlmCb->mutx);

        thrlib_mutxDstry(&rlmCb->mutx);

        comlib_memFree(rlmCb);
    }/* end of while(1) */

    thrlib_mutxUnlock(&intMainCb->rlmMainCb.mutx);

    thrlib_mutxDstry(&intMainCb->rlmMainCb.mutx);

    comlib_memFree(intMainCb->rlmCb);

    return RC_OK;
}
