#ifndef _RSVLIB_INT_H_
#define _RSVLIB_INT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RSV_URL_ARG_TYPE_CMD            1
#define RSV_URL_ARG_TYPE_SIM            2

#define RSV_ID_MAX_LEN                  1024
#define RSV_PWD_MAX_LEN                 1024

#define RSV_MAX_LOG_BUF_LEN             4096

#define RSV_USR_TKN_MAX_LEN             RSV_ID_MAX_LEN+RSV_PWD_MAX_LEN+sizeof(struct timespec)+sizeof(UINT)

#define RSV_SVR_RULE_NONE               1
#define RSV_SVR_RULE_STRM               2

#define RSV_MAX_SVR_CNT                 64

#define RSV_SHA256_LEN                  32
#define RSV_SHA256_STR_LEN              64 /* 1BYTE = 8bit */

#define RSV_MAX_STRM_BUF_LEN            1024

#define RSV_MAX_CMD_CNT                 128
#define RSV_ARG_NAME_LEN                256
#define RSV_PARM_FMT_LEN                256
#define RES_RULE_CMD_FMT_LEN             256

#define RSV_PATH_MAX_LEN                1024

#define RSV_CLI_PRNT_MAX_BUF            2096

#define RSV_CMD_SIG_WAIT 1
#define RSV_CMD_SIG_CMPLTE 2

#define RSV_AUTH_CMD_CODE_REG           1
#define RSV_AUTH_CMD_CODE_CHK           2
#define RSV_AUTH_CMD_CODE_DEL           3
#define RSV_AUTH_CMD_CODE_UPD           4

#define RSV_AUTH_RSLT_CODE_REG_SUCC     1
#define RSV_AUTH_RSLT_CODE_REG_FAILED   2
#define RSV_AUTH_RSLT_CODE_CHK_SUCC     3
#define RSV_AUTH_RSLT_CODE_CHK_FAILED   4
#define RSV_AUTH_RSLT_CODE_DEL_SUCC     5
#define RSV_AUTH_RSLT_CODE_DEL_FAILED   6
#define RSV_AUTH_RSLT_CODE_UPD_SUCC     7
#define RSV_AUTH_RSLT_CODE_UPD_FAILED   8
#define RSV_AUTH_RSLT_CODE_UNKNOWN      9

#if 1
#if 0
#define RSV_LOG(_LEVEL,...){\
    fprintf(stderr,__VA_ARGS__);\
}
#endif
#define RSV_LOG(LEVEL,...){\
    UINT d_logLvl = 0;\
    d_logLvl = rsvlibInt_globGetLogLvl();\
    if((d_logLvl != RSV_NONE) && (d_logLvl >= LEVEL)){\
        /* print log level */\
        rsvlibInt_globLogPrnt(LEVEL, __FILE__, __LINE__, __VA_ARGS__);\
    }\
}
#else 
#define RSV_LOG(_LVL,...){\
    LoglibCb *d_loglibCb = NULL;\
    (d_loglibCb) = rsvlibInt_globGetLoglibCb();\
    if((_LVL) == RSV_ERR){\
        if(d_loglibCb != NULL){\
            LOGLIB_ERR((d_loglibCb),__VA_ARGS__);\
        }\
        else {\
            fprintf(stderr,__VA_ARGS__);\
        }\
    }\
    else if((_LVL) == RSV_NOTY){\
        if(d_loglibCb != NULL){\
            LOGLIB_NOTY((d_loglibCb),__VA_ARGS__);\
        }\
        else {\
            fprintf(stderr,__VA_ARGS__);\
        }\
    }\
    else if((_LVL) == RSV_DBG){\
        if(d_loglibCb != NULL){\
            LOGLIB_DBG((d_loglibCb),__VA_ARGS__);\
        }\
        else {\
            fprintf(stderr,__VA_ARGS__);\
        }\
    }\
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RSVLIB_INT_H_ */
