#include <stdio.h>
#include <unistd.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

/* Linked list macro (PRIVATE) */
#define CHECK_LNKLST(lnkLst){\
    GEN_CHK_ERR_RET(lnkLst == NULL,\
                    COM_LOG(COM_ERR,"lnkLst is NULL\n"),\
                    COMERR_INVALID_LNKLST);\
}

#define CHECK_LNKLST_P(lnkLst){\
    GEN_CHK_ERR_RET(lnkLst == NULL,\
                    COM_LOG(COM_ERR,"lnkLst is NULL\n"),\
                    NULL);\
}

#define CHECK_LNKNODE(lnkNode){\
    GEN_CHK_ERR_RET(lnkNode == NULL,\
                    COM_LOG(COM_ERR,"LnkNode is NULL\n"),\
                    COMERR_INVALID_LNKNODE)\
}

#ifdef GEN_CHK_ERR
#define CHECK_LNKENTCNT(lnkLst){\
	if(lnkLst->maxNode != COM_NOT_LIMIT){\
        GEN_CHK_ERR_RET(lnkLst->maxNode <= lnkLst->nodeCnt,\
                        COM_LOG(COM_ERR,"LnkNode is Max\n"),\
                        COMERR_MAX_NODE);\
	}\
}
#else
#define CHECK_LNKENTCNT(lnkLst)
#endif

/*-------------------- linked list functions --------------------*/ 
FT_PUBLIC RT_RESULT comlib_lnkLstInit(ComlibLnkLst *lnkLst, UINT maxNode)
{
	/* check argument */
	CHECK_LNKLST(lnkLst);

	lnkLst->maxNode = maxNode;
	lnkLst->nodeCnt = 0;
	lnkLst->first = NULL;
	lnkLst->tail = NULL;

	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstAppendFirst(ComlibLnkLst *dst, ComlibLnkLst *src)
{
	CHECK_LNKLST(src);
	CHECK_LNKLST(dst);

    if((src->nodeCnt + dst->nodeCnt) > dst->maxNode){
        COM_LOG(COM_ERR,"max node(src=%d, dst=%d, max=%d)\n",
                src->nodeCnt, dst->nodeCnt, dst->maxNode);
        return COMERR_MAX_NODE;
    }

    if(src->nodeCnt == 0){
        return RC_OK;
    }

    if(dst->nodeCnt == 0){
        dst->first = src->first;
        dst->tail = src->tail;
    }
    else {
        dst->first->prev = src->tail;
        src->tail->next = dst->first;
        dst->first = src->first;
    }

    dst->nodeCnt += src->nodeCnt;

    src->first = NULL;
    src->tail = NULL;
    src->nodeCnt = 0;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstAppendTail(ComlibLnkLst *dst, ComlibLnkLst *src)
{
    CHECK_LNKLST(src);
    CHECK_LNKLST(dst);

    if((src->nodeCnt + dst->nodeCnt) > dst->maxNode){
        COM_LOG(COM_ERR,"max node(src=%d, dst=%d, max=%d)\n",
                src->nodeCnt, dst->nodeCnt, dst->maxNode);
        return COMERR_MAX_NODE;
    }

    if(src->nodeCnt == 0){
        return RC_OK;
    }

    if(dst->nodeCnt == 0){
        dst->first = src->first;
        dst->tail = src->tail;
    }
    else {
        dst->tail->next = src->first;
        src->first->prev = dst->tail;
        dst->tail = src->tail;
    }

    dst->nodeCnt += src->nodeCnt;

    src->first = NULL;
    src->tail = NULL;
    src->nodeCnt = 0;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstInsertFirst(ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);
	CHECK_LNKENTCNT(lnkLst);

	if(lnkLst->first){
		lnkNode->next = lnkLst->first;
		lnkLst->first->prev = lnkNode;
		lnkLst->first = lnkNode;
	}
	else {
		lnkLst->first = lnkNode;
		lnkLst->tail = lnkNode;
		lnkNode->next = NULL;
	}

	lnkNode->prev = NULL;

	lnkLst->nodeCnt++;
	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstInsertTail(ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);
	CHECK_LNKENTCNT(lnkLst);

	/* insert LnkNode */
	if(lnkLst->tail){
		lnkNode->prev = lnkLst->tail;	
		lnkLst->tail->next = lnkNode;
		lnkLst->tail = lnkNode;
	}
	else {
		lnkLst->first = lnkNode;
		lnkLst->tail = lnkNode;
		lnkNode->prev = NULL;
	}

	lnkNode->next = NULL;

	lnkLst->nodeCnt++;

	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstInsertNextNode(ComlibLnkLst *lnkLst, ComlibLnkNode *org, ComlibLnkNode *node)
{
	ComlibLnkNode *tmp = NULL;

	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(org);
	CHECK_LNKNODE(node);
	CHECK_LNKENTCNT(lnkLst);

	if(org->next == NULL){
		org->next = node;
		node->prev = org;
		node->next = NULL;
		lnkLst->tail = node;
	}
	else {
		tmp = org->next;

		org->next = node;
		node->prev = org;

		node->next = tmp;
		tmp->prev = node;
	}

	lnkLst->nodeCnt++;

	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_lnkLstInsertPrevNode(ComlibLnkLst *lnkLst, ComlibLnkNode *org, ComlibLnkNode *node)
{
	ComlibLnkNode *tmp = NULL;

	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(org);
	CHECK_LNKNODE(node);
	CHECK_LNKENTCNT(lnkLst);

	if(org->prev == NULL){
		org->prev = node;
		node->next = org;
		node->prev = NULL;
		lnkLst->first = node;
	}
	else {
		tmp = org->prev;

		org->prev = node;
		node->next = org;

		node->prev = tmp;
		tmp->next = node;
	}

	lnkLst->nodeCnt++;

	return RC_OK;
}

FT_PUBLIC ComlibLnkNode *comlib_lnkLstGetFirst(ComlibLnkLst *lnkLst)
{
	ComlibLnkNode *node;

	CHECK_LNKLST_P(lnkLst);

	if(lnkLst->nodeCnt == 0){
		COM_LOG(COM_ERR,"Linked list is empty\n");
		return NULL;
	}

	node = lnkLst->first;

	if(node->next == NULL){
		lnkLst->first = NULL;
		lnkLst->tail = NULL;
	}
	else{
		lnkLst->first = node->next;
		lnkLst->first->prev = NULL;
	}

	node->next = NULL;

	lnkLst->nodeCnt--;
	
	return node;
}

FT_PUBLIC ComlibLnkNode *comlib_lnkLstGetTail(ComlibLnkLst *lnkLst)
{
	ComlibLnkNode *node;

	CHECK_LNKLST_P(lnkLst);

	if(lnkLst->nodeCnt == 0){
		COM_LOG(COM_ERR,"Linked list is empty\n");
		return NULL;
	}

	node = lnkLst->tail;

	if(node->prev == NULL){
		lnkLst->first = NULL;
		lnkLst->tail = NULL;
	}
	else {
		lnkLst->tail = node->prev;
		lnkLst->tail->next = NULL;
	}

	node->prev = NULL;

	lnkLst->nodeCnt--;

	return node;
}

FT_PUBLIC RT_RESULT comlib_lnkLstDel(ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);

	/* check node count */
	if(lnkLst->nodeCnt == 0){
		COM_LOG(COM_ERR,"Linked list is empty\n");
		return COMERR_LNKLST_IS_EMPTY;
	}

	/* if first data */
	if(lnkNode->prev == NULL){
		lnkLst->first = lnkNode->next;
		if(lnkLst->first){
			lnkLst->first->prev = NULL;
		}
	}

	/* if tail data */
	if(lnkNode->next == NULL){
		lnkLst->tail = lnkNode->prev;
		if(lnkLst->tail){
			lnkLst->tail->next = NULL;
		}
	}

	if(lnkNode->prev != NULL && lnkNode->next != NULL){
		lnkNode->next->prev = lnkNode->prev;
		lnkNode->prev->next = lnkNode->next;
	}

	lnkNode->prev = NULL;
	lnkNode->next = NULL;

	lnkLst->nodeCnt--;

	return RC_OK;
}

