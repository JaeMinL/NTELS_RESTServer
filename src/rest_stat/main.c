#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "rrllib.h"
#include "rrllib.x"
#include "rsvlib.h"
#include "rsvlib.x"
#include "rest_stat.h"

#define HTTP_BAD_REQUEST 404
#define UNAUTHORIZED 401

AuthMngCb authMngCb;

FT_PUBLIC RT_RESULT findUrl(RsvlibSesCb *sesCb, CHAR **who, CHAR **when);
FT_PUBLIC RT_RESULT func(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr);

int main(int argc, char **argv)
{
	SINT ret = RC_OK;
	SINT opt = 0;

#if 0
    CHAR cfgPath[MAIN_CFG_PATH_LEN];
#endif
	CHAR logCfgPath[MAIN_CFG_PATH_LEN];
	LoglibCfg logCfg;
	RsvlibGenCfg rsvCfg;

#if 0
	cfgPath[0] = '\0';
#endif
	logCfgPath[0] = '\0';

	while(1){
		opt = getopt(argc, argv,"c:l:h:");
		if(opt == -1)
		{
			break;
		}

		switch(opt){
#if 0
			case 'c':
				{
					comlib_strSNPrnt(cfgPath, MAIN_CFG_PATH_LEN, "%s", optarg);
				}
				break;
#endif
			case 'l':
				{
					comlib_strSNPrnt(logCfgPath, MAIN_CFG_PATH_LEN, "%s", optarg);
				}
				break;
			case 'h':
				{
					fprintf(stderr,"usage : rest_stat -c [config path] -l [log config path]\n");
					exit(1);
				}
				break;
			default:
				{
					fprintf(stderr,"option \"%c\" is unknown\n",opt);
					fprintf(stderr,"usage : rest_stat -c [config path] -l [log config path]\n");
					exit(1);
				}
				break;
		}/* end of switch(opt) */
	}/* end of while */

#if 0
	if(cfgPath[0] == '\0'){
		fprintf(stderr,"CONFIG NOT EXIST\n");
		exit(1);
	}
#endif

	if(logCfgPath[0] == '\0')
	{
		fprintf(stderr,"LOG CONFIG NOT EXIST\n");
		exit(1);
	}

    /* log setting */
	LOGLIB_GLOB_INIT();

	LOGLIB_INIT_CFG(&logCfg);

	ret = loglib_apiLoadToml(logCfgPath, "REST_IF");
	if(ret != RC_OK){
		fprintf(stderr,"Log config load failed(ret=%d)\n",ret);
	}


	LOGLIB_NOTY("REST","REST INTERFACE START\n");

	/* rest server setting */
	RSV_INIT_GEN_CFG(&rsvCfg, 8800);

	ret = rsvlib_apiInit(1, &rsvCfg);
	if(ret != RC_OK){
		LOGLIB_ERR("REST","REST SERVER INIT FAILED(ret=%d)\n", ret);
		return RC_NOK;
	}

	rsvlib_apiSetLogFunc(RSV_DBG, logPrnt);

	ret = initAuthMngCb(&authMngCb);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","init failed(ret=%d)\n", ret);
		return RC_NOK;
	}

	/* url rule setting */
	ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/{who}/{when}", "[name] [ip] [start] [end]", NULL, func);

	/* run rest server */
	rsvlib_apiRun(1);

	while(1){
	    sleep(1);
	}

	//exit(1);
	//rsvlib_apiDstry(1);
	//loglib_apiDstryLoglibCb(&loglibCb);	

	return 0;
}


FT_PUBLIC RT_RESULT func(UINT mthod, RsvlibSesCb *sesCb)
{
	CHAR *who = NULL;
	CHAR *when = NULL;
	SINT ret = RC_NOK;
	
	ret = userAuth(sesCb, &authMngCb);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","findUrl() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}
	printf("userAuth success!!!!!!!!!!!!!!\n");
	ret = findUrl(sesCb, &who, &when);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","findUrl() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	ret = MakeQuery(sesCb, who, when);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","funcInfo() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT findUrl(RsvlibSesCb *sesCb, CHAR **who, CHAR **when)
{/*find url who(svc/host) and when(1min/5min/hour/day)*/
	SINT ret = RC_NOK;
	RrllibDocArg *docArg = NULL;
	CHAR *argWho = "who";
	CHAR *argWhen = "when";
	CONST CHAR *strWho = NULL;
	CONST CHAR *strWhen = NULL;
	UINT strWhoLen = 0;
	UINT strWhenLen = 0;

	ret = rsvlib_apiFindArg(sesCb, argWho, &docArg);
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "Find svr or host argument failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}
	ret = rsvlib_apiFirstArgVal(docArg, &strWho, &strWhoLen);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Find strWho value failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	ret = rsvlib_apiFindArg(sesCb, argWhen, &docArg);
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "Find static term argument failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}
	ret = rsvlib_apiFirstArgVal(docArg, &strWhen, &strWhenLen);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Find strWhen value failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	(*who) = (CHAR*)strWho;
	(*when) = (CHAR*)strWhen;

	comlib_strChgStrToUpper(strWho, strWhoLen, (*who), strWhoLen);
	if(!comlib_strNCmp(strWho, "SVC", strWhoLen))
	{
		(*who) = "SERVICE";
	}

	comlib_strChgStrToUpper(strWhen, strWhenLen, (*when), strWhenLen);
	if(!comlib_strNCmp(strWhen, "1MIN", strWhenLen))
	{
		(*when) = NULL;
	}
	return RC_OK;
}


FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr)
{
	printf("[%d][%s:%d] %s\n",lvl, file, line, logStr);
}

