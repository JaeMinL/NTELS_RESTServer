#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

/* fast mutex */
FT_PUBLIC RT_RESULT thrlib_mutxInit(ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    /* mutex attribute 
       1. fast : PTHREAD_MUTEX_INITIALIZER 
       2. recurisev : PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
       3. error checking : PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
     */
    ret = pthread_mutex_init(mutx, NULL);
    if(ret != 0){
        if(ret == EAGAIN){
            ret = THRERR_MUTX_AGAIN;
        }
        else if(ret == EINVAL){
            ret = THRERR_MUTX_INVAL;
        }
        else if(ret == ENOMEM){
            ret = THRERR_MUTX_NOMEM;
        }
        else {
            ret = THRERR_MUTX_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_mutxDstry(ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    ret = pthread_mutex_destroy(mutx);
    if(ret != 0){
        if(ret == EBUSY){
            ret = THRERR_MUTX_BUSY;
        }
        else if(ret == EINVAL){
            ret = THRERR_MUTX_INVAL;
        }
        else {
            ret = THRERR_MUTX_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_mutxLock(ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    ret = pthread_mutex_lock(mutx);
    if(ret != 0){
        if(ret == EDEADLK){
            ret = THRERR_MUTX_DEADLK;
        }
        else if(ret == EINVAL){
            ret = THRERR_MUTX_INVAL;
        }
        else {
            ret = THRERR_MUTX_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_mutxTrylock(ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    ret = pthread_mutex_trylock(mutx);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_MUTX_INVAL;
        }
        else if(ret == EBUSY){
            ret = THRERR_MUTX_BUSY;
        }
        else if(ret == EAGAIN){
            ret = THRERR_MUTX_AGAIN;
        }
        else if(ret == EDEADLK){
            ret = THRERR_MUTX_DEADLK;
        }
        else if(ret == EPERM){
            ret = THRERR_MUTX_PERM;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC RT_RESULT thrlib_mutxUnlock(ThrlibMutx *mutx)
{
    UINT ret = RC_OK;

    ret = pthread_mutex_unlock(mutx);
    if(ret != 0){
        if(ret == EINVAL){
            ret = THRERR_MUTX_INVAL;
        }
        else if(ret == EPERM){
            ret = THRERR_MUTX_PERM;
        }
        else {
            ret = THRERR_MUTX_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

