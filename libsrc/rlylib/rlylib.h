/************************************************************************

     Name:     Relay library 

     Type:     C header file

     Desc:     Relay library definition and functions  

     File:     rlylib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RLYLIB_H_
#define _RLYLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*                   relay libary definition                    */ 
/*--------------------------------------------------------------*/
/* thread library log level */
#define RLY_NONE                              0
#define RLY_ERR                               1
#define RLY_NOTY                              2
#define RLY_DBG                               3

#define RLYLIB_TYPE_CLIENT                    1
#define RLYLIB_TYPE_SERVER                    2
#define RLYLIB_TYPE_BOTH                      3

#define RLYLIB_ACPT_TYPE_ALL                  1  /* default */
#define RLYLIB_ACPT_TYPE_CHK_LST              2

#define RLYLIB_DFLT_THRD_CNT                  3
#define RLYLIB_DFLT_WAIT_TM                   100

#define RLYLIB_MAX_RLM_LEN                    64
#define RLYLIB_MAX_HOST_LEN                   64

#define RLYLIB_AFNUM_IPV6                     6
#define RLYLIB_AFNUM_IPV4                     4

#define RLYLIB_TRANS_PROTO_TCP                1

#define RLYLIB_HOST_STA_OPEN                  1
#define RLYLIB_HOST_STA_CLOSED                2

#define RLYLIB_RLM_RULE_RR                    1
#define RLYLIB_RLM_RULE_PS                    2
#define RLYLIB_RLM_RULE_HASH                  3
#define RLYLIB_RLM_RULE_DHT                   4

#define RLYLIB_RLM_HASH_KEY_TYPE_U32          1
#define RLYLIB_RLM_HASH_KEY_TYPE_STR          2

#define RLYLIB_RLM_HASH_TYPE_HASHING_ACT_HOST 1
#define RLYLIB_RLM_HASH_TYPE_HASHING_ALL_HOST 2

#define RLYLIB_RLM_HASH_MAX_KEY_LEN           128

/* connetcion relay type */
#define RLYLIB_CONN_RLY_MODE_RR               1  /* round-robin */
#define RLYLIB_CONN_RLY_MODE_AS               2  /* active-standby (default) */

/* error */
#define RLYERR_NULL                           100
#define RLYERR_INVALID_RLY_TYPE               101
#define RLYERR_INVALID_ACPT_TYPE              102
#define RLYERR_INVALID_DIST_TYPE              103
#define RLYERR_CONN_LNK_LST_INIT_FAILED       104
#define RLYERR_MUTX_INIT_FAILED               105
#define RLYERR_CONN_HASH_INIT_FAILED          106
#define RLYERR_ACPT_LNK_LST_INIT_FAILED       107
#define RLYERR_ACPT_THRD_CRTE_FAILED          108
#define RLYERR_CONN_THRD_POOL_INIT_FAILED     109
#define RLYERR_HOST_CB_ALLOC_FAILED           110
#define RLYERR_INVALID_AFNUM                  111
#define RLYERR_ACPT_CB_EXIST                  112
#define RLYERR_ACPT_CB_ALLOC_FAILED           113 
#define RLYERR_ACT_CONN_LNK_LST_INIT_FAILED   114
#define RLYERR_FREE_CONN_LNK_LST_INIT_FAILED  115
#define RLYERR_ACPT_SOCK_OPEN_FAILED          116
#define RLYERR_ACPT_ADDR_BIND_FAILED          117
#define RLYERR_ACPT_LISTEN_FAILED             118
#define RLYERR_ACPT_CB_INSERT_FAILED          119
#define RLYERR_ACPT_CB_CRTE_FAILED            120
#define RLYERR_ACPT_FAILED                    121
#define RLYERR_SOCK_OPT_FAILED                122
#define RLYERR_ACPT_AGAIN                     123
#define RLYERR_SOCK_FD_SET_FAILED             124
#define RLYERR_INVALID_CONN_CNT               125
#define RLYERR_HOST_IS_FULL                   126
#define RLYERR_INVALID_HOST_LEN               128
#define RLYERR_THRD_Q_INIT_FAILED             129
#define RLYERR_INVALID_MODE                   130
#define RLYERR_LNK_LST_INSERT_FAEILD          131
#define RLYERR_HASH_TBL_INSERT_FAEILD         132
#define RLYERR_INVALID_TYPE                   133
#define RLYERR_HOST_ALREAY_EXIST              134
#define RLYERR_HOST_CB_NOT_EXIST              135
#define RLYERR_CONN_CB_ALLOC_FAILED           136
#define RLYERR_CONN_CB_INSERT_FAILED          137
#define RLYERR_CONN_SOCK_OPEN_FAILED          138
#define RLYERR_CONN_FAILED                    139
#if 0
#define RLYERR_CONN_INPROGRESS                140
#endif
#define RLYERR_SOCK_SELECT_TMOUT              141
#define RLYERR_MSG_BUF_ALLOC_FAILED           142
#define RLYERR_MSG_PUSH_FAILED                143
#define RLYERR_INVALID_BUF_LEN                144
#define RLYERR_THR_Q_POP_ERROR                145
#define RLYERR_SOCK_TERM                      146
#define RLYERR_MSG_BUF_TOO_SMALL              147
#define RLYERR_MSG_POP_FAILED                 148
#define RLYERR_MSG_NOT_EXIST                  149
#define RLYERR_HOST_LNK_LST_INIT_FAILED       150
#define RLYERR_HOST_KEY_MAKE_FAILED           151
#define RLYERR_SOCK_READ_FAILED               152
#define RLYERR_READ_AGAIN                     153
#define RLYERR_UNEXPECT_MSG_CODE              154
#define RLYERR_INVALID_AVP_CODE               155
#define RLYERR_MSG_PARSE_FAILED               156
#define RLYERR_MIAN_CB_ALLOC_FAILED           157
#define RLYERR_HOST_ID_OVERRANGE              158
#define RLYERR_INVALID_LOG_LEVEL              159
#define RLYERR_GLOB_CB_NOT_INIT               160
#define RLYERR_LOG_FUNC_NULL                  161
#define RLYERR_TMR_INIT_FAILED                162
#define RLYERR_MSG_DEC_FAILED                 163
#define RLYERR_CONN_OPEN_FAILED               164
#define RLYERR_MSG_ENC_FAILED                 165
#define RLYERR_SOCK_WRITE_FAILED              166
#define RLYERR_DROP_MSG                       167
#define RLYERR_DROP_MSG_AND_BUF_TOO_SMALL     168
#define RLYERR_OUT_OF_SPACE                   169
#define RLYERR_SRC_STR_EMPTY                  170
#define RLYERR_RLM_CB_ALLOC_FAILED            171
#define RLYERR_RLM_IS_FULL                    172
#define RLYERR_INVALID_RLM_RULE               173
#define RLYERR_RLM_NOT_EXIST                  174
#define RLYERR_REG_HOST_LNK_LST_INIT_FAILED   175
#define RLYERR_REG_HOST_HASH_INIT_FAILED      176
#define RLYERR_REG_HOST_EXIST                 176
#define RLYERR_REG_HOST_FULL                  177
#define RLYERR_RLM_LNK_LST_INIT_FAILED        178
#define RLYERR_REG_HOST_NOT_EXIST             180
#define RLYERR_RLM_HASH_INIT_FAILED           181
#define RLYERR_INVALID_HASH_KEY_TYPE          182
#define RLYERR_INVALID_HASH_TYPE              183
#define RLYERR_INVALID_HASH_KEY_NOT_EXIST     184
#define RLYERR_INVALID_HASH_KEY_LEN           185
#define RLYERR_INVALID_HASHING_FAILED         186
#define RLYERR_BUF_INFO_INIT_FAILED           187
#define RLYERR_BUF_INFO_DSTRY_FAILED          188
#define RLYERR_INVALID_LOCK_TYPE              189
#define RLYERR_COND_INIT_FAILED               190
#define RLYERR_COND_WAIT_FAILED               191
#define RLYERR_COND_TM_OUT                    192

/* macros */
#define RLYLIB_INIT_TMR_OPT_ARG(_optArg){\
    (_optArg)->connTmOut = 0;\
    (_optArg)->connWaitTmOut = 0;\
    (_optArg)->initTmOut = 0;\
    (_optArg)->hBeatSndTmOut = 0;\
    (_optArg)->hBeatTmOut = 0;\
    (_optArg)->hostFreeTmOut = 0;\
    (_optArg)->msgDropTmOut = 0;\
}

#define RLYLIB_INIT_HOST_OPT_ARG(_optArg){\
    (_optArg)->msgDropIndFlg = RC_FALSE;\
    (_optArg)->rlyMode = RLYLIB_CONN_RLY_MODE_AS;\
    (_optArg)->freeTmEnb = RC_FALSE;\
    RLYLIB_INIT_TMR_OPT_ARG(&(_optArg)->tmrOptArg);\
}

#define RLYLIB_INIT_OPT_ARG(_optArg){\
    (_optArg)->maxHostCnt = 0;\
    (_optArg)->maxRlmCnt = 0;\
    (_optArg)->condWaitFlg = RC_FALSE;\
    (_optArg)->waitTm = RLYLIB_DFLT_WAIT_TM;\
    (_optArg)->acptType = RLYLIB_ACPT_TYPE_ALL;\
    (_optArg)->thrdCnt = RLYLIB_DFLT_THRD_CNT;\
    RLYLIB_INIT_HOST_OPT_ARG(&(_optArg)->dfltHostOptArg);\
    (_optArg)->dfltHostOptArg.freeTmEnb = RC_TRUE;\
}

#ifdef __cplusplus
}
#endif

#endif /* _RLYLIB_H_ */
