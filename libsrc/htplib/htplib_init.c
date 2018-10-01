#include <unistd.h>
#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#include "htplib.h"
#include "htplib.x"

FT_PRIVATE RT_RESULT init_staCodeMap(HtplibStaCodeMap *staCodeMap)
{
    UINT i = 0;
    UINT src = 0;
    UINT pos = 0;
    UINT nmb = 0;
    UINT bcd = 0;

    comlib_memMemset(staCodeMap->mapper, HTPLIB_MTHOD_EMPTY_BKT , HTPLIB_STA_CODE_MAPPER_LEN * sizeof(USHORT));

    for(i=1;i<=599;i++){
        src = i;
        pos = 0;
        bcd = 0;

        do{
            nmb = src % 10;

            bcd += (nmb << pos);

            src = src / 10;
            pos += 4;
        }while(src > 0);


        staCodeMap->mapper[bcd] = i;
    }/* end of for(i=0;i<599;i++) */

    return RC_OK;
}

FT_PRIVATE RT_RESULT init_mthodMap(HtplibMthodMap *mthodMap)
{
    //UINT len = 0;
    //UINT idx = 0;
    CHAR *data = NULL;
    CHAR dataLen = 0;

    comlib_memMemset(mthodMap, 0x0, sizeof(HtplibMthodMap));

    /*
     * ex : 'A' = 0
     *      CONNECT = (('C' - 'A')  + ('O' - 'A')) = 2 + 14 = 16
     * | CONNECT | CO | 2 + 14  = 16 |
     * | DELETE  | DE | 3 + 4   = 7  |
     * | GET     | GE | 6 + 4   = 10 |
     * | HEAD    | HE | 7 + 4   = 11 |
     * | OPTIONS | OP | 14 + 15 = 29 |
     * | PUT     | PU | 15 + 20 = 35 |
     * | POST    | PO | 15 + 14 = 29 |
     * | PATCH   | PA | 15 + 0  = 15 |
     * | TRACE   | TR | 19 + 17 = 36 |
     */
    data = "CONNECT";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[16][0] = dataLen;
    mthodMap->mthodBkt[16][1] = HTPLIB_MHOTD_ID_CON;
    comlib_memMemcpy(&mthodMap->mthodBkt[16][2],data, dataLen);

    data = "DELETE";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[7][0] = dataLen;
    mthodMap->mthodBkt[7][1] = HTPLIB_MHOTD_ID_DEL;
    comlib_memMemcpy(&mthodMap->mthodBkt[7][2],data, dataLen);

    data = "GET";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[10][0] = dataLen;
    mthodMap->mthodBkt[10][1] = HTPLIB_MHOTD_ID_GET;
    comlib_memMemcpy(&mthodMap->mthodBkt[10][2],data, dataLen);

    data = "HEAD";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[11][0] = dataLen;
    mthodMap->mthodBkt[11][1] = HTPLIB_MHOTD_ID_HEAD;
    comlib_memMemcpy(&mthodMap->mthodBkt[11][2],data, dataLen);

    data = "OPTIONS";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[29][0] = dataLen;
    mthodMap->mthodBkt[29][1] = HTPLIB_MHOTD_ID_OPT;
    comlib_memMemcpy(&mthodMap->mthodBkt[29][2],data, dataLen);

    data = "PUT";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[35][0] = dataLen;
    mthodMap->mthodBkt[35][1] = HTPLIB_MHOTD_ID_PUT;
    comlib_memMemcpy(&mthodMap->mthodBkt[35][2],data, dataLen);

    data = "POST";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[29][0] = dataLen;
    mthodMap->mthodBkt[29][1] = HTPLIB_MHOTD_ID_POST;
    comlib_memMemcpy(&mthodMap->mthodBkt[29][2],data, dataLen);

    data = "PATCH";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[15][0] = dataLen;
    mthodMap->mthodBkt[15][1] = HTPLIB_MHOTD_ID_PTH;
    comlib_memMemcpy(&mthodMap->mthodBkt[15][2],data, dataLen);

    data = "TRACE";
    dataLen = comlib_strGetLen(data);
    mthodMap->mthodBkt[36][0] = dataLen;
    mthodMap->mthodBkt[36][1] = HTPLIB_MHOTD_ID_TRC;
    comlib_memMemcpy(&mthodMap->mthodBkt[36][2],data, dataLen);

#if 0
    comlib_memMemset(&mthodMap->mthodBkt, ~0x0, HTPLIB_MAX_MTHOD_BKT_LEN * HTPLIB_MAX_ALPHABET_LEN * sizeof(CHAR));

    len = 3;
    {
        idx = 0;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_GET;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "GET"); 
        mthodMap->mthodBkt[len]['G'-'A'] = idx;

        idx = 1;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_PUT;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "PUT"); 
        mthodMap->mthodBkt[len]['P'-'A'] = idx;
    }

    len = 4;
    {
        idx = 2;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_HEAD;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "HEAD"); 
        mthodMap->mthodBkt[len]['H'-'A'] = idx;

        idx = 3;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_POST;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "POST"); 
        mthodMap->mthodBkt[len]['P'-'A'] = idx;
    }

    len = 5;
    {
        idx = 4;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_TRC;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "TRACE"); 
        mthodMap->mthodBkt[len]['T'-'A'] = idx;

        idx = 5;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_PTH;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "PATCH"); 
        mthodMap->mthodBkt[len]['P'-'A'] = idx;
    }

    len = 6;
    {
        idx = 6;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_DEL;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "DELETE"); 
        mthodMap->mthodBkt[len]['P'-'A'] = idx;
    }

    len = 7;
    {
        idx = 7;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_CON;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "CONNECT"); 
        mthodMap->mthodBkt[len]['C'-'A'] = idx;

        idx = 8;
        mthodMap->mthodList[idx][0] = HTPLIB_MHOTD_ID_OPT;
        comlib_strCpy(&mthodMap->mthodList[idx][1], "OPTIONS"); 
        mthodMap->mthodBkt[len]['O'-'A'] = idx;
    }
#endif

    return RC_OK;
}

FT_PUBLIC RT_RESULT htplib_initGlob()
{
    HtplibGlobCb *globCb = NULL;
    UINT ret = RC_OK;

    globCb = htplib_mainGetGlobCb();

    ret = init_staCodeMap(&globCb->staCodeMap);
    if(ret != RC_OK){
        HTP_LOG(HTP_ERR,"Status code mapper init failed\n");
        return RC_NOK;
    }

    ret = init_mthodMap(&globCb->mthodMap);
    if(ret != RC_OK){
        HTP_LOG(THP_ERR,"Method mapper init failed\n");
        return RC_NOK;
    }

    return RC_OK;
}

