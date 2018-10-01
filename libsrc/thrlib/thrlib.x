/************************************************************************

     Name:     Thread library 

     Type:     C structure file

     Desc:     Thread library structure

     File:     thrlib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _THRLIB_X_
#define _THRLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

/* POSIX thread */
typedef pthread_t            ThrlibThrdId;
typedef pthread_attr_t       ThrlibThrdAttr;
typedef pthread_mutex_t      ThrlibMutx;
typedef pthread_cond_t       ThrlibCond;

typedef struct ThrlibTq       ThrlibTq;
typedef struct ThrlibQue      ThrlibQue;
typedef struct ThrlibPool     ThrlibPool;
typedef struct ThrlibPoolWkr  ThrlibPoolWkr;
typedef struct ThrlibPoolJob  ThrlibPoolJob;
typedef struct ThrlibPoolJobQ ThrlibPoolJobQ;

/* thraed function type */
typedef VOID (*ThrlibThrdFunc)      (VOID*);
typedef VOID (ThrlibThrdClnUpFunc)  (VOID*);
typedef VOID (*ThrlibThrdJobFunc)   (ThrlibThrdId, VOID*);

#define CACHE_LINE_SIZE 64
#if 0
#define CACHE_ALIGN __attribute__((aligned(CACHE_LINE_SIZE)))
#else 
#define CACHE_ALIGN 
#endif

/* Circular array queue */
struct ThrlibQue{
    CACHE_ALIGN VOLATILE UINT       first;        /* firset element */
    CACHE_ALIGN VOLATILE UINT       last;         /* last element */
    CACHE_ALIGN VOLATILE SINT       qCnt;         /* queueing count */
    CACHE_ALIGN VOLATILE UINT       maxCnt;       /* max element */
    VOID                            **elmnt;      /* queue element */
};

struct ThrlibTq{
    CACHE_ALIGN UINT                lockType;     /* lock less queue or locked queue */
    CACHE_ALIGN UINT                high;         /* high func */
    CACHE_ALIGN UINT                low;          /* low func */
    CACHE_ALIGN UINT                term;         /* termination flag */
    CACHE_ALIGN UINT                fWaiter;      /* full waiter */
    CACHE_ALIGN UINT                eWaiter;      /* empty waiter */
    CACHE_ALIGN ThrlibMutx          pushMutx;     /* push queue mutex */
    CACHE_ALIGN ThrlibMutx          popMutx;      /* pop queue mutex */
    CACHE_ALIGN ThrlibCond          fCond;        /* full queue condition */
    CACHE_ALIGN ThrlibCond          eCond;        /* empty queue condition */
    CACHE_ALIGN ThrlibQue           que;          /* array queue bucket */
};

struct ThrlibPoolWkr{ /* thread */
    ThrlibThrdId                    tid;          /* pthread id */
    ThrlibPool                      *pool;
    struct timespec                 waitTmOut;
    UINT                            status;
    ComlibLnkNode                   lnkNode;
};

struct ThrlibPoolJob{ /* thread job */
    ThrlibThrdJobFunc                func;
    VOID                            *args;
    ComlibLnkNode                   lnkNode;
};

struct ThrlibPoolJobQ{
    ThrlibMutx                      jobMutx;
    ComlibLnkLst                    regJob;
    ComlibLnkLst                    idleJob;
};

struct ThrlibPool{ /* thread pool main */
    UINT                            wkrMinCnt;
    UINT                            wkrMaxCnt;
    UINT                            wkrCnt;
    UINT                            busyWkrCnt;
    UINT                            idleWkrCnt;
    UINT                            termFlg; /* termination flag */
    ThrlibMutx                      mutx;
    ThrlibCond                      cond; 
    ComlibLnkLst                    busyWkr;
    ComlibLnkLst                    waitWkr;
    UINT                            jobMaxCnt;
    UINT                            strmMaxCnt;
    UINT                            nxtStrmId; 
    ThrlibPoolJobQ                  **jobQ;
};

/* thread functions */
FT_PUBLIC RT_RESULT     thrlib_thrdCrte          (ThrlibThrdId *thrdId, ThrlibThrdAttr *thrdAttr, ThrlibThrdFunc thrdFunc, 
                                                  VOID *arg);
#ifdef __cplusplus
FT_PUBLIC ThrlibThrdId  thrlib_thrdSelf          ();
#else
FT_PUBLIC ThrlibThrdId  thrlib_thrdSelf          (VOID);
#endif
FT_PUBLIC RT_RESULT     thrlib_thrdJoin          (ThrlibThrdId thrdId, VOID **rt_retVal);
FT_PUBLIC RT_RESULT     thrlib_thrdCancel        (ThrlibThrdId thrdId);  
FT_PUBLIC VOID          thrlib_thrdExit          (VOID *ret_val);
FT_PUBLIC RT_RESULT     thrlib_thrdDtch          (ThrlibThrdId thrdId);
FT_PUBLIC RT_RESULT     thrlib_thrdSetCnclSta    (SINT sta, SINT *oldSta);  
FT_PUBLIC RT_RESULT     thrlib_thrdSetCnclType   (SINT type, SINT *oldType);  
FT_PUBLIC RT_RESULT     thrlib_thrdAttrInit      (ThrlibThrdAttr *thrdAttr);  
FT_PUBLIC RT_RESULT     thrlib_thrdAttrSetDtchSta(ThrlibThrdAttr *thrdAttr, SINT dtchSta);  
FT_PUBLIC RT_RESULT     thrlib_thrdAttrGetDtchSta(ThrlibThrdAttr *thrdAttr, SINT *dtchSta);  

/* mutex functions */
FT_PUBLIC RT_RESULT     thrlib_mutxInit          (ThrlibMutx *mutx);
FT_PUBLIC RT_RESULT     thrlib_mutxDstry         (ThrlibMutx *mutx);
FT_PUBLIC RT_RESULT     thrlib_mutxLock          (ThrlibMutx *mutx);
FT_PUBLIC RT_RESULT     thrlib_mutxTrylock       (ThrlibMutx *mutx);
FT_PUBLIC RT_RESULT     thrlib_mutxUnlock        (ThrlibMutx *mutx);

/* condition functions */
FT_PUBLIC RT_RESULT     thrlib_condInit          (ThrlibCond *cond);
FT_PUBLIC RT_RESULT     thrlib_condDstry         (ThrlibCond *cond);
FT_PUBLIC RT_RESULT     thrlib_condWait          (ThrlibCond *cond, ThrlibMutx *mutx);
FT_PUBLIC RT_RESULT     thrlib_condTmWait        (ThrlibCond *cond, ThrlibMutx *mutx, struct timespec *wait);
FT_PUBLIC RT_RESULT     thrlib_condSig           (ThrlibCond *cond);
FT_PUBLIC RT_RESULT     thrlib_condBrdcast       (ThrlibCond *cond);

/* thread queue functions */
FT_PUBLIC RT_RESULT     thrlib_tqInit            (ThrlibTq *tq, UINT lockType, UINT size);
FT_PUBLIC RT_RESULT     thrlib_tqDstry           (ThrlibTq *tq);
FT_PUBLIC RT_RESULT     thrlib_tqPush            (ThrlibTq *tq, VOID *elmnt);
FT_PUBLIC RT_RESULT     thrlib_tqPushBulk        (ThrlibTq *tq, VOID **elmnt, UINT elmntCnt, UINT *rt_sndElmntCnt);
FT_PUBLIC RT_RESULT     thrlib_tqWaitPush        (ThrlibTq *tq, VOID *elmnt, struct timespec *wait);
#if 0
FT_PUBLIC RT_RESULT     thrlib_tqWaitPushBulk    (ThrlibTq *tq, VOID **elmnt, UINT elmntCnt, UINT *rt_sndCnt, 
                                                  struct timespec *wait);
#endif
FT_PUBLIC RT_RESULT     thrlib_tqPop             (ThrlibTq *tq, VOID **rt_elmnt);
FT_PUBLIC RT_RESULT     thrlib_tqWaitPop         (ThrlibTq *tq, VOID **rt_elmnt, struct timespec *wait);

/* thread pool functions */
FT_PUBLIC RT_RESULT     thrlib_poolInit          (ThrlibPool *pool, UINT maxWkrCnt, UINT minWkrCnt, UINT maxStrmCnt, 
                                                  UINT maxJobCnt);
FT_PUBLIC RT_RESULT     thrlib_poolDstry         (ThrlibPool *pool);
FT_PUBLIC RT_RESULT     thrlib_poolRegJob        (ThrlibPool *pool, UINT strmId, ThrlibThrdJobFunc func, VOID *args);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _THRLIB_X_ */
