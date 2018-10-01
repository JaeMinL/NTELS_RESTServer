#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

/* PRIVATE DEFINITION */
#define TIMER_MAX_ENTRY 1024 /* only used 2^x */

/* PRIVATE MACRO */
#if _POSIX_TIMER > 0
#define GET_TIME(tm){\
	clock_gettime(CLOCK_REALTIME, tm);\
}
#else
#define GET_TIME(tm){\
	struct timeval tv;\
	gettimeofday(&tv, NULL);\
	(tm)->tv_sec = tv.tv_sec;\
	(tm)->tv_nsec = tv.tv_usec*1000;\
}
#endif

#define INSERT_TMR_NODE(entry, node){\
	if(entry->tail){\
		node->prev = entry->tail;\
		entry->tail->next = node;\
		entry->tail = node;\
	}\
	else{\
		entry->first = node;\
		entry->tail = node;\
		node->prev = NULL;\
	}\
}

#define DEL_TMR_NODE(entry, node){\
	if(node->prev == NULL){\
		entry->first = node->next;\
		if(entry->first){\
			entry->first->prev = NULL;\
		}\
	}\
	if(node->next == NULL){\
		entry->tail = node->prev;\
		if(entry->tail){\
			entry->tail->next = NULL;\
		}\
	}\
	if(node->prev != NULL && node->next != NULL){\
		node->next->prev = node->prev;\
		node->prev->next = node->next;\
	}\
}

/* Private Function */


FT_PUBLIC RT_RESULT comlib_timerInit(ComlibTimer *tmr, UINT type)
{
    GEN_CHK_ERR_RET(tmr == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer struture(NULL)\n"),
                    COMERR_INVAILD_TIMER);

    switch(type){
        case COM_TIMER_TYPE_SEC:
            tmr->tmSlice = COM_TIMER_TICK_SEC;
            tmr->type = COM_TIMER_TYPE_SEC;
            break;
        case COM_TIMER_TYPE_100M:
            tmr->tmSlice = COM_TIMER_TICK_100MS;
            tmr->type = COM_TIMER_TYPE_100M;
            break;
        default :
            return COMERR_INVAILD_TIMER_TYPE;
            break;
    };

    /* set init time */
    GET_TIME(&tmr->curTs);

#ifdef _BIT_64_
    tmr->lastTm  = (double)(tmr->curTs.tv_sec *COM_TIMER_TICK_SEC) + (double)tmr->curTs.tv_nsec;
#else 
    tmr->lastTs.tv_sec = tmr->curTs.tv_sec;
    tmr->lastTs.tv_nsec = tmr->curTs.tv_nsec;
    tmr->tps = COM_TIMER_TICK_SEC / tmr->tmSlice;
#endif

    tmr->avgDiff = 1000000000; 

    tmr->tick = 0;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerTblInit(ComlibTmrTbl *tbl, ComlibTimer *tmr, ComlibTmrEvtFunc func)
{
    GEN_CHK_ERR_RET(tbl == NULL,
                    COM_LOG(COM_ERR,"Invalid Timer Table\n"),
                    COMERR_INVAILD_TMRTBL);

    GEN_CHK_ERR_RET(tmr == NULL,
                    COM_LOG(COM_ERR,"Invalid Timer\n"),
                    COMERR_INVAILD_TIMER);

    GEN_CHK_ERR_RET(func == NULL,
                    COM_LOG(COM_ERR,"Invalid function\n"),
                    COMERR_INVAILD_TMREVTFUNC);

    tbl->tmr = tmr;
    tbl->prevTick = tmr->tick;
    tbl->entry = comlib_memCalloc(TIMER_MAX_ENTRY,sizeof(ComlibTmrEntry));
    tbl->func = func;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerTblHandler(ComlibTmrTbl *tbl)
{
    ComlibTmrNode *node = NULL;
    ComlibTmrNode *tmpNode = NULL;
    ComlibTmrEntry *tmpEntry = NULL;
    ComlibTimer *tmr = NULL;
    ComlibTmrEntry *entry = NULL;
    UINT indx = 0;
    ULONG diff = 0;
    ULONG prevTick = 0;

    GEN_CHK_ERR_RET(tbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer Table\n"),
                    COMERR_INVAILD_TMRTBL);

    tmr = tbl->tmr;

    prevTick = tbl->prevTick;
    CHECK_TICK_DIFF(prevTick, tmr->tick, diff);

    while(diff){
        prevTick++;

        /* select tm entry */
        indx = prevTick & (TIMER_MAX_ENTRY-1);
        entry = &tbl->entry[indx];

        if(!entry->first){ /* if first is null */
            diff--;
            continue;
        }

        /* check node in entry */
        node = entry->first;
        while(node){
            tmpNode = node->next;

            if(node->resetFlg){
                DEL_TMR_NODE(entry, node);

                node->resetFlg = RC_FALSE;
                node->prev = NULL;
                node->next = NULL;
                node->expTick += node->updateTick;
                node->updateTick = 0;
                indx = node->expTick & (TIMER_MAX_ENTRY - 1);
                tmpEntry = &tbl->entry[indx]; 
                node->ownEntry = tmpEntry;

                INSERT_TMR_NODE(tmpEntry,node);
            }
            else {
                if(node->expTick == prevTick){
                    DEL_TMR_NODE(entry, node);

                    /* init node */
                    node->used = RC_NOT_USED;
                    node->resetFlg = RC_FALSE;
                    node->expTick = 0;
                    node->updateTick = 0;
                    node->ownEntry = NULL;
                    node->prev = NULL;
                    node->next = NULL;

                    /* exec tmr event */
                    (*tbl->func)(node->event, node->data);
                }
            }
            node = tmpNode;
        }/* while(diff) */

        diff--;
    }

    tbl->prevTick = prevTick;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerTblStartTm(ComlibTmrTbl *tbl, ComlibTmrNode *node, UINT event, UINT expTm)
{
    UINT expTick = 0;
    UINT indx = 0;
    ComlibTimer *tmr = NULL;
    ComlibTmrEntry *entry = NULL;

    GEN_CHK_ERR_RET(tbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer Table\n"),
                    COMERR_INVAILD_TMRTBL);

    GEN_CHK_ERR_RET(node == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer Node\n"),
                    COMERR_INVAILD_TMRNODE);

    if(node->used == RC_USED){
        COM_LOG(COM_ERR,"Timer Already start\n");
        return COMERR_TIMER_ALREADY_START;
    }

    tmr = tbl->tmr;

    expTick = tmr->tick + expTm;

    indx = expTick & (TIMER_MAX_ENTRY - 1);

    entry = &tbl->entry[indx];

    /* set node infomation */
    node->used = RC_USED;
    node->expTick = expTick;
    node->event = event;
    node->updateTick = 0;
    node->resetFlg = RC_FALSE;
    node->ownEntry = entry;

    INSERT_TMR_NODE(entry, node);

    node->next = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerTblRestartTm(ComlibTmrTbl *tbl, ComlibTmrNode *node, UINT expTm)
{
    GEN_CHK_ERR_RET(tbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer Table\n"),
                    COMERR_INVAILD_TMRTBL);

    GEN_CHK_ERR_RET(node == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer Node\n"),
                    COMERR_INVAILD_TMRNODE);

    if(node->used != RC_USED){
        COM_LOG(COM_ERR,"timer Node is not used\n");
        return COMERR_NODE_IS_NOT_USED;
    }
    node->resetFlg = RC_TRUE;
    node->updateTick += expTm;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerTblCancelTm(ComlibTmrTbl *tbl, ComlibTmrNode *node)
{
    ComlibTmrEntry *entry;

    GEN_CHK_ERR_RET(tbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer table\n"),
                    COMERR_INVAILD_TMRTBL);

    GEN_CHK_ERR_RET(node == NULL,
                    COM_LOG(COM_ERR,"Invaild Timer node\n"),
                    COMERR_INVAILD_TMRNODE);

    if(node->used == RC_NOT_USED){
        return RC_OK;
    }

    if(node->ownEntry == NULL){
        COM_LOG(COM_ERR,"Invaild Timer entry\n");
        return COMERR_INVAILD_TMRENTRY;
    }

    entry = node->ownEntry;

    DEL_TMR_NODE(entry, node);

    /* init node */
    node->used = RC_NOT_USED;
    node->resetFlg = RC_FALSE;
    node->expTick = 0;
    node->updateTick = 0;
    node->ownEntry = NULL;
    node->prev = NULL;
    node->next = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_timerGetTime(struct timespec *tm)
{
	GET_TIME(tm);

	return R_OK;
}

