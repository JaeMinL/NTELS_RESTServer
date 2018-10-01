#include <stdio.h>
#include <unistd.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

SINT main(SINT argc, CHAR *argv[])
{
    UINT i = 0;
    SINT ret = 0;
    VOID *arg = NULL;
    WldlibCb wldlibCb;
    FILE *fp = NULL;
    CHAR regR[1024];

    if(argc != 2){
        fprintf(stderr,"Invalid arg\n");
        return 0;
    }

    //fp = fopen("./test.txt","r");
    fp = fopen("./rule.txt","r");
    if(fp == NULL){
        fprintf(stderr,"File open failed\n");
        return 0;
    }

    ret = wldlib_init(&wldlibCb);
    if(ret != RC_OK){
        fprintf(stderr,"Wildcard init failed(ret=%d)\n",ret);
        return 0;
    }

    i = 1;
    while(fgets(regR, 1024, fp) != NULL){
        UINT regRLen = 0;
        if(regR[0] == '#'){
            continue;
        }

        regRLen = comlib_strGetLen(regR);
        if(regR[regRLen-1] == '\n'){
            regRLen--;
            regR[regRLen] = '\0';
        }
        fprintf(stderr,"%02d(0x%02x). [%s]\n",i,i,regR);

        ret = wldlib_parseRegRule(&wldlibCb, regR, (VOID*)i);
        if(ret != RC_OK){
            fprintf(stderr,"rule parsing failed(ret=%d)\n",ret);
        }
        i++;
    }

    wldlib_mainPrnt(&wldlibCb);

    //ret = wldlib_parseStr(&wldlibCb, "qqqq5d", 6, &arg);
    ret = wldlib_parseStr(&wldlibCb, argv[1], &arg);
    if(ret != RC_OK){
        fprintf(stderr,"parsing faield(ret=%d)\n",ret);
        return 0;
    }
    fprintf(stderr,"DAT=0x%x\n",*(UINT*)&arg);

    return 0;
}

