#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "smulib.h"
#include "smulib.x"

VOID sigHdlr(UINT sig, VOID *arg)
{
    //fprintf(stderr,"EVNT=%d\n",sig);
    return;
}

VOID tmOutFunc(VOID *arg)
{
    //fprintf(stderr,"THREAD TIMEOUT\n");
}

VOID logFunc(UINT lvl, CHAR *file, UINT line, CHAR *str)
{
    fprintf(stderr,"[%d][%s:%d] %s",lvl, file, line, str);
}

VOID thrdFunc(VOID *args)
{
    SmulibThrdCb thrdCb;

    //fprintf(stderr,"THRAD START\n");
    smulib_apiRegThrdBlk(&thrdCb, tmOutFunc, NULL, RC_TRUE);

    while(1){
        smulib_apiKeepaliveThrdBlk(&thrdCb);
     //   fprintf(stderr,"THRD LOOP\n");
        sleep(1);
    }

    smulib_apiDeregThrdBlk(&thrdCb);

    return ;
}

int main(SINT argc, CHAR *argv[])
{
    //UINT i = 0;
    ThrlibThrdId tid = 0;
    //SmuOptArg optArg;

    //optArg.rlyThrdCnt = 0;
    //
    fprintf(stderr,"TEST BLK START\n");

    //fprintf(stderr,"argc=%d\n",argc);
    //fprintf(stderr,"pthrdid=%lu\n",pthread_self());

    thrlib_thrdCrte(&tid, NULL, thrdFunc, NULL);

    //for(i=0;i<argc;i++){
    //    fprintf(stderr,"arg=[%s]\n",argv[i]);
    //}

    smulib_apiInit("TEST", NULL, NULL);
    smulib_apiSetLogFunc(SMULIB_NOTY, logFunc);

    //smulib_apiSigReg(SIGINT, sigHdlr, NULL);

    while(1){
        smulib_apiKeepalive();

        //sleep(1);
    }

    return 0;
}
