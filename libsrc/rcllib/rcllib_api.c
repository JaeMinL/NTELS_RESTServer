#include <stdio.h>
#include <curl/curl.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rcllib.h"
#include "rcllib.x"
#include "rcllibInt.h"
#include "rcllibInt.x"


FT_PRIVATE size_t api_wrteDat(void *ptr, size_t size, size_t bufSize, void *usrArg);

FT_PRIVATE size_t api_wrteDat(void *ptr, size_t size, size_t bufSize, void *usrArg)
{
    RcllibIntCb *intRcllibCb = NULL;

    intRcllibCb = (RcllibIntCb*)usrArg;

    if(intRcllibCb->rspMsg == NULL){
        comlib_msgGetMsg(&intRcllibCb->rspMsg);
    }

    comlib_msgAddMsgEnd(&intRcllibCb->rspMsg, (CHAR*)ptr, (size * bufSize)); 

    return size * bufSize;
}

FT_PUBLIC RT_RESULT rcllib_apiGlobInit()
{
    rcllibInt_globInit();

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiInit(RcllibCb *rcllibCb)
{

    RcllibIntCb *intRcllibCb = NULL;

    intRcllibCb = comlib_memMalloc(sizeof(RcllibIntCb));

    intRcllibCb->hdlr = curl_easy_init();
    intRcllibCb->reqHdrLst = NULL;
    intRcllibCb->rspMsg = NULL;
    intRcllibCb->usrArg = NULL;

    rcllibCb->main = (RcllibMainCb)intRcllibCb;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiDstry(RcllibCb *rcllibCb)
{
    RcllibIntCb *intRcllibCb = NULL;

    intRcllibCb = (RcllibIntCb*)rcllibCb->main;

    if(intRcllibCb->rspMsg != NULL){
        comlib_msgPutMsg(&intRcllibCb->rspMsg);
        intRcllibCb->rspMsg = NULL;
    }

    curl_easy_cleanup(intRcllibCb->hdlr);

    comlib_memFree(intRcllibCb);

    return RC_OK;
}

#if 0
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
        /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
        /* 'userdata' is set with CURLOPT_HEADERDATA */
        fprintf(stderr,"HDR: %s", buffer);
            return nitems * size;
}
#endif

FT_PUBLIC RT_RESULT rcllib_apiPfrm(RcllibCb *rcllibCb, UINT mthod, CHAR *url, RcllibHdrLst *hdrLst, CHAR *dat)
{
    SINT ret = RC_OK;
    RcllibIntCb *intRcllibCb = NULL;
    CURL *hdlr = NULL;
    CHAR *strMthod = NULL;
    RcllibHdr *hdr = NULL;
    ComlibLnkNode *lnkNode = NULL;

    intRcllibCb = (RcllibIntCb*)rcllibCb->main;

    hdlr = intRcllibCb->hdlr;

    switch(mthod){
        case RCLLIB_MTHOD_GET:    strMthod = "GET";    break;
        case RCLLIB_MTHOD_POST:   strMthod = "POST";   break;
        case RCLLIB_MTHOD_DEL:    strMthod = "DELETE"; break;
        case RCLLIB_MTHOD_PUT:    strMthod = "PUT";    break;
        default:                  return RC_NOK;
    };

    curl_easy_setopt(hdlr, CURLOPT_CUSTOMREQUEST, strMthod);
    curl_easy_setopt(hdlr, CURLOPT_URL, url);

    /* set header */
    if(hdrLst != NULL){
        COM_GET_LNKLST_FIRST(&hdrLst->hdrLl, lnkNode);
        if(lnkNode != NULL){
            CHAR tmpBuf[RCLLIB_MAX_HDR_BUF];

            while(1){
                hdr = lnkNode->data;

                comlib_strSNPrnt(tmpBuf, RCLLIB_MAX_HDR_BUF, "%s: %s", hdr->key, hdr->val);

                intRcllibCb->reqHdrLst = curl_slist_append(intRcllibCb->reqHdrLst, tmpBuf);

                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    break;
                }
            }/* end of while(1) */

        }/* end of if(lnkNode != NULL) */
    }/* end of if(hdrLst != NULL) */

    if(intRcllibCb->reqHdrLst != NULL){
        curl_easy_setopt(hdlr, CURLOPT_HTTPHEADER, intRcllibCb->reqHdrLst);
    }

    curl_easy_setopt(hdlr, CURLOPT_WRITEDATA, intRcllibCb);
    curl_easy_setopt(hdlr, CURLOPT_WRITEFUNCTION, api_wrteDat);

    if(dat != NULL){
        curl_easy_setopt(hdlr, CURLOPT_POSTFIELDS, dat);
    }

    ret = curl_easy_perform(hdlr);
    if(ret != CURLE_OK){
        RCL_LOG(RCL_ERR,"perfom failed(ret=%d)\n",ret);
        return RCLERR_PERFORM_FAILED;
    }

    curl_slist_free_all(intRcllibCb->reqHdrLst);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiSetTimeOut(RcllibCb *rcllibCb, UINT tmOut)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiSetRsvFunc(RcllibCb *rcllibCb, RcllibWrteFunc func, VOID *usrArg)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiInitHdrLst(RcllibHdrLst *hdrLst)
{
    SINT ret = RC_OK;

    ret = comlib_lnkLstInit(&hdrLst->hdrLl, ~0);
    if(ret != RC_OK){
        RCL_LOG(RCL_ERR,"Linked list init failed(ret=%d)\n",ret);

        return RCLERR_LNKLST_INIT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiAddHdrLst(RcllibHdrLst *hdrLst, CONST CHAR *key, CONST CHAR *val)
{
    SINT ret = RC_OK;
    RcllibHdr *hdr = NULL;
    UINT keyLen = 0;
    UINT valLen = 0;

    hdr = comlib_memMalloc(sizeof(RcllibHdr));
    if(hdr == NULL){
        RCL_LOG(RCL_ERR,"Header alloc failed\n");
        return RCLERR_ALLOC_FAILED;
    }

    keyLen = comlib_strGetLen(key);
    if(keyLen == 0){
        return RCLERR_INVALID_KEY_LEN;
    }
    hdr->key = comlib_memMalloc(keyLen+1);

    valLen = comlib_strGetLen(val);
    if(valLen == 0){
        return RCLERR_INVALID_VAL_LEN;
    }
    hdr->val = comlib_memMalloc(valLen+1);

    comlib_strCpy(hdr->key, key);
    comlib_strCpy(hdr->val, val);

    hdr->lnkNode.data = hdr;

    ret = comlib_lnkLstInsertTail(&hdrLst->hdrLl, &hdr->lnkNode);
    if(ret != RC_OK){
        comlib_memFree(hdr->key);
        comlib_memFree(hdr->val);
        comlib_memFree(hdr);
        return RCLERR_LNKLST_INSERT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiDstryHdrLst(RcllibHdrLst *hdrLst)
{
    ComlibLnkNode *lnkNode = NULL;
    RcllibHdr *hdr = NULL;

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&hdrLst->hdrLl);
        if(lnkNode == NULL){
            break;
        }

        hdr = lnkNode->data;

        comlib_memFree(hdr->key);
        comlib_memFree(hdr->val);

        comlib_memFree(hdr);
    }/* end of while(1) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiGetStaCode(RcllibCb *rcllibCb, UINT *rt_staCode)
{
    RcllibIntCb *intRcllibCb = NULL;
    ULONG rspCode = 0;

    intRcllibCb = (RcllibIntCb*)rcllibCb->main;

    curl_easy_getinfo(intRcllibCb->hdlr, CURLINFO_RESPONSE_CODE, &rspCode);

    (*rt_staCode) = (UINT)rspCode;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiCpyRspDatIntoDyn(RcllibCb *rcllibCb, CHAR **rt_dat, UINT *rt_datLen)
{
    RcllibIntCb *intRcllibCb = NULL;
    UINT datLen = 0;
    CHAR *dat = NULL;

    intRcllibCb = (RcllibIntCb*)rcllibCb->main;

    if(intRcllibCb->rspMsg == NULL){
        (*rt_datLen) = 0;
        (*rt_dat) = NULL;

        return RCLERR_DAT_NOT_EXIST;
    }

    comlib_msgGetMsgLen(&intRcllibCb->rspMsg, &datLen);

    comlib_msgCpyAllMsgIntoDynBuf(&intRcllibCb->rspMsg, &dat, &datLen);

    (*rt_dat) = dat;
    (*rt_datLen) = datLen;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rcllib_apiCpyRspDatIntoFix(RcllibCb *rcllibCb, CHAR *rt_dat, UINT maxDatLen, UINT *rt_datLen)
{
    SINT ret = 0;
    UINT datLen = 0;
    RcllibIntCb *intRcllibCb = NULL;
    
    intRcllibCb = (RcllibIntCb*)rcllibCb->main;

    if(intRcllibCb->rspMsg == NULL){
        (*rt_datLen) = 0;

        return RCLERR_DAT_NOT_EXIST;
    }

    comlib_msgGetMsgLen(&intRcllibCb->rspMsg, &datLen);

    if(datLen >= maxDatLen){
        RCL_LOG(RCL_ERR,"Buffer too small(dat=%d, max=%d)\n",datLen, maxDatLen);
        return RCLERR_BUF_TOO_SMALL;
    }

    ret = comlib_msgCpyAllMsgIntoFixBuf(&intRcllibCb->rspMsg, rt_dat, maxDatLen, &datLen);
    if(ret != RC_OK){
        RCL_LOG(RCL_ERR,"Message copy failed(ret=%d)\n",ret);
        return RCLERR_CPY_FAILED;
    }

    (*rt_datLen) = datLen;

    return RC_OK;
}

