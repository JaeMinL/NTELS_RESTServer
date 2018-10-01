#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"

RlylibCb rlylibCb;

	volatile UINT rcvCnt = 0;
int allocBufCnt =0, freeBufCnt= 0, freeBufInsrtCnt=0 ,allocBufInsertCnt =0 ;
unsigned int sockSndCnt = 0, sockRcvCnt = 0;
#define MAX_LEN 300

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str);

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str)
{
fprintf(stderr,"[%d][%s][%d] %s", logLvl, file, line, str); 
}

FT_PUBLIC VOID rcvThrd(VOID *args)
{
	SINT ret = RC_OK;
	UINT rcvLen = 0;
	CHAR rcvBuf[MAX_LEN];
	CHAR rcvHst[125];
	int id;
	CHAR dstHost="TEST";

printf("RCV THRD START\n");
#if 1
	while(1){
		//ret = rlylib_apiRcvFixMsgFromHost(&rlylibCb, dstHost , rcvBuf, MAX_LEN, &rcvLen);
		ret = rlylib_apiRcvFixMsgFromAny(&rlylibCb, NULL, NULL, rcvBuf, MAX_LEN, &rcvLen);
		//ret = rlylib_apiRcvFixMsgFromRlm(&rlylibCb, "TEST", rcvBuf, MAX_LEN, &rcvLen, NULL, NULL);
		if(ret == RLYERR_MSG_NOT_EXIST){
			//usleep(1000);
		}
		else if(ret == RLYERR_DROP_MSG){
			//		dropCnt++;
		}
		else if(ret != RC_OK){
			fprintf(stderr,"Rcv failed(ret=%d)\n",ret);
		}
		else {
			rcvCnt++;
		}
	}
#endif
	return ;
}

FT_PUBLIC UINT hlrsim_toolsCalcRttMs(struct timespec *stTm, struct timespec *endTm)
{
    ULONG diffNSec = 0;
    ULONG diffSec = 0;

    diffSec =  endTm->tv_sec - stTm->tv_sec;

    if(endTm->tv_nsec < stTm->tv_nsec){
        diffNSec = (endTm->tv_nsec+COM_TIMER_TICK_SEC) - stTm->tv_nsec;
        diffSec--;
    }
    else {
        diffNSec = endTm->tv_nsec - stTm->tv_nsec;
    }

    return (diffSec * 1000/* 1sec */) + (diffNSec / 1000000);
}

int main(int argc, char *argv[])
{
	ThrlibThrdId tid;
	UINT i = 0;
	UINT tmpRcv=0;
	UINT  tmpSnd=0;
	CHAR key[128];
	RlylibRlmKey rlmKey;
	SINT opt = 0;
	UINT totRcvCnt = 0;
	UINT totSndCnt = 0;
	UINT sndCnt = 0;
	UINT dropCnt = 0;
	UINT sndTransId;
	CHAR sndHost[256];
	RlylibOptArg optArg;
struct timespec msgSndTm;
struct timespec curTm;
	//UINT sndMsgPerSec = 400000;
	//UINT sndMsgPerSec =550000;
	//	UINT sndMsgPerSec = 800000;
//	UINT sndMsgPerSec = 1000000;
//	UINT sndMsgPerSec = 400000;
	UINT sndMsgPerSec = 1;
	//UINT sndMsgPerSec = 100000;
	//UINT sndMsgPerSec = 200000;
	//UINT sndMsgPerSec = 10000;
	//UINT sndMsgPerSec = 2000;
	//UINT sndMsgPerSec = 1;
	UINT sndMsgPerSecCnt = 0;
	SINT ret = RC_OK;
	UINT spId = 0;
	TrnlibTransAddr dstAddr[2];
	//CHAR dstHost[] = "RECEIVER";
	CHAR dstHost[256];
	CHAR sndBuf[MAX_LEN];
	CHAR rcvBuf[MAX_LEN];
	UINT rcvLen = 0;
	ULONG diff = 0;
	UINT sndBufLen = 300;
	ComlibTimer tmr;
	ULONG prevTick10 = 0;
	ULONG prevTick1 = 0;
	CHAR locHost[RLYLIB_MAX_HOST_LEN];
	UINT dstPort = 0;

	locHost[0] ='\0';

	while(1){
		opt = getopt(argc, argv, "h:p:");

		if(opt == -1){
			break;
		}

		switch(opt){
			case 'p':
				{
					dstPort = atoi(optarg);
				}
				break;
			case 'h':
				{
					strcpy(locHost, optarg);
				}
				break;
			default :
				{
				}
				break;
		}/* end of switch(opt) */
	}/* end of while(1) */

	if(locHost[0] == '\0'){
		fprintf(stderr,"loc host error\n");
		exit(1);
	}

	if(dstPort == 0){
		fprintf(stderr,"port not exist\n");
		exit(1);
	}


	ret = comlib_timerInit(&tmr, COM_TIMER_TYPE_100M);
	if(ret != RC_OK){
		return 0;
	}

#if 0
	ret = comlib_timerTblInit(&tmrTbl, &tmr, hlrsim_timerEvnt);
	if(ret != RC_OK){
		HLRSIM_LOG(HLRSIM_ERR,"Timer table init failed(ret=%d)\n",ret);
		return RC_NOK;
	}
#endif
	//thrlib_thrdCrte(&tid, NULL, rcvThrd, NULL);

	memset(sndBuf,0x7,300);
	ret = rlylib_apiInitGlob();

	rlylib_apiSetLogFunc(RLY_DBG,logPrnt);
	RLYLIB_INIT_OPT_ARG(&optArg);

	optArg.thrdCnt = 3;
//optArg.condWaitFlg = RC_TRUE;
//optArg.waitTm = 50;

	printf("LOC HOST=%s\n",locHost);
	ret = rlylib_apiInitRlylibCb(&rlylibCb, RLYLIB_TYPE_BOTH, locHost, &optArg);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_init error(ret=%d)\n",ret);
	}

	/* add realm */
	ret = rlylib_apiAddRlm(&rlylibCb, "TEST", RLYLIB_RLM_RULE_RR, NULL);
	//ret = rlylib_apiAddRlm(&rlylibCb, "TEST", RLYLIB_RLM_RULE_HASH, NULL);
	if(ret != RC_OK){
		fprintf(stderr,"add realm error(ret=%d)\n",ret);
		exit(1);
	}

	/* addr list */
	for(i=0;i<1;i++){
		sprintf(dstHost,"%s%d","RECEIVER",i);
		//ret = rlylib_apiAddHost(&rlylibCb, dstHost, RLYLIB_TYPE_CLIENT, 0, NULL, &spId);
		ret = rlylib_apiAddHost(&rlylibCb, dstHost, RLYLIB_TYPE_CLIENT, 0, NULL, &spId);
		if(ret != RC_OK){
			fprintf(stderr,"rlylib_addHost error(ret=%d)\n",ret);
		}

		dstAddr[0].port = dstPort+i;
		dstAddr[0].proto = RLYLIB_TRANS_PROTO_TCP;
		dstAddr[0].netAddr.afnum = RLYLIB_AFNUM_IPV4;
		trnlib_utilPton(RLYLIB_AFNUM_IPV4, "127.0.0.1",(VOID*)&dstAddr[0].netAddr.u.ipv4NetAddr);

		ret = rlylib_apiAddConn(&rlylibCb, dstHost, dstAddr, 1);
		if(ret != RC_OK){
			fprintf(stderr,"rlylib_transAddConnCb error(ret=%d)\n",ret);
		}

		ret = rlylib_apiRegHostInRlm(&rlylibCb, "TEST", dstHost, RC_FALSE, NULL);
		if(ret != RC_OK){
			fprintf(stderr,"reg host failed(ret=%d)\n",ret);
			exit(1);
		}
	}

	EXEC_TMR_TICK((&tmr));
	prevTick1 = tmr.tick;
	prevTick10 = tmr.tick;

	while(1) {
		CHECK_TICK_DIFF(prevTick1, tmr.tick, diff);
		if(diff >= 1){ /* 0.1 sec */
			prevTick1  = tmr.tick;
		}

		CHECK_TICK_DIFF(prevTick10, tmr.tick, diff);
		if(diff >= 10){ /* 1 sec */
			tmpRcv = rcvCnt - totRcvCnt;
totRcvCnt = rcvCnt;
			tmpSnd = sndCnt;
		//	sndCnt = 0;
		//	rcvCnt = 0;
		//	totRcvCnt += tmpRcv;
		//	totSndCnt += tmpSnd;
			//totRcvCnt += rcvCnt;
			totSndCnt += sndCnt;
			fprintf(stderr,"===============================================================================================================================\n");
			fprintf(stderr,"sndCnt=%7d, totSndCnt=%7d, sockSndCnt=%7d, rcvCnt=%7d, totRcvCnt=%10d, diff=%10d, dropCnt=%10d\n",
					tmpSnd, totSndCnt, sockSndCnt, tmpRcv, totRcvCnt, totSndCnt-totRcvCnt, dropCnt); 
			fprintf(stderr,"-------------------------------------------------------------------------------------------------------------------------------\n");
			fprintf(stderr,"new   =%7d            %7d  pnd       =%7d         %7d  tot      =%10d       %10d\n",
					allocBufCnt, freeBufCnt,
					freeBufInsrtCnt ,allocBufInsertCnt,
					allocBufCnt + allocBufInsertCnt,
					freeBufCnt + freeBufInsrtCnt);
			fprintf(stderr,"===============================================================================================================================\n");

			dropCnt = 0;
			sndCnt = 0;
			//rcvCnt = 0;
			sockSndCnt = 0;
			allocBufCnt = 0;
			freeBufCnt = 0;
			freeBufInsrtCnt= 0;
			allocBufInsertCnt = 0;
			prevTick10  = tmr.tick;
			if(sndMsgPerSecCnt < sndMsgPerSec){
				while(1){
					//ret = rlylib_apiSndFixMsgToHost(&rlylibCb, dstHost, sndBuf, sndBufLen);
					ret = rlylib_apiSndFixMsgToAny(&rlylibCb, sndBuf, sndBufLen, sndHost, &sndTransId);
					//sprintf(key,"TEST%d",i);
					//i++;
					//if(i == 10){
					//	i = 0;
					//}
					//rlmKey.key = key;
					//rlmKey.keyLen = 5;
					//ret = rlylib_apiSndFixMsgToRlm(&rlylibCb, "TEST", &rlmKey, sndBuf, sndBufLen, sndHost, &sndTransId);
					if(ret != RC_OK){
						fprintf(stderr,"Message send failed(ret=%d)\n",ret);
					}

					sndCnt++;
					sndMsgPerSecCnt++;

					if(sndMsgPerSecCnt == sndMsgPerSec){
						break;
					}
				}
			}
			sndMsgPerSecCnt = 0;

#if 0
			if(totRcvCnt >= (UINT)(sndMsgPerSec * 5)){
				fprintf(stderr,"del host\n");
				rlylib_apiDelHost(&rlylibCb, "RECEIVER");
				sleep(1);
				ret = rlylib_apiAddHost(&rlylibCb, "RECEIVER", RLYLIB_TYPE_CLIENT, 0, NULL, &spId);
				ret = rlylib_apiAddConn(&rlylibCb, "RECEIVER", dstAddr, 1);

				totRcvCnt = 0;
			}
#endif
		}

	//	for(i=0;i<7;i++){
			if(sndMsgPerSecCnt < sndMsgPerSec){
				//ret = rlylib_apiSndFixMsgToHost(&rlylibCb, dstHost, sndBuf, sndBufLen);
				  comlib_timerGetTime(&msgSndTm);
				ret = rlylib_apiSndFixMsgToAny(&rlylibCb, sndBuf, sndBufLen, sndHost, &sndTransId);
				//sprintf(key,"TEST%d",i);
				//i++;
				//if(i == 10){
				//	i = 0;
				//}
				//rlmKey.key = key;
				//rlmKey.keyLen = 5;
				//ret = rlylib_apiSndFixMsgToRlm(&rlylibCb, "TEST", &rlmKey, sndBuf, sndBufLen, sndHost, &sndTransId);
				if(ret != RC_OK){
					fprintf(stderr,"Message send failed(ret=%d)\n",ret);
				}

				sndCnt++;
				sndMsgPerSecCnt++;
			}
		//	else {
		//		break;
		//	}
		//}

#if 1
			while(1){
				//ret = rlylib_apiRcvFixMsgFromHost(&rlylibCb, dstHost , rcvBuf, MAX_LEN, &rcvLen);
				ret = rlylib_apiRcvFixMsgFromAny(&rlylibCb, NULL, NULL, rcvBuf, MAX_LEN, &rcvLen);
				//ret = rlylib_apiRcvFixMsgFromRlm(&rlylibCb, "TEST", rcvBuf, MAX_LEN, &rcvLen, NULL, NULL);
				if(ret == RLYERR_MSG_NOT_EXIST){
					EXEC_TMR_TICK((&tmr));
					if(sndMsgPerSecCnt == sndMsgPerSec){
							usleep(1000);
					}
					break;
				}
				else if(ret == RLYERR_DROP_MSG){
					dropCnt++;
				}
				else if(ret != RC_OK){
					fprintf(stderr,"Rcv failed(ret=%d)\n",ret);
					EXEC_TMR_TICK((&tmr));
					break;
				}
				else {
					comlib_timerGetTime(&curTm);
					rcvCnt++;
					ret = hlrsim_toolsCalcRttMs(&msgSndTm, &curTm);
					fprintf(stderr,"RSP TIME=%d, sndTM=(%d,%d) curTm=(%d,%d)\n",ret, msgSndTm.tv_sec, msgSndTm.tv_nsec, curTm.tv_sec, curTm.tv_nsec);
break;
				}
			}
		//}
#endif

		EXEC_TMR_TICK((&tmr));
	}/* end of while(1) */

	return 0;
}
