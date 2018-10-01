#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#include "htplib.h"
#include "htplib.x"

FT_PUBLIC RT_RESULT htplib_mthodCvtStrToId(CONST CHAR *mthod, CONST UINT mthodLen, UINT *rt_mthodIdx)
{
    SINT ret = RC_OK;
    HtplibMthodMap *mthodMap = NULL;
    HtplibGlobCb *globCb = NULL;
    CHAR firstChr = 0;
    CHAR secChr = 0;
    U_8 idx = 0;

    if(mthodLen < 3){
        HTP_LOG(HTP_ERR,"Invalid method length(len=%d, max=%d)\n",
                mthodLen, HTPLIB_MAX_MTHOD_BKT_LEN);

        return HTPERR_INVALID_MTHOD_LEN;
    }

    globCb = htplib_mainGetGlobCb();

    mthodMap = &globCb->mthodMap;

    if(mthod[0] >= 'a'){
        firstChr = mthod[0] - 'a';
    }
    else {
        firstChr = mthod[0] - 'A';
    }

    if(mthod[1] >= 'a'){
        secChr = mthod[1] - 'a';
    }
    else {
        secChr = mthod[1] - 'A';
    }

    idx = firstChr + secChr;

    if(idx >= 36){
        HTP_LOG(HTP_ERR,"Bucket not exist((index over flow)len=[%d], mthod=[%.*s])\n",
                mthodLen, mthodLen, mthod);
        return HTPERR_MTHOD_BKT_NOT_EXIST;
    }

    if(mthodMap->mthodBkt[idx][0] == 0){
        HTP_LOG(HTP_ERR,"Bucket not exist(len=[%d], mthod=[%.*s])\n",
                mthodLen, mthodLen, mthod);
        return HTPERR_MTHOD_BKT_NOT_EXIST;
    }

    ret = comlib_strCaseNCmp((VOID*)mthod, (VOID*)&mthodMap->mthodBkt[idx][2], (UINT)mthodMap->mthodBkt[idx][0]);
    if(ret != 0){
        HTP_LOG(HTP_ERR,"method not found(%.*s)\n", mthodLen, mthod);
        return HTPERR_INVALID_MTHOD;
    }

    *rt_mthodIdx = (UINT)(mthodMap->mthodBkt[idx][1]);


#if 0
    SINT ret = 0;
    U_8 firstChr = 0;
    HtplibMthodMap *mthodMap = NULL;
    HtplibGlobCb *globCb = NULL;
    U_8 myBkt = 0;

    globCb = htplib_mainGetGlobCb();

    mthodMap = &globCb->mthodMap;

    if(mthodLen >= HTPLIB_MAX_MTHOD_BKT_LEN){
        HTP_LOG(HTP_ERR,"Invalid method length(len=%d, max=%d)\n",
                mthodLen, HTPLIB_MAX_MTHOD_BKT_LEN);

        return HTPERR_INVALID_MTHOD_LEN;
    }

    if(mthod[0] >= 'a'){
        firstChr = mthod[0] - 'a';
    }
    else {
        firstChr = mthod[0] - 'A';
    }

    if(firstChr >= HTPLIB_MAX_ALPHABET_LEN){
        HTP_LOG(HTP_ERR,"Invalid first char(%c)\n",mthod[0]);
        return HTPERR_INVALID_MTHOD_FIRST_CHR;
    }

    myBkt = mthodMap->mthodBkt[mthodLen][firstChr];
    if(myBkt == HTPLIB_MTHOD_EMPTY_BKT){
        HTP_LOG(HTP_ERR,"Bucket not exist(len=[%d], mthod=[%.*s])\n",
                mthodLen, mthodLen, mthod);
        return HTPERR_MTHOD_BKT_NOT_EXIST;
    }

#if 0
    ret = comlib_memMemcmp((VOID*)mthod, (VOID*)&mthodMap->mthodList[myBkt][1], mthodLen);
#else
    ret = comlib_strCaseNCmp(mthod, &mthodMap->mthodList[myBkt][1], mthodLen);
#endif
    if(ret != 0){
        HTP_LOG(HTP_ERR,"method not found(%.*s)\n", mthodLen, mthod);
        return HTPERR_INVALID_MTHOD;
    }

    *rt_mthodIdx = (UINT)(mthodMap->mthodList[myBkt][0]);
#endif

    return RC_OK;
}

