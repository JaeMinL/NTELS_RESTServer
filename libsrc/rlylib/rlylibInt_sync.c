#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"
#include "rlylibInt.h"
#include "rlylibInt.x"

FT_PUBLIC RT_RESULT rlylibInt_syncMutxInit(RlylibIntMainMutx *mainMutx)
{
    SINT ret = RC_OK;

    ret = thrlib_mutxInit(&mainMutx->rdMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Read mutex init failed(ret=%d)\n",ret);
        return RLYERR_MUTX_INIT_FAILED;
    }

    ret = thrlib_mutxInit(&mainMutx->wrMutx);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Write mutex init failed(ret=%d)\n",ret);
        return RLYERR_MUTX_INIT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC ThrlibMutx* rlylibint_sysnGetMutxPtr(RlylibIntMainMutx *mainMutx, UINT type)
{
    switch(type){
        case RLYLIB_MUTX_LOCK_TYPE_RD:
            {
                return &mainMutx->rdMutx;
            }
            break;
        case RLYLIB_MUTX_LOCK_TYPE_WR:
            {
                return &mainMutx->wrMutx;
            }
            break;
        default:
            {
                RLY_LOG(RLY_ERR,"Invalid lock type(type=%d)\n",type);
                return NULL;
            }
            break;
    };
}

FT_PUBLIC RT_RESULT rlylibInt_syncMutxLock(RlylibIntMainMutx *mainMutx, UINT type)
{
    switch(type){
        case RLYLIB_MUTX_LOCK_TYPE_RD:
            {
                thrlib_mutxLock(&mainMutx->rdMutx);
            }
            break;
        case RLYLIB_MUTX_LOCK_TYPE_WR:
            {
                thrlib_mutxLock(&mainMutx->wrMutx);
            }
            break;
        case RLYLIB_MUTX_LOCK_TYPE_ALL:
            {
                thrlib_mutxLock(&mainMutx->rdMutx);
                thrlib_mutxLock(&mainMutx->wrMutx);
            }
            break;
        default:
            {
                RLY_LOG(RLY_ERR,"Invalid lock type(type=%d)\n",type);
                return RLYERR_INVALID_LOCK_TYPE;
            }
            break;
    };

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_syncMutxUnlock(RlylibIntMainMutx *mainMutx, UINT type)
{
    switch(type){
        case RLYLIB_MUTX_LOCK_TYPE_RD:
            {
                thrlib_mutxUnlock(&mainMutx->rdMutx);
            }
            break;
        case RLYLIB_MUTX_LOCK_TYPE_WR:
            {
                thrlib_mutxUnlock(&mainMutx->wrMutx);
            }
            break;
        case RLYLIB_MUTX_LOCK_TYPE_ALL:
            {
                thrlib_mutxUnlock(&mainMutx->wrMutx);
                thrlib_mutxUnlock(&mainMutx->rdMutx);
            }
            break;
        default:
            {
                RLY_LOG(RLY_ERR,"Invalid lock type(type=%d)\n",type);
                return RLYERR_INVALID_LOCK_TYPE;
            }
            break;
    };

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_syncWaitSigCond(ThrlibCond *cond, ThrlibMutx *mutx, ULONG waitTm)
{
    SINT ret = RC_OK;
    struct timespec delay;

    if(waitTm != 0){
        comlib_timerGetTime(&delay);

        /* 1000ms = 1sec */
        if(waitTm >= 1000){ /* 1sec */
            delay.tv_sec += (waitTm / 1000);
            delay.tv_nsec = (waitTm % 1000) * COM_TIMER_TICK_1MS;
        }
        else {
            delay.tv_nsec += waitTm * COM_TIMER_TICK_1MS;
        }

        if(delay.tv_nsec >= COM_TIMER_TICK_SEC){
            delay.tv_sec+=1;
            delay.tv_nsec -= COM_TIMER_TICK_SEC;
        }

        ret = thrlib_condTmWait(cond, mutx, &delay);
        if(ret != RC_OK){
            if(ret == THRERR_COND_TMOUT){
                return RLYERR_COND_TM_OUT;
            }
            else {
                RLY_LOG(RLY_ERR,"Cond time wait failed(ret=%d)\n",ret);
                return RLYERR_COND_WAIT_FAILED;
            }
        }
    }
    else {
        ret = thrlib_condWait(cond, mutx);
        if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Cond wait failed(ret=%d)\n",ret);
            return RLYERR_COND_WAIT_FAILED;
        }
    }

    return RC_OK;
}
