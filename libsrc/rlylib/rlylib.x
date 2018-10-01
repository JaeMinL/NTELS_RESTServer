/************************************************************************

     Name:     Relay library 

     Type:     C structure file

     Desc:     Relay library structure

     File:     rlylib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RLYLIB_X_
#define _RLYLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RlylibRlmOptArg      RlylibRlmOptArg;
typedef struct RlylibHostOptArg     RlylibHostOptArg;
typedef struct RlylibTmrOptArg      RlylibTmrOptArg;
typedef struct RlylibOptArg         RlylibOptArg;
typedef struct RlylibRlmKey         RlylibRlmKey;
typedef struct RlyblibCb            RlylibCb;

typedef struct RlylibIntMainCb      *RlylibMainCb;

/* log function definition */
typedef VOID (*RlylibLogFunc)  (UINT, CHAR*, UINT, CHAR*); /* logFunc(level, file, line, logStr) */

struct RlylibTmrOptArg{
    UINT                            connTmOut;
    UINT                            connWaitTmOut;
    UINT                            initTmOut;
    UINT                            hBeatSndTmOut;
    UINT                            hBeatTmOut;
    UINT                            hostFreeTmOut;
    UINT                            msgDropTmOut;
};

struct RlylibRlmOptArg{
    UINT                            maxRegHostCnt;
    UINT                            hashType; /* hashing active host */
    UINT                            hashKeyType; /* default string */
};

struct RlylibHostOptArg{
    BOOL                            msgDropIndFlg;
    UINT                            rlyMode;
    UINT                            freeTmEnb;
    RlylibTmrOptArg                 tmrOptArg;
};

struct RlylibOptArg{
    UINT                            maxHostCnt; /* default : 256 */
    UINT                            maxRlmCnt; /* default : 256 */
    UINT                            acptType;
    UINT                            thrdCnt;
    BOOL                            condWaitFlg;
    ULONG                           waitTm; /* default : 100ms */
    RlylibHostOptArg                dfltHostOptArg;
};

struct RlylibRlmKey{
    VOID                            *key;
    UINT                            keyLen;
};

struct RlyblibCb{
    RlylibMainCb                    main;
};

/* initialization public function */
FT_PUBLIC RT_RESULT              rlylib_apiInitGlob                   ();
FT_PUBLIC RT_RESULT              rlylib_apiSetLogFunc                 (UINT lvl, RlylibLogFunc logFunc);
FT_PUBLIC RT_RESULT              rlylib_apiInitRlylibCb               (RlylibCb *rlylibCb, UINT rlyType, CHAR *locHost, 
                                                                       RlylibOptArg *optArg);

/* destory public function */
FT_PUBLIC RT_RESULT              rlylib_apiDstryRlylibCb              (RlylibCb *rlylibCb);

/* Control public function */
FT_PUBLIC RT_RESULT              rlylib_apiAddRlm                     (RlylibCb *rlylibCb, CHAR *realm, UINT rule, 
                                                                       RlylibRlmOptArg *rlmOptArg);
FT_PUBLIC RT_RESULT              rlylib_apiRegHostInRlm               (RlylibCb *rlylibCb, CHAR *realm, CHAR *host, 
                                                                       BOOL priFlg /* is primary */, RlylibRlmKey *rlmKey);
FT_PUBLIC RT_RESULT              rlylib_apiAddConn                    (RlylibCb *rlylibCb, CHAR *host, TrnlibTransAddr *dstAddrs, 
                                                                       UINT dstAddrCnt);
FT_PUBLIC RT_RESULT              rlylib_apiAddHost                    (RlylibCb *rlylibCb, CHAR *host, UINT type, UINT hashId, 
                                                                       RlylibHostOptArg *hostOptArg, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylib_apiDelHost                    (RlylibCb *rlylibCb, CHAR *host);
FT_PUBLIC RT_RESULT              rlylib_apiSetPriHost                 (RlylibCb *rlylibCb, CHAR *host);
FT_PUBLIC RT_RESULT              rlylib_apiAddAcptLst                 (RlylibCb *rlylibCb, TrnlibTransAddr *srcAddr);
FT_PUBLIC RT_RESULT              rlylib_apiGetHostSta                 (RlylibCb *rlylibCb, CHAR *host, UINT *rt_sta);

/* Transport public function */
/* ToAny : RR(Round-Robin) 
 * ToPri : PS(Primary-Secondary)
 */
FT_PUBLIC RT_RESULT              rlylib_apiSndFixMsgToAny             (RlylibCb *rlylibCb, CHAR *buf, UINT bufLen, CHAR *rt_host,
                                                                       UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylib_apiSndFixMsgToPri             (RlylibCb *rlylibCb, CHAR *buf, UINT bufLen, 
                                                                       BOOL faFlg /* failover flag */, CHAR *rt_host, 
                                                                       UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylib_apiSndFixMsgToHost            (RlylibCb *rlylibCb, CHAR *host, CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiSndDynMsgToHostId          (RlylibCb *rlylibCb, UINT hostId, CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiSndFixMsgToHostId          (RlylibCb *rlylibCb, UINT hostId, CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiSndFixMsgToRlm             (RlylibCb *rlylibCb, CHAR *realm, RlylibRlmKey *rlmKey, 
                                                                       CHAR *buf, UINT bufLen, CHAR *rt_host, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylib_apiRcvDynMsgFromAny           (RlylibCb *rlylibCb, CHAR *rt_host, UINT *rt_hostId, 
                                                                       CHAR **rt_buf, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiRcvFixMsgFromAny           (RlylibCb *rlylibCb, CHAR *rt_host, UINT *rt_hostId, 
                                                                       CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiRcvDynMsgFromHost          (RlylibCb *rlylibCb, CHAR *host, CHAR **rt_buf, 
                                                                       UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiRcvFixMsgFromHost          (RlylibCb *rlylibCb, CHAR *host, CHAR *rt_buf, 
                                                                       UINT maxBufLen, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylib_apiRcvFixMsgFromRlm           (RlylibCb *rlylibCb, CHAR *realm, CHAR *rt_buf, 
                                                                       UINT maxBufLen, UINT *rt_bufLen, CHAR *rt_host, 
                                                                       UINT *rt_hostId);

#ifdef __cplusplus
}
#endif

#endif /* _RLYLIB_X_ */
