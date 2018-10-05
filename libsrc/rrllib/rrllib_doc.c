#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PUBLIC RT_RESULT rrllib_docGetNewDoc(RrllibDoc **rt_doc)
{
    SINT ret = 0;
    RrllibDoc *doc = NULL;

    doc = comlib_memMalloc(sizeof(RrllibDoc));

    doc->mthod = RRL_MTHOD_NONE;
    doc->mandCnt = 0;
    doc->curPtr = NULL;

    doc->usrArg = NULL;

    ret = comlib_lnkLstInit(&doc->argLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Document argument list init failed(ret=%d)\n",ret);
        return RRLERR_MTHOD_ARG_LNKLST_INIT_FAEILD;
    }

    *rt_doc = doc;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docArgDstry(RrllibDocArg *docArg)
{
    ComlibLnkNode *lnkNode = NULL;
    RrllibDocArgVal *argVal = NULL;

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&docArg->valLL);
        if(lnkNode != NULL){
            argVal = lnkNode->data;

            comlib_memFree(argVal);
        }
        else {
            break;
        }
    }/* end of while(1) */

    comlib_memFree(docArg);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docDstry(RrllibDoc *doc)
{
    SINT ret = 0;
    RrllibDocArg *docArg = NULL;
    ComlibLnkNode *lnkNode = NULL;

    while(1){
        lnkNode =  comlib_lnkLstGetFirst(&doc->argLL);
        if(lnkNode != NULL){
            docArg = lnkNode->data;

            ret = rrllib_docArgDstry(docArg);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Arguemnt destory failed(ret=%d)\n",ret);
            }
        }
        else {
            break;
        }
    }/* end of while(1) */

    comlib_memFree(doc);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docFindArg(RrllibDoc *doc, CHAR *name, UINT nameLen, RrllibDocArg **rt_docArg)
{
    SINT ret = 0;
    RrllibDocArg *docArg = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&doc->argLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }

    while(1){
        docArg = lnkNode->data;

        if(docArg->nameLen != nameLen){
            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
            continue;
        }

        ret = comlib_strCaseNCmp(docArg->name, name, nameLen);
        if(ret == 0){
            *rt_docArg = docArg;
            return RC_OK;
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            break;
        }
    }/* end of while(1) */

    return RC_NOK;
}

FT_PUBLIC RT_RESULT rrllib_docGetFirstArg(RrllibDoc *doc, RrllibDocArg **rt_docArg)
{
    RrllibDocArg *docArg = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&doc->argLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }

    doc->curPtr = lnkNode;

    docArg = lnkNode->data;

    (*rt_docArg) = docArg;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docGetNxtArg(RrllibDoc *doc, RrllibDocArg **rt_docArg)
{
    RrllibDocArg *docArg = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(doc->curPtr == NULL){
        COM_GET_LNKLST_FIRST(&doc->argLL, lnkNode);
        if(lnkNode == NULL){
            return RC_NOK;
        }
    }
    else{
        COM_GET_NEXT_NODE(doc->curPtr);
        if(doc->curPtr == NULL){
            return RC_NOK;
        }

        lnkNode = doc->curPtr;
    }

    doc->curPtr = lnkNode;

    docArg = lnkNode->data;

    (*rt_docArg) = docArg;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docAddArg(RrllibDoc *doc, CHAR *name, UINT nameLen, RrllibDocArg **rt_docArg)
{
    SINT ret = 0;
    RrllibDocArg *docArg = NULL;

    if((nameLen == 0) ||
       (nameLen > RRL_DOC_ARG_NAME_LEN)){
        RRL_LOG(RRL_ERR,"Invalid Argument name length(len=%d, max=%d)\n", nameLen, RRL_DOC_ARG_NAME_LEN);
        return RRLERR_INVALID_DOC_ARG_NAME_LEN;
    }

    docArg = comlib_memMalloc(sizeof(RrllibDocArg));

    docArg->curPtr = NULL;
    comlib_strNCpy(docArg->name, name, nameLen);
    docArg->nameLen = nameLen;

    ret = comlib_lnkLstInit(&docArg->valLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Arguemnt value list init failed(ret=%d)\n",ret);
        comlib_memFree(docArg);
        return RRLERR_DOC_ARG_VAL_LNKLST_INIT_FAILED;
    }

    docArg->lnkNode.data = docArg;

    ret = comlib_lnkLstInsertTail(&doc->argLL, &docArg->lnkNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Arguemnt insert faeild(ret=%d)\n",ret);
        comlib_memFree(docArg);
        return RRLERR_DOC_ARG_INSERT_FAILED;
    }

    if(rt_docArg != NULL){
        *rt_docArg = docArg;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docAddArgVal(RrllibDocArg *docArg, CONST CHAR *val, UINT valLen)
{
    SINT ret = 0;
    RrllibDocArgVal *docArgVal = NULL;

    if((valLen >= RRL_DOC_ARG_VAL_LEN) ||
       (valLen == 0)){
        RRL_LOG(RRL_ERR,"Invalid value length(len=%d, max=%d)\n",valLen, RRL_DOC_ARG_VAL_LEN);
        return RRLERR_DOC_ARG_VAL_INVALID_LEN;
    }

    docArgVal = comlib_memMalloc(sizeof(RrllibDocArgVal));

    comlib_strNCpy(docArgVal->val, (CHAR*)val, valLen);
    docArgVal->val[valLen] = '\0';
    docArgVal->valLen = valLen;
    
    docArgVal->lnkNode.data = docArgVal;

    ret = comlib_lnkLstInsertTail(&docArg->valLL, &docArgVal->lnkNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Value insert failed(ret=%d)\n",ret);
        comlib_memFree(docArgVal);
        return RRLERR_DOC_ARG_VAL_INSERT_FAEILD;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docGetFirstVal(RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen)
{
    RrllibDocArgVal *docArgVal = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&docArg->valLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }

    docArgVal = lnkNode->data;

    (*rt_val) = docArgVal->val;
    (*rt_valLen) = docArgVal->valLen;

    docArg->curPtr = lnkNode;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docGetNxtVal(RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen)
{
    RrllibDocArgVal *docArgVal = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(docArg->curPtr == NULL){
    COM_GET_LNKLST_FIRST(&docArg->valLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }
    }
    else {
        COM_GET_NEXT_NODE(docArg->curPtr);
        if(docArg->curPtr == NULL){
            return RC_NOK;
        }

        lnkNode = docArg->curPtr;
    }

    docArgVal = lnkNode->data;

    (*rt_val) = docArgVal->val;
    (*rt_valLen) = docArgVal->valLen;

    docArg->curPtr = lnkNode;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docArgPrnt(RrllibDocArg *docArg)
{
    RrllibDocArgVal *argVal = NULL;
    ComlibLnkNode *lnkNode = NULL;

    rrllib_globDispPrnt("  ARG : NAME   = [%.*s]\n", docArg->nameLen, docArg->name);
    rrllib_globDispPrnt("  ARG : ARGCNT = [%d]\n",docArg->valLL.nodeCnt);

    COM_GET_LNKLST_FIRST(&docArg->valLL, lnkNode);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        argVal = lnkNode->data;

        rrllib_globDispPrnt("  ARG :   ARGVAL = [%.*s]\n", argVal->valLen, argVal->val);

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            return RC_OK;
        }
    }/* end of while(1) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_docPrnt(RrllibDoc *doc)
{
    SINT ret = 0;
    UINT argAddr  = 0;
    ComlibLnkNode *lnkNode = NULL;
    RrllibDocArg *docArg = NULL;
    CHAR *strMthod = NULL;

    switch(doc->mthod){
        case RRL_MTHOD_NONE: strMthod = "NONE";    break;
        case RRL_MTHOD_GET:  strMthod = "GET";     break;
        case RRL_MTHOD_POST: strMthod = "POST";    break;
        case RRL_MTHOD_DEL:  strMthod = "DEL";     break;
        case RRL_MTHOD_PUT:  strMthod = "PUT";     break;
        default:             strMthod = "UNKNOWN"; break;
    }/* end of switch(doc->mthod) */

    rrllib_globDispPrnt("METHOD=[%s]\n", strMthod);

    comlib_memMemcpy(&argAddr, &doc->usrArg, sizeof(UINT));
    rrllib_globDispPrnt("USRARG=[0x%x]\n", argAddr);
    rrllib_globDispPrnt("ARGCNT=[%d]\n", doc->argLL.nodeCnt);

    COM_GET_LNKLST_FIRST(&doc->argLL, lnkNode);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        docArg = lnkNode->data;

        ret = rrllib_docArgPrnt(docArg);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Arguemnt print failed(ret=%d)\n",ret);
            return RC_OK;
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            return RC_OK;
        }
    }/* end of while(1) */

    return RC_OK;
}

