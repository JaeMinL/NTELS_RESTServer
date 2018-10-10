#ifndef _RSVLIB_API_X_
#define _RSVLIB_API_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RsvlibSesCb       RsvlibSesCb;
typedef struct RsvlibHttpHdr     RsvlibHttpHdr;
typedef struct RsvlibSslCfg      RsvlibSslCfg;
typedef struct RsvlibSvrCfg      RsvlibSvrCfg;
typedef struct RsvlibGenCfg      RsvlibGenCfg;

typedef RT_RESULT    (*RsvlibProcFunc)       (UINT mthod, RsvlibSesCb*);
typedef RT_RESULT    (*RsvlibStrmProcFunc)   (VOID*, U_64, CHAR*, SIZET max);
typedef RT_RESULT    (*RsvlibStrmFreeFunc)   (VOID *usrArg);

/* log function definition */
typedef VOID    (*RsvlibLogFunc)         (UINT, CHAR*, UINT, CHAR*); /* logFunc(level, file, line, logStr) */

struct RsvlibSslCfg{
    CHAR               *cert;
    CHAR               *key;
};

struct RsvlibSvrCfg{
    UINT               port;
    UINT               thrdCnt;
    RsvlibSslCfg       sslCfg;
};

struct RsvlibGenCfg{
    RsvlibSvrCfg       svrCfg;
};

struct RsvlibHttpHdr{
    ComlibLnkNode      lnkNode;
	UINT               keyLen;
	CHAR               *key;
	UINT               valLen;
	CHAR               *val;
};

struct RsvlibSesCb{
    struct {
        ComlibLnkNode  *hdrPtr;
        ComlibLnkLst   hdrLst;
        RrllibDoc      *doc;
        UINT           datLen;
        CHAR           *dat;
    } req;
    struct {
        UINT           staCode;
        ComlibLnkLst   hdrLst;
        UINT           datLen;
        CHAR           *dat;
    } rsp;
    TrnlibTransAddr    dstTrnsAddr;
    VOID               *strmUsrArg;
    VOID               *usrArg;
};

FT_PUBLIC RT_RESULT      rsvlib_apiSetLogFunc        (UINT lvl, RsvlibLogFunc logFunc);
FT_PUBLIC RT_RESULT      rsvlib_apiInit              (UINT id, RsvlibGenCfg *genCfg);
FT_PUBLIC RT_RESULT      rsvlib_apiRun               (UINT id);
FT_PUBLIC RT_RESULT      rsvlib_apiStop              (UINT id);
FT_PUBLIC RT_RESULT      rsvlib_apiSetRule           (UINT id, UINT mthod, CHAR *url, CHAR *query,
                                                      VOID *usrArg, RsvlibProcFunc func);
FT_PUBLIC RT_RESULT      rsvlib_apiSetStaCode        (RsvlibSesCb *sesCb, UINT staCode);
FT_PUBLIC RT_RESULT      rsvlib_apiSetRspDat         (RsvlibSesCb *sesCb, CHAR *rspDat, BOOL cpyFlg);
FT_PUBLIC RT_RESULT      rsvlib_apiSetRuleStrm       (UINT id, UINT mthod, CHAR *url, CHAR *query, VOID *usrArg,
                                                      RsvlibProcFunc func,
                                                      RsvlibStrmProcFunc procFunc, RsvlibStrmFreeFunc freeFunc,
                                                      UINT strmBufLen);
FT_PUBLIC RT_RESULT      rsvlib_apiFindHdr           (RsvlibSesCb *sesCb, CHAR *key, UINT keyLen, UINT seq,
                                                      CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiFindCaseHdr       (RsvlibSesCb *sesCb, CHAR *key, UINT keyLen, UINT seq,
                                                      CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiGetFirstHdr       (RsvlibSesCb *sesCb, 
                                                      CHAR **rt_key, UINT *rt_keyLen, 
                                                      CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiGetNxtHdr         (RsvlibSesCb *sesCb, 
                                                      CHAR **rt_key, UINT *rt_keyLen, 
                                                      CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiSetRspHdr         (RsvlibSesCb *sesCb, CHAR *key, CHAR *val, BOOL cpyFlg);
FT_PUBLIC RT_RESULT      rsvlib_apiFindArg           (RsvlibSesCb *sesCb, CHAR *name, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT      rsvlib_apiFirstArg          (RsvlibSesCb *sesCb, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT      rsvlib_apiNxtArg            (RsvlibSesCb *sesCb, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT      rsvlib_apiFirstArgVal       (RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiNxtArgVal         (RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT      rsvlib_apiSetSimDat         (UINT id, UINT mthod, CHAR *url, CHAR *simDat, BOOL cpyFlg);
FT_PUBLIC RT_RESULT      rsvlib_apiEnbSimDat         (UINT id, UINT mthod, CHAR *url);
FT_PUBLIC RT_RESULT      rsvlib_apiDisbSimDat        (UINT id, UINT mthod, CHAR *url);
FT_PUBLIC RT_RESULT      rsvlib_apiDelSimDat         (UINT id, UINT mthod, CHAR *url);

#ifdef __cplusplus
}
#endif

#endif /* _RSVLIB_API_X_ */

