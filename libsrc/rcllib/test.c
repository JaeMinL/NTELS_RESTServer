#include <stdio.h>

#include <curl/curl.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rcllib.h"
#include "rcllib.x"

int main()
{
    RcllibCb rcllibCb;
    UINT staCode = 0;
    CHAR *dat = NULL;
    CHAR fixDat[1024];
    UINT datLen = 0;

    rcllib_apiGlobInit();

    rcllib_apiInit(&rcllibCb);

    rcllib_apiPfrm(&rcllibCb, RCLLIB_MTHOD_GET,
                   "http://192.168.62.17:8080/v1/statistics/resource/cpu?start-time=1508811860&end-time=1513004400&node-id=ATOM",
                   NULL, NULL);

    //rcllib_apiCpyRspDatIntoDyn(&rcllibCb, &dat, &datLen);
    fprintf(stderr, "%.*s\n",datLen, dat);
    rcllib_apiCpyRspDatIntoFix(&rcllibCb, fixDat, 1024, &datLen);
   
    fprintf(stderr, "%.*s\n",datLen, fixDat);

    rcllib_apiGetStaCode(&rcllibCb, &staCode);

    fprintf(stderr,"%d\n",staCode);

    rcllib_apiDstry(&rcllibCb);

    return 0;
}
