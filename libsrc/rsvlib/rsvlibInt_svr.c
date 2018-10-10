#include <stdio.h>
#include <arpa/inet.h>

#include <microhttpd.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rrllib.h"
#include "rrllib.x"
#include "trnlib.h"
#include "trnlib.x"

#include "rsvlib.h"
#include "rsvlib.x"
#include "rsvlibInt.h"
#include "rsvlibInt.x"

typedef struct SvrSesCb  SvrSesCb;
typedef struct SvrRsp    SvrRsp;

struct SvrRsp{
	UINT                 rsltCode; /* HTTP REQUEST CODE */
    ComlibLnkLst         hdrLst;
	UINT                 datLen;
	CHAR                 *dat;
};

struct SvrSesCb{
    ComlibLnkNode        lnkNode;
    RsvlibIntSvrThrdCb   *svrThrdCb; /* own */
    UINT                 rcvMsgCnt;
    TrnlibTransAddr      dstTrnsAddr;
    UINT                 urlLen;
    VOID                 *strmUsrArg;
    RrllibDoc            *doc;
    ComlibLnkLst         hdrLst; 
    CHAR                 *url;
    ComlibMsg            *msg;
};

FT_PRIVATE RT_RESULT     svr_ansCon           (void *cls, struct MHD_Connection *connection,
                                               const char *url, const char *method,
                                               const char *version, const char *upload_data,
                                               size_t *upload_data_size, void **con_cls);
FT_PRIVATE RT_RESULT     svr_newSes           (RsvlibIntSvrThrdCb *thrdCb, SvrSesCb **rt_sesCb, 
                                               CONST CHAR *url, UINT afnum, struct sockaddr *cliAddr);
FT_PRIVATE RT_RESULT     svr_sesCtrl          (SvrSesCb *sesCb, CHAR *dat, UINT datLen);
FT_PRIVATE RT_RESULT     svr_sesDstry         (SvrSesCb *sesCb);
FT_PRIVATE RT_RESULT     svr_chkHeader        (VOID *cls, enum MHD_ValueKind kind, CONST CHAR *key , CONST CHAR *val);
FT_PRIVATE RT_RESULT     svr_parseKvp         (VOID *cls, enum MHD_ValueKind kind, CONST CHAR *key , CONST CHAR *val);
FT_PRIVATE RT_RESULT     svr_usrHdlr          (RsvlibIntSvrThrdMainCb *svrThrdMainCb, UINT mthod,
                                               SvrSesCb *svrSes, SvrRsp **rt_svrRsp);
FT_PRIVATE ssize_t       svr_strmCallback     (void *cls, uint64_t pos, char *buf, size_t max);
FT_PRIVATE VOID          svr_strmFreeCallback (void *cls);

FT_PRIVATE RT_RESULT svr_newSes(RsvlibIntSvrThrdCb *thrdCb, SvrSesCb **rt_sesCb, 
                                CONST CHAR *url, UINT afnum, struct sockaddr *cliAddr)
{
    SINT ret = RC_OK;
    SvrSesCb *sesCb = NULL;
    RsvlibIntSvrThrdMainCb *svrThrdMainCb = NULL;
    struct sockaddr_in *sockAddrPtr = NULL;
    struct sockaddr_in6 *sockAddr6Ptr = NULL;

    sesCb = comlib_memMalloc(sizeof(SvrSesCb));
    if(sesCb == NULL){
        RSV_LOG(RSV_ERR,"Server session alloc failed\n");
        return RC_NOK;
    }

    sesCb->urlLen = comlib_strGetLen((CHAR*)url);
    sesCb->url = comlib_memMalloc(sesCb->urlLen+1);

    sesCb->strmUsrArg = NULL;

    sesCb->lnkNode.data = sesCb;

    ret = comlib_lnkLstInit(&sesCb->hdrLst, ~0);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Header linked list init failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    comlib_strCpy(sesCb->url, (CHAR*)url);

    sesCb->rcvMsgCnt = 0;

    comlib_msgGetMsg(&sesCb->msg);

    if(afnum == TRNLIB_AFNUM_IPV4){
        sockAddrPtr = (struct sockaddr_in*)cliAddr;
        sesCb->dstTrnsAddr.port = sockAddrPtr->sin_port;
        sesCb->dstTrnsAddr.proto = TRNLIB_TRANS_PROTO_TCP;
        sesCb->dstTrnsAddr.netAddr.afnum = TRNLIB_AFNUM_IPV4;
        sesCb->dstTrnsAddr.netAddr.u.ipv4NetAddr = ntohl(sockAddrPtr->sin_addr.s_addr);
    }
    else {
        sockAddr6Ptr = (struct sockaddr_in6*)cliAddr;
        sesCb->dstTrnsAddr.port = sockAddr6Ptr->sin6_port;
        sesCb->dstTrnsAddr.proto = TRNLIB_TRANS_PROTO_TCP;
        sesCb->dstTrnsAddr.netAddr.afnum = TRNLIB_AFNUM_IPV6;
        comlib_memMemcpy(sesCb->dstTrnsAddr.netAddr.u.ipv6NetAddr, &sockAddr6Ptr->sin6_addr,sizeof(TrnlibIpv6NetAddr));
    }

    svrThrdMainCb = thrdCb->svrThrdMainCb;

    thrlib_mutxLock(&svrThrdMainCb->sesMutx);
    ret = comlib_lnkLstInsertTail(&svrThrdMainCb->actvSvrSes, &sesCb->lnkNode);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"server sessio nreg failed(ret=%d)\n",ret);
    }
    thrlib_mutxUnlock(&svrThrdMainCb->sesMutx);

    sesCb->svrThrdCb = thrdCb;

    (*rt_sesCb) = sesCb;

    return RC_OK;
}

FT_PRIVATE RT_RESULT svr_sesCtrl(SvrSesCb *sesCb, CHAR *dat, UINT datLen)
{
    SINT ret = RC_OK;

    sesCb->rcvMsgCnt++;

    ret = comlib_msgAddMsgEnd(&sesCb->msg, dat, datLen);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Post message add failed(ret=%d)\n",ret);
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT svr_sesDstry(SvrSesCb *sesCb)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL;
    RsvlibHttpHdr *httpHdr = NULL;
    RsvlibIntSvrThrdCb *thrdCb = NULL;
    RsvlibIntSvrThrdMainCb *svrThrdMainCb = NULL;

    sesCb->rcvMsgCnt = 0;

    if(sesCb->url != NULL){
        comlib_memFree(sesCb->url);
        sesCb->url = NULL;
    }

    if(sesCb->doc != NULL){
        rrllib_docDstry(sesCb->doc);
    }

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&sesCb->hdrLst);
        if(lnkNode == NULL){
            break;
        }

        httpHdr = lnkNode->data;

        rsvlibInt_svrFreeHdr(httpHdr);
    }/* end of while(1) */

    thrdCb = sesCb->svrThrdCb;

    svrThrdMainCb = thrdCb->svrThrdMainCb;

    thrlib_mutxLock(&svrThrdMainCb->sesMutx);
    ret = comlib_lnkLstDel(&svrThrdMainCb->actvSvrSes, &sesCb->lnkNode);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"server sessio nreg failed(ret=%d)\n",ret);
    }
    thrlib_mutxUnlock(&svrThrdMainCb->sesMutx);
    comlib_msgPutMsg(&sesCb->msg);

    return RC_OK;
}

FT_PRIVATE RT_RESULT svr_chkHeader(VOID *cls, enum MHD_ValueKind kind, CONST CHAR *key , CONST CHAR *val)
{
    SINT ret = RC_OK;

    RsvlibHttpHdr *httpHdr = NULL;
    SvrSesCb *sesCb = NULL;

    sesCb = (SvrSesCb*)cls;

    ret = rsvlibInt_svrNewHdr(key, val, &httpHdr);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"New http header make fialed(ret=%d)\n",ret);
        return MHD_NO;
    }

    ret = comlib_lnkLstInsertTail(&sesCb->hdrLst, &httpHdr->lnkNode);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"http header insert failed(ret=%d)\n",ret);
        return MHD_NO;
    }

    return MHD_YES;
}

FT_PUBLIC RT_RESULT rsvlibInt_svrFreeHdr(RsvlibHttpHdr *httpHdr)
{
    comlib_memFree(httpHdr->key);
    comlib_memFree(httpHdr->val);

    comlib_memFree(httpHdr);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlibInt_svrNewHdr(CONST CHAR *key, CONST CHAR *val, RsvlibHttpHdr **rt_httpHdr)
{
    RsvlibHttpHdr *hdr = NULL;
    UINT len = 0;

    hdr = comlib_memMalloc(sizeof(RsvlibHttpHdr));
    if(hdr == NULL){
        RSV_LOG(RSV_ERR,"Header alloc failed\n");
        return RC_NOK;
    }

    hdr->lnkNode.data = hdr;

    len = comlib_strGetLen(key);
    hdr->key = comlib_memMalloc(len+1);
    comlib_strCpy(hdr->key, key);
    hdr->keyLen = len;

    len = comlib_strGetLen(val);
    hdr->val = comlib_memMalloc(len+1);
    comlib_strCpy(hdr->val, val);
    hdr->valLen = len;

    (*rt_httpHdr) = hdr;

    return RC_OK;
}

FT_PRIVATE RT_RESULT svr_usrHdlr(RsvlibIntSvrThrdMainCb *svrThrdMainCb, UINT mthod,
                                 SvrSesCb *svrSesCb, SvrRsp **rt_svrRsp)
{
    SINT ret = RC_OK;
    ThrlibThrdId tid = 0;
    RsvlibHttpHdr *httpHdr = NULL;
    ComlibLnkNode *lnkNode = NULL;
    RsvlibIntSvrRule *svrRule = NULL;
    SvrRsp *svrRsp = NULL;
    UINT rcvDatLen = 0;
    CHAR *rcvDat = NULL;
    RsvlibSesCb sesCb;

    ret = ret;

    tid = thrlib_thrdSelf();

    RSV_LOG(RSV_DBG,"[SVR][REQ_RSP] RECEIVE USR TID=[%lu]\n",tid);

    svrRule = (RsvlibIntSvrRule*)svrSesCb->doc->usrArg;

    ret = comlib_lnkLstInit(&sesCb.req.hdrLst, ~0);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Session request header list init faeild(ret=%d)\n",ret);
        return RC_NOK;
    }

    ret = comlib_lnkLstInit(&sesCb.rsp.hdrLst, ~0);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Session response header list init faeild(ret=%d)\n",ret);
        return RC_NOK;
    }

    ret = comlib_lnkLstAppendTail(&sesCb.req.hdrLst, &svrSesCb->hdrLst);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"header append faeild(ret=%d)\n",ret);
        return RC_NOK;
    }

    if(sesCb.req.hdrLst.nodeCnt == 0){
        sesCb.req.hdrPtr = NULL;
    }
    else{
        COM_GET_LNKLST_FIRST(&sesCb.req.hdrLst, sesCb.req.hdrPtr);
    }

    sesCb.req.doc = svrSesCb->doc;
    sesCb.usrArg = svrRule->usrArg;
    sesCb.strmUsrArg = NULL;

    comlib_memMemcpy(&sesCb.dstTrnsAddr, &svrSesCb->dstTrnsAddr, sizeof(TrnlibTransAddr));

    comlib_msgCpyAllMsgIntoDynBuf(&svrSesCb->msg, &rcvDat, &rcvDatLen);

    sesCb.req.dat = rcvDat;
    sesCb.req.datLen = rcvDatLen;

    sesCb.rsp.staCode = 0;
    sesCb.rsp.datLen = 0;
    sesCb.rsp.dat = NULL;

    ret = svrRule->func(mthod, &sesCb);

    /* free request header */
    while(1){
        lnkNode = comlib_lnkLstGetFirst(&sesCb.req.hdrLst);
        if(lnkNode == NULL){
            break;
        }

        httpHdr = lnkNode->data;

        rsvlibInt_svrFreeHdr(httpHdr);
    }/* end of while(1) */

    svrRsp = comlib_memMalloc(sizeof(SvrRsp));

    if(sesCb.rsp.staCode != 0){
        svrRsp->rsltCode = sesCb.rsp.staCode;
    }
    else {
        svrRsp->rsltCode = MHD_HTTP_OK;
    }

    if(sesCb.strmUsrArg != NULL){
        svrSesCb->strmUsrArg = sesCb.strmUsrArg;
    }

    ret = comlib_lnkLstInit(&svrRsp->hdrLst, ~0);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Header list init failed(ret=%d)\n",ret);
        comlib_memFree(svrRsp);
        return RC_NOK;
    }

    if(sesCb.rsp.hdrLst.nodeCnt != 0){
        ret = comlib_lnkLstAppendTail(&svrRsp->hdrLst, &sesCb.rsp.hdrLst);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Resposne hader append failed(ret=%d)\n",ret);
            comlib_memFree(svrRsp);
            return RC_NOK;
        }
    }

    svrRsp->datLen = 0;

    if(sesCb.rsp.datLen != 0){
        svrRsp->dat = sesCb.rsp.dat;
        svrRsp->datLen = sesCb.rsp.datLen;
    }

    (*rt_svrRsp) = svrRsp;

    return RC_OK;
}

FT_PRIVATE ssize_t svr_strmCallback(void *cls, uint64_t pos, char *buf, size_t max)
{
    SvrSesCb *sesCb = NULL;
    RsvlibIntSvrRule *svrRule = NULL;

    sesCb = (SvrSesCb*)cls;

    svrRule = (RsvlibIntSvrRule*)sesCb->doc->usrArg;

    return svrRule->strmFunc(sesCb->strmUsrArg, pos, buf, max);
}

FT_PRIVATE VOID svr_strmFreeCallback(void *cls)
{
    SINT ret = RC_OK;
    SvrSesCb *sesCb = NULL;
    RsvlibIntSvrRule *svrRule = NULL;

    fprintf(stderr,"STRM FREE\n");
    sesCb = (SvrSesCb*)cls;

    svrRule = (RsvlibIntSvrRule*)sesCb->doc->usrArg;

    svrRule->strmFreeFunc(sesCb->strmUsrArg);

    ret = svr_sesDstry(sesCb);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Post session destory failed(ret=%d)\n",ret);
    }

    comlib_memFree(sesCb);
}

FT_PRIVATE RT_RESULT svr_parseKvp(VOID *cls, enum MHD_ValueKind kind, CONST CHAR *key , CONST CHAR *val)
{
    SINT ret = 0;
    RrllibQueryDocCfg *queryDocCfg = NULL;

    queryDocCfg = cls;

    if(queryDocCfg->doc == NULL){
        RSV_LOG(RSV_ERR,"document not exist\n");
        return MHD_NO;
    }

    if(val == NULL){
        RSV_LOG(RSV_ERR,"value not exist(key=%s)\n", key);
        comlib_memFree(queryDocCfg->doc);
        queryDocCfg->doc = NULL;
        return MHD_NO;
    }
    ret = rrllib_parseKvpToDoc((CHAR*)key, comlib_strGetLen((CHAR*)key), (CHAR*)val, comlib_strGetLen((CHAR*)val), queryDocCfg);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Key value setting failed(ret=%d)\n",ret);
        comlib_memFree(queryDocCfg->doc);
        queryDocCfg->doc = NULL;
        return MHD_NO;
    }

    return MHD_YES;
}

FT_PRIVATE RT_RESULT svr_ansCon(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *strMthod,
                                const char *ver, const char *upload_data,
                                size_t *upload_data_size, void **con_cls)
{
    SINT ret = RC_OK;
    ThrlibThrdId tid = 0;
    UINT mthod = 0;
    UINT mthodId = 0;
    UINT errStaCode = MHD_HTTP_OK;
    RrllibQueryDocCfg queryDocCfg;
    RrllibResMthod *resMthod = NULL;
    RrllibDoc *doc = NULL;
    RsvlibIntSvrThrdCb *svrThrdCb = NULL;
    SvrRsp *rsp = NULL;
    ComlibLnkNode *lnkNode = NULL;
    struct sockaddr_in *cliSock = NULL;
    struct MHD_Response *mhdRsp = NULL;
    SvrSesCb *sesCb = NULL;

    svrThrdCb = (RsvlibIntSvrThrdCb*)cls;

    tid = thrlib_thrdSelf();
    cliSock = (struct sockaddr_in*)MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;

    if(comlib_strCaseCmp(strMthod,"POST") == 0){
        mthod = RRL_MTHOD_POST;
        mthodId = RSV_MTHOD_POST;
    }
    else if(comlib_strCaseCmp(strMthod,"DELETE") == 0){
        mthod = RRL_MTHOD_DEL;
        mthodId = RSV_MTHOD_DEL;
    }
    else if(comlib_strCaseCmp(strMthod,"GET") == 0){
        mthod = RRL_MTHOD_GET;
        mthodId = RSV_MTHOD_GET;
    }
    else if(comlib_strCaseCmp(strMthod,"PUT") == 0){
        mthod = RRL_MTHOD_PUT;
        mthodId = RSV_MTHOD_PUT;
    }

    if(*con_cls == NULL){ /* first sequence */
        struct sockaddr *cliAddr = NULL;

        /* check url */
        ret = rrllib_parseUriPath(&svrThrdCb->svrThrdMainCb->rrlCb, mthod, (CHAR*)url, 
                                  comlib_strGetLen((CHAR*)url), &resMthod, &doc);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Uri parsing faield(ret=%d)\n",ret);
            errStaCode = MHD_HTTP_NOT_FOUND;
            goto goto_retErr;
        }

        queryDocCfg.resMthod = resMthod;
        queryDocCfg.doc = doc;

        /* get query data */
        ret = MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, svr_parseKvp, &queryDocCfg);
        if(queryDocCfg.doc == NULL){
            RSV_LOG(RSV_ERR,"Key value pair parinsg failed\n");
            errStaCode = MHD_HTTP_BAD_REQUEST;
            goto goto_retErr;
        }

        /* check mand check */
        if(doc->mandCnt != resMthod->mandCnt){
            RSV_LOG(RSV_ERR,"Mandatory query not exist\n");
            errStaCode = MHD_HTTP_BAD_REQUEST;
            goto goto_retErr;
        }


        /* ipv4 addr */
        cliAddr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;

        ret = svr_newSes(svrThrdCb, &sesCb, url, TRNLIB_AFNUM_IPV4, cliAddr);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Post session make failed(ret=%d)\n",ret);
            rrllib_docDstry(doc);
            return MHD_NO;
        }

        ret = MHD_get_connection_values(connection, MHD_HEADER_KIND, svr_chkHeader, sesCb);

        sesCb->doc = doc;

        *con_cls = sesCb;

        RSV_LOG(RSV_DBG,"RECEIVE FIRST SEQ HTTP MESSAGE(mthod=[%s], url=[%s])\n",strMthod, url);

        return MHD_YES;
    }/* end of if(*con_cls == NULL) first sequence */

    if(*upload_data_size != 0){ /* post sequence continue */
        /* update request message */
        ret = svr_sesCtrl((SvrSesCb*)*con_cls, (CHAR*)upload_data, *upload_data_size);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Post session control failed(ret=%d)\n",ret);
            return MHD_NO;
        }

        *upload_data_size = 0;

        return MHD_YES;
    }
    else { /* last sequence */
        RsvlibIntSvrRule *svrRule = NULL;

        sesCb = (SvrSesCb*)*con_cls;

        svrRule = (RsvlibIntSvrRule*)sesCb->doc->usrArg;

        ret = svr_usrHdlr(svrThrdCb->svrThrdMainCb, mthodId, sesCb, &rsp);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Post handler failed(ret=%d)\n",ret);
            svr_sesDstry(sesCb);
            comlib_memFree(sesCb);

            errStaCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
            goto goto_retErr;
        }

        if((svrRule->type == RSV_SVR_RULE_STRM) && (rsp->rsltCode == MHD_HTTP_OK)){
            mhdRsp = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                        svrRule->svrOpt.maxStrmBufLen,
                                                        svr_strmCallback, sesCb, svr_strmFreeCallback);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, mhdRsp);
            MHD_destroy_response (mhdRsp);

            RSV_LOG(RSV_DBG,"[SVR][RSP_SND] PORT=[%u], MTHOD=[%s], URL=[%s] TID=[%lu]\n",
                    cliSock->sin_port, strMthod, url, tid);

            comlib_memFree(rsp);

            //return MHD_YES;
            return ret;
        }

        ret = svr_sesDstry(sesCb);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Post session destory failed(ret=%d)\n",ret);
        }

        comlib_memFree(sesCb);
    }/* end of else last sequence */

    /* make response */
    if(rsp != NULL){
        mhdRsp = MHD_create_response_from_buffer (rsp->datLen, (VOID*) rsp->dat, MHD_RESPMEM_MUST_COPY);

        MHD_add_response_header(mhdRsp, "Content-Type", "application/json");

        COM_GET_LNKLST_FIRST(&rsp->hdrLst, lnkNode);
        if(lnkNode != NULL){
            RsvlibHttpHdr *httpHdr = NULL;

            while(1){
                httpHdr = lnkNode->data;

                MHD_add_response_header(mhdRsp, httpHdr->key, httpHdr->val);

                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    break;
                }
            }/* end of while(1) */
        }/* end of if(lnkNode != NULL) */

        ret = MHD_queue_response (connection, rsp->rsltCode, mhdRsp);
        MHD_destroy_response (mhdRsp);

        if(rsp->hdrLst.nodeCnt != 0){
            RsvlibHttpHdr *httpHdr = NULL;

            while(1){
                lnkNode = comlib_lnkLstGetFirst(&rsp->hdrLst);
                if(lnkNode == NULL){
                    break;
                }

                httpHdr = lnkNode->data;

                rsvlibInt_svrFreeHdr(httpHdr);

            }/* end of while(1) */
        }

        if(rsp->datLen != 0){
            comlib_memFree(rsp->dat);
        }

        comlib_memFree(rsp);
    }
    else {
        errStaCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
        goto goto_retErr;
    }

    RSV_LOG(RSV_DBG,"[SVR][RSP_SND] PORT=[%u], MTHOD=[%s], URL=[%s] TID=[%lu]\n",
            cliSock->sin_port, strMthod, url, tid);

    return MHD_YES;

goto_retErr:
    RSV_LOG(RSV_ERR,"[SVR][RSP_SND] PORT=[%u], MTHOD=[%s], URL=[%s] TID=[%lu], ERR=[%d]\n",
            cliSock->sin_port, strMthod, url, tid, errStaCode);

    mhdRsp = MHD_create_response_from_buffer (0, (VOID*)NULL, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(mhdRsp, "Content-Type", "application/json");
    ret = MHD_queue_response (connection, errStaCode, mhdRsp);
    MHD_destroy_response (mhdRsp);

    return MHD_YES;
}

FT_PUBLIC RT_RESULT rsvlibInt_svrStop(RsvlibIntSvrThrdMainCb *svrThrdMainCb)
{
    SINT ret = RC_OK;

    MHD_stop_daemon(svrThrdMainCb->dm);

    ret = rsvlibInt_svrClearSvrSes(svrThrdMainCb);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Server session clear failed(ret=%d)\n",ret);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlibInt_svrClearSvrSes(RsvlibIntSvrThrdMainCb *svrThrdMainCb)
{
    SINT ret = RC_OK;
    SvrSesCb *sesCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    while(1){
        COM_GET_LNKLST_FIRST(&svrThrdMainCb->actvSvrSes, lnkNode);
        if(lnkNode == NULL){
            break;
        }
        
        fprintf(stderr,"SES\n");
        sesCb = lnkNode->data;

        ret = svr_sesDstry(sesCb);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"Session destroy faield(ret=%d)\n",ret);
        }
    }/* end of while(1) */


    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlibInt_svrStart(RsvlibIntSvrThrdMainCb *svrThrdMainCb, CHAR *cert, CHAR *key, UINT thrdCnt, UINT port)
{
    SINT ret = RC_OK;
    RsvlibIntSvrThrdCb *svrThrdCb = NULL;
    CHAR *strKey = NULL;
    CHAR *strCert = NULL;
    BOOL thrdPoolFlg = RC_TRUE;

    svrThrdCb = comlib_memMalloc(sizeof(RsvlibIntSvrThrdCb));

    svrThrdCb->svrThrdMainCb  = svrThrdMainCb;

    if(thrdCnt == 0){
        thrdPoolFlg = RC_FALSE;
    }

    if((cert == NULL) || (key == NULL)){
        if(thrdPoolFlg == RC_TRUE){
            svrThrdMainCb->dm = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, port, NULL, NULL,
                                                  &svr_ansCon, svrThrdCb, 
                                                  MHD_OPTION_THREAD_POOL_SIZE, thrdCnt, MHD_OPTION_END);
        }
        else {
            svrThrdMainCb->dm = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION , port, NULL, NULL,
                                                  &svr_ansCon, svrThrdCb, 
                                                  MHD_OPTION_END);
        }
    }
    else {
        ret = rsvlibInt_utilReadFile(key, &strKey);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"ssl key read failed(ret=%d)\n",ret);
            return RC_NOK;
        }

        ret = rsvlibInt_utilReadFile(cert, &strCert);
        if(ret != RC_OK){
            RSV_LOG(RSV_ERR,"ssl cert read failed(ret=%d)\n",ret);
            comlib_memFree(strKey);
            return RC_NOK;
        }
        if(thrdPoolFlg == RC_TRUE){
            svrThrdMainCb->dm = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL, port, NULL, NULL,
                                                  &svr_ansCon, svrThrdCb,
                                                  MHD_OPTION_HTTPS_MEM_KEY,
                                                  strKey,
                                                  MHD_OPTION_HTTPS_MEM_CERT,
                                                  strCert,
                                                  MHD_OPTION_THREAD_POOL_SIZE, thrdCnt, MHD_OPTION_END);
        }
        else {
            svrThrdMainCb->dm = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL, port, NULL, NULL,
                                                  &svr_ansCon, svrThrdCb,
                                                  MHD_OPTION_HTTPS_MEM_KEY,
                                                  strKey,
                                                  MHD_OPTION_HTTPS_MEM_CERT,
                                                  strCert,
                                                  MHD_OPTION_END);
        }

        comlib_memFree(strKey);
        comlib_memFree(strCert);
    }/* end of else */

    if(svrThrdMainCb->dm == NULL){
        RSV_LOG(RSV_ERR,"svr http start failed\n");
    }

    return RC_OK;
}

