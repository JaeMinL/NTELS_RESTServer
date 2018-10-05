/************************************************************************

     Name:     Log library 

     Type:     C header file

     Desc:     Log library internal definition and functions  

     File:     loglibInt.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _LOGLIB_INT_H_
#define _LOGLIB_INT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* log library internal level */
#define LOG_INT_NONE                         0
#define LOG_INT_ERR                          1
#define LOG_INT_NOTY                         2
#define LOG_INT_DBG                          3

#define LOGLIB_MAX_ENTRY_CNT                 256

#define LOGLIB_LOG_MAX_BUF_LEN               3120

#define LOGLIB_THRD_MSG_CODE_LOG             1

#define LOGLIB_THRD_MSG_AVP_CODE_LVL         1
#define LOGLIB_THRD_MSG_AVP_CODE_FNAME       2
#define LOGLIB_THRD_MSG_AVP_CODE_LINE        3
#define LOGLIB_THRD_MSG_AVP_CODE_DATA        4

/* log library log */
#ifdef LOGLIB_LOG
#define LOG_LOG(LEVEL,...){\
    fprintf(stderr,__VA_ARGS__);\
}
#else
#define LOG_LOG(LEVEL,...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LOGLIB_INT_H_ */
