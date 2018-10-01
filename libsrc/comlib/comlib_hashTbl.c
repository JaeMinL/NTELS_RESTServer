#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#define HASH_GET_ENTRY_INDX(_hashTbl, _key, _rt_entIdx){\
	if((_hashTbl)->bitMask != 0){\
		(_rt_entIdx) = (_key) & (_hashTbl)->bitMask;\
	}\
	else{\
		(_rt_entIdx) = (_key) % (_hashTbl)->nmbEntry;\
	}\
}

#define HASH_GET_HASH_KEY(_hashTbl, _key, _rt_hashKey, _ret){\
    (_ret) = RC_NOK;\
    if((_hashTbl)->hashType == COM_HASH_TYPE_MURMUR){\
        (_ret) = hashTbl_getIndxFromMurMur((_key)->key, (_key)->keyLen, &(_rt_hashKey));\
    }\
    else if((_hashTbl)->hashType == COM_HASH_TYPE_USRKEY){\
        (_ret) = RC_OK;\
        (_rt_hashKey) = (_key)->hashKey;\
    }\
    else if((_hashTbl)->hashType == COM_HASH_TYPE_STRING){\
        (_ret) = hashTbl_getIndxFromStr((_key)->key, (_key)->keyLen, &(_rt_hashKey));\
    }\
    else if((_hashTbl)->hashType ==  COM_HASH_TYPE_UINT){\
        (_ret) = hashTbl_getIndxFromUInt((_key)->key, (_key)->keyLen, &(_rt_hashKey));\
    }\
    else if((_hashTbl)->hashType ==  COM_HASH_TYPE_USR){\
        (_ret) = (*(_hashTbl)->func)((_key)->key, (_key)->keyLen, &(_rt_hashKey));\
    }\
}

/* Private Function difinition */
FT_PRIVATE RT_RESULT        hashTbl_getIndxFromStr         (CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey);
FT_PRIVATE RT_RESULT        hashTbl_getIndxFromMurMur      (CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey);
FT_PRIVATE RT_RESULT        hashTbl_getIndxFromUInt        (CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey);
FT_PRIVATE INLINE RT_RESULT hashTbl_checkKey               (CONST VOID *fKey, CONST UINT fKeyLen, CONST VOID *sKey, 
                                                            CONST UINT sKeyLen);
FT_PRIVATE RT_RESULT        hashTbl_findHashNode           (CONST ComlibHashEntry *entry, CONST ComlibHashKey *key, 
                                                            CONST UINT nodeId, ComlibHashNode **rt_node);
FT_PRIVATE RT_RESULT        hashTbl_findHashNodeByHashKey  (CONST ComlibHashEntry *entry, CONST UINT key, CONST UINT nodeId, 
                                                            ComlibHashNode **rt_node);
FT_PRIVATE RT_RESULT        hashTbl_insertHashNode         (ComlibHashEntry *entry, CONST ComlibHashKey *key, 
                                                            ComlibHashNode *node);
FT_PRIVATE RT_RESULT        hashTbl_insertPrevHashNode     (ComlibHashEntry *entry, ComlibHashNode *prevNode, 
                                                            CONST ComlibHashKey *key, ComlibHashNode *node);
FT_PRIVATE RT_RESULT        hashTbl_delHashNode            (ComlibHashEntry *entry, ComlibHashNode *node);
FT_PRIVATE RT_RESULT        hashTbl_delHashNodeByHashKey   (ComlibHashEntry *entry, ComlibHashNode *node);
FT_PRIVATE RT_RESULT        hashTbl_getHashNodeByHashKey   (ComlibHashEntry *entry, CONST UINT hashKey, 
                                                            CONST UINT nodeId, ComlibHashNode **rt_hNode);
FT_PRIVATE RT_RESULT        hashTbl_getHashNode            (ComlibHashEntry *entry, CONST UINT hashKey, 
                                                            CONST ComlibHashKey *key, CONST UINT nodeId, 
                                                            ComlibHashNode **rt_hNode);
FT_PRIVATE RT_RESULT        hashTbl_freeHashNode           (ComlibHashEntry *entry, ComlibHashNode *node);

FT_PRIVATE RT_RESULT hashTbl_getIndxFromUInt(CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey)
{
    (*rt_hashKey) = (UINT)(*((UINT*)key));

    return RC_OK;
}

FT_PRIVATE RT_RESULT hashTbl_getIndxFromMurMur(CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey)
{
    return comlib_hashMurMur(key, keyLen, 0, rt_hashKey);
}

FT_PRIVATE RT_RESULT hashTbl_getIndxFromStr(CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey)
{
    return comlib_hashStr(key, keyLen, rt_hashKey);
}

FT_PRIVATE INLINE RT_RESULT hashTbl_checkKey(CONST VOID *fKey, CONST UINT fKeyLen, CONST VOID *sKey, CONST UINT sKeyLen)
{
    UINT ret = RC_OK;

    if(fKeyLen != sKeyLen){
        COM_LOG(COM_ERR,"Key length is mismatch(key1=%d, key2=%d)\n",fKeyLen, sKeyLen);
        return COMERR_KEY_LENGTH_IS_MISMATCH;
    }

    ret = comlib_memMemcmp(fKey, sKey, fKeyLen);
    if(ret != 0){
        COM_LOG(COM_ERR,"Key is mismatch(keyLen=%d)\n",fKeyLen);
        return COMERR_KEY_IS_MISMATCH;
    }

    return RC_OK;
}

FT_PRIVATE RT_RESULT hashTbl_findHashNodeByHashKey(CONST ComlibHashEntry *entry, CONST UINT key, CONST UINT nodeId, 
                                                   ComlibHashNode **rt_node)
{
    UINT seqNum = 0;
    REGISTER ComlibHashNode *node = NULL;

    node = entry->first;
    while(node){
        if(node->key.hashKey == key){
            if(seqNum == nodeId){
                *rt_node = node;
                return RC_OK;
            }
            else {
                seqNum++;
            }
        }

        node = node->next;
    }

    return RC_NOK;
}

FT_PRIVATE RT_RESULT hashTbl_findHashNode(CONST ComlibHashEntry *entry, CONST ComlibHashKey *key, CONST UINT nodeId, 
                                          ComlibHashNode **rt_node)
{
    BOOL ret = RC_OK;
    UINT seqNum = 0;
    REGISTER ComlibHashNode *node = NULL;

    node = entry->first;
    while(node){
        ret = hashTbl_checkKey(key->key, key->keyLen, node->key.key, node->key.keyLen);
        if(ret == RC_OK){
            if(seqNum == nodeId){
                *rt_node = node;
                return RC_OK;
            }
            else {
                seqNum++;
            }
        }

        node = node->next;
    }

    return RC_NOK;
}

FT_PRIVATE RT_RESULT hashTbl_freeHashNode(ComlibHashEntry *entry, ComlibHashNode *node)
{
    if(node->prev == NULL){
        entry->first = node->next;
        if(entry->first){
            entry->first->prev = NULL;
        }
    }

    if(node->next == NULL){
        entry->tail = node->prev;
        if(entry->tail){
            entry->tail->next = NULL;
        }
    }

    if(node->prev != NULL && node->next != NULL){
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }

    node->own = NULL;
    node->prev = NULL;
    node->next = NULL;

    entry->nodeCnt--;

    return RC_OK;
}

FT_PRIVATE RT_RESULT hashTbl_getHashNodeByHashKey(ComlibHashEntry *entry, CONST UINT hashKey, CONST UINT nodeId, 
                                                  ComlibHashNode **rt_hNode)
{
    UINT seqNum = 0;
    ComlibHashNode *node = NULL;

    node = entry->first;
    while(node){
        if(hashKey == node->key.hashKey){
            if(seqNum == nodeId){
                if(rt_hNode != NULL){
                    (*rt_hNode) = node;
                }

                hashTbl_freeHashNode(entry, node);

                return RC_OK;
            }
            else {
                seqNum++;
            }
        }/* end of if(hashKey == node->key.hashKey) */

        node = node->next;
    }/* end of while(node) */

    return RC_NOK;
}

FT_PRIVATE RT_RESULT hashTbl_delHashNode(ComlibHashEntry *entry, ComlibHashNode *node)
{
    BOOL ret = RC_OK;
    UINT matchCnt = 0;
    ComlibHashNode *tmpNode = NULL;

    if(node->prev != NULL){
        tmpNode = node->prev;
        if(tmpNode->key.hashKey == node->key.hashKey){
            ret = hashTbl_checkKey(tmpNode->key.key, tmpNode->key.keyLen, node->key.key, node->key.keyLen);
            if(ret == RC_OK){/* match */
                goto goto_freeHashNode;
            }
            else {
                matchCnt++;
            }
        }
    }

    if(node->next != NULL){
        tmpNode = node->next;
        if(tmpNode->key.hashKey == node->key.hashKey){
            ret = hashTbl_checkKey(tmpNode->key.key, tmpNode->key.keyLen, node->key.key, node->key.keyLen);
            if(ret == RC_OK){/* match */
                goto goto_freeHashNode;
            }
            else {
                matchCnt++;
            }
        }
    }

    if(matchCnt != 0){
        entry->cmpAllCnt--;
    }

goto_freeHashNode:
    hashTbl_freeHashNode(entry, node);

    return RC_OK;
}

FT_PRIVATE RT_RESULT hashTbl_delHashNodeByHashKey(ComlibHashEntry *entry, ComlibHashNode *node)
{
    return hashTbl_freeHashNode(entry, node);
}

FT_PRIVATE RT_RESULT hashTbl_getHashNode(ComlibHashEntry *entry, CONST UINT hashKey, CONST ComlibHashKey *key, 
                                         CONST UINT nodeId, ComlibHashNode **rt_hashNode)
{
    BOOL ret = RC_OK;
    UINT seqNum = 0;
    UINT matchCnt = 0;
    BOOL delFlg = RC_FALSE;
    ComlibHashNode *tmpNode = NULL;
    ComlibHashNode *node = NULL;

    node = entry->first;
    while(node){
        if(hashKey == node->key.hashKey){
            ret = hashTbl_checkKey(key->key, key->keyLen, node->key.key, node->key.keyLen);
            if(ret == RC_OK){/* key match */
                if(seqNum == nodeId){
                    tmpNode = node->next;

                    if(rt_hashNode != NULL){
                        (*rt_hashNode) = node;
                    }

                    hashTbl_freeHashNode(entry, node);
                    delFlg = RC_TRUE;
                    node = tmpNode;
                    if((matchCnt >= 1) || (node == NULL)){
                        break;
                    }
                    else {
                        continue;
                    }
                }
                else {
                    seqNum++;
                }
            }
            else {
                matchCnt++;
            }

            if((delFlg == RC_TRUE) && (matchCnt >= 1)){
                break;
            }
        }/* end of if(hashKey == node->key.hashKey) */

        node = node->next;
    }/* end of while(node) */

    if(delFlg == RC_TRUE){
        if(matchCnt >= 1){
            entry->cmpAllCnt--;
        }

        return RC_OK;
    }

    return RC_NOK;
}

FT_PRIVATE RT_RESULT hashTbl_insertPrevHashNode(ComlibHashEntry *entry, ComlibHashNode *prevNode, CONST ComlibHashKey *key, 
                                                ComlibHashNode *node)
{
    /* set hash key */
    node->key.key = key->key;
    node->key.keyLen = key->keyLen;

    node->own = entry;

    if(prevNode->prev == NULL){
        node->prev = NULL;
        prevNode->prev = node;
        node->next = prevNode;

        entry->first = node;
    }
    else {
        node->prev = prevNode->prev;
        node->prev->next = node;
        node->next = prevNode;
        prevNode->prev = node;
    }

    entry->nodeCnt++;

    return RC_OK;
}

FT_PRIVATE RT_RESULT hashTbl_insertHashNode(ComlibHashEntry *entry, CONST ComlibHashKey *key, ComlibHashNode *node)
{
    /* set hash key */
    node->key.key = key->key;
    node->key.keyLen = key->keyLen;

    node->own = entry;

    if(entry->tail){
        node->prev = entry->tail;
        entry->tail->next = node;
        entry->tail = node;
    }
    else {
        entry->first = node;
        entry->tail = node;
        node->prev = NULL;
    }

    node->next = NULL;

    entry->nodeCnt++;

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashTblInit(ComlibHashTbl *hashTbl, CONST UINT nmbEntry, CONST BOOL dupFlg, CONST UINT hashType, 
                                       ComlibHashFunc func)
{
    UINT i = 0;

    /* check argument */
    GEN_CHK_ERR_RET( hashTbl == NULL, 
                     COM_LOG(COM_ERR,"Invaild Hash List\n"),
                     COMERR_INVALID_HASHTBL);

    GEN_CHK_ERR_RET(nmbEntry == 0,
                    COM_LOG(COM_ERR,"Number of entry is zero\n"),
                    COMERR_NMBENTRY_IS_ZERO);

    GEN_CHK_ERR_RET(dupFlg != RC_TRUE && dupFlg != RC_FALSE,
                    COM_LOG(COM_ERR,"Invaild Duplicate flag(%d)\n",dupFlg),
                    COMERR_INVALID_DUP_FLAG);

    /* set hash structure */
    hashTbl->dupFlg = dupFlg;
    hashTbl->nmbEntry = nmbEntry;

    switch(hashType){
        case COM_HASH_TYPE_STRING:
        case COM_HASH_TYPE_UINT:
        case COM_HASH_TYPE_MURMUR:
        case COM_HASH_TYPE_USRKEY:
            {
                hashTbl->hashType = hashType;
            }
            break;
        case COM_HASH_TYPE_USR:
            {
                if(func == NULL){
                    COM_LOG(COM_ERR,"Hashing function not exist\n");
                    return COMERR_HASH_FUNC_NOT_EXIST;
                }

                hashTbl->func = func;
                hashTbl->hashType = hashType;
            }
            break;
        default :
            {
                COM_LOG(COM_ERR,"Invalid hash type(%d)\n",hashType);
                return COMERR_INVALID_HASHTYPE;
            }
            break;
    };

    /* set number of entry */
    hashTbl->entry = comlib_memMalloc(sizeof(ComlibHashEntry) * nmbEntry);

    for(i=0;i<nmbEntry;i++){
        hashTbl->entry[i].nodeCnt = 0;
        hashTbl->entry[i].cmpAllFlg = RC_FALSE;
        hashTbl->entry[i].cmpAllCnt = 0;
        hashTbl->entry[i].first = NULL;
        hashTbl->entry[i].tail= NULL;
    }

    /* check bit mask */
    if((nmbEntry & (nmbEntry -1)) == 0){
        hashTbl->bitMask = nmbEntry - 1;
    }
    else {
        hashTbl->bitMask = 0x0;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashTblDstry(ComlibHashTbl *hashTbl)
{
	GEN_CHK_ERR_RET(hashTbl == NULL,
		COM_LOG(COM_ERR,"Invaild Hash List\n"),
		COMERR_INVALID_HASHTBL);

    comlib_memFree(hashTbl->entry);

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashTblInsertHashNode(ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, ComlibHashNode *node)
{
    BOOL ret = RC_NOK;
    UINT indx = 0;
    UINT hashKey = 0;
    ComlibHashNode *findNode = NULL;
    ComlibHashEntry *entry = NULL;

    GEN_CHK_ERR_RET(hashTbl == NULL,
                    COM_LOG(COM_ERR,"Invaild hash table(hash table pointer is null)\n"),
                    COMERR_INVALID_HASHTBL);

    GEN_CHK_ERR_RET(key == NULL,
                    COM_LOG(COM_ERR,"Invaild key(Key pointer is null)\n"),
                    COMERR_INVALID_HASHKEY);

    GEN_CHK_ERR_RET(node == NULL,
                    COM_LOG(COM_ERR,"Invaild data(Data pointer is null)\n"),
                    COMERR_INVALID_NODE);

    /* get hash entry */
    HASH_GET_HASH_KEY(hashTbl, key, hashKey, ret);

    GEN_CHK_ERR_RET(ret == RC_NOK,
                    COM_LOG(COM_ERR,"Can not find hash entry\n"),
                    COMERR_CAN_NOT_FIND_HASHENTRY);

    HASH_GET_ENTRY_INDX(hashTbl, hashKey, indx);

    node->key.hashKey = hashKey;

    entry = &hashTbl->entry[indx];

    /* check key */
    ret = hashTbl_findHashNodeByHashKey(entry, hashKey, 0, &findNode);
    if(ret == RC_OK){
        ret = hashTbl_checkKey(key->key, key->keyLen, findNode->key.key, findNode->key.keyLen);
        if(ret != RC_OK){
            entry->cmpAllFlg = RC_TRUE;
            entry->cmpAllCnt++;
        }
        else {
            if(hashTbl->dupFlg != RC_OK){
                return COMERR_NODE_ALREADY_EXIST;
            }
        }
    }

    if(entry->cmpAllFlg == RC_TRUE){
        ret = hashTbl_insertPrevHashNode(entry, findNode, key, node);
    }
    else {
        ret = hashTbl_insertHashNode(entry, key, node);
    }
    GEN_CHK_ERR_RET(ret == RC_NOK,
                    COM_LOG(COM_ERR,"Can not insert hash Node\n"),
                    COMERR_HASHNODE_INSERT_FAIL);

    if(entry->nodeCnt > hashTbl->maxNodeBktCnt){
        hashTbl->maxNodeBktCnt = entry->nodeCnt;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashTblDelHashNode(ComlibHashTbl *hashTbl, ComlibHashNode *node)
{
    SINT ret = RC_OK;
    ComlibHashEntry *entry = NULL;

    GEN_CHK_ERR_RET(hashTbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Hash table (hash table pointer is null)\n"),
                    COMERR_INVALID_HASHTBL);

    GEN_CHK_ERR_RET(node->own == NULL,
                    COM_LOG(COM_ERR,"hash entry not exist\n"),
                    COMERR_CAN_NOT_FIND_HASHENTRY);

    entry = node->own;

    if(entry->cmpAllFlg == RC_FALSE){
        ret = hashTbl_delHashNodeByHashKey(entry, node);
        if(ret != RC_OK){
            return COMERR_HASHNODE_NOT_EXIST;
        }
    }
    else {
        ret = hashTbl_delHashNode(entry, node);
        if(ret != RC_OK){
            return COMERR_HASHNODE_NOT_EXIST;
        }

        if(entry->cmpAllCnt == 0){
            entry->cmpAllFlg = RC_FALSE;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_hashTblFindHashNode(CONST ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, CONST UINT nodeId, 
                                               ComlibHashNode **rt_node)
{
    UINT indx = 0;
    UINT hashKey = 0;
    BOOL ret = RC_NOK;
    ComlibHashEntry *entry = NULL;

    GEN_CHK_ERR_RET(hashTbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Hash table(Hash table pointer is null)\n"),
                    COMERR_INVALID_HASHTBL);

    GEN_CHK_ERR_RET(key == NULL,
                    COM_LOG(COM_ERR,"Invaild key(Key pointer is null)\n"),
                    COMERR_INVALID_HASHKEY);

    GEN_CHK_ERR_RET(hashTbl->dupFlg == RC_FALSE && nodeId != 0,
                    COM_LOG(COM_ERR,"Invadil nodeId (duplicate flag is disable)(nodeId=%d)\n",nodeId),
                    COMERR_INVAILD_NODEID);

    /* get hash entry */
    HASH_GET_HASH_KEY(hashTbl, key, hashKey, ret);

    GEN_CHK_ERR_RET(ret == RC_NOK,
                    COM_LOG(COM_ERR,"Can not find hash entry\n"),
                    COMERR_CAN_NOT_FIND_HASHENTRY);

    HASH_GET_ENTRY_INDX(hashTbl, hashKey, indx);

    entry = &hashTbl->entry[indx];
    if(entry->cmpAllFlg == RC_TRUE){
        ret = hashTbl_findHashNode(entry, key, nodeId, rt_node);
    }
    else {
        ret = hashTbl_findHashNodeByHashKey(entry, hashKey, nodeId, rt_node);
    }

    return ret;
}

FT_PUBLIC RT_RESULT comlib_hashTblGetHashNode(ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, CONST UINT nodeId, 
                                              ComlibHashNode **rt_node)
{
    UINT indx = 0;
    UINT hashKey = 0;
    BOOL ret = 0;
    ComlibHashEntry *entry = NULL;

    GEN_CHK_ERR_RET(hashTbl == NULL,
                    COM_LOG(COM_ERR,"Invaild Hash table (Hash table pointer is null)\n"),
                    COMERR_INVALID_HASHTBL);

    GEN_CHK_ERR_RET(key == NULL,
                    COM_LOG(COM_ERR,"Invaild key(Key pointer is null)\n"),
                    COMERR_INVALID_HASHKEY);

    GEN_CHK_ERR_RET(hashTbl->dupFlg == RC_FALSE && nodeId != 0,
                    COM_LOG(COM_ERR,"Invadil nodeId (duplicate flag is disable)(nodeId=%d)\n",nodeId),
                    COMERR_INVAILD_NODEID);

    /* get hash entry */
    HASH_GET_HASH_KEY(hashTbl, key, hashKey, ret);

    GEN_CHK_ERR_RET(ret == RC_NOK,
                    COM_LOG(COM_ERR,"Can not find hash entry\n"),
                    COMERR_CAN_NOT_FIND_HASHENTRY);

    HASH_GET_ENTRY_INDX(hashTbl, hashKey, indx);

    entry = &hashTbl->entry[indx];
    if(entry->cmpAllFlg == RC_FALSE){
        ret = hashTbl_getHashNodeByHashKey(entry, hashKey, nodeId, rt_node); 
        if(ret != RC_OK){
            return COMERR_HASHNODE_NOT_EXIST;
        }
    }
    else {
        ret = hashTbl_getHashNode(entry, hashKey, key, nodeId, rt_node); 
        if(ret != RC_OK){
            return COMERR_HASHNODE_NOT_EXIST;
        }

        if(entry->cmpAllCnt == 0){
            entry->cmpAllFlg = RC_FALSE;
        }
    }

    return ret;
}

