/************************************************************************

     Name:     Software Management User library 

     Type:     C structure file

     Desc:     Software Management User library structure

     File:     smulib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _SMULIB_X_
#define _SMULIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SmulibOptArg        SmulibOptArg;
typedef struct SmulibThrdBlkCfg    SmulibThrdBlkCfg;
typedef struct SmulibThrdCb        SmulibThrdCb;

typedef struct SmulibIntThrdBlkCb  *SmulibThrdBlkCb;

typedef VOID (*SmulibLogFunc)     (UINT, CHAR*, UINT, CHAR*); /* logFunc(level, file, line, logStr) */
typedef VOID (*SmulibSigFunc)     (UINT, VOID*);              /* sigFunc(sig, usrArg) */
typedef VOID (*SmulibTmOutFunc)   (VOID*);                    /* tmOutFunc(usrArg) */

struct SmulibOptArg{
    UINT              rlyThrdCnt;
    UINT              freezeTmOut;
    UINT              hbSndTmOut;
    UINT              regSndTmOut;
    SmulibTmOutFunc   tmOutFunc;
    UINT              logLvl;
    SmulibLogFunc     logFunc;
};

struct SmulibThrdBlkCfg{
    UINT              thrdTmOut;
};

struct SmulibThrdCb{
    SmulibThrdBlkCb   main;
};

/* smu_api.c */
FT_PUBLIC RT_RESULT            smulib_apiInit             (CHAR *name, TrnlibTransAddr *smdAddr, SmulibOptArg *optArg);
FT_PUBLIC RT_RESULT            smulib_apiDstry            ();
FT_PUBLIC RT_RESULT            smulib_apiSigReg           (UINT sigNo,  SmulibSigFunc func, VOID *usrArg);
FT_PUBLIC RT_RESULT            smulib_apiSetLogFunc       (UINT lvl, SmulibLogFunc logFunc);
FT_PUBLIC UINT                 smulib_apiGetLogLvl        ();
FT_PUBLIC RT_RESULT            smulib_apiSetLogLvl        (UINT logLvl);
FT_PUBLIC RT_RESULT            smulib_apiRegThrdBlk       (SmulibThrdCb *thrdCb, SmulibTmOutFunc tmOutFunc, VOID *arg, 
                                                           BOOL killPrcFlg, SmulibThrdBlkCfg *thrdBlkCfg);
FT_PUBLIC RT_RESULT            smulib_apiDeregThrdBlk     (SmulibThrdCb *thrdCb);
FT_PUBLIC RT_RESULT            smulib_apiKeepaliveThrdBlk (SmulibThrdCb *thrdCb);
FT_PUBLIC RT_RESULT            smulib_apiKeepalive        ();

#ifdef __cplusplus
}
#endif

#endif /* _SMULIB_X_ */

