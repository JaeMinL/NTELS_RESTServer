#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PRIVATE RT_RESULT path_addPath            (RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, 
                                              RrllibResPath **rt_resPath);
FT_PRIVATE RT_RESULT path_chkDupDynPathChk   (RrllibResPath *resPath, CONST CHAR *name, UINT nameLen);

FT_PRIVATE RT_RESULT path_chkDupDynPathChk(RrllibResPath *resPath, CONST CHAR *name, UINT nameLen)
{
    SINT ret = 0;
    RrllibResPathLst *chkPathLst = NULL;
    RrllibResPath *chkPath = NULL;

    /* first */
    chkPathLst = resPath->own;
    chkPath = resPath;

    while(1){
        if(chkPathLst->pathType == RRL_PATH_TYPE_DYN){
            if(chkPath->nameLen == nameLen){
                ret = comlib_strCaseNCmp(chkPath->name, name, nameLen);
                if(ret == 0){
                    return RC_OK;
                }
            }
        }

        /* get next */
        chkPath = chkPathLst->own;
        if(chkPath == NULL){
            break;
        }

        chkPathLst = chkPath->own;
    }/* end of while(1) */

    return RC_NOK;
}

FT_PRIVATE RT_RESULT path_addPath(RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, RrllibResPath **rt_resPath)
{
    SINT ret = 0;
    RrllibCb *rrlCb = NULL;
    RrllibResPath *resPath = NULL;

    if(nameLen > RRL_PATH_NAME_LEN){
        RRL_LOG(RRL_ERR,"Invalid path name length(len=%d, max=%d)\n",nameLen, RRL_PATH_NAME_LEN);
        return RRLERR_INVALID_PATH_NAME_LEN;
    }

    rrlCb = resPathLst->root;

    /* make resource path */
    resPath = comlib_memMalloc(sizeof(RrllibResPath));

    comlib_memMemcpy(resPath->name, (VOID*)name, nameLen);

    resPath->nameLen = nameLen;

    if(rrlCb->caseFlg == RC_FALSE){
        ret = comlib_strChgStrToUpper(name, nameLen, resPath->key, RRL_PATH_NAME_LEN);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Case change failed(ret=%d)\n",ret);
            comlib_memFree(resPath);
            return RRLERR_CASE_CHANGE_FAILED;
        }
    }
    else {
        comlib_strNCpy(resPath->key, (CHAR*)name, nameLen);
    }

    resPath->keyLen = nameLen;

    resPath->root = resPathLst->root;
    resPath->nxtPathLst = NULL;
    resPath->own = resPathLst;

    ret = comlib_lnkLstInit(&resPath->mthodLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Method lnklst init failed(ret=%d)\n",ret);
        return RRLERR_METHOD_LNKLST_INIT_FAILED;
    }

    resPath->lnkNode.data = resPath;

    ret = comlib_lnkLstInsertTail(&resPathLst->resPathLL, &resPath->lnkNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path insert failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_INSERT_FAEILD;
    }

    resPath->hNode.data = resPath;

    resPath->hNode.key.key = resPath->key;
    resPath->hNode.key.keyLen = resPath->keyLen;

    ret = comlib_hashTblInsertHashNode(&resPathLst->resPathHT, &resPath->hNode.key, &resPath->hNode); 
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"failed to insert resource path into hash table(ret=%d,name=%.*s)\n",ret, nameLen, name);
        return RRLEER_RES_PATH_HNODE_INSERT_FAEILD;
    }

    if(rt_resPath != NULL){
        *rt_resPath = resPath;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathChkDynPath(RrllibResPathLst *resPathLst, CHAR *name, UINT nameLen, RrllibResPath **rt_resPath)
{
    SINT ret = RC_OK;
    RrllibResPath *chkPath = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(resPathLst->pathType != RRL_PATH_TYPE_DYN){
        RRL_LOG(RRL_ERR,"next path list already exist(tkn=%.*s)\n", nameLen, name);
        return RRLERR_PATH_LST_ALREADY_EXIST;
    }

    COM_GET_LNKLST_FIRST(&resPathLst->resPathLL, lnkNode);
    if(lnkNode == NULL){
        RRL_LOG(RRL_ERR,"next dyn path not exist(tkn=%.*s)\n", nameLen, name);
        return RRLERR_PATH_LST_ALREADY_EXIST;

    }

    chkPath = lnkNode->data;

    ret = comlib_strNCmp(chkPath->name, name, nameLen);
    if(ret != 0){
        RRL_LOG(RRL_ERR,"Next path already exist\n");
        return RRLERR_PATH_LST_ALREADY_EXIST;
    }

    if(rt_resPath != NULL){
        (*rt_resPath) = chkPath;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathAddFirstFixPathLst(RrllibCb *rrlCb, RrllibResPathLst **rt_resPathLst)
{
    SINT ret = 0;

    if(rrlCb->resPathLst != NULL){
        RRL_LOG(RRL_ERR,"First path already exist\n");
        return RRLERR_PATH_LST_ALREADY_EXIST;

    }

    rrlCb->resPathLst = comlib_memMalloc(sizeof(RrllibResPathLst));

    rrlCb->resPathLst->pathType = RRL_PATH_TYPE_FIX;
    rrlCb->resPathLst->own = NULL;
    rrlCb->resPathLst->root = rrlCb;

    ret = comlib_hashTblInit(&rrlCb->resPathLst->resPathHT, RRL_RES_PATH_HASH_ENTRY_CNT, RC_FALSE,COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path hash table init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_HASH_TBL_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&rrlCb->resPathLst->resPathLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path lnklst init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_LNKLST_INIT_FAILED;
    }

    if(rt_resPathLst != NULL){
        *rt_resPathLst = rrlCb->resPathLst;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathAddFirstDynPath(RrllibCb *rrlCb, CONST CHAR *name, UINT nameLen, RrllibResPath **rt_resPath)
{
    SINT ret = 0;
    RrllibResPath *newResPath = NULL;

    if(rrlCb->resPathLst != NULL){
        RRL_LOG(RRL_ERR,"First path already exist\n");
        return RRLERR_PATH_LST_ALREADY_EXIST;

    }

    rrlCb->resPathLst = comlib_memMalloc(sizeof(RrllibResPathLst));

    rrlCb->resPathLst->pathType = RRL_PATH_TYPE_DYN;
    rrlCb->resPathLst->own = NULL;
    rrlCb->resPathLst->root = rrlCb;

    ret = comlib_lnkLstInit(&rrlCb->resPathLst->resPathLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path lnklst init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_LNKLST_INIT_FAILED;
    }

    ret = comlib_hashTblInit(&rrlCb->resPathLst->resPathHT, RRL_RES_PATH_HASH_ENTRY_CNT, RC_FALSE,COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path hash table init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_HASH_TBL_INIT_FAILED;
    }

    ret = path_addPath(rrlCb->resPathLst, name, nameLen, &newResPath);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Path add failed(ret=%d)\n",ret);
        return ret;
    }

    if(rt_resPath != NULL){
        *rt_resPath = newResPath;
    }

    return RC_OK;
}


FT_PUBLIC RT_RESULT rrllib_pathAddNxtFixPathLst(RrllibResPath *resPath, RrllibResPathLst **rt_resPathLst)
{
    SINT ret = 0;
    RrllibResPathLst *resPathLst = NULL;

    if(resPath->nxtPathLst != NULL){
        RRL_LOG(RRL_ERR,"Next path already exist\n");
        return RRLERR_PATH_LST_ALREADY_EXIST;
    }

    resPathLst = comlib_memMalloc(sizeof(RrllibResPathLst));

    resPathLst->pathType = RRL_PATH_TYPE_FIX;
    resPathLst->own = resPath;
    resPathLst->root = resPath->root;

    ret = comlib_lnkLstInit(&resPathLst->resPathLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path lnklst init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_LNKLST_INIT_FAILED;
    }

    ret = comlib_hashTblInit(&resPathLst->resPathHT, RRL_RES_PATH_HASH_ENTRY_CNT, RC_FALSE,COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path hash table init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_HASH_TBL_INIT_FAILED;
    }

    resPath->nxtPathLst = resPathLst;

    if(rt_resPathLst != NULL){
        *rt_resPathLst = resPathLst;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathAddNxtDynPath(RrllibResPath *resPath, CONST CHAR *name, UINT nameLen, RrllibResPath **rt_resPath)
{
    SINT ret = 0;
    RrllibResPath *newResPath = NULL;
    RrllibResPathLst *resPathLst = NULL;

    if(resPath->nxtPathLst != NULL){
        RRL_LOG(RRL_ERR,"Next path already exist\n");
        return RRLERR_PATH_LST_ALREADY_EXIST;
    }

    ret = path_chkDupDynPathChk(resPath, name, nameLen);
    if(ret == RC_OK){
        RRL_LOG(RRL_ERR,"Duplicat dynamic path already exist(ret=%d, name=%.*s)\n",ret, nameLen, name);
        return RRLERR_DYN_PATH_ALREADY_EXIST;
    }

    resPathLst = comlib_memMalloc(sizeof(RrllibResPathLst));

    resPathLst->pathType = RRL_PATH_TYPE_DYN;
    resPathLst->own = resPath;
    resPathLst->root = resPath->root;

    ret = comlib_lnkLstInit(&resPathLst->resPathLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path lnklst init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_LNKLST_INIT_FAILED;
    }

    resPath->nxtPathLst = resPathLst;

    ret = comlib_hashTblInit(&resPathLst->resPathHT, RRL_RES_PATH_HASH_ENTRY_CNT, RC_FALSE,COM_HASH_TYPE_STRING, NULL);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Resource path hash table init failed(ret=%d)\n",ret);
        return RRLERR_RES_PATH_HASH_TBL_INIT_FAILED;
    }

    ret = path_addPath(resPathLst, name, nameLen, &newResPath);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Path add failed(ret=%d)\n",ret);
        return ret;
    }

    if(rt_resPath != NULL){
        *rt_resPath = newResPath;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathAddFixPath(RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, 
                                          RrllibResPath **rt_resPath)
{
    SINT ret = 0;
    RrllibResPath *resPath = NULL;

    if(resPathLst->pathType != RRL_PATH_TYPE_FIX){
        RRL_LOG(RRL_ERR,"Invalid Path type(type=%d)\n",resPathLst->pathType);
        return RRLERR_INVALID_PATH_TYPE;
    }

    ret = rrllib_pathFindFixPath(resPathLst, name, nameLen, NULL);
    if(ret == RC_OK){
        RRL_LOG(RRL_ERR,"Path already exist(name=%.*s)\n",nameLen, name);
        return RRLERR_PATH_ALREADY_EXIST;
    }

    ret = path_addPath(resPathLst, name, nameLen, &resPath);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Path add failed(ret=%d)\n",ret);
        return ret;
    }

    if(rt_resPath != NULL){
        *rt_resPath = resPath;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathFindFixPath(RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, 
                                           RrllibResPath **rt_resPath)
{
    SINT ret = 0;
    RrllibCb *rrlCb = NULL;
    ComlibHashKey hKey;
    ComlibHashNode *hNode = NULL;

    rrlCb = resPathLst->root;

    if(rrlCb->caseFlg == RC_TRUE){
        hKey.key = (VOID*)name;
        hKey.keyLen = nameLen;
    }
    else {
        hKey.key = comlib_memMalloc(nameLen);

        comlib_strChgStrToUpper(name, nameLen, hKey.key, nameLen);
        hKey.keyLen = nameLen;
    }

    ret = comlib_hashTblFindHashNode(&resPathLst->resPathHT, &hKey, 0, &hNode);
    if(ret != RC_OK){
        if(rrlCb->caseFlg == RC_FALSE){
            comlib_memFree(hKey.key);
        }
        return RRLERR_NOT_EXIST;
    }

    if(rrlCb->caseFlg == RC_FALSE){
        comlib_memFree(hKey.key);
    }
    *rt_resPath = hNode->data;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathPrntPath(RrllibResPath *resPath, UINT pathType, UINT depth, RrllibPrntUsrArgFunc func)
{
    SINT ret = RC_OK;
    RrllibResMthod *resMthod = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(pathType == RRL_PATH_TYPE_DYN){
        rrllib_globDispPrnt("%*s+{%.*s}\n",depth, "", resPath->nameLen, resPath->name);
    }
    else {
        rrllib_globDispPrnt("%*s+[%.*s]\n",depth, "", resPath->nameLen, resPath->name);
    }

    COM_GET_LNKLST_FIRST(&resPath->mthodLL, lnkNode);
    if(lnkNode != NULL){
        while(1){
            resMthod = lnkNode->data;

            ret = rrllib_mthodPrntMthod(resMthod, depth+1, func);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Method print failed(ret=%d)\n",ret);
                return RC_NOK;
            }

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }

        rrllib_globDispPrnt("\n");
    }/* end of if(lnkNode != NULL) */

    if(resPath->nxtPathLst != NULL){
        ret = rrllib_pathPrntPathLst(resPath->nxtPathLst, depth+3, func);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Path list print failed(ret=%d)\n",ret);
            return RC_NOK;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathPrntPathLst(RrllibResPathLst *resPathLst, UINT depth, RrllibPrntUsrArgFunc func)
{
    SINT ret = RC_OK;
    RrllibResPath *resPath = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&resPathLst->resPathLL, lnkNode);
    if(lnkNode == NULL){
        return RC_OK;
    }

    while(1){
        resPath = lnkNode->data;

        ret = rrllib_pathPrntPath(resPath, resPathLst->pathType, depth, func);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Path print failed(ret=%d)\n",ret);
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            break;
        }
    }/* end of while(1) */
    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathDstryPath(RrllibResPathLst *resPathLst, RrllibResPath *resPath)
{
    SINT ret = 0;
    RrllibResMthod *resMthod = NULL;
    ComlibLnkNode *lnkNode = NULL;

    /* destory hash table */
    ret = comlib_hashTblDelHashNode(&resPathLst->resPathHT, &resPath->hNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Hash table delete faeild(ret=%d)\n",ret);
    }

    /* destory linked list */
    ret = comlib_lnkLstDel(&resPathLst->resPathLL, &resPath->lnkNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Linked list delete failed(ret=%d)\n",ret);
    }

    /* mthod dstory */
    while(1){
        lnkNode = comlib_lnkLstGetFirst(&resPath->mthodLL);
        if(lnkNode == NULL){
            break;
        }

        resMthod = lnkNode->data;

        ret = rrllib_mthodDstry(resMthod);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Method destory failed(ret=%d, mthod=%s)\n",ret, rrllib_mthodGetStr(resMthod->mthod));
        }

        comlib_memFree(resMthod);
    }/* end of while(1) */

    if(resPath->nxtPathLst != NULL){
        ret = rrllib_pathDstryPathLst(resPath->nxtPathLst);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Next path dstroy failed(ret=%d, name=%*.s)\n", ret, resPath->nameLen, resPath->name);
            return ret;
        }
    }

    comlib_memFree(resPath);

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_pathDstryPathLst(RrllibResPathLst *resPathLst)
{
    SINT ret = 0;
    RrllibResPath *resPath = NULL;
    ComlibLnkNode *lnkNode = NULL;

    while(1){
        COM_GET_LNKLST_FIRST(&resPathLst->resPathLL, lnkNode);
        if(lnkNode == NULL){
            comlib_memFree(resPathLst);
            return RC_OK;
        }

        resPath = lnkNode->data;

        ret = rrllib_pathDstryPath(resPathLst, resPath);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Path destory failed(ret=%d)\n",ret);
        }
    }/* end of while(1) */

    comlib_memFree(resPathLst);

    return RC_OK;
}

