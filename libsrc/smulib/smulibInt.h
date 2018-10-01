#ifndef _SMU_INT_H_
#define _SMU_INT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SMULIB_STR_PID_MAX_LEN               12

#define SMU_MAIN_BLK_STA_ACT                 1
#define SMU_MAIN_BLK_STA_INACT               2

#define SMULIB_SMD_STA_CLOSE                 0
#define SMULIB_SMD_STA_REG_WAIT              1
#define SMULIB_SMD_STA_OPEN                  2

#define SMULIB_TMR_EVNT_REG_SND_TMOUT        1
#define SMULIB_TMR_EVNT_HB_SND_TMOUT         2
#define SMULIB_TMR_EVNT_APP_HB_TMOUT         3
#define SMULIB_TMR_EVNT_FREEZE_TMOUT         4

#define SMULIB_MAX_LOG_BUF_LEN               4096

#define SMULIB_MSG_CODE_HB                   1

#ifndef SMULIB_LOG
#define SMU_LOG(_LEVEL,...)
#else 
#define SMU_LOG(_LEVEL,...){\
    UINT d_logLvl = 0;\
    d_logLvl = smulibInt_globGetLogLvl();\
    if(_LEVEL <= d_logLvl){\
        smulibInt_globLogPrnt(_LEVEL, __FILE__, __LINE__, __VA_ARGS__);\
    }\
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SMU_INT_H_ */
