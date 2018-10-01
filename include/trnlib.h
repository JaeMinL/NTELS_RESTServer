/************************************************************************

     Name:     Transport library 

     Type:     C header file

     Desc:     Transport library definition and functions  

     File:     trnlib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _TRNLIB_H_
#define _TRNLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Transport library include */

/*--------------------------------------------------------------*/
/*                 Transport libary definition                  */ 
/*--------------------------------------------------------------*/
/* Transport library log level */
#define TRN_NONE                          0
#define TRN_ERR                           1
#define TRN_NOTY                          2
#define TRN_DBG                           3

/*------- Transport socket definition -------*/

#define TRNLIB_AFNUM_IPV6                 6
#define TRNLIB_AFNUM_IPV4                 4

#define TRNLIB_TRANS_PROTO_TCP            1
#define TRNLIB_TRANS_PROTO_UDP            2
#define TRNLIB_TRANS_PROTO_SCTP           3

#define TRNLIB_STR_TRANS_ADDR_LEN         128

#define TRNLIB_DFLT_LSN_Q                 128

#define TRNLIB_SOCK_LVL_SOCK              1
#define TRNLIB_SOCK_LVL_PROTO_TCP         2

#define TRNLIB_NET_ADDR_IPV4_SIZE         sizeof(U_16) + sizeof(TrnlibIpv4NetAddr)
#define TRNLIB_NET_ADDR_IPV6_SIZE         sizeof(U_16) + sizeof(TrnlibIpv6NetAddr)

#define TRNLIB_OPT_NAME_BROADCAST         1
#define TRNLIB_OPT_NAME_DEBUG             2
#define TRNLIB_OPT_NAME_DONTLINGER        3
#define TRNLIB_OPT_NAME_DONTROUTE         4
#define TRNLIB_OPT_NAME_OOBINLINE         5
#define TRNLIB_OPT_NAME_GRP_PRI           6
#define TRNLIB_OPT_NAME_KEEPALIVE         7
#define TRNLIB_OPT_NAME_LINGER            8
#define TRNLIB_OPT_NAME_RCVBUF            9
#define TRNLIB_OPT_NAME_REUSEADDR         10
#define TRNLIB_OPT_NAME_SNDBUF            11

/* Transport library error code definiation */
/* Socket definition */ 
#define TRNERR_INVALID_SOCK_FD            100
#define TRNERR_INVALID_OPT_VAL            101
#define TRNERR_INVALID_OPT_LEN            102
#define TRNERR_UNKNOWN_LEVEL              103
#define TRNERR_SOCK_FD_IS_FILE            104
#define TRNERR_CONN_INPROGRESS            105
#define TRNERR_CONN_FAILED                106
#define TRNERR_SOCK_TERM                  107
#define TRNERR_AGAIN                      108
#define TRNERR_WOULDBLOCK                 109
#define TRNERR_NOBUFS                     110
#define TRNERR_SOCK_SND_FAILED            111
#define TRNERR_CONN_SOCK_OPEN_FAILED      112
#define TRNERR_SOCK_OPT_FAILED            113
#define TRNERR_INVALID_AFNUM              114
#define TRNERR_SRC_STR_EMPTY              115
#define TRNERR_ACPT_AGAIN                 116
#define TRNERR_ACPT_FAILED                117
#define TRNERR_SOCK_SELECT_TMOUT          118
#define TRNERR_ACPT_ADDR_BIND_FAILED      119
#define TRNERR_ACPT_SOCK_OPEN_FAILED      120
#define TRNERR_ACPT_LISTEN_FAILED         121
#define TRNERR_OUT_OF_SPACE               122

/*--------------------------------------------------------------*/
/*              Transport library macro                         */ 
/*--------------------------------------------------------------*/
/* Transport library log */
#ifdef TRNLIB_LOG
#define TRN_LOG(LEVEL,...){\
	printf(__VA_ARGS__);\
}
#else
#define TRN_LOG(LEVEL,...)
#endif

#define TRNLIB_IS_IPV4_NET_ADDR(_netAddr, _ret){\
    if((_netAddr)->afnum == TRNLIB_AFNUM_IPV4){\
        (_ret) = RC_TRUE;\
    }\
    else {\
        (_ret) = RC_FALSE;\
    }\
}

#define TRNLIB_IS_IPV6_NET_ADDR(_netAddr, _ret){\
    if((_netAddr)->afnum == TRNLIB_AFNUM_IPV6){\
        (_ret) = RC_TRUE;\
    }\
    else {\
        (_ret) = RC_FALSE;\
    }\
}

#define TRNLIB_SET_IPV4_NET_ADDR(_netAddr, _addr){\
    (_netAddr)->afnum = TRNLIB_AFNUM_IPV4;\
    (_netAddr)->u.ipv4NetAddr = (_addr);\
}

#define TRNLIB_SET_IPV6_NET_ADDR(_netAddr, _addr){\
    (_netAddr)->afnum = TRNLIB_AFNUM_IPV6;\
    comlib_memMemcpy((_netAddr)->u.ipv6NetAddr, (_addr), sizeof(TrnlibIpv6NetAddr));\
}

#define TRNLIB_INIT_NET_ADDR(_netAddr){\
    comlib_memMemset((_netAddr), 0x0, sizeof(TrnlibNetAddr));\
}

#define TRNLIB_IS_SAME_NET_ADDR(_addr1, _addr2, _ret){\
    (_ret) = RC_FALSE;\
    if((_addr1)->afnum == (_addr2)->afnum){\
        if((_addr1)->afnum == TRNLIB_AFNUM_IPV4){\
            if((_addr1)->u.ipv4NetAddr == (_addr2)->u.ipv4NetAddr){\
                (_ret) = RC_TRUE;\
            }\
        }\
        else {\
            if(comlib_memMemcmp((_addr1)->u.ipv6NetAddr, (_addr2)->u.ipv6NetAddr, sizeof(TrnlibNetAddr)) == 0){\
                (_ret) = RC_TRUE;\
            }\
        }\
    }\
}

#define TRNLIB_CPY_NET_ADDR(_dst, _src){\
    comlib_memMemcpy(_dst, _src, sizeof(TrnlibNetAddr));\
}

#define TRNLIB_CPY_IPV6ADDR_TO_STR(_str, _maxStrLen, _ptr){\
    trnlib_utilNtop(TRNLIB_AFNUM_IPV6, (CHAR*)(_ptr), (CHAR*)(_str), (_maxStrLen));\
}


#define TRNLIB_CPY_IPV4ADDR_TO_STR(_str, _maxStrLen, _addr){\
    comlib_strSNPrnt((CHAR*)(_str),(UINT)(_maxStrLen), "%d.%d.%d.%d", \
                     ((_addr) & 0xff000000)>> 24, \
                     ((_addr) & 0x00ff0000)>> 16,\
                     ((_addr) & 0x0000ff00)>> 8,\
                     ((_addr) & 0x000000ff));\
}

#define TRNLIB_CPY_IPADDR_TO_STR(_str, _maxStrLen, _addr){\
    if((_maxStrLen) < 15){\
        _str[0] = '\0';\
    }\
    else {\
        if((_addr)->afnum == TRNLIB_AFNUM_IPV6){\
            comlib_memMemcpy((VOID*)(_str), (CONST VOID*)"IPv6:",5);\
            TRNLIB_CPY_IPV6ADDR_TO_STR(&(_str[5]), (_maxStrLen)-5, (_addr)->u.ipv6NetAddr);\
        }\
        else {/* IPV4 */\
            comlib_memMemcpy((VOID*)(_str), (CONST VOID*)"IPv4:",5);\
            TRNLIB_CPY_IPV4ADDR_TO_STR(&(_str[5]), (_maxStrLen)-5, (_addr)->u.ipv4NetAddr);\
        }\
    }\
}

#define TRNLIB_CPY_TRANS_ADDR_TO_STR(_str, _maxStrLen, _transAddr){\
    UINT d_strLen = 0;\
    TRNLIB_CPY_IPADDR_TO_STR((_str), (_maxStrLen), &(_transAddr)->netAddr);\
    d_strLen = comlib_strGetLen((_str));\
    if(d_strLen < (_maxStrLen)){\
        comlib_strSNPrnt(&(_str)[d_strLen], (_maxStrLen) - d_strLen ," PORT=%d",(_transAddr)->port);\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRNLIB_H_ */
