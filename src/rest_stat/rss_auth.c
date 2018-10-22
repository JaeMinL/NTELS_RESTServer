#include <stdio.h>
#include <curl/curl.h>

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
#define UNAUTHORIZED 401
#define HTTP_OK 200
#define HDR_LEN 300
#define AUTH_URL "http://192.168.6.89:8090/v1/auth/token"

RT_RESULT rss_tmrEvtFunc(UINT event, VOID *data);

RT_RESULT rss_tmrEvtFunc(UINT event, VOID *data)
{
	SINT ret = RC_OK;
	if(event == 0)
	{
		/*timer에서 지우기*/
		TokenNode* tokNode = (TokenNode *)data;
		LOGLIB_NOTY("REST", "Token Node : %s REMOVE\n", tokNode->accTok);

		pthread_rwlock_wrlock(&tokNode->ownMngMainCb->tokenMainMutx);

		ret = comlib_timerTblCancelTm(&tokNode->ownMngMainCb->tokenTmrTbl, &tokNode->tNode);
		if(ret != RC_OK)
		{
			pthread_rwlock_unlock(&tokNode->ownMngMainCb->tokenMainMutx);
			return ret;
		}
		ret = rss_tokenNodeDel(&tokNode->ownMngMainCb->tokenHashTbl, &tokNode->hNode);
		if(ret != RC_OK)
		{
			pthread_rwlock_unlock(&tokNode->ownMngMainCb->tokenMainMutx);
			return ret;
		}

		pthread_rwlock_unlock(&tokNode->ownMngMainCb->tokenMainMutx);
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rss_authInit(AuthMngCb* authMngCb)
{
	SINT ret = RC_OK;

	if(authMngCb == NULL)
	{
		LOGLIB_ERR("REST","authentication control block is null(ret=%d)\n", ret);
	}

	ret = rss_authInitMain(&authMngCb->main);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","init failed(ret=%d)\n", ret);
		return RC_NOK;
	}

	return RC_OK;
}



FT_PUBLIC RT_RESULT rss_authInitMain(AuthMngMainCb* authMngMainCb)
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
		LOGLIB_ERR("REST","TOKEN CACHE HASH TABLE INIT FAILED(ret=%d)\n", ret);
		comlib_memFree(authMngMainCb);
		return RC_NOK;
    }

	if(pthread_rwlock_init(&authMngMainCb->tokenMainMutx, NULL) != 0)
	{
		LOGLIB_ERR("REST", "main mutex init failed\n");
		comlib_memFree(authMngMainCb);
		return RC_NOK;
	}

	comlib_timerInit(&authMngMainCb->tmr, COM_TIMER_TYPE_SEC);
	ret = comlib_timerTblInit(&authMngMainCb->tokenTmrTbl, &authMngMainCb->tmr, rss_tmrEvtFunc);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN CACHE TIMER INIT FAILED(ret=%d)\n", ret);
		comlib_memFree(&authMngMainCb->tmr);
		comlib_memFree(authMngMainCb);
		return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT rss_authDstry(AuthMngCb* authMngCb)
{
	SINT ret = RC_OK;

	if(authMngCb == NULL)
	{
		LOGLIB_ERR("REST","authentication control block is null(ret=%d)\n", ret);
	}

	ret = rss_authDstryMain(&authMngCb->main);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","destroy failed(ret=%d)\n", ret);
		return RC_NOK;
	}

	return RC_OK;
}


FT_PUBLIC RT_RESULT rss_authDstryMain(AuthMngMainCb* authMngMainCb)
{
	SINT ret = RC_OK;
	if(authMngMainCb == NULL)
	{
		LOGLIB_ERR("REST","authentication main control block is null(ret=%d)\n", ret);
	}

	if(pthread_rwlock_destroy(&authMngMainCb->tokenMainMutx) != 0)
	{
		LOGLIB_ERR("REST", "main mutex init failed\n");
		comlib_memFree(authMngMainCb);
	}

	rss_tokenNodeDelAll(authMngMainCb);
	ret = comlib_hashTblDstry(&authMngMainCb->tokenHashTbl);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","TOKEN CACHE DESTROY FAILED(ret=%d)\n", ret);
		comlib_memFree(authMngMainCb);
		return RC_NOK;
    }

    comlib_memFree(authMngMainCb);

	return RC_OK;
}

FT_PUBLIC RT_RESULT rss_authSvr(RsvlibSesCb *sesCb, AuthMngCb *authMngCb)
{
	SINT ret = RC_NOK;
	CURLcode retCurl = CURLE_FAILED_INIT;
	CHAR *keyTok = "Authorization";
	UINT keyTokLen = comlib_strGetLen(keyTok);
	CHAR *accTok = NULL;
	CHAR accTokHeader[HDR_LEN];
	UINT accTokLen = 0;
	UINT i=0;
	struct curl_slist *headers = NULL;

	AuthMngMainCb *authMngMain = NULL;
	accTokHeader[0] = '\0';
	CURL *curl = NULL;
	long res_code = 0;

	if(sesCb == NULL || authMngCb == NULL)
	{
		LOGLIB_ERR("REST","userAuth parameter is null(ret=%d)\n", ret);
		return RC_NOK;
	}

	authMngMain = &authMngCb->main;

	ret = rsvlib_apiFindHdr(sesCb, keyTok, keyTokLen, 0, &accTok, &accTokLen);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST"," 'Authorization' header not found error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	i = snprintf(accTokHeader, keyTokLen + accTokLen + 4, "%s: %s", keyTok, accTok);
	if(i<0 || i>= keyTokLen + accTokLen + 4)
	{
		LOGLIB_ERR("REST", "snprintf() make authorization header error\n");
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	/*토큰 캐시에 있는지 확인*/
	ret = rss_tokenNodeCheck(sesCb, &authMngMain->tokenHashTbl, accTok, accTokLen);
	if(ret == RC_OK)
	{
		LOGLIB_NOTY("REST","Access Token exit in Token Cache\n");
		return RC_OK;
	}

	/*토큰 없으면*/
	LOGLIB_NOTY("REST","Authentication Request to Auth Server\n");
	headers = curl_slist_append(headers, accTokHeader);

	curl = curl_easy_init();
	if(!curl)
	{
		LOGLIB_ERR("REST","curl_easy_init() error(ret=%d)\n", CURLE_FAILED_INIT);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(curl, CURLOPT_URL, AUTH_URL);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	retCurl = curl_easy_perform(curl);
	if(retCurl != CURLE_OK)
	{
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		LOGLIB_ERR("REST","curl_easy_perform() error(ret=%d)\n", retCurl);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	if(res_code != HTTP_OK)
	{
		LOGLIB_ERR("REST","OAuth server response: %ld\n", res_code);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	/*mutex write lock*/
	pthread_rwlock_wrlock(&authMngMain->tokenMainMutx);

	/*토큰 insert (HashTable & TimerTbl)*/
	ret = rss_tokenNodeInsert(sesCb, authMngMain, accTok, accTokLen, 10);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","Token Node Insert Failed\n");
	}

	/*mutex write unlock*/
	pthread_rwlock_unlock(&authMngMain->tokenMainMutx);

	return RC_OK;
}
