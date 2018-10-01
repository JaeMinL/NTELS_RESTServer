#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"

#define MAX_LEN 300
unsigned int sockSndCnt = 0, sockRcvCnt = 0;
int allocBufCnt = 0, freeBufCnt = 0,freeBufInsrtCnt=0 ,allocBufInsertCnt = 0;

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str);

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str)
{
fprintf(stderr,"[%d][%s][%d] %s", logLvl, file, line, str); 
}



FT_PUBLIC void quitSignal (int signo)
{
    fprintf(stderr ,  "########### PROCESS SHUTDOWN ########## signo=%d\n", signo);
    exit(1);

} /** END OF quitSignal **/

int main(int argc, char *argv[])
{
	SINT ret = RC_OK;
	CHAR rcvBuf[MAX_LEN];
	CHAR *pRcvBuf;
	RlylibOptArg optArg;
	UINT transId = 0;
	UINT loopCnt = 0;
	UINT rcvLen = 0;
	UINT rcvCnt = 0;
	UINT totRcvCnt = 0;
	RlylibCb rlylibCb;
	TrnlibTransAddr srcAddr;
	CHAR host[RLYLIB_MAX_HOST_LEN];
	ComlibTimer tmr;
	//ComlibTmrTbl tmrTbl;
	ULONG diff = 0;
	ULONG prevTick10 = 0;
	ULONG prevTick1 = 0;
	CHAR rcvHost[RLYLIB_MAX_HOST_LEN];
	UINT locPort = 0;
	SINT opt = 0;

	signal(SIGINT,  quitSignal);
	signal(SIGTERM, quitSignal);

	while(1){
		opt = getopt(argc, argv, "h:p:");

		if(opt == -1){
			break;
		}

		switch(opt){
			case 'p':
				{
					locPort = atoi(optarg);
				}
				break;
			case 'h':
				{
					strcpy(host, optarg);
				}
				break;
			default :
				{
				}
				break;
		}/* end of switch(opt) */
	}/* end of while(1) */

	if(locPort == 0){
		fprintf(stderr,"local port not exist\n");
		exit(1);
	}

	ret = comlib_timerInit(&tmr, COM_TIMER_TYPE_100M);
	if(ret != RC_OK){
		return 0;
	}

	rlylib_apiInitGlob();

	rlylib_apiSetLogFunc(RLY_NOTY,logPrnt);

	RLYLIB_INIT_OPT_ARG(&optArg);

	optArg.thrdCnt = 3;
	optArg.condWaitFlg = RC_TRUE;

	//ret = rlylib_apiInitRlylibCb(&rlylibCb, RLYLIB_TYPE_BOTH, host, NULL);
	ret = rlylib_apiInitRlylibCb(&rlylibCb, RLYLIB_TYPE_BOTH, host, &optArg);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_init error(ret=%d)\n",ret);
	}

	srcAddr.port = locPort;
	srcAddr.proto = RLYLIB_TRANS_PROTO_TCP;
	srcAddr.netAddr.afnum = RLYLIB_AFNUM_IPV4;
	//srcAddr.netAddr.afnum = RLYLIB_AFNUM_IPV6;
	//inet_pton(AF_INET6, "::1", srcAddr.netAddr.u.ipv6NetAddr);
	//rlylib_apiPton(RLYLIB_AFNUM_IPV6, "::", srcAddr.netAddr.u.ipv6NetAddr);
	trnlib_utilPton(TRNLIB_AFNUM_IPV4, "0.0.0.0", &srcAddr.netAddr.u.ipv4NetAddr);

	ret = rlylib_apiAddAcptLst(&rlylibCb, &srcAddr);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_add accept list error(ret=%d)\n",ret);
	}

	EXEC_TMR_TICK((&tmr));
	prevTick1 = tmr.tick;
	prevTick10 = tmr.tick;

	while(1) {
		CHECK_TICK_DIFF(prevTick1, tmr.tick, diff);
		if(diff >= 1){ /* 0.1 sec */
			prevTick1  = tmr.tick;
			if(rcvCnt == 0){
				usleep(1000);
			}
		}

		CHECK_TICK_DIFF(prevTick10, tmr.tick, diff);
		if(diff >= 10){ /* 1 sec */
			prevTick10  = tmr.tick;

			totRcvCnt += rcvCnt;
			fprintf(stderr,"rcvCnt=%d sockRcvCnt=%d, totRcvCnt=%d\n", rcvCnt, sockRcvCnt, totRcvCnt);
			rcvCnt = 0;
		}

		while(1){
			//ret = rlylib_transRcvFixMsgFromHost(&rlylibCb, "SENDER", rcvBuf, MAX_LEN, &rcvLen);
			//ret = rlylib_apiRcvFixMsgFromAny(&rlylibCb, rcvHost, &transId, rcvBuf, MAX_LEN, &rcvLen);
			ret = rlylib_apiRcvDynMsgFromAny(&rlylibCb, rcvHost, &transId, &pRcvBuf, &rcvLen);
			if(ret == RLYERR_MSG_NOT_EXIST){
				//usleep(500);
				EXEC_TMR_TICK((&tmr));
				break;
			}
			else if(ret != RC_OK){
				sleep(1);
				fprintf(stderr,"Rcv failed(ret=%d)\n",ret);
				EXEC_TMR_TICK((&tmr));
				break;
			}
			else {
//memset(rcvBuf, 0x01,rcvLen);
				//ret = rlylib_transSndFixMsgToHost(&rlylibCb, "SENDER", rcvBuf, rcvLen);
				//ret = rlylib_apiSndFixMsgToHost(&rlylibCb, rcvHost, rcvBuf, rcvLen);
				//ret = rlylib_apiSndFixMsgToHostId(&rlylibCb, transId, rcvBuf, rcvLen);
				ret = rlylib_apiSndDynMsgToHostId(&rlylibCb, transId, pRcvBuf, rcvLen);
				if(ret != RC_OK){
					//fprintf(stderr,"Message send failed(ret=%d)\n",ret);
				}

				rcvCnt++;
			}

			loopCnt++;

			if(loopCnt == 2000){
				break;
			}
		}
		usleep(1500);


		EXEC_TMR_TICK((&tmr));
	}


	return 0;
}
