#include <unistd.h>
#include <sys/time.h>

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

#define HOST_SOCK_CHK_LOOP 1000

#define HOST_SND_MSG_TO_ANY_HOST(_intMainCb, _priHostCb, _sndBuf, _ret){\
    UINT d_i=0;\
    UINT d_maxHostCnt = 0;\
    ComlibLnkNode *d_lnkNode = NULL;\
    RlylibIntHostCb *d_sndHostCb = NULL;\
    d_sndHostCb = (_priHostCb);\
    d_lnkNode = &(_priHostCb)->mainLnkNode;\
    d_maxHostCnt = (_intMainCb)->hostLnkLst.nodeCnt;\
    (_ret) = RLYERR_MSG_PUSH_FAILED;\
    for(d_i=0;d_i<d_maxHostCnt;d_i++){\
        if(d_sndHostCb->status == RLYLIB_HOST_STA_OPEN){\
            (_ret) = thrlib_tqPush((d_sndHostCb)->sndTq, (_sndBuf));\
            if((_ret) == RC_OK){\
                break;\
            }\
            else {\
                RLY_LOG(RLY_NOTY,"Thread queue push failed(host=%s, ret=%d)\n",d_sndHostCb->host, (_ret));\
            }\
        }\
        COM_GET_NEXT_NODE(d_lnkNode);\
        if(d_lnkNode == NULL){\
            COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, d_lnkNode);\
            if(d_lnkNode == NULL){\
                break;\
            }\
        }\
        d_sndHostCb = d_lnkNode->data;\
    }/* end of for(d_i=0;d_i<d_maxHostCnt;d_i++) */\
    if((_ret) != RC_OK){\
        d_sndHostCb = (_priHostCb);\
        (_ret) = thrlib_tqPush(d_sndHostCb->sndTq, (_sndBuf));\
        if((_ret) != RC_OK){\
            RLY_LOG(RLY_NOTY,"Origin Thread queue push failed(host=%s, ret=%d)\n",d_sndHostCb->host, (_ret));\
        }\
    }/* end of if((_ret) != RC_OK) */\
}

#define HOST_MSG_SEND_TO_SOCK(_connCb, _sndBuf, _sndBufLen, _rt_ret){\
    (_rt_ret) = trnlib_sockWrite((_connCb)->fd, RC_TRUE, (_sndBuf),  (_sndBufLen), NULL);\
    if((_rt_ret) != RC_OK){\
        RLY_LOG(RLY_ERR,"socket send failed(ret=%d)(close socket)\n",(_rt_ret));\
        (_rt_ret) = host_closeConnCb((_connCb));\
        if((_rt_ret) != RC_OK){\
            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",(_rt_ret));\
        }\
    }\
}

#define HOST_MAKE_RCV_MSG_RSLT(_intMainCb, _dynFlg, _rcvBuf, _rcvBufLen, _allocBufLen, _rt_rcvMsgRslt, _rt_ret){\
    SINT d_ret;\
    (_rt_rcvMsgRslt)->rt_bufLen = (_rcvBufLen);\
    if((_dynFlg) == RC_TRUE){\
        (_rt_rcvMsgRslt)->rt_buf = (_rcvBuf);\
    }\
    else{\
        if((_rt_rcvMsgRslt)->rt_bufLen > (_rt_rcvMsgRslt)->allocBufLen){\
            RLY_LOG(RLY_ERR,"Max buffer length is too small(msgLen=%d, bufLen=%d)\n",\
                    (_rt_rcvMsgRslt)->rt_bufLen, (_rt_rcvMsgRslt)->allocBufLen);\
            (_rt_rcvMsgRslt)->rt_bufLen = 0;\
            (_rt_ret) = RLYERR_MSG_BUF_TOO_SMALL;\
        }\
        else {\
            comlib_memMemcpy((_rt_rcvMsgRslt)->rt_buf, (_rcvBuf), (_rt_rcvMsgRslt)->rt_bufLen);\
            rlylibInt_syncMutxLock(&(_intMainCb)->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);\
            (d_ret) = rlylibInt_msgFreeMsgBuf(&(_intMainCb)->msgBufInfo, (_rcvBuf), (_allocBufLen));\
            if((d_ret) != RC_OK){\
                RLY_LOG(RLY_ERR,"Message buffer alloc failed(ret=%d)\n",(d_ret));\
            }\
            rlylibInt_syncMutxUnlock(&(_intMainCb)->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);\
            (_rt_ret) = RC_OK;\
        }\
    }\
}

FT_PRIVATE RlylibIntConnCb*   host_findAnyOpenConnCb     (RlylibIntHostCb *hostCb);
FT_PRIVATE RT_RESULT          host_openConnCb            (RlylibIntConnCb *connCb);
FT_PRIVATE RT_RESULT          host_connOpenChk           (RlylibIntHostCb *hostCb, RlylibIntConnCb *connCb);
FT_PRIVATE RT_RESULT          host_freeConnCb            (RlylibIntConnCb *connCb);
FT_PRIVATE RT_RESULT          host_closeConnCb           (RlylibIntConnCb *connCb);
FT_PRIVATE RT_RESULT          host_freeAllConnCb         (RlylibIntHostCb *hostCb);
FT_PRIVATE RT_RESULT          host_chkSndMsg             (RlylibIntHostCb *hostCb, UINT *rt_procCnt);
FT_PRIVATE RT_RESULT          host_chkDropMsg            (RlylibIntHostCb *hostCb);
FT_PRIVATE RT_RESULT          host_rcvConnMsg            (RlylibIntHostCb *hostCb, RlylibIntConnCb *connCb, UINT *rt_rcvCnt);
FT_PRIVATE RT_RESULT          host_chkAllConn            (RlylibIntHostCb *hostCb, UINT *rt_procCnt);
FT_PRIVATE RT_RESULT          host_chkCtrlMsg            (RlylibIntHostCb *hostCb);

FT_PRIVATE RT_RESULT host_openConnCb(RlylibIntConnCb *connCb)
{
    SINT ret = RC_OK;

    ret = trnlib_sockOpenConn(&connCb->dstAddr, &connCb->fd, RC_TRUE);
    if(ret != RC_OK && 
            ret != TRNERR_CONN_INPROGRESS){
        RLY_LOG(RLY_ERR,"Connection open failed(ret=%d)\n",ret);
        connCb->status = RLYLIB_CONN_STA_CLOSED;
        connCb->fd = 0;

        return RLYERR_CONN_SOCK_OPEN_FAILED;
    }

    connCb->status = RLYLIB_CONN_STA_CONNECTED;

    return RC_OK;
}

FT_PRIVATE RT_RESULT host_connOpenChk(RlylibIntHostCb *hostCb, RlylibIntConnCb *connCb)
{
    SINT ret = RC_OK;
    CHAR tmpMsgBuf[RLYLIB_TMP_MSG_BUF_LEN];
    UINT tmpMsgBufLen = 0;
    RlylibIntMsgInitReq initReq;

    ret = trnlib_sockConnChk(&connCb->dstAddr, connCb->fd);
    if((ret != RC_OK) && (ret != TRNERR_CONN_INPROGRESS)){
        RLY_LOG(RLY_ERR,"Connection check error(ret=%d)\n",ret);
        ret = host_closeConnCb(connCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
        }

        return RLYERR_SOCK_TERM;
    }

    /* send init request */
    initReq.hostLen = comlib_strGetLen(hostCb->locHost);
    initReq.host = hostCb->locHost;
    initReq.hostKey = hostCb->hostKey;
    initReq.hostKeyLen = hostCb->hostKeyLen;

    ret = rlylibInt_msgEncInitReq(&initReq, tmpMsgBuf, RLYLIB_TMP_MSG_BUF_LEN, &tmpMsgBufLen);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Init request message init failed(ret=%d)\n",ret);
        ret = host_closeConnCb(connCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
        }

        return RLYERR_MSG_ENC_FAILED;
    }

    ret = trnlib_sockWrite(connCb->fd, RC_TRUE, tmpMsgBuf, tmpMsgBufLen, NULL);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Init request message send failed(ret=%d)\n",ret);
        ret = host_closeConnCb(connCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
        }

        return RLYERR_SOCK_WRITE_FAILED;
    }


    return RC_OK;
}

FT_PRIVATE RT_RESULT host_closeConnCb(RlylibIntConnCb *connCb)
{
    CHAR strAddr[RLYLIB_STR_TRANS_ADDR_LEN];

    TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, RLYLIB_STR_TRANS_ADDR_LEN, &connCb->dstAddr);
    RLY_LOG(RLY_ERR,"Connection close(addr=%s, fd=%d, status=%d)\n", strAddr, connCb->fd, connCb->status);

    if(connCb->status != RLYLIB_CONN_STA_CLOSED){
        trnlib_sockClose(connCb->fd);
        connCb->fd = 0;
    }

    if(connCb->msgChk != NULL){
        if(connCb->msgChk->data != NULL){
            comlib_memFree(connCb->msgChk->data);
        }
        comlib_memFree(connCb->msgChk);
        connCb->msgChk = NULL;
    }

    connCb->tmpBufLen = 0;
    connCb->remDataLen = 0;

    connCb->status = RLYLIB_CONN_STA_CLOSED;

    return RC_OK;
}

FT_PRIVATE RlylibIntConnCb* host_findAnyOpenConnCb(RlylibIntHostCb *hostCb)
{
    RlylibIntConnCb *connCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(hostCb->rlyMode == RLYLIB_CONN_RLY_MODE_AS){
        COM_GET_LNKLST_FIRST(&hostCb->connLst, lnkNode);
        if(lnkNode == NULL){
            return NULL;
        }

        while(1){
            connCb = lnkNode->data;

            if(connCb->status == RLYLIB_CONN_STA_OPEN){
                return connCb;
            }

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                return NULL;
            }
        }/* end of while(1) */
    }/* end of if(hostCb->rlyMode == RLYLIB_CONN_RLY_MODE_AS) */
    else { /* RR */
        if(hostCb->nxtConn == NULL){
            COM_GET_LNKLST_FIRST(&hostCb->connLst, lnkNode);
            if(lnkNode == NULL){
                return NULL;
            }

            connCb = lnkNode->data;

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                hostCb->nxtConn = NULL;
            }
            else {
                hostCb->nxtConn = lnkNode->data;
            }

            return connCb;
        }
        else {
            connCb = hostCb->nxtConn;

            lnkNode = &connCb->lnkNode;

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                hostCb->nxtConn = NULL;
            }
            else {
                hostCb->nxtConn = lnkNode->data;
            }

            return connCb;
        }/* end of else */
    }/* end of else RR */
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostRcvDynMsg(RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb, 
		CHAR **rt_buf, UINT *rt_bufLen, U_32 *rt_allocBufLen)
{
    SINT ret = RC_OK;
    RlylibIntMsgChk *msgChk = NULL;
    RlylibIntMsgHdr *msgHdr = NULL;

    ret = thrlib_tqPop(hostCb->rcvTq, (VOID*)&msgChk);
    if(ret == THRERR_TQ_EMPTY){
        return RLYERR_MSG_NOT_EXIST;
    }
    else if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread queue pop faild(ret=%d)\n",ret);
        return RLYERR_MSG_POP_FAILED;
    }

    msgHdr = (RlylibIntMsgHdr*)&msgChk->hdrs[sizeof(RlylibIntMsgQHdr)];

    *(rt_buf) = msgChk->data;
    *rt_bufLen = msgHdr->msgLen;
    *rt_allocBufLen = msgChk->dataLen;

    msgChk->dataLen = 0;
    msgChk->data = NULL;

    RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
    }

    if(msgHdr->msgCode == RLYLIB_MSG_CODE_DROP_DATA){
        return RLYERR_DROP_MSG;
    }
    else {
        return RC_OK;
    }
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostRcvFixMsg(RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb, 
		CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen)
{
    SINT ret = RC_OK;
    RlylibIntMsgChk *msgChk = NULL;
    RlylibIntMsgHdr *msgHdr = NULL;

    ret = thrlib_tqPop(hostCb->rcvTq, (VOID*)&msgChk);
    if(ret == THRERR_TQ_EMPTY){
        return RLYERR_MSG_NOT_EXIST;
    }
    else if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread queue pop faild(ret=%d)\n",ret);
        return RLYERR_MSG_POP_FAILED;
    }

    msgHdr = (RlylibIntMsgHdr*)&msgChk->hdrs[sizeof(RlylibIntMsgQHdr)];

    if(msgHdr->msgLen >  maxBufLen){
        RLY_LOG(RLY_ERR,"Max buffer length is too small(msgLen=%d, bufLen=%d)\n",msgHdr->msgLen, maxBufLen);
        if(msgHdr->msgCode == RLYLIB_MSG_CODE_DROP_DATA){
            return RLYERR_DROP_MSG_AND_BUF_TOO_SMALL;
        }
        else {
            return RLYERR_MSG_BUF_TOO_SMALL;
        }
    }

    comlib_memMemcpy(rt_buf, msgChk->data, msgHdr->msgLen);

    *rt_bufLen = msgHdr->msgLen;

    RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
    }

    if(msgHdr->msgCode == RLYLIB_MSG_CODE_DROP_DATA){
        return RLYERR_DROP_MSG;
    }
    else {
        return RC_OK;
    }
}

FT_PRIVATE RT_RESULT host_rcvConnMsg(RlylibIntHostCb *hostCb, RlylibIntConnCb *connCb, UINT *rt_rcvCnt)
{
    SINT ret = RC_OK;
    UINT rcvCnt = 0;
    UINT msgRcvCnt = 0;
    ULONG curTick = 0;
    UINT rcvBufLen = 0;
    U_32 msgCode = 0;
    U_32 msgLen = 0;
    RlylibIntMsgHdr *msgHdr = NULL;
    RlylibIntMsgInitAck initAck;
    UINT tmpBufLen = 0;
    CHAR *stBufPtr = NULL;
    UINT remBufLen = 0;

    tmpBufLen = connCb->tmpBufLen;
    stBufPtr = &connCb->tmpBuf[tmpBufLen];

    ret = trnlib_sockRead(connCb->fd, RLYLIB_TMP_MSG_BUF_LEN - tmpBufLen, stBufPtr, &rcvBufLen);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Conn Socket termination(fd=%d, ret=%d, host=%s)(close socket)\n",
                connCb->fd, ret, hostCb->host);
        ret = host_closeConnCb(connCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
        }

        return RLYERR_SOCK_READ_FAILED;
    }

    tmpBufLen += rcvBufLen;

    remBufLen = tmpBufLen;
    stBufPtr = connCb->tmpBuf;

    while(1){
        if(remBufLen < RLYLIB_MSG_HDR_LEN){
            if(remBufLen != 0){ /* shift */
                comlib_memMemcpy(connCb->tmpBuf, stBufPtr, remBufLen);
            }
            connCb->tmpBufLen = remBufLen;
            break;
        }

        /* make header */
        if((connCb->msgChk == NULL) && (remBufLen >= RLYLIB_MSG_HDR_LEN)){ 
            /* get header  */
            msgHdr = (RlylibIntMsgHdr*)stBufPtr;
            stBufPtr += RLYLIB_MSG_HDR_LEN;
            remBufLen -= RLYLIB_MSG_HDR_LEN;

            RLYLIB_GET_4BYTE(msgCode, (CHAR*)&msgHdr->msgCode);
            RLYLIB_GET_4BYTE(msgLen, (CHAR*)&msgHdr->msgLen);

            msgHdr->msgCode = msgCode;
            msgHdr->msgLen = msgLen;

            if(msgLen != 0){ /* make chunk */
                ret = thrlib_tqPop(&hostCb->freeMsgChkQ, (VOID*)&connCb->msgChk);
                if(ret == THRERR_TQ_EMPTY){
                    connCb->msgChk = comlib_memMalloc(sizeof(RlylibIntMsgChk));
                }
                else if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"Free MsgChk pop failed(ret=%d)\n",ret);
                    connCb->msgChk = comlib_memMalloc(sizeof(RlylibIntMsgChk));
                }

                if(connCb->msgChk == NULL){
                    RLY_LOG(RLY_ERR,"Can not make message chunk\n");
                    return RLYERR_MSG_BUF_ALLOC_FAILED;
                }

                comlib_memMemcpy(&connCb->msgChk->hdrs[sizeof(RlylibIntMsgQHdr)], msgHdr,RLYLIB_MSG_HDR_LEN);

                ret = rlylibInt_msgAllocMsgBuf(&hostCb->msgBufInfo, msgLen, 
                        (VOID*)&connCb->msgChk->data, &connCb->msgChk->dataLen);
                if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"Can not make message buffer(ret=%d)\n",ret);
                    comlib_memFree(connCb->msgChk);
                    connCb->msgChk = NULL;
                    return RLYERR_MSG_BUF_ALLOC_FAILED;
                }

                connCb->remDataLen = msgLen;
                rcvCnt++;
            }/* end of if(msgLen != 0) */
            else {
                if(msgHdr->msgCode == RLYLIB_CONN_TMR_EVNT_HBEAT){
                    /* receive heartbeat */
                    RLY_LOG(RLY_DBG,"Receive Heart beat(host=%s, fd=%d)\n",hostCb->host, connCb->fd);
                    /* reset timer */
                    curTick = rlylibInt_globGetCurTick();
                    connCb->lstUpdTm = curTick;
                }
            }
        }/* end of if((tmpBufLen - msgCur) > RLYLIB_MSG_HDR_LEN) */

        if(connCb->msgChk != NULL){
            msgHdr = (RlylibIntMsgHdr*)&connCb->msgChk->hdrs[sizeof(RlylibIntMsgQHdr)];
            if(connCb->remDataLen > remBufLen){
                comlib_memMemcpy(&connCb->msgChk->data[msgHdr->msgLen - connCb->remDataLen], stBufPtr, remBufLen);
                connCb->remDataLen -= remBufLen;
                stBufPtr = connCb->tmpBuf;
                remBufLen = 0;
            }/* end of if(connCb->remDataLen > remBufLen) */
            else if(connCb->remDataLen <= remBufLen){
                comlib_memMemcpy(&connCb->msgChk->data[msgHdr->msgLen - connCb->remDataLen], stBufPtr, connCb->remDataLen);
                stBufPtr += connCb->remDataLen;
                remBufLen -= connCb->remDataLen;
                connCb->remDataLen = 0;

                /* send message */
                if((msgHdr->msgCode == RLYLIB_MSG_CODE_DATA) && 
                        (connCb->status == RLYLIB_CONN_STA_OPEN)){
                    /* send message to main block */
                    ret = thrlib_tqPush(hostCb->rcvTq, connCb->msgChk);
                    if(ret != RC_OK){
                        RLY_LOG(RLY_ERR,"Thread queue push failed(ret=%d)\n",ret);
                        RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, connCb->msgChk,ret); 
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Message chunk free faield(ret=%d)\n",ret);
                        }
                    }

                    msgRcvCnt++;
                    if(msgRcvCnt == 1){ /* sig once */
                        /* cond signal */
                        thrlib_condSig(hostCb->rdCond);
                    }
                }
                else if((msgHdr->msgCode == RLYLIB_MSG_CODE_INIT_ACK) &&
                        (connCb->status == RLYLIB_CONN_STA_INIT)){
                    /* decode data */
                    ret = rlylibInt_msgDecInitAck(connCb->msgChk->data,
                            msgHdr->msgLen ,&initAck);
                    if(ret != RC_OK){
                        RLY_LOG(RLY_ERR,"Dcode failed(ret=%d)\n",ret);
                        ret = host_closeConnCb(connCb);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
                        }
                        return RLYERR_MSG_DEC_FAILED;
                    }

                    if(initAck.rslt != RLYLIB_RSLT_CODE_SUCC){
                        RLY_LOG(RLY_ERR,"Init ack failed(rsltCode=%d)\n",initAck.rslt);
                        ret = host_closeConnCb(connCb);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
                        }
                        return RLYERR_CONN_OPEN_FAILED;
                    }

                    ret = comlib_memMemcmp(initAck.host, hostCb->host, initAck.hostLen);
                    if(ret != 0){
                        RLY_LOG(RLY_ERR,"Host mismatch(ack=%s, loc=%s)\n", initAck.host, hostCb->host);
                        ret = host_closeConnCb(connCb);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
                        }
                        return RLYERR_CONN_OPEN_FAILED;
                    }

                    connCb->status = RLYLIB_CONN_STA_OPEN;

                    RLYLIB_INC_OPEN_CONN_CNT(hostCb);

                    RLY_LOG(RLY_DBG,"Connection open(fd=%d)\n",connCb->fd);

                    curTick = rlylibInt_globGetCurTick();
                    connCb->lstUpdTm = curTick;
                    RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.hBeatSndTmOut, 
                            RLYLIB_CONN_TMR_EVNT_HBEAT);

                    RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, connCb->msgChk, ret);
                }
                else {
                    RLY_LOG(RLY_ERR,"Invalid message (code=%d, msgLen=%d, sta=%d)(close conn)\n",
                            msgHdr->msgCode, msgHdr->msgLen, connCb->status);
                    ret = host_closeConnCb(connCb);
                    if(ret != RC_OK){
                        RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
                    }
                    return RLYERR_MSG_DEC_FAILED;
                }
                connCb->msgChk = NULL;
            }/* end of else if(connCb->remDataLen <= (tmpBufLen - msgCur)) */
        }/* end of if(connCb->msgChk != NULL) */
    }/* end of while(1) */

    *rt_rcvCnt = rcvCnt;

    return RC_OK;
}

FT_PRIVATE RT_RESULT host_chkAllConn(RlylibIntHostCb *hostCb, UINT *rt_procCnt)
{
    SINT ret = RC_OK;
    UINT rcvCnt = 0;
    UINT connSta = 0;
    UINT procCnt = 0;
    ULONG curTick = 0;
    RlylibIntConnCb *connCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    /* get current time */
    curTick = rlylibInt_globGetCurTick();

    COM_GET_LNKLST_FIRST(&hostCb->connLst, lnkNode);
    if(lnkNode == NULL){
        *rt_procCnt = 0;
        return RC_OK;
    }

    while(1){
        connCb = lnkNode->data;

        rcvCnt = 0;

        connSta = connCb->status;
        if((connSta == RLYLIB_CONN_STA_OPEN) || 
                (connSta == RLYLIB_CONN_STA_INIT)){
            ret = host_rcvConnMsg(hostCb, connCb, &rcvCnt);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Receive message failed(ret=%d)\n",ret);
                if(connSta == RLYLIB_CONN_STA_OPEN){
                    RLYLIB_DEC_OPEN_CONN_CNT(hostCb);
                }

                /* conn timer start */
                if(hostCb->type == RLYLIB_TYPE_SERVER){
                    connCb->tmrHndlNode.used = RC_FALSE;
                }
                else {
                    RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connTmOut, 
                            RLYLIB_CONN_TMR_EVNT_CONN);
                }
            }
            else {
                procCnt += rcvCnt;
                if(rcvCnt > 0){
                    connCb->lstUpdTm = curTick;
                }
            }
        }/* end of if(connCb->status == RLYLIB_CONN_STA_OPEN) */
        else if(connCb->status == RLYLIB_CONN_STA_CONNECTED){
            ret = host_connOpenChk(hostCb, connCb);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Connection open check failed(ret=%d)\n",ret);

                RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, 
                        hostCb->tmrInfo.connTmOut, RLYLIB_CONN_TMR_EVNT_CONN);

            }
            else {
                connCb->status = RLYLIB_CONN_STA_INIT;

                /* start init timer */
                RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, 
                        hostCb->tmrInfo.initTmOut, RLYLIB_CONN_TMR_EVNT_INIT);
            }
        }

        /* check last updated time */
        if((connCb->status == RLYLIB_CONN_STA_OPEN) && 
                ((curTick - connCb->lstUpdTm) >= hostCb->tmrInfo.hBeatTmOut)){
            /* close connection */
            RLY_LOG(RLY_ERR,"Connection Heartbeat timeout(tmOut=%d)\n", hostCb->tmrInfo.hBeatTmOut);

            ret = host_closeConnCb(connCb);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
            }

            RLYLIB_DEC_OPEN_CONN_CNT(hostCb);

            if(hostCb->type == RLYLIB_TYPE_SERVER){
                connCb->tmrHndlNode.used = RC_FALSE;
            }
            else{
                RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connTmOut, 
                        RLYLIB_CONN_TMR_EVNT_CONN);
            }
        }/* end of if((connCb->status == RLYLIB_CONN_STA_OPEN) && 
            (connCb->lstUpdTm > hostCb->tmrInfo.hBeatTmOut)) */ 

        /* check timer */
        if(connCb->tmrHndlNode.used == RC_USED){
            /* check timer */
            if(curTick >= connCb->tmrHndlNode.tmOutTick){
                switch(connCb->tmrHndlNode.evnt){
                    case RLYLIB_CONN_TMR_EVNT_CONN:
                        {
                            CHAR strAddr[RLYLIB_STR_TRANS_ADDR_LEN];

                            /* send to connection */
                            ret = host_openConnCb(connCb);
                            if(ret != RC_OK){
                                TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, RLYLIB_STR_TRANS_ADDR_LEN, &connCb->dstAddr);
                                RLY_LOG(RLY_ERR,"Connection open failed(host=%s, addr=%s, ret=%d)\n",hostCb->host, strAddr, ret);
                                RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connTmOut, 
                                        RLYLIB_CONN_TMR_EVNT_CONN);
                            }
                            else {
                                TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, RLYLIB_STR_TRANS_ADDR_LEN, &connCb->dstAddr);
                                RLY_LOG(RLY_ERR,"Connection open(host=%s, addr=%s, fd=%d)\n",hostCb->host, strAddr, connCb->fd);

                                /* start conn wait tmr */
                                RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connWaitTmOut, 
                                        RLYLIB_CONN_TMR_EVNT_CONN_WAIT);
                            }
                        }
                        break;
                    case RLYLIB_CONN_TMR_EVNT_HBEAT:
                        {
                            RLY_LOG(RLY_DBG,"Send to heart beat(fd=%d)\n",connCb->fd);
                            /* send to heartbeat */
                            ret = rlylibInt_msgEncHBeatInd(hostCb->tmpMsgBuf, RLYLIB_TMP_MSG_BUF_LEN, &hostCb->tmpMsgBufLen);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR,"Hbeat indication encoding failed(ret=%d)\n",ret);
                            }
                            else {
                                ret = trnlib_sockWrite(connCb->fd, RC_TRUE, hostCb->tmpMsgBuf, hostCb->tmpMsgBufLen, NULL);
                                if(ret != RC_OK){
                                    RLY_LOG(RLY_ERR,"HBeat request message send failed(ret=%d)\n",ret);
                                    ret = host_closeConnCb(connCb);
                                    if(ret != RC_OK){
                                        RLY_LOG(RLY_ERR,"Connection Control block close failed(ret=%d)\n",ret);
                                    }

                                    RLYLIB_DEC_OPEN_CONN_CNT(hostCb);
                                    connCb->tmrHndlNode.used = RC_FALSE;

                                    if(hostCb->type != RLYLIB_TYPE_SERVER){
                                        RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connTmOut, 
                                                                      RLYLIB_CONN_TMR_EVNT_CONN);
                                    }
                                }/* if(ret != RC_OK) */
                                else {
                                    RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.hBeatSndTmOut, 
                                                                  RLYLIB_CONN_TMR_EVNT_HBEAT);
                                }

                                hostCb->tmpMsgBufLen = 0;
                            }
                        }
                        break;
                    case RLYLIB_CONN_TMR_EVNT_INIT:
                        {
                            RLY_LOG(RLY_ERR,"Connection init timeout\n");
                            host_closeConnCb(connCb);
                            RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.connTmOut, 
                                    RLYLIB_CONN_TMR_EVNT_CONN);
                        }
                        break;
                };/* end of switch(connCb->tmrHndlNode.evnt) */
            }/* end of if(diff >= connCb->tmrHndlNode.tmOutTick) */
        }/* end of if(connCb->tmrHndlNod.used == RC_USED) */

        /* get next node */
        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            *rt_procCnt = procCnt;
            return RC_OK;
        }
    }/* end of while(1) */

    *rt_procCnt = procCnt;
    return RC_OK;
}

FT_PRIVATE RT_RESULT host_chkDropMsg(RlylibIntHostCb *hostCb)
{
    SINT ret = RC_OK;
    UINT dropCnt = 0;
    ULONG curTick = 0;
    RlylibIntMsgQHdr *msgQHdr = NULL;
    RlylibIntMsgHdr *msgHdr = NULL;

    curTick = rlylibInt_globGetCurTick();

    while(1){
        if(hostCb->tmpSndMsgChk != NULL){
            msgQHdr = (RlylibIntMsgQHdr*)hostCb->tmpSndMsgChk->hdrs;
        }
        else {
            ret = thrlib_tqPop(hostCb->sndTq, (VOID*)&hostCb->tmpSndMsgChk);
            if(ret == THRERR_TQ_EMPTY){
                return RC_OK;
            }
            else if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"send thread queue pop error(ret=%d)\n",ret);
                return RLYERR_THR_Q_POP_ERROR;
            }

            msgQHdr = (RlylibIntMsgQHdr*)hostCb->tmpSndMsgChk->hdrs;
        }

        if((curTick - msgQHdr->msgTick) >= hostCb->tmrInfo.msgDropTmOut){
            /* drop message */
            RLY_LOG(RLY_DBG,"Drop message(curTick=%d, tick=%d, tmOut=%d)\n",
                    curTick, msgQHdr->msgTick, hostCb->tmrInfo.msgDropTmOut);
            if(hostCb->msgDropIndFlg == RC_FALSE){
                RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, hostCb->tmpSndMsgChk, ret);
                if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
                }
            }
            else {
                msgHdr = (RlylibIntMsgHdr*)&((CHAR*)hostCb->tmpSndMsgChk->hdrs)[sizeof(RlylibIntMsgQHdr)];

                /* message code */
                RLY_LOG(RLY_DBG,"Drop message Ind\n");
                msgHdr->msgCode = RLYLIB_MSG_CODE_DROP_DATA;
                ret = thrlib_tqPush(hostCb->rcvTq, hostCb->tmpSndMsgChk);
                if(ret != RC_OK){
                    RLY_LOG(RLY_NOTY,"drop data push failed(host=%s, ret=%d)\n",hostCb->host, ret);
                }
            }

            hostCb->tmpSndMsgChk = NULL;
            dropCnt++;
        }/* if((curTick - msgQHdr->msgTick) >= hostCb->msgDropTm) */
        else {
            break;
        }
    }/* end of while(1) */

    if(dropCnt != 0){
        RLY_LOG(RLY_ERR,"Drop count=%d\n",dropCnt);
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT host_chkSndMsg(RlylibIntHostCb *hostCb, UINT *rt_procCnt)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT remTmpBufLen = 0;
    UINT sndMsgLen = 0;
    UINT curTick = 0;
    UINT procCnt = 0;
    UINT msgLen = 0;
    U_32 bufSize = 0;
    CHAR *sndBuf = NULL;
    RlylibIntMsgChk *msgChk= NULL;
    RlylibIntMsgHdr *msgHdr = NULL;
    RlylibIntConnCb *connCb = NULL;

    /* find connection */
    connCb = host_findAnyOpenConnCb(hostCb);
    if(connCb == NULL){
        *rt_procCnt = 0;
        ret = host_chkDropMsg(hostCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Drop message check failed(ret=%d)\n",ret);
        }

        return RC_OK;
    }

    for(i=0;i<RLYLIB_SND_LOOP_CNT;i++){
        /* get message queue */
        if(hostCb->tmpSndMsgChk == NULL){
            ret = thrlib_tqPop(hostCb->sndTq, (VOID*)&msgChk);
            if(ret == THRERR_TQ_EMPTY){
                *rt_procCnt = 0;
                break;
            }
            else if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"send thread queue pop error(ret=%d)\n",ret);
                *rt_procCnt = 0;
                return RLYERR_THR_Q_POP_ERROR;
            }
        }
        else {
            msgChk = hostCb->tmpSndMsgChk;
            hostCb->tmpSndMsgChk= NULL;
        }

        procCnt++;
        /* make send message */
        remTmpBufLen = RLYLIB_TMP_MSG_BUF_LEN - hostCb->tmpMsgBufLen;

        /* send message */
        //msgQHdr = (RlylibIntMsgQHdr*)msgChk->hdrs;

        /* make send data */
        if(msgChk->msgHdrIncFlg == RC_TRUE){
            msgHdr = (RlylibIntMsgHdr*)msgChk->data;
        }
        else {
            msgHdr = (RlylibIntMsgHdr*)&msgChk->hdrs[sizeof(RlylibIntMsgQHdr)];
        }

        sndMsgLen = msgHdr->msgLen + sizeof(RlylibIntMsgHdr);
        msgLen = msgHdr->msgLen;

        while(1){
            if(sndMsgLen <= remTmpBufLen){
                sndBuf = &hostCb->tmpMsgBuf[hostCb->tmpMsgBufLen];

                if(msgChk->msgHdrIncFlg == RC_TRUE){
                    comlib_memMemcpy(sndBuf, msgChk->data, sndMsgLen);
                }
                else {
                    comlib_memMemcpy(sndBuf, msgHdr, sizeof(RlylibIntMsgHdr));
                    comlib_memMemcpy(&sndBuf[sizeof(RlylibIntMsgHdr)], msgChk->data, msgLen);
                }

                msgHdr = (RlylibIntMsgHdr*)sndBuf;

                RLYLIB_PUT_U32((CHAR*)&msgHdr->msgLen, msgLen);
                RLYLIB_PUT_U32((CHAR*)&msgHdr->msgCode, RLYLIB_MSG_CODE_DATA);

                remTmpBufLen -= sndMsgLen;
                hostCb->tmpMsgBufLen += sndMsgLen;

                RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, msgChk, ret);
                if(ret != RC_OK){
                    RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
                }

                break;
            }
            else {
                /* send temp message */
                if(hostCb->tmpMsgBufLen != 0){
                    HOST_MSG_SEND_TO_SOCK(connCb, hostCb->tmpMsgBuf, hostCb->tmpMsgBufLen, ret);
                    if(ret != RC_OK){
                        RLYLIB_DEC_OPEN_CONN_CNT(hostCb);
                        *rt_procCnt = 0;
                        RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, msgChk, ret);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
                        }

                        return RC_NOK;
                    }

                    hostCb->tmpMsgBufLen = 0;
                    remTmpBufLen = RLYLIB_TMP_MSG_BUF_LEN;
                }/* end of if(connCb->tmpMsgBufLen != 0) */

                if(sndMsgLen <= remTmpBufLen){
                    continue;
                }
                else {
                    if(msgChk->msgHdrIncFlg == RC_TRUE){
                        sndBuf = msgChk->data;
                    }
                    else {
                        /* make new message */
                        ret = rlylibInt_msgAllocMsgBuf(&hostCb->msgBufInfo, sndMsgLen, (VOID*)&sndBuf, &bufSize);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Message buffer alloc failed(ret=%d)\n",ret);
                            RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, msgChk, ret);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR,"Buffer free failed(ret=%)\n",ret);
                            }

                            msgChk = NULL;

                            return RC_NOK;
                        }
                        /* make message */
                        comlib_memMemcpy((CHAR*)sndBuf, (CHAR*)msgHdr, (UINT)sizeof(RlylibIntMsgHdr));
                        comlib_memMemcpy(&sndBuf[sizeof(RlylibIntMsgHdr)], msgChk->data, msgLen);
                    }

                    msgHdr = (RlylibIntMsgHdr*)sndBuf;

                    RLYLIB_PUT_U32((CHAR*)&msgHdr->msgLen, msgLen);
                    RLYLIB_PUT_U32((CHAR*)&msgHdr->msgCode, RLYLIB_MSG_CODE_DATA);

                    HOST_MSG_SEND_TO_SOCK(connCb, sndBuf, sndMsgLen, ret);
                    if(ret != RC_OK){
                        RLYLIB_DEC_OPEN_CONN_CNT(hostCb);
                        *rt_procCnt = 0;
                        RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, msgChk, ret);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
                        }

                        return RC_NOK;
                    }

                    /* free allocated message */
                    if(msgChk->msgHdrIncFlg != RC_TRUE){
                        ret = rlylibInt_msgFreeMsgBuf(&hostCb->msgBufInfo, sndBuf, bufSize);
                        if(ret != RC_OK){
                            RLY_LOG(RLY_ERR,"Message  buffer free failed(ret=%d)\n",ret);
                        }
                    }/* end of if(msgChk->msgHdrIncFlg != RC_TRUE) */

                    RLYLIB_FREE_MSG_CHK(&hostCb->freeMsgChkQ, &hostCb->msgBufInfo, msgChk, ret);
                    if(ret != RC_OK){
                        RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
                    }

                    break; /* message send end */
                }/* end of if(sndMsgLen <= remTmpBufLen) */
            }/* end of if(sndMsgLen < remTmpBufLen) */
        }/* end of while(1) */
    }/* end of for(i=0;i<RLYLIB_SND_LOOP_CNT;i++) */ 

    /* send all message */
    if(hostCb->tmpMsgBufLen != 0){
        /* send all message */
        HOST_MSG_SEND_TO_SOCK(connCb, hostCb->tmpMsgBuf, hostCb->tmpMsgBufLen, ret);
        if(ret != RC_OK){
            RLYLIB_DEC_OPEN_CONN_CNT(hostCb);
            *rt_procCnt = 0;
            hostCb->tmpMsgBufLen = 0;
            return RC_NOK;
        }

        hostCb->tmpMsgBufLen = 0;
    }/* end of if(connCb->tmpMsgBufLen != 0) */

    curTick = rlylibInt_globGetCurTick();

    /* reset timer */
    if(procCnt != 0){
        RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.hBeatSndTmOut, 
                                      RLYLIB_CONN_TMR_EVNT_HBEAT);
    }

    *rt_procCnt = procCnt;

    return RC_OK;
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostSndCtrlMsg(RlylibIntHostCb *hostCb, UINT ctrlCode)
{
    SINT ret = RC_OK;
    RlylibIntMsgHdr *msgHdr = NULL;

    msgHdr = comlib_memMalloc(sizeof(RlylibIntMsgHdr));

    msgHdr->msgLen = sizeof(RlylibIntMsgHdr);
    msgHdr->msgCode = ctrlCode;

    ret = thrlib_tqPush(hostCb->ctrlTq, msgHdr);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread queue push failed(ret=%d, host=%s, ctrlCode=%d)\n",ret, hostCb->host, ctrlCode);
        comlib_memFree(msgHdr);
        return RLYERR_MSG_PUSH_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostDelHost(RlylibIntMainCb *intMainCb, CHAR *host)
{
    SINT ret = RC_OK;
    UINT hostLen = 0;
    RlylibIntHostCb *hostCb = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    hostLen = comlib_strGetLen(host);

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    /* find host */
    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Hostport control block not exist(%s)\n",host);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

        return RLYERR_HOST_CB_NOT_EXIST;
    }

    ret = rlylibInt_hostSndCtrlMsg(hostCb, RLYLIB_CTRL_MSG_CODE_DSTRY_HOST);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Control message send failed(ret=%d)\n",ret);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

        return ret;
    }

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostMakeHostKey(CHAR *locHost, UINT locHostLen, U_32 id, CHAR *rt_hostKey, 
		UINT maxHostKeyLen, UINT *rt_hostKeyLen)
{
    struct timeval tv;
    UINT hostKeyLen = 0;

    hostKeyLen = locHostLen +  sizeof(U_32) + sizeof(struct timeval);

    if(maxHostKeyLen < (locHostLen +  sizeof(U_32) + sizeof(struct timeval))){
        RLY_LOG(RLY_ERR,"Host key buffer is too small(%d)\n", maxHostKeyLen);
        return RLYERR_MSG_BUF_TOO_SMALL;
    }

    gettimeofday(&tv, NULL);

    comlib_memMemcpy(rt_hostKey, locHost, locHostLen);
    comlib_memMemcpy(&rt_hostKey[locHostLen], (CHAR*)&id, sizeof(U_32));
    comlib_memMemcpy(&rt_hostKey[locHostLen + sizeof(U_32)], (CHAR*)&tv, sizeof(struct timeval));

    *rt_hostKeyLen = hostKeyLen;

    return RC_OK;
}

FT_PRIVATE RT_RESULT host_chkCtrlMsg(RlylibIntHostCb *hostCb)
{
    SINT ret = RC_OK;
    RlylibIntMsgHdr *msgHdr = NULL;
    CHAR *ctrlMsg = NULL;

    /* check control message */
    ret = thrlib_tqPop(hostCb->ctrlTq, (VOID*)&ctrlMsg);
    if((ret != RC_OK) && (ret != THRERR_TQ_EMPTY)){
        RLY_LOG(RLY_ERR,"Control message pop failed(ret=%d)\n",ret);
        goto goto_freeHostCb;
    }
    else if(ret == THRERR_TQ_EMPTY){
        return RC_OK;
    }

    /* check control message */
    msgHdr = (RlylibIntMsgHdr*)ctrlMsg;

    if(msgHdr->msgCode == RLYLIB_CTRL_MSG_CODE_DSTRY_HOST){
        comlib_memFree(ctrlMsg);

        goto goto_freeHostCb;
    }
    else {
        RLY_LOG(RLY_ERR,"Invalid message code (code=%d)\n",msgHdr->msgCode);
        comlib_memFree(ctrlMsg);
        return RC_OK;
    }

goto_freeHostCb:
    /* freeze now */
    hostCb->dstryFlg = RC_TRUE;

    return RC_OK;
}

FT_PUBLIC VOID rlylibInt_hostThrdMain(VOID *args)
{
    SINT ret = RC_OK;
    UINT sleepUs = 1;
    UINT sleepCnt = 0;
    BOOL sleepFlg = RC_FALSE;
    UINT procCnt = 0;
    UINT loopCnt = 0;
    ULONG lastRcvTick = 0;
    ULONG curTick = 0;
    UINT sockChkLoop = 0;
    RlylibIntMainCb *intMainCb = NULL;
    RlylibIntHostMainCb *hostMainCb = NULL;
    RlylibIntHostThrdCb *hostThrdCb = NULL;
    RlylibIntHostCb *hostCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    hostThrdCb = args;

    hostThrdCb->procCnt = 0;

    hostMainCb = hostThrdCb->main;
    intMainCb = hostMainCb->main;

    while(1){
        if(hostThrdCb->tmrUpdFlg == RC_TRUE){
            rlylibInt_globUpdCurTick();
        }

        /* get current time */
        curTick = rlylibInt_globGetCurTick();

        thrlib_mutxLock(&hostMainCb->mutx);

        if(sleepFlg == RC_TRUE){
            if((sleepUs == RLYLIB_MAX_USLEEP_US) && 
               ((curTick - lastRcvTick) > 3 /* 0.3sec */) &&
               (hostMainCb->sleepCnt == hostMainCb->hostCnt)){
                hostMainCb->sleepCnt++;
                rlylibInt_syncWaitSigCond(&hostMainCb->cond, &hostMainCb->mutx, sleepUs);
                hostMainCb->sleepCnt--;
            }
            else {
                hostMainCb->sleepCnt++;
                thrlib_mutxUnlock(&hostMainCb->mutx);
                usleep(sleepUs);
                thrlib_mutxLock(&hostMainCb->mutx);
                hostMainCb->sleepCnt--;
            }
            sleepFlg = RC_FALSE;
        }/* end of if(sleepFlg == RC_TRUE) */

        if(hostMainCb->termFlg == RC_TRUE){
            hostMainCb->actThrdCnt--;
            thrlib_mutxUnlock(&hostMainCb->mutx);
            thrlib_thrdExit(0);
        }

        lnkNode = comlib_lnkLstGetFirst(&hostMainCb->hostLnkLst);
        if(lnkNode == NULL){
            thrlib_mutxUnlock(&hostMainCb->mutx);
            usleep(3000);
            continue;
        }

        hostCb = lnkNode->data;

        thrlib_mutxUnlock(&hostMainCb->mutx);

        if(hostThrdCb->tmrUpdFlg == RC_TRUE){
            rlylibInt_globUpdCurTick();
        }

        /* check freeze time */
        if((hostCb->freeTmFlg == RC_TRUE) || 
           (hostCb->dstryFlg == RC_TRUE)){
            if(((curTick - hostCb->freeTm) >= hostCb->tmrInfo.hostFreeTmOut) ||
               (hostCb->dstryFlg == RC_TRUE)){
                /* free all */
                rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

                RLY_LOG(RLY_ERR,"Destroy host (host=%s, curTick=%d, freeTm=%d, hostFreeTmOut=%d, dstryFlg=%d)\n",
                        hostCb->host, curTick, hostCb->freeTm, hostCb->tmrInfo.hostFreeTmOut, hostCb->dstryFlg);

                rlylibInt_hostDstryHostCb(intMainCb, hostCb);

                rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

                continue;
            }
        }/* end of if(hostCb->freeTmFlg == RC_TRUE) */

        /* process host control block */
        thrlib_mutxLock(&hostCb->mutx);

        ret = host_chkCtrlMsg(hostCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Control message check failed(ret=%d, host=%s)\n",ret, hostCb->host);
        }

        /* check send queue */
        sockChkLoop = 0;
        while(1){
            ret = host_chkSndMsg(hostCb, &procCnt);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Send message check failed(ret=%d)\n",ret);
                break;
            }

            if((procCnt == 0) || (sockChkLoop == HOST_SOCK_CHK_LOOP)){
                break;
            }

            sockChkLoop++;
        }

        hostThrdCb->procCnt += sockChkLoop;

        /* check receive queue */
        sockChkLoop = 0;
        while(1){
            ret = host_chkAllConn(hostCb, &procCnt);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Receive message check failed(ret=%d)\n",ret);
                break;
            }

            if((procCnt == 0) || (sockChkLoop == HOST_SOCK_CHK_LOOP)){
                break;
            }

            sockChkLoop++;
        }

        hostThrdCb->procCnt += sockChkLoop;

        thrlib_mutxUnlock(&hostCb->mutx);
        /* process end */

        if(hostThrdCb->tmrUpdFlg == RC_TRUE){
            rlylibInt_globUpdCurTick();
        }

        /* send to idle */
        thrlib_mutxLock(&hostMainCb->mutx);

        ret = comlib_lnkLstInsertTail(&hostMainCb->hostLnkLst, &hostCb->thrdLoopLnkNode);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"hostport loop linked list insert failed(ret=%d)\n",ret);
        }

        if(hostThrdCb->procCnt != 0){
            lastRcvTick = curTick;
        }

        if((hostMainCb->hostCnt == loopCnt) &&
           hostThrdCb->procCnt <= (HOST_SOCK_CHK_LOOP)){
            sleepFlg = RC_TRUE;

            hostThrdCb->procCnt = 0;
            loopCnt = 0;
        }
        else if(hostMainCb->hostCnt == loopCnt){
            hostThrdCb->procCnt = 0;
            loopCnt = 0;
        }

        thrlib_mutxUnlock(&hostMainCb->mutx);

        if(sleepFlg == RC_TRUE){
            /* calc sleep us */
            sleepUs += RLYLIB_INC_USLEEP_US;

            if(sleepUs >= RLYLIB_MAX_USLEEP_US){
                sleepUs = RLYLIB_MAX_USLEEP_US;
            }
            sleepCnt++;
        }
        else {
            sleepUs = RLYLIB_MIN_USLEEP_US;
            sleepCnt = 0;
        }

        loopCnt++;
    }/* end of while(1) */

    return;
}

FT_PUBLIC RlylibIntHostCb* rlylibInt_hostFindHostCbByHost(RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen)
{
    SINT ret = RC_OK;
    ComlibHashKey hKey;
    ComlibHashNode *hNode;

    hKey.key = host;
    hKey.keyLen = hostLen;

    ret = comlib_hashTblFindHashNode(&intMainCb->hostHt, &hKey, 0, &hNode);
    if(ret == RC_OK){
        return hNode->data;
    }
    else if(ret != RC_NOK){
        RLY_LOG(RLY_ERR,"Hash find failed(ret=%d)\n",ret);
    }

    return NULL;
}

/* thread not safe */
FT_PRIVATE RT_RESULT host_freeConnCb(RlylibIntConnCb *connCb)
{
    SINT ret = RC_OK;

    RLY_LOG(RLY_NOTY,"Free Connection(fd=%d, status=%d)\n", connCb->fd, connCb->status);

    ret = host_closeConnCb(connCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"connection control blcok closed failed(ret=%d)\n",ret);
    }

    comlib_memFree(connCb);

    return RC_OK;
}

FT_PRIVATE RT_RESULT host_freeAllConnCb(RlylibIntHostCb *hostCb)
{
    SINT ret = RC_OK;
    RlylibIntConnCb *connCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    lnkNode =  comlib_lnkLstGetFirst(&hostCb->connLst);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        connCb = lnkNode->data;

        ret = host_freeConnCb(connCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Connection free failed(ret=%d)\n",ret);
        }

        lnkNode =  comlib_lnkLstGetFirst(&hostCb->connLst);
        if(lnkNode == NULL){
            return RC_OK;
        }
    }/* end of while(1) */

    return RC_OK;
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostAcptHostCb(RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen, CHAR *hostKey, UINT hostKeyLen,
		UINT *rt_hostId)
{
    SINT ret = RC_OK;
    RlylibIntHostCb *hostCb = NULL;

    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        /* make new hostCb */
        ret = rlylibInt_hostMakeHostCb(intMainCb, host, hostLen, hostKey, hostKeyLen, 
                RLYLIB_TYPE_SERVER, &intMainCb->dfltHostOptArg, rt_hostId);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"hostCb make failed(ret=%d)\n",ret);
            return ret;
        }
    }
    else {
        /* mutex */
        thrlib_mutxLock(&hostCb->mutx);

        *rt_hostId = hostCb->spId;

        /* check host Key */
        ret = comlib_memMemcmp(hostCb->hostKey, hostKey, hostKeyLen);
        if(ret == 0){
            /* hostCb exsit */
            thrlib_mutxUnlock(&hostCb->mutx);
            return RC_OK;
        }

        /* close prev connection */
        RLY_LOG(RLY_NOTY,"Clear all connection(host=%.*s)\n", hostLen, host);
        ret = host_freeAllConnCb(hostCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"All connection free failed(ret=%d)\n",ret);
        }

        /* change host key */
        comlib_memMemcpy(hostCb->hostKey, hostKey, hostKeyLen);
        hostCb->hostKeyLen = hostKeyLen;

        thrlib_mutxUnlock(&hostCb->mutx);
    }

    return RC_OK;
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostMakeConnCb(RlylibIntHostCb *hostCb, BOOL fdFlg, UINT fd, TrnlibTransAddr *dstAddr)
{
    SINT ret = RC_OK;
    ULONG curTick = 0;
    RlylibIntConnCb *connCb = NULL;

    connCb = comlib_memMalloc(sizeof(RlylibIntConnCb));
    if(connCb == NULL){
        RLY_LOG(RLY_ERR,"Conneciton control block allocation failed\n");
        return RLYERR_CONN_CB_ALLOC_FAILED;
    }

    connCb->lnkNode.data = connCb;
    connCb->lnkNode.prev = NULL;
    connCb->lnkNode.next = NULL;

    connCb->tmrHndlNode.used = RC_FALSE;
    connCb->tmrHndlNode.tmOutTick = 0;

    connCb->lstUpdTm = 0;
    connCb->remDataLen = 0;
    connCb->tmpBufLen = 0;
    connCb->msgChk = NULL;

    if(fdFlg == RC_TRUE){
        connCb->fd = fd;
        connCb->status = RLYLIB_CONN_STA_OPEN;

        RLYLIB_INC_OPEN_CONN_CNT(hostCb);

        RLY_LOG(RLY_DBG,"Connection open(fd=%d)\n",fd);

        curTick = rlylibInt_globGetCurTick();

        connCb->lstUpdTm = curTick;

        RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, hostCb->tmrInfo.hBeatSndTmOut, RLYLIB_CONN_TMR_EVNT_HBEAT);
    }
    else {
        connCb->fd = 0;
        connCb->status = RLYLIB_CONN_STA_CLOSED;

        if(hostCb->type == RLYLIB_TYPE_SERVER){
            connCb->tmrHndlNode.evnt = RLYLIB_CONN_TMR_EVNT_NONE;
        }
        else{
            curTick = rlylibInt_globGetCurTick();
            RLYLIB_SET_CONN_TMR_HNDL_NODE(&connCb->tmrHndlNode, curTick, 0, RLYLIB_CONN_TMR_EVNT_CONN);
        }
    }

    comlib_memMemcpy(&connCb->dstAddr, dstAddr, sizeof(TrnlibTransAddr));

    ret = comlib_lnkLstInsertTail(&hostCb->connLst, &connCb->lnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Connection control block insert failed(%d)\n", ret);
        comlib_memFree(connCb);
        return RLYERR_CONN_CB_INSERT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostAddAcptConnCb(RlylibIntMainCb *intMainCb, UINT hostId, UINT fd, TrnlibTransAddr *dstAddr)
{
    SINT ret = RC_OK;
    RlylibIntHostCb *hostCb = NULL;

    hostCb = intMainCb->hostCb[hostId];
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"HostCb not exist(%d)\n",hostId);
        return RLYERR_HOST_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&hostCb->mutx);

    ret = rlylibInt_hostMakeConnCb(hostCb, RC_TRUE, fd, dstAddr);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"connection control block make failed(ret=%d\n",ret);
    }

    thrlib_mutxUnlock(&hostCb->mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostAddConnCb(RlylibIntMainCb *intMainCb, CHAR *host, TrnlibTransAddr *dstAddrs, UINT dstAddrCnt)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT hostLen = 0;
    RlylibIntHostCb *hostCb = NULL;

    if(dstAddrCnt == 0 ||
            dstAddrCnt > RLYLIB_DFLT_MAX_CONN_CNT){
        RLY_LOG(RLY_ERR,"Invalid destiation address(%d)\n",dstAddrCnt);
        return RLYERR_INVALID_CONN_CNT;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    hostLen = comlib_strGetLen(host);

    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Hostport control block not exist\n");
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
        return RLYERR_HOST_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&hostCb->mutx);

    for(i=0;i<dstAddrCnt;i++){
        ret = rlylibInt_hostMakeConnCb(hostCb, RC_FALSE, 0, &dstAddrs[i]);
        if(ret != RC_OK){
            thrlib_mutxUnlock(&hostCb->mutx);
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
            return ret;
        }
    }/* end of for(i=0;i<dstAddrCnt;i++) */

    thrlib_mutxUnlock(&hostCb->mutx);
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;
}


FT_PUBLIC RT_RESULT rlylibInt_hostRcvMsgFromAny(RlylibIntMainCb *intMainCb, CHAR *rt_host, UINT *rt_hostId, 
		BOOL dynFlg, RlylibIntRcvMsgRslt *rt_rcvMsgRslt) 
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT lastRcvHostId = 0;
    U_32 allocBufLen = 0;
    UINT rcvBufLen = 0;
    UINT hostCnt = 0;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntHostCb *hostCb = NULL;
    CHAR *rcvBuf = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    hostCnt = intMainCb->hostMainCb.maxHostCnt;

    RLYLIB_COND_LOOP_START(&intMainCb->mainMutx.rdMutx, &intMainCb->rdCond,  intMainCb->condWaitFlg, 5, intMainCb->waitTm);
    {/* COND_LOOP_FUNCTION START */
        lastRcvHostId = intMainCb->lastRcvHostId;
        if(lastRcvHostId > intMainCb->hostMainCb.maxHostCnt){
            lastRcvHostId = 0;
        }

        hostCb =  intMainCb->hostCb[lastRcvHostId];
        if(hostCb == NULL){
            /* find any */
            COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
            if(lnkNode == NULL){
                rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

                return RLYERR_MSG_NOT_EXIST;
            }

            hostCb = lnkNode->data;
        }/* end of if(hostCb == NULL) */
        else {
            lnkNode = &hostCb->mainLnkNode;
        }

        for(i=0;i< hostCnt;i++){
            ret = rlylibInt_hostRcvDynMsg(intMainCb, hostCb, &rcvBuf, &rcvBufLen, &allocBufLen);
            if((ret != RC_OK) && (ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
                RLY_LOG(RLY_ERR,"Rcv fix message error(ret=%d)\n",ret);
                goto goto_getNxtHost;
            }
            else if(ret == RLYERR_MSG_NOT_EXIST){
                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
                }
                hostCb = lnkNode->data;

                continue;
            }
            else if((ret == RC_OK) || (ret == RLYERR_DROP_MSG)){
                if(rt_host != NULL){
                    comlib_strCpy(rt_host, hostCb->host);
                }

                if(rt_hostId != NULL){
                    *rt_hostId = hostCb->spId;
                }

                RLYLIB_COND_LOOP_BREAK(RC_OK);
            }
        }/* end of for(i=0;i< hostCnt;i++) */
    }/* COND_LOOP_FUNCTION END */
    RLYLIB_COND_LOOP_END(ret);
    if(ret == RLYERR_COND_TM_OUT){
        ret = RLYERR_MSG_NOT_EXIST;
    }

goto_getNxtHost:
    COM_GET_NEXT_NODE(lnkNode);
    if(lnkNode == NULL){
        COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
        if(lnkNode == NULL){
            RLY_LOG(RLY_ERR,"Host control block not exist\n");
            intMainCb->lastRcvHostId = 0;
        }/* end of if(intMainCb->condWaitFlg == RC_TRUE) */
        else {
            hostCb = lnkNode->data;
            intMainCb->lastRcvHostId = hostCb->spId;

        }
    }
    else {
        hostCb = lnkNode->data;
        intMainCb->lastRcvHostId = hostCb->spId;
    }

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    if(ret == RC_OK){
        HOST_MAKE_RCV_MSG_RSLT(intMainCb, dynFlg, rcvBuf, rcvBufLen, allocBufLen, rt_rcvMsgRslt, ret);
    } /* end of if(ret == RC_OK) */

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_hostRcvMsgFromHost(RlylibIntMainCb *intMainCb, CHAR *host, BOOL dynFlg, 
		RlylibIntRcvMsgRslt *rt_rcvMsgRslt)
{
    SINT ret = RC_OK;
    UINT hostLen = 0;
    UINT rcvBufLen = 0;
    UINT allocBufLen = 0;
    CHAR *rcvBuf = NULL;
    RlylibIntHostCb *hostCb = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    hostLen = comlib_strGetLen(host);
    if(hostLen == 0){
        RLY_LOG(RLY_ERR,"Invalid host lenth(zero)\n");
        return RLYERR_INVALID_HOST_LEN;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    RLYLIB_COND_LOOP_START(&intMainCb->mainMutx.rdMutx, &intMainCb->rdCond,  intMainCb->condWaitFlg, 5, intMainCb->waitTm);
    {/* COND_LOOP_FUNCTION START */
        /* find host */
        hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
        if(hostCb == NULL){
            RLY_LOG(RLY_ERR,"Host control block not exist(host=%.*s)\n", hostLen, host);
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

            return RLYERR_HOST_CB_NOT_EXIST;
        }

        ret = rlylibInt_hostRcvDynMsg(intMainCb, hostCb, &rcvBuf, &rcvBufLen, &allocBufLen);
        if((ret != RC_OK) && (ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
            RLY_LOG(RLY_ERR,"Rcv fix message error(ret=%d)\n",ret);
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

            return ret;
        }
        else if((ret == RC_OK) || (ret == RLYERR_DROP_MSG)){
            RLYLIB_COND_LOOP_BREAK(ret);
        }
    }/* COND_LOOP_FUNCTION END */
    RLYLIB_COND_LOOP_END(ret);

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    if(ret == RLYERR_COND_TM_OUT){
        return RLYERR_MSG_NOT_EXIST;
    }

    /* message exist */
    HOST_MAKE_RCV_MSG_RSLT(intMainCb, dynFlg, rcvBuf, rcvBufLen, allocBufLen, rt_rcvMsgRslt, ret);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_hostSndFixMsgToPri(RlylibIntMainCb *intMainCb, CHAR *buf, UINT bufLen, BOOL faFlg /* failover flag */, CHAR *rt_host, UINT *rt_hostId)
{
    SINT ret = RC_OK;
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

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    /* find host */
    hostCb = intMainCb->hostCb[intMainCb->priHostId];
    if(hostCb == NULL){
        ComlibLnkNode *lnkNode = NULL;

        COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
        if(lnkNode == NULL){
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

            RLY_LOG(RLY_ERR,"HostCb not exist\n");
            RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
            if(ret != RC_OK){
                RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
            }
            return RLYERR_HOST_CB_NOT_EXIST;
        }

        hostCb = lnkNode->data;
    }/* end of if(hostCb == NULL) */

    if(faFlg == RC_TRUE){
        HOST_SND_MSG_TO_ANY_HOST(intMainCb, hostCb, msgChk, ret);
    }
    else{
        ret = thrlib_tqPush(hostCb->sndTq, msgChk);
    }

    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message sned failed(ret=%d, faFlg=%d)\n",ret, faFlg);
    }
    else {
        if(rt_host != NULL){
            comlib_strCpy(rt_host, hostCb->host);
        }

        if(rt_hostId != NULL){
            *(rt_hostId) = hostCb->spId;
        }
    }

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_hostSndFixMsgToAny(RlylibIntMainCb *intMainCb, CHAR *buf, UINT bufLen, CHAR *rt_host, UINT *rt_hostId)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
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

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    /* find host */
    hostCb = intMainCb->hostCb[intMainCb->lastSndHostId];
    if(hostCb == NULL){

        COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
        if(lnkNode == NULL){
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

            RLY_LOG(RLY_ERR,"HostCb not exist\n");
            intMainCb->lastSndHostId = 0;
            return RLYERR_HOST_CB_NOT_EXIST;
        }

        hostCb = lnkNode->data;
    }/* end of if(hostCb == NULL) */

    HOST_SND_MSG_TO_ANY_HOST(intMainCb, hostCb, msgChk, ret);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"All thread queue push failed(ret=%d)\n",ret);
        ret = RLYERR_MSG_PUSH_FAILED;
    }
    else {
        if(rt_host != NULL){
            comlib_strCpy(rt_host, hostCb->host);
        }

        if(rt_hostId != NULL){
            *(rt_hostId) = hostCb->spId;
        }
        ret = RC_OK;
    }

    /* cond signal */
    thrlib_condSig(&intMainCb->hostMainCb.cond);

    /* get next host id */
    lnkNode = &hostCb->mainLnkNode;
    COM_GET_NEXT_NODE(lnkNode);
    if(lnkNode == NULL){
        COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
        if(lnkNode == NULL){
            intMainCb->lastSndHostId = 0;
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

            return ret;
        }
    }

    hostCb = lnkNode->data;

    intMainCb->lastSndHostId = hostCb->spId;

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    return ret;
}

FT_PUBLIC RT_RESULT rlylibInt_hostSndMsgToHostId(RlylibIntMainCb *intMainCb, UINT hostId, BOOL dynFlg, CHAR *buf, UINT bufLen)
{
    SINT ret = RC_OK;
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

    if(hostId >= intMainCb->hostMainCb.maxHostCnt){
        RLY_LOG(RLY_ERR,"hostId overrange (hostId=%d)\n",hostId);
        return RLYERR_HOST_ID_OVERRANGE;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    /* make message */
    if(dynFlg == RC_TRUE){
        RLYLIB_MAKE_SND_DYN_MSG_CHK(&intMainCb->freeMsgChkQ, buf, bufLen , msgChk, ret);
    }
    else{
        RLYLIB_MAKE_SND_FIX_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, buf, bufLen , msgChk, ret);
    }
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer make failed(ret=%d)\n",ret);\
            return ret;
    }

    /* find host */
    hostCb = intMainCb->hostCb[hostId];
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Hostport control block not exist(id=%d)\n",hostId);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return RLYERR_HOST_CB_NOT_EXIST;
    }

    ret = thrlib_tqPush(hostCb->sndTq, msgChk);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread queue push failed(ret=%d)\n",ret);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return RLYERR_MSG_PUSH_FAILED;
    }

    thrlib_condSig(&intMainCb->hostMainCb.cond);

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostSndMsgToHost(RlylibIntMainCb *intMainCb, CHAR *host, BOOL dynFlg, CHAR *buf, UINT bufLen)
{
    SINT ret = RC_OK;
    UINT hostLen = 0;
    RlylibIntMsgChk *msgChk = NULL;
    RlylibIntHostCb *hostCb = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay control block is NULL\n"),
                    RLYERR_NULL);

    /* check buffer length */
    if(bufLen == 0){
        RLY_LOG(RLY_ERR,"Invalid buffer len(zero)\n");
        return RLYERR_INVALID_BUF_LEN;
    }

    hostLen = comlib_strGetLen(host);
    if(hostLen == 0){
        RLY_LOG(RLY_ERR,"Invalid host lenth(zero)\n");
        return RLYERR_INVALID_HOST_LEN;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    /* send message */
    if(dynFlg == RC_TRUE){
        RLYLIB_MAKE_SND_DYN_MSG_CHK(&intMainCb->freeMsgChkQ, buf, bufLen , msgChk, ret);
    }
    else {
        RLYLIB_MAKE_SND_FIX_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, buf, bufLen , msgChk, ret);
    }
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer make failed(ret=%d)\n",ret);
        return ret;
    }

    /* find host */
    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Hostport control block not exist(%s)\n",host);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return RLYERR_HOST_CB_NOT_EXIST;
    }

    ret = thrlib_tqPush(hostCb->sndTq, msgChk);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread queue push failed(ret=%d)\n",ret);
        RLYLIB_FREE_MSG_CHK(&intMainCb->freeMsgChkQ, &intMainCb->msgBufInfo, msgChk, ret);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message chunk free failed(ret=%d)\n",ret);
        }
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

        return RLYERR_MSG_PUSH_FAILED;
    }

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_WR);

    return RC_OK;
}

/* thread unsafe  */
FT_PUBLIC RT_RESULT rlylibInt_hostMakeHostCb(RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen, 
                                             CHAR *hostKey, UINT hostKeyLen, UINT type, RlylibHostOptArg *hostOptArg, 
                                             UINT *rt_hostId)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT hostId = 0;
    RlylibIntHostCb *hostCb = NULL;

    /* create hostaction */
    for(i=0;i<intMainCb->hostMainCb.maxHostCnt;i++){
        if(intMainCb->hostCb[i] == NULL){
            break;
        }
    }/* end of for(i=0;i<intMainCb->maxHostCnt;i++) */

    if(i == intMainCb->hostMainCb.maxHostCnt){
        return RLYERR_HOST_IS_FULL;
    }
    else {
        hostId = i;
    }

    hostCb = comlib_memMalloc(sizeof(RlylibIntHostCb));
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Hostation alloc failed\n");
        comlib_memFree(hostCb);
        return RLYERR_HOST_CB_ALLOC_FAILED;
    }

    hostCb->spId = hostId;

    *rt_hostId = hostCb->spId;

    if(hostLen >= RLYLIB_MAX_HOST_LEN){
        RLY_LOG(RLY_ERR,"Host langth too long(len=%d)\n",hostLen);
        return RLYERR_INVALID_HOST_LEN;
    }

    //comlib_strCpy(hostCb->host, host);
    comlib_strNCpy(hostCb->host, host, hostLen);
    hostCb->host[hostLen] = '\0';

    comlib_strCpy(hostCb->locHost, intMainCb->locHost);

    hostCb->hKey.key = hostCb->host;
    hostCb->hKey.keyLen = hostLen;

    hostCb->openConnCnt = 0;

    hostCb->regRlmCnt = 0;

    hostCb->freeTmEnb = RC_FALSE;
    hostCb->freeTmFlg = RC_FALSE;
    hostCb->dstryFlg = RC_FALSE;
    hostCb->tmpMsgBufLen = 0;

    if(hostOptArg == NULL){
        hostCb->msgDropIndFlg = RC_FALSE;
        hostCb->freeTmEnb = RC_FALSE;
        hostCb->rlyMode = intMainCb->dfltHostOptArg.rlyMode;

        RLYLIB_SET_TMR_INFO(&hostCb->tmrInfo, &intMainCb->dfltHostOptArg.tmrOptArg);
    }
    else {
        hostCb->msgDropIndFlg = hostOptArg->msgDropIndFlg;

        if(hostOptArg->rlyMode != 0){
            hostCb->rlyMode = hostOptArg->rlyMode;
        }
        else {
            hostCb->rlyMode = intMainCb->dfltHostOptArg.rlyMode;
        }

        hostCb->freeTmEnb = hostOptArg->freeTmEnb;

        RLYLIB_SET_TMR_INFO(&hostCb->tmrInfo, &hostOptArg->tmrOptArg);
    }

    /* make hostKey */
    if(hostKeyLen == 0){
        ret = rlylibInt_hostMakeHostKey(hostCb->host, hostLen, hostCb->spId, 
                hostCb->hostKey, 
                RLYLIB_MAX_HOST_KEY_LEN, 
                &hostCb->hostKeyLen);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Host key make failed(ret=%d)\n",ret);
            comlib_memFree(hostCb);
            return RLYERR_HOST_KEY_MAKE_FAILED;
        }
    }
    else {
        comlib_memMemcpy(hostCb->hostKey, hostKey, hostKeyLen);
        hostCb->hostKeyLen = hostKeyLen;
    }

    hostCb->hNode.data = hostCb;
    hostCb->hNode.prev = NULL;
    hostCb->hNode.next = NULL;

    hostCb->mainLnkNode.data = hostCb;
    hostCb->mainLnkNode.prev = NULL;
    hostCb->mainLnkNode.next = NULL;

    hostCb->thrdLoopLnkNode.data = hostCb;
    hostCb->thrdLoopLnkNode.prev = NULL;
    hostCb->thrdLoopLnkNode.next = NULL;

    hostCb->type = type;

    hostCb->status = RLYLIB_HOST_STA_CLOSED;

    ret = thrlib_mutxInit(&hostCb->mutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"main mutex init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_MUTX_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&hostCb->connLst, ~0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"conn linked list init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_CONN_LNK_LST_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&hostCb->regRlmBktLst, ~0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"reg realm bucket list init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_RLM_LNK_LST_INIT_FAILED;
    }

    ret = rlylibInt_msgInitMsgBufInfo(&hostCb->msgBufInfo, RLYLIB_DFLT_HOST_MSG_BUF_CNT);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"buffer infor init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_BUF_INFO_INIT_FAILED;
    }

    hostCb->nxtConn = NULL;

    hostCb->rdCond = &intMainCb->rdCond;

    hostCb->tmpSndMsgChk = NULL;

    ret = thrlib_tqInit(&hostCb->freeMsgChkQ, THR_TQ_LOCK_TYPE_LOCK_FREE, RLYLIB_DFLT_FREE_CHK_Q_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread free message chunk queue init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_THRD_Q_INIT_FAILED;
    }

    hostCb->sndTq = comlib_memMalloc(sizeof(ThrlibTq));
    ret = thrlib_tqInit(hostCb->sndTq, THR_TQ_LOCK_TYPE_LOCK_FREE, RLYLIB_DFLT_MAX_TQ_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread send queue init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_THRD_Q_INIT_FAILED;
    }

    hostCb->rcvTq = comlib_memMalloc(sizeof(ThrlibTq));
    ret = thrlib_tqInit(hostCb->rcvTq, THR_TQ_LOCK_TYPE_LOCK_FREE, RLYLIB_DFLT_MAX_TQ_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread send queue init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_THRD_Q_INIT_FAILED;
    }

    hostCb->ctrlTq = comlib_memMalloc(sizeof(ThrlibTq));
    ret = thrlib_tqInit(hostCb->ctrlTq, THR_TQ_LOCK_TYPE_LOCKED, RLYLIB_DFLT_MAX_TQ_SIZE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Thread control queue init failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_THRD_Q_INIT_FAILED;
    }

    ret = comlib_lnkLstInsertTail(&intMainCb->hostLnkLst, &hostCb->mainLnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host linked list insert failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_LNK_LST_INSERT_FAEILD;
    }

    ret = comlib_lnkLstInsertTail(&intMainCb->hostMainCb.hostLnkLst, &hostCb->thrdLoopLnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host linked list insert failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_LNK_LST_INSERT_FAEILD;
    }

    ret = comlib_hashTblInsertHashNode(&intMainCb->hostHt, &hostCb->hKey, &hostCb->hNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host Hash table insert failed(ret=%d)\n",ret);
        comlib_memFree(hostCb);
        return RLYERR_HASH_TBL_INSERT_FAEILD;
    }

    intMainCb->hostCb[hostId] = hostCb;

    intMainCb->lastRcvHostId = hostCb->spId;
    intMainCb->lastSndHostId = hostCb->spId;

    /* set primary host */
    if((intMainCb->priHostId != hostCb->spId) &&
            (intMainCb->hostCb[intMainCb->priHostId] == NULL)){
        intMainCb->priHostId = hostCb->spId;
    }

    intMainCb->hostMainCb.hostCnt++;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostSetPriHostCb(RlylibIntMainCb *intMainCb, CHAR *host)
{
    RlylibIntHostCb *hostCb = NULL;
    UINT hostLen = 0;

    hostLen = comlib_strGetLen(host);
    if((hostLen >= RLYLIB_MAX_HOST_LEN) || 
       (hostLen == 0)){
        RLY_LOG(RLY_ERR,"Invalid hostLen(%d)\n",hostLen);
        return RLYERR_INVALID_HOST_LEN;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Host not exist\n");
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
        return RLYERR_HOST_CB_NOT_EXIST;
    }

    intMainCb->priHostId = hostCb->spId;

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;
}

/* thread unsafe */
FT_PUBLIC RT_RESULT rlylibInt_hostDstryHostCb(RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb)
{
    SINT ret = RC_OK;
    VOID *rcvMsg = NULL;
    RlylibIntMsgChk *msgChk = NULL;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntHostCb *nxtHostCb = NULL;
    RlylibIntHostMainCb *hostMainCb = NULL;

    hostMainCb = &intMainCb->hostMainCb;

    hostCb->status = RLYLIB_HOST_STA_CLOSED;

    /* delete host */
    ret = comlib_lnkLstDel(&intMainCb->hostLnkLst, &hostCb->mainLnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Main Host node delete failed(ret=%d)\n",ret);
    }

#if 0
    ret = comlib_hashTblDelHashNode(&intMainCb->hostHt, &hostCb->hKey, 0);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Hash node delete failed(ret=%d)\n",ret);
    }
#else
    ret = comlib_hashTblDelHashNode(&intMainCb->hostHt, &hostCb->hNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Hash node delete failed(ret=%d)\n",ret);
    }
#endif

    intMainCb->hostCb[hostCb->spId] = NULL;

    COM_GET_LNKLST_FIRST(&intMainCb->hostLnkLst, lnkNode);
    if(lnkNode != NULL){
        nxtHostCb = lnkNode->data;
    }

    if(intMainCb->lastRcvHostId == hostCb->spId){
        if(nxtHostCb == NULL){
            intMainCb->lastRcvHostId = 0;
        }
        else {
            intMainCb->lastRcvHostId = nxtHostCb->spId;
        }
    }

    if(intMainCb->lastSndHostId == hostCb->spId){
        if(nxtHostCb == NULL){
            intMainCb->lastSndHostId = 0;
        }
        else {
            intMainCb->lastSndHostId = nxtHostCb->spId;
        }
    }

    if(intMainCb->priHostId == hostCb->spId){
        if(nxtHostCb == NULL){
            intMainCb->priHostId = 0;
        }
        else {
            intMainCb->priHostId = nxtHostCb->spId;
        }
    }

    ret = rlylibInt_msgDstryMsgBufInfo(&hostCb->msgBufInfo);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer destroy failed(ret=%d)\n",ret);
        return RLYERR_BUF_INFO_DSTRY_FAILED;
    }

    /* free all */
    while(1){
        ret = thrlib_tqPop(&hostCb->freeMsgChkQ, (VOID*)&msgChk);
        if(ret != RC_OK){
            break;
        }

        if(msgChk->data != NULL){
            comlib_memFree(msgChk->data);
            msgChk->data = NULL;
        }

        comlib_memFree(msgChk);
    }/* end of while(1) */

    ret = thrlib_tqDstry(&hostCb->freeMsgChkQ);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Receive thread queue dstory failed(ret=%d)\n",ret);

    }

    while(1){
        ret = thrlib_tqPop(hostCb->rcvTq, (VOID*)&msgChk);
        if(ret != RC_OK){
            break;
        }

        if(msgChk->data != NULL){
            comlib_memFree(msgChk->data);
            msgChk->data = NULL;
        }

        comlib_memFree(msgChk);
    }/* end of while(1) */

    ret = thrlib_tqDstry(hostCb->rcvTq);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Receive thread queue dstory failed(ret=%d)\n",ret);

    }

    comlib_memFree(hostCb->rcvTq);

    while(1){
        ret = thrlib_tqPop(hostCb->sndTq, (VOID*)&msgChk);
        if(ret != RC_OK){
            break;
        }
        if(msgChk->data != NULL){
            comlib_memFree(msgChk->data);
            msgChk->data = NULL;
        }

        comlib_memFree(msgChk);

    }/* end of while(1) */

    ret = thrlib_tqDstry(hostCb->sndTq);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Send thread queue dstory failed(ret=%d)\n",ret);

    }

    comlib_memFree(hostCb->sndTq);

    while(1){
        ret = thrlib_tqPop(hostCb->ctrlTq, &rcvMsg);
        if(ret != RC_OK){
            break;
        }

        comlib_memFree(rcvMsg);
    }/* end of while(1) */


    ret = thrlib_tqDstry(hostCb->ctrlTq);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Control thread queue dstory failed(ret=%d)\n",ret);

    }

    comlib_memFree(hostCb->ctrlTq);


    /* destroy all connection */
    ret = host_freeAllConnCb(hostCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"All connection free failed(ret=%d)\n",ret);
    }

    if(hostCb->tmpSndMsgChk != NULL){
        if(hostCb->tmpSndMsgChk->data != NULL){
            comlib_memFree(hostCb->tmpSndMsgChk->data);
            comlib_memFree(hostCb->tmpSndMsgChk);
        }
    }

    hostCb->tmpSndMsgChk = NULL;

    thrlib_mutxDstry(&hostCb->mutx);

    comlib_memFree(hostCb);

    hostMainCb->hostCnt--;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostAddHost(RlylibIntMainCb *intMainCb, CHAR *host, UINT type, UINT hashId, RlylibHostOptArg *hostOptArg, UINT *rt_hostId)
{
    SINT ret = RC_OK;
    UINT hostLen = 0;
    RlylibIntHostCb *hostCb = NULL;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Relay library is null\n"),
                    RLYERR_NULL);

    if(type != RLYLIB_TYPE_CLIENT &&
       type != RLYLIB_TYPE_SERVER &&
       type != RLYLIB_TYPE_BOTH){
        RLY_LOG(RLY_ERR,"Invalid type (%d)\n",type);
        return RLYERR_INVALID_TYPE;
    }

    hostLen = comlib_strGetLen(host);
    if((hostLen >= RLYLIB_MAX_HOST_LEN) || 
       (hostLen == 0)){
        RLY_LOG(RLY_ERR,"Invalid hostLen(%d)\n",hostLen);
        return RLYERR_INVALID_HOST_LEN;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    thrlib_mutxLock(&intMainCb->hostMainCb.mutx);

    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb != NULL){
        RLY_LOG(RLY_ERR,"Host already exist\n");
        thrlib_mutxLock(&intMainCb->hostMainCb.mutx);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

        return RLYERR_HOST_ALREAY_EXIST;
    }

    ret = rlylibInt_hostMakeHostCb(intMainCb, host, hostLen, NULL, 0, type, hostOptArg, rt_hostId);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host control block make failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&intMainCb->hostMainCb.mutx);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

        return ret;
    }

    thrlib_mutxUnlock(&intMainCb->hostMainCb.mutx);
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostGetSta(RlylibIntMainCb *intMainCb, CHAR *host, UINT *rt_sta)
{
    RlylibIntHostCb *hostCb = NULL;
    UINT hostLen = 0;

    hostLen = comlib_strGetLen(host);
    if((hostLen >= RLYLIB_MAX_HOST_LEN) || (hostLen == 0)){
        RLY_LOG(RLY_ERR,"Invalid hostLen(%d)\n",hostLen);
        return RLYERR_INVALID_HOST_LEN;
    }

    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    hostCb = rlylibInt_hostFindHostCbByHost(intMainCb, host, hostLen);
    if(hostCb == NULL){
        RLY_LOG(RLY_ERR,"Host not exist\n");
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);
        return RLYERR_HOST_CB_NOT_EXIST;
    }

    (*rt_sta) = hostCb->status;

    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_RD);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_hostDstryAll(RlylibIntMainCb *intMainCb)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntHostCb *hostCb = NULL;
    RlylibIntHostMainCb *hostMainCb = NULL;

    hostMainCb = &intMainCb->hostMainCb;

    thrlib_mutxLock(&hostMainCb->mutx);

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&hostMainCb->hostLnkLst);
        if(lnkNode == NULL){
            break;
        }

        hostCb = lnkNode->data;

        ret = rlylibInt_hostDstryHostCb(intMainCb, hostCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Host control block destory failed(ret=%d)\n");
        }
    }/* end of while(1) */

    thrlib_mutxUnlock(&hostMainCb->mutx);

    thrlib_mutxDstry(&hostMainCb->mutx);
    thrlib_condDstry(&hostMainCb->cond);

    comlib_memFree(intMainCb->hostCb);

    return RC_OK;
}
