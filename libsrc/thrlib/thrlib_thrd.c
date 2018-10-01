#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

FT_PUBLIC RT_RESULT thrlib_thrdCrte(ThrlibThrdId *thrdId, ThrlibThrdAttr *thrdAttr, ThrlibThrdFunc thrdFunc, VOID *arg)
{
    SINT ret = RC_OK;

    ret = pthread_create(thrdId, thrdAttr, (VOID *(*)(VOID *))thrdFunc, arg);
    if(ret != 0){
        if(ret == EAGAIN){
            ret = THRERR_THRD_AGAIN;
        }
        else if(ret == EINVAL){
            ret = THRERR_THRD_INVAL;
        }
        else{
            ret = THRERR_THRD_UNKNOWN;
        }
    }
    else {
        ret = RC_OK;
    }

    return ret;
}

FT_PUBLIC ThrlibThrdId thrlib_thrdSelf(VOID)
{
    return pthread_self();
}

FT_PUBLIC RT_RESULT thrlib_thrdDtch(ThrlibThrdId thrdId)
{
    SINT ret = RC_OK;

    ret = pthread_detach(thrdId);
    if(ret != 0){
        if(ret == EINVAL){
            return THRERR_THRD_INVAL;
        }
        else if(ret == ESRCH){
            return THRERR_THRD_ESRCH;
        }
        else{
            return RC_NOK;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdJoin(ThrlibThrdId thrdId, VOID **rt_retVal)
{
    SINT ret = RC_OK;

    ret = pthread_join(thrdId, rt_retVal);
    if(ret == EDEADLK){
        return THRERR_THRD_DLOCK;
    }
    else if(ret == EINVAL){
        return THRERR_THRD_INVAL;
    }
    else if(ret == ESRCH){
        return THRERR_THRD_ESRCH;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdCancel(ThrlibThrdId thrdId)
{
    SINT ret = RC_OK;

    ret = pthread_cancel(thrdId);
    if(ret != 0){
        if(ret == ESRCH){
            return THRERR_THRD_INVAL_ID;
        }
        else{
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

FT_PUBLIC VOID thrlib_thrdExit(VOID *ret_val)
{
    pthread_exit(ret_val);
}

FT_PUBLIC RT_RESULT thrlib_thrdSetCnclSta(SINT sta, SINT *oldSta)
{
    SINT ret = RC_OK; 

    ret = pthread_setcancelstate(sta, oldSta);
    if(ret != 0){
        if(ret == EINVAL){
            return THRERR_THRD_INVAL;
        }
        else {
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdSetCnclType(SINT type, SINT *oldType)
{
    SINT ret = RC_OK;

    ret = pthread_setcanceltype(type, oldType);
    if(ret != 0){
        if(ret == EINVAL){
            return THRERR_THRD_INVAL;
        }
        else {
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdAttrInit(ThrlibThrdAttr *thrdAttr)
{
    SINT ret = RC_OK;

    ret = pthread_attr_init(thrdAttr);
    if(ret != 0){
        if(ret == ENOMEM){
            return THRERR_THRD_NOMEM;
        }
        else{
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdAttrSetDtchSta(ThrlibThrdAttr *thrdAttr, SINT dtchSta)
{
    SINT ret = RC_OK;

    ret = pthread_attr_setdetachstate(thrdAttr, dtchSta);
    if(ret != 0){
        if(ret == EINVAL){
            return THRERR_THRD_INVAL;
        }
        else{
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT thrlib_thrdAttrGetDtchSta(ThrlibThrdAttr *thrdAttr, SINT *dtchSta)
{
    SINT ret = RC_OK;

    ret = pthread_attr_getdetachstate(thrdAttr, dtchSta);
    if(ret != 0){
        if(ret == EINVAL){
            return THRERR_THRD_INVAL;
        }
        else{
            return THRERR_THRD_UNKNOWN;
        }
    }

    return RC_OK;
}

