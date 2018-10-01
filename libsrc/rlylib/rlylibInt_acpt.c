#include <unistd.h>
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

FT_PRIVATE RlylibIntAcptCb*    acpt_findAcptCb             (RlylibIntAcptThrdCb *acptThrdCb, TrnlibTransAddr *srcAddr);
FT_PRIVATE RT_RESULT           acpt_chkTmpConn             (RlylibIntTmpConnCb *tmpConnCb, RlylibIntMsgInitReq *initReq);
FT_PRIVATE RT_RESULT           acpt_dstryAcptCb            (RlylibIntAcptThrdCb *acptThrdCb, RlylibIntAcptCb *acptCb);
FT_PRIVATE VOID                acpt_clnAcptThrd            (VOID *args);

FT_PRIVATE RlylibIntAcptCb* acpt_findAcptCb(RlylibIntAcptThrdCb *acptThrdCb, TrnlibTransAddr *srcAddr)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntAcptCb *acptCb = NULL;

    COM_GET_LNKLST_FIRST(&acptThrdCb->acptLnkLst, lnkNode);
    if(lnkNode == NULL){
        return NULL;
    }

    while(1){
        acptCb = lnkNode->data;

        ret = comlib_memMemcmp(&acptCb->srcAddr, srcAddr, sizeof(TrnlibTransAddr));
        if(ret == 0){
            return acptCb;
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            return NULL;
        }
    }/* end of while(1) */
}

FT_PRIVATE RT_RESULT acpt_dstryAcptCb(RlylibIntAcptThrdCb *acptThrdCb, RlylibIntAcptCb *acptCb)
{
    trnlib_sockDelFdSet(acptThrdCb->rdSockFdSet, acptCb->fd);

    trnlib_sockClose(acptCb->fd);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_acptDstry(RlylibIntAcptThrdCb *acptThrdCb)
{
    return RC_OK;
}

FT_PRIVATE VOID acpt_clnAcptThrd(VOID *args)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
    RlylibIntAcptCb *acptCb = NULL;
    RlylibIntTmpConnCb *tmpConnCb = NULL;
    RlylibIntAcptThrdCb *acptThrdCb = NULL;

    acptThrdCb = args;

    thrlib_mutxDstry(&acptThrdCb->mutx);

    /* clear connection */
    while(1){
        lnkNode = comlib_lnkLstGetFirst(&acptThrdCb->actConnLnkLst);
        if(lnkNode == NULL){
            break;
        }

        tmpConnCb = lnkNode->data;

        trnlib_sockClose(tmpConnCb->fd);

        comlib_memFree(tmpConnCb);
    }/* end of while(1) */

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&acptThrdCb->freeConnLnkLst);
        if(lnkNode == NULL){
            break;
        }

        tmpConnCb = lnkNode->data;

        comlib_memFree(tmpConnCb);
    }/* end of while(1) */

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&acptThrdCb->acptLnkLst);
        if(lnkNode == NULL){
            break;
        }

        acptCb = lnkNode->data;

        ret = acpt_dstryAcptCb(acptThrdCb, acptCb);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Accept control block destory failed(ret=%d)\n",ret);
        }

        comlib_memFree(acptCb);
    }/* end of while(1) */

    acptThrdCb->termFlg = RC_TRUE;

    return;
}

FT_PRIVATE RT_RESULT acpt_chkTmpConn(RlylibIntTmpConnCb *tmpConnCb, RlylibIntMsgInitReq *initReq)
{
    SINT ret = RC_OK;
    UINT rcvBufLen = 0;
    RlylibIntMsgHdr *msgHdr = NULL;

    /* check header */
    if(tmpConnCb->hdrBufLen != RLYLIB_MSG_HDR_LEN){
        if(tmpConnCb->hdrBufLen == 0){
            ret = trnlib_sockRead(tmpConnCb->fd, RLYLIB_MSG_HDR_LEN, tmpConnCb->hdrBuf, &rcvBufLen);
        }
        else {
            ret = trnlib_sockRead(tmpConnCb->fd, RLYLIB_MSG_HDR_LEN - tmpConnCb->hdrBufLen, 
                    &tmpConnCb->hdrBuf[tmpConnCb->hdrBufLen], &rcvBufLen);
        }
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Socket read failed(ret=%d)\n",ret);
            return RLYERR_SOCK_READ_FAILED;
        }/* end of if(ret != RC_OK) */

        tmpConnCb->hdrBufLen += rcvBufLen;

        if(tmpConnCb->hdrBufLen < RLYLIB_MSG_HDR_LEN){
            return RLYERR_READ_AGAIN;
        }

        /* check msg Hdr */
        msgHdr = (RlylibIntMsgHdr*)tmpConnCb->hdrBuf;

        RLYLIB_GET_4BYTE(msgHdr->msgLen, (CHAR*)&msgHdr->msgLen); /* byte ordering */
        RLYLIB_GET_4BYTE(msgHdr->msgCode, (CHAR*)&msgHdr->msgCode); /* byte ordering */

        if(msgHdr->msgCode != RLYLIB_MSG_CODE_INIT_REQ){
            RLY_LOG(RLY_ERR,"Unexpect message code(%d)\n",msgHdr->msgCode);
            return RLYERR_UNEXPECT_MSG_CODE;
        }

        if(msgHdr->msgLen > RLYLIB_TMP_MSG_BUF_LEN){
            RLY_LOG(RLY_ERR,"Invalid message buffer lenth(%d)\n",msgHdr->msgLen);
            return RLYERR_INVALID_BUF_LEN;
        }

        tmpConnCb->remBufLen = msgHdr->msgLen;
    }/* if(tmpConnCb->hdrBufLen != RLYLIB_MSG_HDR_LEN) */

    /* receive message */
    ret = trnlib_sockRead(tmpConnCb->fd, tmpConnCb->remBufLen, &tmpConnCb->msgBuf[tmpConnCb->msgBufLen], &rcvBufLen);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Socket read failed(ret=%d)\n",ret);
        return RLYERR_SOCK_READ_FAILED;
    }

    tmpConnCb->msgBufLen += rcvBufLen;
    tmpConnCb->remBufLen -= rcvBufLen;

    if(tmpConnCb->remBufLen == 0){ /* receive all message */
        /* parse init avp */
        ret = rlylibInt_msgDecInitReq(tmpConnCb->msgBuf, tmpConnCb->msgBufLen, initReq);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Message decode failed(ret=%d)\n",ret);
            return RLYERR_MSG_PARSE_FAILED;
        }
    }/* end of if(tmpConnCb->remBufLen == 0) */
    else {
        return RLYERR_READ_AGAIN;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT acpt_addTmpConn(RlylibIntAcptThrdCb *acptThrdCb, UINT acptFd, TrnlibTransAddr *dstAddr)
{
    SINT ret = RC_OK;
    RlylibIntTmpConnCb *tmpConnCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    /* check free conn list */
    lnkNode = comlib_lnkLstGetFirst(&acptThrdCb->freeConnLnkLst);
    if(lnkNode == NULL){
        tmpConnCb = comlib_memMalloc(sizeof(RlylibIntTmpConnCb));
        tmpConnCb->lnkNode.data = tmpConnCb;
        tmpConnCb->lnkNode.prev = NULL;
        tmpConnCb->lnkNode.next = NULL;
    }
    else {
        tmpConnCb = lnkNode->data;
    }

    /* set new connection */
    tmpConnCb->fd = acptFd;

    comlib_memMemcpy(&tmpConnCb->dstAddr, dstAddr, sizeof(TrnlibTransAddr));

    tmpConnCb->hdrBufLen = 0;
    tmpConnCb->msgBufLen = 0;

    ret = comlib_lnkLstInsertTail(&acptThrdCb->actConnLnkLst, &tmpConnCb->lnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Act connection linked list insert failed(ret=%d)\n",ret);
        ret = comlib_lnkLstInsertTail(&acptThrdCb->freeConnLnkLst, &tmpConnCb->lnkNode);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"free connection linked list insert failed(ret=%d)\n",ret);
        }
        trnlib_sockClose(tmpConnCb->fd);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC VOID rlylibInt_acptMainThrd(VOID *args)
{
    SINT ret = RC_OK;
    UINT acptFd = 0;
    RlylibIntMainCb *intMainCb = NULL;
    RlylibIntAcptThrdCb *acptThrdCb = NULL;
    RlylibIntTmpConnCb *tmpConnCb = NULL;
    RlylibIntAcptCb *acptCb = NULL;
    TrnlibTransAddr dstAddr;
    RlylibIntMsgInitReq initReq;
    RlylibIntMsgInitAck initAck;
    ComlibLnkNode *lnkNode = NULL;
    UINT tmpMsgBufLen;
    CHAR tmpMsgBuf[RLYLIB_TMP_MSG_BUF_LEN];

    intMainCb = (RlylibIntMainCb*)args;
    acptThrdCb = &intMainCb->acptThrdCb;

    /* detach thread */
    thrlib_thrdDtch(acptThrdCb->acptTid);

    /* pthread only */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

    thrlib_mutxLock(&acptThrdCb->mutx);

    /* initialization */
    acptThrdCb->tmpConnCnt = 0;

    thrlib_mutxUnlock(&acptThrdCb->mutx);

    /* cancel enable */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    thrlib_thrdSetCnclType(THR_THRD_CNCL_DEFERRED, NULL);

    THRLIB_THRDCLNUP_PUSH(acpt_clnAcptThrd, acptThrdCb);

    while(1){
        /* update current tick */
        rlylibInt_globUpdCurTick();

        if(acptThrdCb->termSig == RC_TRUE){
            thrlib_thrdExit(0);
        }

        thrlib_mutxLock(&acptThrdCb->mutx);
        thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

        COM_GET_LNKLST_FIRST(&acptThrdCb->acptLnkLst,lnkNode);
        if(lnkNode == NULL){
            thrlib_mutxUnlock(&acptThrdCb->mutx);
            /* sleep */
            usleep(3000);
        }
        else {
            if(acptThrdCb->tmpConnCnt != 0){
                ret = trnlib_sockSelect(acptThrdCb->rdSockFdSet, NULL, 0, NULL);
            }
            else {
                ret = trnlib_sockSelect(acptThrdCb->rdSockFdSet, NULL, 5, NULL);
            }
            if(ret == RC_OK){
                while(1) {
                    acptCb = lnkNode->data;

                    ret = trnlib_sockChkFdSet(acptThrdCb->rdSockFdSet, acptCb->fd);
                    if(ret == RC_OK){
                        ret = trnlib_sockAcpt(acptCb->fd, &acptFd, &dstAddr, RC_TRUE);
                        if(ret != RC_OK){
                            if(ret != RLYERR_ACPT_AGAIN){
                                RLY_LOG(RLY_ERR,"Socket accept failed(ret=%d)\n",ret);
                            }
                        }
                        else {
                            RLY_LOG(RLY_NOTY,"Accept socket(fd=%d, acptFd=%d)\n",acptCb->fd, acptFd);
                            /* insert connection */
                            ret = acpt_addTmpConn(acptThrdCb, acptFd, &dstAddr);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR,"Temp connection insert failed(ret=%d)\n",ret);
                            }

                            acptThrdCb->tmpConnCnt++;
                        }/* end of else */
                    }/* end of if(ret == RC_OK) */

                    COM_GET_NEXT_NODE(lnkNode);
                    if(lnkNode == NULL){
                        thrlib_mutxUnlock(&acptThrdCb->mutx);
                        /* sleep */
                        //usleep(3000);
                        break;
                    }
                }/* end of while(1) */
            }/* end of else */
            else {
                thrlib_mutxUnlock(&acptThrdCb->mutx);
                //if(acptThrdCb->tmpConnCnt == 0){
                //    usleep(3000);
                //}
            }
        }/* end of if(ret == RC_OK) */

        /* check temp connection */
        if(acptThrdCb->tmpConnCnt != 0){
            rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

            thrlib_mutxLock(&acptThrdCb->mutx);

            if(acptThrdCb->tmpConnCnt != 0){
                COM_GET_LNKLST_FIRST(&acptThrdCb->actConnLnkLst, lnkNode);
                if(lnkNode == NULL){
                    acptThrdCb->tmpConnCnt = 0;
                }
                else {
                    while(1){
                        tmpConnCb = lnkNode->data;

                        ret = acpt_chkTmpConn(tmpConnCb, &initReq);
                        if(ret == RC_OK){ /* connection success */
                            UINT hostId = 0;

                            thrlib_mutxLock(&intMainCb->hostMainCb.mutx);

                            ret = rlylibInt_hostAcptHostCb(intMainCb, initReq.host, initReq.hostLen, 
                                    initReq.hostKey, initReq.hostKeyLen, &hostId);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR,"hostCb make failed(ret=%d)\n",ret);
                            }

                            RLY_LOG(RLY_NOTY,"Receive INIT(host=%.*s, fd=%d, hostId=%d)\n", 
                                    initReq.hostLen, initReq.host, tmpConnCb->fd, hostId);

                            /* make acptConn */
                            ret = rlylibInt_hostAddAcptConnCb(intMainCb, hostId, tmpConnCb->fd, &tmpConnCb->dstAddr);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR,"Connection control block add faeild(ret=%d)\n",ret);
                            }
                            else {
                                initAck.host = intMainCb->locHost;
                                initAck.hostLen  = comlib_strGetLen(intMainCb->locHost);
                                initAck.rslt = RLYLIB_RSLT_CODE_SUCC;

                                RLY_LOG(RLY_DBG,"Send INIT ACK(host=%s)\n", intMainCb->locHost);
                                /* send to init ack */
                                ret = rlylibInt_msgEncInitAck(&initAck, tmpMsgBuf, RLYLIB_TMP_MSG_BUF_LEN,  &tmpMsgBufLen);
                                if(ret != RC_OK){
                                    RLY_LOG(RLY_ERR,"Init ack encoding failed(ret=%d)\n",ret);
                                }
                                else{
                                    ret = trnlib_sockWrite(tmpConnCb->fd, RC_TRUE, tmpMsgBuf, tmpMsgBufLen, NULL);
                                    if(ret != RC_OK){
                                        RLY_LOG(RLY_ERR,"Socket write failed(ret=%d, err=%d:%s)\n",ret, errno, strerror(errno));
                                    }
                                }

                            }/* end of else */

                            ret = comlib_lnkLstDel(&acptThrdCb->actConnLnkLst, &tmpConnCb->lnkNode);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR," linked list deleted failed(ret=%d)\n",ret);
                            }
                            else {
                                ret = comlib_lnkLstInsertTail(&acptThrdCb->freeConnLnkLst, &tmpConnCb->lnkNode);
                                if(ret != RC_OK){
                                    RLY_LOG(RLY_ERR,"LnkLst insert failed(ret=%d)\n",ret);
                                }
                            }

                            thrlib_mutxUnlock(&intMainCb->hostMainCb.mutx);
                        }
                        else if(ret != RLYERR_READ_AGAIN){
                            RLY_LOG(RLY_ERR,"tmp connection check failed(ret=%d)\n",ret);
                            /* close connection */
                            trnlib_sockClose(tmpConnCb->fd);
                            tmpConnCb->fd = 0;
                            ret = comlib_lnkLstDel(&acptThrdCb->actConnLnkLst, &tmpConnCb->lnkNode);
                            if(ret != RC_OK){
                                RLY_LOG(RLY_ERR," linked list deleted failed(ret=%d)\n",ret);
                            }
                            else {
                                ret = comlib_lnkLstInsertTail(&acptThrdCb->freeConnLnkLst, &tmpConnCb->lnkNode);
                                if(ret != RC_OK){
                                    RLY_LOG(RLY_ERR,"LnkLst insert failed(ret=%d)\n",ret);
                                }
                            }
                        }

                        COM_GET_NEXT_NODE(lnkNode);
                        if(lnkNode == NULL){
                            break;
                        }
                    }/* end of while(1) */
                }

            }/* end of if(acptThrdCb->tmpConnCnt != 0) */

            thrlib_mutxUnlock(&acptThrdCb->mutx);
            rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
        }/* end of if(acptThrdCb->tmpConnCnt != 0) */
        thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    }/* end of while(1) */

    THRLIB_THRDCLNUP_POP(0);
}

FT_PUBLIC RT_RESULT rlylibInt_acptAddAcptLst(RlylibIntMainCb *intMainCb, TrnlibTransAddr *srcAddr)
{
    SINT ret = RC_OK;

    GEN_CHK_ERR_RET(intMainCb == NULL,
                    RLY_LOG(RLY_ERR,"Rlylib cotnrol block is null\n"),
                    RLYERR_NULL);

    /* lock */
    rlylibInt_syncMutxLock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);

    ret = rlylibInt_acptAddAcptCb(&intMainCb->acptThrdCb, srcAddr);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"accept control block create failed(ret=%d)\n",ret);
        rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
        return ret;
    }

    /* unlock */
    rlylibInt_syncMutxUnlock(&intMainCb->mainMutx, RLYLIB_MUTX_LOCK_TYPE_ALL);
    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_acptAddAcptCb(RlylibIntAcptThrdCb *acptThrdCb, TrnlibTransAddr *srcAddr)
{
    SINT ret = RC_OK;
    RlylibIntAcptCb *acptCb = NULL;
    CHAR srcStrAddr[RLYLIB_STR_TRANS_ADDR_LEN];

    /* check afnum */
    GEN_CHK_ERR_RET(srcAddr->netAddr.afnum != RLYLIB_AFNUM_IPV4  &&
                    srcAddr->netAddr.afnum != RLYLIB_AFNUM_IPV6,
                    RLY_LOG(RLY_ERR,"Invalid afnum(%d)\n",srcAddr->netAddr.afnum),
                    RLYERR_INVALID_AFNUM);

    thrlib_mutxLock(&acptThrdCb->mutx);

    acptCb = acpt_findAcptCb(acptThrdCb, srcAddr);
    if(acptCb != NULL){
        RLY_LOG(RLY_ERR,"Accept control block exist\n");
        thrlib_mutxUnlock(&acptThrdCb->mutx);
        return RLYERR_ACPT_CB_EXIST;
    }

    acptCb = comlib_memMalloc(sizeof(RlylibIntAcptCb));
    if(acptCb == NULL){
        RLY_LOG(RLY_ERR,"Accept control block allocate failed\n");
        thrlib_mutxUnlock(&acptThrdCb->mutx);
        return RLYERR_ACPT_CB_ALLOC_FAILED;
    }

    acptCb->lnkNode.data = acptCb;
    acptCb->lnkNode.next = NULL;
    acptCb->lnkNode.prev = NULL;

    comlib_memMemcpy(&acptCb->srcAddr, srcAddr, sizeof(TrnlibTransAddr));

    acptCb->main = acptThrdCb;

    ret = trnlib_sockOpenLsnSock(srcAddr, &acptCb->fd, RLYLIB_DFLT_LSN_Q, RC_TRUE);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Listen failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&acptThrdCb->mutx);
        return ret;
    }

    /* fd set */
    ret = trnlib_sockAddFdSet(acptThrdCb->rdSockFdSet, acptCb->fd);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Socket fd set failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&acptThrdCb->mutx);
        return RLYERR_SOCK_FD_SET_FAILED;
    }


    /* debug log */
    TRNLIB_CPY_TRANS_ADDR_TO_STR(srcStrAddr, RLYLIB_STR_TRANS_ADDR_LEN, srcAddr);
    RLY_LOG(RLY_NOTY,"OPEN LOCAL SOCK(ip=%s, fd=%d)\n",srcStrAddr, acptCb->fd);

    ret = comlib_lnkLstInsertTail(&acptThrdCb->acptLnkLst, &acptCb->lnkNode);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Accept control block insert faeild(ret=%d)\n",ret);
        comlib_memFree(acptCb);
        thrlib_mutxUnlock(&acptThrdCb->mutx);
        return RLYERR_ACPT_CB_INSERT_FAILED;
    }

    thrlib_mutxUnlock(&acptThrdCb->mutx);

    return RC_OK;
}
