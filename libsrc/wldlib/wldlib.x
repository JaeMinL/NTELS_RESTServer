/************************************************************************

     Name:     Wildcard library

     Type:     C structure file

     Desc:     Wildcard library structure

     File:     wldlib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _WLDLIB_X_
#define _WLDLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WldlibChrCb           WldlibChrCb;
typedef struct WldlibWldChrSet       WldlibWldChrSet;
typedef struct WldlibBlkLst          WldlibBlkLst;
typedef struct WldlibFullMatchBkt    WldlibFullMatchBlk;
typedef struct WldlibFullMatchLst    WldlibFullMatchLst;
typedef struct WldlibCb              WldlibCb;

struct WldlibChrCb{
    UINT                      blkType; /* 1 : fixed, 2 : wildcard  3: uni whildcard */
    union{
        struct {
            BOOL              dynRegFlg;
            UINT              strLen;
            CHAR              *str;
            ComlibHashNode    hNode;
        } fix;
        struct {
            UINT              cnt;
            CHAR              lstChr;
            //ComlibLnkLst      ruleLst; /* WldlibWldChrRule  chrRule */
        } uni;
        struct {
            CHAR              lstChr; /* array */
            //ComlibLnkLst      ruleLst;  /* WldlibWldChrRule  chrRule */ 
        } wld;
    }u;
    VOID                      *usrArg;
    ComlibLnkLst              regFullMatchLL;
    ComlibLnkNode             lnkNode;
    WldlibBlkLst              *nxtLst;
};

struct WldlibWldChrSet{
    UINT                      lstChrCnt;
    UINT                      maxLstChr;
    UINT                      minChrCnt;
    CHAR                      *lstChrLst;
    WldlibChrCb               **chrCb;
    WldlibChrCb               *lstChrCb; /* any */
};

/* later */
#if 0
struct WldlibWldChrRule{
    BOOL matchFlg;
    UINT chrCnt;
    CHAR *lstChrLst;
    CHAR *lstChr;
    VOID *usrArg;
    ComlibLnkLst              regFullMatchLL;
};

struct WldlibWldRuleSet{
    ComlibLnkLst ruleLL;
};
#endif

struct WldlibBlkLst{
    /* full match */
    UINT                      strLen;
    ComlibHashTbl             fixHT;      /* priority : first */
    ComlibLnkLst              fixLL;
    /* uni wildcard */
    UINT                      maxCnt;
    UINT                      minCnt;
    ComlibLnkLst              uniWldLL;   /* priority : second */
    /* wildcard */
    WldlibWldChrSet           wldChrSet; /* priority : third */
    /* other */
    WldlibChrCb               *own;
    ComlibLnkNode             lnkNode;
};

struct WldlibFullMatchBkt{
    UINT                      strLen;
    CHAR                      *str;
    WldlibChrCb               *own;
    ComlibLnkNode             regNode;
    ComlibLnkNode             lstNode;
    ComlibHashNode            hNode;
};

struct WldlibFullMatchLst{
    ComlibHashTbl             fullMatchHT; /* find first */
    ComlibLnkLst              fullMatchLL; /* full match list */
};

struct WldlibCb{
    WldlibFullMatchLst        fullMatchLst; /* find first */
    WldlibBlkLst              *blkLst;     /* find second */
    ComlibLnkLst              freeFullMatchBktLst;
    ComlibLnkLst              freeBlkLst;
    ComlibLnkLst              freeWldChr;
};

/* wldlib_init.c */
FT_PUBLIC RT_RESULT wldlib_init                (WldlibCb *wldlibCb);

/* wldlib_main.c */
FT_PUBLIC RT_RESULT wldlib_mainPrnt            (WldlibCb *wldlibCb);

/* wldlib_blkLst.c */
FT_PUBLIC RT_RESULT wldlib_blkLstMakeFirst     (WldlibCb *wldlibCb, WldlibBlkLst **rt_blkLst);
FT_PUBLIC RT_RESULT wldlib_blkLstMakeNxt       (WldlibChrCb *chrCb, WldlibBlkLst **rt_blkLst);
FT_PUBLIC RT_RESULT wldlib_blkLstFindFixed     (WldlibBlkLst *blkLst, CHAR *str, WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstAddFixed      (WldlibBlkLst *blkLst, CHAR *str, UINT strLen, VOID *usrArg, 
                                                WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstAddWld        (WldlibBlkLst *blkLst, CHAR lstChr, UINT minCnt, VOID *usrArg, 
                                                WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstAddUni        (WldlibBlkLst *blkLst, UINT chrCnt, CHAR lstChr, VOID *usrArg, 
                                                WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstFindWld       (WldlibBlkLst *blkLst, CHAR *str, CHAR **rt_lstCur,  WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstInsrtFixed    (WldlibBlkLst *blkLst, WldlibChrCb *chrCb);
FT_PUBLIC RT_RESULT wldlib_blkLstSpilt         (WldlibBlkLst *blkLst, UINT strLen);
FT_PUBLIC RT_RESULT wldlib_blkLstPrnt          (UINT depth, WldlibBlkLst *blkLst);

/* wldlib_chr.c */
FT_PUBLIC RT_RESULT wldlib_chrMakeFix          (CHAR *str, UINT strLen, BOOL dynRegFlg, VOID *usrArg, WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_chrMakeUni          (UINT chrCnt, CHAR lstChr, VOID *usrArg, WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_chrMakeWld          (CHAR lstChr, VOID *usrArg, WldlibChrCb **rt_chrCb);
FT_PUBLIC RT_RESULT wldlib_chrPrnt             (UINT depth, WldlibChrCb *chrCb, BOOL treeFlg);

/* wldlib_parse.c */
FT_PUBLIC RT_RESULT wldlib_parseStr            (WldlibCb *wldlibCb, CHAR *str, VOID **rt_usrArg);
FT_PUBLIC RT_RESULT wldlib_parseRegRule        (WldlibCb *wldlibCb, CHAR *str, VOID *usrArg);


#ifdef __cplusplus
}
#endif

#endif /* _WLDLIB_X_ */

