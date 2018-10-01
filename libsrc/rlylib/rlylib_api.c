#include <unistd.h>

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

/* initialization */
FT_PUBLIC RT_RESULT rlylib_apiInitGlob()
{
	SINT ret = RC_OK;

	ret = rlylibInt_globInit();

	return ret;
}

FT_PUBLIC RT_RESULT rlylib_apiSetLogFunc(UINT lvl, RlylibLogFunc logFunc)
{
	SINT ret = RC_OK;

	if(logFunc == NULL){
		return RLYERR_LOG_FUNC_NULL;
	}

	if(lvl > RLY_DBG){
		return RLYERR_INVALID_LOG_LEVEL;
	}

	ret = rlylibInt_globSetLogFunc(lvl, logFunc);

	return ret;
}


FT_PUBLIC RT_RESULT rlylib_apiInitRlylibCb(RlylibCb *rlylibCb, UINT rlyType, CHAR *locHost, RlylibOptArg *optArg)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibInt_globGetInitFlg() != RC_TRUE){
		RLY_LOG(RLY_ERR,"Global Control block init first\n");
		return RLYERR_GLOB_CB_NOT_INIT;
	}

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = comlib_memMalloc(sizeof(RlylibIntMainCb));
	if(intMainCb == NULL){
		RLY_LOG(RLY_ERR,"Internal main control blcok alloc failed\n");
		return RLYERR_MIAN_CB_ALLOC_FAILED;
	}

	ret = rlylibInt_initMainCb(intMainCb, rlyType, locHost, optArg);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"init failed(ret=%d)\n",ret);
		return ret;
	}

	rlylibCb->main = (RlylibMainCb)intMainCb;

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiDstryRlylibCb(RlylibCb *rlylibCb)
{
    SINT ret = RC_OK;
    RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

    ret = rlylibInt_dstryMainCb(intMainCb);
    if(ret != RC_OK){
        RLY_LOG(RLY_ERR,"Relay control block destory failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

#if 0
/* util */
FT_PUBLIC RT_RESULT rlylib_apiNtop(UINT afnum, CONST VOID *src, CHAR *dst, UINT size)
{
	SINT ret = RC_OK;

	ret = trnlib_utilNtop(afnum, src, dst, size);

	return ret;
}

FT_PUBLIC RT_RESULT rlylib_apiPton(UINT afnum, CONST CHAR *src, VOID *dst)
{
	SINT ret = RC_OK;

	ret = trnlib_utilPton(afnum, src, dst);

	return ret;
}
#endif

/* control */
FT_PUBLIC RT_RESULT rlylib_apiAddConn(RlylibCb *rlylibCb, CHAR *host, TrnlibTransAddr *dstAddrs, UINT dstAddrCnt)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostAddConnCb(intMainCb, host, dstAddrs, dstAddrCnt);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"connction control block add failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiAddHost(RlylibCb *rlylibCb, CHAR *host, UINT type, UINT hashId, RlylibHostOptArg *hostOptArg , UINT *rt_hostId)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostAddHost(intMainCb, host, type, hashId, hostOptArg, rt_hostId);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"host add failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiDelHost(RlylibCb *rlylibCb, CHAR *host)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relya control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostDelHost(intMainCb, host);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"Host delete failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiSetPriHost(RlylibCb *rlylibCb, CHAR *host)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relya control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSetPriHostCb(intMainCb, host);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"Primary host setting failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiAddAcptLst(RlylibCb *rlylibCb, TrnlibTransAddr *srcAddr)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_acptAddAcptLst(intMainCb, srcAddr);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"acpt list add failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiGetHostSta(RlylibCb *rlylibCb, CHAR *host, UINT *rt_sta)
{
    return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiAddRlm(RlylibCb *rlylibCb, CHAR *realm, UINT rule, RlylibRlmOptArg *rlmOptArg)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_rlmAddRlm(intMainCb, realm, rule, rlmOptArg);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"realm add failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiRegHostInRlm(RlylibCb *rlylibCb, CHAR *realm, CHAR *host, BOOL priFlg /* is primary */, 
		RlylibRlmKey *rlmKey)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_rlmRegHost(intMainCb, realm, host, priFlg, rlmKey);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"realm add failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiRlmDeregHostInRlm(RlylibCb *rlylibCb, CHAR *realm, CHAR *host)
{
	return RC_OK;
}

/* transport */
FT_PUBLIC RT_RESULT rlylib_apiSndFixMsgToRlm(RlylibCb *rlylibCb, CHAR *realm, RlylibRlmKey *rlmKey, 
		CHAR *buf, UINT bufLen, CHAR *rt_host, UINT *rt_hostId)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_rlmSndFixMsgToRlm(intMainCb, realm, rlmKey, buf, bufLen, rt_host, rt_hostId);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"Message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiSndFixMsgToHost(RlylibCb *rlylibCb, CHAR *host, CHAR *buf, UINT bufLen)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSndMsgToHost(intMainCb, host, RC_FALSE, buf, bufLen);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiSndFixMsgToPri(RlylibCb *rlylibCb, CHAR *buf, UINT bufLen, BOOL faFlg /* failover flag */, CHAR *rt_host, UINT *rt_hostId)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSndFixMsgToPri(intMainCb, buf, bufLen, faFlg, rt_host, rt_hostId);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;

}

FT_PUBLIC RT_RESULT rlylib_apiSndFixMsgToAny(RlylibCb *rlylibCb, CHAR *buf, UINT bufLen, CHAR *rt_host, UINT *rt_hostId)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSndFixMsgToAny(intMainCb, buf, bufLen, rt_host, rt_hostId);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiSndDynMsgToHostId(RlylibCb *rlylibCb, UINT hostId, CHAR *buf, UINT bufLen)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSndMsgToHostId(intMainCb, hostId, RC_TRUE, buf, bufLen);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}


FT_PUBLIC RT_RESULT rlylib_apiSndFixMsgToHostId(RlylibCb *rlylibCb, UINT hostId, CHAR *buf, UINT bufLen)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_hostSndMsgToHostId(intMainCb, hostId, RC_FALSE, buf, bufLen);
	if(ret != RC_OK){
		RLY_LOG(RLY_ERR,"message send failed(ret=%d)\n",ret);
		return ret;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT rlylib_apiRcvDynMsgFromHost(RlylibCb *rlylibCb, CHAR *host, CHAR **rt_buf, UINT *rt_bufLen)
{
	SINT ret = RC_OK;
	RlylibIntRcvMsgRslt rcvMsgRslt;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	rcvMsgRslt.rt_buf = NULL;
	rcvMsgRslt.allocBufLen= 0;
	rcvMsgRslt.rt_bufLen = 0;

	ret = rlylibInt_hostRcvMsgFromHost(intMainCb, host, RC_FALSE, &rcvMsgRslt);
	if(ret != RC_OK){
		if((ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
			RLY_LOG(RLY_ERR,"Message receive failed(ret=%d)\n",ret);
		}

		*rt_buf = rcvMsgRslt.rt_buf;
		*rt_bufLen = rcvMsgRslt.rt_bufLen;
		return ret;
	}

	*rt_buf = rcvMsgRslt.rt_buf;
	*rt_bufLen = rcvMsgRslt.rt_bufLen;

	return ret;
}


FT_PUBLIC RT_RESULT rlylib_apiRcvFixMsgFromHost(RlylibCb *rlylibCb, CHAR *host, CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen)
{
	SINT ret = RC_OK;
	RlylibIntRcvMsgRslt rcvMsgRslt;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	rcvMsgRslt.rt_buf = rt_buf;
	rcvMsgRslt.allocBufLen = maxBufLen;
	rcvMsgRslt.rt_bufLen = 0;

	ret = rlylibInt_hostRcvMsgFromHost(intMainCb, host, RC_TRUE, &rcvMsgRslt);
	if(ret != RC_OK){
		if((ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
			RLY_LOG(RLY_ERR,"Message receive failed(ret=%d)\n",ret);
		}
		return ret;

	}
	*rt_bufLen = rcvMsgRslt.rt_bufLen;

	return ret;
}

FT_PUBLIC RT_RESULT rlylib_apiRcvDynMsgFromAny(RlylibCb *rlylibCb, CHAR *rt_host, UINT *rt_hostId, CHAR **rt_buf, UINT *rt_bufLen)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;
	RlylibIntRcvMsgRslt rcvMsgRslt;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	rcvMsgRslt.rt_buf = NULL;
	rcvMsgRslt.allocBufLen = 0;
	rcvMsgRslt.rt_bufLen = 0;

	ret = rlylibInt_hostRcvMsgFromAny(intMainCb, rt_host, rt_hostId, RC_TRUE, &rcvMsgRslt);
	if(ret != RC_OK){
		if((ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
			RLY_LOG(RLY_ERR,"Message receive failed(ret=%d)\n",ret);
		}

		*rt_buf = rcvMsgRslt.rt_buf;
		*rt_bufLen = rcvMsgRslt.rt_bufLen;
		return ret;
	}

	*rt_buf = rcvMsgRslt.rt_buf;
	*rt_bufLen = rcvMsgRslt.rt_bufLen;

	return ret;
}


FT_PUBLIC RT_RESULT rlylib_apiRcvFixMsgFromAny(RlylibCb *rlylibCb, CHAR *rt_host, UINT *rt_hostId, CHAR *rt_buf, UINT maxBufLen, 
		UINT *rt_bufLen)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;
	RlylibIntRcvMsgRslt rcvMsgRslt;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	rcvMsgRslt.rt_buf = rt_buf;
	rcvMsgRslt.allocBufLen = maxBufLen;
	rcvMsgRslt.rt_bufLen = 0;

	ret = rlylibInt_hostRcvMsgFromAny(intMainCb, rt_host, rt_hostId, RC_FALSE, &rcvMsgRslt);
	if(ret != RC_OK){
		if((ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
			RLY_LOG(RLY_ERR,"Message receive failed(ret=%d)\n",ret);
		}

		*rt_bufLen = rcvMsgRslt.rt_bufLen;
		return ret;
	}

	*rt_bufLen = rcvMsgRslt.rt_bufLen;

	return ret;
}

FT_PUBLIC RT_RESULT rlylib_apiRcvFixMsgFromRlm(RlylibCb *rlylibCb, CHAR *realm, CHAR *rt_buf, UINT maxBufLen, UINT *rt_bufLen, 
		CHAR *rt_host, UINT *rt_hostId)
{
	SINT ret = RC_OK;
	RlylibIntMainCb *intMainCb = NULL;

	if(rlylibCb == NULL){
		RLY_LOG(RLY_ERR,"Relay control block is null\n");
		return RLYERR_NULL;
	}

	intMainCb = (RlylibIntMainCb*)rlylibCb->main;

	ret = rlylibInt_rlmRcvFixMsgFromRlm(intMainCb, realm, rt_buf, maxBufLen, rt_bufLen, rt_host, rt_hostId);
	if(ret != RC_OK){
		if((ret != RLYERR_MSG_NOT_EXIST) && (ret != RLYERR_DROP_MSG)){
			RLY_LOG(RLY_ERR,"Message receive failed(ret=%d)\n",ret);
		}
		return ret;
	}

	return ret;
}
