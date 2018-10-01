#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "trnlib.h"
#include "trnlib.x"

/* Private Socket defininition */
#define SOCK_SNDBUFF_LEN            8*1024*1024
#define SOCK_RCVBUFF_LEN            8*1024*1024

typedef union SockAddrIn     SockAddrIn; 

union SockAddrIn{
    struct sockaddr_in          sockAddr;
    struct sockaddr_in6         sockAddr6;
};

/* private declear */
struct TrnlibSockFdSet{
    UINT                        fdNum;
    fd_set                      set;
};

FT_PRIVATE RT_RESULT sock_getSockOptLvl(UINT optLvl, SINT *rt_rslt);
FT_PRIVATE RT_RESULT sock_getSockOptName(UINT optName, SINT *rt_rslt);

FT_PRIVATE RT_RESULT sock_getSockOptLvl(UINT optLvl, SINT *rt_rslt)
{
    switch(optLvl){
        case TRNLIB_SOCK_LVL_SOCK:      *rt_rslt = SOL_SOCKET;     break;
        case TRNLIB_SOCK_LVL_PROTO_TCP: *rt_rslt = IPPROTO_TCP;    break;
        default:
                                        TRN_LOG(TRN_ERR,"Invalid Option level(%d)\n",optLvl);
                                        return RC_NOK;
    };

    return RC_OK;
}

FT_PRIVATE RT_RESULT sock_getSockOptName(UINT optName, SINT *rt_rslt)
{
    switch(optName){
        case TRNLIB_OPT_NAME_BROADCAST:  *rt_rslt = SO_BROADCAST;      break;
        case TRNLIB_OPT_NAME_DEBUG:      *rt_rslt = SO_DEBUG;          break;
        case TRNLIB_OPT_NAME_DONTROUTE:  *rt_rslt = SO_DONTROUTE;      break;
        case TRNLIB_OPT_NAME_OOBINLINE:  *rt_rslt = SO_OOBINLINE;      break;
        case TRNLIB_OPT_NAME_KEEPALIVE:  *rt_rslt = SO_KEEPALIVE;      break;
        case TRNLIB_OPT_NAME_LINGER:     *rt_rslt = SO_LINGER;         break;
        case TRNLIB_OPT_NAME_RCVBUF:     *rt_rslt = SO_RCVBUF;         break;
        case TRNLIB_OPT_NAME_REUSEADDR:  *rt_rslt = SO_REUSEADDR;      break;
        case TRNLIB_OPT_NAME_SNDBUF:     *rt_rslt = SO_SNDBUF;         break;
        default:
                                     TRN_LOG(TRN_ERR,"Invalid Option Name(%d)\n",optName);
                                     return RC_NOK;
    };

    return RC_OK;
}


FT_PUBLIC TrnlibSockFdSet* trnlib_sockAllocSockFdSet()
{
    TrnlibSockFdSet *sockFdSet = NULL;

    sockFdSet = comlib_memMalloc(sizeof(struct TrnlibSockFdSet));

    sockFdSet->fdNum = 0;

    FD_ZERO(&sockFdSet->set);

    return sockFdSet;
}

FT_PUBLIC RT_RESULT trnlib_sockFreeSockFdSet(TrnlibSockFdSet *sockFdSet)
{
    comlib_memFree(sockFdSet);

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockAddFdSet(TrnlibSockFdSet *sockFdSet, UINT fd)
{
    FD_SET(fd, &sockFdSet->set);

    if(fd >= sockFdSet->fdNum){
        sockFdSet->fdNum = fd + 1;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockDelFdSet(TrnlibSockFdSet *sockFdSet, UINT fd)
{
    FD_CLR(fd, &sockFdSet->set);

    if((fd+1) == sockFdSet->fdNum){
        sockFdSet->fdNum--;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockChkFdSet(TrnlibSockFdSet *sockFdSet, UINT fd)
{
    SINT ret = RC_OK;
    ret = FD_ISSET(fd, &sockFdSet->set);
    if(ret == 0){
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockClose(UINT fd)
{
    SINT ret = RC_OK;

    TRN_LOG(TRN_ERR,"Socket close(fd=%d)\n",fd);

    ret = close(fd);
    if(ret == -1){
        TRN_LOG(TRN_ERR,"socket close error(ret=%d, errno=%d:%s)\n",ret, errno,strerror(errno));
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockSetOpt(UINT fd, SINT lvl, SINT optName, VOID *optVal, UINT optLen)
{
    SINT ret = RC_OK;

    ret = setsockopt(fd, lvl, optName, optVal, optLen);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"socket option setting failed(ret=%d, err=%d(%s))\n",ret, errno, strerror(errno));
        if(ret == EBADF){
            return TRNERR_INVALID_SOCK_FD;
        }
        else if(ret == EFAULT){
            return TRNERR_INVALID_OPT_VAL;
        }
        else if(ret == EINVAL){
            return TRNERR_INVALID_OPT_LEN;
        }
        else if(ret == ENOPROTOOPT){
            return TRNERR_UNKNOWN_LEVEL;
        }
        else if(ret == ENOTSOCK){
            return TRNERR_SOCK_FD_IS_FILE;
        }

    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockNonSetLingerFlg(UINT fd, BOOL onOffFlg, UINT linger)
{
    SINT ret = RC_OK;
    SINT optLvl = 0;
    SINT optName = 0;
    struct linger lngrOpt;

    ret = sock_getSockOptLvl(TRNLIB_SOCK_LVL_SOCK, &optLvl);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option level(ret=%d, lvl=%d)\n",ret, TRNLIB_SOCK_LVL_SOCK);
        return RC_NOK;
    }

    ret = sock_getSockOptName(TRNLIB_OPT_NAME_LINGER, &optName);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option Name(ret=%d, lvl=%d)\n",ret, TRNLIB_OPT_NAME_REUSEADDR);
        return RC_NOK;
    }

    if(onOffFlg == RC_TRUE){
        lngrOpt.l_onoff  = 1;
    }
    else {
        lngrOpt.l_onoff  = 0;
    }

    lngrOpt.l_linger = linger;

    ret = trnlib_sockSetOpt(fd, optLvl, optName, (VOID*) &lngrOpt, sizeof(struct linger));
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"socket option setting failed(ret=%d, lvl=%d, name=%d)\n",ret, optLvl, optName); 
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockSetNonBlkMode(UINT fd)
{
    SINT ret = RC_OK;
    SINT sockOpt = 0;

    sockOpt = fcntl(fd, F_GETFL, 0);
    if (sockOpt < 0){
        TRN_LOG(TRN_ERR,"F_GETFL error(fd=%d, err=%d(%s))\n", fd, errno, strerror(errno));
        return RC_NOK;
    }

    sockOpt |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, sockOpt);
    if(ret < 0){
        TRN_LOG(TRN_ERR,  "F_SETFL error(fd=%d err=%d(%s))\n", fd, errno, strerror(errno));
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockDfltSockOpt(UINT fd)
{
    SINT ret = RC_OK;
    SINT optLvl = 0;
    SINT optName = 0;
    SINT sockOpt = 0;
    SINT buffSize = 0;

    /* ip address reuse flag set (binding) */
    ret = sock_getSockOptLvl(TRNLIB_SOCK_LVL_SOCK, &optLvl);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option level(ret=%d, lvl=%d)\n",ret, TRNLIB_SOCK_LVL_SOCK);
        return RC_NOK;
    }

    ret = sock_getSockOptName(TRNLIB_OPT_NAME_REUSEADDR, &optName);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option Name(ret=%d, lvl=%d)\n",ret, TRNLIB_OPT_NAME_REUSEADDR);
        return RC_NOK;
    }

    sockOpt = 1;
    ret = trnlib_sockSetOpt(fd, optLvl, optName, (VOID*) &sockOpt, sizeof(SINT));
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"socket option setting failed(ret=%d, lvl=%d, name=%d)\n",ret, optLvl, optName); 
        return RC_NOK;
    }

    /* non-blocking mode set */
    ret = trnlib_sockSetNonBlkMode(fd);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Non-blocking mode setting failed(ret=%d)\n",ret);
        return RC_NOK;
    }

    /* SO_LINGER flag set (prevent CLOSE_WAIT) */
    ret = trnlib_sockNonSetLingerFlg(fd, RC_TRUE, 0);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,  "Linger flag setting failed(ret=%d, fd=%d, err=%d(%s))\n", 
                ret, fd, errno, strerror(errno));
        return RC_NOK;
    }

    /* send buffer size set */
    ret = sock_getSockOptLvl(TRNLIB_SOCK_LVL_SOCK, &optLvl);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option level(ret=%d, lvl=%d)\n",ret, TRNLIB_SOCK_LVL_SOCK);
        return RC_NOK;
    }

    ret = sock_getSockOptName(TRNLIB_OPT_NAME_SNDBUF, &optName);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option Name(ret=%d, lvl=%d)\n",ret, TRNLIB_OPT_NAME_SNDBUF);
        return RC_NOK;
    }

    buffSize = SOCK_SNDBUFF_LEN;
    ret = trnlib_sockSetOpt(fd, optLvl, optName, (VOID*) &buffSize, sizeof(SINT));
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"socket option setting failed(ret=%d, lvl=%d, name=%d)\n",ret, optLvl, optName); 
        return RC_NOK;
    }

    /* receive buffer size set */
    ret = sock_getSockOptName(TRNLIB_OPT_NAME_RCVBUF, &optName);
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"Invalid option Name(ret=%d, lvl=%d)\n",ret, TRNLIB_OPT_NAME_RCVBUF);
        return RC_NOK;
    }

    buffSize = SOCK_RCVBUFF_LEN;
    ret = trnlib_sockSetOpt(fd, optLvl, optName, (VOID*) &buffSize, sizeof(SINT));
    if(ret != RC_OK){
        TRN_LOG(TRN_ERR,"socket option setting failed(ret=%d, lvl=%d, name=%d)\n",ret, optLvl, optName); 
        return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockConnChk(TrnlibTransAddr *dstAddr, UINT fd)
{
    SINT ret = RC_OK;
    struct sockaddr_in sockAddr;
    struct sockaddr_in6 sockAddr6;
    VOID *sockAddrPtr = NULL;
    UINT sockAddrLen = 0;
    CHAR strAddr[TRNLIB_STR_TRANS_ADDR_LEN];

    if(dstAddr->netAddr.afnum == TRNLIB_AFNUM_IPV6){
        comlib_memMemset((CHAR*)&sockAddr6, 0x0, sizeof(struct sockaddr_in6));
        sockAddr6.sin6_family = AF_INET6;
        sockAddr6.sin6_port = htons(dstAddr->port);
        ret = comlib_memMemcmp(dstAddr->netAddr.u.ipv6NetAddr, &sockAddr6.sin6_addr, sizeof(TrnlibIpv6NetAddr));
        if(ret != 0){
            comlib_memMemcpy(&sockAddr6.sin6_addr, dstAddr->netAddr.u.ipv6NetAddr, sizeof(TrnlibIpv6NetAddr));
        }
        else {
            sockAddr6.sin6_addr = in6addr_any;
        }
        sockAddrLen = sizeof(struct sockaddr_in6);
        sockAddrPtr = (VOID*)&sockAddr6;
    }
    else {
        comlib_memMemset((CHAR*)&sockAddr, 0x0, sizeof(struct sockaddr_in));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(dstAddr->port);
        if(dstAddr->netAddr.u.ipv4NetAddr == 0){
            sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else {
            sockAddr.sin_addr.s_addr = htonl(dstAddr->netAddr.u.ipv4NetAddr);
        }
        sockAddrLen = sizeof(struct sockaddr_in);
        sockAddrPtr = (void*)&sockAddr;
    }

    ret = connect(fd, sockAddrPtr, sockAddrLen);
    if(ret == -1){
        if(errno == EISCONN){
            return RC_OK;
        }
        else if( errno == EINPROGRESS ||
                errno == EALREADY) {
            return TRNERR_CONN_INPROGRESS;
        }
        else{
            TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, TRNLIB_STR_TRANS_ADDR_LEN, dstAddr);
            TRN_LOG(TRN_ERR,"Socket connect failed(addr=%s, ret=%d, errno=%d:%s)\n",
                    strAddr, ret, errno, strerror(errno));
            return TRNERR_CONN_FAILED;
        }

    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockRead(UINT fd, UINT rcvLen, CHAR *rt_buf, UINT *rt_bufLen)
{
    SINT bufLen = 0;

    bufLen = read(fd, rt_buf, rcvLen);
    if (bufLen < 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)){
            *rt_bufLen = 0;
            return RC_OK;
        }
        else{
            *rt_bufLen = 0;
            TRN_LOG(TRN_ERR,"socket read error(errno=%d:%s)\n", errno, strerror(errno));
            return RC_NOK;
        }
    }
    else if (bufLen == 0){
        *rt_bufLen = 0;
        return TRNERR_SOCK_TERM;
    }

    *rt_bufLen = bufLen;

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockWrite(UINT fd, BOOL sndWaitFlg, CHAR *buf, UINT bufLen, UINT *rt_remBufLen)
{
    SINT ret = RC_OK;
    UINT total = 0;
    SINT sndLen= 0;
    UINT remLen= 0;

    total = bufLen;
    sndLen = bufLen;
    remLen = bufLen;

    if(rt_remBufLen != NULL){
        *rt_remBufLen = 0;
    }

    while(1){
        sndLen = write(fd, (CHAR*) &buf[total-sndLen], remLen);
        if (sndLen == -1) {
            if (errno == EINTR) {
                TRN_LOG(TRN_ERR, " write socket EINTR error=%d\n", errno );
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS) {
                if(sndWaitFlg == RC_TRUE){
                    usleep(100);
                    continue;
                }
                else {
                    switch(errno){
                        case EAGAIN:         ret = TRNERR_AGAIN;           break;
#if 0 /* EAGAIN and EWOULDBLOCK is same */
                        case EWOULDBLOCK:    ret = TRNERR_WOULDBLOCK;      break;
#endif
                        case ENOBUFS:        ret = TRNERR_NOBUFS;          break;
                        default:             
                                             {
                                                 TRN_LOG(TRN_ERR,"Unknown errno(%d(%s))\n", errno, strerror(errno));
                                                 ret = TRNERR_SOCK_SND_FAILED; break;
                                             }
                    }

                    if(rt_remBufLen != NULL){
                        *rt_remBufLen = remLen;
                    }

                    return ret;
                }
            }/* end of else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS) */
            else {
                TRN_LOG(TRN_ERR, " write socket fail(%d: %s)\n", errno, strerror(errno));
                return RC_NOK;
            }
        }
        else if (sndLen < remLen)
        {
            remLen -= sndLen;
            continue;
        }
        else if(sndLen == remLen){
            return RC_OK;
        }
    }/* end of while(1) */
}

FT_PUBLIC RT_RESULT trnlib_sockSetLsnSock(UINT fd, UINT maxLsnQ, BOOL dfltSockOptFlg)
{
    SINT ret = RC_OK;
    UINT lsnQ = 0;

    if(maxLsnQ == 0){
        lsnQ = TRNLIB_DFLT_LSN_Q;
    }
    else {
        lsnQ = maxLsnQ;
    }

    if(dfltSockOptFlg == RC_TRUE){
        /* set socket option */
        ret = trnlib_sockDfltSockOpt(fd);
        if(ret < 0){
            TRN_LOG(TRN_ERR,"socket option setting failed(fd=%d, errno=%d(%s))\n", fd, errno, strerror(errno));
            return TRNERR_SOCK_OPT_FAILED;
        }
    }

    /* listen socket */
    ret = listen(fd, lsnQ);
    if(ret < 0){
        TRN_LOG(TRN_ERR,"socket listen failed(fd=%d, errno=%d(%s))\n", fd, errno, strerror(errno));
        return TRNERR_ACPT_LISTEN_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockOpenLsnSock(TrnlibTransAddr *srcAddr, UINT *rt_fd, UINT maxLsnQ, BOOL dfltSockOptFlg)
{
    SINT ret = RC_OK;
    SINT fd = 0;
    struct sockaddr_in sockAddr;
    struct sockaddr_in6 sockAddr6;
    VOID *sockAddrPtr = NULL;
    UINT sockAddrLen = 0;
    CHAR strAddr[TRNLIB_STR_TRANS_ADDR_LEN];

    /* open socket */
    if(srcAddr->netAddr.afnum == TRNLIB_AFNUM_IPV4){
        fd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    }
    else if(srcAddr->netAddr.afnum == TRNLIB_AFNUM_IPV6){
        fd = socket(AF_INET6,SOCK_STREAM, IPPROTO_TCP);
    }

    if(fd < 0){
        TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, TRNLIB_STR_TRANS_ADDR_LEN, srcAddr);
        TRN_LOG(TRN_ERR,"socket open failed(addr=%s, errno=%d(%s))\n",strAddr,errno, strerror(errno));
        return TRNERR_ACPT_SOCK_OPEN_FAILED;
    }

    /* bind socket */
    if(srcAddr != NULL){
        if(srcAddr->netAddr.afnum == TRNLIB_AFNUM_IPV6){
            comlib_memMemset((CHAR*)&sockAddr6, 0x0, sizeof(struct sockaddr_in6));
            sockAddr6.sin6_family = AF_INET6;
            sockAddr6.sin6_port = htons(srcAddr->port);
            ret = comlib_memMemcmp(srcAddr->netAddr.u.ipv6NetAddr, &sockAddr6.sin6_addr, sizeof(TrnlibIpv6NetAddr));
            if(ret != 0){
                comlib_memMemcpy(&sockAddr6.sin6_addr, srcAddr->netAddr.u.ipv6NetAddr, sizeof(TrnlibIpv6NetAddr));
            }
            else {
                sockAddr6.sin6_addr = in6addr_any;
            }
            sockAddrLen = sizeof(struct sockaddr_in6);
            sockAddrPtr = (VOID*)&sockAddr6;
        }
        else {
            comlib_memMemset((CHAR*)&sockAddr, 0x0, sizeof(struct sockaddr_in));
            sockAddr.sin_family = AF_INET;
            sockAddr.sin_port = htons(srcAddr->port);
            if(srcAddr->netAddr.u.ipv4NetAddr == 0){
                sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            }
            else {
                sockAddr.sin_addr.s_addr = htonl(srcAddr->netAddr.u.ipv4NetAddr);
            }
            sockAddrLen = sizeof(struct sockaddr_in);
            sockAddrPtr = (void*)&sockAddr;
        }

        ret = bind(fd, sockAddrPtr, sockAddrLen);
        if(ret < 0){
            TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, TRNLIB_STR_TRANS_ADDR_LEN, srcAddr);
            TRN_LOG(TRN_ERR,"socket bind failed(addr=%s, errno=%d(%s))\n",strAddr, errno, strerror(errno));
            trnlib_sockClose(fd);
            return TRNERR_ACPT_ADDR_BIND_FAILED;
        }
    }/* if(srcAddr != NULL) */

    ret = trnlib_sockSetLsnSock(fd, maxLsnQ, dfltSockOptFlg);
    if(ret != RC_OK){
        TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, TRNLIB_STR_TRANS_ADDR_LEN, srcAddr);
        TRN_LOG(TRN_ERR,"Socket liston failed(ret=%d, addr=%s)\n", ret, strAddr);
        trnlib_sockClose(fd);
        return ret;
    }

    *rt_fd = fd;

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockOpenConn(TrnlibTransAddr *dstAddr,  UINT *rt_fd, BOOL dfltSockOptFlg)
{
    SINT ret = RC_OK;
    SINT fd = 0;

    /* open socket */
    if(dstAddr->netAddr.afnum == TRNLIB_AFNUM_IPV4){
        fd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    }
    else if(dstAddr->netAddr.afnum == TRNLIB_AFNUM_IPV6){
        fd = socket(AF_INET6,SOCK_STREAM, IPPROTO_TCP);
    }

    if(fd < 0){
        CHAR strAddr[TRNLIB_STR_TRANS_ADDR_LEN];

        TRNLIB_CPY_TRANS_ADDR_TO_STR(strAddr, TRNLIB_STR_TRANS_ADDR_LEN, dstAddr);
        TRN_LOG(TRN_ERR,"socket open failed(addr=%s, errno=%d(%s))\n",strAddr,errno, strerror(errno));
        return TRNERR_CONN_SOCK_OPEN_FAILED;
    }

    if(dfltSockOptFlg == RC_TRUE){
        ret = trnlib_sockDfltSockOpt(fd);
        if(ret !=  RC_OK){
            TRN_LOG(TRN_ERR,"Default socket option setting failed(ret=%d)\n",ret);
            trnlib_sockClose(fd);
            return TRNERR_SOCK_OPT_FAILED;
        }
    }

    *rt_fd = fd;

    ret = trnlib_sockConnChk(dstAddr, fd);
    if(ret != RC_OK && ret != TRNERR_CONN_INPROGRESS){
        trnlib_sockClose(fd);
    }

    return ret;
}

FT_PUBLIC RT_RESULT trnlib_sockSelect(TrnlibSockFdSet *rdSockFdSet,  TrnlibSockFdSet *exSockFdSet, 
        UINT tmOutTick /* 0.1 sec */, UINT *rt_evntSockNum)
{
    SINT ret = RC_OK;
    fd_set rdSet;
    fd_set exSet;
    fd_set *pRdSet = NULL;
    fd_set *pExSet = NULL;
    struct timeval tvTmOut;

    if(tmOutTick >= 10){
        tvTmOut.tv_sec = tmOutTick / 10;
        tvTmOut.tv_usec = (tmOutTick % 10) * 100000;
    }
    else if(tmOutTick != 0){
        tvTmOut.tv_sec = 0;
        tvTmOut.tv_usec = tmOutTick * 100000;
    }
    else {
        tvTmOut.tv_sec = 0;
        tvTmOut.tv_usec = 0;
    }

    if(exSockFdSet != NULL){
        if(rdSockFdSet->fdNum != exSockFdSet->fdNum){
            TRN_LOG(TRN_ERR,"Invalid fd num(%d, %d)\n", rdSockFdSet->fdNum, exSockFdSet->fdNum);
            return RC_NOK;
        }
    }

    if(rdSockFdSet != NULL){
        comlib_memMemcpy(&rdSet, &rdSockFdSet->set, sizeof(fd_set));
        pRdSet = &rdSet;
    }

    if(exSockFdSet != NULL){
        comlib_memMemcpy(&exSet, &exSockFdSet->set, sizeof(fd_set));
        pExSet = &exSet;
    }

    ret = select(rdSockFdSet->fdNum, pRdSet, NULL, pExSet, &tvTmOut);
    if(ret < 0){
        TRN_LOG(TRN_ERR,"Select failed(ret=%d, err=%d:%s)\n",ret,errno, strerror(errno));
        return RC_NOK;
    }
    else if(ret == 0){
        return TRNERR_SOCK_SELECT_TMOUT;
    }

    if(rt_evntSockNum != NULL){
        *rt_evntSockNum = ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_sockAcpt(UINT lsnFd, UINT *rt_acptFd, TrnlibTransAddr *rt_dstAddr, BOOL dfltSockOptFlg)
{
    SINT ret = RC_OK;
    SINT acptFd = 0;
    UINT len = 0;
    struct sockaddr_in *sockAddrPtr = NULL;
    struct sockaddr_in6 *sockAddr6Ptr = NULL;
    SockAddrIn sockAddrIn;

    sockAddrPtr = &sockAddrIn.sockAddr;
    sockAddr6Ptr = &sockAddrIn.sockAddr6;

    acptFd = accept(lsnFd, (struct sockaddr*)&sockAddrIn, (socklen_t*)&len);
    if(acptFd < 0){
        if(errno == EAGAIN){
            return TRNERR_ACPT_AGAIN;
        }
        TRN_LOG(TRN_ERR,"accept failed(errno=%d(%s))\n",errno,strerror(errno));
        return TRNERR_ACPT_FAILED;
    }

    if(len == sizeof(struct sockaddr_in)){ /* IPv4 */
        rt_dstAddr->port = sockAddrPtr->sin_port;
        rt_dstAddr->proto = TRNLIB_TRANS_PROTO_TCP;
        rt_dstAddr->netAddr.afnum = TRNLIB_AFNUM_IPV4;
        rt_dstAddr->netAddr.u.ipv4NetAddr = ntohl(sockAddrPtr->sin_addr.s_addr);
    }
    else { /* IPv6 */
        rt_dstAddr->port = sockAddr6Ptr->sin6_port;
        rt_dstAddr->proto = TRNLIB_TRANS_PROTO_TCP;
        rt_dstAddr->netAddr.afnum = TRNLIB_AFNUM_IPV6;
        comlib_memMemcpy(rt_dstAddr->netAddr.u.ipv6NetAddr, &sockAddr6Ptr->sin6_addr,sizeof(TrnlibIpv6NetAddr));
    }

    if(dfltSockOptFlg == RC_TRUE){
        ret = trnlib_sockDfltSockOpt(acptFd);
        if(ret != RC_OK){
            TRN_LOG(TRN_ERR,"Default socket option setting failed(ret=%d)\n",ret);
            return TRNERR_SOCK_OPT_FAILED;
        }
    }

    *rt_acptFd = acptFd;

    return RC_OK;
}

