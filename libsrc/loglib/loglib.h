/************************************************************************

     Name:     Log library 

     Type:     C header file

     Desc:     log library definition and functions  

     File:     loglib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _LOGLIB_H_
#define _LOGLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOGLIB_DISP_LVL_BIT               0x01
#define LOGLIB_DISP_FILE_BIT              0x02
#define LOGLIB_DISP_LINE_BIT              0x04
#define LOGLIB_DISP_TIME_BIT              0x08

#ifdef LOGLIB_DISP_ENV_LVL
#define LOGLIB_DISP_LVL_SET_BIT LOGLIB_DISP_LVL_BIT
#else
#define LOGLIB_DISP_LVL_SET_BIT 0
#endif
#ifdef  LOGLIB_DISP_ENV_FILE
#define LOGLIB_DISP_FILE_SET_BIT LOGLIB_DISP_FILE_BIT
#else
#define LOGLIB_DISP_FILE_SET_BIT 0
#endif
#ifdef  LOGLIB_DISP_ENV_LINE
#define LOGLIB_DISP_LINE_SET_BIT LOGLIB_DISP_LINE_BIT
#else
#define LOGLIB_DISP_LINE_SET_BIT 0
#endif
#ifdef  LOGLIB_DISP_ENV_TIME
#define LOGLIB_DISP_TIME_SET_BIT LOGLIB_DISP_TIME_BIT
#else
#define LOGLIB_DISP_TIME_SET_BIT 0
#endif

#define LOGLIB_APND_DISP_ALL_LOG_BIT      0x0
#define LOGLIB_APND_DISP_ERR_LOG_BIT      0x1
#define LOGLIB_APND_DISP_NOTY_LOG_BIT     0x2
#define LOGLIB_APND_DISP_DBG_LOG_BIT      0x4

#define LOGLIB_LVL_CNT                    4
#define LOGLIB_LVL_NONE                   0
#define LOGLIB_LVL_ERR                    1
#define LOGLIB_LVL_NOTY                   2
#define LOGLIB_LVL_DBG                    3

#define LOGLIB_WR_TYPE_DIR                1 /* default */
#define LOGLIB_WR_TYPE_THRD               2

#define LOGLIB_APND_TYPE_NONE             0
#define LOGLIB_APND_TYPE_STDOUT           1
#define LOGLIB_APND_TYPE_FILE             2 /* default */
#define LOGLIB_APND_TYPE_SYSLOG           3 
#define LOGLIB_APND_TYPE_RMT              4 /* remote */
#define LOGLIB_APND_TYPE_USR              5 /* user appender*/

#define LOGLIB_STDOUT_TYPE_ERR            1   
#define LOGLIB_STDOUT_TYPE_STD            2

#define LOGLIB_DFLT_MAX_LOG_SIZE          5242880 /* 500mb */

#define LOGLIB_LOG_PATH_MAX_LEN           1024
#define LOGLIB_LOG_NAME_MAX_LEN           24

#define LOGLIB_APND_NAME_MAX_LEN          128

/* error */
#define LOGERR_INVALID_WR_TYPE            100
#define LOGERR_LOG_FILE_OPEN_FAILED       101
#define LOGERR_LOG_DIR_CRTE_FAILED        102
#define LOGERR_INVLAID_LOG_LVL            103
#define LOGERR_INVALID_APND_NAME_LEN      104
#define LOGERR_INVALID_APND_TYPE          105
#define LOGERR_INVLAID_LOG_PATH_LEN       106
#define LOGERR_INVLAID_LOG_APND_NAME_LEN  107
#define LOGERR_APND_LNLLST_INIT_FAIELD    108
#define LOGERR_APND_INSERT_FAILED         109
#define LOGERR_APND_CFG_NOT_EXIST         110
#define LOGERR_THRD_QUE_INIT_FAILED       111
#define LOGERR_THRD_CRTE_FAILED           112
#define LOGERR_THRD_QUE_PUSH_FAILED       113
#define LOGERR_APND_ALREADY_EXIST         114
#define LOGERR_APND_NOT_EXIST             115
#define LOGERR_APND_DEL_FAILED            116
#define LOGERR_INVALID_FLAG               117
#define LOGERR_XML_PARSING_FAILED         118
#define LOGERR_APND_BUCKET_INIT_FAILED    119
#define LOGERR_APND_TYPE_NOT_EXSIT        120
#define LOGERR_APND_INVALID_TYPE          121
#define LOGERR_INVALID_STDOUT_TYPE        122
#define LOGERR_APNDER_INSERT_FAILED       123
#define LOGERR_LOG_CFG_NOT_EXIST          124
#define LOGERR_INVALID_SYSLOG_FAC_TYPE    125
#define LOGERR_MAND_APND_DATA_NOT_EXIST   126
#define LOGERR_LOG_NAME_LEN_TO_LONG       127
#define LOGERR_LOG_NAME_NOT_EXIST         128
#define LOGERR_LOG_REG_FAILED             129
#define LOGERR_GLOB_NOT_BEEN_INITTIALIZED 130
#define LOGERR_TOML_PARSING_FAILED        131
#define LOGERR_HASH_TBL_INIT_FAILED       132
#define LOGERR_LNK_LST_INIT_FAILED        133
#define LOGERR_ALREADY_EXIST              134
#define LOGERR_SYSLOG_FAC_NOT_EXIST       135

#define LOGLIB_INIT_CFG(_cfg){\
    (_cfg)->dfltLogLvl = LOGLIB_LVL_ERR;\
    (_cfg)->wrType = LOGLIB_WR_TYPE_DIR;\
    (_cfg)->dfltApndType = LOGLIB_APND_TYPE_STDOUT;\
    (_cfg)->dfltApndCfg.u.stdout.type = LOGLIB_STDOUT_TYPE_ERR;\
};

#define LOGLIB_INIT_APND_CFG(_apndCfg){\
    (_apndCfg)->dispBit = 0;\
    (_apndCfg)->logLvlBit = LOGLIB_APND_DISP_ALL_LOG_BIT;\
};

#ifdef LOGLIB_ENB_LVL
#if (LOGLIB_ENB_LVL >= 1)
#define LOGLIB_ERR_ENB

#if (LOGLIB_ENB_LVL >= 2)
#define LOGLIB_NOTY_ENB

#if (LOGLIB_ENB_LVL >= 3)
#define LOGLIB_DBG_ENB

#endif /* (LGOLIB_ENB_LVL >= 3) */
#endif /* (LGOLIB_ENB_LVL >= 2) */
#endif /* (LGOLIB_ENB_LVL >= 1) */
#else /* LOGLIB_ENB_LVL */
#define LOGLIB_ERR_ENB
#define LOGLIB_NOTY_ENB
#define LOGLIB_DBG_ENB
#endif /* LOGLIB_ENB_LVL */

#define LOGLIB_GLOB_INIT(){\
    loglib_apiGlobInit(LOGLIB_DISP_TIME_SET_BIT | LOGLIB_DISP_LVL_SET_BIT | \
                       LOGLIB_DISP_FILE_SET_BIT | LOGLIB_DISP_LINE_SET_BIT);\
}

#ifdef LOGLIB_ERR_ENB
#define LOGLIB_ERR(_name, ...){\
    if(loglib_apiGetMaxLogLvl() >= LOGLIB_LVL_ERR){\
        loglib_apiLogWrite((_name), LOGLIB_LVL_ERR, __BASE_FILE__, __LINE__, __VA_ARGS__);\
    }\
};
#else 
#define LOGLIB_ERR(_name, ...)
#endif

#ifdef LOGLIB_NOTY_ENB
#define LOGLIB_NOTY(_name, ...){\
    if(loglib_apiGetMaxLogLvl() >= LOGLIB_LVL_NOTY){\
        loglib_apiLogWrite((_name), LOGLIB_LVL_NOTY, __BASE_FILE__, __LINE__, __VA_ARGS__);\
    }\
};
#else
#define LOGLIB_NOTY(_name, ...)
#endif

#ifdef LOGLIB_DBG_ENB
#define LOGLIB_DBG(_name, ...){\
    if(loglib_apiGetMaxLogLvl() >= LOGLIB_LVL_DBG){\
        loglib_apiLogWrite((_name), LOGLIB_LVL_DBG, __BASE_FILE__, __LINE__, __VA_ARGS__);\
    }\
};
#else 
#define LOGLIB_DBG(_name, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LOGLIB_H_ */
