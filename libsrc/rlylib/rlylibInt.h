
/************************************************************************

     Name:     Relay library 

     Type:     C header file

     Desc:     Relay internal library definition and functions  

     File:     rlylibInt.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RLYLIB_INT_H_
#define _RLYLIB_INT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RLYLIB_DFLT_LSN_Q                     100 /* socket listen queue size */

#define RLYLIB_MAX_LOG_BUF_LEN                4096

#define RLYLIB_STR_TRANS_ADDR_LEN             128

#define RLYLIB_MSG_HDR_LEN                    (sizeof(U_32) * 3)

#if 0
#define RLYLIB_HOST_STA_OPEN                  1
#define RLYLIB_HOST_STA_CLOSED                2
#endif

#define RLYLIB_CONN_STA_CLOSED                0
#define RLYLIB_CONN_STA_CONNECTED             1
#define RLYLIB_CONN_STA_INIT                  2
#define RLYLIB_CONN_STA_OPEN                  3

#define RLYLIB_MAX_FD_CNT                     5
#define RLYLIB_DFLT_MAX_HOST_CNT              256
#define RLYLIB_DFLT_MAX_RLM_CNT               256

#define RLYLIB_DFLT_MAX_TQ_SIZE               500000

#define RLYLIB_DFLT_FREE_CHK_Q_SIZE           65536

#define RLYLIB_DFLT_MAX_CONN_CNT              64

#define RLYLIB_DIST_DHT_DFLT_RNG              256

#define RLYLIB_DFLT_MAX_REG_HOST_CNT          256

#define RLYLIB_MAX_HOST_KEY_LEN               256

#define RLYLIB_SND_LOOP_CNT                   7
#define RLYLIB_TMP_MSG_BUF_LEN                16384

#define RLYLIB_THRD_POOL_DFLT_STRM            5

#define RLYLIB_DFLT_HOST_MSG_BUF_CNT          20960
#define RLYLIB_DFLT_MAIN_MSG_BUF_CNT          31920

#define RLYLIB_MAX_USLEEP_US                  700
#define RLYLIB_INC_USLEEP_US                  5
#define RLYLIB_MIN_USLEEP_US                  5

#define RLYLIB_MSG_BUF_CNT                    3
#define RLYLIB_MSG_512_BUF_IDX                0
#define RLYLIB_MSG_1024_BUF_IDX               1
#define RLYLIB_MSG_2048_BUF_IDX               2

#define RLYLIB_MSG_512_BUF_SIZE               512
#define RLYLIB_MSG_1024_BUF_SIZE              1024
#define RLYLIB_MSG_2048_BUF_SIZE              2048

#define RLYLIB_DFLT_TMR_EVNT_CONN_TMOUT       30
#define RLYLIB_DFLT_TMR_EVNT_CONN_WAIT_TMOUT  30
#define RLYLIB_DFLT_TMR_EVNT_INIT_TMOUT       30
#define RLYLIB_DFLT_TMR_EVNT_HBEAT_SND_TMOUT  30
#define RLYLIB_DFLT_TMR_EVNT_HBEAT_TMOUT      90
#define RLYLIB_DFLT_TMR_EVNT_MSG_DROP_TMOUT   0
#define RLYLIB_DFLT_TMR_EVNT_FREE_TMOUT       50

#define RLYLIB_MSG_CODE_INIT_REQ              1
#define RLYLIB_MSG_CODE_INIT_ACK              2
#define RLYLIB_MSG_CODE_DATA                  3
#define RLYLIB_MSG_CODE_HB                    4
#define RLYLIB_MSG_CODE_DROP_DATA             5 /* internal message code */

/* control message code */
#define RLYLIB_CTRL_MSG_CODE_DSTRY_HOST       1 /* internal message code */

#define RLYLIB_AVP_CODE_HOST_KEY              1
#define RLYLIB_AVP_CODE_HOST                  2
#define RLYLIB_AVP_CODE_RESULT                3

#define RLYLIB_RSLT_CODE_SUCC                 1

#define RLYLIB_CONN_TMR_EVNT_NONE             0
#define RLYLIB_CONN_TMR_EVNT_CONN             1
#define RLYLIB_CONN_TMR_EVNT_CONN_WAIT        2
#define RLYLIB_CONN_TMR_EVNT_INIT             3
#define RLYLIB_CONN_TMR_EVNT_HBEAT            4

#define RLYLIB_MUTX_LOCK_TYPE_RD              1
#define RLYLIB_MUTX_LOCK_TYPE_WR              2
#define RLYLIB_MUTX_LOCK_TYPE_ALL             3

/*--------------------------------------------------------------*/
/*                    relay library macro                       */ 
/*--------------------------------------------------------------*/
/* Relay library log */
#ifdef RLYLIB_LOG
#define RLY_LOG(LEVEL,...){\
    UINT d_logLvl = 0;\
    d_logLvl = rlylibInt_globGetLogLvl();\
    if((d_logLvl != RLY_NONE) && (d_logLvl >= LEVEL)){\
        /* print log level */\
        rlylibInt_globLogPrnt(LEVEL, __FILE__, __LINE__, __VA_ARGS__);\
    }\
}
#else 
#define RLY_LOG(LEVEL,...)
#endif

#define RLYLIB_INC_OPEN_CONN_CNT(_hostCb){\
    (_hostCb)->openConnCnt++;\
    hostCb->status = RLYLIB_HOST_STA_OPEN;\
    hostCb->freeTmFlg = RC_FALSE;\
}

#define RLYLIB_DEC_OPEN_CONN_CNT(_hostCb){\
    (_hostCb)->openConnCnt--;\
    if((_hostCb)->openConnCnt == 0){\
        hostCb->status = RLYLIB_HOST_STA_CLOSED;\
        if((_hostCb)->freeTmEnb == RC_TRUE){\
            hostCb->freeTm = rlylibInt_globGetCurTick();\
            hostCb->freeTmFlg = RC_TRUE;\
        }\
    }\
} 

#define RLYLIB_PUT_U32(_dst, _src){\
    UINT d_idx = 0;\
    (_dst)[d_idx] = ((_src) & 0xff000000) >> 24;\
    (_dst)[d_idx+1] = ((_src) & 0x00ff0000) >> 16;\
    (_dst)[d_idx+2] = ((_src) & 0x0000ff00) >> 8;\
    (_dst)[d_idx+3] = (_src) & 0x000000ff;\
}

#define RLYLIB_GET_4BYTE(_dat, _mBuf){\
    (_dat) = (((UCHAR)(_mBuf)[0] << 24) |((UCHAR)(_mBuf)[1] << 16) | ((UCHAR)(_mBuf)[2] << 8) | ((UCHAR) (_mBuf)[3]));\
}

#define RLYLIB_SET_CONN_TMR_HNDL_NODE(_tmrHndlNode, _curTick, _tmOut, _evnt){\
    (_tmrHndlNode)->tmOutTick = (_curTick) + (_tmOut);\
    (_tmrHndlNode)->used = RC_USED;\
    (_tmrHndlNode)->evnt = (_evnt);\
}

#define RLYLIB_SET_TMR_INFO(_tmrInfo, _tmrOptArg){\
    if((_tmrOptArg)->connTmOut != 0){\
        (_tmrInfo)->connTmOut = (_tmrOptArg)->connTmOut;\
    }\
    else {\
        (_tmrInfo)->connTmOut = RLYLIB_DFLT_TMR_EVNT_CONN_TMOUT;\
    }\
    if((_tmrOptArg)->connWaitTmOut != 0){\
        (_tmrInfo)->connWaitTmOut = (_tmrOptArg)->connWaitTmOut;\
    }\
    else {\
        (_tmrInfo)->connWaitTmOut = RLYLIB_DFLT_TMR_EVNT_CONN_WAIT_TMOUT;\
    }\
    if((_tmrOptArg)->initTmOut != 0){\
        (_tmrInfo)->initTmOut = (_tmrOptArg)->initTmOut;\
    }\
    else {\
        (_tmrInfo)->initTmOut = RLYLIB_DFLT_TMR_EVNT_INIT_TMOUT;\
    }\
    if((_tmrOptArg)->hBeatSndTmOut != 0){\
        (_tmrInfo)->hBeatSndTmOut = (_tmrOptArg)->hBeatSndTmOut;\
    }\
    else {\
        (_tmrInfo)->hBeatSndTmOut = RLYLIB_DFLT_TMR_EVNT_HBEAT_SND_TMOUT;\
    }\
    if((_tmrOptArg)->hBeatTmOut != 0){\
        (_tmrInfo)->hBeatTmOut = (_tmrOptArg)->hBeatTmOut;\
    }\
    else {\
        (_tmrInfo)->hBeatTmOut = RLYLIB_DFLT_TMR_EVNT_HBEAT_TMOUT;\
    }\
    if((_tmrOptArg)->hostFreeTmOut != 0){\
        (_tmrInfo)->hostFreeTmOut = (_tmrOptArg)->hostFreeTmOut;\
    }\
    else {\
        (_tmrInfo)->hostFreeTmOut = RLYLIB_DFLT_TMR_EVNT_FREE_TMOUT;\
    }\
    if((_tmrOptArg)->msgDropTmOut != 0){\
        (_tmrInfo)->msgDropTmOut = (_tmrOptArg)->msgDropTmOut;\
    }\
    else {\
        (_tmrInfo)->msgDropTmOut = RLYLIB_DFLT_TMR_EVNT_MSG_DROP_TMOUT;\
    }\
}

#define RLYLIB_SET_DFLT_TMR_INFO(_tmrInfo){\
    (_tmrInfo)->connTmOut = RLYLIB_DFLT_TMR_EVNT_CONN_TMOUT;\
    (_tmrInfo)->connWaitTmOut = RLYLIB_DFLT_TMR_EVNT_CONN_WAIT_TMOUT;\
    (_tmrInfo)->initTmOut = RLYLIB_DFLT_TMR_EVNT_INIT_TMOUT;\
    (_tmrInfo)->hBeatSndTmOut = RLYLIB_DFLT_TMR_EVNT_HBEAT_SND_TMOUT;\
    (_tmrInfo)->hBeatTmOut = RLYLIB_DFLT_TMR_EVNT_HBEAT_TMOUT;\
    (_tmrInfo)->hostFreeTmOut = RLYLIB_DFLT_TMR_EVNT_FREE_TMOUT;\
    (_tmrInfo)->msgDropTmOut = RLYLIB_DFLT_TMR_EVNT_MSG_DROP_TMOUT;\
}

#define RLYLIB_FREE_MSG_CHK(_freeMsgChkQ, _msgBufInfo,  _msgChk, _ret){\
    if((_msgChk)->data != NULL){\
        if((_msgBufInfo) != NULL){\
            rlylibInt_msgFreeMsgBuf((_msgBufInfo),(_msgChk)->data, (_msgChk)->dataLen);\
            (_msgChk)->dataLen = 0;\
        }\
        else {\
            comlib_memFree((_msgChk)->data);\
        }\
        (_msgChk)->data = NULL;\
    }\
    (_msgChk)->msgHdrIncFlg = RC_FALSE;\
    (_ret) = thrlib_tqPush((_freeMsgChkQ), (_msgChk));\
    if((_ret) == THRERR_TQ_FULL){\
        comlib_memFree((_msgChk));\
        (_ret) = RC_OK;\
    }\
    else if((_ret) != RC_OK){\
        RLY_LOG(RLY_ERR,"free Message chunk queue push failed(ret=%d)\n",ret);\
        comlib_memFree((_msgChk));\
    }\
    (_msgChk) = NULL;\
}

#define RLYLIB_MAKE_SND_DYN_MSG_CHK(_freeMsgChkQ, _buf, _bufLen, _msgChk, _ret){\
    RlylibIntMsgQHdr *d_msgQHdr = NULL;\
    RlylibIntMsgHdr *d_msgHdr = NULL;\
    (_ret) = thrlib_tqPop((_freeMsgChkQ), (VOID*)&(_msgChk));\
    if((_ret) == THRERR_TQ_EMPTY){\
        (_msgChk) = comlib_memMalloc(sizeof(RlylibIntMsgChk));\
        (_ret) = RC_OK;\
    }\
    else if((_ret) != RC_OK){\
        RLY_LOG(RLY_ERR,"Message pop failed(ret=%d)\n",(_ret));\
        (_ret) = RLYERR_MSG_POP_FAILED;\
    }\
    if((_ret) == RC_OK){\
        (_msgChk)->data = (_buf);\
        (_msgChk)->msgHdrIncFlg = RC_FALSE;\
        d_msgQHdr = (RlylibIntMsgQHdr*)(_msgChk)->hdrs;\
        d_msgQHdr->msgTick = rlylibInt_globGetCurTick();\
        (_msgChk)->dataLen = (_bufLen);\
        d_msgHdr = (RlylibIntMsgHdr*)&(_msgChk)->hdrs[sizeof(RlylibIntMsgQHdr)];\
        d_msgHdr->msgLen = (_bufLen);\
        (_ret) = RC_OK;\
    }\
}

#define RLYLIB_MAKE_SND_FIX_MSG_CHK(_freeMsgChkQ, _msgBufInfo, _buf, _bufLen, _msgChk, _ret){\
    RlylibIntMsgQHdr *d_msgQHdr = NULL;\
    RlylibIntMsgHdr *d_msgHdr = NULL;\
    (_ret) = thrlib_tqPop((_freeMsgChkQ), (VOID*)&(_msgChk));\
    if((_ret) == THRERR_TQ_EMPTY){\
        (_msgChk) = comlib_memMalloc(sizeof(RlylibIntMsgChk));\
        (_ret) = RC_OK;\
    }\
    else if((_ret) != RC_OK){\
        RLY_LOG(RLY_ERR,"Message pop failed(ret=%d)\n",(_ret));\
        (_ret) = RLYERR_MSG_POP_FAILED;\
    }\
    if((_ret) == RC_OK){\
        if((_msgBufInfo) != NULL){\
            ret = rlylibInt_msgAllocMsgBuf((_msgBufInfo), (_bufLen) + sizeof(RlylibIntMsgHdr), \
                    (VOID*)&(_msgChk)->data, &(_msgChk)->dataLen);\
            if(ret != RC_OK){\
                RLY_LOG(RLY_ERR,"Message buffer alloc failed(ret=%d)\n");\
                (_msgChk)->data = NULL;\
            }\
        }\
        else{\
            (_msgChk)->dataLen = (_bufLen);\
            (_msgChk)->data = comlib_memMalloc((_bufLen) + sizeof(RlylibIntMsgHdr));\
        }\
        if((_msgChk)->data == NULL){\
            comlib_memFree((_msgChk));\
            (_ret) = RLYERR_MSG_BUF_ALLOC_FAILED;\
        }\
        else {\
            (_msgChk)->msgHdrIncFlg = RC_TRUE;\
            d_msgQHdr = (RlylibIntMsgQHdr*)(_msgChk)->hdrs;\
            d_msgQHdr->msgTick = rlylibInt_globGetCurTick();\
            d_msgHdr = (RlylibIntMsgHdr*)(_msgChk)->data;\
            d_msgHdr->msgLen = (_bufLen);\
            comlib_memMemcpy(&(_msgChk)->data[sizeof(RlylibIntMsgHdr)], (_buf), (_bufLen));\
            (_ret) = RC_OK;\
        }/* end of if((_msgChk)_>data == NULL) */\
    }\
}

#define RLYLIB_COND_LOOP_START(_mutx, _cond, _condWaitFlg, _itvlChkTm, _waitTm)\
{\
    SINT d_intRet = RC_OK;\
    ThrlibMutx *d_intMutx = (_mutx);\
    ThrlibCond *d_intCond = (_cond);\
    ULONG d_intItvlChkTm = (_itvlChkTm);\
    ULONG d_intWaitTm = (_waitTm);\
    ULONG d_intSleepTm = 0;\
    ULONG d_intTotSleepTm = 0;\
    BOOL d_intCondWaitFlg = RC_FALSE;\
    if((_condWaitFlg) == RC_TRUE){\
        d_intCondWaitFlg = RC_TRUE;\
    }\
    while(1){\

#define RLYLIB_COND_LOOP_BREAK(_setRet)\
        d_intRet = (_setRet);\
        goto goto_intCondLoopEnd;

#define RLYLIB_COND_LOOP_END(_rt_ret)\
        if(d_intCondWaitFlg == RC_TRUE){\
            if(d_intWaitTm != 0){\
                if(d_intItvlChkTm == 0){\
                    d_intSleepTm = d_intWaitTm;\
                }\
                else{\
                    d_intSleepTm = d_intItvlChkTm;\
                    d_intTotSleepTm += d_intSleepTm;\
                }\
                if(d_intTotSleepTm > d_intWaitTm){\
                    d_intSleepTm = d_intTotSleepTm - d_intWaitTm;\
                    d_intTotSleepTm = d_intWaitTm;\
                }\
            }/* if(d_intWaitTm != 0) */\
            else {\
                if(d_intItvlChkTm == 0){\
                    d_intSleepTm = 0;\
                }\
                else{\
                    d_intSleepTm = d_intItvlChkTm;\
                }\
            }\
            d_intRet = rlylibInt_syncWaitSigCond(d_intCond, d_intMutx, d_intSleepTm);\
            if(d_intRet == RC_OK){\
                continue;\
            }\
            else if(d_intRet == RLYERR_COND_TM_OUT){\
                if(d_intTotSleepTm == d_intWaitTm){\
                    (_rt_ret) = RLYERR_COND_TM_OUT;\
                    break;\
                }\
                else {\
                    continue;\
                }\
            }\
            else {\
                RLY_LOG(RLY_ERR,"Cond wait failed(ret=%d)\n",d_intRet);\
                (_rt_ret) = d_intRet;\
                break;\
            }\
        }\
        else {\
            (_rt_ret) = RLYERR_COND_TM_OUT;\
            break;\
        }\
        continue;\
        goto_intCondLoopEnd:\
        (_rt_ret) = d_intRet;\
        break;\
    }/* end of while(1){ */\
}

#ifdef __cplusplus
}
#endif

#endif
