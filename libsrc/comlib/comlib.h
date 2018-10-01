/************************************************************************

     Name:     Common library 

     Type:     C header file

     Desc:     Common library definition and functions  

     File:     comlib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _COMLIB_H_
#define _COMLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define COM_NOT_LIMIT                    0

/*--------------------------------------------------------------*/
/*              common libary definition                        */ 
/*--------------------------------------------------------------*/
/* common libary log level */
#define COM_NONE                         0
#define COM_ERR                          1
#define COM_NOTY                         2
#define COM_DBG                          3

/*------- Linked list definition -------*/

/*-------    Hash definition     -------*/
/* Hash type */
#define COM_HASH_TYPE_STRING             0
#define COM_HASH_TYPE_UINT               1
#define COM_HASH_TYPE_USR                2 /* user hashing function */
#define COM_HASH_TYPE_MURMUR             3
#define COM_HASH_TYPE_USRKEY             4

/*-------   Timer definition     -------*/
/* Timer Type */
#define COM_TIMER_TYPE_SEC               0
#define COM_TIMER_TYPE_100M              1

/* Tiemr Tick */
#define COM_TIMER_TICK_SEC               1000000000      /* second */
#define COM_TIMER_TICK_100MS             100000000	     /* 0.1sec */
#define COM_TIMER_TICK_1MS               1000000         /* 1 millisecond */

/* Timer State */
#define COM_TIMER_ACT                    0
#define COM_TIMER_INACT                  1

/*-------   Message definition   -------*/
#define COM_MSG_MAX_LEN                  0x7fffffff
#define COM_MSG_BUF_CHAIN_LEN            0x80

/* common library error code definition */
/* Linked list error code */
#define COMERR_INVALID_LNKLST            101
#define COMERR_INVALID_LNKNODE           102
#define COMERR_MAX_NODE                  103
#define COMERR_LNKLST_IS_EMPTY           104

/* Hash error code */
#define COMERR_INVALID_HASHTBL           201
#define COMERR_INVALID_HASHTYPE          202
#define COMERR_INVALID_HASHKEY           203
#define COMERR_INVALID_NODE              204
#define COMERR_INVALID_DUP_FLAG          205
#define COMERR_CAN_NOT_FIND_HASHENTRY    206
#define COMERR_NMBENTRY_IS_ZERO          207
#define COMERR_KEY_LENGTH_IS_MISMATCH    208
#define COMERR_KEY_IS_MISMATCH           209
#define COMERR_NODE_ALREADY_EXIST        210
#define COMERR_HASHNODE_INSERT_FAIL      211
#define COMERR_HASHNODE_NOT_EXIST        212
#define COMERR_INVAILD_NODEID            213
#define COMERR_HASH_FUNC_NOT_EXIST       214

/* Timer error code */
#define COMERR_INVAILD_TIMER             301
#define COMERR_INVAILD_TIMER_TYPE        302
#define COMERR_INVAILD_TMRTBL            303
#define COMERR_INVAILD_TMRNODE           304
#define COMERR_INVAILD_TMRENTRY          305
#define COMERR_INVAILD_TICK_VALUE        306
#define COMERR_NODE_IS_NOT_USED          307
#define COMERR_INVAILD_TMREVTFUNC        308
#define COMERR_TIMER_ALREADY_START       309

/* Message buffer error code */
#define COMERR_MSG_NULL                  401
#define COMERR_MSG_HDR_NULL              402
#define COMERR_MSG_LEN_IS_FULL           403
#define COMERR_MSG_ALLOC_FAIL            404
#define COMERR_MSG_INVALID_CUR           405
#define COMERR_MSG_INVALID_LEN           406
#define COMERR_MSG_CUR_OVERFLOW          407
#define COMERR_MSG_EMPTY                 408
#define COMERR_MSG_CAN_NOT_FIND_CHK      409
#define COMERR_MSG_BUFFER_IS_SMALL       410
#define COMERR_MSG_LEN_OVERFLOW          411
#define COMERR_MSG_CAN_NOT_MAKE_NEW_MSG  412
#define COMERR_MSG_HDR2_NULL             413

/* file error code */
#define COMERR_FILE_ACCES                500
#define COMERR_FILE_DQUOT                501
#define COMERR_FILE_EXIST                502
#define COMERR_FILE_FAULT                503
#define COMERR_FILE_LOOP                 504
#define COMERR_FILE_MLINK                505
#define COMERR_FILE_NAMETOOLONG          506
#define COMERR_FILE_NOENT                507
#define COMERR_FILE_NOMEM                508
#define COMERR_FILE_NOSPC                509
#define COMERR_FILE_NOTDIR               510
#define COMERR_FILE_PERM                 511
#define COMERR_FILE_ROFS                 512

/*--------------------------------------------------------------*/
/*                   common library macro                       */ 
/*--------------------------------------------------------------*/
/* byteorder macro */
#define COMLIB_PUT_2BYTE(_dst, _src){\
    UINT d_idx = 0;\
    (_dst)[d_idx+0] = ((_src) & 0x0000ff00) >> 8;\
    (_dst)[d_idx+1] = (_src) & 0x000000ff;\
}

#define COMLIB_PUT_3BYTE(_dst, _src){\
    UINT d_idx = 0;\
    (_dst)[d_idx+0] = ((_src) & 0x00ff0000) >> 16;\
    (_dst)[d_idx+1] = ((_src) & 0x0000ff00) >> 8;\
    (_dst)[d_idx+2] = (_src) & 0x000000ff;\
}

#define COMLIB_PUT_4BYTE(_dst, _src){\
    UINT d_idx = 0;\
    (_dst)[d_idx] = ((_src) & 0xff000000) >> 24;\
    (_dst)[d_idx+1] = ((_src) & 0x00ff0000) >> 16;\
    (_dst)[d_idx+2] = ((_src) & 0x0000ff00) >> 8;\
    (_dst)[d_idx+3] = (_src) & 0x000000ff;\
}

#define COMLIB_GET_1BYTE_MV_CUR(_dat, _mBuf){\
    COMLIB_GET_1BYTE(_dat, _mBuf);\
    _mBuf ++;\
}

#define COMLIB_GET_1BYTE(_dat, _mBuf){\
    (_dat) = ((UCHAR) *(_mBuf));\
}

#define COMLIB_GET_2BYTE_MV_CUR(_dat, _mBuf){\
    COMLIB_GET_2BYTE(_dat, _mBuf);\
    _mBuf += 2;\
}

#define COMLIB_GET_2BYTE(_dat, _mBuf){\
    (_dat) = (((UCHAR)(_mBuf)[0] << 8) | ((UCHAR) (_mBuf)[1]));\
}

#define COMLIB_GET_3BYTE_MV_CUR(_dat, _mBuf){\
    COMLIB_GET_3BYTE(_dat, _mBuf);\
    _mBuf += 3;\
}

#define COMLIB_GET_3BYTE(_dat, _mBuf){\
    (_dat) = (((UCHAR)(_mBuf)[0] << 16) | ((UCHAR)(_mBuf)[1] << 8) | ((UCHAR) (_mBuf)[2]));\
}

#define COMLIB_GET_4BYTE_MV_CUR(_dat, _mBuf){\
    COMLIB_GET_4BYTE(_dat, _mBuf);\
    _mBuf += 4;\
}

#define COMLIB_GET_4BYTE(_dat, _mBuf){\
    (_dat) = (((UCHAR)(_mBuf)[0] << 24) |((UCHAR)(_mBuf)[1] << 16) | ((UCHAR)(_mBuf)[2] << 8) | ((UCHAR) (_mBuf)[3]));\
}

/* Linked list macro (PUBLIC) */
#define COM_GET_LNKLST_FIRST(_lnkLst, _entry) (_entry) = (_lnkLst)->first;
#define COM_GET_LNKLST_TAIL(_lnkLst, _entry) (_entry) = (_lnkLst)->tail;
#define COM_GET_NEXT_NODE(_entry) (_entry) = (_entry)->next;
#define COM_GET_PREV_NODE(_entry) (_entry) = (_entry)->prev;

/* Timer macro (PUBLIC) */
#ifdef _BIT_64_
#define EXEC_TMR_TICK(tmr){\
    DOUBLE curTm;\
    DOUBLE diff;\
    comlib_timerGetTime(&(tmr)->curTs);\
    curTm = (DOUBLE)((tmr)->curTs.tv_sec * COM_TIMER_TICK_SEC) + (DOUBLE)(tmr)->curTs.tv_nsec;\
    diff = (curTm - (tmr)->lastTm);\
    if(diff >= (tmr)->tmSlice){\
        (tmr)->tick += (ULONG)(diff / (tmr)->tmSlice);\
    }\
}
#else 
#define EXEC_TMR_TICK(tmr){\
    ULONG diff = 0;\
    SINT d_ret = RC_OK;\
    comlib_timerGetTime(&(tmr)->curTs);\
    if((tmr)->curTs.tv_sec == (tmr)->lastTs.tv_sec){\
        if((tmr)->curTs.tv_nsec < (tmr)->lastTs.tv_nsec){\
            (tmr)->errTm += (tmr)->avgDiff;\
            diff = 0;\
        }\
        else{\
            diff = (ULONG)((tmr)->curTs.tv_nsec - (tmr)->lastTs.tv_nsec);\
            /*(tmr)->avgDiff = (diff + (tmr)->avgDiff) / 2;*/\
            (tmr)->avgDiff = (0.875 * (tmr)->avgDiff) + (0.125 * diff);\
        }\
    }\
    else if((d_ret=(tmr)->curTs.tv_sec - (tmr)->lastTs.tv_sec) > 0){\
        diff = (tmr)->curTs.tv_nsec + (COM_TIMER_TICK_SEC - (tmr)->lastTs.tv_nsec);\
        if(d_ret > 1){\
            (tmr)->tick += (ULONG)((d_ret -1) * (tmr)->tps);\
            (tmr)->lastTs.tv_sec = (tmr)->curTs.tv_sec;\
        }\
        /*(tmr)->avgDiff = (diff + (tmr)->avgDiff) / 2;*/\
        (tmr)->avgDiff = (0.875 * (tmr)->avgDiff) + (0.125 * diff);\
    }\
    else {\
        (tmr)->errTm += (tmr)->avgDiff;\
        diff = 0;\
    }\
    if(diff > ((tmr)->avgDiff * 3)){\
        diff = (tmr)->avgDiff;\
    }\
    diff += (tmr)->errTm;\
    if(diff >= (ULONG)(tmr)->tmSlice){\
        (tmr)->tick += (ULONG)(diff / (ULONG)(tmr)->tmSlice);\
        (tmr)->lastTs.tv_sec = (tmr)->curTs.tv_sec;\
        (tmr)->lastTs.tv_nsec = (tmr)->curTs.tv_nsec;\
        (tmr)->errTm = diff % (ULONG)(tmr)->tmSlice;\
    }\
}
#endif

#define COM_GET_REF_TIME(tmr, refTm){\
    (refTm) = (tmr)->curTs.tv_sec;\
}

#define CHECK_TICK_DIFF(prevTick, curTick, diff){\
    if(prevTick > curTick){\
        diff = (((ULONG)~0) - prevTick) + curTick;\
    }\
    else {\
        diff = curTick - prevTick;\
    }\
}

#define GET_CUR_TICK(tmr) (tmr)->tick

#define GET_TMRNODE_STATE(tmrNode, rt_evnt, ret){\
    if((tmrNode)->used != RC_USED){\
        ret = COM_TIMER_INACT;\
    }\
    else {\
        ret = COM_TIMER_ACT;\
        rt_evnt = (tmrNode)->event;\
    }\
}

/* Common library log */
#ifdef COMLIB_LOG
#define COM_LOG(LEVEL,...){\
    printf(__VA_ARGS__);\
}
#else 
#define COM_LOG(LEVEL,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _COMLIB_H_ */
