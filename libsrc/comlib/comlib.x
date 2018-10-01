/************************************************************************

     Name:     Common library 

     Type:     C structure file

     Desc:     Common library structure

     File:     comlib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _COMLIB_X_
#define _COMLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

/* common library data type */
typedef        U_8               ComlibData;

/* Linked list */
typedef struct ComlibLnkLst      ComlibLnkLst;
typedef struct ComlibLnkNode     ComlibLnkNode;

/* Hash */
typedef struct ComlibHashNode    ComlibHashNode;
typedef struct ComlibHashEntry   ComlibHashEntry;
typedef struct ComlibHashTbl     ComlibHashTbl;
typedef struct ComlibHashKey     ComlibHashKey;

/* Timer */
typedef struct ComlibTimer       ComlibTimer;
typedef struct ComlibTmrTbl      ComlibTmrTbl;
typedef struct ComlibTmrNode     ComlibTmrNode;
typedef struct ComlibTmrNodeInfo ComlibTmrNodeInfo;
typedef struct ComlibTmrEntry    ComlibTmrEntry;

/* Message buffer */
typedef struct ComlibMsg         ComlibMsg;

/* Hash function definition */
/* hashFunc(key, keyLen, *rt_hashKey) */
typedef RT_RESULT (*ComlibHashFunc)       (CONST VOID*, CONST UINT, UINT*); /* hashing function */

/* Timer function definition */
/* evtFunc(event, usrArg) */
typedef RT_RESULT (*ComlibTmrEvtFunc)     (UINT, VOID*); /* timer event function */

/* Linked list */
struct ComlibLnkNode{
    ComlibLnkNode       *prev;
    ComlibLnkNode       *next;
    VOID                *data;
};

struct ComlibLnkLst{
    UINT                maxNode;    /* max node (0 : Not limited) */
    UINT                nodeCnt;    /* Linked list node count */
    ComlibLnkNode       *first;
    ComlibLnkNode       *tail;
};

/* Hash */
struct ComlibHashKey{
    VOID                *key;
    UINT                keyLen;
    UINT                hashKey;
};

struct ComlibHashNode{
    ComlibHashKey       key;
    ComlibHashEntry     *own;
    ComlibHashNode 	    *next;      /* next HashNode */
    ComlibHashNode 	    *prev;      /* prev HashNode */
    VOID                *data;
};

struct ComlibHashEntry{
    UINT                nodeCnt;    /* node count */
    BOOL                cmpAllFlg; /* compare all message */
    UINT                cmpAllCnt;
    ComlibHashNode      *first;
    ComlibHashNode      *tail;
};

struct ComlibHashTbl{
    UINT                hashType;   /* Hash Type */
    UINT                maxNodeBktCnt;
    UINT                nmbEntry;   /* Number of entry*/
    UINT                bitMask;    /* bit mask */
    BOOL                dupFlg;     /* Allow duplicate key */
    ComlibHashEntry     *entry;     /* Hash entry */
    ComlibHashFunc      func;       /* hashing function */
};

/* Timer */
struct ComlibTimer{
    UINT                type;       /* Timer type */
    DOUBLE              tmSlice;	
#ifdef _BIT_64_
    DOUBLE              lastTm;
    DOUBLE              errTm;
#else 
    struct timespec     lastTs;
    ULONG               errTm;
    ULONG               tps;        /* tick per Sec */
#endif
    struct timespec     curTs;
    ULONG               avgDiff;
    ULONG               tick;
};

struct ComlibTmrNode{
    BOOL                used;
    ULONG               expTick;
    UINT                event;
    ULONG               updateTick;
    BOOL                resetFlg;	
    ComlibTmrEntry      *ownEntry;
    VOID                *data;
    ComlibTmrNode       *prev;
    ComlibTmrNode       *next;
};

struct ComlibTmrEntry{
    ComlibTmrNode       *first;
    ComlibTmrNode       *tail;
};

struct ComlibTmrTbl{
    ComlibTimer         *tmr;
    ComlibTmrEvtFunc    func;
    ULONG               prevTick;
    ComlibTmrEntry      *entry;
};

/*
**      Message buffer architecture
**
**      +--------+------- +   +--------+------+   +--------+------+
**      | MsgBuf | MsgHdr |-->| MsgBuf | Data |-->| MsgBuf | Data | .....
**      +--------+--------+   +--------+------+   +--------+------+
*/
struct ComlibMsg{ /* = Msb block */
    /* single linked list */
    ComlibMsg           *prev;      /* previous message buffer Chunk*/
    ComlibMsg           *next;      /* next message buffer Chunk*/
    UINT                bufSize;    /* total buffer Size */
    ComlibData          *sPtr;      /* start of data */
    ComlibData          *ePtr;      /* end of data */
    ComlibData          *base;      /* base data buffer */
    ComlibData          *lim;       /* limit of data buffer */
};

/*--------------------------------------------------------------*/
/*              common libary public function                   */
/*--------------------------------------------------------------*/
/* Memory management function */
FT_PUBLIC VOID*          comlib_memMalloc              (CONST SIZET size);
FT_PUBLIC VOID*          comlib_memCalloc              (CONST SIZET num, CONST SIZET size);
FT_PUBLIC RT_RESULT      comlib_memMemcmp              (CONST VOID *s1, CONST VOID *s2, CONST SIZET n);
FT_PUBLIC VOID*          comlib_memMemcpy              (VOID *dst, CONST VOID *src, CONST SIZET n);
FT_PUBLIC VOID*          comlib_memMemset              (VOID *src, CONST UINT c, CONST SIZET n);
FT_PUBLIC VOID*          comlib_memMemmov              (VOID *dst, CONST VOID *src, CONST SIZET n);
FT_PUBLIC VOID           comlib_memFree                (VOID *ptr);

/* Linked list public function */
FT_PUBLIC RT_RESULT      comlib_lnkLstInit             (ComlibLnkLst *lnkLst, UINT maxNode);
FT_PUBLIC RT_RESULT      comlib_lnkLstAppendFirst      (ComlibLnkLst *dst, ComlibLnkLst *src);
FT_PUBLIC RT_RESULT      comlib_lnkLstAppendTail       (ComlibLnkLst *dst, ComlibLnkLst *src);
FT_PUBLIC RT_RESULT      comlib_lnkLstInsertFirst      (ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode);
FT_PUBLIC RT_RESULT      comlib_lnkLstInsertTail       (ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode);
FT_PUBLIC RT_RESULT      comlib_lnkLstInsertNextNode   (ComlibLnkLst *lnkLst, ComlibLnkNode *org, ComlibLnkNode *node);
FT_PUBLIC RT_RESULT      comlib_lnkLstInsertPrevNode   (ComlibLnkLst *lnkLst, ComlibLnkNode *org, ComlibLnkNode *node);
FT_PUBLIC ComlibLnkNode* comlib_lnkLstGetFirst         (ComlibLnkLst *lnkLst);
FT_PUBLIC ComlibLnkNode* comlib_lnkLstGetTail          (ComlibLnkLst *lnkLst);
FT_PUBLIC RT_RESULT      comlib_lnkLstDel              (ComlibLnkLst *lnkLst, ComlibLnkNode *lnkNode);

/* Hash function */
FT_PUBLIC RT_RESULT      comlib_hashMurMur             (CONST VOID *key, CONST UINT keyLen, CONST U_32 seed, 
                                                        REGISTER UINT *rt_hashKey);
FT_PUBLIC RT_RESULT      comlib_hashStr                (CONST VOID *key, CONST UINT keyLen, REGISTER UINT *rt_hashKey);

/* Hash table public function */
FT_PUBLIC RT_RESULT      comlib_hashTblInit            (ComlibHashTbl *hashTbl, CONST UINT nmbEntry, CONST BOOL dupFlg, 
                                                        CONST UINT hashType, ComlibHashFunc func);
FT_PUBLIC RT_RESULT      comlib_hashTblDstry           (ComlibHashTbl *hashTbl);
FT_PUBLIC RT_RESULT      comlib_hashTblInsertHashNode  (ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, ComlibHashNode *node);
FT_PUBLIC RT_RESULT      comlib_hashTblDelHashNode     (ComlibHashTbl *hashTbl, ComlibHashNode *node);
FT_PUBLIC RT_RESULT      comlib_hashTblFindHashNode    (CONST ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, 
                                                        CONST UINT nodeId, ComlibHashNode **rt_node);
FT_PUBLIC RT_RESULT      comlib_hashTblGetHashNode     (ComlibHashTbl *hashTbl, CONST ComlibHashKey *key, CONST UINT nodeId, 
                                                        ComlibHashNode **rt_node);

/* Timer public function */
FT_PUBLIC RT_RESULT      comlib_timerInit              (ComlibTimer *tmr, UINT type);
FT_PUBLIC RT_RESULT      comlib_timerGetTime           (struct timespec *tm);
FT_PUBLIC RT_RESULT      comlib_timerTblInit           (ComlibTmrTbl *tbl, ComlibTimer *tmr, ComlibTmrEvtFunc func);
FT_PUBLIC RT_RESULT      comlib_timerTblHandler        (ComlibTmrTbl *tbl);
FT_PUBLIC RT_RESULT      comlib_timerTblStartTm        (ComlibTmrTbl *tbl, ComlibTmrNode *node, UINT event, UINT expTm);
FT_PUBLIC RT_RESULT      comlib_timerTblRestartTm      (ComlibTmrTbl *tbl, ComlibTmrNode *node, UINT expTm);
FT_PUBLIC RT_RESULT      comlib_timerTblCancelTm       (ComlibTmrTbl *tbl, ComlibTmrNode *node);

/* Message public function */
FT_PUBLIC RT_RESULT      comlib_msgGetMsg              (ComlibMsg **msg);
FT_PUBLIC RT_RESULT      comlib_msgPutMsg              (ComlibMsg **msg);
FT_PUBLIC RT_RESULT      comlib_msgGetMsgLen           (ComlibMsg **msg, UINT *len);
FT_PUBLIC RT_RESULT      comlib_msgAddCharFirst        (ComlibMsg **msg, CHAR data);
FT_PUBLIC RT_RESULT      comlib_msgAddCharEnd          (ComlibMsg **msg, CHAR data);
FT_PUBLIC RT_RESULT      comlib_msgAddMsgFirst         (ComlibMsg **msg, CHAR *data, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgAddMsgEnd           (ComlibMsg **msg, CHAR *data, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgAddMsg              (ComlibMsg **msg, UINT cur, CHAR *data, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgRmvCharFirst        (ComlibMsg **msg);
FT_PUBLIC RT_RESULT      comlib_msgRmvCharEnd          (ComlibMsg **msg);
FT_PUBLIC RT_RESULT      comlib_msgRmvMsgFirst         (ComlibMsg **msg, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgRmvMsgEnd           (ComlibMsg **msg, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgRmvMsg              (ComlibMsg **msg, UINT cur, UINT len);
FT_PUBLIC RT_RESULT      comlib_msgSpiltMsg            (ComlibMsg **msg1, UINT cur, ComlibMsg **msg2);
FT_PUBLIC RT_RESULT      comlib_msgCpyMsgIntoFixBuf    (ComlibMsg **msg, UINT cur, UINT len, CHAR *buf);
FT_PUBLIC RT_RESULT      comlib_msgCpyAllMsgIntoFixBuf (ComlibMsg **msg, CHAR *buf, UINT bufLen, UINT *rt_cpyLen);
FT_PUBLIC RT_RESULT      comlib_msgCpyAllMsgIntoDynBuf (ComlibMsg **msg, CHAR **buf, UINT *cpyLen);

/* String public function */
FT_PUBLIC UINT           comlib_strGetLen              (CONST CHAR *src);
FT_PUBLIC SINT           comlib_strCmp                 (CONST CHAR *s1, CONST CHAR *s2);
FT_PUBLIC SINT           comlib_strNCmp                (CONST CHAR *s1, CONST CHAR *s2, UINT len);
FT_PUBLIC SINT           comlib_strCaseCmp             (CONST CHAR *s1, CONST CHAR *s2);
FT_PUBLIC SINT           comlib_strCaseNCmp            (CONST CHAR *s1, CONST CHAR *s2, UINT len);
FT_PUBLIC RT_RESULT      comlib_strCpy                 (CHAR *dst, CONST CHAR *src);
FT_PUBLIC RT_RESULT      comlib_strNCpy                (CHAR *dst, CONST CHAR *src, UINT len);
FT_PUBLIC UINT           comlib_strSNPrnt              (CHAR *str, UINT size, CONST CHAR *fmt,...);
FT_PUBLIC CHAR           comlib_strToUpper             (CONST CHAR ch);
FT_PUBLIC RT_RESULT      comlib_strChgStrToUpper       (CONST CHAR *src, UINT srcLen, CHAR *rt_dst, UINT maxLen);
FT_PUBLIC CHAR           comlib_strToLower             (CONST CHAR ch);
FT_PUBLIC RT_RESULT      comlib_strChgStrToLower       (CONST CHAR *src, UINT srcLen, CHAR *rt_dst, UINT maxLen);
FT_PUBLIC VOID           comlib_strHexToStr            (U_8 val, CHAR *rslt);
FT_PUBLIC UINT           comlib_strAtoi                (CONST CHAR *src, UINT len);
FT_PUBLIC CHAR*          comlib_strFindChr             (CONST CHAR *in, CONST CHAR c);
FT_PUBLIC CHAR*          comlib_strFindStr             (CONST CHAR *src, CONST CHAR *str);
FT_PUBLIC SIZET          comlib_strCSpn                (CONST CHAR *src, CONST CHAR *chrSet);
FT_PUBLIC SIZET          comlib_strSpn                 (CONST CHAR *src, CONST CHAR *chrSet);

/* file function */
FT_PUBLIC RT_RESULT      comlib_fileFrceDir            (CONST CHAR *path, U_16 mode);

/* utility function */
FT_PUBLIC UINT           comlib_utilAtoi               (CONST CHAR *src, UINT len);
FT_PUBLIC CHAR*          comlib_utilItoaU32            (U_32 val, CHAR* buf);
FT_PUBLIC CHAR*          comlib_utilItoaS32            (S_32 val, CHAR* buf);
FT_PUBLIC CHAR*          comlib_utilItoaU64            (U_64 val, CHAR* buf);
FT_PUBLIC CHAR*          comlib_utilItoaS64            (S_64 val, CHAR* buf);

/* file function */
FT_PUBLIC RT_RESULT      comlib_fileMkdir              (CONST CHAR *path, CONST U_16 mode);
FT_PUBLIC RT_RESULT      comlib_fileFrceDir            (CONST CHAR *path, CONST U_16 mode);

#ifdef __cplusplus
}
#endif

#endif /* _COMLIB_X_ */

