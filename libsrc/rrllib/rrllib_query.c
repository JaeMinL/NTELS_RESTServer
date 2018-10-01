#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PUBLIC RT_RESULT rrllib_queryAdd(RrllibResMthod*resMthod, BOOL mandFlg, CONST CHAR *name, UINT nameLen, UINT maxCnt)
{
	SINT ret = 0;
	RrllibQuery *query = NULL;

	if((nameLen == 0) || 
	   (nameLen > RRL_QUERY_NAME_LEN)){
		RRL_LOG(RRL_ERR,"Invalid query name length(len=%d, max=%d)\n", nameLen, RRL_QUERY_NAME_LEN);
		return RRLERR_INVALID_QUERY_NAME_LEN;
	}

	/* find */
	ret = rrllib_queryFind(resMthod, name, nameLen, NULL);
	if(ret == RC_OK){
		RRL_LOG(RRL_ERR,"Query already exist\n");
		return RRLERR_QUERY_ALREADY_EXIST;
	}

	query = comlib_memMalloc(sizeof(RrllibQuery));

	comlib_strNCpy(query->name, (CHAR*)name, nameLen);
	query->nameLen = nameLen;

	if((mandFlg != RC_TRUE) &&
	   (mandFlg != RC_FALSE)){
		RRL_LOG(RRL_ERR,"Invalid mand flag(%d)\n",mandFlg);
		return RRLERR_INVALID_MAND_FLAG;

	}
	if(mandFlg == RC_TRUE){
		query->mandFlg = RC_TRUE;
		resMthod->mandCnt++;
	}
	else {
		query->mandFlg = RC_FALSE;
	}

	if(maxCnt == 0){
		query->maxCnt = 1;
	}
	else {
		query->maxCnt = maxCnt;
	}

	query->lnkNode.data = query;

	ret = comlib_lnkLstInsertTail(&resMthod->queryLL, &query->lnkNode);
	if(ret != RC_OK){
		RRL_LOG(RRL_ERR,"Query insert faeild(ret=%d)\n",ret);
		return RRLERR_QUERY_INSERT_FAILED;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_queryFind(RrllibResMthod *resMthod, CONST CHAR *name, UINT nameLen, RrllibQuery **rt_query)
{
    SINT ret = 0;
    RrllibCb *rrlCb = NULL;
    RrllibQuery *query = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&resMthod->queryLL, lnkNode);
    if(lnkNode == NULL){
        return RC_NOK;
    }

    rrlCb = resMthod->root;

    while(1){
        query = lnkNode->data;

        if(query->nameLen == nameLen){
            if(rrlCb->caseFlg == RC_FALSE){
                ret = comlib_strCaseNCmp(query->name, name, nameLen);
            }
            else {
                ret = comlib_strNCmp(query->name, name, nameLen);
            }
            if(ret == 0){
                if(rt_query != NULL){
                    *rt_query = query;
                }
                return RC_OK;
            }
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            return RC_NOK;
        }

    }/* end of while(1) */

    return RC_NOK;
}

FT_PUBLIC RT_RESULT rrllib_queryPrnt(RrllibQuery *query)
{
    if(query->mandFlg == RC_TRUE){
        rrllib_globDispPrnt(" {%.*s}", query->nameLen, query->name);
    }
    else {
        rrllib_globDispPrnt(" [%.*s]", query->nameLen, query->name);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_queryDstry(RrllibQuery *query)
{
    query->maxCnt = 0;
    query->mandFlg = RC_FALSE;
    query->nameLen = 0;

    return RC_OK;
}

