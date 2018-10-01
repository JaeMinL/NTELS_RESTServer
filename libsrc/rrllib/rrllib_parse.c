#include <uriparser/Uri.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

FT_PRIVATE RT_RESULT parse_ChkChar        (CONST CHAR data);
FT_PRIVATE CHAR*     parse_skipSpace      (CHAR *dat, UINT datLen);
FT_PRIVATE CHAR*     parse_getNum         (CHAR *dat, UINT datLen);
FT_PRIVATE CHAR*     parse_getStr         (CHAR *dat, UINT datLen);
FT_PRIVATE RT_RESULT parse_kvpToDoc       (CONST CHAR *key, UINT keyLen, CONST CHAR *val, UINT valLen, VOID *arg);
FT_PRIVATE RT_RESULT parse_kvp            (CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg, 
                                           CHAR **rt_nxtStTkn); /* key-value pair */
FT_PRIVATE RT_RESULT parse_uri            (RrllibCb *rrlCb, UriUriA *parseDat, UINT mthod, 
                                           RrllibDoc **rt_doc, RrllibResMthod **rt_mthod);
FT_PRIVATE RT_RESULT parse_query          (CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg);
FT_PRIVATE RT_RESULT parse_resCfgGetTkn   (CONST CHAR *dat, UINT datLen, CONST CHAR endTkn, CHAR **rt_endTkn);
FT_PRIVATE RT_RESULT parse_queryCfg       (RrllibResMthod *resMthod, CONST CHAR *query, UINT queryLen);
FT_PRIVATE RT_RESULT parse_resPathCfg     (VOID *cb/*control block */, UINT firstFlg,  CONST CHAR *res, UINT resLen, 
                                           RrllibResPath **rt_resPath, CHAR **rt_endTkn);
FT_PRIVATE RT_RESULT parse_queryParmCfg   (RrllibResMthod *resMthod, CONST CHAR *query, UINT queryLen, BOOL mandFlg, 
                                           CHAR **rt_endTkn);

FT_PRIVATE RT_RESULT parse_ChkChar(CONST CHAR data)
{
    if(data >= 48 &&  data <= 57){ /* 0 ~ 9 */
        return RRL_TKN_TYPE_DIGIT_CHAR;
    }
    else if((data >=  65 &&  data <= 90) || /* A ~ Z */
            (data >= 97 && data<= 122)){ /* a ~ z */
        return RRL_TKN_TYPE_ALPHA_CHAR;
    }
    else if((data == '$') || (data == '-') ||
            (data == '_') || (data == '@') || 
            (data == '.') || (data == '&')){
        return RRL_TKN_TYPE_SAFE_CHAR;
    }
    else if((data == '!') || (data == '*') || 
            (data == '"') || (data == '\'') || 
            (data == '(') || (data == ')') ||
            (data == ',')){
        return RRL_TKN_TYPE_EXTRA_CHAR;
    }
    else if( (data == '=') || (data == ';') ||
            (data == '/') || (data == '#') ||
            (data == '?') || (data == ':') ||
            (data == ' ')){
        return RRL_TKN_TYPE_RESVRD_CHAR;
    }
    else if( (data == '{') || (data == '}') ||
            (data == '[') || (data == ']')){
        return RRL_TKN_TYPE_CFG_TKN_CHAR;
    }

    return RC_NOK;
}

FT_PRIVATE CHAR* parse_skipSpace(CHAR *dat, UINT datLen)
{
    UINT i = 0;
    CHAR *cur = NULL;

    cur = dat;

    for(i=0;i<datLen;i++){
        if((*cur) == ' '){
            cur++;
        }
        else {
            break;
        }
    }

    return cur;
}

FT_PRIVATE CHAR *parse_getNum(CHAR *dat, UINT datLen)
{
    SINT ret = 0;
    CHAR *cur = NULL;
    UINT remLen = 0;

    remLen = datLen;
    cur = (CHAR*)dat;

    while(1){
        if(remLen == 0){
            break;
        }

        ret = parse_ChkChar(*cur);
        if(ret != RRL_TKN_TYPE_DIGIT_CHAR){
            return cur;
        }

        cur++;
        remLen--;
    }/* end of while(1) */

    return cur;
}


FT_PRIVATE CHAR *parse_getStr(CHAR *dat, UINT datLen)
{
    SINT ret = 0;
    CHAR *cur = NULL;
    UINT remLen = 0;

    remLen = datLen;
    cur = (CHAR*)dat;

    while(1){
        if(remLen == 0){
            break;
        }

        ret = parse_ChkChar(*cur);
        if((ret != RRL_TKN_TYPE_ALPHA_CHAR) &&
           (ret != RRL_TKN_TYPE_DIGIT_CHAR) &&
           (ret != RRL_TKN_TYPE_SAFE_CHAR)){
            return cur;
        }

        if(ret == RRL_TKN_TYPE_SAFE_CHAR){
            if((*cur != '_') &&
               (*cur != '-')){
                return cur;
            }
        }

        cur++;
        remLen--;
    }/* end of while(1) */

    return cur;
}

FT_PRIVATE RT_RESULT parse_resCfgGetTkn(CONST CHAR *dat, UINT datLen, CONST CHAR endTkn, CHAR **rt_endTkn)
{
    SINT ret = 0;
    CHAR *cur = NULL;
    UINT remLen = 0;

    remLen = datLen;
    cur = (CHAR*)dat;

    while(1){
        if(remLen == 0){
            break;
        }

        if((*cur) == endTkn){
            *rt_endTkn = cur;
            return RC_OK;
        }

        ret = parse_ChkChar(*cur);
        if((ret != RRL_TKN_TYPE_ALPHA_CHAR) &&
           (ret != RRL_TKN_TYPE_DIGIT_CHAR) &&
           (ret != RRL_TKN_TYPE_SAFE_CHAR)){
            RRL_LOG(RRL_ERR,"Invalid Token(%c)\n",*cur);
            return RRLERR_INVALID_TOKEN;
        }

        if(ret == RRL_TKN_TYPE_SAFE_CHAR){
            if(((*cur) != '_') && 
               ((*cur) != '-')){
                RRL_LOG(RRL_ERR,"Invalid Token(%c)\n",*cur);
                return RRLERR_INVALID_TOKEN;
            }

        }

        cur++;
        remLen--;
    }/* end of while(1) */

    *rt_endTkn = cur-1;

    return RRLERR_END;
}

FT_PRIVATE RT_RESULT parse_kvpToDoc(CONST CHAR *key, UINT keyLen, CONST CHAR *val, UINT valLen, VOID *arg)
{
    SINT ret = 0;
    RrllibResMthod *resMthod = NULL;
    RrllibDocArg *docArg = NULL;
    RrllibQueryDocCfg *queryDocCfg = NULL;
    RrllibQuery *query = NULL;
    RrllibDoc *doc = NULL;

    RRL_LOG(RRL_DBG,"PARSE:KEY=%.*s, VAL=%.*s\n", keyLen, key, valLen, val);

    queryDocCfg = (RrllibQueryDocCfg*)arg;

    doc = queryDocCfg->doc;
    resMthod = queryDocCfg->resMthod;

    ret = rrllib_queryFind(resMthod, key, keyLen, &query);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Invalid Query Key(key=%.*s)\n",keyLen, key);
        return RC_NOK;
    }

    ret = rrllib_docFindArg(doc, (CHAR*)key, keyLen, &docArg);
    if(ret == RC_OK){
        if(docArg->valLL.nodeCnt >= query->maxCnt){
            RRL_LOG(RRL_ERR,"Too many query value(cnt=%d, maxCnt=%d)\n",
                    docArg->valLL.nodeCnt, query->maxCnt);
            return RC_NOK;
        }
    }
    else {
        ret = rrllib_docAddArg(doc, (CHAR*)key, keyLen, &docArg);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Document arguemnt create failed(ret=%d)\n",ret);
            return RC_NOK;
        }

        if(query->mandFlg == RC_TRUE){
            doc->mandCnt++;
        }
    }

    ret = rrllib_docAddArgVal(docArg, val, valLen);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Document arguemnt value insert failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT parse_kvp(CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg, 
                                    CHAR **rt_nxtStTkn) /* key-value pair */
{
    SINT ret = 0;
    CHAR *cur = NULL;
    CHAR *stTkn = NULL;
    CHAR *keyTkn = NULL;
    UINT keyTknLen = 0;
    UINT remLen = 0;

    remLen = queryLen;
    cur = (CHAR*)query;
    stTkn = cur;

    while(1){
        if(remLen == 0){
            break;
        }

        ret = parse_ChkChar(*cur);
        if(ret == RC_NOK){
            RRL_LOG(RRL_ERR,"Invalid token(%c)\n",*cur);
            return RRLERR_INVALID_TOKEN;
        }

        if(stTkn == NULL){
            stTkn = cur;
        }

        switch(*cur){
            case '=':
                {
                    if(keyTkn != NULL){
                        RRL_LOG(RRL_ERR,"Key already exist(key=%.*s)\n",keyTknLen, keyTkn);
                        return RRLERR_QUEYR_KEY_ALREADY_EXIST;
                    }

                    keyTkn = stTkn;
                    keyTknLen = cur - stTkn;
                    stTkn = NULL;
                }
                break;
            case '&':
                {
                    if(keyTkn == NULL){
                        RRL_LOG(RRL_ERR,"Key not exist\n");
                        return RRLERR_QUERY_KEY_NOT_EXIST;
                    }

                    ret = func(keyTkn, keyTknLen, stTkn, cur - stTkn, arg);
                    if(ret != RC_OK){
                        RRL_LOG(RRL_ERR,"Query function failed(ret=%d)\n",ret);
                        return RRLERR_QUERY_FUNC_FAILED;
                    }

                    (*rt_nxtStTkn) = cur+1;
                    return RC_OK;
                }
                break;
        }; /* end ofswitch(ret) */

        cur++;
        remLen--;
    }/* end of while(1) */

    if(keyTkn == NULL){
        RRL_LOG(RRL_ERR,"Key not exist\n");
        return RRLERR_QUERY_KEY_NOT_EXIST;
    }

    ret = func(keyTkn, keyTknLen, stTkn, cur - stTkn, arg);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Query function failed(ret=%d)\n",ret);
        return RRLERR_QUERY_FUNC_FAILED;
    }

    return RRLERR_QUERY_STR_END;
}

FT_PRIVATE RT_RESULT parse_query(CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg)
{
	SINT ret = 0;
	UINT remLen = 0;
	CHAR *stTkn = NULL;

	stTkn =  (CHAR*)query;

	if(queryLen == 0){
		return RC_OK;
	}

	remLen = queryLen;

	while(1){
		ret =  parse_kvp(stTkn, remLen, func, arg, &stTkn);
		if(ret != RC_OK){
			if(ret == RRLERR_QUERY_STR_END){
				break;
			}
			RRL_LOG(RRL_ERR,"Key value pair parsing failed(ret=%d)\n",ret);
			return ret;
		}

		remLen = queryLen - (stTkn - query);
	}/* end of while(1) */

	return RC_OK;
}

FT_PRIVATE RT_RESULT parse_queryParmCfg(RrllibResMthod *resMthod, CONST CHAR *query, UINT queryLen, BOOL mandFlg, 
                                       CHAR **rt_endTkn)
{
    SINT ret = 0;
    UINT remLen = 0;
    UINT tknLen = 0;
    UINT maxCnt = 0;
    CHAR *parm = NULL;
    UINT parmLen = 0;
    CHAR endChar = '\0';
    CHAR *endTkn = NULL;
    CHAR *cur = NULL;

    if(mandFlg == RC_TRUE){
        endChar = '}';
    }
    else {
        endChar = ']';
    }

    remLen = queryLen;

    /* get String */
    cur = parse_getStr((CHAR*)query, queryLen);

    parmLen = cur - query;
    parm = (CHAR*)query;
    remLen -= parmLen;

    if(remLen == 0){
        RRL_LOG(RRL_ERR,"Invalid token(%c)\n",*cur);
        return RRLERR_INVALID_TOKEN;
    }

    if((*cur) == ':'){
        remLen--;
        if(remLen == 0){
            RRL_LOG(RRL_ERR,"Invalid token(%c)\n",*cur);
            return RRLERR_INVALID_TOKEN;
        }
        cur++;

        endTkn = parse_getNum(cur, remLen);

        tknLen = endTkn - cur;

        if(remLen <= tknLen){
            RRL_LOG(RRL_ERR,"Invalid token length(len=%d, remLen=%d)\n", tknLen, remLen);
            return RRLERR_INVALID_TOKEN;
        }

        maxCnt = comlib_utilAtoi(cur, tknLen);

        remLen -= tknLen;
        cur += tknLen;
    }/* end of if((*cur) == ':') */

    if((*cur) == endChar){
        RRL_LOG(RRL_DBG,"PARSE:PARM=%.*s, MAXCNT=%d\n", parmLen, parm, maxCnt);
        /* add param */
        ret = rrllib_queryAdd(resMthod, mandFlg, parm, parmLen, maxCnt);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Query add failed(ret=%d)\n",ret);
            return ret;
        }
    }
    else {
        RRL_LOG(RRL_ERR,"Invalid token(%c)\n",*cur);
            return RRLERR_INVALID_TOKEN;
    }

    if(rt_endTkn != NULL){
        *rt_endTkn = cur;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT parse_queryCfg(RrllibResMthod *resMthod, CONST CHAR *query, UINT queryLen)
{
    SINT ret = RC_OK;
    UINT remLen = 0;
    BOOL mandFlg = RC_FALSE;
    CHAR *cur = NULL;
    CHAR *endTkn = NULL;
    CHAR *stTkn = NULL;

    cur = (CHAR*)query;
    remLen = queryLen;

    while(1){
        stTkn = parse_skipSpace(cur, remLen);

        remLen -= stTkn - cur;
        if(remLen == 0){
            return RC_OK;
        }

        cur = stTkn;

        /* get next tkn */
        switch(*cur){
            case '{':
            case '[':
                {
                    remLen--;
                    if(remLen == 0){
                        return RRLERR_INVALID_TOKEN;
                    }

                    stTkn = cur+1;

                    if((*cur) == '{'){
                        mandFlg = RC_TRUE;
                    }
                    else {
                        mandFlg = RC_FALSE;
                    }

                    ret = parse_queryParmCfg(resMthod, stTkn, remLen, mandFlg, &endTkn);
                    if(ret != RC_OK){
                        RRL_LOG(RRL_ERR,"Token parsing faeild(ret=%d)\n",ret);
                        return ret;
                    }

                    remLen -= endTkn-stTkn;
                    if(remLen == 0){
                        break;
                    }

                    cur = endTkn+1;
                    remLen--;
                }
                break;
            default :
                RRL_LOG(RRL_ERR,"Invalid token(%c)\n",*cur);
                return RRLERR_INVALID_TOKEN;
        }
    }/* end of while(1) */

    return RC_OK;
}

FT_PRIVATE RT_RESULT parse_resPathCfg(VOID *cb/*control block */, UINT firstFlg,  CONST CHAR *res, UINT resLen, 
                                           RrllibResPath **rt_resPath, CHAR **rt_endTkn)
{
    SINT ret = 0;
    CHAR *cur = NULL;
    UINT remLen = 0;
    CHAR *endTkn = NULL;
    RrllibResPath *newPath = NULL;
    RrllibResPath *resPath = NULL;
    RrllibResPathLst *resPathLst = NULL;
    RrllibCb *rrlCb = NULL;

    remLen = resLen;
    cur = (CHAR*)res;

    if((*cur) == '{'){
        cur++;
        remLen--;
        ret = parse_resCfgGetTkn(cur, remLen, '}', &endTkn);
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Token parsing faeild(ret=%d)\n",ret);
            return ret;
        }

        RRL_LOG(RRL_DBG,"PARSE:OPT PATH VAL=[%.*s]\n",(UINT)(endTkn - cur), cur);
        if(firstFlg == RC_TRUE){
            rrlCb = (RrllibCb*)cb;

            if(rrlCb->resPathLst != NULL){
                ret = rrllib_pathChkDynPath(resPath->nxtPathLst, cur, (UINT)(endTkn-cur), &newPath);
            }
            else {
                ret = rrllib_pathAddFirstDynPath(rrlCb, cur, (UINT)(endTkn-cur), &newPath);
            }
        }
        else {
            resPath = (RrllibResPath*)cb;

            if(resPath->nxtPathLst != NULL){
                ret = rrllib_pathChkDynPath(resPath->nxtPathLst, cur, (UINT)(endTkn-cur), &newPath);
            }/* end of if(resPath->resPathLst != NULL) */
            else {
                ret = rrllib_pathAddNxtDynPath(resPath, cur, (UINT)(endTkn-cur), &newPath);
            }
        }
        if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Path create failed(ret=%d, tkn=%.*s)\n", ret, (UINT)(endTkn - cur), cur);
            return ret;
        }

        *rt_resPath = newPath;

        if(((endTkn - cur) + 1 ) >= remLen){
            *rt_endTkn = endTkn;
            return RC_OK;
        }

        /* end line check */
        cur = endTkn+1;

        if((*cur) == '/'){
            *rt_endTkn = cur;
            return RC_OK;
        }
        else {
            return RRLERR_INVALID_TOKEN;
        }

        return ret;
    }/* end of if((*cur) == '{') */
    else {
        UINT tknLen = 0;
        UINT endFlg = RC_FALSE;

        ret = parse_resCfgGetTkn(cur, remLen, '/', &endTkn);
        if((ret != RC_OK) &&
           (ret != RRLERR_END)){
            *rt_endTkn = cur;
            return ret;
        }

        if(ret == RRLERR_END){
            RRL_LOG(RRL_DBG,"PARSE:FIX PATH VAL=[%.*s]\n",(UINT)(endTkn - cur)+1, cur);
            tknLen = (endTkn - cur) +1;
            endFlg = RC_TRUE;
        }
        else{
            RRL_LOG(RRL_DBG,"PARSE:FIX PATH VAL=[%.*s]\n",(UINT)(endTkn - cur), cur);
            tknLen = (endTkn - cur);
            endFlg = RC_FALSE;
        }

        if(firstFlg == RC_TRUE){
            rrlCb = (RrllibCb*)cb;

            if(rrlCb->resPathLst == NULL){
                ret = rrllib_pathAddFirstFixPathLst(rrlCb, &resPathLst);
                if(ret != RC_OK){
                    RRL_LOG(RRL_ERR,"First fixed path list create failed(ret=%d)\n",ret);
                    return ret;
                }
            }
            else {
                resPathLst = rrlCb->resPathLst;
            }
        }
        else {
            resPath = (RrllibResPath*)cb;

            if(resPath->nxtPathLst == NULL){
                ret = rrllib_pathAddNxtFixPathLst(resPath, &resPathLst);
                if(ret != RC_OK){
                    RRL_LOG(RRL_ERR,"Next fixed path list create failed(ret=%d)\n",ret);
                    return ret;
                }
            }
            else {
                resPathLst = resPath->nxtPathLst;
            }
        }
        
        /* find */
        ret = rrllib_pathFindFixPath(resPathLst, cur, tknLen, &newPath);
        if(ret != RC_OK){
            ret = rrllib_pathAddFixPath(resPathLst, cur, tknLen, &newPath);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Next fixed path create failed(ret=%d)\n",ret);
                return ret;
            }
        }

        *rt_resPath = newPath;

        *rt_endTkn = endTkn;

        if(endFlg == RC_TRUE){
            return RRLERR_END;
        }
        else {
            return RC_OK;
        }
    }/* end of else */

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_parseKvpToDoc(CONST CHAR *key, UINT keyLen, CONST CHAR *val, UINT valLen, 
                                         RrllibQueryDocCfg *queryDocCfg)
{
    SINT ret = 0;

    ret = parse_kvpToDoc(key, keyLen, val, valLen, (VOID*)queryDocCfg);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Query key value pair parinsg failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_parseUriQuery(CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg)
{
    return parse_query(query, queryLen, func, arg);
}

FT_PUBLIC RT_RESULT rrllib_parseUriFull(RrllibCb *rrlCb, UINT mthod, CONST CHAR *uri, UINT uriLen, RrllibDoc **rt_doc)
{
    SINT ret = 0;
    CHAR *uriStr = NULL;
    UriParserStateA state;
    UriUriA parseDat;
    RrllibDoc *doc = NULL;
    RrllibResMthod *resMthod = NULL;
    RrllibQueryDocCfg queryDocCfg;
    UriTextRangeA   *querySeg = NULL;

    state.uri = &parseDat;

    uriStr = comlib_memMalloc(uriLen+1);

    comlib_strNCpy(uriStr, (CHAR*)uri, uriLen);

    uriStr[uriLen] = '\0';

    state.uri = &parseDat;

    ret = uriParseUriA(&state, uriStr);
    if(ret != URI_SUCCESS){
        RRL_LOG(RRL_ERR,"Uri parsing failed(ret=%d)\n",ret);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return RRLERR_PARINSG_FAILED;
    }

    ret = parse_uri(rrlCb, &parseDat, mthod, &doc, &resMthod);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Uri parsing failed(ret=%d)\n",ret);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return ret;
    }

    /* check query */
    querySeg = &parseDat.query;

    queryDocCfg.doc = doc;
    queryDocCfg.resMthod = resMthod;
    ret = parse_query(querySeg->first, (querySeg->afterLast - querySeg->first), parse_kvpToDoc, &queryDocCfg);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Query parinsg failed(ret=%d)\n",ret);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return ret;
    }

    if(doc->mandCnt != resMthod->mandCnt){
        RRL_LOG(RRL_ERR,"Mandantory value not exist(uriLen=%d, mand=%d, rcv=%d)\n",uriLen, resMthod->mandCnt, doc->mandCnt);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return RRLERR_MAND_VALUE_NOT_EXSIT;
    }

    comlib_memFree(uriStr);
    uriFreeUriMembersA(&parseDat);

    *rt_doc = doc;

    return RC_OK;
}

FT_PUBLIC RT_RESULT rrllib_parseUriPath(RrllibCb *rrlCb, UINT mthod, CONST CHAR *uri, UINT uriLen, RrllibResMthod **rt_mthod, 
                                        RrllibDoc **rt_doc)
{
    SINT ret = 0;
    CHAR *uriStr = NULL;
    UriParserStateA state;
    UriUriA parseDat;
    RrllibDoc *doc = NULL;
    RrllibResMthod *resMthod = NULL;

    state.uri = &parseDat;

    uriStr = comlib_memMalloc(uriLen+1);

    comlib_strNCpy(uriStr, (CHAR*)uri, uriLen);

    uriStr[uriLen] = '\0';

    state.uri = &parseDat;

    ret = uriParseUriA(&state, uriStr);
    if(ret != URI_SUCCESS){
        RRL_LOG(RRL_ERR,"Uri parsing failed(ret=%d)\n",ret);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return RRLERR_PARINSG_FAILED;
    }

    ret = parse_uri(rrlCb, &parseDat, mthod, &doc, &resMthod);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Uri parsing failed(ret=%d)\n",ret);
        comlib_memFree(uriStr);
        uriFreeUriMembersA(&parseDat);
        return ret;
    }

    comlib_memFree(uriStr);
    uriFreeUriMembersA(&parseDat);

    if(rt_mthod != NULL){
        *rt_mthod = resMthod;
    }

    *rt_doc = doc;

    return RC_OK;
}

FT_PRIVATE RT_RESULT parse_uri(RrllibCb *rrlCb, UriUriA *parseDat, UINT mthod, 
									RrllibDoc **rt_doc, RrllibResMthod **rt_mthod)
{
    SINT ret = 0;
    ComlibLnkNode *lnkNode = NULL;
    RrllibResPathLst *resPathLst = NULL;
    RrllibResPath *resPath = NULL;
    RrllibResMthod *resMthod = NULL;
    RrllibDoc *doc = NULL;
    RrllibDocArg *docArg = NULL;
    CHAR *uriStr = NULL;
    UriPathSegmentA *pathSeg = NULL;

    /* make Doc */
    pathSeg = parseDat->pathHead;
    if(pathSeg == NULL){
        RRL_LOG(RRL_ERR,"Uri path not exist\n");
        ret = RRLERR_URI_PATH_NOT_EXIST;
        goto goto_parseErr;
    }

    ret = rrllib_docGetNewDoc(&doc);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Document create failed(ret=%d)\n",ret);
        goto goto_parseErr;
    }

    resPathLst = rrlCb->resPathLst;

    RRL_LOG(RRL_DBG,"PARSE:URI=%s\n", uriStr);

    /* check path */
    while(1){
        if(resPathLst->pathType == RRL_PATH_TYPE_DYN){
            COM_GET_LNKLST_FIRST(&resPathLst->resPathLL, lnkNode);
            if(lnkNode == NULL){
                RRL_LOG(RRL_ERR,"dynamic path not exist\n");
                ret = RRLERR_DYN_PATH_NOT_EXIST;
                goto goto_parseErr;
            }

            resPath = lnkNode->data;

            RRL_LOG(RRL_DBG,"PARSE:DYN PATH=[%.*s=%.*s]\n", resPath->nameLen, resPath->name, 
                    (UINT)(pathSeg->text.afterLast - pathSeg->text.first),
                    pathSeg->text.first);

            ret = rrllib_docFindArg(doc, resPath->name, resPath->nameLen, &docArg);
            if(ret != RC_OK){
                ret = rrllib_docAddArg(doc, resPath->name, resPath->nameLen, &docArg);
                if(ret != RC_OK){
                    RRL_LOG(RRL_ERR,"Document arguemnt create failed(ret=%d)\n",ret);
                    goto goto_parseErr;
                }
            }
            else {
                RRL_LOG(RRL_ERR,"Document already exist(%.*s)\n", resPath->nameLen, resPath->name);
                ret = RRLERR_DOC_ARG_DUP;
                goto goto_parseErr;
            }

            ret = rrllib_docAddArgVal(docArg, pathSeg->text.first, (UINT)(pathSeg->text.afterLast - pathSeg->text.first));
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Document arguemnt value insert failed(ret=%d)\n",ret);
                goto goto_parseErr;
            }
        }/* end of if(resPathLst->pathType == RRL_PATH_TYPE_DYN) */
        else { /* FIXED */
            ret = rrllib_pathFindFixPath(resPathLst, pathSeg->text.first, 
                                         (pathSeg->text.afterLast - pathSeg->text.first),
                                         &resPath);
            if(ret != RC_OK){
                RRL_LOG(RRL_ERR,"Path not exist(path=%.*s ret=%d)\n",
                        (UINT)(pathSeg->text.afterLast - pathSeg->text.first),
                        pathSeg->text.first, ret);
                ret = RRLERR_FIX_PATH_NOT_EXIST;
                goto goto_parseErr;
            }
            RRL_LOG(RRL_DBG,"PARSE:FIX PATH=[%.*s]\n", resPath->nameLen, resPath->name);
        }

        /* get next */
        pathSeg = pathSeg->next;
        if(pathSeg == NULL){
            break;
        }

        resPathLst = resPath->nxtPathLst;
        if(resPathLst == NULL){
            RRL_LOG(RRL_ERR,"next path not exist(tkn=%.*s)\n", 
                    (UINT)(pathSeg->text.afterLast - pathSeg->text.first),
                    pathSeg->text.first);
            ret = RRLERR_NXT_PATH_NOT_EXSIT;
            goto goto_parseErr;
        }
    }/* end of while(1) */

    /* check method */
    COM_GET_LNKLST_FIRST(&resPath->mthodLL, lnkNode);
    if(lnkNode == NULL){
        RRL_LOG(RRL_ERR,"Method not exist\n");
        ret = RRLERR_MTHOD_NOT_EXSIT;
        goto goto_parseErr;
    }

    while(1){
        resMthod = lnkNode->data;

        if(resMthod->mthod == mthod){ /* find */
            RRL_LOG(RRL_DBG,"PARSE:METHOD=%d(%s)\n",mthod, rrllib_mthodGetStr(mthod));
            doc->mthod = resMthod->mthod;
            break;
        }

        COM_GET_NEXT_NODE(lnkNode);
        if(lnkNode == NULL){
            RRL_LOG(RRL_ERR,"Method not exist(cnt=%d)\n", resPath->mthodLL.nodeCnt);
            ret = RRLERR_MTHOD_NOT_EXSIT;
            goto goto_parseErr;
        }
    }/* end of while(1) */

    doc->usrArg = resMthod->arg;

    *rt_doc = doc;
	*rt_mthod = resMthod;

    return RC_OK;

goto_parseErr:
    if(doc != NULL){
        rrllib_docDstry(doc);
    }

    return ret;
}

FT_PUBLIC RT_RESULT rrllib_parseResCfg(RrllibCb *rrlCb, CONST CHAR *res, UINT resLen, UINT mthod, 
                                       CONST CHAR *query, UINT queryLen, VOID *usrArg)
{
    SINT ret = 0;
    CHAR *cur = NULL;
    CHAR *endTkn = NULL;
    UINT remLen = 0;
    UINT firstFlg = RC_TRUE;
    RrllibResPath *resPath = NULL;
    RrllibResMthod *resMthod = NULL;
    VOID *cb = NULL;

    remLen = resLen;
    cur = (CHAR*)res;

    if((mthod !=  RRL_MTHOD_GET) && (mthod !=  RRL_MTHOD_POST) &&
       (mthod !=  RRL_MTHOD_DEL) && (mthod !=  RRL_MTHOD_PUT)){
        RRL_LOG(RRL_ERR,"Invlaid method(%d)\n",mthod);
        return RRLERR_INVALID_MTHOD;
    }

    /* skip space */
    cur = parse_skipSpace((CHAR*)res, resLen);

    if((*cur) != '/'){
        RRL_LOG(RRL_ERR,"Invalid First token(%c)\n",*cur);
        return RRLERR_INVALID_TOKEN;
    }

    cur++;

    remLen = resLen - (UINT)(cur - res);

    cb = (VOID*)rrlCb;
    firstFlg = RC_TRUE;

    while(1){
        ret = parse_resPathCfg(cb, firstFlg,  cur , remLen, &resPath , &endTkn);
        if(ret == RRLERR_END){
            break;
        }
        else if(ret != RC_OK){
            RRL_LOG(RRL_ERR,"Path config paring failed(ret=%d)\n",ret);
            return ret;
        }

        cb = (VOID*)resPath;
        firstFlg = RC_FALSE;

        remLen -= endTkn - cur;
        cur = endTkn+1;
        remLen--;
        if(remLen == 0){
            break;
        }
    }/* end of while(1) */

    /* find method */
    ret = rrllib_mthodAdd(resPath, mthod, usrArg, &resMthod);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Method add failed(ret=%d)\n",ret);
        return ret;
    }

    /* added query */
    ret = parse_queryCfg(resMthod, query, queryLen);
    if(ret != RC_OK){
        RRL_LOG(RRL_ERR,"Query add failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

