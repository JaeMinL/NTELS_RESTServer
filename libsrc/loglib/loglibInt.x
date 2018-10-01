/************************************************************************

     Name:     Log library 

     Type:     C structure file

     Desc:     Log internal library structure

     File:     loglibInt.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _LOGLIB_INT_X_
#define _LOGLIB_INT_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoglibIntThrdMsgHdr    LoglibIntThrdMsgHdr;
typedef struct LoglibIntThrdMsgAvpHdr LoglibIntThrdMsgAvpHdr;
typedef struct LoglibIntThrdCb        LoglibIntThrdCb;
typedef struct LoglibIntApndCb        LoglibIntApndCb;
typedef struct LoglibIntApndInfo      LoglibIntApndInfo;
typedef struct LoglibIntMainCb        LoglibIntMainCb;

struct LoglibIntThrdMsgHdr{
    UINT                        cmdCode;
    UINT                        msgLen;
    struct tm                   msgTms;
};

struct LoglibIntThrdMsgAvpHdr{
    UINT                        cmdCode;
    UINT                        msgLen;
};

struct LoglibIntApndCb{ /* append control block */
    UINT                        nameLen;
    CHAR                        name[LOGLIB_APND_NAME_MAX_LEN];
    BOOL                        logLvlBit; 
    U_32                        prevDispBit; /* previous dispBit */
    U_32                        dispBit; /* file, line, time */
    UINT                        apndType;
    union {
        struct {
            FILE                *fp;
            struct tm           lastTms;
            UINT                logPathLen;
            CHAR                logPath[LOGLIB_LOG_PATH_MAX_LEN];
            UINT                nameLen;
            CHAR                name[LOGLIB_LOG_NAME_MAX_LEN];
            UINT                maxLogSize;
            UINT                logSize;
        }file;
        struct {
            UINT                type;
        } stdout;
        struct {
            UINT                fac; /* facility */
        }syslog;
        struct {
            LoglibUsrLogFunc    usrLogFunc;
        }usr;
    }u;
    ComlibLnkNode               lnkNode;
};

struct LoglibIntApndInfo{
    ComlibLnkLst                apndLL;
    CHAR                        logBuf[LOGLIB_LOG_MAX_BUF_LEN];
};

struct LoglibIntThrdCb{
    ThrlibMutx                  mutx;
    ThrlibCond                  cond;
    ThrlibThrdId                tid;
    ThrlibTq                    rcvTq;
    LoglibIntApndInfo           apndInfo;
};

struct LoglibIntMainCb{
    ThrlibMutx                  mutx;
    UINT                        wrType; /* 1 : WRITE DIRECT , 2 : WRITE BY THREAD*/
    union {
        struct {
            ThrlibTq            *sndTq;
            ThrlibCond          *cond;
            LoglibIntThrdCb     *thrdCb;
        }thrd;
        struct {
            LoglibIntApndInfo   apndInfo;
        }dir;
    }u;
};

/* loglibInt_glob.c */
FT_PUBLIC U_32             loglibInt_globGetDispEnv    ();
FT_PUBLIC VOID             loglibInt_globSetDispEnv    (UINT bit);

/* loglibInt_init.c */
FT_PUBLIC RT_RESULT        loglibInt_init              (LoglibIntMainCb *mainCb, LoglibCfg *cfg);

/* loglibInt_apnd.c */
FT_PUBLIC RT_RESULT        loglibInt_apndSetPrevDispBit(LoglibIntApndCb *apndCb);
FT_PUBLIC U_32             loglibInt_apndGetDispBit    (LoglibIntApndCb *apndCb);
FT_PUBLIC RT_RESULT        loglibInt_apndSetDispBit    (LoglibIntApndCb *apndCb, U_32 bit);
FT_PUBLIC RT_RESULT        loglibInt_apndMakeFile      (LoglibIntApndCb *apndCb, CHAR *usrLog, UINT usrLogLen);
FT_PUBLIC RT_RESULT        loglibInt_apndDel           (LoglibIntApndInfo *apndInfo, LoglibIntApndCb *apndCb);
FT_PUBLIC RT_RESULT        loglibInt_apndDelAll        (LoglibIntApndInfo *apndInfo);
FT_PUBLIC RT_RESULT        loglibInt_apndFind          (LoglibIntApndInfo *apndInfo, CHAR *name, UINT nameLen, 
                                                        LoglibIntApndCb **rt_apndCb);
FT_PUBLIC RT_RESULT        loglibInt_apndMake          (LoglibIntApndCb *apndCb);
FT_PUBLIC RT_RESULT        loglibInt_apndWrite         (LoglibIntApndCb *apndCb, UINT lvl, CONST CHAR *logBuf, struct tm *curTms);
FT_PUBLIC RT_RESULT        loglibInt_apndInit          (UINT type, CHAR *name, LoglibApndCfg *apndCfg, 
                                                        LoglibIntApndCb **rt_apndCb);

/* loglibInt_thrd.c */
FT_PUBLIC RT_RESULT        loglibInt_thrdLogWrite      (LoglibIntApndInfo *apndInfo, UINT lvl, CONST CHAR *fName, UINT fNameLen, 
                                                        UINT line, CONST CHAR *logBuf, UINT logBufLen, struct tm *curTms);
FT_PUBLIC VOID             loglibInt_thrdMain          (VOID *args);

/* loglibInt_misc.c */
FT_PUBLIC CHAR*            loglibInt_miscGetStrLvl     (UINT lvl);
FT_PUBLIC RT_RESULT        loglibInt_msicPrntHdr       (LoglibIntApndCb *apndCb, UINT lvl, struct tm *curTms, 
                                                        CONST CHAR *fName, UINT fNameLen, UINT line, 
                                                        CHAR *rt_logBuf, UINT maxLogBufLen, UINT *rt_logBufLen);

/* loglibInt_cfg.c */
FT_PUBLIC RT_RESULT        loglibInt_cfgLoadCfg        (LoglibCb *loglibCb, CHAR *cfgFile, CHAR *name);

#ifdef __cplusplus
}
#endif

#endif /* _LOGLIB_INT_X_ */

