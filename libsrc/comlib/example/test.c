#include <stdio.h>
#include <unistd.h> #include <stdlib.h>
#include <string.h>

#include "cmlib.h"

#define TEST_DATA 5

#define EVENT_1	1
#define EVENT_2 2
/* public */
	int flag= 0;

typedef struct listTest listTest; 
typedef struct hashTest hashTest;
typedef struct timerTest timerTest;

struct listTest{
	int index;
	LnkNode	entry;
};

struct hashTest{
	int index;
	char strId[32];
	int strLen;
	HashNode hNode;
};

struct timerTest{
	int event;
	TmrNode node;
};
RT_RESULT EvtFunc(UINT event, VOID *data);

TmrTbl tmrTbl;
timerTest test3[TEST_DATA];

RT_RESULT EvtFunc(UINT event, VOID *data)
{
	static int timer = 30;
	static int count = 0;
	fprintf(stderr,"Event Start = %d\n", event);
	if(event == 1){
		if(flag==1){
			fprintf(stderr,"test\n");
		}
		count ++;
		cmlib_timerTblStartTm(&tmrTbl, &test3[1].node, test3[1].event, 3);
		if(count == 10){
			cmlib_timerTblStartTm(&tmrTbl, &test3[1].node, test3[1].event, 3);
			fprintf(stderr,"Cancel tmr1\n");
			cmlib_timerTblCancelTm(&tmrTbl,&test3[1].node);
			cmlib_timerTblCancelTm(&tmrTbl,&test3[1].node);
			flag = 1;
		}
	}
	if(event == 2){
		cmlib_timerTblStartTm(&tmrTbl, &test3[2].node, test3[2].event, 20);
	}
	if(event == 3){
		timer--;
		if(timer <0){
			timer = 30;
		}
		cmlib_timerTblStartTm(&tmrTbl, &test3[3].node, test3[3].event, timer);
	}
	if(event == 4){
		cmlib_timerTblStartTm(&tmrTbl, &test3[4].node, test3[4].event, 10);
	}
	return 1;
}

int main()
{
	int intTmp = 0;
	int ret = 0;
	int i = 0;
	int len = 0;
	HashNode *hFind;
	LnkLst list;
	HashTbl tbl;
	listTest test1[TEST_DATA];
	hashTest test2[TEST_DATA];
	HashKey key;
	char tmp[32];
	double tick;

	Timer tmr;

	memset(&list,0,sizeof(LnkLst));
	memset(test1,0,sizeof(listTest)*TEST_DATA);

	CM_LOG(CM_ERR,"TEST START\n");
	for(i=0;i<TEST_DATA;i++){
		test1[i].index = i;
		test1[i].entry.data =  &test1[i];
	}

	for(i=0;i<TEST_DATA;i++){
		test2[i].index = i;
		len = sprintf(tmp,"STR_%d\n",i);
		strcpy(test2[i].strId,tmp);
		test2[i].strLen = len;
		test2[i].hNode.data = &test2[i];
	}

	for(i=0;i<TEST_DATA;i++){
		test3[i].event = i;
		test3[i].node.data = &test3[i];
	}

    cmlib_lnkLstInit(&list, 0);

	cmlib_lnkLstInsertTail(&list, &test1[3].entry);
	cmlib_lnkLstDel(&list, &test1[3].entry);

	/* hash test */
	//cmlib_hashTblInit(&tbl,5, RC_FALSE,CM_HASH_TYPE_STRING);
	cmlib_hashTblInit(&tbl,5, RC_FALSE,CM_HASH_TYPE_UINT);
	//key.key = test2[4].strId;
	//key.keyLen = test2[4].strLen;
	intTmp = 21;
	key.key = &intTmp;
	key.keyLen = sizeof(unsigned int);
	ret = cmlib_hashTblInsertHashNode(&tbl,&key, &test2[4].hNode);
	printf("ret = %d\n",ret);
	intTmp = 22;
	ret = cmlib_hashTblInsertHashNode(&tbl,&key, &test2[4].hNode);
	printf("ret = %d\n",ret);

	ret = cmlib_hashTblFindHashNode(&tbl,&key,0,&hFind);
	printf("ret = %d\n",ret);

	ret = cmlib_hashTblDelHashNode(&tbl, &key, 0);
	printf("ret = %d\n", ret);


	/* timer test */
	//cmlib_timerInit(&tmr, CM_TIMER_TYPE_100M);
	cmlib_timerInit(&tmr, CM_TIMER_TYPE_SEC);
	cmlib_timerTblInit(&tmrTbl, &tmr, EvtFunc);

	cmlib_timerTblStartTm(&tmrTbl, &test3[1].node, test3[1].event, 3);
	cmlib_timerTblStartTm(&tmrTbl, &test3[2].node, test3[2].event, 20);
	cmlib_timerTblStartTm(&tmrTbl, &test3[3].node, test3[3].event, 30);
	cmlib_timerTblStartTm(&tmrTbl, &test3[4].node, test3[4].event, 10);

	for(;;){
		if(tick != GET_CUR_TICK(&tmr)){
			fprintf(stderr,"tick=%lf\n",tick);
			tick = GET_CUR_TICK(&tmr);
		}
		if(tmr.tick == 2){
			fprintf(stderr,"tick=%lf, %lf\n",tick, GET_CUR_TICK(&tmr));
			sleep(2);
		}
		EXEC_TMR_TICK(&tmr);

		cmlib_timerTblHandler(&tmrTbl);

	}
	return 0;

}

