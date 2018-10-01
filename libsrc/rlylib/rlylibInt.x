/************************************************************************

     Name:     Relay library 

     Type:     C structure file

     Desc:     Relay internal library structure

     File:     rlylibInt.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RLYLIB_INT_X_
#define _RLYLIB_INT_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RlylibIntMsgChk         RlylibIntMsgChk;
typedef struct RlylibIntMsgQHdr        RlylibIntMsgQHdr;
typedef struct RlylibIntMsgBufInfo     RlylibIntMsgBufInfo;
typedef struct RlylibIntMsgHdr         RlylibIntMsgHdr;
typedef struct RlylibIntRcvMsgRslt     RlylibIntRcvMsgRslt;
typedef struct RlylibIntAvpHdr         RlylibIntAvpHdr;
typedef struct RlylibIntMsgInitReq     RlylibIntMsgInitReq;
typedef struct RlylibIntMsgInitAck     RlylibIntMsgInitAck;
typedef struct RlylibIntTmrInfo        RlylibIntTmrInfo;
typedef struct RlylibIntTmrHndlNode    RlylibIntTmrHndlNode;
typedef struct RlylibIntHostCb         RlylibIntHostCb; 
typedef struct RlylibIntRegHostBkt     RlylibIntRegHostBkt;
typedef struct RlylibIntRegRlmBkt      RlylibIntRegRlmBkt;
typedef struct RlylibIntRlmCb          RlylibIntRlmCb;
typedef struct RlylibIntConnCb         RlylibIntConnCb;
typedef struct RlylibIntRlmMainCb      RlylibIntRlmMainCb;
typedef struct RlylibIntHostMainCb     RlylibIntHostMainCb;
typedef struct RlylibIntHostThrdCb     RlylibIntHostThrdCb;
typedef struct RlylibIntAcptThrdCb     RlylibIntAcptThrdCb;
typedef struct RlylibIntTmpConnCb      RlylibIntTmpConnCb;
typedef struct RlylibIntAcptCb         RlylibIntAcptCb;
typedef struct RlylibIntMainMutx       RlylibIntMainMutx;
typedef struct RlylibIntMainCb         RlylibIntMainCb;
typedef struct RlylibIntGlobCb         RlylibIntGlobCb;

/*
   MsgQHdr : Internal extension header 
   MsgHdr : External header 

   Internal message format (application <--> relyay library)
   [MsgQHdr][MsgHdr][data format]

   Sned message format (host<-->host)
   [MsgHdr][data format]

   data format
   TYPE1. Control message(INIT, INIT_ACK, HB)
     [AVP][AVP][AVP]...
   TYPE2. normal message(Application data)
     [APPLICATION DATA]
*/
struct RlylibIntMsgQHdr{
    ULONG                           msgTick; /* send or receive time */
};

/* message standard */
struct RlylibIntMsgHdr{
    U_32                            msgLen; /* real message length */
    U_32                            msgCode;
    U_32                            strmId;
};

struct RlylibIntMsgChk{ /* message chunk */
    BOOL                            msgHdrIncFlg; /* message header include flag */
    CHAR                            hdrs[sizeof(RlylibIntMsgQHdr) + sizeof(RlylibIntMsgHdr)];
    U_32                            dataLen; /* allocated message buffer size */
    CHAR                            *data;
};

/* return to app */
struct RlylibIntRcvMsgRslt{
    UINT                            rt_bufLen;
    UINT                            allocBufLen; /* static buffer length */ 
    CHAR                            *rt_buf;
};

struct RlylibIntAvpHdr{
    U_32                            msgLen;
    U_32                            avpCode;
};

/* message handler */
struct RlylibIntMsgInitReq{
    UINT                            hostLen;
    CHAR                            *host;
    UINT                            hostKeyLen;
    CHAR                            *hostKey;
};

struct RlylibIntMsgInitAck{
    UINT                            hostLen;
    CHAR                            *host;
    UINT                            rslt;
};

struct RlylibIntMsgBufInfo{
    UINT                            maxBufCnt;
    ThrlibMutx                      allocMutx;
    ThrlibMutx                      freeMutx;
    ThrlibTq                        freeBuf[RLYLIB_MSG_BUF_CNT];
};


struct RlylibIntTmrInfo{
    UINT                            connTmOut;
    UINT                            connWaitTmOut;
    UINT                            initTmOut;
    UINT                            hBeatSndTmOut;
    UINT                            hBeatTmOut;
    UINT                            hostFreeTmOut;
    UINT                            msgDropTmOut;
};

struct RlylibIntRegRlmBkt{ /* reg realm bucket */
    ComlibLnkNode                   lnkNode;
    RlylibIntRlmCb                  *regRlm;
};

struct RlylibIntRegHostBkt{ /* reg host bucket */
    UINT                            spId; /* bucket id */
    UINT                            hashId; /* for hash */
    CHAR                            host[RLYLIB_MAX_HOST_LEN];
    RlylibIntHostCb                 *regHost;
    ComlibLnkNode                   lnkNode;
    ComlibHashKey                   hKey;
    ComlibHashNode                  hNode;
    RlylibRlmKey                    rlmKey; /* for DHT */
    CHAR                            rlmKeyData[RLYLIB_RLM_HASH_MAX_KEY_LEN];
};

struct RlylibIntRlmCb{
    UINT                            spId;
    ThrlibMutx                      mutx;
    CHAR                            locRlm[RLYLIB_MAX_RLM_LEN];
    ComlibHashKey                   hKey;
    ComlibHashNode                  hNode;
    ComlibLnkNode                   thrdLoopLnkNode;
    UINT                            rule;
    struct {
        union{
            struct { 
                UINT                nxtSndRegHostId;
            }                       rndRbin; /* round-robin */
            struct { 
                UINT                priRegHostId;
            }priSec; /* primary-secondary */
            struct { /* hashing node */
                UINT                hashType; /* HASHING ACT or HASHING ALL */ 
                UINT                hashKeyType; /* STRING, U_32 */ 
                UINT                bitMask; /* for bit hashing*/ 
                RlylibIntRegHostBkt **regHostBkt;
            }hash; /* primary-secondary */
        }u;
    }ruleData;
    UINT                            nxtRcvHostId;
    UINT                            maxRegHostCnt;
    UINT                            regHostCnt;
    ComlibLnkLst                    regHostLst;
    ComlibHashTbl                   regHostHt;
    RlylibIntRegHostBkt             **regHostBkt;
    ThrlibTq                        sndTq;
    ThrlibTq                        ctrlTq;
};

/* for thread process */
struct RlylibIntRlmMainCb{
    ThrlibMutx                      mutx;
    UINT                            maxRlmCnt;
    UINT                            rlmCnt;
    ComlibLnkLst                    rlmLnkLst; 
    RlylibIntMainCb                 *main;
};

struct RlylibIntHostCb{
    UINT                            spId;
    BOOL                            freeTmEnb; /* free time enable value */
    BOOL                            freeTmFlg;
    ULONG                           freeTm;
    BOOL                            dstryFlg; /* destroy now */
    ThrlibCond                      *rdCond; /* main wait for read ptr */
    /* key : [locHost][spId][struct timeval] */
    UINT                            hostKeyLen;
    CHAR                            hostKey[RLYLIB_MAX_HOST_KEY_LEN];
    UINT                            type; /* client, server */
    UINT                            rlyMode; /* RR, AS(Active-Standby) */
    CHAR                            locHost[RLYLIB_MAX_HOST_LEN];
    UINT                            regRlmCnt;
    ComlibLnkLst                    regRlmBktLst; /* rlm bucket list */
    CHAR                            host[RLYLIB_MAX_HOST_LEN];
    RlylibIntTmrInfo                tmrInfo; /* timeout values */
    RlylibIntMsgBufInfo             msgBufInfo;
    ThrlibTq                        freeMsgChkQ;
    ComlibHashKey                   hKey;
    ComlibHashNode                  hNode;
    ComlibLnkNode                   mainLnkNode;
    ComlibLnkNode                   thrdLoopLnkNode;
    UINT                            status; /* open, closed */
    ThrlibMutx                      mutx;
    UINT                            openConnCnt;
    RlylibIntConnCb                 *nxtConn;
    ComlibLnkLst                    connLst;
    UINT                            tmpMsgBufLen;
    CHAR                            tmpMsgBuf[RLYLIB_TMP_MSG_BUF_LEN];
    BOOL                            msgDropIndFlg; /* message drop indication flag */
    RlylibIntMsgChk                 *tmpSndMsgChk; /* for drop */
    ThrlibTq                        *sndTq;
    ThrlibTq                        *rcvTq;
    ThrlibTq                        *ctrlTq; /* control queue */
};

struct RlylibIntTmrHndlNode{
    BOOL                            used;
    UINT                            evnt;
    ULONG                           tmOutTick;
};

struct RlylibIntConnCb{
    ComlibLnkNode                   lnkNode;
    UINT                            fd;
    UINT                            status;
    ULONG                           lstUpdTm; /* last alive time */
    RlylibIntTmrHndlNode            tmrHndlNode;
    TrnlibTransAddr                 dstAddr;
    UINT                            tmpBufLen;
    CHAR                            tmpBuf[RLYLIB_TMP_MSG_BUF_LEN];
    UINT                            remDataLen;
    RlylibIntMsgChk                 *msgChk;
};

struct RlylibIntTmpConnCb{
    UINT                            fd;
    ComlibLnkNode                   lnkNode;
    TrnlibTransAddr                 dstAddr;
    UINT                            hdrBufLen;
    CHAR                            hdrBuf[RLYLIB_MSG_HDR_LEN];
    UINT                            remBufLen;
    UINT                            msgBufLen;
    CHAR                            msgBuf[RLYLIB_TMP_MSG_BUF_LEN];
};

struct RlylibIntAcptCb{
    ComlibLnkNode                   lnkNode;
    RlylibIntAcptThrdCb             *main;
    UINT                            fd;
    TrnlibTransAddr                 srcAddr; /* key */
};

struct RlylibIntAcptThrdCb{
    ThrlibThrdId                    acptTid;
    BOOL                            termSig;
    BOOL                            termFlg;
    ThrlibMutx                      mutx;
    TrnlibSockFdSet                 *rdSockFdSet;
    TrnlibSockFdSet                 *wrSockFdSet;
    TrnlibSockFdSet                 *exSockFdSet;
    UINT                            tmpConnCnt;
    ComlibLnkLst                    acptLnkLst;
    ComlibLnkLst                    actConnLnkLst;
    ComlibLnkLst                    freeConnLnkLst;
};

/* for thread process */
struct RlylibIntHostMainCb{
    ThrlibMutx                      mutx;
    ThrlibCond                      cond;
    BOOL                            termFlg;
    UINT                            actThrdCnt;
    UINT                            maxHostCnt;
    UINT                            hostCnt;
    UINT                            sleepCnt;
    ComlibLnkLst                    hostLnkLst; 
    RlylibIntMainCb                 *main;
    /* select */
};

struct RlylibIntHostThrdCb{
    ThrlibThrdId                    tid;
    UINT                            procCnt;
    BOOL                            tmrUpdFlg; /* timer tick update flag */
    RlylibIntHostMainCb             *main;
};

struct RlylibIntMainMutx{
    ThrlibMutx                      rdMutx;
    ThrlibMutx                      wrMutx;
};

/* mutex tree 
** level 1 : mainCb (Control mutex(add, change, delete))
** level 2 : rlmMainCb, hostMainCb (Thread mutex)
** level 3 : realmCb, hostCb (data control mutex)
** level 4 : thread queue 
*/
struct RlylibIntMainCb{
    UINT                            rlyType; /* client, server, both */
    UINT                            acptType; /* ACPT_ALL, CHK_ACPT_LST */
    /* don't change */
    CHAR                            locHost[RLYLIB_MAX_HOST_LEN];
    RlylibIntMainMutx               mainMutx;
    BOOL                            condWaitFlg;
    ULONG                           waitTm; /* 1ms sec */
    ThrlibCond                      rdCond; /* wait for read */
    RlylibIntAcptThrdCb             acptThrdCb;
    RlylibHostOptArg                dfltHostOptArg;
    /* message control */
    ThrlibTq                        freeMsgChkQ;
    RlylibIntMsgBufInfo             msgBufInfo;
    /* host control */
    ComlibLnkLst                    hostLnkLst;
    ComlibHashTbl                   hostHt;
    UINT                            priHostId;
    UINT                            lastRcvHostId;
    UINT                            lastSndHostId;
    RlylibIntHostMainCb             hostMainCb;
    RlylibIntHostCb                 **hostCb;
    /* rlm control */
    RlylibIntRlmMainCb              rlmMainCb;
    ComlibHashTbl                   rlmHt;
    RlylibIntRlmCb                  **rlmCb;
    UINT                            thrdCnt;
};

struct RlylibIntGlobCb{
    ThrlibMutx                      mutx;
    ComlibTimer                     tmr;
    UINT                            logLvl;
    UINT                            logBufLen;
    CHAR                            logBuf[RLYLIB_MAX_LOG_BUF_LEN];
    RlylibLogFunc                   logFunc;
};

/* intrnal functions */
/* rlylibInt_glob.c */
FT_PUBLIC ULONG                  rlylibInt_globGetCurTick                ();
FT_PUBLIC RT_RESULT              rlylibInt_globUpdCurTick                ();
FT_PUBLIC RT_RESULT              rlylibInt_globGetInitFlg                ();
FT_PUBLIC RT_RESULT              rlylibInt_globGetLogLvl                 ();
FT_PUBLIC RT_RESULT              rlylibInt_globSetLogFunc                (UINT lvl, RlylibLogFunc logFunc);
FT_PUBLIC RT_RESULT              rlylibInt_globLogPrnt                   (UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...);
FT_PUBLIC RT_RESULT              rlylibInt_globInit                      ();

/* rlylibInt_init.c */
FT_PUBLIC RT_RESULT              rlylibInt_initMainCb                    (RlylibIntMainCb *intMainCb, UINT rlyType, 
                                                                          CHAR *locHost, RlylibOptArg *optArg);
FT_PUBLIC RT_RESULT              rlylibInt_initGlob                      ();

/* rlylibInt_dstry.c */
FT_PUBLIC RT_RESULT              rlylibInt_dstryMainCb                   (RlylibIntMainCb *intMainCb);

/* rlylibInt_acpt.c */
FT_PUBLIC VOID                   rlylibInt_acptMainThrd                  (VOID *args);
FT_PUBLIC RT_RESULT              rlylibInt_acptAddAcptLst                (RlylibIntMainCb *intMainCb, TrnlibTransAddr *srcAddr);
FT_PUBLIC RT_RESULT              rlylibInt_acptAddAcptCb                 (RlylibIntAcptThrdCb *acptThrdCb, 
                                                                          TrnlibTransAddr *srcAddr);

/* rlylibInt_sock.c */
FT_PUBLIC RT_RESULT              rlylibInt_sockNtop                      (UINT afnum, CONST VOID *src, CHAR *dst, UINT size);
FT_PUBLIC RT_RESULT              rlylibInt_sockPton                      (UINT afnum, CONST CHAR *src, VOID *dst);
FT_PUBLIC RT_RESULT              rlylibInt_sockClose                     (UINT sockFd);
FT_PUBLIC TrnlibSockFdSet        rlylibInt_sockAllocSockFdSet            ();
FT_PUBLIC RT_RESULT              rlylibInt_sockSelect                    (TrnlibSockFdSet rdSockFdSet,  
                                                                          TrnlibSockFdSet exSockFdSet, 
                                                                          UINT tmOutTick /* 0.1sec */, UINT *rt_evntSockNum);
FT_PUBLIC RT_RESULT              rlylibInt_sockOpenLsn                   (TrnlibTransAddr *srcAddr, UINT *rt_fd); /* listen */
FT_PUBLIC RT_RESULT              rlylibInt_sockFreeSockFdSet             (TrnlibSockFdSet *sockFdSet);
FT_PUBLIC RT_RESULT              rlylibInt_sockAddFdSet                  (TrnlibSockFdSet *sockFdSet,  UINT fd);
FT_PUBLIC RT_RESULT              rlylibInt_sockDelFdSet                  (TrnlibSockFdSet *sockFdSet,  UINT fd);
FT_PUBLIC RT_RESULT              rlylibInt_sockChkFdSet                  (TrnlibSockFdSet *sockFdSet, UINT fd);
FT_PUBLIC RT_RESULT              rlylibInt_sockDfltSockOpt               (UINT sockFd);
FT_PUBLIC RT_RESULT              rlylibInt_sockConnChk                   (TrnlibTransAddr *dstAddr, UINT fd);
FT_PUBLIC RT_RESULT              rlylibInt_sockOpenConn                  (TrnlibTransAddr *dstAddr,  UINT *rt_fd);
FT_PUBLIC RT_RESULT              rlylibInt_sockAcpt                      (UINT lsnFd, UINT *rt_acptFd, TrnlibTransAddr *rt_dstAddr);
FT_PUBLIC RT_RESULT              rlylibInt_sockWrite                     (UINT fd, CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_sockRead                      (UINT fd, UINT rcvLen, CHAR *rt_buf, UINT *rt_bufLen);

/* rlylibInt_host.c */
FT_PUBLIC VOID                   rlylibInt_hostThrdMain                  (VOID *args);
FT_PUBLIC RT_RESULT              rlylibInt_hostDstryAll                  (RlylibIntMainCb *intMainCb);
FT_PUBLIC RT_RESULT              rlylibInt_hostSndCtrlMsg                (RlylibIntHostCb *hostCb, UINT ctrlCode);
FT_PUBLIC RT_RESULT              rlylibInt_hostSetPriHostCb              (RlylibIntMainCb *intMainCb, CHAR *host);
FT_PUBLIC RT_RESULT              rlylibInt_hostMakeHostKey               (CHAR *locHost, UINT locHostLen, U_32 id, 
                                                                          CHAR *rt_hostKey, UINT maxHostKeyLen, 
                                                                          UINT *rt_hostKeyLen);
FT_PUBLIC RlylibIntHostCb*       rlylibInt_hostFindHostCbByHost          (RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen);
FT_PUBLIC RT_RESULT              rlylibInt_hostAcptHostCb                (RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen, 
                                                                          CHAR *hostKey, UINT hostKeyLen, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_hostAddAcptConnCb             (RlylibIntMainCb *intMainCb, UINT hostId, UINT fd, 
                                                                          TrnlibTransAddr *dstAddr);
FT_PUBLIC RT_RESULT              rlylibInt_hostSndFixMsgToAny            (RlylibIntMainCb *intMainCb, CHAR *buf, UINT bufLen, 
                                                                          CHAR *rt_host, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_hostSndFixMsgToPri            (RlylibIntMainCb *intMainCb, CHAR *buf, UINT bufLen, 
                                                                          BOOL faFlg /* failover flag */, CHAR *rt_host, 
                                                                          UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_hostSndMsgToHost              (RlylibIntMainCb *intMainCb, CHAR *host, BOOL dynFlg, 
                                                                          CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_hostSndMsgToHostId            (RlylibIntMainCb *intMainCb, UINT hostId, BOOL dynFlg,
                                                                          CHAR *buf, UINT bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_hostMakeHostCb                (RlylibIntMainCb *intMainCb, CHAR *host, UINT hostLen, 
                                                                          CHAR *hostKey, UINT hostKeyLen, UINT type, 
                                                                          RlylibHostOptArg *hostOptArg, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_hostDstryHostCb               (RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb);
FT_PUBLIC RT_RESULT              rlylibInt_hostMakeConnCb                (RlylibIntHostCb *hostCb, BOOL fdFlg, UINT fd, 
                                                                          TrnlibTransAddr *dstAddr);
FT_PUBLIC RT_RESULT              rlylibInt_hostAddAcptConnCb             (RlylibIntMainCb *intMainCb, UINT hostId, UINT fd, 
                                                                          TrnlibTransAddr *dstAddr);
FT_PUBLIC RT_RESULT              rlylibInt_hostAddConnCb                 (RlylibIntMainCb *intMainCb, CHAR *host, 
                                                                          TrnlibTransAddr *dstAddrs, UINT dstAddrCnt);
FT_PUBLIC RT_RESULT              rlylibInt_hostAddHost                   (RlylibIntMainCb *intMainCb, CHAR *host, UINT type, 
                                                                          UINT hashId, RlylibHostOptArg *hostOptArg, 
                                                                          UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_hostDelHost                   (RlylibIntMainCb *mainCb, CHAR *host);
FT_PUBLIC RT_RESULT              rlylibInt_hostRcvMsgFromHost            (RlylibIntMainCb *intMainCb, CHAR *host, BOOL dynFlg, 
                                                                          RlylibIntRcvMsgRslt *rt_rcvMsgRslt);
FT_PUBLIC RT_RESULT              rlylibInt_hostRcvMsgFromAny             (RlylibIntMainCb *intMainCb, CHAR *rt_host, 
                                                                          UINT *rt_hostId, BOOL dynFlg, 
                                                                          RlylibIntRcvMsgRslt *rt_rcvMsgRslt);
FT_PUBLIC RT_RESULT              rlylibInt_hostRcvFixMsg                 (RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb, 
                                                                          CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_hostRcvDynMsg                 (RlylibIntMainCb *intMainCb, RlylibIntHostCb *hostCb, 
                                                                          CHAR **rt_buf, UINT *rt_bufLen, U_32 *rt_allocBufLen);
FT_PUBLIC RT_RESULT              rlylibInt_hostGetSta                    (RlylibIntMainCb *intMainCb, CHAR *host, UINT *rt_sta);

/* rlylibInt_rlm.c */
FT_PUBLIC RT_RESULT              rlylibInt_rlmSndFixMsgToRlm             (RlylibIntMainCb *intMainCb, CHAR *realm, 
                                                                          RlylibRlmKey *rlmKey, CHAR *buf, UINT bufLen, 
                                                                          CHAR *rt_host, UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_rlmRcvFixMsgFromRlm           (RlylibIntMainCb *intMainCb, CHAR *realm, CHAR *rt_buf, 
                                                                          UINT maxBufLen, UINT *rt_bufLen, CHAR *rt_host, 
                                                                          UINT *rt_hostId);
FT_PUBLIC RT_RESULT              rlylibInt_rlmRegHost                    (RlylibIntMainCb *intMainCb, CHAR *realm, CHAR *host, 
                                                                          BOOL priFlg, RlylibRlmKey *rlmKey);
FT_PUBLIC RT_RESULT              rlylibInt_rlmMakeRlmCb                  (RlylibIntMainCb *intMainCb, CHAR *rlm, UINT rule,
                                                                          RlylibRlmOptArg *rlmOptArg);
FT_PUBLIC RT_RESULT              rlylibInt_rlmAddRlm                     (RlylibIntMainCb *intMainCb, CHAR *realm, UINT rule,
                                                                          RlylibRlmOptArg *rlmOptArg);
FT_PUBLIC RT_RESULT              rlylibInt_rlmDstryAll                   (RlylibIntMainCb *intMainCb);

/* rlylibInt_msg.c */
FT_PUBLIC RT_RESULT              rlylibInt_msgInitMsgBufInfo             (RlylibIntMsgBufInfo *msgBufInfo, UINT maxBufCnt);
FT_PUBLIC RT_RESULT              rlylibInt_msgDstryMsgBufInfo            (RlylibIntMsgBufInfo *msgBufInfo);
FT_PUBLIC RT_RESULT              rlylibInt_msgAllocMsgBuf                (RlylibIntMsgBufInfo*msgBufInfo, UINT size, 
                                                                          VOID **rt_buf, UINT *rt_size);
FT_PUBLIC RT_RESULT              rlylibInt_msgFreeMsgBuf                 (RlylibIntMsgBufInfo *msgBufInfo, VOID *buf, UINT size);
FT_PUBLIC RT_RESULT              rlylibInt_msgEncHBeatInd                (CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_msgDecInitReq                 (CHAR *buf, UINT bufLen, 
                                                                          RlylibIntMsgInitReq *rt_initReq);
FT_PUBLIC RT_RESULT              rlylibInt_msgDecInitAck                 (CHAR *buf, UINT bufLen, 
                                                                          RlylibIntMsgInitAck *rt_initAck);
FT_PUBLIC RT_RESULT              rlylibInt_msgEncInitReq                 (RlylibIntMsgInitReq *initReq, CHAR *rt_buf, 
                                                                          UINT maxBufLen, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT              rlylibInt_msgEncInitAck                 (RlylibIntMsgInitAck *initAck, CHAR *rt_buf, 
                                                                          UINT maxBufLen, UINT *rt_bufLen);

/* rlylibInt_sync.c */
FT_PUBLIC RT_RESULT              rlylibInt_syncMutxInit                  (RlylibIntMainMutx *mainMutx);
FT_PUBLIC ThrlibMutx*            rlylibint_sysnGetMutxPtr                (RlylibIntMainMutx *mainMutx, UINT type);
FT_PUBLIC RT_RESULT              rlylibInt_syncMutxLock                  (RlylibIntMainMutx *mainMutx, UINT type);
FT_PUBLIC RT_RESULT              rlylibInt_syncMutxUnlock                (RlylibIntMainMutx *mainMutx, UINT type);
FT_PUBLIC RT_RESULT              rlylibInt_syncWaitSigCond               (ThrlibCond *cond, ThrlibMutx *mutx, ULONG waitTm);

#ifdef __cplusplus
}
#endif

#endif
