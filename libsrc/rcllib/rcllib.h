#ifndef _RCLLIB_H_
#define _RCLLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RCL_NONE                              0
#define RCL_ERR                               1
#define RCL_NOTY                              2
#define RCL_DBG                               3

#define RCLLIB_MTHOD_GET                      1
#define RCLLIB_MTHOD_POST                     2
#define RCLLIB_MTHOD_DEL                      3
#define RCLLIB_MTHOD_PUT                      4

#define RCLLIB_MAX_HDR_BUF                    1024

#define RCLERR_BUF_TOO_SMALL                  100
#define RCLERR_DAT_NOT_EXIST                  101
#define RCLERR_CPY_FAILED                     102
#define RCLERR_PERFORM_FAILED                 103
#define RCLERR_LNKLST_INIT_FAILED             104
#define RCLERR_ALLOC_FAILED                   105
#define RCLERR_LNKLST_INSERT_FAILED           106
#define RCLERR_INVALID_KEY_LEN                107
#define RCLERR_INVALID_VAL_LEN                108

#ifdef __cplusplus
}
#endif

#endif /* _RCLLIB_H_ */
