#ifndef _SMU_INT_X_
#define _SMU_INT_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SmulibIntMsgComHdr   SmulibIntMsgComHdr;
typedef struct SmulibIntCfg         SmulibIntCfg;
typedef struct SmulibIntSmdInfo     SmulibIntSmdInfo;
typedef struct SmulibIntLocInfo     SmulibIntLocInfo;
typedef struct SmulibIntTmrInfo     SmulibIntTmrInfo;
typedef struct SmulibIntSig         SmulibIntSig;
typedef struct SmulibIntThrdBlkCb   SmulibIntThrdBlkCb;
typedef struct SmulibIntThrdBlkInfo SmulibIntThrdBlkInfo;
typedef struct SmulibIntMainBlkInfo SmulibIntMainBlkInfo;
typedef struct SmulibIntSigInfo     SmulibIntSigInfo;
typedef struct SmulibIntLogInfo     SmulibIntLogInfo;
typedef struct SmulibIntMainCb      SmulibIntMainCb;
typedef struct SmulibIntGlobCb      SmulibIntGlobCb;

/* internal common header */
struct SmulibIntMsgComHdr{
    U_32                         cmdCode;
    U_32                         msgLen;
};

struct SmulibIntSmdInfo{
    UINT                         spId; /* smd id */
    UINT                         status;
    UINT                         hbSndWaitCnt;
    RlylibCb                     rlylibCb;
    UINT                         hbSndTmOut;
    UINT                         regSndTmOut;
    UINT                         freezeTmOut;
    TrnlibTransAddr              trnAddr;
    ComlibTmrNode                tmrNode;
};

struct SmulibIntLocInfo{
    UINT                         pid;
    CHAR                         strPid[SMULIB_STR_PID_MAX_LEN];
    UINT                         nameLen;
    CHAR                         name[SMD_PROC_NAME_MAX_LEN];
    ComlibTmrNode                tmrNode;
};

struct SmulibIntTmrInfo{
    ComlibTimer                  tmr;
    ComlibTmrTbl                 tmrTbl;
};

struct SmulibIntCfg{
    CHAR                         name[SMD_PROC_NAME_MAX_LEN];
    BOOL                         smdAddrFlg;
    TrnlibTransAddr              smdAddr;
    ThrlibTq                     *rcvTq;
    SmulibOptArg                 optArg;
};

struct SmulibIntSig{
    UINT                         sigNo;
    SmulibSigFunc                func;
    VOID                         *usrArg;
    ComlibHashNode               hNode;
};

struct SmulibIntSigInfo{
    ThrlibMutx                   mutx;
    ComlibHashTbl                sigHT;
};

struct SmulibIntMainCb{
    ThrlibTq                     *rcvTq;
    SmulibIntTmrInfo             tmrInfo;
    SmulibIntLocInfo             locInfo;
    SmulibIntSmdInfo             smdInfo;
};

struct SmulibIntLogInfo{
    ThrlibMutx                   mutx;
    UINT                         logLvl;
    SmulibLogFunc                logFunc;
    UINT                         logBufLen;
    CHAR                         logBuf[SMULIB_MAX_LOG_BUF_LEN];
};

struct SmulibIntThrdBlkCb{
    ThrlibMutx                   mutx;
    ThrlibThrdId                 tid;
    UINT                         thrdTmOut;
    BOOL                         killPrcFlg;
    struct {
        ULONG                    lstUpdTick; /* writer : app  */
    } appCtrl;
    VOID                         *usrArg;
    /* func */
    SmulibTmOutFunc              tmOutFunc;
    ComlibLnkNode                lnkNode;
};

struct SmulibIntThrdBlkInfo{
    ThrlibMutx                   mutx;
    ComlibLnkLst                 blkLL;
};

struct SmulibIntMainBlkInfo{
    UINT                         status;
    ULONG                        lstUpdTick;
    /* func */
    SmulibTmOutFunc              tmOutFunc;
};

struct SmulibIntGlobCb{
    ULONG                        curTick; /* time writer : main thread */
    SmulibIntMainBlkInfo         mainBlkInfo;
    SmulibIntThrdBlkInfo         thrdBlkInfo;
    SmulibIntSigInfo             sigInfo;
    SmulibIntLogInfo             logInfo;
    ThrlibTq                     sndTq;
};

/* smulibInt_glob.c */
FT_PUBLIC RT_RESULT             smulibInt_globGetInitFlg         ();
FT_PUBLIC ThrlibTq*             smulibInt_globGetSndTq           ();
FT_PUBLIC SmulibIntSigInfo*     smulibInt_globGetSigInfo         ();
FT_PUBLIC RT_RESULT             smulibInt_globSetLogFunc         (UINT lvl, SmulibLogFunc logFunc);
FT_PUBLIC UINT                  smulibInt_globGetLogLvl          ();
FT_PUBLIC RT_RESULT             smulibInt_globSetLogLvl          (UINT logLvl);
FT_PUBLIC RT_RESULT             smulibInt_globLogPrnt            (UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...);
FT_PUBLIC SmulibIntMainBlkInfo* smulibInt_globGetMainBlkInfo     ();
FT_PUBLIC SmulibIntThrdBlkInfo* smulibInt_globGetThrdBlkInfo     ();
FT_PUBLIC VOID                  smulibInt_globUpdCurTick         (ULONG curTick);
FT_PUBLIC ULONG                 smulibInt_globGetCurTick         ();
FT_PUBLIC RT_RESULT             smulibInt_globInit               ();
FT_PUBLIC RT_RESULT             smulibInt_globKeepalive          ();

/* smulibInt_main.c */
FT_PUBLIC VOID                  smulibInt_mainRlyLog             (UINT lvl, CHAR *file, UINT line, CHAR *str);
FT_PUBLIC RT_RESULT             smulibInt_mainTermProc           (BOOL frceDnFlg/* force down flag */);
FT_PUBLIC SmulibIntLocInfo*     smulibInt_mainGetLocInfo         ();
FT_PUBLIC SmulibIntSmdInfo*     smulibInt_mainGetSmdInfo         ();
FT_PUBLIC SmulibIntTmrInfo*     smulibInt_mainGetTmrInfo         ();
FT_PUBLIC VOID                  smulibInt_mainThrdMain           (VOID *arg);

/* smulibInt_init.c */
FT_PUBLIC RT_RESULT             smulibInt_init                   (SmulibIntMainCb *mainCb, CHAR *name, TrnlibTransAddr *trnAddr, 
                                                                  ThrlibTq *rcvTq, SmulibOptArg *optArg);
/* smulibInt_lh.c */
FT_PUBLIC RT_RESULT             smulibInt_lhSndRegReq            (SmulibIntSmdInfo *smdInfo, UINT pid, CHAR *name, UINT nameLen);
FT_PUBLIC RT_RESULT             smulibInt_lhRegCfmHdlr           (SmulibIntSmdInfo *smdInfo, CHAR *host, SmdMsgRegCfm *regCfm);
FT_PUBLIC RT_RESULT             smulibInt_lhSndHBeatReq          (SmulibIntSmdInfo *smdInfo, UINT pid);
FT_PUBLIC RT_RESULT             smulibInt_lhSndTermReq           (SmulibIntSmdInfo *smdInfo, UINT pid);
FT_PUBLIC RT_RESULT             smulibInt_lhRcvHdlr              (SmulibIntMainCb *mainCb, UINT *rt_loopCnt);

/* smulibInt_uh.c */
FT_PUBLIC RT_RESULT             smulibInt_uhChkMainTick          (ULONG curTick, SmulibIntMainBlkInfo *mainBlkInfo, 
                                                                  SmulibIntLocInfo *locInfo);
FT_PUBLIC RT_RESULT             smulibInt_uhRcvHdlr              (SmulibIntMainCb *mainCb, UINT *rt_loopCnt);

/* smulibInt_sig.c */
FT_PUBLIC RT_RESULT             smulibInt_sigReg                 (UINT sigNo, SmulibSigFunc func, VOID *usrArg);

/* smulibInt_thBlk.c */
FT_PUBLIC RT_RESULT             smulibInt_thBlkInit              (SmulibIntThrdBlkInfo *thrdBlkInfo, SmulibTmOutFunc tmOutFunc, 
                                                                  VOID *args, BOOL killPrcFlg, SmulibThrdBlkCfg *thrdBlkCfg, 
                                                                  SmulibIntThrdBlkCb **rt_thrdBlkCb);
FT_PUBLIC RT_RESULT             smulibInt_thBlkDstry             (SmulibIntThrdBlkInfo *thrdBlkInfo, 
                                                                  SmulibIntThrdBlkCb *thrdBlkCb);
FT_PUBLIC RT_RESULT             smulibInt_thBlkKeepalive         (SmulibIntThrdBlkCb *thrdBlkCb);
FT_PUBLIC RT_RESULT             smulibInt_thBlkChkThrd           ();

/* smulibInt_timer.c */
FT_PUBLIC RT_RESULT             smulibInt_timerAppTmOutHdlr      (SmulibIntLocInfo *locInfo);
FT_PUBLIC RT_RESULT             smulibInt_timerEvnt              (UINT evnt, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif /* _SMU_INT_X_ */

