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
    UINT rspDatLen = 0;
    CHAR *rspDat = NULL;

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

    ret = rcllib_apiPfrm(&rcllibCb, RCLLIB_MTHOD_GET, "http://example.com", NULL, NULL);
    if(ret != RC_OK){
        fprintf(stderr,"Request failed(ret=%d)\n",ret);
        return -1;
    }

    ret = rcllib_apiCpyRspDatIntoDyn(&rcllibCb, &rspDat, &rspDatLen);
    if(ret != RC_OK){
        fprintf(stderr,"Response data not exist(ret=%d)\n",ret);
        return -1;
    }

    printf("== response data ==\n");
    printf("%.*s\n",rspDatLen, rspDat);

    comlib_memFree(rspDat);

    ret = rcllib_apiDstry(&rcllibCb);
    if(ret != RC_OK){
        fprintf(stderr,"REST Client control block destory failed(ret=%d)\n",ret);
        return -1;
    }

    return RC_OK;
}
