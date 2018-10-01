/************************************************************************

     Name:     REST Rule library

     Type:     C include file

     Desc:     REST Rule library definition and macro

     File:     rrllib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RRLLIB_H_
#define _RRLLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RRL_PATH_NAME_LEN                     512
#define RRL_QUERY_NAME_LEN                    512
#define RRL_ARG_NAME_LEN                      512
#define RRL_VAL_FMT_LEN                       1024
#define RRL_CMD_FMT_LEN                       1024
#define RRL_DOC_ARG_NAME_LEN                  1024
#define RRL_DOC_ARG_VAL_LEN                   1024

#define RRL_RES_PATH_HASH_ENTRY_CNT           256

#define RRL_PATH_TYPE_DYN                     1
#define RRL_PATH_TYPE_FIX                     2

#define RRL_MTHOD_NONE                        0
#define RRL_MTHOD_GET                         1
#define RRL_MTHOD_POST                        2
#define RRL_MTHOD_DEL                         3
#define RRL_MTHOD_PUT                         4

#define RRL_TKN_TYPE_DIGIT_CHAR               1
#define RRL_TKN_TYPE_ALPHA_CHAR               2
#define RRL_TKN_TYPE_SAFE_CHAR                3
#define RRL_TKN_TYPE_EXTRA_CHAR               4
#define RRL_TKN_TYPE_RESVRD_CHAR              5
#define RRL_TKN_TYPE_CFG_TKN_CHAR             6

#define RRLLIB_MAX_TMP_BUF_LEN                1024

#define RRLERR_RRLCB_IS_NULL                  100
#define RRLERR_LNKLST_INIT_FAIELD             101
#define RRLERR_INVALID_NAME_LEN               102
#define RRLERR_INVALID_PATH_TYPE              103
#define RRLERR_NOT_EXIST                      104
#define RRLERR_INVALID_PATH_NAME_LEN          105
#define RRLERR_METHOD_LNKLST_INIT_FAILED      106
#define RRLERR_PATH_LST_ALREADY_EXIST         107
#define RRLERR_QUERY_LNKLST_INIT_FAILED       108
#define RRLERR_RES_PATH_INSERT_FAEILD         109
#define RRLERR_RES_PATH_LNKLST_INIT_FAILED    110
#define RRLERR_PATH_ALREADY_EXIST             107
#define RRLERR_PARINSG_FAILED                 108
#define RRLERR_FIX_PATH_NOT_EXIST             109
#define RRLERR_DYN_PATH_NOT_EXIST             110
#define RRLERR_URI_PATH_NOT_EXIST             111
#define RRLERR_INVALID_MTHOD                  114
#define RRLERR_MTHOD_INSERT_FAEILD            113
#define RRLERR_NXT_PATH_NOT_EXSIT             114
#define RRLERR_MTHOD_NOT_EXSIT                115
#define RRLERR_MTHOD_ARG_LNKLST_INIT_FAEILD   116
#define RRLERR_INVALID_DOC_ARG_NAME_LEN       117
#define RRLERR_DOC_ARG_VAL_LNKLST_INIT_FAILED 118
#define RRLERR_DOC_ARG_INSERT_FAILED          119
#define RRLERR_DOC_ARG_VAL_INVALID_LEN        120
#define RRLERR_DOC_ARG_VAL_INSERT_FAEILD      121 
#define RRLERR_DOC_ARG_DUP                    122 
#define RRLERR_DYN_PATH_ALREADY_EXIST         123
#define RRLERR_QUERY_INSERT_FAILED            124
#define RRLERR_QUERY_ALREADY_EXIST            125
#define RRLERR_INVALID_TOKEN                  126
#define RRLERR_QUEYR_KEY_ALREADY_EXIST        127
#define RRLERR_QUERY_KEY_NOT_EXIST            128
#define RRLERR_QUERY_FUNC_FAILED              129
#define RRLERR_QUERY_STR_END                  130
#define RRLERR_INVALID_QUERY_NAME_LEN         131
#define RRLERR_END                            132
#define RRLERR_INVALID_MAND_FLAG              133
#define RRLERR_MAND_VALUE_NOT_EXSIT           134
#define RRLERR_GLOB_CB_NOT_INIT               135
#define RRLERR_INVALID_LOG_LEVEL              136
#define RRLERR_RES_PATH_HASH_TBL_INIT_FAILED  137
#define RRLEER_RES_PATH_HNODE_INSERT_FAEILD   138
#define RRLERR_CASE_CHANGE_FAILED             139

#define RRL_NONE                              0
#define RRL_ERR                               1
#define RRL_NOTY                              2
#define RRL_DBG                               3

#ifdef RRL_ERR_ENB
#define RRL_ERR_ENB 1
#else
#define RRL_ERR_ENB 0
#endif
#ifdef RRL_NOTY_ENB
#define RRL_NOTY_ENB 1
#else
#define RRL_NOTY_ENB 0
#endif
#ifdef RRL_DBG_ENB
#define RRL_DBG_ENB 1
#else
#define RRL_DBG_ENB 0
#endif

#ifndef RRLLIB_LOG
#define RRL_LOG(LEVEL,...)
#else 
#define RRL_LOG(_LEVEL,...){\
    if(((_LEVEL) == RRL_ERR) && (RRL_ERR_ENB == 1)){\
        UINT d_logLvl = 0;\
        d_logLvl = rrllib_globGetLogLvl();\
        if((d_logLvl != RRL_NONE) && (d_logLvl >= (_LEVEL))){\
            /* print log level */\
            rrllib_globLogPrnt((_LEVEL), __FILE__, __LINE__, __VA_ARGS__);\
        }\
    }\
    if(((_LEVEL) == RRL_NOTY) && (RRL_NOTY_ENB == 1)){\
        UINT d_logLvl = 0;\
        d_logLvl = rrllib_globGetLogLvl();\
        if((d_logLvl != RRL_NONE) && (d_logLvl >= (_LEVEL))){\
            /* print log level */\
            rrllib_globLogPrnt((_LEVEL), __FILE__, __LINE__, __VA_ARGS__);\
        }\
    }\
    if(((_LEVEL) == RRL_DBG) && (RRL_DBG_ENB == 1)){\
        UINT d_logLvl = 0;\
        d_logLvl = rrllib_globGetLogLvl();\
        if((d_logLvl != RRL_NONE) && (d_logLvl >= (_LEVEL))){\
            /* print log level */\
            rrllib_globLogPrnt((_LEVEL), __FILE__, __LINE__, __VA_ARGS__);\
        }\
    }\
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RESTRULE_H_ */

