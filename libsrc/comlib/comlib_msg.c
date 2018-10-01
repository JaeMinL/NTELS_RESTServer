#include "gendef.h"

#include "comlib.h"
#include "comlib.x"

/* Private structure */
typedef struct MsgCfg 			MsgCfg;
typedef struct MsgInfo 			MsgInfo;
typedef struct MsgHdr 			MsgHdr;

struct MsgCfg{
	UINT 	bufLen;			/* buffer chain length */
	UINT 	maxLen;			/* max message length */
};

/* message information (for debug) */
struct MsgInfo{
	UINT				evnt;		/* message type (currently not used) */
	struct timespec     tm;			/* allociate time */
	VOID				*allocAddr; /* allociate address line */
};

/* message information */
struct MsgHdr{
	UINT				len;		/* total message legnth */
	ComlibMsg			*endChk;	/* end of message Chunk*/
	ComlibMsg			*curChk;	/* read cursor chunk */
	ComlibData			*cur;		/* read cursor */
#if 0
	ComlibMsgInfo		info;		/* message information (for debug) */
#endif
};

STATIC MsgCfg g_msgCfg = {
	COM_MSG_BUF_CHAIN_LEN,
	COM_MSG_MAX_LEN
};

/* Private mecro */
#define ALLOC_NEW_CHK(chk, len){\
	(chk) = comlib_memMalloc((len));\
	if((chk) == NULL){\
		COM_LOG(COM_ERR,"Message allocate failed\n");\
		return COMERR_MSG_ALLOC_FAIL;\
	}\
	(chk)->bufSize = 0;\
	(chk)->base = (ComlibData*)(chk) + sizeof(ComlibMsg);\
	(chk)->lim = (ComlibData*)(chk) + (len);\
}

#define COPY_DATA_INTO_CHK(new, sCur, data, size){\
	(new)->sPtr = (sCur);\
	(new)->ePtr = (sCur) + (size);\
	(new)->bufSize += (size);\
	comlib_memMemcpy((sCur), (data), (size));\
}

#define FIND_MSG_IDX(msg, idx){\
	while((idx)){\
		if((msg)->bufSize > (idx)){\
			break;\
		}\
		(idx) -= (msg)->bufSize;\
		(msg) = (msg)->next;\
	}\
}

#define CALC_CHK_CNT(idx, chkCnt){\
	UINT d_cnt = 0;\
	UINT d_bufLen = 0;\
	d_bufLen = g_msgCfg.bufLen - sizeof(ComlibMsg);\
	while((idx)){\
		if((idx) < d_bufLen){\
			break;\
		}\
	 	(idx) -= d_bufLen;\
		d_cnt++;\
	}\
	(chkCnt) = d_cnt;\
}

#define MAKE_NEW_CHK(msgHdr, curChk, data, len){\
	UINT d_newChkIdx = 0;\
	UINT d_newChkCnt = 0;\
	ComlibMsg *d_tmpMsg = NULL;\
	/* calc make chunk count */\
	d_newChkIdx = (len);\
	CALC_CHK_CNT(d_newChkIdx, d_newChkCnt);\
	/* create new chunks */\
	while(d_newChkCnt){\
		ALLOC_NEW_CHK(d_tmpMsg, g_msgCfg.bufLen);\
		COPY_DATA_INTO_CHK(d_tmpMsg, d_tmpMsg->base, (data), chkBufLen);\
		(data) += d_tmpMsg->bufSize;\
		d_tmpMsg->prev = (curChk);\
		d_tmpMsg->next = (curChk)->next;\
		(curChk)->next = d_tmpMsg;\
		if(d_tmpMsg->next != NULL){\
			d_tmpMsg->next->prev = d_tmpMsg;\
		}\
		(curChk) = d_tmpMsg;\
		d_newChkCnt--;\
	}/* end of while(newChkCnt) */\
	/* make remain msg */\
	if(d_newChkIdx != 0){\
		ALLOC_NEW_CHK(d_tmpMsg, g_msgCfg.bufLen);\
		COPY_DATA_INTO_CHK(d_tmpMsg, d_tmpMsg->base, (data), d_newChkIdx);\
		d_tmpMsg->prev = (curChk);\
		d_tmpMsg->next = (curChk)->next;\
		(curChk)->next = d_tmpMsg;\
		if(d_tmpMsg->next != NULL){\
			d_tmpMsg->next->prev = d_tmpMsg;\
		}\
	}/* if(newChkIdx != 0) */\
	if(d_tmpMsg != NULL && d_tmpMsg->next == NULL){\
		msgHdr->endChk= d_tmpMsg;\
	}\
}\

#define SPLIT_MSG(msgHdr, curChk, chkCur, msg){\
	ComlibMsg *d_tmpMsg = NULL;\
	UINT d_first = 0;\
	UINT d_last = 0;\
	if((d_first = (chkCur) - (curChk)->sPtr) > (d_last = (curChk)->ePtr - (chkCur))){\
		/* first is big, split end msg*/\
		if(d_last != 0){\
			ALLOC_NEW_CHK(d_tmpMsg, g_msgCfg.bufLen);\
			COPY_DATA_INTO_CHK(d_tmpMsg, d_tmpMsg->base, (chkCur), d_last);\
			d_tmpMsg->next = (curChk)->next;\
			d_tmpMsg->prev = (curChk);\
			(curChk)->next = d_tmpMsg;\
			if(d_tmpMsg->next != NULL){\
				d_tmpMsg->next->prev = d_tmpMsg;\
			}\
			else{\
				(msgHdr)->endChk = d_tmpMsg;\
			}\
			(curChk)->bufSize = d_first;\
			(curChk)->ePtr = (chkCur);\
			(msg) = d_tmpMsg;\
		}\
		else {\
			(msg) = (curChk)->next;\
		}\
	}\
	else {\
		if(d_first != 0){\
			/* last is big, spit first msg */\
			ALLOC_NEW_CHK(d_tmpMsg, g_msgCfg.bufLen);\
			COPY_DATA_INTO_CHK(d_tmpMsg, d_tmpMsg->base, (curChk)->sPtr, d_first);\
			d_tmpMsg->next = (curChk);\
			d_tmpMsg->prev = (curChk)->prev;\
			(curChk)->prev = d_tmpMsg;\
			if(d_tmpMsg->next != NULL){\
				d_tmpMsg->prev->next = d_tmpMsg;\
			}\
			(curChk)->sPtr = (chkCur);\
			(curChk)->bufSize = d_last;\
			/* set current chunk */\
			(msg) = (curChk);\
			(curChk) = d_tmpMsg;\
		}\
		else {\
			(msg) = (curChk);\
			(curChk) = (curChk)->prev;\
		}\
	}/* end of else */\
}

FT_PUBLIC RT_RESULT comlib_msgMsgInit()
{
	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgGetMsg(ComlibMsg **msg)
{
    UINT size = 0;
    MsgHdr *msgHdr = NULL;

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    /* init msg */
    size = sizeof(ComlibMsg) + sizeof(MsgHdr);

    *msg = comlib_memMalloc(size);

    msgHdr = (MsgHdr*)((ComlibData*)(*msg) + sizeof(ComlibMsg));
    (*msg)->next = NULL;
    (*msg)->prev = NULL;
    (*msg)->bufSize = sizeof(MsgHdr);
    (*msg)->sPtr = (ComlibData*)msgHdr;
    (*msg)->ePtr = (ComlibData*)(*msg)->sPtr + sizeof(MsgHdr);
    (*msg)->base = (ComlibData*)msgHdr;
    (*msg)->lim = (ComlibData*)((*msg)->ePtr);

    msgHdr->len = 0;
    msgHdr->endChk = NULL;
    msgHdr->cur = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgPutMsg(ComlibMsg **msg)
{
    MsgHdr *msgHdr = NULL;

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)((*msg)->sPtr);
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    /* check max length */
    if(msgHdr->len != 0){
        ComlibMsg *curChk = NULL;
        ComlibMsg *tmp = NULL;

        curChk = (*msg)->next;

        /* free all message */
        while(curChk){
            tmp = curChk;
            curChk = curChk->next;

            comlib_memFree(tmp);
        }/* end of while(curChk) */
    }/* end of if(msgHdr->len != 0) */

    comlib_memFree((*msg));

    (*msg) = NULL;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgGetMsgLen(ComlibMsg **msg, UINT *len)
{
    MsgHdr *msgHdr = NULL;

    /* check message */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    *len = msgHdr->len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgAddCharFirst(ComlibMsg **msg, CHAR data)
{
    ComlibMsg *tmp = NULL;
    MsgHdr *msgHdr = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    /* check max length */
    if(msgHdr->len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is full(len=%u, max=%u)\n",msgHdr->len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_IS_FULL;
    }

    /* insert char data */
    tmp = (*msg)->next;

    /* check data */
    if(tmp == NULL || tmp->sPtr == tmp->base){
        ComlibMsg *new = NULL;

        ALLOC_NEW_CHK(new, g_msgCfg.bufLen);

        /* init new chunk */
        new->ePtr = new->lim;
        new->sPtr = new->lim;

        /* set chain */
        (*msg)->next = new;
        new->prev = (*msg);

        /* if first data */
        if(msgHdr->endChk == NULL){
            msgHdr->endChk = new;
            new->next = NULL;
        }
        else {
            new->next = tmp;
        }

        tmp = new;
    }/* end of if(tmp == NULL || (*tmp)->sPtr == (*msg)->base) */

    tmp->sPtr--;

    /* insert data */
    *tmp->sPtr = data;
    tmp->bufSize++;
    msgHdr->len++;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgAddCharEnd(ComlibMsg **msg, CHAR data)
{
    ComlibMsg *tmp = NULL;
    MsgHdr *msgHdr = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    tmp = (*msg)->next;
    if(tmp == NULL){
        COM_LOG(COM_ERR,"Can not find msg chunk\n");
        return COMERR_MSG_CAN_NOT_FIND_CHK;
    }

    if(tmp->bufSize == 1){
        /* free message chunk */
        (*msg)->next = tmp->next;

        if(tmp->next == NULL){
            msgHdr->endChk = NULL;
        }
        else{
            tmp->next->prev = (*msg);
        }

        comlib_memFree(tmp);
    }
    else {
        /* delete char */
        tmp->sPtr++;
        tmp->bufSize--;
    }

    msgHdr->len--;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgAddMsgFirst(ComlibMsg **msg, CHAR *data, UINT len)
{
    UINT cpyTotLen = 0;
    UINT cpyLen = 0;
    UINT chkBufLen = 0;
    CHAR *cpyData = NULL;
    ComlibData *cpyPtr = NULL;
    MsgHdr *msgHdr = NULL;
    ComlibMsg *curChk = NULL;
    ComlibMsg *tmp = NULL;

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    if(len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Invalid Length value(%d)\n",len);
        return COMERR_MSG_INVALID_LEN;
    }

    if(len == 0){
        return RC_OK;
    }

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    /* check max length */
    if(msgHdr->len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is full(len=%u, max=%u)\n",msgHdr->len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_IS_FULL;
    }

    if((msgHdr->len + len) >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is overflow(msgLen=%u, dataLen=%u max=%u)\n",msgHdr->len, len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_OVERFLOW;
    }

    /* get buffer */
    chkBufLen = g_msgCfg.bufLen - sizeof(ComlibMsg);

    curChk = (*msg)->next;
    cpyTotLen =  len;

    if(curChk == NULL){/* if first message */
        cpyData = data;
        curChk = (*msg);

        MAKE_NEW_CHK(msgHdr, curChk, cpyData, len);

        goto goto_addMsgFirstEnd;
    }/* if(curChk == NULL) */
    else {
        cpyData = data+len;
    }/* end of else */

    if(curChk->base != curChk->sPtr){
        cpyLen = curChk->sPtr - curChk->base;
        if(cpyLen > cpyTotLen){
            curChk->sPtr -= cpyTotLen;
            cpyLen = cpyTotLen;
        }
        else {
            curChk->sPtr = curChk->base;
        }

        cpyData -= cpyLen;

        comlib_memMemcpy(curChk->sPtr, cpyData, cpyLen);

        curChk->sPtr = curChk->base;

        cpyTotLen -= cpyLen;
        curChk->bufSize += cpyLen;
    }/* end of if(curChk->base != curChk->sPtr) */

    /* copy data */
    while(cpyTotLen){
        ALLOC_NEW_CHK(tmp , g_msgCfg.bufLen);

        if(cpyTotLen < chkBufLen){
            cpyPtr = tmp->lim - cpyTotLen;
            cpyLen = cpyTotLen;
            cpyTotLen = 0;
        }
        else {
            cpyPtr = tmp->base;
            cpyLen = chkBufLen;
            cpyTotLen -= chkBufLen;
        }

        cpyData -= cpyLen;
        COPY_DATA_INTO_CHK((tmp), cpyPtr, cpyData, cpyLen);

        tmp->next = curChk;
        tmp->prev = curChk->prev;
        curChk->prev = tmp;
        tmp->prev->next =tmp;

        curChk = tmp;
    }/* end of while(cpyTotLen) */

goto_addMsgFirstEnd:
    msgHdr->len += len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgAddMsgEnd(ComlibMsg **msg, CHAR *data, UINT len)
{
    UINT cpyTotLen = 0;
    UINT cpyLen = 0;
    UINT chkBufLen = 0;
    CHAR *cpyData = NULL;
    ComlibData *cpyPtr = NULL;
    MsgHdr *msgHdr = NULL;
    ComlibMsg *curChk = NULL;
    ComlibMsg *tmp = NULL;

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    if(len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Invalid Length value(%d)\n",len);
        return COMERR_MSG_INVALID_LEN;
    }

    if(len == 0){
        return RC_OK;
    }

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    /* check max length */
    if(msgHdr->len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is full(len=%u, max=%u)\n",msgHdr->len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_IS_FULL;
    }

    if((msgHdr->len + len) >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is overflow(msgLen=%u, dataLen=%u max=%u)\n",msgHdr->len, len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_OVERFLOW;
    }

    /* get buffer */
    chkBufLen = g_msgCfg.bufLen - sizeof(ComlibMsg);

    curChk = msgHdr->endChk;
    cpyTotLen =  len;

    if(curChk == NULL){
        cpyData = data;
        curChk = (*msg);

        MAKE_NEW_CHK(msgHdr, curChk, cpyData, len);

        goto goto_addMsgEndEnd;
    }/* if(curChk == NULL) */
    else {
        cpyData = data;
    }

    /* add message */
    if(curChk->ePtr != curChk->lim){
        cpyLen = curChk->lim - curChk->ePtr;
        if(cpyLen > cpyTotLen){
            cpyLen = cpyTotLen;
        }

        cpyTotLen -= cpyLen;

        comlib_memMemcpy(curChk->ePtr, cpyData, cpyLen);

        curChk->ePtr += cpyLen;

        curChk->bufSize += cpyLen;
        cpyData += cpyLen;
    }/* end of if(curChk->ePtr != curChk->lim) */

    /* make new chunk */
    while(cpyTotLen){
        ALLOC_NEW_CHK(tmp , g_msgCfg.bufLen);

        if(cpyTotLen < chkBufLen){
            cpyPtr = tmp->base;
            cpyLen = cpyTotLen;
            cpyTotLen = 0;
        }
        else {
            cpyPtr = tmp->base;
            cpyLen = chkBufLen;
            cpyTotLen -= chkBufLen;
        }

        COPY_DATA_INTO_CHK(tmp, cpyPtr, cpyData, cpyLen);

        msgHdr->endChk->next = tmp;
        tmp->prev = msgHdr->endChk;
        msgHdr->endChk = tmp;
        tmp->next = NULL;

        cpyData += cpyLen;
    }/* end of while(cpyTotLen) */

goto_addMsgEndEnd:
    msgHdr->len += len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgRmvCharEnd(ComlibMsg **msg)
{
    MsgHdr *msgHdr = NULL;
    ComlibMsg *tmp = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    tmp = msgHdr->endChk;
    if(tmp == NULL){
        COM_LOG(COM_ERR,"Can not find msg chunk\n");
        return COMERR_MSG_CAN_NOT_FIND_CHK;
    }

    if(tmp->bufSize == 1){
        /* free buffer */
        msgHdr->endChk = tmp->prev;
        tmp->prev->next = NULL;

        comlib_memFree(tmp);
    }
    else {
        tmp->bufSize--;
        tmp->ePtr--;
    }

    msgHdr->len--;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgRmvMsg(ComlibMsg **msg, UINT cur, UINT len)
{
    UINT chkIdx = 0;
    UINT rmvLen = 0;
    //UINT chkBufLen = 0;
    ComlibData *chkCur = NULL; /* chunk cursor */
    MsgHdr *msgHdr = NULL;
    ComlibMsg *curChk = NULL;
    ComlibMsg *rmvChk = NULL;
    ComlibMsg *tmp = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if((cur+len) > msgHdr->len){
        COM_LOG(COM_ERR,"Invalid length(msgLen=%d, len=%d)\n",msgHdr->len, len);
        return COMERR_MSG_INVALID_LEN;
    }

    /* get buffer length */
    //chkBufLen = g_msgCfg.bufLen - sizeof(ComlibMsg);

    /* find chunk */
    chkIdx = cur;
    curChk = (*msg)->next; /* get first data */
    FIND_MSG_IDX(curChk, chkIdx);


    /* check msg chunk for spilt */
    if(curChk == NULL){ /* if end of data */
        curChk = msgHdr->endChk;
    }

    chkCur = curChk->sPtr + chkIdx;
    SPLIT_MSG(msgHdr,curChk, chkCur, tmp);

    rmvChk = tmp;
    rmvLen = len;

    while(rmvLen){
        if(rmvChk->bufSize > rmvLen){
            /* if buffer size is big */
            rmvChk->bufSize -= rmvLen;
            rmvChk->sPtr += rmvLen;
            rmvLen = 0;
        }/* if(rmvChk->bufSize >= rmvLen) */
        else {
            /* if buffer size is small */
            rmvLen -= rmvChk->bufSize;

            curChk->next = rmvChk->next;

            if(rmvChk->next != NULL){
                rmvChk->next->prev = curChk;
            }

            comlib_memFree(rmvChk);

            rmvChk = curChk->next;
        }/* end of else */
    }/* end of while(rmvLen) */

    msgHdr->len -= len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgRmvMsgFirst(ComlibMsg **msg, UINT len)
{
    UINT rmvLen = 0;
    MsgHdr *msgHdr = NULL;
    ComlibMsg *curChk = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(len == 0){
        return RC_OK;
    }

    if(msgHdr->len < len){
        COM_LOG(COM_ERR,"Invalid length(msgLen=%d, len=%d)\n",msgHdr->len, len);
        return COMERR_MSG_INVALID_LEN;
    }

    curChk = (*msg)->next;

    while(rmvLen){
        if(curChk->bufSize > rmvLen){
            curChk->sPtr += rmvLen;
            curChk->bufSize -= rmvLen;
            rmvLen = 0;
        }
        else {
            /* if buffer size is small */
            rmvLen -= curChk->bufSize;
            (*msg)->next = curChk->next;

            if(curChk->next != NULL){
                curChk->next->prev = (*msg);
            }

            comlib_memFree(curChk);

            curChk = (*msg)->next;
        }/* end of else */
    }/* end of while(rmvLen) */

    if((*msg)->next == NULL){
        msgHdr->endChk = NULL;
    }

    msgHdr->len -= len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgRmvMsgEnd(ComlibMsg **msg, UINT len)
{
    UINT rmvLen = 0;
    MsgHdr *msgHdr = NULL;
    ComlibMsg *curChk = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(len == 0){
        return RC_OK;
    }

    if(msgHdr->len < len){
        COM_LOG(COM_ERR,"Invalid length(msgLen=%d, len=%d)\n",msgHdr->len, len);
        return COMERR_MSG_INVALID_LEN;
    }

    rmvLen = len;

    curChk = msgHdr->endChk;

    while(rmvLen){
        if(curChk->bufSize > rmvLen){
            curChk->ePtr -= rmvLen;
            curChk->bufSize -= rmvLen;
            rmvLen = 0;
        }
        else{
            rmvLen -= curChk->bufSize;

            curChk->prev->next = NULL;
            msgHdr->endChk = curChk->prev;

            comlib_memFree(curChk);

            curChk = msgHdr->endChk;
        }
    }/* end of while(rmvLen) */

    if((*msg)->next == NULL){
        msgHdr->endChk = NULL;
    }

    msgHdr->len -= len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgAddMsg(ComlibMsg **msg, UINT cur, CHAR *data, UINT len)
{
    ComlibMsg *curChk = NULL;
    ComlibMsg *tmp = NULL;
    MsgHdr *msgHdr = NULL;
    UINT chkIdx = 0;
    UINT chkBufLen = 0; 	/* real buffer size */
    ComlibData *chkCur = NULL; /* chunk cursor */

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    if(cur >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Invalid cursor value(%d)\n",cur);
        return COMERR_MSG_INVALID_CUR;
    }

    if(len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Invalid Length value(%d)\n",len);
        return COMERR_MSG_INVALID_LEN;
    }

    if(len == 0){
        return RC_OK;
    }

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len < cur){
        COM_LOG(COM_ERR,"Cursor overflow(%d)\n",cur);
        return COMERR_MSG_CUR_OVERFLOW;
    }

    /* check max length */
    if(msgHdr->len >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is full(len=%u, max=%u)\n",msgHdr->len, g_msgCfg.maxLen);
        return COMERR_MSG_LEN_IS_FULL;
    }

    if((msgHdr->len + len) >= g_msgCfg.maxLen){
        COM_LOG(COM_ERR,"Message is full(curlen(curLen,len)=%u(%d,%d), max=%u)\n", 
                (msgHdr->len+len), msgHdr->len, len,  g_msgCfg.maxLen);
        return COMERR_MSG_LEN_IS_FULL;
    }

    /* get buffer */
    chkBufLen = g_msgCfg.bufLen - sizeof(ComlibMsg);

    chkIdx = cur;
    curChk = (*msg)->next; /* get first data */
    FIND_MSG_IDX(curChk, chkIdx);

    /* check msg chunk for spilt */
    if(curChk == NULL){ /* if end of data */
        if(msgHdr->endChk == NULL){
            curChk = (*msg);
        }
        else {
            curChk = msgHdr->endChk;
        }
        goto goto_addMsgMakeNewChk;
    }/* if(curChk == NULL) */
    else {
        chkCur = &curChk->sPtr[chkIdx];
    }/* end of else */

    SPLIT_MSG(msgHdr, curChk, chkCur, tmp);
    /* for warning */
    tmp = tmp;

goto_addMsgMakeNewChk:
    MAKE_NEW_CHK(msgHdr, curChk, data, len);

    msgHdr->len += len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgRmvCharFirst(ComlibMsg **msg)
{
    MsgHdr *msgHdr = NULL;
    ComlibMsg *tmp = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    tmp = (*msg)->next;
    if(tmp == NULL){
        COM_LOG(COM_ERR,"Can not find msg chunk\n");
        return COMERR_MSG_CAN_NOT_FIND_CHK;
    }

    if(tmp->bufSize == 1){
        /* free message chunk */
        (*msg)->next = tmp->next;

        if(tmp->next == NULL){
            msgHdr->endChk = NULL;
        }
        else{
            tmp->next->prev = (*msg);
        }

        comlib_memFree(tmp);
    }
    else {
        /* delete char */
        tmp->sPtr++;
        tmp->bufSize--;
    }

    msgHdr->len--;

    return RC_OK;
}


#if 0
FT_PUBLIC RT_RESULT comlib_msgChgChar()
{
}

FT_PUBLIC RT_RESULT comlib_msgChgMsg()
{
}
#endif

FT_PUBLIC RT_RESULT comlib_msgCmpMsg(ComlibMsg **msg1, UINT cur1, ComlibMsg **msg2, UINT cur2, UINT len, SINT *rt_ret)
{
    SINT ret = RC_OK;
    UINT chkIdx1 = 0;
    UINT chkIdx2 = 0;
    UINT chkLen1 = 0;
    UINT chkLen2 = 0;
    ComlibMsg *curChk1 = NULL;
    ComlibMsg *curChk2 = NULL;
    MsgHdr *msgHdr1 = NULL;
    MsgHdr *msgHdr2 = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg1 == NULL,
                    COM_LOG(COM_ERR,"Message1 is NULL\n"),
                    COMERR_MSG_NULL);

    GEN_CHK_ERR_RET(msg2 == NULL,
                    COM_LOG(COM_ERR,"Message2 is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr1 = (MsgHdr*)(*msg1)->sPtr;
    if(msgHdr1 == NULL){
        COM_LOG(COM_ERR,"Message header1 is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr1->len == 0){
        COM_LOG(COM_ERR,"Message1 is empty\n");
        return COMERR_MSG_EMPTY;
    }

    msgHdr2 = (MsgHdr*)(*msg2)->sPtr;
    if(msgHdr2 == NULL){
        COM_LOG(COM_ERR,"Message header2 is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr2->len == 0){
        COM_LOG(COM_ERR,"Message2 is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(len == 0){
        COM_LOG(COM_ERR,"compare length is 0\n");
        return COMERR_MSG_INVALID_LEN;
    }

    if(len > (msgHdr1->len - cur1) || 
       len > (msgHdr2->len - cur2)){
        COM_LOG(COM_ERR,"message length is overflow (msg1Len=%d, cur1=%d, msg2Len=%d, cur2=%d len=%d)\n",
                msgHdr1->len, cur1, msgHdr2->len, cur2, len);
        return COMERR_MSG_LEN_OVERFLOW;
    }

    /* get msg cursor */
    chkIdx1 = cur1;
    curChk1 = (*msg1)->next;
    FIND_MSG_IDX(curChk1, chkIdx1);

    chkIdx2 = cur2;
    curChk2 = (*msg2)->next;
    FIND_MSG_IDX(curChk2, chkIdx2);

    while(len){
        chkLen1 = curChk1->bufSize - chkIdx1;
        chkLen2 = curChk2->bufSize - chkIdx2;

        if(chkLen1 >= len && 
           chkLen2 >= len){
            ret = comlib_memMemcmp(&curChk1->sPtr[chkIdx1], &curChk2->sPtr[chkIdx2], len);

            *rt_ret = ret;
            return RC_OK;
        }

        if(chkLen1 <= chkLen2) {
            /* if chunk1 is small */
            ret = comlib_memMemcmp(&curChk1->sPtr[chkIdx1], &curChk2->sPtr[chkIdx2], chkLen1);
            if(ret != 0){
                *rt_ret = ret;
                return RC_OK;
            }

            chkIdx1 = 0;
            curChk1 = curChk1->next;

            chkIdx2 += chkLen1;
            if(chkIdx2 == curChk2->bufSize){
                chkIdx2 = 0;
                curChk2 = curChk2->next;
            }

            len -= chkLen1;
        }/* if(chkLen1 < chkLen2) */
        else {
            /* if chunk2 is small */
            ret = comlib_memMemcmp(&curChk1->sPtr[chkIdx1], &curChk2->sPtr[chkIdx2], chkLen2);
            if(ret != 0){
                *rt_ret = ret;
                return RC_OK;
            }

            chkIdx2 = 0;
            curChk2 = curChk2->next;

            chkIdx1 +=  chkLen2;

            len -= chkLen2;
        }/* end of else */
    }/* end of while(len--) */

    *rt_ret = 0;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgSpiltMsg(ComlibMsg **msg1, UINT cur, ComlibMsg **msg2)
{
    UINT ret = RC_OK;
    UINT chkIdx = 0;
    MsgHdr *msgHdr1 = NULL;
    MsgHdr *msgHdr2 = NULL;
    ComlibMsg *curChk = NULL;
    ComlibMsg *tmp = NULL;
    ComlibData *chkCur = NULL; /* chunk cursor */

    /* check msg */
    GEN_CHK_ERR_RET(msg1 == NULL,
                    COM_LOG(COM_ERR,"Message1 is NULL\n"),
                    COMERR_MSG_NULL);

    GEN_CHK_ERR_RET(msg2 == NULL,
                    COM_LOG(COM_ERR,"Message2 is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr1 = (MsgHdr*)(*msg1)->sPtr;
    if(msgHdr1 == NULL){
        COM_LOG(COM_ERR,"Message header1 is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr1->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(cur == 0){
        COM_LOG(COM_ERR,"Invaild cursor(cur=0)\n");
        return COMERR_MSG_INVALID_CUR;
    }

    if(cur >= msgHdr1->len){
        COM_LOG(COM_ERR,"Invalid cursor(cur=%d, len=%d)\n",cur, msgHdr1->len);
        return COMERR_MSG_CUR_OVERFLOW;
    }

    /* make new header */
    ret = comlib_msgGetMsg(msg2);
    if(ret != RC_OK){
        COM_LOG(COM_ERR,"Can noet make New message (ret=%d)\n",ret);
        return COMERR_MSG_CAN_NOT_MAKE_NEW_MSG;
    }

    chkIdx = cur;
    curChk = (*msg1)->next; /* get first data */
    FIND_MSG_IDX(curChk, chkIdx);

    /* check msg chunk for spilt */
    if(curChk == NULL){ /* if end of data */
        curChk = msgHdr1->endChk;
    }

    chkCur = &curChk->sPtr[chkIdx];

    SPLIT_MSG(msgHdr1, curChk, chkCur, tmp);

    msgHdr2 = (MsgHdr*)(*msg2)->sPtr;
    if(msgHdr2 == NULL){
        COM_LOG(COM_ERR,"Message header2 is NULL\n");
        return COMERR_MSG_HDR2_NULL;
    }

    /* make new data info */
    (*msg2)->next = tmp;
    tmp->prev = (*msg2);
    msgHdr2->endChk = msgHdr1->endChk;
    msgHdr2->len = msgHdr1->len - cur;

    /* change old data info */
    msgHdr1->endChk = curChk;
    msgHdr1->len = cur;
    msgHdr1->endChk->next = NULL;

    return RC_OK;
}

#if 0
FT_PUBLIC RT_RESULT comlib_msgExamChar()
{
}

FT_PUBLIC RT_RESULT comlib_msgExamMsg()
{
}
#endif

FT_PUBLIC RT_RESULT comlib_msgCpyMsgIntoFixBuf(ComlibMsg **msg, UINT cur, UINT len, CHAR *buf)
{
    CHAR *bufCur = NULL; /* buffer cursor */
    UINT remLen = 0;
    UINT cpyLen = 0;
    UINT chkIdx = 0;
    ComlibMsg *curChk = NULL;
    MsgHdr *msgHdr = NULL;

    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(msgHdr->len < cur){
        COM_LOG(COM_ERR,"Invalid cursor(msgLen=%d, cur=%d)\n",msgHdr->len, cur);
        return COMERR_MSG_CUR_OVERFLOW;
    }

    if(msgHdr->len > (cur+len)){
        COM_LOG(COM_ERR,"Buffer is small(msgLen=%d, bufLen=%d)\n",msgHdr->len, (cur+len));
        return COMERR_MSG_BUFFER_IS_SMALL;
    }

    chkIdx = cur;
    curChk = (*msg)->next;
    FIND_MSG_IDX(curChk, chkIdx);

    if(curChk == NULL){/* if end of data chunk */
        curChk = msgHdr->endChk;
    }

    bufCur = buf;
    remLen = len;
    cpyLen = curChk->bufSize - chkIdx;

    if(cpyLen <= remLen){
        comlib_memMemcpy(bufCur, (curChk->sPtr+chkIdx), remLen);
        goto goto_cpyMsgInfoFixBufEnd;
    }

    comlib_memMemcpy(bufCur, (curChk->sPtr+chkIdx), cpyLen);
    bufCur+= cpyLen;
    remLen -= cpyLen;

    curChk = curChk->next;

    while(remLen){
        if(curChk->bufSize < remLen){
            cpyLen = curChk->bufSize;
        }
        else {
            cpyLen = remLen;
        }
        comlib_memMemcpy(bufCur,curChk->sPtr, cpyLen);

        curChk = curChk->next;
        remLen -= cpyLen;
    }/* end of while(remLen) */

goto_cpyMsgInfoFixBufEnd:
    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgCpyAllMsgIntoFixBuf(ComlibMsg **msg, CHAR *buf, UINT bufLen, UINT *rt_cpyLen)
{
    CHAR *bufCur = NULL; /* buffer cursor */
    ComlibMsg *curChk = NULL;
    MsgHdr *msgHdr = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    if(msgHdr->len > bufLen){
        COM_LOG(COM_ERR,"Buffer is small(msgLen=%d, bufLen=%d)\n",msgHdr->len, bufLen);
        return COMERR_MSG_BUFFER_IS_SMALL;
    }

    curChk = (*msg)->next;
    bufCur = buf;

    while(curChk){
        comlib_memMemcpy(bufCur,curChk->sPtr, curChk->bufSize);

        bufCur += curChk->bufSize;
        curChk = curChk->next;
    }

    (*rt_cpyLen) = msgHdr->len;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_msgCpyAllMsgIntoDynBuf(ComlibMsg **msg, CHAR **buf, UINT *cpyLen)
{
    CHAR *bufCur = NULL; /* buffer cursor */
    ComlibMsg *curChk = NULL;
    MsgHdr *msgHdr = NULL;

    /* check msg */
    GEN_CHK_ERR_RET(msg == NULL,
                    COM_LOG(COM_ERR,"Message is NULL\n"),
                    COMERR_MSG_NULL);

    msgHdr = (MsgHdr*)(*msg)->sPtr;
    if(msgHdr == NULL){
        COM_LOG(COM_ERR,"Message header is NULL\n");
        return COMERR_MSG_HDR_NULL;
    }

    if(msgHdr->len == 0){
        COM_LOG(COM_ERR,"Message is empty\n");
        return COMERR_MSG_EMPTY;
    }

    curChk = (*msg)->next;
    bufCur = NULL;

    /* allociate buffer */
    *buf = comlib_memMalloc(msgHdr->len);
    if(*buf == NULL){
        COM_LOG(COM_ERR,"Buffer allocate failed\n");
        return COMERR_MSG_ALLOC_FAIL;
    }

    bufCur = (*buf);

    while(curChk){
        comlib_memMemcpy(bufCur, curChk->sPtr, curChk->bufSize);

        bufCur += curChk->bufSize;
        curChk = curChk->next;
    }

    *cpyLen = msgHdr->len;

    return RC_OK;
}
