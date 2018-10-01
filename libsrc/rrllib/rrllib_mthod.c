#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PUBLIC RT_RESULT rrllib_mthodAdd(RrllibResPath *resPath, UINT mthod, VOID *arg, RrllibResMthod **rt_resMthod)
{
    SINT ret = 0;
    RrllibResMthod *resMthod = NULL;

    if((mthod !=  RRL_MTHOD_GET) && (mthod !=  RRL_MTHOD_POST) &&
       (mthod !=  RRL_MTHOD_DEL) && (mthod !=  RRL_MTHOD_PUT)){
        RRL_LOG(RRL_ERR,"Invlaid method(%d)\n",mthod);
        return RRLERR_INVALID_MTHOD;
    }

    resMthod = comlib_memMalloc(sizeof(RrllibResMthod));

    resMthod->mthod = mthod;
    resMthod->arg = arg;
    resMthod->mandCnt = 0;
    resMthod->root = resPath->root;

    ret = comlib_lnkLstInit(&resMthod->queryLL, ~0);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Query lnklst init failed(ret=%d)\n",ret);
        return RRLERR_QUERY_LNKLST_INIT_FAILED;
    }

    resMthod->lnkNode.data = resMthod;

    ret = comlib_lnkLstInsertTail(&resPath->mthodLL, &resMthod->lnkNode);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"method insert failed(ret=%d)\n",ret);
        comlib_memFree(resMthod);

        return RRLERR_MTHOD_INSERT_FAEILD;
    }

    if(rt_resMthod != NULL){
        (*rt_resMthod) = resMthod;
    }

    return RC_OK;
}

FT_PUBLIC CHAR* rrllib_mthodGetStr(UINT mthod)
{
    switch(mthod){
        case  RRL_MTHOD_NONE: return "NONE"; break;
        case  RRL_MTHOD_GET:  return "GET";  break;
        case  RRL_MTHOD_POST: return "POST"; break;
        case  RRL_MTHOD_DEL:  return "DEL";  break;
        case  RRL_MTHOD_PUT:  return "PUT";  break;
        default:              return "NONE"; break;
    }
}

FT_PUBLIC RT_RESULT rrllib_mthodPrntMthod(RrllibResMthod *resMthod, UINT depth, RrllibPrntUsrArgFunc func)
{
    SINT ret = 0;
    CHAR *strMthod = NULL;
    RrllibQuery *query = NULL;
    ComlibLnkNode *lnkNode = NULL;

    strMthod = rrllib_mthodGetStr(resMthod->mthod);

    rrllib_globDispPrnt("%*s> %s :",depth,"",strMthod);

    COM_GET_LNKLST_FIRST(&resMthod->queryLL, lnkNode);
    if(lnkNode != NULL){
        //rrllib_globDispPrnt("\n");
        //return RC_OK;
        while(1){
            query = lnkNode->data;

            ret = rrllib_queryPrnt(query);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Query print failed(ret=%d)\n",ret);
            }

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }

    if(resMthod->arg != NULL){
        if(func != NULL){
            rrllib_globDispPrnt("\n");
            ret = func(resMthod->arg, depth+1);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"User arguemnt print failed(ret=%d)\n",ret);
            }
        }
    }

    rrllib_globDispPrnt("\n");

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_mthodDstry(RrllibResMthod *resMthod)
{
    SINT ret = 0;
    RrllibQuery *query = NULL;
    ComlibLnkNode *lnkNode = NULL;

    while(1){
        lnkNode = comlib_lnkLstGetFirst(&resMthod->queryLL);
        if(lnkNode == NULL){
            break;
        }

        query = lnkNode->data;

        ret = rrllib_queryDstry(query);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Query destory failed(ret=%d, query=%*.s)\n",ret, query->nameLen, query->name);
        }

        comlib_memFree(query);
    }/* end of while(1) */

    return RC_OK;
}
