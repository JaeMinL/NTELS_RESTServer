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
#include "rlylibInt.h"
#include "rlylibInt.x"

#define MSG_ADD_AVP_OCTET(_avpCode, _src, _srcLen, _msg, _rt_msgLen){\
    UINT d_pndMsg = 0;\
    RLYLIB_PUT_U32((_msg), (_srcLen));\
    RLYLIB_PUT_U32(&(_msg)[4], (_avpCode));\
    comlib_memMemcpy(&(_msg)[8], (_src), (_srcLen));\
    d_pndMsg = ((4-(_srcLen) % 4)) % 4;\
    (_rt_msgLen) = (_srcLen) + d_pndMsg + 8/* hdrLen */;\
}

#define MSG_ADD_AVP_U32(_avpCode, _src, _msg, _rt_msgLen){\
    RLYLIB_PUT_U32((_msg), (4));\
    RLYLIB_PUT_U32(&(_msg)[4], (_avpCode));\
    RLYLIB_PUT_U32(&(_msg)[8], (_src));\
    (_rt_msgLen) = (4/* U32 */) + 8/* hdrLen */;\
}

#define MSG_GET_AVP_HDR(_src, _avpLen, _avpCode, _avpData, _nxtCur){\
    RLYLIB_GET_4BYTE((_avpLen), (_src));\
    RLYLIB_GET_4BYTE((_avpCode), &(_src)[4]);\
    (_avpData) = &(_src)[8];\
    (_nxtCur) += (_avpLen) + 8;\
    (_nxtCur) =  (_nxtCur) + ((4 - ( (_avpLen) % 4))) % 4;\
}

FT_PUBLIC RT_RESULT rlylibInt_msgDecInitReq(CHAR *buf, UINT bufLen, RlylibIntMsgInitReq *rt_initReq)
{
    UINT cur = 0;
    CHAR *avpData = 0;
    UINT avpLen = 0;
    UINT avpCode = 0;
    UINT hostKeyCnt = 0;
    UINT hostCnt = 0;

    while(1){
        MSG_GET_AVP_HDR(&buf[cur], avpLen, avpCode, avpData, cur);
        switch(avpCode){
            case RLYLIB_AVP_CODE_HOST_KEY:
                {
                    if(hostKeyCnt > 0){
                        RLY_LOG(RLY_ERR,"Host already exist\n");
                        return RC_NOK;
                    }

                    rt_initReq->hostKey = avpData;
                    rt_initReq->hostKeyLen = avpLen;
                    hostKeyCnt++;
                }
                break;

            case RLYLIB_AVP_CODE_HOST:
                {
                    if(hostCnt > 0){
                        RLY_LOG(RLY_ERR,"Host already exist\n");
                        return RC_NOK;
                    }

                    rt_initReq->host = avpData;
                    rt_initReq->hostLen = avpLen;
                    hostCnt++;
                }
                break;

            default :
                {
                    RLY_LOG(RLY_ERR,"Invalid avpCode(%d)\n",avpCode);
                    return RLYERR_INVALID_AVP_CODE;
                }
        }/* end of switch(avpCode) */

        if(cur == bufLen){
            break;
        }
    }/* while(1) */

    if((hostCnt != 1) && (hostKeyCnt != 1)){
        RLY_LOG(RLY_ERR,"Mand avp not exist(hostCnt=%d, hostKeyCnt=%d)\n", hostCnt, hostKeyCnt);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgDecInitAck(CHAR *buf, UINT bufLen, RlylibIntMsgInitAck *rt_initAck)
{
    UINT cur = 0;
    CHAR *avpData = 0;
    UINT avpLen = 0;
    UINT avpCode = 0;
    UINT hostCnt = 0;
    UINT rsltCnt = 0;

    while(1){
        MSG_GET_AVP_HDR(&buf[cur], avpLen, avpCode, avpData, cur);
        switch(avpCode){
            case RLYLIB_AVP_CODE_RESULT:
                {
                    if(rsltCnt > 0){
                        RLY_LOG(RLY_ERR,"Result already exist\n");
                        return RC_NOK;
                    }

                    RLYLIB_GET_4BYTE(rt_initAck->rslt, avpData);
                    rsltCnt++;
                }
                break;

            case RLYLIB_AVP_CODE_HOST:
                {
                    if(hostCnt > 0){
                        RLY_LOG(RLY_ERR,"Host already exist\n");
                        return RC_NOK;
                    }

                    rt_initAck->host = avpData;
                    rt_initAck->hostLen = avpLen;
                    hostCnt++;
                }
                break;

            default :
                {
                    RLY_LOG(RLY_ERR,"Invalid avpCode(%d)\n",avpCode);
                    return RLYERR_INVALID_AVP_CODE;
                }
        }/* end of switch(avpCode) */

        if(cur >= bufLen){
            break;
        }
    }/* while(1) */

    if((hostCnt != 1) && (rsltCnt != 1)){
        RLY_LOG(RLY_ERR,"Mand avp not exist(hostCnt=%d, rsltCnt=%d)\n", hostCnt, rsltCnt);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgEncInitReq(RlylibIntMsgInitReq *initReq, CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen)
{
    RlylibIntMsgHdr *msgHdr = NULL;
    CHAR *msg = NULL;
    UINT msgLen = 0;
    UINT avpLen = 0;

    GEN_CHK_ERR_RET(maxBufLen <= sizeof(RlylibIntMsgHdr),
                    RLY_LOG(RLY_ERR,"Message buffer is too small(maxBufLen=%d)\n", maxBufLen),
                    RLYERR_MSG_BUF_TOO_SMALL);

    msgHdr = (RlylibIntMsgHdr*)rt_buf;

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgCode), RLYLIB_MSG_CODE_INIT_REQ);

    msg = (CHAR*)&rt_buf[sizeof(RlylibIntMsgHdr)];

    MSG_ADD_AVP_OCTET(RLYLIB_AVP_CODE_HOST_KEY, initReq->hostKey, initReq->hostKeyLen, msg, avpLen);
    msg += avpLen;
    msgLen += avpLen;

    MSG_ADD_AVP_OCTET(RLYLIB_AVP_CODE_HOST, initReq->host, initReq->hostLen,  msg, avpLen);
    msg += avpLen;
    msgLen += avpLen;

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgLen), msgLen);

    *rt_bufLen = msgLen + sizeof(RlylibIntMsgHdr);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgEncInitAck(RlylibIntMsgInitAck *initAck, CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen)
{
    RlylibIntMsgHdr *msgHdr = NULL;
    CHAR *msg = NULL;
    UINT msgLen = 0;
    UINT avpLen = 0;

    GEN_CHK_ERR_RET(maxBufLen <= sizeof(RlylibIntMsgHdr),
                    RLY_LOG(RLY_ERR,"Message buffer is too small(maxBufLen=%d)\n", maxBufLen),
                    RLYERR_MSG_BUF_TOO_SMALL);

    msgHdr = (RlylibIntMsgHdr*)rt_buf;

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgCode), RLYLIB_MSG_CODE_INIT_ACK);

    msg = (CHAR*)&rt_buf[sizeof(RlylibIntMsgHdr)];

    MSG_ADD_AVP_U32(RLYLIB_AVP_CODE_RESULT, initAck->rslt, msg, avpLen);
    msg += avpLen;
    msgLen += avpLen;

    MSG_ADD_AVP_OCTET(RLYLIB_AVP_CODE_HOST, initAck->host, initAck->hostLen,  msg, avpLen);
    msg += avpLen;
    msgLen += avpLen;

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgLen), msgLen);

    *rt_bufLen = msgLen + sizeof(RlylibIntMsgHdr);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgEncHBeatInd(CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen)
{
    RlylibIntMsgHdr *msgHdr = NULL;

    GEN_CHK_ERR_RET(maxBufLen <= sizeof(RlylibIntMsgHdr),
                    RLY_LOG(RLY_ERR,"Message buffer is too small(maxBufLen=%d)\n", maxBufLen),
                    RLYERR_MSG_BUF_TOO_SMALL);

    msgHdr = (RlylibIntMsgHdr*)rt_buf;

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgCode), RLYLIB_MSG_CODE_HB);

    RLYLIB_PUT_U32((CHAR*)(&msgHdr->msgLen), 0);

    *rt_bufLen = sizeof(RlylibIntMsgHdr);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgInitMsgBufInfo(RlylibIntMsgBufInfo *msgBufInfo, UINT maxBufCnt)
{
    SINT ret = RC_OK;
    UINT i = 0;

    msgBufInfo->maxBufCnt = maxBufCnt;

    if(maxBufCnt != 0){
        for(i=0;i<RLYLIB_MSG_BUF_CNT;i++){
            ret = thrlib_tqInit(&msgBufInfo->freeBuf[i], THR_TQ_LOCK_TYPE_LOCK_FREE, maxBufCnt);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"message buffer queue init failed(ret=%d)\n",ret);
                return RLYERR_THRD_Q_INIT_FAILED;
            }
        }
    }/* end of if(maxBufCnt != 0) */

    ret = thrlib_mutxInit(&msgBufInfo->allocMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"alloc mutex init failed(ret=%d)\n",ret);
        return RLYERR_MUTX_INIT_FAILED;
    }

    ret = thrlib_mutxInit(&msgBufInfo->freeMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Free mutex init failed(ret=%d)\n",ret);
        return RLYERR_MUTX_INIT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgDstryMsgBufInfo(RlylibIntMsgBufInfo *msgBufInfo)
{
    SINT ret = RC_OK;
    UINT i = 0;
    VOID *rcvMsg = NULL;

    if(msgBufInfo->maxBufCnt != 0){
        for(i=0;i<RLYLIB_MSG_BUF_CNT;i++){
            while(1){
                ret = thrlib_tqPop(&msgBufInfo->freeBuf[i], &rcvMsg);
                if(ret == THRERR_TQ_EMPTY){
                    break;
                }
                else if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"message buffer pop error(ret=%d)\n",ret);
                    break;
                }

                comlib_memFree(rcvMsg);
            }

            ret = thrlib_tqDstry(&msgBufInfo->freeBuf[i]);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"thread queue destory failed(ret=%d)\n",ret);
            }
        }/* end of for(i=0;i<RLYLIB_MSG_BUF_CNT;i++) */

        msgBufInfo->maxBufCnt = 0;
    }/* end of if(msgBufInfo->maxBufCnt != 0) */

    ret = thrlib_mutxDstry(&msgBufInfo->allocMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Alloc mutex destroy failed(ret=%d)\n",ret);
    }

    ret = thrlib_mutxDstry(&msgBufInfo->freeMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Free mutex destroy failed(ret=%d)\n",ret);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgAllocMsgBuf(RlylibIntMsgBufInfo *msgBufInfo, UINT size, VOID **rt_buf, UINT *rt_size)
{
    SINT ret = RC_OK;
    UINT bufSize = 0;
    UINT bufQIdx = 0;
    CHAR *buf = NULL;

    /* check size */
    if(size <= 512){
        bufSize = 512;
        bufQIdx = RLYLIB_MSG_512_BUF_IDX;
    }
    else if(size <= 1024){
        bufSize = 1024;
        bufQIdx = RLYLIB_MSG_1024_BUF_IDX;
    }
    else if(size <= 2048){
        bufSize = 2048;
        bufQIdx = RLYLIB_MSG_2048_BUF_IDX;
    }
    else {
        bufSize = size;
        goto goto_alloc;
    }

#if 0
    if(msgBufInfo->totBufCnt == 0){
        goto goto_alloc;
    }
#endif
    thrlib_mutxLock(&msgBufInfo->allocMutx);

    ret = thrlib_tqPop(&msgBufInfo->freeBuf[bufQIdx], (VOID*)&buf);
    if(ret == THRERR_TQ_EMPTY){
        thrlib_mutxUnlock(&msgBufInfo->allocMutx);
        goto goto_alloc;
    }
    else if(ret != RC_OK){
        RLY_LOG(RLY_ERR," Free buffer queue pop failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&msgBufInfo->allocMutx);
        goto goto_alloc;
    }

    thrlib_mutxUnlock(&msgBufInfo->allocMutx);

    *(rt_size) = bufSize;
    *(rt_buf) = buf;

    return RC_OK;

goto_alloc:
    *(rt_size) = bufSize;
    *(rt_buf) = comlib_memMalloc(bufSize);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_msgFreeMsgBuf(RlylibIntMsgBufInfo *msgBufInfo, VOID *buf, UINT size)
{
    SINT ret = RC_OK;
    UINT bufQIdx = 0;
    BOOL freeBufFlg = RC_TRUE;

    /* check size */
    if(size == 512){
        bufQIdx = RLYLIB_MSG_512_BUF_IDX;
        freeBufFlg = RC_FALSE;
    }
    else if(size == 1024){
        bufQIdx = RLYLIB_MSG_1024_BUF_IDX;
        freeBufFlg = RC_FALSE;
    }
    else if(size == 2048){
        bufQIdx = RLYLIB_MSG_2048_BUF_IDX;
        freeBufFlg = RC_FALSE;
    }
    else {
        goto goto_free;
    }

#if 0
    if(msgBufInfo->totBufCnt >= msgBufInfo->maxBufCnt){
        freeBufFlg = RC_FALSE;
    }
#endif

    if(freeBufFlg == RC_TRUE){
        goto goto_free;
    }
    else {
        thrlib_mutxLock(&msgBufInfo->freeMutx);

        ret = thrlib_tqPush(&msgBufInfo->freeBuf[bufQIdx], buf);
        if(ret == THRERR_TQ_FULL){
            thrlib_mutxUnlock(&msgBufInfo->freeMutx);
            goto goto_free;
        }
        else if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"buffer push failed(ret=%d)\n",ret);
            thrlib_mutxUnlock(&msgBufInfo->freeMutx);
            goto goto_free;
        }

        thrlib_mutxUnlock(&msgBufInfo->freeMutx);
    }

    return RC_OK;

goto_free:
    comlib_memFree(buf);

    return RC_OK;
}

