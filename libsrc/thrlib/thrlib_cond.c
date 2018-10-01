#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

FT_PUBLIC RT_RESULT thrlib_condInit(ThrlibCond *cond)
{
    UINT ret = RC_OK;

    ret = pthread_cond_init(cond, NULL);
    if(ret != 0){
        if(ret == EAGAIN){
            ret = THRERR_COND_AGAIN;
        }
        else if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else if(ret == ENOMEM){
            ret = THRERR_COND_NOMEM;
        }
        else{
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_condDstry(ThrlibCond *cond)
{
    UINT ret = RC_OK;

    ret = pthread_cond_destroy(cond);
    if(ret != 0){
        if(ret == EBUSY){
            ret = THRERR_COND_BUSY;
        }
        else if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else {
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_condWait(ThrlibCond *cond, ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    ret = pthread_cond_wait(cond, mutx);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else{
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_condTmWait(ThrlibCond *cond, ThrlibMutx *mutx, struct timespec *wait)
{
    SINT ret = RC_OK;

    ret = pthread_cond_timedwait(cond, mutx, wait);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else if(ret == ETIMEDOUT){
            ret = THRERR_COND_TMOUT;
        }
        else{
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_condSig(ThrlibCond *cond)
{
    UINT ret = RC_OK;

    ret = pthread_cond_signal(cond);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else {
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_condBrdcast(ThrlibCond *cond)
{
    UINT ret = RC_OK;

    ret = pthread_cond_broadcast(cond);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_COND_INVAL;
        }
        else {
            ret = THRERR_COND_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return RC_OK;
}
