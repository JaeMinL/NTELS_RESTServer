#include <stdio.h>
#include <unistd.h>

#include "gendef.h"

#include "comlib.h"
#include "comlib.x"
#include "rcllib.h"
#include "rcllib.x"

int main()
{
    SINT ret = RC_OK;
    RcllibCb rcllibCb;
    RcllibHdrLst reqHdrLst;

    ret = rcllib_apiGlobInit();
    if(ret != RC_OK){
        fprintf(stderr,"REST Client global control block init failed(ret=%d)\n",ret);
        return -1;
    }

    ret = rcllib_apiInit(&rcllibCb);
    if(ret != RC_OK){
        fprintf(stderr,"REST Client control block init failed(ret=%d)\n",ret);
        return -1;
    }

    { /* setting http header */
        ret = rcllib_apiInitHdrLst(&reqHdrLst);
        if(ret != RC_OK){
            fprintf(stderr,"REST Header list init failed(ret=%d)\n",ret);
            return -1;
        }

        rcllib_apiAddHdrLst(&reqHdrLst, "Authorization","test");
    }

    ret = rcllib_apiPfrm(&rcllibCb, RCLLIB_MTHOD_GET, "http://example.com", &reqHdrLst, NULL);
    if(ret != RC_OK){
        fprintf(stderr,"Request failed(ret=%d)\n",ret);
        return -1;
    }

    ret = rcllib_apiDstryHdrLst(&reqHdrLst);
    if(ret != RC_OK){
        fprintf(stderr,"Header destory failed(ret=%d)\n",ret);
        return -1;
    }

    ret = rcllib_apiDstry(&rcllibCb);
    if(ret != RC_OK){
        fprintf(stderr,"REST Client control block destory failed(ret=%d)\n",ret);
        return -1;
    }

    return RC_OK;
}

