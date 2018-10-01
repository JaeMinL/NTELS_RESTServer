#ifndef _RSVLIB_API_H_
#define _RSVLIB_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RSV_MTHOD_GET                 1
#define RSV_MTHOD_POST                2
#define RSV_MTHOD_DEL                 3
#define RSV_MTHOD_HEAD                4
#define RSV_MTHOD_UPD                 5
#define RSV_MTHOD_OPT                 6
#define RSV_MTHOD_PUT                 7
#define RSV_MTHOD_TRACE               8
#define RSV_MTHOD_CON                 9

#define RSVERR_GLOB_CB_NOT_INIT       100
#define RSVERR_INVALID_LOG_LEVEL      101
#define RSVERR_LOG_FUNC_NULL          102
#define RSVERR_RSVLIB_CB_NOT_EXIST    103
#define RSVERR_ALREADY_RUNNING        104
#define RSVERR_UNSUPP_MTHOD           105
#define RSVERR_RULE_PARSE_FALED       106
#define RSVERR_ALLOC_FAILED           107
#define RSVERR_LNKLST_INSERT_FAILED   108
#define RSVERR_GLOB_CB_INIT_FAILED    109
#define RSVERR_RSVLIB_CB_EXIST        110
#define RSVERR_LNKLST_INIT_FAILED     111
#define RSVERR_RULE_INIT_FAILED       112
#define RSVERR_RSVLIB_CB_SET_FAILED   113
#define RSVERR_SVR_START_FAILED       114
#define RSVERR_RULE_NOT_EXIST         115
//#define RSVERR_LNKLST_INIT_FAILED     116

#define RSV_INIT_GEN_CFG(_cfg, _port, _thrdCnt){\
    (_cfg)->svrCfg.port = (_port);\
    (_cfg)->svrCfg.thrdCnt = (_thrdCnt);\
    (_cfg)->svrCfg.sslCfg.cert = NULL;\
    (_cfg)->svrCfg.sslCfg.key = NULL;\
}

#ifdef __cplusplus
}
#endif

#endif /* _RSVLIB_API_H_ */
