#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "loglibInt.h"
#include "loglibInt.x"

STATIC U_32                 g_loglibDispEnv = LOGLIB_DISP_TIME_BIT | LOGLIB_DISP_LVL_BIT | 
                                              LOGLIB_DISP_FILE_BIT | LOGLIB_DISP_LINE_BIT;
STATIC BOOL                 g_loglibInitFlg = RC_FALSE;
STATIC LoglibIntGlobCb      g_globCb;

FT_PUBLIC RT_RESULT loglibInt_globGetInitFlg(VOID)
{
    return g_loglibInitFlg;
}

FT_PUBLIC RT_RESULT loglibInt_globInit(VOID)
{
    SINT ret = RC_OK;
    UINT i = 0;
    LoglibIntGlobCb *globCb = NULL;

    if(g_loglibInitFlg == RC_TRUE){
        return RC_OK;
    }

    globCb = &g_globCb;

    thrlib_mutxInit(&globCb->mutx);

    for(i=0;i<LOGLIB_LVL_CNT;i++){
        globCb->maxLogLvl[i] = 0;
    }

    globCb->maxLogLvlVal = LOGLIB_LVL_NONE;

    globCb->dfltMainCb = NULL;

    ret = comlib_hashTblInit(&globCb->logHt, LOGLIB_MAX_ENTRY_CNT, 
            RC_FALSE, COM_HASH_TYPE_MURMUR, NULL);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Log name hash table init failed(ret=%d)\n",ret);
        return LOGERR_HASH_TBL_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&globCb->logLl, ~0);
    if(ret != RC_OK){
        LOG_LOG(LOG_INT_ERR,"Log Name Linked list init failed(ret=%d)\n",ret);
        return LOGERR_LNK_LST_INIT_FAILED;
    }

    g_loglibInitFlg = RC_TRUE;

    return RC_OK;
}

FT_PUBLIC U_32 loglibInt_globGetDispEnv(VOID)
{
    return g_loglibDispEnv;
}

FT_PUBLIC VOID loglibInt_globSetDispEnv(U_32 bit)
{
    g_loglibDispEnv = bit;
}

FT_PUBLIC LoglibIntMainCb* loglibInt_globGetDfltLoglibCb(VOID)
{
    return g_globCb.dfltMainCb;
}

FT_PUBLIC LoglibIntMainCb* loglibInt_globDelDfltLoglibCb(VOID)
{
    LoglibIntMainCb *mainCb = NULL;

    mainCb = g_globCb.dfltMainCb;

    g_globCb.dfltMainCb = NULL;

    return mainCb;
}

FT_PUBLIC RT_RESULT loglibInt_globRegLoglibCb(LoglibIntMainCb *mainCb)
{
    SINT ret = RC_OK;
    LoglibIntGlobCb *globCb = NULL;

    globCb = &g_globCb;

    thrlib_mutxLock(&globCb->mutx);

    if(mainCb->nameLen == 0){/* default */
        globCb->dfltMainCb = mainCb;
    }
    else {
        ret = comlib_lnkLstInsertTail(&globCb->logLl, &mainCb->lnkNode);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Linked list insert failed(ret=%d)\n",ret);
            thrlib_mutxUnlock(&globCb->mutx);
            return LOGERR_LOG_REG_FAILED;
        }

        ret = comlib_hashTblInsertHashNode(&globCb->logHt, &mainCb->hNode.key, &mainCb->hNode);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Hash Table failed(ret=%d)\n",ret);
            thrlib_mutxUnlock(&globCb->mutx);
            return LOGERR_LOG_REG_FAILED;
        }
    }

    thrlib_mutxUnlock(&globCb->mutx);

    return RC_OK;
}

/* thread unsafe */
FT_PUBLIC RT_RESULT loglibInt_globFindLoglibCb(CONST CHAR *name, LoglibIntMainCb **rt_mainCb, BOOL delFlg)
{
    SINT ret = RC_OK;
    UINT nameLen = 0;
    CHAR upperName[LOGLIB_LOG_NAME_MAX_LEN];
    LoglibIntGlobCb *globCb = NULL;
    ComlibHashKey hKey;
    ComlibHashNode *hNode = NULL;

    globCb = &g_globCb;

    if(name == NULL){
        if(globCb->dfltMainCb == NULL){
            return RC_NOK;
        }

        (*rt_mainCb) = globCb->dfltMainCb;
        if(delFlg == RC_TRUE){
            globCb->dfltMainCb = NULL;
        }

        return RC_OK;
    }/* end of if(name == NULL) */

    nameLen = comlib_strGetLen(name);

    if(nameLen >= (LOGLIB_LOG_NAME_MAX_LEN-1)){
        return LOGERR_LOG_NAME_LEN_TO_LONG;
    }

    comlib_strChgStrToUpper(name, nameLen, upperName, LOGLIB_LOG_NAME_MAX_LEN);

    hKey.key = upperName;
    hKey.keyLen = nameLen;

    if(delFlg == RC_FALSE){
        ret = comlib_hashTblFindHashNode(&globCb->logHt, &hKey, 0, &hNode);
        if(ret != RC_OK){
            return RC_NOK;
        }
    }
    else {
        LoglibIntMainCb *mainCb = NULL;

        ret = comlib_hashTblGetHashNode(&globCb->logHt, &hKey, 0, &hNode);
        if(ret != RC_OK){
            return RC_NOK;
        }

        mainCb = hNode->data;

        ret = comlib_lnkLstDel(&globCb->logLl, &mainCb->lnkNode);
        if(ret != RC_OK){
            LOG_LOG(LOG_INT_ERR,"Main control block delete failed(ret=%d)\n",ret);
            return RC_NOK;
        }
    }

    (*rt_mainCb) = hNode->data;

    return RC_OK;
}

FT_PUBLIC VOID loglibInt_globLock(VOID)
{
    thrlib_mutxLock(&g_globCb.mutx);
}

FT_PUBLIC VOID loglibInt_globUnlock(VOID)
{
    thrlib_mutxUnlock(&g_globCb.mutx);
}

FT_PUBLIC RT_RESULT loglibInt_globSetMaxLogLvl(UINT lvl)
{
    if(lvl > LOGLIB_LVL_CNT){
        LOG_LOG(LOG_INT_ERR,"Invalid log level(%d)\n",lvl);
        return RC_NOK;
    }

    g_globCb.maxLogLvl[lvl]++;

    if(g_globCb.maxLogLvlVal < lvl){
        g_globCb.maxLogLvlVal = lvl;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_globUnsetMaxLogLvl(UINT lvl)
{
    UINT chkLvl = LOGLIB_LVL_NONE;

    if(lvl > LOGLIB_LVL_CNT){
        LOG_LOG(LOG_INT_ERR,"Invalid log level(%d)\n",lvl);
        return LOGERR_INVLAID_LOG_LVL;
    }

    g_globCb.maxLogLvl[lvl]--;

    chkLvl = lvl;
    while(1){
        if(g_globCb.maxLogLvl[chkLvl] != 0){
            g_globCb.maxLogLvlVal = chkLvl;
            break;
        }

        if(chkLvl <= LOGLIB_LVL_NONE){
            g_globCb.maxLogLvlVal = LOGLIB_LVL_NONE;
            break;
        }
    }/* end of while(1) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT loglibInt_globChkMaxLogLvl(UINT lvl)
{
    if(lvl > LOGLIB_LVL_CNT){
        return LOGERR_INVLAID_LOG_LVL;
    }

    if(g_globCb.maxLogLvl[lvl] != 0){
        return RC_OK;
    }

    return RC_NOK;
}

FT_PUBLIC UINT loglibInt_globGetMaxLogLvl(VOID)
{
    return g_globCb.maxLogLvlVal;
}

