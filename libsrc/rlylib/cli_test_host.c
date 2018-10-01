#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "gendef.h"
#include "cmlib.h"
#include "cmlib.x"
#include "thlib.h"
#include "thlib.x"
#include "rlylib.h"
#include "rlylib.x"

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str);

FT_PUBLIC VOID logPrnt(UINT logLvl, CHAR *file, UINT line, CHAR *str)
{
fprintf(stderr,"[%d][%s][%d] %s", logLvl, file, line, str); 
}

#define MAX_LEN 300
int main(int argc, char *argv[])
{
	SINT opt = 0;
	UINT rcvCnt = 0;
	UINT totRcvCnt = 0;
	UINT sndCnt = 0;
	UINT dropCnt = 0;
	UINT sndTransId;
	CHAR sndHost[256];
	//UINT sndMsgPerSec = 140000;
	UINT sndMsgPerSec = 100000;
	//UINT sndMsgPerSec = 1000;
	//	UINT sndMsgPerSec = 140000;
	//UINT sndMsgPerSec = 100000;
	//UINT sndMsgPerSec = 80000;
	//UINT sndMsgPerSec = 90000;
	//UINT sndMsgPerSec = 50000;
	//UINT sndMsgPerSec = 2000;
	UINT sndMsgPerSecCnt = 0;
	SINT ret = RC_OK;
	UINT spId = 0;
	RlylibCb rlylibCb;
	//RlylibTransAddr srcAddr;
	RlylibTransAddr dstAddr[2];
	CHAR dstHost[] = "RECEIVER";
	CHAR sndBuf[MAX_LEN];
	CHAR rcvBuf[MAX_LEN];
	UINT rcvLen = 0;
	ULONG diff = 0;
	UINT sndBufLen = 300;
	CmlibTimer tmr;
	//CmlibTmrTbl tmrTbl;
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

	ret = cmlib_timerInit(&tmr, CM_TIMER_TYPE_100M);
	if(ret != RC_OK){
		return 0;
	}

#if 0
	ret = cmlib_timerTblInit(&tmrTbl, &tmr, hlrsim_timerEvnt);
	if(ret != RC_OK){
		HLRSIM_LOG(HLRSIM_ERR,"Timer table init failed(ret=%d)\n",ret);
		return RC_NOK;
	}
#endif

	memset(sndBuf,0x7,300);
	ret = rlylib_apiInitGlob();

	rlylib_apiSetLogFunc(RLY_NOTY,logPrnt);

	ret = rlylib_apiInitRlylibCb(&rlylibCb, RLYLIB_TYPE_BOTH, locHost, NULL);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_init error(ret=%d)\n",ret);
	}

#if 0
	srcAddr.port = 12125;
	srcAddr.proto = RLYLIB_TRANS_PROTO_TCP;
	srcAddr.netAddr.afnum = RLYLIB_AFNUM_IPV4;
	srcAddr.netAddr.u.ipv4NetAddr = 0;

	ret = rlylib_acptAddAcptLst(&rlylibCb, &srcAddr);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_add accept list error(ret=%d)\n",ret);
	}
#endif
	ret = rlylib_apiAddHost(&rlylibCb, "RECEIVER", RLYLIB_TYPE_CLIENT, 0, NULL, &spId);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_addHost error(ret=%d)\n",ret);
	}


	dstAddr[0].port = dstPort;
	dstAddr[0].proto = RLYLIB_TRANS_PROTO_TCP;
	//dstAddr[0].netAddr.afnum = RLYLIB_AFNUM_IPV4;
	//dstAddr[0].netAddr.u.ipv4NetAddr = ntohl(inet_addr("127.0.0.1"));
	//dstAddr[0].netAddr.afnum = RLYLIB_AFNUM_IPV6;
	dstAddr[0].netAddr.afnum = RLYLIB_AFNUM_IPV4;
	//inet_pton(AF_INET6, "::1",dstAddr[0].netAddr.u.ipv6NetAddr);
	//rlylib_apiPton(RLYLIB_AFNUM_IPV6, "::1",dstAddr[0].netAddr.u.ipv6NetAddr);
	rlylib_apiPton(RLYLIB_AFNUM_IPV4, "127.0.0.1",dstAddr[0].netAddr.u.ipv6NetAddr);
#if 0
	dstAddr[1].port = dstPort;
	dstAddr[1].proto = RLYLIB_TRANS_PROTO_TCP;
	dstAddr[1].netAddr.afnum = RLYLIB_AFNUM_IPV4;
	dstAddr[1].netAddr.u.ipv4NetAddr = ntohl(inet_addr("127.0.0.1"));
#endif

	ret = rlylib_apiAddConn(&rlylibCb, "RECEIVER", dstAddr, 1);
	if(ret != RC_OK){
		fprintf(stderr,"rlylib_transAddConnCb error(ret=%d)\n",ret);
	}

	//sleep(1);

	//	ret = rlylib_transSndFixMsgToHost(&rlylibCb, dstHost, sndBuf, sndBufLen);

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
			totRcvCnt += rcvCnt;
			fprintf(stderr,"sndCnt=%d, rcvCnt=%d, totRcvCnt=%d, dropCnt=%d\n",sndCnt, rcvCnt, totRcvCnt, dropCnt);
			sndCnt = 0;
			rcvCnt = 0;
			dropCnt = 0;
			prevTick10  = tmr.tick;
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

		if(sndMsgPerSecCnt < sndMsgPerSec){
			//ret = rlylib_apiSndFixMsgToHost(&rlylibCb, dstHost, sndBuf, sndBufLen);
			ret = rlylib_apiSndFixMsgToAny(&rlylibCb, sndBuf, sndBufLen, sndHost, &sndTransId);
			if(ret != RC_OK){
				fprintf(stderr,"Message send failed(ret=%d)\n",ret);
			}

			sndCnt++;
			sndMsgPerSecCnt++;
		}

		ret = rlylib_apiRcvFixMsgFromHost(&rlylibCb, dstHost , rcvBuf, MAX_LEN, &rcvLen);
		if(ret == RLYERR_MSG_NOT_EXIST){
			EXEC_TMR_TICK((&tmr));
			if(sndMsgPerSecCnt == sndMsgPerSec){
				usleep(1000);
			}
			continue;
		}
		else if(ret == RLYERR_DROP_MSG){
			dropCnt++;
		}
		else if(ret != RC_OK){
			fprintf(stderr,"Rcv failed(ret=%d)\n",ret);
			EXEC_TMR_TICK((&tmr));
			continue;
		}
		else {
			rcvCnt++;
		}

		EXEC_TMR_TICK((&tmr));
	}/* end of while(1) */

	return 0;
}
