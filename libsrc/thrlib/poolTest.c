#include <unistd.h>
#include <stdio.h>

#include "gendef.h"
#include "cmlib.h"
#include "cmlib.x"
#include "thlib.h"
#include "thlib.x"

#define TEST_NUM 10000000 

extern int regJobCnt;
typedef struct TestMsg TestMsg;

struct TestMsg{
	UINT id;
};

struct timespec endTm;
	ThlibPool pool; 

#define TEST_BIN_SIZE 20490
VOID funcTest1(ThlibThrdId tid, VOID *args)
{
	UINT i = 0;
	UINT a = 0;

	for(i=0;i<TEST_BIN_SIZE;i++){
		a= a*a;
	}

	if((UINT)args == TEST_NUM-1){
		cmlib_timerGetTime(&endTm);

		fprintf(stderr,"end   args=%d tm=%7ld, %7ld\n",args, endTm.tv_sec , endTm.tv_nsec);
	}
	if((UINT)args == TEST_NUM-2){

		fprintf(stderr,"prev args=%d\n",args);
	}
}

VOID funcTest2(ThlibThrdId tid, VOID *args)
{
	SINT ret = 0;

	UINT i = 0 ;

	i = (UINT)args;

	ret = thlib_poolRegJob(&pool, 0, funcTest1, i);
	if(ret!= RC_OK){
		fprintf(stderr,"error=%d\n",ret);
	}

	i++;
	if(i != TEST_NUM){
		ret = thlib_poolRegJob(&pool, 0, funcTest2, i);

	}
}

FT_PUBLIC UINT hlrsim_toolsCalcRttMs(struct timespec *stTm, struct timespec *endTm)
{
    ULONG diffNSec = 0;
    ULONG diffSec = 0;

    diffSec =  endTm->tv_sec - stTm->tv_sec;

    if(endTm->tv_nsec < stTm->tv_nsec){
        diffNSec = (endTm->tv_nsec+CM_TIMER_TICK_SEC) - stTm->tv_nsec;
        diffSec--;
    }
    else {
        diffNSec = endTm->tv_nsec - stTm->tv_nsec;
    }

    return (diffSec * CM_TIMER_TICK_SEC/* 1sec */) + (diffNSec);
}

int main()
{
	UINT actvNodeCnt = 0;
	UINT sndCnt = 0;
	UINT pushCnt=0;
	SINT ret = 0;
	UINT diff = 0;
	UINT i = 0;
	UINT j = 0;
	UINT prntCnt=0;
	UINT sesLoopCnt = 0;
	UINT loopCnt = 0;
	time_t curTm = 0;
	time_t diffTm1= 0;
	struct timespec tm;
	TestMsg testMsg;
	CmlibLnkNode  *lnkNode = NULL;
	ThlibPoolWkr *wkr = NULL;

	fprintf(stderr, "start test\n");

	memset(&endTm,0x0,sizeof(struct timespec));
	thlib_poolInit(&pool, 30, 5,  10, 0);

	cmlib_timerGetTime(&tm);
	curTm = time(NULL);
	diffTm1 = curTm;

	fprintf(stderr,"start tm=%7d, %7d\n",tm.tv_sec , tm.tv_nsec);
#if 0
	int sndCnt = 0;
	for(sndCnt = 0;sndCnt < 5; sndCnt++){
		ret = thlib_poolRegJob(&pool, 0, funcTest2, i);
	}
#endif
	while(1){
#if 1
		if(i != TEST_NUM){
			if(sesLoopCnt == 1){
				if(sndCnt == 2000){
					sndCnt = 0;
					ret = thlib_poolRegJob(&pool, 0, funcTest1, i);
					if(ret!= RC_OK){
						fprintf(stderr,"error=%d\n",ret);
					}
					i++;
					pushCnt++;
				}

			}
			else {
				ret = thlib_poolRegJob(&pool, 0, funcTest1, i);
				if(ret!= RC_OK){
					fprintf(stderr,"error=%d\n",ret);
				}
				i++;
				pushCnt++;
			}
		}
		else {

			//sleep(15);
			//i = 0;
		}
#endif

		if(loopCnt == 300){
			curTm = time(NULL);
			if(curTm != diffTm1) {
				fprintf(stderr,"[%d]queued regCnt=%d\n",
						prntCnt, regJobCnt);
				prntCnt++;
				usleep(1000);

				CM_GET_LNKLST_FIRST((&pool.busyWkr), lnkNode);
				if(lnkNode != NULL){
					while(1){
						wkr = lnkNode->data;

						//fprintf(stderr,"procjobCnt=%d\n",wkr->procJobCnt);
						fprintf(stderr,"%d\n",wkr->procJobCnt);
						CM_GET_NEXT_NODE(lnkNode);
						if(lnkNode == NULL){
							fprintf(stderr,"\n");
							break;
						}
					}
				}

				if(prntCnt == 7){ 
					thlib_poolDstry(&pool);
					fprintf(stderr,"RESET POOL\n");
					thlib_poolInit(&pool, 15, 7,  10, 0);
					i = 0;
				}

				j = 0;
				while(1){
					if(j == pool.strmMaxCnt){ 
						break;
					}
					fprintf(stderr,"Job[%d]=%d\n", j, pool.jobQ[j]->regJob.nodeCnt);
					actvNodeCnt += pool.jobQ[j]->regJob.nodeCnt;
					j++;
				}

				if(actvNodeCnt == 0 && i == TEST_NUM){
					sleep(15);
					thlib_poolDstry(&pool);
					fprintf(stderr,"CLEAR POOL\n");
					thlib_poolInit(&pool, 10, 5,  10, 0);

					fprintf(stderr,"RE ACTIVE\n");
					i = 0;
					sesLoopCnt++;
				}
				actvNodeCnt = 0;
				diffTm1 = curTm;
			}
			loopCnt = 0;
		}

		loopCnt++;
		sndCnt++;
	}
	return 0;
}
