#include <arpa/inet.h>
#include <errno.h>

#include "gendef.h"
#include "trnlib.h"
#include "trnlib.x"

FT_PUBLIC RT_RESULT trnlib_utilPton(UINT afnum, CONST CHAR *src, VOID *dst)
{
    SINT ret = RC_OK;
    SINT af = 0;

    if(afnum == TRNLIB_AFNUM_IPV6){
        af = AF_INET6;
    }
    else if(afnum == TRNLIB_AFNUM_IPV4){
        af = AF_INET;
    }
    else{
        TRN_LOG(TRN_ERR,"Invalid afnum(%d)\n",afnum);
        return TRNERR_INVALID_AFNUM;
    }

    ret = inet_pton(af, src, dst);
    if(ret == 0){
        TRN_LOG(TRN_ERR,"source address emtpy\n");
        return TRNERR_SRC_STR_EMPTY;
    }
    else if(ret < 0){
        TRN_LOG(TRN_ERR,"Pton error(errno=%d:%s)\n", errno, strerror(errno));
        if(errno == EAFNOSUPPORT){
            return TRNERR_INVALID_AFNUM;
        }
        else{
            return RC_NOK;
        }
    }

    if(afnum == TRNLIB_AFNUM_IPV4){
        *(UINT*)dst = trnlib_utilNtohl(*(UINT*)dst);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT trnlib_utilNtop(UINT afnum, CONST VOID *src, CHAR *dst, UINT size)
{
    SINT af = 0;

    if(afnum == TRNLIB_AFNUM_IPV6){
        af = AF_INET6;
    }
    else if(afnum == TRNLIB_AFNUM_IPV4){
        af = AF_INET;
    }
    else{
        TRN_LOG(TRN_ERR,"Invalid afnum(%d)\n",afnum);
        return TRNERR_INVALID_AFNUM;
    }

    if(inet_ntop(af, src, dst, size /* max size */) == NULL){
        TRN_LOG(TRN_ERR,"Ntop error(errno=%d:%s)\n", errno, strerror(errno));
        if(errno == EAFNOSUPPORT){
            return TRNERR_INVALID_AFNUM;
        }
        else if(errno == ENOSPC){
            return TRNERR_OUT_OF_SPACE;
        }
        else {
            return RC_NOK;
        }
    }

    return RC_OK;
}

FT_PUBLIC U_32 trnlib_utilHtonl(U_32 host)
{
    return htonl(host);
}

FT_PUBLIC U_32 trnlib_utilNtohl(U_32 net)
{
    return ntohl(net);
}

FT_PUBLIC U_16 trnlib_utilHtons(U_16 host)
{
    return htons(host);
}

FT_PUBLIC U_16 trnlib_utilNtohs(U_16 net)
{
    return ntohs(net);
}

