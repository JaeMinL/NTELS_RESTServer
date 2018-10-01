#ifndef _HTPLIB_X_
#define _HTPLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HtplibStaCodeMap HtplibStaCodeMap;
typedef struct HtplibMthodMap   HtplibMthodMap;
typedef struct HtplibGlobCb     HtplibGlobCb;

struct HtplibStaCodeMap{
    USHORT                mapper[HTPLIB_STA_CODE_MAPPER_LEN];
};

#if 0
struct HtplibMthodMap{
    /* data index list */
    /* 7 = max length, 26 = alphabet */
    CHAR                  mthodBkt[HTPLIB_MAX_MTHOD_BKT_LEN][HTPLIB_MAX_ALPHABET_LEN];
    /* data buffer */
    /* GET, PUT, HEAD, POST, TRACE, PATCH, DELETE ,CONNECT ,OPTIONS */
    CHAR                  mthodList[HTPLIB_MTHOD_CNT][HTPLIB_MAX_MTHOD_BKT_LEN+1];
};
#else
struct HtplibMthodMap{
    /* mthodBkt[][0] = length
     * mthodBkt[][1] = idx 
     * mthodBkt[][2] = data
     */
    U_8                   mthodBkt[36][9];
};
#endif

struct HtplibGlobCb{
    HtplibStaCodeMap      staCodeMap;
    HtplibMthodMap        mthodMap;
};

/* htplib_init.c */
FT_PUBLIC RT_RESULT     htplib_initGlob               (VOID);

/* htplib_main.c */
FT_PUBLIC HtplibGlobCb* htplib_mainGetGlobCb          (VOID);
//FT_PUBLIC RT_RESULT     htplib_mainHtpIsReq           (CONST CHAR *htpLine, CONST UINT htpLineLen, BOOL *rt_reqFlg);
FT_PUBLIC RT_RESULT     htplib_mainHtpIsReq           (CONST CHAR *htpLine, CONST UINT htpLineLen, 
                                                       BOOL *rt_reqFlg, UINT *rt_mthodIdx);

/* htplib_staCode.c */
FT_PUBLIC RT_RESULT     htplib_staCodeCvtStrToNum     (CONST CHAR *staCode, CONST UINT staCodeLen, U_16*rt_staCode);

/* htplib_mthod.c */
FT_PUBLIC RT_RESULT     htplib_mthodCvtStrToId        (CONST CHAR *mthod, CONST UINT mthodLen, UINT *rt_mthodIdx);

#ifdef __cplusplus
}
#endif

#endif /* _HTPLIB_X_ */

