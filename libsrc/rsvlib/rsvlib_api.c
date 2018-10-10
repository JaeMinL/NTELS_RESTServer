#include <unistd.h>
#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rrllib.h"
#include "rrllib.x"

#include "rsvlib.h"
#include "rsvlib.x"
#include "rsvlibInt.h"
#include "rsvlibInt.x"

FT_PRIVATE RT_RESULT api_findHdr(RsvlibSesCb *sesCb,
                                 CHAR *key, UINT keyLen, UINT seq,
                                 CHAR **rt_val, UINT *rt_valLen, 
                                 BOOL caseFlg);

FT_PRIVATE RT_RESULT api_findHdr(RsvlibSesCb *sesCb,
                                 CHAR *key, UINT keyLen, UINT seq,
                                 CHAR **rt_val, UINT *rt_valLen, 
                                 BOOL caseFlg)
{
    SINT ret = 0;
    RsvlibHttpHdr *httpHdr = NULL;
    ComlibLnkNode *lnkNode = NULL;
    UINT findSeq = 0;

    findSeq = seq;

    COM_GET_LNKLST_FIRST(&sesCb->req.hdrLst, lnkNode);
    if(lnkNode != NULL){
        while(1){
            httpHdr = lnkNode->data;

            if(httpHdr->keyLen == keyLen){
                if(caseFlg == RC_TRUE){
                    ret = comlib_strCaseNCmp(httpHdr->key, key, keyLen);
                }
                else{
                    ret = comlib_strNCmp(httpHdr->key, key, keyLen);
                }
                if(ret == 0){
                    if(findSeq == 0){
                        (*rt_val) = httpHdr->val;
                        (*rt_valLen) = httpHdr->valLen;

                        return RC_OK;
                    }

                    findSeq--;
                }
            }/* end of if(httpHdr->keyLen == keyLen) */

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }/* end of if(lnkNode != NULL) */

    return RC_NOK;

}


FT_PUBLIC RT_RESULT rsvlib_apiSetLogFunc(UINT lvl, RsvlibLogFunc logFunc)
{
    SINT ret = RC_OK;

    if(logFunc == NULL){
        return RSVERR_LOG_FUNC_NULL;
    }

    if(lvl > RSV_DBG){
        return RSVERR_INVALID_LOG_LEVEL;
    }

    ret = rsvlibInt_globSetLogFunc(lvl, logFunc);

    return ret;
}

FT_PUBLIC RT_RESULT rsvlib_apiSetRule(UINT id, UINT mthod, CHAR *url, CHAR *query, VOID *usrArg, RsvlibProcFunc func)
{
    SINT ret = RC_OK;
    RsvlibIntCb *rsvlibCb = NULL;
    RsvlibIntSvrRule *svrRule = NULL;
    UINT mthodId = 0;

    rsvlibCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibCb == NULL){
        RSV_LOG(RSV_ERR,"Rsvlib control block not exist\n");
        return RSVERR_RSVLIB_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&rsvlibCb->runMutx);

    if(rsvlibCb->runFlg == RC_TRUE){
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_ALREADY_RUNNING;
    }

    switch(mthod){
        case RSV_MTHOD_GET:  mthodId = RRL_MTHOD_GET;  break;
        case RSV_MTHOD_POST: mthodId = RRL_MTHOD_POST; break;
        case RSV_MTHOD_DEL:  mthodId = RRL_MTHOD_DEL;  break;
        case RSV_MTHOD_PUT:  mthodId = RRL_MTHOD_PUT;  break;
        case RSV_MTHOD_HEAD:
        case RSV_MTHOD_UPD:
        case RSV_MTHOD_OPT:
        case RSV_MTHOD_TRACE:
        case RSV_MTHOD_CON:
        default:
                             RSV_LOG(RSV_ERR,"Unsuppoert method(%d)\n", mthod);
                             thrlib_mutxUnlock(&rsvlibCb->runMutx);
                             return RSVERR_UNSUPP_MTHOD;
    }

    svrRule = comlib_memMalloc(sizeof(RsvlibIntSvrRule));
    if(svrRule == NULL){
        RSV_LOG(RSV_ERR,"Server rule alloc failed\n");
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_ALLOC_FAILED;
    }

#if 0
    svrRule->svrOpt.simDat = NULL;
#else
    ret = comlib_lnkLstInit(&svrRule->svrOpt.simDatLl, ~0);
    if(ret != RC_OK){
       RSV_LOG(RSV_ERR,"Simulator date linked list init failed(ret=%d)\n",ret);
       return RSVERR_LNKLST_INIT_FAILED;
    }
#endif
    svrRule->svrOpt.maxStrmBufLen = 0;
    svrRule->type = RSV_SVR_RULE_NONE;
    svrRule->usrArg = usrArg;
    svrRule->func = func;

    ret = rrllib_parseResCfg(&rsvlibCb->svrThrdMainCb.rrlCb,
                             url,
                             comlib_strGetLen(url),
                             mthodId, 
                             query,
                             comlib_strGetLen(query),
                             (VOID*)svrRule);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Rsvlib rulet config failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_RULE_PARSE_FALED;
    }

    thrlib_mutxUnlock(&rsvlibCb->runMutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiSetRuleStrm(UINT id, UINT mthod, CHAR *url, CHAR *query, VOID *usrArg, 
                                          RsvlibProcFunc func,
                                         RsvlibStrmProcFunc strmFunc, RsvlibStrmFreeFunc freeFunc, UINT strmBufLen)
{
    SINT ret = RC_OK;
    RsvlibIntCb *rsvlibCb = NULL;
    RsvlibIntSvrRule *svrRule = NULL;
    UINT mthodId = 0;

    rsvlibCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibCb == NULL){
        RSV_LOG(RSV_ERR,"Rsvlib control block not exist\n");
        return RSVERR_RSVLIB_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&rsvlibCb->runMutx);

    if(rsvlibCb->runFlg == RC_TRUE){
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_ALREADY_RUNNING;
    }

    if(rsvlibCb->genCfg.svrCfg.thrdCnt != 0){
        return RSVERR_THRD_PER_CONN_ONLY;
    }

    switch(mthod){
        case RSV_MTHOD_GET:  mthodId = RRL_MTHOD_GET;  break;
        case RSV_MTHOD_POST: mthodId = RRL_MTHOD_POST; break;
        case RSV_MTHOD_DEL:  mthodId = RRL_MTHOD_DEL;  break;
        case RSV_MTHOD_PUT:  mthodId = RRL_MTHOD_PUT;  break;
        case RSV_MTHOD_HEAD:
        case RSV_MTHOD_UPD:
        case RSV_MTHOD_OPT:
        case RSV_MTHOD_TRACE:
        case RSV_MTHOD_CON:
        default:
                             RSV_LOG(RSV_ERR,"Unsuppoert method(%d)\n", mthod);
                             thrlib_mutxUnlock(&rsvlibCb->runMutx);
                             return RSVERR_UNSUPP_MTHOD;
    }

    svrRule = comlib_memMalloc(sizeof(RsvlibIntSvrRule));
    if(svrRule == NULL){
        RSV_LOG(RSV_ERR,"Server rule alloc failed\n");
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_ALLOC_FAILED;
    }

    //svrRule->svrOpt.simDat = NULL;
    svrRule->type = RSV_SVR_RULE_STRM;
    if(strmBufLen == 0){
        svrRule->svrOpt.maxStrmBufLen = RSV_MAX_STRM_BUF_LEN;
    }
    else{
        svrRule->svrOpt.maxStrmBufLen = strmBufLen;
    }

    svrRule->usrArg = usrArg;
    svrRule->func = func;
    svrRule->strmFunc = strmFunc;
    svrRule->strmFreeFunc = freeFunc;

    ret = rrllib_parseResCfg(&rsvlibCb->svrThrdMainCb.rrlCb,
                             url,
                             comlib_strGetLen(url),
                             mthodId, 
                             query,
                             comlib_strGetLen(query),
                             (VOID*)svrRule);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Rsvlib rulet config failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_RULE_PARSE_FALED;
    }

    thrlib_mutxUnlock(&rsvlibCb->runMutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiFindHdr(RsvlibSesCb *sesCb,
                                      CHAR *key, UINT keyLen, UINT seq,
                                      CHAR **rt_val, UINT *rt_valLen)
{
    return api_findHdr(sesCb,key, keyLen, seq, rt_val,rt_valLen, RC_FALSE);
}

FT_PUBLIC RT_RESULT rsvlib_apiFindCaseHdr(RsvlibSesCb *sesCb,
                                          CHAR *key, UINT keyLen, UINT seq,
                                          CHAR **rt_val, UINT *rt_valLen)
{
    return api_findHdr(sesCb,key, keyLen, seq, rt_val,rt_valLen, RC_TRUE);
}

FT_PUBLIC RT_RESULT rsvlib_apiGetFirstHdr(RsvlibSesCb *sesCb, 
                                        CHAR **rt_key, UINT *rt_keyLen, 
                                        CHAR **rt_val, UINT *rt_valLen)
{
    RsvlibHttpHdr *httpHdr = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&sesCb->req.hdrLst, lnkNode);
    if(lnkNode == NULL){
        (*rt_keyLen) = 0;
        (*rt_valLen) = 0;
        return RC_OK;
    }

    httpHdr = lnkNode->data;

    (*rt_key) = httpHdr->key;
    (*rt_keyLen) = httpHdr->keyLen;
    (*rt_val) = httpHdr->val;
    (*rt_valLen) = httpHdr->valLen;

    sesCb->req.hdrPtr = lnkNode;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiGetNxtHdr(RsvlibSesCb *sesCb, 
                                      CHAR **rt_key, UINT *rt_keyLen, 
                                      CHAR **rt_val, UINT *rt_valLen)
{
    RsvlibHttpHdr *httpHdr = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(sesCb->req.hdrPtr == NULL){
        (*rt_keyLen) = 0;
        (*rt_valLen) = 0;
        return RC_OK;
    }

    COM_GET_NEXT_NODE(sesCb->req.hdrPtr);
    if(sesCb->req.hdrPtr == NULL){
        (*rt_keyLen) = 0;
        (*rt_valLen) = 0;
        return RC_OK;
    }

    lnkNode = sesCb->req.hdrPtr;

    httpHdr = lnkNode->data;

    (*rt_key) = httpHdr->key;
    (*rt_keyLen) = httpHdr->keyLen;
    (*rt_val) = httpHdr->val;
    (*rt_valLen) = httpHdr->valLen;

    sesCb->req.hdrPtr = lnkNode;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiSetRspHdr(RsvlibSesCb *sesCb, CHAR *key, CHAR *val, BOOL cpyFlg)
{
    SINT ret = RC_OK;
    UINT keyLen = 0;
    UINT valLen = 0;
    RsvlibHttpHdr *httpHdr = NULL;

    keyLen = comlib_strGetLen(key);
    valLen = comlib_strGetLen(val);

    httpHdr = comlib_memMalloc(sizeof(RsvlibHttpHdr));
    if(httpHdr == NULL){
        RSV_LOG(RSV_ERR,"Http header alloc failed\n");
        return RC_NOK;
    }

    if(cpyFlg == RC_TRUE){
        httpHdr->key = comlib_memMalloc(keyLen + 1);
        comlib_strCpy(httpHdr->key, key);
        httpHdr->keyLen = keyLen;

        httpHdr->val = comlib_memMalloc(valLen + 1);
        comlib_strCpy(httpHdr->val, val);
        httpHdr->valLen = valLen;
    }
    else{
        httpHdr->key = key;
        httpHdr->keyLen = keyLen;

        httpHdr->val = val;
        httpHdr->valLen = valLen;
    }

    httpHdr->lnkNode.data = httpHdr;

    ret = comlib_lnkLstInsertTail(&sesCb->rsp.hdrLst, &httpHdr->lnkNode);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Linked list intart failed(ret=%d)\n",ret);
        return RSVERR_LNKLST_INSERT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiInit(UINT id, RsvlibGenCfg *genCfg)
{
    return rsvlibInt_init(id, genCfg);
}

FT_PUBLIC RT_RESULT rsvlib_apiRun(UINT id)
{
    return rsvlibInt_mainRun(id);
}

FT_PUBLIC RT_RESULT rsvlib_apiSetStaCode(RsvlibSesCb *sesCb, UINT staCode)
{
    sesCb->rsp.staCode = staCode; 

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiSetRspDat(RsvlibSesCb *sesCb, CHAR *rspDat, BOOL cpyFlg)
{
    sesCb->rsp.datLen = comlib_strGetLen(rspDat);

    if(cpyFlg == RC_TRUE){
        sesCb->rsp.dat = comlib_memMalloc(sesCb->rsp.datLen + 1);
        comlib_strCpy(sesCb->rsp.dat, rspDat);
    }
    else{
        sesCb->rsp.dat = rspDat;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiStop(UINT id)
{
    rsvlibInt_mainStop(id);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiFindArg(RsvlibSesCb *sesCb, CHAR *name, RrllibDocArg **rt_docArg)
{
    SINT ret = RC_OK;
    UINT nameLen = 0;

    nameLen = comlib_strGetLen(name);

    /* RC_OK = FOUND
     * RC_NOK = NOT FOUND
     */
    ret = rrllib_docFindArg(sesCb->req.doc, name, nameLen, rt_docArg);

    return ret;
}

FT_PUBLIC RT_RESULT rsvlib_apiFirstArg(RsvlibSesCb *sesCb, RrllibDocArg **rt_docArg)
{
    /* RC_OK = FOUND
     * RC_NOK = NOT FOUND
     */
    return rrllib_docGetFirstArg(sesCb->req.doc, rt_docArg);
}

FT_PUBLIC RT_RESULT rsvlib_apiNxtArg(RsvlibSesCb *sesCb, RrllibDocArg **rt_docArg)
{
    /* RC_OK = FOUND
     * RC_NOK = NOT FOUND
     */
    return rrllib_docGetNxtArg(sesCb->req.doc, rt_docArg);
}

FT_PUBLIC RT_RESULT rsvlib_apiFirstArgVal(RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen)
{
    /* RC_OK = FOUND
     * RC_NOK = NOT FOUND
     */
    return rrllib_docGetFirstVal(docArg, rt_val, rt_valLen);
}

FT_PUBLIC RT_RESULT rsvlib_apiNxtArgVal(RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen)
{
    return rrllib_docGetNxtVal(docArg, rt_val, rt_valLen);
}

FT_PUBLIC RT_RESULT rsvlib_apiSetSimDat(UINT id, UINT mthod, CHAR *url, CHAR *simDat, BOOL cpyFlg)
{
#if 0
    RrllibResMthod *resMthod = NULL;
    RrllibDoc *doc = NULL;

    rsvlibCb = rsvlibInt_globGetRsvlibIntCb(id);
    if(rsvlibCb == NULL){
        RSV_LOG(RSV_ERR,"Rsvlib control block not exist\n");
        return RSVERR_RSVLIB_CB_NOT_EXIST;
    }

    thrlib_mutxLock(&rsvlibCb->runMutx);

    ret = rrllib_parseUriPath(&rsvlibCb->svrThrdMainCb->rrlCb, mthod, (CHAR*)url,
                              comlib_strGetLen((CHAR*)url), &resMthod, &doc);
    if(ret != RC_OK){
        RSV_LOG(RSV_ERR,"Url not exist(mthod=%d, url=%s\n",mthod, url);
        thrlib_mutxUnlock(&rsvlibCb->runMutx);
        return RSVERR_RULE_NOT_EXIST;
    }

    svrRule = (RsvlibIntSvrRule*)doc->usrArg;

    simDat = comlib_memMalloc(sizeof(RsvlibIntSimDat));

    simDat->datLen = comlib_strGetLen(simDat);

    if(cpyFlg == RC_TRUE){
        simDat->dat = comlib_memMalloc(simDat->datLen);
        comlib_strNCpy(simDat->dat, simDat, simDat->datLen);
    }
    else{
        simDat->dat = simDat;
    }


    rrllib_docDstry(doc);

    thrlib_mutxUnlock(&rsvlibCb->runMutx);
#endif

    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiEnbSimDat(UINT id, UINT mthod, CHAR *url)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiDisbSimDat(UINT id, UINT mthod, CHAR *url)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rsvlib_apiDelSimDat(UINT id, UINT mthod, CHAR *url)
{
    return RC_OK;
}

