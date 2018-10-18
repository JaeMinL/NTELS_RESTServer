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
#include "rlylib.h"
#include "rlylib.x"
#include "rest_stat.h"

#define UNAUTHORIZED 401
#define HDR_LEN 300
#define HTTP_OK "HTTP/1.1 200 OK\r\n"
#define AUTH_URL "http://192.168.6.89:8090/v1/auth/token"

typedef struct resHeaderDat resHeaderDat;

struct resHeaderDat
{
	size_t size;
	CHAR data[HDR_LEN];
	//CHAR *data;
};

FT_PRIVATE size_t header_callback(CHAR* header, size_t size, size_t nitems, resHeaderDat *dat)
{
	dat->size += (size * nitems);
	if(dat->data[0] == '\0')
	{
		snprintf(dat->data, (UINT)nitems + 1, "%s", header);
	}

	return size * nitems;
}


FT_PUBLIC RT_RESULT userAuth(RsvlibSesCb *sesCb, AuthMngCb *authMngCb)
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
	resHeaderDat resDat = {0, "\0"};
	accTokHeader[0] = '\0';

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
	ret = tokenCacheCheck(sesCb, &authMngMain->tokenHashTbl, accTok, accTokLen);
	if(ret == RC_OK)
	{
		LOGLIB_NOTY("REST","Access Token exit in Token Cache\n");
		return RC_OK;
	}

	/*토큰 없으면*/
	LOGLIB_NOTY("REST","Authentication Request to Auth Server\n");
	headers = curl_slist_append(headers, accTokHeader);

	CURL *hnd = curl_easy_init();
	if(!hnd)
	{
		LOGLIB_ERR("REST","curl_easy_init() error(ret=%d)\n", CURLE_FAILED_INIT);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(hnd, CURLOPT_URL, AUTH_URL);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(hnd, CURLOPT_HEADERDATA, &resDat);
	retCurl = curl_easy_perform(hnd);
	if(retCurl != CURLE_OK)
	{
		curl_slist_free_all(headers);
		curl_easy_cleanup(hnd);
		LOGLIB_ERR("REST","curl_easy_perform() error(ret=%d)\n", retCurl);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	curl_easy_cleanup(hnd);
	curl_slist_free_all(headers);

	if(comlib_strNCmp(resDat.data, HTTP_OK, comlib_strGetLen(HTTP_OK)))
	{
		LOGLIB_ERR("REST","OAuth server response: %s\n", resDat.data);
		rsvlib_apiSetStaCode(sesCb, UNAUTHORIZED);
		return RC_NOK;
	}

	/*mutex write lock*/
	pthread_rwlock_wrlock(&authMngMain->tokenMainMutx);

	/*토큰 insert*/
	tokenNodeInsert(sesCb, &authMngMain->tokenHashTbl, "abcdefghijklmn", 14, 3600);
	ret = tokenNodeInsert(sesCb, &authMngMain->tokenHashTbl, accTok, accTokLen, 3600);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","Token Node Insert Failed\n");
	}

	/*mutex write unlock*/
	pthread_rwlock_unlock(&authMngMain->tokenMainMutx);

	return RC_OK;
}





