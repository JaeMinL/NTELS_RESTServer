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

FT_PUBLIC RT_RESULT tokenCacheDestroy()
{
	return RC_OK;
}

FT_PUBLIC RT_RESULT tokenCacheCheck(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen)
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

FT_PUBLIC RT_RESULT tokenNodeInsert(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen, UINT exp_in)
{
	TokenHashNode *tokenNode = NULL;
	SINT ret = RC_OK;

	if(tokenHashTbl == NULL || accTok == NULL || accTokLen == 0 || exp_in == 0)
	{
		LOGLIB_ERR("REST","TOKEN NODE INSERT FAILED\n");
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	tokenNode = comlib_memMalloc(sizeof(TokenHashNode));
	tokenNode->accTok = comlib_memMalloc(sizeof(CHAR) * accTokLen);
	comlib_strNCpy(tokenNode->accTok, accTok, accTokLen);
	tokenNode->exp_in = exp_in;

	tokenNode->hNode.key.key = tokenNode->accTok;
	tokenNode->hNode.key.keyLen = accTokLen;
	tokenNode->hNode.data = tokenNode;

	ret = comlib_hashTblInsertHashNode(tokenHashTbl, &tokenNode->hNode.key, &tokenNode->hNode);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN HASH NODE INSERT FAILED error(ret=%d)\n", ret);
		comlib_memFree(tokenNode->accTok);
		comlib_memFree(tokenNode);
	}

	// printf("hkey insert : %d\n", (UINT)tokenNode->hNode.key.hashKey % 300);
	// printf("key insert : %s\n", tokenNode->hNode.key.key);

	return RC_OK;
}

FT_PUBLIC RT_RESULT tokenCacheDel()
{
	return RC_OK;
}
