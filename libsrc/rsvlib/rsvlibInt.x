#ifndef _RSVLIB_INT_X_
#define _RSVLIB_INT_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RsvlibIntSvrThrdMainCb  RsvlibIntSvrThrdMainCb;
typedef struct RsvlibIntSvrThrdCb      RsvlibIntSvrThrdCb;
typedef struct RsvlibIntSimDat         RsvlibIntSimDat;
typedef struct RsvlibIntSimTran        RsvlibIntSimTran;
typedef struct RsvlibIntUrlArg         RsvlibIntUrlArg;
typedef struct RsvlibIntCb             RsvlibIntCb;
typedef struct RsvlibIntSimRspCfg      RsvlibIntSimRspCfg;
typedef struct RsvlibIntTknCb          RsvlibIntTknCb;
typedef struct RsvlibIntTknInfo        RsvlibIntTknInfo;
typedef struct RsvlibIntSvrRule        RsvlibIntSvrRule;
typedef struct RsvlibIntGlobCb         RsvlibIntGlobCb;

struct RsvlibIntSimRspCfg{    
    ComlibLnkLst           simLnkLst;
};

struct RsvlibIntSimDat{
    UINT                   datLen;
    CHAR                   *dat;
    ComlibLnkNode          lnkNode;
};

struct RsvlibIntSimTran{
    ComlibLnkNode          *curNode;
    ComlibLnkLst           simLnkLst;
};

struct RsvlibIntUrlArg{
    UINT                   type;
    BOOL                   authFlg;
    union {
        RsvlibIntSimTran   *simTran;
    }u;
};

struct RsvlibIntSvrRule{
    struct {
        ComlibLnkLst      simDatLl;
        //RsvlibIntSimDat    *simDat;
        UINT               maxStrmBufLen;
    }                      svrOpt;
    UINT                   type; /* streming or */
    VOID                   *usrArg;
    RsvlibProcFunc         func;
    RsvlibStrmProcFunc     strmFunc;
    RsvlibStrmFreeFunc     strmFreeFunc;
};

struct RsvlibIntSvrThrdCb{
    RsvlibIntSvrThrdMainCb *svrThrdMainCb; /* own */
};

struct RsvlibIntSvrThrdMainCb{
    RrllibCb               rrlCb; /* read only(thread unsafe) */
    ThrlibMutx             sesMutx; /* server session mutex */
    UINT                   selectUseFlg;
    UINT                   thrdCnt;
    ComlibLnkLst           actvSvrSes;
    struct MHD_Daemon      *dm;
};

struct RsvlibIntCb{
    ThrlibMutx             runMutx; /* running mutx */
    BOOL                   runFlg;
    RsvlibGenCfg           genCfg;
    RsvlibIntSvrThrdMainCb svrThrdMainCb;
};

struct RsvlibIntGlobCb{
    //BOOL                   loglibInitFlg;
    //LoglibCb               loglibCb;
    ThrlibMutx             mutx;
    UINT                   logLvl;
    UINT                   logBufLen;
    CHAR                   logBuf[RSV_MAX_LOG_BUF_LEN];
    RsvlibLogFunc          logFunc;
    UINT                   rsvlibCbCnt;
    RsvlibIntCb            *rsvlibCb[RSV_MAX_SVR_CNT];
};

/* rsvlibInt_glob.c */
#if 0
FT_PUBLIC LoglibCb*     rsvlibInt_globGetLoglibCb          ();
#endif
FT_PUBLIC RT_RESULT     rsvlibInt_globSetRsvlibIntCb       (UINT id, RsvlibIntCb *rsvlibCb);
FT_PUBLIC RsvlibIntCb*  rsvlibInt_globGetRsvlibIntCb       (UINT id);
FT_PUBLIC RT_RESULT     rsvlibInt_globGetLogLvl            ();
FT_PUBLIC RT_RESULT     rsvlibInt_globSetLogFunc           (UINT lvl, RsvlibLogFunc logFunc);
FT_PUBLIC RT_RESULT     rsvlibInt_globLogPrnt              (UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...);
FT_PUBLIC RT_RESULT     rsvlibInt_globInit                 ();

/* rsvlibInt_init.c */
FT_PUBLIC RT_RESULT     rsvlibInt_init                     (UINT id, RsvlibGenCfg *genCfg);

/* rsvlibInt_main.c */
FT_PUBLIC RsvlibIntCb*  rsvlibInt_mainGetRsvlibIntCb    ();
FT_PUBLIC RT_RESULT     rsvlibInt_mainRun                  (UINT id);
FT_PUBLIC RT_RESULT     rsvlibInt_mainStop                 (UINT id);

/* rsvlibInt_svr.c */
FT_PUBLIC RT_RESULT     rsvlibInt_svrNewHdr                (CONST CHAR *key, CONST CHAR *val, RsvlibHttpHdr **rt_httpHdr);
FT_PUBLIC RT_RESULT     rsvlibInt_svrClearSvrSes           (RsvlibIntSvrThrdMainCb *svrThrdMainCb);
FT_PUBLIC RT_RESULT     rsvlibInt_svrFreeHdr               (RsvlibHttpHdr *httpHdr);
FT_PUBLIC RT_RESULT     rsvlibInt_svrStart                 (RsvlibIntSvrThrdMainCb *svrThrdMainCb, CHAR *cert, CHAR *key, 
                                                            UINT thrdCnt, UINT port);
FT_PUBLIC RT_RESULT     rsvlibInt_svrStop                  (RsvlibIntSvrThrdMainCb *svrThrdMainCb);

/* rsvlibInt_util.c */
FT_PUBLIC RT_RESULT     rsvlibInt_utilReadFile             (CHAR *path, CHAR **rt_buf);

#ifdef __cplusplus
}
#endif

#endif /* _RSVLIB_INT_X_ */

