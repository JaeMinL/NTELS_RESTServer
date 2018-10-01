#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "gendef.h"

#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"

void logFunc(struct tm *curTms, UINT lvl, CHAR *fName, UINT line, CHAR *logStr)
{
    fprintf(stderr,"LOG FUNC\n");
    fprintf(stderr,"[%d][%s:%d] %s\n",lvl, fName, line, logStr);
    return;
}

int main()
{
    SINT ret = RC_OK;
    LoglibCb loglibCb;
    LoglibCfg cfg;
    LoglibApndCfg apndCfg;

    LOGLIB_INIT_CFG(&cfg);

    //cfg.wrType = LOGLIB_WR_TYPE_THRD;

    ret = loglib_apiLoadCfg(&loglibCb, "./cfg_sample/log.xml", "TEST");
    if(ret != RC_OK){
        fprintf(stderr," loglib cfg init failed(ret=%d)\n",ret);
        return 0;
    }

    //loglib_apiDeregApnd(&loglibCb, "STDOUT");
#if 0
    loglib_apiSetApndLogLvl(&loglibCb, "STDOUT", LOGLIB_APND_DISP_ERR_LOG_BIT
                            | LOGLIB_APND_DISP_NOTY_LOG_BIT
                            | LOGLIB_APND_DISP_DBG_LOG_BIT
                           );
#endif

    LOGLIB_ERR(&loglibCb, "ERROR LOG\n");
    LOGLIB_NOTY(&loglibCb, "NOTIFY LOG\n");
    LOGLIB_DBG(&loglibCb, "DBG LOG\n");

    sleep(3);

    loglib_apiDstryLoglibCb(&loglibCb);
#if 0
    ret = loglib_apiInitLoglibCb(&loglibCb, "./","TEST", &cfg);
    if(ret != RC_OK){
        fprintf(stderr," loglib init failed(ret=%d)\n",ret);
        return 0;
    }

    ret = loglib_apiRegApnd(&loglibCb, "TEST_APND", LOGLIB_APND_TYPE_STDOUT, NULL);
    if(ret != RC_OK){
        fprintf(stderr,"Append failed(ret=%d)\n",ret);
        return 0;
    }

    LOGLIB_INIT_APND_CFG(&apndCfg);
    apndCfg.u.file.logPath = "./TEST";
    apndCfg.u.file.name = "TESTER";
    apndCfg.u.file.maxLogSize = 1000;
    apndCfg.lineFlg = RC_TRUE;
    apndCfg.fileFlg = RC_TRUE;

    ret = loglib_apiRegApnd(&loglibCb, "TEST_APND2", LOGLIB_APND_TYPE_FILE, &apndCfg);
    if(ret != RC_OK){
        fprintf(stderr,"Append failed(ret=%d)\n",ret);
        return 0;
    }

    ret = loglib_apiRegApnd(&loglibCb, "TEST_SYSLOG", LOGLIB_APND_TYPE_SYSLOG, NULL);
    if(ret != RC_OK){
        fprintf(stderr,"Append failed(ret=%d)\n",ret);
        return 0;
    }

    LOGLIB_INIT_APND_CFG(&apndCfg);
    apndCfg.lineFlg = RC_TRUE;
    apndCfg.fileFlg = RC_TRUE;

    apndCfg.u.usr.usrLogFunc = logFunc;

    ret = loglib_apiRegApnd(&loglibCb, "TEST_USER", LOGLIB_APND_TYPE_USR, &apndCfg);
    if(ret != RC_OK){
        fprintf(stderr,"Append failed(ret=%d)\n",ret);
        return 0;
    }

    loglib_apiSetLogLvl(&loglibCb, LOGLIB_LVL_DBG);

    loglib_apiLogWrite(&loglibCb, LOGLIB_LVL_ERR, __FILE__, __LINE__, "TEST\n");

    loglib_apiSetDispLvl(&loglibCb, "TEST_APND", RC_TRUE);

    LOGLIB_ERR(&loglibCb, "ERROR LOG\n");
    LOGLIB_NOTY(&loglibCb, "NOTIFY LOG\n");

    //    ret = loglib_apiDeregApnd(&loglibCb, "TEST_APND");

    LOGLIB_DBG(&loglibCb, "DBG LOG\n");

    sleep(5);

    loglib_apiDstryLoglibCb(&loglibCb);

    sleep(5);
#endif

    return 0;
}

