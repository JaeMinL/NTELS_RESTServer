/************************************************************************

     Name:     Software Management User library 

     Type:     C include file

     Desc:     Software Management User library definition and macro

     File:     smulib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _SMULIB_H_
#define _SMULIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SMULIB_DFLT_RLY_THRD_CNT           1
#define SMULIB_DFLT_FREEZE_TM_OUT          30
#define SMULIB_DFLT_HB_SND_TM_OUT          10
#define SMULIB_DFLT_REG_SND_TM_OUT         10
#define SMULIB_DFLT_THRD_TM_OUT            30

#define SMUERR_THRD_BLK_CRTE_FAILED        100
#define SMUERR_THRD_BLK_DEL_FAILED         101
#define SMUERR_THRD_BLK_LNKLST_INIT_FAILED 102
#define SMUERR_SIG_HASHTBL_INIT_FAILED     103
#define SMUERR_SNDQ_INIT_FAILED            104
#define SMUERR_INVALID_KILL_FLAG           105
#define SMUERR_GLOB_CB_NOT_INIT            106
#define SMUERR_INVALID_LOG_LEVEL           107
#define SMUERR_NAME_NOT_EXIST              108
#define SMUERR_NAME_TOO_LONG               109
#define SMUERR_THRD_CRTE_FAILED            110
#define SMUERR_SIG_ALREADY_REG             111
#define SMUERR_SIG_REG_FAILED              112
#define SMUERR_SMD_ADDR_ENV_NOT_EXIST      113
#define SMUERR_SMD_ADDR_PARSING_FAILED     114
#define SMUERR_GLOB_CB_INIT_FIRST          115
#define SMUERR_RLY_INIT_FAILED             116
#define SMUERR_TIMER_INIT_FAILED           117
#define SMUERR_TIMER_TBL_INIT_FAILED       118
#define SMUERR_RLY_HOST_ADD_FAILED         119
#define SMUERR_RLY_CONN_ADD_FAILED         120
#define SMUERR_REG_REG_SND_FAILED          121
#define SMUERR_REG_REG_TMOUT_START_FAILED  122

#define SMU_NONE                           0
#define SMU_ERR                            1
#define SMU_NOTY                           2
#define SMU_DBG                            3

#define SMULIB_INIT_OPT_ARG(_optArg){\
    (_optArg)->rlyThrdCnt = SMULIB_DFLT_RLY_THRD_CNT;\
    (_optArg)->freezeTmOut = SMULIB_DFLT_FREEZE_TM_OUT;\
    (_optArg)->hbSndTmOut = SMULIB_DFLT_HB_SND_TM_OUT;\
    (_optArg)->regSndTmOut = SMULIB_DFLT_REG_SND_TM_OUT;\
    (_optArg)->tmOutFunc = NULL;\
    (_optArg)->logLvl = SMU_ERR;\
    (_optArg)->logFunc = NULL;\
}

#define SMULIB_INIT_THRD_BLK_CFG(_cfg){\
    (_cfg)->thrdTmOut = SMULIB_DFLT_THRD_TM_OUT;\
}

#ifdef __cplusplus
}
#endif

#endif /* _SMULIB_H_ */
