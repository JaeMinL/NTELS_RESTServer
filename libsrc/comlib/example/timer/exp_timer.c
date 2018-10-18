#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#define TMR_EVNT_TMOUT_1SEC    1
#define TMR_EVNT_TMOUT_3SEC    2
#define TMR_EVNT_TMOUT_5SEC    3

typedef struct TmrEvntCb    TmrEvntCb;
typedef struct TmrCb        TmrCb;

struct TmrEvntCb{
    UINT                  tmrEvnt;
    UINT                  tmOutIntvl;
    ComlibTmrNode         tmrNode;
};

struct TmrCb{
    ComlibTimer           tmr;
    ComlibTmrTbl          tmrTbl;
};

TmrCb g_tmrCb;

FT_PRIVATE RT_RESULT tmrEvntFunc(UINT event, VOID *data);

FT_PRIVATE RT_RESULT tmrEvntFunc(UINT event, VOID *data)
{
    SINT ret = RC_OK;
    TmrEvntCb *evntCb = NULL;
    UINT tmOutVal = 0;

    evntCb = data;

    switch(event){
        case TMR_EVNT_TMOUT_1SEC:
            {
                fprintf(stderr,"1sec timeout occure\n");

                tmOutVal = 10;
            }
            break;
        case TMR_EVNT_TMOUT_3SEC:
            {
                fprintf(stderr,"3sec timeout occure\n");

                tmOutVal = 30;
            }
            break;
        case TMR_EVNT_TMOUT_5SEC:
            {
                fprintf(stderr,"5sec timeout occure\n");

                tmOutVal = 50;
            }
            break;
    };

    fprintf(stderr,"restart %dsec timeout event\n", tmOutVal);

    ret = comlib_timerTblStartTm(&g_tmrCb.tmrTbl, &evntCb->tmrNode, evntCb->tmrEvnt, tmOutVal);
    if(ret != RC_OK){
        fprintf(stderr,"1sec timer start failed(ret=%d)\n",ret);
        return -1;
    }

    return 1;
}

int main()
{
    SINT ret = RC_OK;
    TmrEvntCb *evntCb = NULL;

    /* timer setting */
    ret = comlib_timerInit(&g_tmrCb.tmr, COM_TIMER_TYPE_100M);
    if(ret != RC_OK){
        fprintf(stderr,"Timer init failed(ret=%d)\n",ret);
        return -1;
    }

    /* timer table setting */
    ret = comlib_timerTblInit(&g_tmrCb.tmrTbl, &g_tmrCb.tmr, tmrEvntFunc);
    if(ret != RC_OK){
        fprintf(stderr,"Timer table init failed(ret=%d)\n",ret);
        return -1;
    }

    /* 1sec timer start */
    {
        evntCb = comlib_memMalloc(sizeof(TmrEvntCb));

        evntCb->tmrEvnt = TMR_EVNT_TMOUT_1SEC;
        evntCb->tmOutIntvl = 10;
        evntCb->tmrNode.data = evntCb;;

        comlib_timerTblStartTm(&g_tmrCb.tmrTbl, &evntCb->tmrNode, evntCb->tmrEvnt, 10);
        if(ret != RC_OK){
            fprintf(stderr,"5sec timer start failed(ret=%d)\n",ret);
            return -1;
        }
    }

    /* 3sec timer start */
    {
        evntCb = comlib_memMalloc(sizeof(TmrEvntCb));

        evntCb->tmrEvnt = TMR_EVNT_TMOUT_3SEC;
        evntCb->tmOutIntvl = 30;
        evntCb->tmrNode.data = evntCb;;

        comlib_timerTblStartTm(&g_tmrCb.tmrTbl, &evntCb->tmrNode, evntCb->tmrEvnt, 30);
        if(ret != RC_OK){
            fprintf(stderr,"5sec timer start failed(ret=%d)\n",ret);
            return -1;
        }
    }

    /* 5sec timer start */
    {
        evntCb = comlib_memMalloc(sizeof(TmrEvntCb));

        evntCb->tmrEvnt = TMR_EVNT_TMOUT_5SEC;
        evntCb->tmOutIntvl = 50;
        evntCb->tmrNode.data = evntCb;;

        ret = comlib_timerTblStartTm(&g_tmrCb.tmrTbl, &evntCb->tmrNode, evntCb->tmrEvnt, 50);
        if(ret != RC_OK){
            fprintf(stderr,"5sec timer start failed(ret=%d)\n",ret);
            return -1;
        }
    }

    for(;;){
        EXEC_TMR_TICK(&g_tmrCb.tmr);

        comlib_timerTblHandler(&g_tmrCb.tmrTbl);
    }/* end of for(;;) */

    return 0;
}

