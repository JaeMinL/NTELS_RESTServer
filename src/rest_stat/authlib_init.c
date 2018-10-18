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
#include "rrllib.h"
#include "rrllib.x"
#include "rsvlib.h"
#include "rsvlib.x"
#include "rest_stat.h"

#define BUKET_CNT 300

FT_PUBLIC RT_RESULT initAuthMngCb(AuthMngCb* authMngCb);
FT_PUBLIC RT_RESULT initAuthMngMainCb(AuthMngMainCb* authMngMainCb);


FT_PUBLIC RT_RESULT initAuthMngCb(AuthMngCb* authMngCb)
{
	SINT ret = RC_OK;

	if(authMngCb == NULL)
	{
		LOGLIB_ERR("REST","authentication main control block is null(ret=%d)\n", ret);
	}

	ret = initAuthMngMainCb(&authMngCb->main);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","init failed(ret=%d)\n", ret);
		return RC_NOK;
	}

	return RC_OK;
}



FT_PUBLIC RT_RESULT initAuthMngMainCb(AuthMngMainCb* authMngMainCb)
{
	SINT ret = RC_OK;

	if(authMngMainCb == NULL)
	{
		LOGLIB_ERR("REST","authentication main control block is null(ret=%d)\n", ret);
	}

	/* Token Cache hash table setting*/
	ret = comlib_hashTblInit(&authMngMainCb->tokenHashTbl, BUKET_CNT, RC_FALSE, COM_HASH_TYPE_STRING, NULL);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN CACHE INIT FAILED(ret=%d)\n", ret);
		comlib_memFree(authMngMainCb);
		return RC_NOK;
    }

	if(pthread_rwlock_init(&authMngMainCb->tokenMainMutx, NULL) != 0){
		LOGLIB_ERR("REST", "main mutex init failed\n");
		comlib_memFree(authMngMainCb);
		return RC_NOK;
	}

    return RC_OK;
}
