/************************************************************************

     Name:     Thread library 

     Type:     C header file

     Desc:     Thread library definition and functions  

     File:     thrlib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _THRLIB_H_
#define _THRLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* thread common library include */

/* support 
** 1. posix thread 
*/
#include <pthread.h>

/*--------------------------------------------------------------*/
/*                 thread libary definition                     */ 
/*--------------------------------------------------------------*/
/* thread library log level */
#define THR_NONE                          0
#define THR_ERR                           1
#define THR_NOTY                          2
#define THR_DBG                           3

/*------- Thread queue definition -------*/
#define THR_TQ_LOCK_TYPE_LOCKED           1
#define THR_TQ_LOCK_TYPE_LOCK_FREE        2

#define THR_TQ_ELMT_TYPE_PTR              1
#define THR_TQ_ELMT_TYPE_MEM              2

/*------- Thread pool definition --------*/
#define THR_POOL_COND_SLEEP_ST_CNT        5
#define THR_POOL_COND_SLEEP_TM            10 /* sec */

#define THR_POOL_STA_IDLE                 1
#define THR_POOL_STA_RUN                  2

#define THR_THRD_CNCL_ENABLE              PTHREAD_CANCEL_ENABLE
#define THR_THRD_CNCL_DISABLE             PTHREAD_CANCEL_DISABLE
#define THR_THRD_CNCL_DEFERRED            PTHREAD_CANCEL_DEFERRED
#define THR_THRD_CNCL_ASYNC               PTHREAD_CANCEL_ASYNCHRONOUS

/* thred library error code definiation */
/* Thread definition */ 
#define THRERR_THRD_AGAIN                 100
#define THRERR_THRD_INVAL                 101
#define THRERR_THRD_UNKNOWN               102
#define THRERR_THRD_INVAL_ID              103
#define THRERR_THRD_NOMEM                 104
#define THRERR_THRD_ESRCH                 105
#define THRERR_THRD_DLOCK                 106

/* Mutex definition */
#define THRERR_MUTX_UNKNOWN               200
#define THRERR_MUTX_AGAIN                 201
#define THRERR_MUTX_INVAL                 202
#define THRERR_MUTX_NOMEM                 203
#define THRERR_MUTX_DEADLK                204
#define THRERR_MUTX_PERM                  205
#define THRERR_MUTX_BUSY                  206

/* Condition definition */  
#define THRERR_COND_UNKNOWN               300
#define THRERR_COND_AGAIN                 301
#define THRERR_COND_INVAL                 302
#define THRERR_COND_NOMEM                 303
#define THRERR_COND_BUSY                  304
#define THRERR_COND_TMOUT                 305

/* thread queue */
#define THRERR_TQ_NULL                    400
#define THRERR_TQ_INVALID_SIZE            401
#define THRERR_TQ_INVALID_LOCK_TYPE       402
#define THRERR_TQ_INVALID_ELMNT_TYPE      403
#define THRERR_TQ_ALLOC_FAILED            404
#define THRERR_TQ_MUTX_INIT_FAILED        405
#define THRERR_TQ_COND_INIT_FAILED        406
#define THRERR_TQ_COND_FAILED             407
#define THRERR_TQ_ELMNT_IS_NULL           408
#define THRERR_TQ_FULL                    409
#define THRERR_TQ_EMPTY                   410
#define THRERR_TQ_TIMEOUT                 411
#define THRERR_TQ_MUTX_DSTRY_FAILED       412
#define THRERR_TQ_COND_DSTRY_FAILED       413

/* thread pool */
#define THRERR_POOL_NULL                  500
#define THRERR_POOL_LNK_LST_INIT_FAILED   501
#define THRERR_POOL_MUTX_INIT_FAILED      502
#define THRERR_POOL_WORKER_IS_FULL        503
#define THRERR_POOL_JOB_IS_FULL           504
#define THRERR_POOL_LNK_LST_INSERT_FAILED 506
#define THRERR_POOL_COND_INIT_FAILED      507
#define THRERR_POOL_WKR_CRTE_FAILED       508
#define THRERR_POOL_WKR_THRD_CRTE_FAILED  509
#define THRERR_POOL_TERM_WAIT             510
#define THRERR_POOL_INVAL_STRM_ID         511
#define THRERR_POOL_JOB_FUNC_NOT_EXIST    512

/*--------------------------------------------------------------*/
/*                    thread library macro                      */ 
/*--------------------------------------------------------------*/
/* Thread library log */
#ifdef THRLIB_LOG
#define THR_LOG(LEVEL,...){\
    fprintf(stderr,__VA_ARGS__);\
}
#else
#define THR_LOG(LEVEL,...)
#endif

/* clean up macro */
#define THRLIB_THRDCLNUP_PUSH(thrdClnUpFunc, arg) pthread_cleanup_push(thrdClnUpFunc, arg);

#define THRLIB_THRDCLNUP_POP(exec) pthread_cleanup_pop(exec);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _THRLIB_H_ */
