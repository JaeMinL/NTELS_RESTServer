#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#include "htplib.h"
#include "htplib.x"

FT_PUBLIC RT_RESULT htplib_staCodeCvtStrToNum(CONST CHAR *staCode, CONST UINT staCodeLen, U_16 *rt_staCode)
{
    U_16 bcd = 0;
    HtplibGlobCb *globCb = NULL;
    HtplibStaCodeMap *staCodeMap = NULL;

    if(staCodeLen != 3){
        HTP_LOG(HTP_ERR,"Invalid status code length(len=%d)\n", staCodeLen);
        return HTPERR_INVELID_STA_CODE_LEN;
    }

    /* make bcd */
    bcd = ((staCode[0] - 0x30) << 8) + ((staCode[1] - 0x30) << 4) + (staCode[2] - 0x30); 

    if(bcd >= HTPLIB_STA_CODE_MAPPER_LEN){
        HTP_LOG(TTP_ERR,"Status code too big(%.*s)\n",staCodeLen, staCode);
        return HTPERR_INVELID_STA_CODE;
    }

    globCb = htplib_mainGetGlobCb();

    staCodeMap = &globCb->staCodeMap;

    *rt_staCode = staCodeMap->mapper[bcd];

    return RC_OK;
}

