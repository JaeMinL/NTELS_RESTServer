#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"

typedef struct _ttt{
	unsigned int id;
	ThrlibThrdId thrdId;
	ThrlibTq *tq;
}testThrd;


ThrlibTq tq;

#if 0
unsigned int term = 10000000;
#endif
unsigned int term = 100000000;
unsigned int j = 0;
struct timeval tm1;
struct timeval tm2;

VOID sender(VOID *arg)
{
	UINT ret = RC_OK;
    UINT wantCnt = 0;
    UINT sndCnt = 0;
	unsigned int i = 0;
	ThrlibTq *que = NULL;
	//unsigned int a = 1;
	long a = 1;
    VOID *bulk[5];

	que = arg;

	if(j == 0){
		gettimeofday(&tm1, NULL);
	}
    while(1) {
#if 0
            sleep(1);
        wantCnt = 5;
        bulk[0] = a++;
        bulk[1] = a++;
        bulk[2] = a++;
        bulk[3] = a++;
        bulk[4] = a++;

        while(1){
            ret = thrlib_tqPushBulk(que, bulk, wantCnt, &sndCnt);
            if(ret == THRERR_TQ_FULL){
                fprintf(stderr,"Queue is full(a=%d)\n",a);
                continue;
            }
            if(sndCnt!= wantCnt){
                wantCnt = wantCnt - sndCnt;
                memcpy(&bulk[0], &bulk[sndCnt], wantCnt);
                continue;
            }
            else {
                break;
            }
        }
#else
#if 1
        sleep(1);
        ret = thrlib_tqPush(que, (VOID*)a);
#else 
        ret = thrlib_tqWaitPush(que, (VOID*)a,NULL);
#endif
        if(ret == THRERR_TQ_FULL){
            fprintf(stderr,"Queue is full(a=%d)\n",a);
            continue;
        }
        a++;
#endif

#if 0
        for(i=0;i<10000;i++){
        }
#endif

        j++;
        if(j >= term){
#if 0
            gettimeofday(&tm2, NULL);
            fprintf(stderr,"end sec=%d, usec=%d)\n",
                    (tm2.tv_sec - tm1.tv_sec),
                    (tm2.tv_usec - tm1.tv_usec));
            while(1);
#endif
        }
    }
}

VOID receiver(VOID *arg)
{
	UINT ret = RC_OK;
	long a = 0;
	ThrlibTq *que;
	unsigned int test = 1;
	struct timespec wait;
	testThrd *tThrd = NULL;

	tThrd = (testThrd*)arg;
	que = tThrd->tq;

	while(1) {
		wait.tv_sec = 1;
		wait.tv_nsec = 0;
#if 1
		ret = thrlib_tqPop(que,(VOID*)&a );
#else 
		ret = thrlib_tqWaitPop(que,(VOID*)&a, &wait);
#endif
		if(ret == THRERR_TQ_EMPTY){
		//	fprintf(stderr,"empyt\n");
			continue;
		}
		if(ret != RC_OK){
#if 1
			fprintf(stderr, "id=%d, timeout\n",tThrd->id);
#endif
			continue;
		}
#if 1
		if(test == 0){
			test = 1;
		}

        printf("a=%d\n",a);
		if(test != a){
			fprintf(stderr,"Invalid test=%d, id=%d)\n",test, a);
			exit(0);
		}
#endif

#if 1
		//fprintf(stderr, "id=%d, a=%d test=%d\n",tThrd->id, a,test);
		if(a == term){
			gettimeofday(&tm2, NULL);
			fprintf(stderr,"end sec=%d, usec=%d)\n",
					(tm2.tv_sec - tm1.tv_sec),
					(tm2.tv_usec - tm1.tv_usec));
			exit(0);
		}
#endif
		test++;
	}
}

int main()
{
	testThrd thrd[5];
	ThrlibThrdId thrdId1 = 0;
	ThrlibThrdId thrdId2 = 0;
	ThrlibThrdId thrdId3 = 0;
	ThrlibThrdId thrdId4 = 0;
	ThrlibThrdId thrdId5 = 0;
	UINT ret = RC_OK;

#if 0
	ret = thrlib_tqInit(&tq,TH_TQ_LOCK_TYPE_LOCKED, 8192 * 2);
#else
#if 0
	ret = thrlib_tqInit(&tq,THR_TQ_LOCK_TYPE_LOCK_FREE, 8192 * 2);
#else
	ret = thrlib_tqInit(&tq,THR_TQ_LOCK_TYPE_LOCK_FREE, 32);
#endif
#endif

	/* sender */
	thrlib_thrdCrte(&thrdId1, NULL, sender, &tq);
	//thrlib_thrdCrte(&thrdId2, NULL, sender, &tq);
	//thrlib_thrdCrte(&thrdId3, NULL, sender, &tq);
	//thrlib_thrdCrte(&thrdId4, NULL, sender, &tq);
	//thrlib_thrdCrte(&thrdId5, NULL, sender, &tq);


	/* receiver */
	thrd[0].id = 0;
	thrd[0].tq = &tq;
	thrlib_thrdCrte(&thrd[0].thrdId, NULL, receiver, &thrd[0]);
#if 0
	thrd[1].id = 1;
	thrd[1].tq = &tq;
	thrlib_thrdCrte(&thrd[1].thrdId, NULL, receiver, &thrd[1]);
	thrd[2].id = 2;
	thrd[2].tq = &tq;
	thrlib_thrdCrte(&thrd[2].thrdId, NULL, receiver, &thrd[2]);
	thrd[3].id = 3;
	thrd[3].tq = &tq;
	thrlib_thrdCrte(&thrd[3].thrdId, NULL, receiver, &thrd[3]);
	thrd[4].id = 4;
	thrd[4].tq = &tq;
	thrlib_thrdCrte(&thrd[4].thrdId, NULL, receiver, &thrd[4]);
#endif

	while(1);
	return 0;
}
