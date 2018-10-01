#include <unistd.h>
#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"
#include "rlylibInt.h"
#include "rlylibInt.x"

FT_PUBLIC RT_RESULT rlylibInt_dstryMainCbForFork(RlylibIntMainCb *intMainCb)
{
    SINT ret = RC_OK;
    RlylibIntMsgChk *msgChk = NULL;

    /* clear acpt */
    if(intMainCb->rlyType != RLYLIB_TYPE_CLIENT){
        intMainCb->acptThrdCb.termSig = RC_TRUE;

        while(1){
            if(intMainCb->acptThrdCb.termFlg == RC_TRUE){
                break;
            }
            usleep(500);
        }/* end of while(1) */
    }

    /* clear thread */
    intMainCb->hostMainCb.termFlg = RC_TRUE;

    while(1){
        if(intMainCb->hostMainCb.actThrdCnt == 0){
            break;
        }
        usleep(500);
    }/* end of while(1) */

    /* close realm */
    ret = rlylibInt_rlmDstryAll(intMainCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Realm destory failed(ret=%d)\n",ret);
    }

    /* close all host */
    ret = rlylibInt_hostDstryAll(intMainCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host destory failed(ret=%d)\n",ret);
    }

    /* close main */
    thrlib_condDstry(&intMainCb->rdCond);

    while(1){
        ret = thrlib_tqPop(&intMainCb->freeMsgChkQ, (VOID*)&msgChk);
        if(ret == THRERR_TQ_EMPTY){
            break;
        }
        else if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Free msg chunk queue pop failed(ret=%d)\n",ret);
            break;
        }

        comlib_memFree(msgChk);
    }/* end of while(1) */

    ret = thrlib_tqDstry(&intMainCb->freeMsgChkQ);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Free message chunk queue free failed(ret=%d)\n",ret);
    }

    ret = rlylibInt_msgDstryMsgBufInfo(&intMainCb->msgBufInfo);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer destory failed(ret=%d)\n",ret);
    }

    ret = comlib_hashTblDstry(&intMainCb->hostHt);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host hash table destory failed(ret=%d)\n",ret);
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylibInt_dstryMainCb(RlylibIntMainCb *intMainCb)
{
    SINT ret = RC_OK;
    RlylibIntMsgChk *msgChk = NULL;

    /* clear acpt */
    if(intMainCb->rlyType != RLYLIB_TYPE_CLIENT){
        intMainCb->acptThrdCb.termSig = RC_TRUE;

        while(1){
            if(intMainCb->acptThrdCb.termFlg == RC_TRUE){
                break;
            }
            usleep(500);
        }/* end of while(1) */
    }

    /* clear thread */
    intMainCb->hostMainCb.termFlg = RC_TRUE;

    while(1){
        if(intMainCb->hostMainCb.actThrdCnt == 0){
            break;
        }
        usleep(500);
    }/* end of while(1) */

    /* close realm */
    ret = rlylibInt_rlmDstryAll(intMainCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Realm destory failed(ret=%d)\n",ret);
    }

    /* close all host */
    ret = rlylibInt_hostDstryAll(intMainCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host destory failed(ret=%d)\n",ret);
    }

    /* close main */
    thrlib_condDstry(&intMainCb->rdCond);

    while(1){
        ret = thrlib_tqPop(&intMainCb->freeMsgChkQ, (VOID*)&msgChk);
        if(ret == THRERR_TQ_EMPTY){
            break;
        }
        else if(ret != RC_OK){
            RLY_LOG(RLY_ERR,"Free msg chunk queue pop failed(ret=%d)\n",ret);
            break;
        }

        comlib_memFree(msgChk);
    }/* end of while(1) */

    ret = thrlib_tqDstry(&intMainCb->freeMsgChkQ);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Free message chunk queue free failed(ret=%d)\n",ret);
    }

    ret = rlylibInt_msgDstryMsgBufInfo(&intMainCb->msgBufInfo);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Message buffer destory failed(ret=%d)\n",ret);
    }

    ret = comlib_hashTblDstry(&intMainCb->hostHt);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Host hash table destory failed(ret=%d)\n",ret);
    }

    return RC_OK;
}
