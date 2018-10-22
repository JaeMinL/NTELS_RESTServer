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

#define HASH_KEY_INDEX 0
#define UNAUTHORIZED 401

FT_PUBLIC RT_RESULT rss_tokenNodeCheck(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen)
{
	ComlibHashKey hKey;
	ComlibHashNode *hNode = NULL;
	SINT ret = RC_NOK;

	if(tokenHashTbl == NULL || accTok == NULL || accTokLen == 0)
	{
		LOGLIB_ERR("REST","TOKEN NODE FIND FAILED\n");
		return RC_NOK;
	}

	hKey.key = accTok;
	hKey.keyLen = accTokLen;

	ret = comlib_hashTblFindHashNode(tokenHashTbl, &hKey, HASH_KEY_INDEX, &hNode);
	
	return ret;
}

FT_PUBLIC RT_RESULT rss_tokenNodeInsert(RsvlibSesCb *sesCb, AuthMngMainCb *authMngMain, CHAR *accTok, UINT accTokLen, UINT exp_in)
{
	TokenNode *tokenNode = NULL;
	SINT ret = RC_OK;

	if(authMngMain == NULL || accTok == NULL || accTokLen == 0 || exp_in == 0)
	{
		LOGLIB_ERR("REST","TOKEN NODE INSERT FAILED\n");
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	/*insert Node in Hash Table*/
	tokenNode = comlib_memMalloc(sizeof(TokenNode));
	tokenNode->accTok = comlib_memMalloc(sizeof(CHAR) * accTokLen);
	comlib_strNCpy(tokenNode->accTok, accTok, accTokLen);
	tokenNode->exp_in = exp_in;

	tokenNode->hNode.key.key = tokenNode->accTok;
	tokenNode->hNode.key.keyLen = accTokLen;
	tokenNode->hNode.data = tokenNode;

	ret = comlib_hashTblInsertHashNode(&authMngMain->tokenHashTbl, &tokenNode->hNode.key, &tokenNode->hNode);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN HASH NODE INSERT FAILED error(ret=%d)\n", ret);
		comlib_memFree(tokenNode->accTok);
		comlib_memFree(tokenNode);
	}

	/*insert Node in Timer Table*/
	tokenNode->tNode.data = tokenNode;
	tokenNode->event = 0;
	tokenNode->ownMngMainCb = authMngMain;
	ret = comlib_timerTblStartTm(&authMngMain->tokenTmrTbl, &tokenNode->tNode, tokenNode->event, tokenNode->exp_in);
	//ret = comlib_timerTblStartTm(&authMngMain->tokenTmrTbl, &tokenNode->tNode, tokenNode->event, 10);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN TIMER NODE INSERT FAILED error(ret=%d)\n", ret);
		comlib_memFree(tokenNode->accTok);
		comlib_memFree(tokenNode);
	}

	// printf("hkey insert : %d\n", (UINT)tokenNode->hNode.key.hashKey % 300);
	// printf("key insert : %s\n", tokenNode->hNode.key.key);

	return RC_OK;
}

FT_PUBLIC RT_RESULT rss_tokenNodeDel(ComlibHashTbl *tokenHashTbl, ComlibHashNode *hNode)
{
	SINT ret = RC_OK;
	TokenNode* tokNode = NULL;

	if(tokenHashTbl == NULL || hNode == NULL)
	{
		LOGLIB_ERR("REST","TOKEN NODE INSERT FAILED\n");
		return RC_NOK;
	}
	tokNode = (TokenNode*)hNode->data;

	ret = comlib_hashTblDelHashNode(tokenHashTbl, hNode);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN HASH NODE DELETE FAILED error(ret=%d)\n", ret);
		return ret;
	}

	comlib_memFree(tokNode->accTok);
	comlib_memFree(tokNode);

	return ret;
}


FT_PUBLIC RT_RESULT rss_tokenNodeDelAll(AuthMngMainCb* authMngMainCb)
{
	SINT ret = RC_OK;
	UINT i = 0, j = 0;

	ComlibHashTbl* tokenHashTbl = NULL;
	ComlibTmrTbl* tokenTmrTbl = NULL;
	TokenNode* tokNode = NULL;

	if(authMngMainCb == NULL){
		return RC_NOK;
	}

	tokenHashTbl = &authMngMainCb->tokenHashTbl;
	tokenTmrTbl = &authMngMainCb->tokenTmrTbl;

	pthread_rwlock_wrlock(&authMngMainCb->tokenMainMutx);
	for(i = 0; i < tokenHashTbl->nmbEntry; i++)
	{
		for(j = 0; j < tokenHashTbl->entry[i].nodeCnt; j++)
		{
			tokNode = (TokenNode *)tokenHashTbl->entry[i].first->data;
			ret = comlib_timerTblCancelTm(tokenTmrTbl, &tokNode->tNode);
			if(ret != RC_OK)
			{
				pthread_rwlock_unlock(&authMngMainCb->tokenMainMutx);
				return ret;
			}
			ret = rss_tokenNodeDel(tokenHashTbl, tokenHashTbl->entry[i].first);
			if(ret != RC_OK)
			{
				pthread_rwlock_unlock(&authMngMainCb->tokenMainMutx);
				return ret;
			}
		}
	}
	pthread_rwlock_unlock(&authMngMainCb->tokenMainMutx);
	return ret;
}





