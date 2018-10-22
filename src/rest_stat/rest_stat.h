#ifndef _REST_STAT_H_
#define _REST_STAT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AuthMngMainCb        AuthMngMainCb;
typedef struct AuthMngCb          	AuthMngCb;
typedef struct TokenNode            TokenNode;

struct AuthMngMainCb{
    pthread_rwlock_t          		tokenMainMutx;
    ComlibHashTbl                   tokenHashTbl;
    ComlibTmrTbl                    tokenTmrTbl;
    ComlibTimer                     tmr;
};

struct AuthMngCb{
    AuthMngMainCb                   main;
};

struct TokenNode{
    CHAR* 							accTok;
    UINT 							exp_in;
    UINT                            event;
    ComlibHashNode					hNode;
    ComlibTmrNode                   tNode;
    AuthMngMainCb*                  ownMngMainCb;
};

/**/
FT_PUBLIC RT_RESULT rss_dateChange(CONST CHAR *date_old, CHAR *date_store);
FT_PUBLIC RT_RESULT rss_connDBGetResult(CHAR *query, RsvlibSesCb *sesCb, CONST CHAR *who);
FT_PUBLIC RT_RESULT rss_queryMake(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);

/*tokenCache*/
FT_PUBLIC RT_RESULT rss_tokenNodeCheck(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen);
FT_PUBLIC RT_RESULT rss_tokenNodeInsert(RsvlibSesCb *sesCb, AuthMngMainCb *authMngMain, CHAR *accTok, UINT accTokLen, UINT exp_in);
FT_PUBLIC RT_RESULT rss_tokenNodeDel(ComlibHashTbl *tokenHashTbl, ComlibHashNode *hNode);
FT_PUBLIC RT_RESULT rss_tokenNodeDelAll(AuthMngMainCb* authMngMainCb);

/*authlib_init*/
RT_RESULT rss_tmrEvtFunc(UINT event, VOID *data);
FT_PUBLIC RT_RESULT rss_authInit(AuthMngCb* authMngCb);
FT_PUBLIC RT_RESULT rss_authInitMain(AuthMngMainCb* authMngMainCb);
FT_PUBLIC RT_RESULT rss_authDstry(AuthMngCb* authMngCb);
FT_PUBLIC RT_RESULT rss_authDstryMain(AuthMngMainCb* authMngMainCb);
FT_PUBLIC RT_RESULT rss_authSvr(RsvlibSesCb *sesCb, AuthMngCb *authMngCb);

#define MAIN_CFG_PATH_LEN               256

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr);


#ifdef __cplusplus
}
#endif

#endif /* _REST_STAT_H_ */