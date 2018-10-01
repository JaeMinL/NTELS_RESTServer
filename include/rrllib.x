/************************************************************************

     Name:     REST Rule library

     Type:     C include file

     Desc:     REST Rule library structure and function prototype 

     File:     rrllib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _RRLLIB_X_
#define _RRLLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

/* rest rule library */

typedef struct RrllibResMthod    RrllibResMthod;
typedef struct RrllibResPath     RrllibResPath;
typedef struct RrllibResPathLst  RrllibResPathLst;
typedef struct RrllibCb          RrllibCb;
typedef struct RrllibQuery       RrllibQuery;
typedef struct RrllibQueryDocCfg RrllibQueryDocCfg;
typedef struct RrllibDocArgVal   RrllibDocArgVal;
typedef struct RrllibDocArg      RrllibDocArg;
typedef struct RrllibDoc         RrllibDoc;
typedef struct RrllibGlobCb      RrllibGlobCb;

/* QueryFunc(key, keyLen, value, valueLen, user arg) */
typedef RT_RESULT (*RrllibQueryKvpFunc)      (CONST CHAR*, UINT, CONST CHAR*, UINT, VOID*); 
/* PrntUsrArgFunc(usrArg, depth) */
typedef RT_RESULT (*RrllibPrntUsrArgFunc)    (VOID*, UINT );
/* display(logStr) */
typedef RT_RESULT (*RrllibDispFunc)          (CHAR*); 
/* logFunc(level, file, line, logStr) */
typedef VOID      (*RrllibLogFunc)           (UINT, CHAR*, UINT, CHAR*); 

struct RrllibDocArgVal{
    UINT                    valLen;
    CHAR                    val[RRL_DOC_ARG_VAL_LEN];
    ComlibLnkNode           lnkNode;
};

struct RrllibDocArg{
    UINT                    nameLen;
    CHAR                    name[RRL_DOC_ARG_NAME_LEN];
    ComlibLnkNode           *curPtr; /* current selected value pointer */
    ComlibLnkLst            valLL; /* multi value (ex. qeuery, /test?ip=127.0.0.1&ip=127.0.0.2 */
    ComlibLnkNode           lnkNode;
};

struct RrllibDoc{
    UINT                    mthod;
    VOID                    *usrArg;
    UINT                    mandCnt;
    ComlibLnkNode           *curPtr;
    ComlibLnkLst            argLL;
};

struct RrllibResMthod{
    UINT                    mthod;
    VOID                    *arg;
    UINT                    mandCnt;
    RrllibCb                *root;
    ComlibLnkLst            queryLL;
    ComlibLnkNode           lnkNode;
};

struct RrllibResPath{
    UINT                    nameLen;
    CHAR                    name[RRL_PATH_NAME_LEN];
    UINT                    keyLen;
    CHAR                    key[RRL_PATH_NAME_LEN]; /* path name length(Upper case) */
    RrllibCb                *root;
    RrllibResPathLst        *own;
    RrllibResPathLst        *nxtPathLst;
    ComlibLnkLst            mthodLL;
    ComlibLnkNode           lnkNode;
    ComlibHashNode          hNode;
};

struct RrllibResPathLst{
    UINT                    pathType;
    RrllibCb                *root;
    RrllibResPath           *own;
    ComlibLnkLst            resPathLL;
    ComlibHashTbl           resPathHT;
};

struct RrllibQuery{
    UINT                    maxCnt;
    BOOL                    mandFlg;
    UINT                    nameLen;
    CHAR                    name[RRL_QUERY_NAME_LEN];
    ComlibLnkNode           lnkNode;
};

struct RrllibQueryDocCfg{
    RrllibResMthod          *resMthod;
    RrllibDoc               *doc;
};

struct RrllibCb{
    UINT                    caseFlg; /* RC_TRUE : check upper and lower case */
    RrllibResPathLst        *resPathLst;
};

struct RrllibGlobCb{
    UINT                    logLvl;
    UINT                    tmpBufLen;
    CHAR                    tmpBuf[RRLLIB_MAX_TMP_BUF_LEN];
    RrllibLogFunc           logFunc;
    RrllibDispFunc          dispFunc;
};

/* rrllib_glob */
FT_PUBLIC RT_RESULT   rrllib_globGetLogLvl         ();
FT_PUBLIC BOOL        rrllib_globGetInitFlg        ();
FT_PUBLIC RT_RESULT   rrllib_globSetLogFunc        (UINT lvl, RrllibLogFunc logFunc);
FT_PUBLIC RT_RESULT   rrllib_globLogPrnt           (UINT lvl, CHAR *file, UINT line, CONST CHAR *fmt,...);
FT_PUBLIC RT_RESULT   rrllib_globInit              ();
FT_PUBLIC RT_RESULT   rrllib_globDispPrnt          (CONST CHAR *fmt,...);
FT_PUBLIC RT_RESULT   rrllib_globSetDispFunc       (RrllibDispFunc dispFunc);

/* rrllib_main */
FT_PUBLIC RT_RESULT   rrllib_mainInit              (RrllibCb *rrlCb, BOOL caseFlg);
FT_PUBLIC RT_RESULT   rrllib_mainPrnt              (RrllibCb *rrlCb, RrllibPrntUsrArgFunc func);
FT_PUBLIC RT_RESULT   rrllib_mainDstry             (RrllibCb *rrlCb);

/* rrllib_path */
FT_PUBLIC RT_RESULT   rrllib_pathChkDynPath        (RrllibResPathLst *resPathLst, CHAR *name, UINT nameLen, 
                                                    RrllibResPath **rt_resPath);
FT_PUBLIC RT_RESULT   rrllib_pathAddFirstFixPathLst(RrllibCb *rrlCb, RrllibResPathLst **rt_resPathLst);
FT_PUBLIC RT_RESULT   rrllib_pathAddFirstDynPath   (RrllibCb *rrlCb, CONST CHAR *name, UINT nameLen, RrllibResPath **rt_resPath);
FT_PUBLIC RT_RESULT   rrllib_pathAddNxtFixPathLst  (RrllibResPath *resPath, RrllibResPathLst **rt_resPathLst);
FT_PUBLIC RT_RESULT   rrllib_pathAddFixPath        (RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, 
                                                    RrllibResPath **rt_resPath);
FT_PUBLIC RT_RESULT   rrllib_pathAddNxtDynPath     (RrllibResPath *resPath, CONST CHAR *name, UINT nameLen, 
                                                    RrllibResPath **rt_resPath);
FT_PUBLIC RT_RESULT   rrllib_pathFindFixPath       (RrllibResPathLst *resPathLst, CONST CHAR *name, UINT nameLen, 
                                                    RrllibResPath **rt_resPath);
FT_PUBLIC RT_RESULT   rrllib_pathPrntPath          (RrllibResPath *resPath, UINT pathType, UINT depth, RrllibPrntUsrArgFunc func);
FT_PUBLIC RT_RESULT   rrllib_pathPrntPathLst       (RrllibResPathLst *resPathLst, UINT depth, RrllibPrntUsrArgFunc func);
FT_PUBLIC RT_RESULT   rrllib_pathDstryPath         (RrllibResPathLst *resPathLst, RrllibResPath *resPath);
FT_PUBLIC RT_RESULT   rrllib_pathDstryPathLst      (RrllibResPathLst *resPathLst);

/* rrlib_mthod */
FT_PUBLIC RT_RESULT   rrllib_mthodAdd              (RrllibResPath *resPath, UINT mthod, VOID *arg, RrllibResMthod **rt_resMthod);;
FT_PUBLIC CHAR*       rrllib_mthodGetStr           (UINT mthod);
FT_PUBLIC RT_RESULT   rrllib_mthodPrntMthod        (RrllibResMthod *resMthod, UINT depth, RrllibPrntUsrArgFunc func);
FT_PUBLIC RT_RESULT   rrllib_mthodDstry            (RrllibResMthod *resMthod);

/* rrllib_doc */
FT_PUBLIC RT_RESULT   rrllib_docGetNewDoc          (RrllibDoc **rt_doc);
FT_PUBLIC RT_RESULT   rrllib_docAddArg             (RrllibDoc *doc, CHAR *name, UINT nameLen, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT   rrllib_docGetFirstArg        (RrllibDoc *doc, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT   rrllib_docGetNxtArg          (RrllibDoc *doc, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT   rrllib_docFindArg            (RrllibDoc *doc, CHAR *name, UINT nameLen, RrllibDocArg **rt_docArg);
FT_PUBLIC RT_RESULT   rrllib_docAddArgVal          (RrllibDocArg *docArg, CONST CHAR *val, UINT valLen);
FT_PUBLIC RT_RESULT   rrllib_docGetFirstVal        (RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT   rrllib_docGetNxtVal          (RrllibDocArg *docArg, CONST CHAR **rt_val, UINT *rt_valLen);
FT_PUBLIC RT_RESULT   rrllib_docDstry              (RrllibDoc *doc);
FT_PUBLIC RT_RESULT   rrllib_docArgDstry           (RrllibDocArg *docArg);
FT_PUBLIC RT_RESULT   rrllib_docArgPrnt            (RrllibDocArg *docArg);
FT_PUBLIC RT_RESULT   rrllib_docPrnt               (RrllibDoc *doc);

/* rrllib_query */
FT_PUBLIC RT_RESULT   rrllib_queryAdd              (RrllibResMthod *resMthod, BOOL mandFlg, CONST CHAR *name, UINT nameLen, 
                                                    UINT maxCnt);
FT_PUBLIC RT_RESULT   rrllib_queryFind             (RrllibResMthod *resMthod, CONST CHAR *name, UINT nameLen, 
                                                    RrllibQuery **rt_query);
FT_PUBLIC RT_RESULT   rrllib_queryPrnt             (RrllibQuery *query);
FT_PUBLIC RT_RESULT   rrllib_queryDstry            (RrllibQuery *query);

/* rrllib_parse */
FT_PUBLIC RT_RESULT   rrllib_parseUriFull          (RrllibCb *rrlCb, UINT mthod, CONST CHAR *uri, UINT uriLen, 
                                                    RrllibDoc **rt_doc);
FT_PUBLIC RT_RESULT   rrllib_parseUriPath          (RrllibCb *rrlCb, UINT mthod, CONST CHAR *uri, UINT uriLen, 
                                                    RrllibResMthod **rt_resMthod, RrllibDoc **rt_doc);
FT_PUBLIC RT_RESULT   rrllib_parseUriQuery         (CONST CHAR *query, UINT queryLen, RrllibQueryKvpFunc func, VOID *arg);
FT_PUBLIC RT_RESULT   rrllib_parseKvpToDoc         (CONST CHAR *key, UINT keyLen, CONST CHAR *val, UINT valLen,
                                                    RrllibQueryDocCfg *queryDocCfg);
FT_PUBLIC RT_RESULT   rrllib_parseResCfg           (RrllibCb *rrlCb, CONST CHAR *res, UINT resLen, UINT mthod, 
                                                    CONST CHAR *query, UINT queryLen, VOID *usrArg);

#ifdef __cplusplus
}
#endif

#endif /* _RESTRULE_X_ */

