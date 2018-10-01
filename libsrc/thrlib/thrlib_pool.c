#include <unistd.h>
#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

#define THRLIB_POOL_CLEAR_LNKLST(_lnkLst){\
    ComlibLnkNode *d_lnkNode = NULL;\
    d_lnkNode = comlib_lnkLstGetFirst((_lnkLst));\
    if(d_lnkNode != NULL){\
        while(d_lnkNode != NULL){\
            comlib_memFree(d_lnkNode->data);\
            d_lnkNode = comlib_lnkLstGetFirst((_lnkLst));\
        }\
    }\
}

FT_PRIVATE RT_RESULT pool_crteWkr     (ThrlibPool *pool, struct timespec *waitTmOut);
FT_PRIVATE VOID      pool_clnWkr      (VOID *args);
FT_PRIVATE VOID      pool_wkrThrd     (VOID *args);

/* unlock function */
FT_PRIVATE RT_RESULT pool_crteWkr(ThrlibPool *pool, struct timespec *waitTmOut)
{
    SINT ret = RC_OK;
    ThrlibPoolWkr *wkr = NULL;
    ComlibLnkNode *lnkNode = NULL;

    GEN_CHK_ERR_RET(pool == NULL,
                    THR_LOG(THR_ERR,"Worker thread create failed(Thread pool structure not exist"),
                    THRERR_POOL_NULL);

    if(pool->wkrCnt > pool->wkrMaxCnt){
        THR_LOG(THR_ERR,"Worker is full\n");
        return THRERR_POOL_WORKER_IS_FULL;
    } 

    lnkNode = comlib_lnkLstGetFirst(&pool->waitWkr);
    if(lnkNode == NULL){
        wkr = comlib_memMalloc(sizeof(ThrlibPoolWkr));
        if(wkr == NULL){
            THR_LOG(THR_ERR,"Worker thread alloccate failed(err=%d:%s)\n",errno, strerror(errno));
            return THRERR_POOL_WKR_CRTE_FAILED;
        }
    }
    else {
        wkr = (ThrlibPoolWkr*)lnkNode->data;
    }

    pool->wkrCnt++;
    pool->idleWkrCnt++;

    wkr->pool = pool;

    if(waitTmOut == NULL){
        wkr->waitTmOut.tv_sec = 0; 
        wkr->waitTmOut.tv_nsec = 0; 
    }
    else {
        comlib_memMemcpy(&wkr->waitTmOut, waitTmOut, sizeof(struct timespec));
    }

    wkr->status = THR_POOL_STA_IDLE; 

    wkr->lnkNode.data = (VOID*)wkr;
    wkr->lnkNode.prev = NULL;
    wkr->lnkNode.next = NULL;

    ret = thrlib_thrdCrte(&wkr->tid, NULL, pool_wkrThrd, wkr);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Worker thread create failed(ret=%d)\n",ret);
        pool->wkrCnt--;
        pool->idleWkrCnt--;
        comlib_memFree(wkr);
        return THRERR_POOL_WKR_THRD_CRTE_FAILED;
    }

    return RC_OK;
}

FT_PRIVATE VOID pool_clnWkr(VOID *args)
{
    SINT ret = RC_OK; 
    ThrlibPoolWkr *wkr = NULL;
    ThrlibPool *pool = NULL; 

    wkr = (ThrlibPoolWkr*)args;

    pool = wkr->pool;

    /* regstration process */
    thrlib_mutxLock(&pool->mutx);

    ret = comlib_lnkLstDel(&pool->busyWkr, &wkr->lnkNode);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Linked list delete failed(ret=%d)\n",ret);
    }

    pool->wkrCnt--;

    wkr->tid = 0;
    wkr->waitTmOut.tv_nsec = 0;
    wkr->waitTmOut.tv_sec = 0;
    wkr->status = THR_POOL_STA_IDLE;

    ret = comlib_lnkLstInsertTail(&pool->waitWkr, &wkr->lnkNode);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Worker thread insert failed(ret=%d)\n",ret);
        comlib_memFree(wkr);
    } 

    /* regstration process */
    thrlib_mutxUnlock(&pool->mutx);
} 

FT_PRIVATE VOID pool_wkrThrd(VOID *args)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT jobIdx = 0;
    UINT sleepWaitCnt = 0;
    UINT strmMaxCnt = 0;
    struct timespec delay;
    struct timespec sleepTmOut;
    ThrlibPool *pool = NULL;
    ThrlibPoolWkr *wkr = NULL;
    ThrlibPoolJobQ *jobQ = NULL;
    ComlibLnkNode *lnkNode = NULL;
    ThrlibPoolJob *job = NULL;

    wkr = (ThrlibPoolWkr*)args;

    /* detach thread */
    thrlib_thrdDtch(wkr->tid);

    /* pthread only */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

    /* init thread */
    pool = wkr->pool;

    /* regstration process */
    thrlib_mutxLock(&pool->mutx);

    ret = comlib_lnkLstInsertTail(&pool->busyWkr, &wkr->lnkNode);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Worker thread insert failed(ret=%d)\n",ret);
        comlib_memFree(wkr);
        thrlib_mutxUnlock(&pool->mutx);
    } 

    wkr->status = THR_POOL_STA_RUN;
    pool->idleWkrCnt--;
    pool->busyWkrCnt++;

    strmMaxCnt = pool->strmMaxCnt;

    thrlib_mutxUnlock(&pool->mutx);

    /* cancel enable */
    thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    thrlib_thrdSetCnclType(THR_THRD_CNCL_DEFERRED, NULL);

    THRLIB_THRDCLNUP_PUSH(pool_clnWkr, wkr);

    jobIdx = 0;

    while(1){
        lnkNode = NULL;

        /* check term flag */
        if(pool->termFlg == RC_TRUE){
            /* close worker */
            thrlib_thrdExit(0);
        }

        /* check job */
        for(i=0;i<strmMaxCnt;i++){
            if(jobIdx == strmMaxCnt){
                jobIdx = 0;
            }
            jobQ = pool->jobQ[jobIdx];

            ret = thrlib_mutxTrylock(&jobQ->jobMutx);
            if(ret == THRERR_MUTX_BUSY){
                jobIdx++;
                continue;
            }

            lnkNode = comlib_lnkLstGetFirst(&jobQ->regJob);
            thrlib_mutxUnlock(&jobQ->jobMutx);
            if(lnkNode != NULL){
                break;
            }

            jobIdx++;
        }/* end of for(i=0;i<strmMaxCnt;i++) */

        if(lnkNode == NULL){
            sleepWaitCnt++;

            /* sleep */
            if(sleepWaitCnt > THR_POOL_COND_SLEEP_ST_CNT){
                thrlib_mutxLock(&pool->mutx);
                wkr->status = THR_POOL_STA_IDLE;
                pool->busyWkrCnt--;
                pool->idleWkrCnt++;

                ret = comlib_timerGetTime(&delay);
                comlib_memMemcpy(&sleepTmOut, &delay, sizeof(struct timespec));

                sleepTmOut.tv_sec += wkr->waitTmOut.tv_sec;
                sleepTmOut.tv_nsec += wkr->waitTmOut.tv_nsec;

                while(1){
                    ret = thrlib_condTmWait(&pool->cond, &pool->mutx, &delay);
                    if(ret == THRERR_COND_TMOUT){
                        ret = comlib_timerGetTime(&delay);

                        if(wkr->waitTmOut.tv_sec != 0 || wkr->waitTmOut.tv_nsec != 0){
                            /* check time */
                            if((delay.tv_sec > sleepTmOut.tv_sec) ||
                                    ((delay.tv_sec == sleepTmOut.tv_sec) && (delay.tv_nsec >= sleepTmOut.tv_nsec))){
                                if(pool->wkrCnt > pool->wkrMinCnt){
                                    /* close worker */
                                    thrlib_mutxUnlock(&pool->mutx);
                                    thrlib_thrdExit(0);
                                }
                            } 
                        }/* end of if(wkr->waitTmOut.tv_sec != 0 || wkr->waitTmOut.tv_nsec != 0) */
                    }
                    else {
                        break;
                    }

                    delay.tv_nsec += COM_TIMER_TICK_100MS;
                    if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
                        delay.tv_nsec -= COM_TIMER_TICK_SEC;
                        delay.tv_sec += 1;
                    }

                    /* check term flag */
                    if(pool->termFlg == RC_TRUE){
                        /* close worker */
                        thrlib_mutxUnlock(&pool->mutx);
                        thrlib_thrdExit(0);
                    }
                }/* end of while(1) */

                wkr->status = THR_POOL_STA_RUN;
                pool->idleWkrCnt--;
                pool->busyWkrCnt++;

                thrlib_mutxUnlock(&pool->mutx);

                sleepWaitCnt = 0;
            }/* if(sleepWaitCnt > THR_POOL_COND_SLEEP_ST_CNT) */
            else {
                usleep(500);
            }

            continue;
        }/* if(lnkNode == NULL) */

        sleepWaitCnt = 0;

        thrlib_thrdSetCnclSta(THR_THRD_CNCL_DISABLE, NULL);

        wkr->status = THR_POOL_STA_RUN;
        job = (ThrlibPoolJob*)lnkNode->data;

        /* process job */
        job->func(wkr->tid, job->args);

        wkr->status = THR_POOL_STA_IDLE;

        job->func = NULL;
        job->args = NULL;

        thrlib_mutxLock(&jobQ->jobMutx);

        ret = comlib_lnkLstInsertTail(&jobQ->idleJob, &job->lnkNode);
        if(ret != RC_OK){
            THR_LOG(THR_ERR, "Idle job insert failed(ret=%d)\n",ret);
            comlib_memFree(job);
        }
        thrlib_mutxUnlock(&jobQ->jobMutx);

        thrlib_thrdSetCnclSta(THR_THRD_CNCL_ENABLE,NULL);
    }/* end of while(1) */

    THRLIB_THRDCLNUP_POP(0);
}

FT_PUBLIC RT_RESULT thrlib_poolInit(ThrlibPool *pool, UINT maxWkrCnt, UINT minWkrCnt, UINT maxStrmCnt, UINT maxJobCnt)
{
    SINT ret = RC_OK;
    UINT i = 0;
    ThrlibPoolJobQ *jobQ = NULL;

    GEN_CHK_ERR_RET(pool == NULL,
                    THR_LOG(THR_ERR,"Thread pool structure is NULL\n"),
                    THRERR_POOL_NULL);

    if(maxWkrCnt < minWkrCnt){
        maxWkrCnt = minWkrCnt;
    }

    pool->wkrMinCnt = minWkrCnt;
    pool->wkrMaxCnt = maxWkrCnt;

    pool->wkrCnt = 0;
    pool->busyWkrCnt = 0;
    pool->idleWkrCnt = 0;

    pool->termFlg = RC_FALSE;

    /* maxJobCnt == 0 : unlimit */
    pool->jobMaxCnt = maxJobCnt;

    ret = comlib_lnkLstInit(&pool->busyWkr, maxWkrCnt);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Busy worker link list init failed(maxCnt=%d, ret=%d)\n",maxWkrCnt,ret);
        return THRERR_POOL_LNK_LST_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&pool->waitWkr, maxWkrCnt);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Wait worker link list init failed(maxCnt=%d, ret=%d)\n",maxWkrCnt,ret);
        return THRERR_POOL_LNK_LST_INIT_FAILED;
    }

    if(maxStrmCnt == 0){
        maxStrmCnt = 1;
    }

    pool->strmMaxCnt = maxStrmCnt;
    pool->nxtStrmId = 0;
    pool->jobQ = comlib_memMalloc(sizeof(ThrlibPoolJobQ*) *maxStrmCnt);

    for(i=0;i<maxStrmCnt;i++){
        pool->jobQ[i] = comlib_memMalloc(sizeof(ThrlibPoolJobQ));

        jobQ = pool->jobQ[i];

        ret = thrlib_mutxInit(&jobQ->jobMutx);
        if(ret != RC_OK){
            THR_LOG(THR_ERR,"Mutex init failed(ret=%d)\n",ret);
            return THRERR_POOL_MUTX_INIT_FAILED;
        }

        if(maxJobCnt == 0){
            ret = comlib_lnkLstInit(&jobQ->regJob, ~0);
            if(ret != RC_OK){
                THR_LOG(THR_ERR,"Registered Job link list init failed(maxCnt=unlimit, ret=%d)\n",ret);
                return THRERR_POOL_LNK_LST_INIT_FAILED;
            }

            ret = comlib_lnkLstInit(&jobQ->idleJob, ~0);
            if(ret != RC_OK){
                THR_LOG(THR_ERR,"Registered Job link list init failed(maxCnt=unlimit, ret=%d)\n",ret);
                return THRERR_POOL_LNK_LST_INIT_FAILED;
            }
        }
        else {
            ret = comlib_lnkLstInit(&jobQ->regJob, maxJobCnt);
            if(ret != RC_OK){
                THR_LOG(THR_ERR,"Registered Job link list init failed(maxCnt=unlimit, ret=%d)\n",ret);
                return THRERR_POOL_LNK_LST_INIT_FAILED;
            }

            ret = comlib_lnkLstInit(&jobQ->idleJob, maxJobCnt);
            if(ret != RC_OK){
                THR_LOG(THR_ERR,"Registered Job link list init failed(maxCnt=unlimit, ret=%d)\n",ret);
                return THRERR_POOL_LNK_LST_INIT_FAILED;
            }
        }
    }/* end of for(i=0;i<maxStrmCnt;i++) */

    ret = thrlib_mutxInit(&pool->mutx);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Mutex init failed(ret=%d)\n",ret);
        return THRERR_POOL_MUTX_INIT_FAILED;
    }

    ret = thrlib_condInit(&pool->cond);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"Cond init failed(ret=%d)\n",ret);
        return THRERR_POOL_COND_INIT_FAILED;
    } 

    /* create worker thread */ 
    thrlib_mutxLock(&pool->mutx);
    for(i=0;i<minWkrCnt;i++){
        ret = pool_crteWkr(pool, NULL);
        if(ret != RC_OK){
            THR_LOG(THR_ERR,"Worker create failed(i=%d, ret=%d)\n",i, ret);
        }
    }
    thrlib_mutxUnlock(&pool->mutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_poolDstry(ThrlibPool *pool)
{
    UINT i = 0; 
    ThrlibPoolJobQ *jobQ = NULL;

    pool->termFlg = RC_TRUE;

    /* wait */ 
    while(1){
        if(pool->wkrCnt == 0){
            /* clear job table */
            for(i=0;i<pool->strmMaxCnt;i++){
                jobQ = pool->jobQ[i];

                THRLIB_POOL_CLEAR_LNKLST(&jobQ->regJob);
                THRLIB_POOL_CLEAR_LNKLST(&jobQ->idleJob);

                thrlib_mutxDstry(&jobQ->jobMutx);

                comlib_memFree(jobQ);
            }/* end of for(i=0;i<strmMaxCnt;i++) */

            comlib_memFree(pool->jobQ);

            /* clear worker table */
            THRLIB_POOL_CLEAR_LNKLST(&pool->busyWkr);
            THRLIB_POOL_CLEAR_LNKLST(&pool->waitWkr);

            /* clear all */
            thrlib_mutxDstry(&pool->mutx);
            thrlib_condDstry(&pool->cond);

            comlib_memMemset(pool, 0x0, sizeof(ThrlibPool));

            break;
        }/* end of if(pool->wkrCnt != 0) */

        usleep(500);
    }/* end of while(1) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_poolRegJob(ThrlibPool *pool, UINT strmId, ThrlibThrdJobFunc func, VOID *args)
{
    SINT ret = RC_OK;
    ComlibLnkNode *lnkNode = NULL; 
    ThrlibPoolJob *job = NULL;
    ThrlibPoolJobQ *jobQ = NULL;

    GEN_CHK_ERR_RET(pool == NULL,
                    THR_LOG(THR_ERR,"Thread pool structure not exist\n"),
                    THRERR_POOL_NULL);

    GEN_CHK_ERR_RET(func == NULL,
                    THR_LOG(THR_ERR,"Thread function not exist\n"),
                    THRERR_POOL_JOB_FUNC_NOT_EXIST);

    thrlib_mutxLock(&pool->mutx);

    if(pool->termFlg == RC_TRUE){
        thrlib_mutxUnlock(&pool->mutx);
        return THRERR_POOL_TERM_WAIT;
    }

    if(strmId == 0){
        strmId = pool->nxtStrmId;
        pool->nxtStrmId++;
        if(pool->nxtStrmId >= pool->strmMaxCnt){
            pool->nxtStrmId = 0;
        }
    }
    else {
        if(strmId >= pool->strmMaxCnt){
            thrlib_mutxUnlock(&pool->mutx);
            return THRERR_POOL_INVAL_STRM_ID;
        }
    }

    /* check empty stream */
goto_chkNxtStrmId:
    jobQ = pool->jobQ[strmId];

    ret = thrlib_mutxTrylock(&jobQ->jobMutx);
    if(ret == THRERR_MUTX_BUSY){
        pool->nxtStrmId++;
        if(pool->nxtStrmId >= pool->strmMaxCnt){
            pool->nxtStrmId = 0;
        }

        strmId = pool->nxtStrmId;

        goto goto_chkNxtStrmId;
    }

    /* job registration */
    lnkNode = comlib_lnkLstGetFirst(&jobQ->idleJob);
    if(lnkNode == NULL){
        job = comlib_memMalloc(sizeof(ThrlibPoolJob));
        job->lnkNode.data = job;
    }
    else {
        job = lnkNode->data;
    }

    job->lnkNode.prev = NULL;
    job->lnkNode.next = NULL;

    job->func = func;
    job->args = args;

    if((jobQ->regJob.nodeCnt != 0) && 
       (pool->busyWkrCnt ==  pool->wkrCnt) && 
       (pool->wkrCnt < pool->wkrMaxCnt)){
        struct timespec waitTmOut; 

        /* create new worker */
        waitTmOut.tv_sec = THR_POOL_COND_SLEEP_TM; 
        waitTmOut.tv_nsec = 0;
        ret = pool_crteWkr(pool, &waitTmOut);
        if(ret != RC_OK){
            THR_LOG(THR_ERR,"Worker thread create failed(ret=%d)\n",ret); 
            /* continue */
        }
    }

    ret = comlib_lnkLstInsertTail(&jobQ->regJob, &job->lnkNode);
    if(ret == COMERR_MAX_NODE){
        thrlib_mutxUnlock(&jobQ->jobMutx);
        thrlib_mutxUnlock(&pool->mutx);

        return THRERR_POOL_JOB_IS_FULL;
    }
    else if(ret != RC_OK){
        THR_LOG(THR_ERR,"Reg job queue insert failed(ret=%d)\n",ret);
        thrlib_mutxUnlock(&jobQ->jobMutx);
        thrlib_mutxUnlock(&pool->mutx);

        return THRERR_POOL_LNK_LST_INSERT_FAILED;
    }

    thrlib_mutxUnlock(&jobQ->jobMutx);

    thrlib_condSig(&pool->cond);

    thrlib_mutxUnlock(&pool->mutx);

    return RC_OK;
}

