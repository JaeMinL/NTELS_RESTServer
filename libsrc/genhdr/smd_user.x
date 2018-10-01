#ifndef _SMD_USER_X_
#define _SMD_USER_X_

typedef struct SmdMsgComHdr SmdMsgComHdr;
typedef struct SmdMsgRegReq SmdMsgRegReq;
typedef struct SmdMsgRegCfm SmdMsgRegCfm;
typedef struct SmdMsgHBReq  SmdMsgHBReq;
typedef struct SmdMsgHBCfm  SmdMsgHBCfm;
typedef struct SmdMsgTermReq SmdMsgTermReq;
typedef struct SmdMsgTermCfm SmdMsgTermCfm;
typedef struct SmdMsgTermInd SmdTermInd;
typedef struct SmdMsgTermRsp SmdMsgTermRsp;

/* common message header */
struct SmdMsgComHdr{
        U_32 cmdCode;
            U_32 msgLen;
};

/* APP(UP) -> SMD(LO) */
struct SmdMsgRegReq{
        U_32 pid;
            UINT nameLen;
                CHAR name[SMD_PROC_NAME_MAX_LEN];
};

/* SMD -> APP */
struct SmdMsgRegCfm{
        U_32 rsltCode;
};

/* APP -> SMD */
struct SmdMsgHBReq{
        U_32 pid;
};

/* SMD -> APP */
struct SmdMsgHBCfm{
        U_32 rsltCode;
};

/* APP -> SMD */
struct SmdMsgTermReq{
        U_32 pid;
};

/* SMD -> APP */
struct SmdMsgTermCfm{
        U_32 rsltCode;
};


/* SMD -> APP */
struct SmdMsgTermInd{
        U_32 pid;
            U_32 sig;
};

/* APP -> SMD */
struct SmdMsgTermRsp{
        U_32 rsltCode;
};

#endif /* _SMD_USER_X_ */

