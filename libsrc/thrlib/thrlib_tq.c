#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

/* Private macro */
#define WAIT_CHECK(_que, _waiter, _cond, _mutx, _delay, _delayFlg, _ret){\
    (_ret) = RC_OK;\
    /* wait signal */\
    (_waiter)++;\
    if((_delayFlg) == RC_FALSE){\
        (_ret) = thrlib_condWait((_cond), (_mutx));\
        if((_ret) != RC_OK){\
            THR_LOG(THR_ERR,"Cond wait failed(ret=%d)\n",(_ret));\
            (_waiter)--;\
        }\
    }/* if((_wait) == NULL) */\
    else {\
        (_ret) = thrlib_condTmWait((_cond), (_mutx), (_delay));\
        if((_ret) == THRERR_COND_TMOUT){\
            (_waiter)--;\
            ret = THRERR_TQ_TIMEOUT;\
        }\
        else if((_ret) != RC_OK){\
            THR_LOG(THR_ERR,"cond timed wait error(ret=%d)\n",(_ret));\
            (_waiter)--;\
        }\
    }/* end of else */\
    (_waiter)--;\
}

FT_PUBLIC RT_RESULT thrlib_tqInit(ThrlibTq *tq, UINT lockType, UINT size)
{
    UINT ret = RC_OK;
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
            THR_LOG(THR_ERR,"Thread queue structure is NULL (lockType=%d, size=%d)\n",lockType, size),
            THRERR_TQ_NULL);

    GEN_CHK_ERR_RET(lockType != THR_TQ_LOCK_TYPE_LOCKED &&
            lockType != THR_TQ_LOCK_TYPE_LOCK_FREE,
            THR_LOG(THR_ERR,"Invalid thread queue lockType(lockType=%d, size=%d)\n",lockType, size),
            THRERR_TQ_INVALID_LOCK_TYPE);

    GEN_CHK_ERR_RET(size == 0,
            THR_LOG(THR_ERR,"thread queue size is zero(lockType=%d)\n",lockType),
            THRERR_TQ_INVALID_SIZE);

    /* set tq info */
    tq->lockType = lockType;
    tq->high = 0;
    tq->low = 0;
    tq->term = 0;
    tq->fWaiter = 0;
    tq->eWaiter = 0;

    /* init mutex */
    ret = thrlib_mutxInit(&tq->pushMutx);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"push mutex init failed(ret=%d)\n",ret);
        return THRERR_TQ_MUTX_INIT_FAILED;
    }

    ret = thrlib_mutxInit(&tq->popMutx);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"pop mutex init failed(ret=%d)\n",ret);
        return THRERR_TQ_MUTX_INIT_FAILED;
    }

    /* init cond */
    ret = thrlib_condInit(&tq->fCond);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"condition init failed(ret=%d)\n",ret);
        return THRERR_TQ_COND_INIT_FAILED;
    }

    ret = thrlib_condInit(&tq->eCond);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"condition init failed(ret=%d)\n",ret);
        return THRERR_TQ_COND_INIT_FAILED;
    }

    /* create queue */
    que = &tq->que;
    que->qCnt = 0;
    que->maxCnt = size;
    que->first = 0;
    que->last = 0;

    //que->elmnt = comlib_memMalloc(sizeof(VOID*) * (size+1));
    que->elmnt = comlib_memMalloc(sizeof(VOID*) * (size));
    if(que->elmnt == NULL){
        THR_LOG(THR_ERR,"thread queue alloc failed\n");
        return THRERR_TQ_ALLOC_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_tqDstry(ThrlibTq *tq)
{
    UINT ret = RC_OK;
    ThrlibQue *que = NULL;

    if(tq == NULL){
        THR_LOG(THR_ERR,"Thread queue structure is NULL)\n");
        return THRERR_TQ_NULL;
    }

    que = &tq->que;

    ret = thrlib_condDstry(&tq->fCond);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"fCond destroy failed(ret=%d)\n",ret);
        return THRERR_TQ_COND_DSTRY_FAILED;
    }
    ret = thrlib_condDstry(&tq->eCond);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"eCond destroy failed(ret=%d)\n",ret);
        return THRERR_TQ_COND_DSTRY_FAILED;
    }

    ret = thrlib_mutxDstry(&tq->pushMutx);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"push mutex destroy failed(ret=%d)\n",ret);
        return THRERR_TQ_MUTX_DSTRY_FAILED;
    }

    ret = thrlib_mutxDstry(&tq->popMutx);
    if(ret != RC_OK){
        THR_LOG(THR_ERR,"pop mutex destroy failed(ret=%d)\n",ret);
        return THRERR_TQ_MUTX_DSTRY_FAILED;
    }

    comlib_memFree(que->elmnt);

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_tqPushBulk(ThrlibTq *tq, VOID **elmnt, UINT elmntCnt, UINT *rt_sndElmntCnt)
{
    UINT cpyLen = 0;
    UINT remLen = 0;
    UINT first = 0;
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
            THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
            THRERR_TQ_NULL);

    GEN_CHK_ERR_RET(elmnt == NULL,
            THR_LOG(THR_ERR,"element is NULL\n"),
            THRERR_TQ_ELMNT_IS_NULL);

    que = &tq->que;

    /* lock */
    thrlib_mutxLock(&tq->pushMutx);

    /* snapshot */
    first = que->first;

    if(first < que->last){
        remLen = (que->maxCnt - que->last) + first;
    }
    else{
        remLen = que->maxCnt - (first - que->last);
    }

    if(remLen == 0){
        thrlib_mutxUnlock(&tq->pushMutx);
        return THRERR_TQ_FULL;
    }

    if(remLen < elmntCnt){
        elmntCnt = remLen;
    }

    if((que->last + elmntCnt) >= que->maxCnt){
        UINT remCpyLen = 0;

        cpyLen = que->maxCnt - que->last;
        remCpyLen = elmntCnt - cpyLen;

        comlib_memMemcpy(&que->elmnt[que->last], elmnt, sizeof(VOID*) * cpyLen);
        comlib_memMemcpy(&que->elmnt[0], &elmnt[cpyLen],  sizeof(VOID*) * remCpyLen);

        que->last = remCpyLen;
    }
    else {
        comlib_memMemcpy(&que->elmnt[que->last], &elmnt[0], sizeof(VOID*) *  elmntCnt);
        que->last += elmntCnt;
    }

    que->qCnt += elmntCnt;

    /* send signal */
    if(tq->eWaiter){
        thrlib_condSig(&tq->eCond);
    }

    /* unlock */
    thrlib_mutxUnlock(&tq->pushMutx);

    (*rt_sndElmntCnt) = elmntCnt;

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_tqPush(ThrlibTq *tq, VOID *elmnt)
{
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
            THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
            THRERR_TQ_NULL);

    GEN_CHK_ERR_RET(elmnt == NULL,
            THR_LOG(THR_ERR,"element is NULL\n"),
            THRERR_TQ_ELMNT_IS_NULL);

    que = &tq->que;

    /* lock */
    thrlib_mutxLock(&tq->pushMutx);

    if(que->last == (que->maxCnt)){
        if(que->first == 0){
            thrlib_mutxUnlock(&tq->pushMutx);
            return THRERR_TQ_FULL;
        }
    }
    else {
        if(que->last == (que->first -1)){
            thrlib_mutxUnlock(&tq->pushMutx);
            return THRERR_TQ_FULL;
        }
    }

    que->elmnt[que->last] = elmnt;

    if(que->last == (que->maxCnt-1)){
        que->last = 0;
    }
    else {
        que->last++;
    }

    que->qCnt++;

    /* send signal */
    if(tq->eWaiter){
        thrlib_condSig(&tq->eCond);
    }

    /* unlock */
    thrlib_mutxUnlock(&tq->pushMutx);

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_tqWaitPush(ThrlibTq *tq, VOID *elmnt, struct timespec *wait)
{
    UINT ret = RC_OK;
    struct timespec delay;
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
            THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
            THRERR_TQ_NULL);

    if(tq->lockType == THR_TQ_LOCK_TYPE_LOCK_FREE){
        THR_LOG(THR_ERR,"lock type is Lock free\n");
        return THRERR_TQ_INVALID_LOCK_TYPE;
    }

    if(elmnt == NULL){
        THR_LOG(THR_ERR,"element is NULL\n");
        return THRERR_TQ_ELMNT_IS_NULL;
    }

    que = &tq->que;

    thrlib_mutxLock(&tq->pushMutx);

    if(que->qCnt >= que->maxCnt){
        UINT delayFlg = RC_FALSE;

        if(wait != NULL){
            /* get current time */
            ret = comlib_timerGetTime(&delay);

            /* update time */
            delay.tv_sec += wait->tv_sec;
            delay.tv_nsec += wait->tv_nsec;
            if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
                delay.tv_nsec -= COM_TIMER_TICK_SEC;
                delay.tv_sec += 1;
            }
            delayFlg = RC_TRUE;
        }

        do{
            WAIT_CHECK((que), (tq->fWaiter), &(tq->fCond), &(tq->pushMutx), &(delay), (delayFlg), (ret));
            if(ret != RC_OK){
                thrlib_mutxUnlock(&tq->pushMutx);
                if(ret == THRERR_TQ_TIMEOUT) {
                    return ret;
                }
                else {
                    THR_LOG(THR_ERR,"wait check failed(ret=%d)\n",ret);
                    return ret;
                }
            }
        }while(que->qCnt < que->maxCnt);
    }/* end of if(que->qCnt >= que->maxCnt) */

    que->elmnt[que->last] = elmnt;

    if(que->last == (que->maxCnt-1)){
        que->last = 0;
    }
    else {
        que->last++;
    }

    que->qCnt++;

    /* send signal */
    if(tq->eWaiter){
        thrlib_condSig(&tq->eCond);
    }

    thrlib_mutxUnlock(&tq->pushMutx);

    return RC_OK;
}

#if 0
FT_PUBLIC RT_RESULT thrlib_tqWaitPushBulk(ThrlibTq *tq, VOID **elmnt, UINT elmntCnt, 
                                          struct timespec *wait, UINT *rt_sndElmntCnt)
{
    UINT ret = RC_OK;
    struct timespec delay;
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
            THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
            THRERR_TQ_NULL);

    if(tq->lockType == THR_TQ_LOCK_TYPE_LOCK_FREE){
        THR_LOG(THR_ERR,"lock type is Lock free\n");
        return THRERR_TQ_INVALID_LOCK_TYPE;
    }

    if(elmnt == NULL){
        THR_LOG(THR_ERR,"element is NULL\n");
        return THRERR_TQ_ELMNT_IS_NULL;
    }

    que = &tq->que;

    thrlib_mutxLock(&tq->pushMutx);

    /*-----*/

    while(1){
        first = que->first;

        if(first < que->last){
            remLen = (que->maxCnt - que->last) + first;
        }
        else{
            remLen = que->maxCnt - (first - que->last);
        }

        if(remLen == 0){
            if(wait != NULL){
                /* get current time */
                ret = comlib_timerGetTime(&delay);

                /* update time */
                delay.tv_sec += wait->tv_sec;
                delay.tv_nsec += wait->tv_nsec;
                if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
                    delay.tv_nsec -= COM_TIMER_TICK_SEC;
                    delay.tv_sec += 1;
                }
                delayFlg = RC_TRUE;
            }

            do{
                WAIT_CHECK((que), (tq->fWaiter), &(tq->fCond), &(tq->pushMutx), &(delay), (delayFlg), (ret));
                if(ret != RC_OK){
                    thrlib_mutxUnlock(&tq->pushMutx);
                    if(ret == THRERR_TQ_TIMEOUT) {
                        return ret;
                    }
                    else {
                        THR_LOG(THR_ERR,"wait check failed(ret=%d)\n",ret);
                        return ret;
                    }
                }

                if(first < que->last){
                    remLen = (que->maxCnt - que->last) + first;
                }
                else{
                    remLen = que->maxCnt - (first - que->last);
                }

            }while(remLen == 0);
        }/* end of if(remLen == 0) */

    }/* end of while(1) */


    }/* end of if((que->last + elmntCnt) >= que->maxCnt) */
    /*-----*/

    if(que->qCnt >= que->maxCnt){
        UINT delayFlg = RC_FALSE;

        if(wait != NULL){
            /* get current time */
            ret = comlib_timerGetTime(&delay);

            /* update time */
            delay.tv_sec += wait->tv_sec;
            delay.tv_nsec += wait->tv_nsec;
            if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
                delay.tv_nsec -= COM_TIMER_TICK_SEC;
                delay.tv_sec += 1;
            }
            delayFlg = RC_TRUE;
        }

        do{
            WAIT_CHECK((que), (tq->fWaiter), &(tq->fCond), &(tq->pushMutx), &(delay), (delayFlg), (ret));
            if(ret != RC_OK){
                thrlib_mutxUnlock(&tq->pushMutx);
                if(ret == THRERR_TQ_TIMEOUT) {
                    return ret;
                }
                else {
                    THR_LOG(THR_ERR,"wait check failed(ret=%d)\n",ret);
                    return ret;
                }
            }
        }while(que->qCnt < que->maxCnt);
    }/* end of if(que->qCnt >= que->maxCnt) */

    que->elmnt[que->last] = elmnt;

    if(que->last == (que->maxCnt-1)){
        que->last = 0;
    }
    else {
        que->last++;
    }

    que->qCnt++;

    /* send signal */
    if(tq->eWaiter){
        thrlib_condSig(&tq->eCond);
    }

    thrlib_mutxUnlock(&tq->pushMutx);

    return RC_OK;
}
#endif

FT_PUBLIC RT_RESULT thrlib_tqPop(ThrlibTq *tq, VOID **rt_elmnt)
{
    ThrlibQue *que = NULL;
    UINT first = 0;

    GEN_CHK_ERR_RET(tq == NULL,
                    THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
                    THRERR_TQ_NULL);

    que = &tq->que;

    /* lock */
    if(tq->lockType == THR_TQ_LOCK_TYPE_LOCKED){
        thrlib_mutxLock(&tq->popMutx);
    }

    if(que->first == que->last){
        if(tq->lockType == THR_TQ_LOCK_TYPE_LOCKED){
            thrlib_mutxUnlock(&tq->popMutx);
        }
        return THRERR_TQ_EMPTY;
    }

    first = que->first;

    (*rt_elmnt) = que->elmnt[first];

    que->elmnt[first] = NULL;

    if(first >= (que->maxCnt-1)){
        que->first = 0;
    }
    else {
        que->first++;
    }

    que->qCnt--;

    if(tq->lockType == THR_TQ_LOCK_TYPE_LOCKED){
        thrlib_mutxUnlock(&tq->popMutx);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_tqWaitPop(ThrlibTq *tq, VOID **rt_elmnt, struct timespec *wait)
{
    UINT ret = RC_OK;
    struct timespec delay;
    ThrlibQue *que = NULL;

    GEN_CHK_ERR_RET(tq == NULL,
                    THR_LOG(THR_ERR,"Thread queue structure is NULL\n"),
                    THRERR_TQ_NULL);

    if(tq->lockType == THR_TQ_LOCK_TYPE_LOCK_FREE){
        THR_LOG(THR_ERR,"lock type is Lock free\n");
        return THRERR_TQ_INVALID_LOCK_TYPE;
    }

    que = &tq->que;

    /* lock */
    thrlib_mutxLock(&tq->popMutx);

    if(que->qCnt == 0){
        UINT delayFlg = RC_FALSE;
        if(wait != NULL){
            /* get current time */
            ret = comlib_timerGetTime(&delay);

            /* update time */
            delay.tv_sec += wait->tv_sec;
            delay.tv_nsec += wait->tv_nsec;
            if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
                delay.tv_nsec -= COM_TIMER_TICK_SEC;
                delay.tv_sec += 1;
            }
            delayFlg = RC_TRUE;
        }

        do{
            WAIT_CHECK((que), (tq->eWaiter), &(tq->eCond), &(tq->popMutx), &(delay), (delayFlg), (ret));
            if(ret != RC_OK){
                thrlib_mutxUnlock(&tq->popMutx);
                if(ret == THRERR_TQ_TIMEOUT) {
                    return ret;
                }
                else {
                    THR_LOG(THR_ERR,"wait check failed(ret=%d)\n",ret);
                    return ret;
                }
            }
        }while (que->qCnt == 0); 
    }/* end of if(que->qCnt == 0) */

    (*rt_elmnt) = que->elmnt[que->first];

    if(que->first == (que->maxCnt - 1)){
        que->first = 0;
    }
    else {
        que->first++;
    }

    que->qCnt--;

    if(tq->fWaiter){
        thrlib_condSig(&tq->fCond);
    }

    thrlib_mutxUnlock(&tq->popMutx);

    return RC_OK;
}

