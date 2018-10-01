#ifndef _RCLLIB_X_
#define _RCLLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RclibIntMainCb      *RcllibMainCb;

typedef struct RcllibHdr           RcllibHdr;
typedef struct RcllibHdrLst     RcllibHdrLst;
typedef struct RcllibCb            RcllibCb;

typedef UINT    (*RcllibWrteFunc)        (VOID *usrArg);

/* log function definition */
typedef VOID    (*RcllibLogFunc)         (UINT, CHAR*, UINT, CHAR*); /* logFunc(level, file, line, logStr) */

struct RcllibHdr{
    ComlibLnkNode          lnkNode;
    CHAR                   *key;
    CHAR                   *val;
};

struct RcllibHdrLst{
        ComlibLnkLst       hdrLl;
};

struct RcllibCb{
    RcllibMainCb           main;
};

FT_PUBLIC RT_RESULT      rcllib_apiGlobInit          ();
FT_PUBLIC RT_RESULT      rcllib_apiInit              (RcllibCb *rcllibCb);
FT_PUBLIC RT_RESULT      rcllib_apiDstry             (RcllibCb *rcllibCb);
FT_PUBLIC RT_RESULT      rcllib_apiPfrm              (RcllibCb *rcllibCb, UINT mthod, CHAR *url,
                                                      RcllibHdrLst *hdrLst, CHAR *dat);
FT_PUBLIC RT_RESULT      rcllib_apiSetTimeOut        (RcllibCb *rcllibCb, UINT tmOut);
FT_PUBLIC RT_RESULT      rcllib_apiSetRsvFunc        (RcllibCb *rcllibCb, RcllibWrteFunc func, VOID *usrArg);
FT_PUBLIC RT_RESULT      rcllib_apiInitHdrLst        (RcllibHdrLst *hdrLst);
FT_PUBLIC RT_RESULT      rcllib_apiAddHdrLst         (RcllibHdrLst *hdrLst, CONST CHAR *key, CONST CHAR *val);
FT_PUBLIC RT_RESULT      rcllib_apiDstryHdrLst       (RcllibHdrLst *hdrLst);
FT_PUBLIC RT_RESULT      rcllib_apiGetStaCode        (RcllibCb *rcllibCb, UINT *rt_staCode);
FT_PUBLIC RT_RESULT      rcllib_apiCpyRspDatIntoDyn  (RcllibCb *rcllibCb, CHAR **rt_dat, UINT *rt_datLen);
FT_PUBLIC RT_RESULT      rcllib_apiCpyRspDatIntoFix  (RcllibCb *rcllibCb, CHAR *rt_dat, UINT maxDatLen, 
                                                      UINT *rt_datLen);

#ifdef __cplusplus
}
#endif

#endif /* _RCLLIB_X_ */

