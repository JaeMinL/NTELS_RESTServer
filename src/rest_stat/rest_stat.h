#ifndef _REST_STAT_H_
#define _REST_STAT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AuthMngMainCb        AuthMngMainCb;
typedef struct AuthMngCb          	AuthMngCb;
typedef struct TokenHashNode 		TokenHashNode;

struct AuthMngMainCb{
    pthread_rwlock_t          		tokenMainMutx;
    ComlibHashTbl                   tokenHashTbl;
    /*
    ComlibLnkNode                   LnkNode;
    ComlibLnkLst                    Lst;
    */
};

struct AuthMngCb{
    AuthMngMainCb                   main;
};

struct TokenHashNode{
    CHAR* 							accTok;
    UINT 							exp_in;
    ComlibHashNode					hNode;
};

FT_PUBLIC RT_RESULT ChDate(CONST CHAR *date_old, CHAR *date_store);
FT_PUBLIC RT_RESULT DbResult(CHAR *query, RsvlibSesCb *sesCb, CONST CHAR *who);
FT_PUBLIC RT_RESULT MakeQuery(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);

/*authMng*/
FT_PUBLIC RT_RESULT userAuth(RsvlibSesCb *sesCb, AuthMngCb *authMngCb);

/*tokenCache*/
FT_PUBLIC RT_RESULT tokenCacheDestroy();
FT_PUBLIC RT_RESULT tokenCacheCheck(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen);
FT_PUBLIC RT_RESULT tokenNodeInsert(RsvlibSesCb *sesCb, ComlibHashTbl *tokenHashTbl, CHAR *accTok, UINT accTokLen, UINT exp_in);
FT_PUBLIC RT_RESULT tokenCacheDel();

/*authlib_init*/
FT_PUBLIC RT_RESULT initAuthMngCb(AuthMngCb* authMngCb);
FT_PUBLIC RT_RESULT initAuthMngMainCb(AuthMngMainCb* authMngMainCb);

#define MAIN_CFG_PATH_LEN               256

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr);


#ifdef __cplusplus
}
#endif

#endif /* _REST_STAT_H_ */