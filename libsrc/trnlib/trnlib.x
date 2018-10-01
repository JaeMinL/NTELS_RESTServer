/************************************************************************

     Name:     Transport library 

     Type:     C structure file

     Desc:     Transport library structure

     File:     trnlib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _TRNLIB_X_
#define _TRNLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

/* transport library data type */
typedef U_32                        TrnlibIpv4NetAddr;
typedef U_8                         TrnlibIpv6Addr[16];       /* 16 byte Ipv6 Addr */
typedef TrnlibIpv6Addr              TrnlibIpv6NetAddr;

typedef struct TrnlibNetAddr        TrnlibNetAddr;
typedef struct TrnlibTransAddr      TrnlibTransAddr;

#if 0
typedef struct TrnlibSockFdSet      *TrnlibSockFdSet;
#else
typedef struct TrnlibSockFdSet      TrnlibSockFdSet;
#endif

struct TrnlibNetAddr{
        U_16                        afnum;
        union{
                TrnlibIpv4NetAddr   ipv4NetAddr;
                TrnlibIpv6NetAddr   ipv6NetAddr;
        }u;
};

struct TrnlibTransAddr{
        U_16                        port;
        UINT                        proto;
        TrnlibNetAddr               netAddr;
};

/* socket functions */
#ifdef __cplusplus
FT_PUBLIC TrnlibSockFdSet* trnlib_sockAllocSockFdSet     ();
#else
FT_PUBLIC TrnlibSockFdSet* trnlib_sockAllocSockFdSet     (VOID);
#endif
FT_PUBLIC RT_RESULT        trnlib_sockFreeSockFdSet      (TrnlibSockFdSet *sockFdSet);
FT_PUBLIC RT_RESULT        trnlib_sockAddFdSet           (TrnlibSockFdSet *sockFdSet, UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockDelFdSet           (TrnlibSockFdSet *sockFdSet, UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockChkFdSet           (TrnlibSockFdSet *sockFdSet, UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockClose              (UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockSetOpt             (UINT fd, SINT lvl, SINT optName, VOID *optVal, UINT optLen);
FT_PUBLIC RT_RESULT        trnlib_sockNonSetLingerFlg    (UINT fd, BOOL onOffFlg, UINT linger);
FT_PUBLIC RT_RESULT        trnlib_sockSetNonBlkMode      (UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockDfltSockOpt        (UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockConnChk            (TrnlibTransAddr *dstAddr, UINT fd);
FT_PUBLIC RT_RESULT        trnlib_sockRead               (UINT fd, UINT rcvLen, CHAR *rt_buf, UINT *rt_bufLen);
FT_PUBLIC RT_RESULT        trnlib_sockWrite              (UINT fd, BOOL sndWaitFlg, CHAR *buf, UINT bufLen, UINT *rt_remBufLen);
FT_PUBLIC RT_RESULT        trnlib_sockSetLsnSock         (UINT fd, UINT maxLsnQ, BOOL dfltSockOptFlg);
FT_PUBLIC RT_RESULT        trnlib_sockOpenLsnSock        (TrnlibTransAddr *srcAddr, UINT *rt_fd, UINT maxLsnQ, 
                                                          BOOL dfltSockOptFlg);
FT_PUBLIC RT_RESULT        trnlib_sockOpenConn           (TrnlibTransAddr *dstAddr,  UINT *rt_fd, BOOL dfltSockOptFlg);
FT_PUBLIC RT_RESULT        trnlib_sockSelect             (TrnlibSockFdSet *rdSockFdSet,  TrnlibSockFdSet *exSockFdSet, 
                                                          UINT tmOutTick /* 0.1 sec */, UINT *rt_evntSockNum);
FT_PUBLIC RT_RESULT        trnlib_sockAcpt               (UINT lsnFd, UINT *rt_acptFd, TrnlibTransAddr *rt_dstAddr, 
                                                          BOOL dfltSockOptFlg);

/* util functions */
FT_PUBLIC RT_RESULT        trnlib_utilPton               (UINT afnum, CONST CHAR *src, VOID *dst);
FT_PUBLIC RT_RESULT        trnlib_utilNtop               (UINT afnum, CONST VOID *src, CHAR *dst, UINT size);
FT_PUBLIC U_32             trnlib_utilHtonl              (U_32 host);
FT_PUBLIC U_32             trnlib_utilNtohl              (U_32 net);
FT_PUBLIC U_16             trnlib_utilHtons              (U_16 host);
FT_PUBLIC U_16             trnlib_utilNtohs              (U_16 net);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRNLIB_X_ */

