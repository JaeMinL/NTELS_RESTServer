#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"
#include "rbrlib.h"
#include "rbrlib.x"

UINT test_func(UINT mthod, RbrlibSesCb *sesCb);
UINT test_gen(VOID *usrArg, U_64 pos, CHAR *buf, SIZET max);
UINT test_free(VOID *usrArg);

VOID test_init()
{
    SINT ret = 0;
    RbrlibGenCfg genCfg;

    RBR_INIT_GEN_CFG(&genCfg, 8080, 5);

    rbrlib_apiInit(1, &genCfg);

    ret = rbrlib_apiSetRule(1, RBR_MTHOD_GET, "/v1/test", "{test} [user]", NULL, test_func);
    ret = rbrlib_apiSetRuleStrm(1, RBR_MTHOD_GET, "/v1/test/stream", "{test}", NULL, test_gen, test_free, 0);

    if(ret != RC_OK){
        fprintf(stderr,"ERR\n");
    }
    rbrlib_apiRun(1);
}

UINT test_free(VOID *usrArg)
{
    return 0;
}

UINT test_gen(VOID *usrArg, U_64 pos, CHAR *buf, SIZET max)
{
    fprintf(stderr,"pos=%d, max=%d\n",pos, max);
    if (max < 80)
        return 0;
    memset (buf, 'A', max - 1);
    buf[79] = '\n';
    fprintf(stderr,"DATA GEN\n");
    sleep(1);
    return 80;
}

UINT test_func(UINT mthod, RbrlibSesCb *sesCb)
{
    SINT ret = RC_OK;
    CHAR *rspDat = NULL;
    CHAR *key = NULL;
    UINT keyLen = 0;
    CHAR *val = NULL;
    UINT valLen = 0;
    fprintf(stderr,"USER FUNCTION\n");
#if 0
    if(rcvDat != NULL){
        fprintf(stderr,"DAT=[%.*s]\n",rcvDatLen, rcvDat);
    }
#endif

    while(1){
        rbrlib_apiGetNxtHdr(sesCb, &key, &keyLen, &val, &valLen);
        if(keyLen == 0){
            break;
        }
        fprintf(stderr,"KEY=%.*s, VAL=%.*s)\n",keyLen, key, valLen, val);
    }

    RrllibDoc *doc = NULL;
    RrllibDocArg *docArg = NULL;

    doc = sesCb->req.doc;

    CONST CHAR *dat = NULL;
    UINT datLen = NULL;

    //ret = rrllib_docFindArg(doc, "test", 4, &docArg);
    //ret = rrllib_docGetFirstArg(doc, &docArg);
    while(1){ 
        ret = rrllib_docGetNxtArg(doc, &docArg);
        if(ret == RC_OK){
            while(1){
                ret = rrllib_docGetNxtVal(docArg, &dat, &datLen);
                if(ret != RC_OK){
                    break;
                }

                fprintf(stderr,"dat=%.*s\n",datLen, dat);
            }
        }
        else {
            break;
        }
    }

    rbrlib_apiSetRspHdr(sesCb, "TEST", "VALUE", RC_TRUE);
    rspDat = "{ \"RESULT\":\"SUCCCESS\"}";

    rbrlib_apiSetRspDat(sesCb, rspDat, RC_TRUE);

    return 0;
}

int main()
{
    test_init();

    while(1){
        sleep(10);
        rbrlib_apiStop(1);
    }

    return 0;
}

