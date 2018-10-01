#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "trnlib.h"
#include "trnlib.x"

#include "htplib.h"
#include "htplib.x"

HtplibGlobCb g_htpGlobCb;

FT_PUBLIC HtplibGlobCb* htplib_mainGetGlobCb()
{
    return &g_htpGlobCb;
}

/* if http is request type 
 *   return RC_OK
 * else 
 *   retunr RC_NOK
 */

FT_PUBLIC RT_RESULT htplib_mainHtpIsReq(CONST CHAR *htpLine, CONST UINT htpLineLen, BOOL *rt_reqFlg, UINT *rt_mthodIdx)
{
    SINT ret = RC_OK;
#if 0
    STATIC CHAR reqHdr[20] = {
        0,0,1,1,0,  /* a - e */
        0,1,1,0,0,  /* f - j */
        0,0,0,0,1,  /* k - o */
        1,0,0,0,1}; /* p - t */
    U_8 tmp = 0;
#endif
    //STATIC CHAR mthodMap[36][7];
    UINT mthodIdx = 0;

    if(htpLineLen < 4 ){
        HTP_LOG(HTP_ERR,"Http line to small(len=%d)\n", htpLineLen);
        return RC_NOK;
    }

    /* HTTP = 0x48545450 */
    if( trnlib_utilNtohl((*(U_32*)htpLine)) != 0x48545450){
        /* check request */
#if 0
        if(htpLine[0] >= 'a'){
            tmp = htpLine[0] - 'a';
        }
        else {
            tmp = htpLine[0] - 'A';
        }

        if((tmp >= 20) ||
           (reqHdr[(UINT)tmp] == 0)){
            return RC_NOK;
        }
#endif
        ret = htplib_mthodCvtStrToId(htpLine, htpLineLen, &mthodIdx);
        if(ret != RC_OK){
            HTP_LOG(HTP_ERR,"Can not convert method(ret=%d, str=%.*s)\n", ret, htpLineLen, htpLine);
            return RC_NOK;
        }

        if(rt_mthodIdx != NULL){
            (*rt_mthodIdx) = mthodIdx;
        }

        (*rt_reqFlg) = RC_TRUE;

        return RC_OK;
    }

    (*rt_reqFlg) = RC_FALSE;

    return RC_OK;
}

