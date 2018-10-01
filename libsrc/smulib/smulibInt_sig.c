#include <unistd.h>
#include <stdio.h>
#include "signal.h"

#include "gendef.h"

#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "rlylib.h"
#include "rlylib.x"

#include "smd_user.h"
#include "smd_user.x"
#include "smulib.h"
#include "smulib.x"
#include "smulibInt.h"
#include "smulibInt.x"

FT_PRIVATE VOID smulibInt_sigMain(SINT sigNo);

FT_PRIVATE VOID smulibInt_sigMain(SINT sigNo)
{
    SINT ret = RC_OK;
    ComlibHashNode *hNode = NULL;
    ComlibHashKey hKey;
    SmulibIntSig *sig = NULL;
    SmulibIntSigInfo *sigInfo = NULL;

    sigInfo = smulibInt_globGetSigInfo();

    thrlib_mutxLock(&sigInfo->mutx);

    /* run signal function */
    hKey.key = &sigNo;
    hKey.keyLen = sizeof(sigNo);

    ret = comlib_hashTblFindHashNode(&sigInfo->sigHT, &hKey, 0, &hNode);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Signal function not found(ret=%d, sigNo=%d)\n",ret, sigNo);
        return;
    }

    sig = hNode->data;

    sig->func(sigNo, sig->usrArg);

    thrlib_mutxUnlock(&sigInfo->mutx);

    return;
}

FT_PUBLIC RT_RESULT smulibInt_sigReg(UINT sigNo, SmulibSigFunc func, VOID *usrArg)
{
    SINT ret = RC_OK;
    ComlibHashKey hKey;
    ComlibHashNode *hNode = NULL;
    SmulibIntSig *sig = NULL;
    SmulibIntSigInfo *sigInfo = NULL;

    sigInfo = smulibInt_globGetSigInfo();

    thrlib_mutxLock(&sigInfo->mutx);

    hKey.key = &sigNo;
    hKey.keyLen = sizeof(sigNo);

    ret = comlib_hashTblFindHashNode(&sigInfo->sigHT, &hKey, 0, &hNode);
    if(ret == RC_OK){
        SMU_LOG(SMU_ERR,"Signal already registered(ret=%d)\n",ret);
        return SMUERR_SIG_ALREADY_REG;
    }

    sig = comlib_memMalloc(sizeof(SmulibIntSig));

    sig->sigNo = sigNo;
    sig->func = func;

    sig->hNode.data =sig;
    sig->usrArg = usrArg;

    sig->hNode.key.key = &sig->sigNo;
    sig->hNode.key.keyLen = sizeof(sig->sigNo);

    ret = comlib_hashTblInsertHashNode(&sigInfo->sigHT, &sig->hNode.key, &sig->hNode);
    if(ret != RC_OK){
        SMU_LOG(SMU_ERR,"Signal control block reg faield(ret=%d)\n",ret);
        return SMUERR_SIG_REG_FAILED;
    }

    thrlib_mutxUnlock(&sigInfo->mutx);

    signal(sigNo, (VOID*)smulibInt_sigMain);

    return RC_OK;
}

