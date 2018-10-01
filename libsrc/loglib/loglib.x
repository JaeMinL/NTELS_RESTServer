/************************************************************************

     Name:     Log library 

     Type:     C structure file

     Desc:     Log library structure

     File:     loglib.x

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _LOGLIB_X_
#define _LOGLIB_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoglibCfg        LoglibCfg;
typedef struct LoglibApndCfg    LoglibApndCfg;
typedef struct LoglibIntMainCb  *LoglibMainCb;
typedef struct LoglibCb         LoglibCb;

typedef VOID (*LoglibUsrLogFunc)  (struct tm*, UINT, CONST CHAR*, UINT, CHAR*); /* logFunc(level, file, line, logStr) */

/*
 * == SYSLOG PRIORITY ==
 * -Priority 
 *   LOG_EMERG      0           system is unusable
 *   LOG_ALERT      1           action must be taken immediately
 *   LOG_CRIT       2           critical conditions
 *   LOG_ERR        3           error conditions
 *   LOG_WARNING    4           warning conditions
 *   LOG_NOTICE     5           normal but significant condition
 *   LOG_INFO       6           informational
 *   LOG_DEBUG      7           debug-level messages
 *  
 * -Facility
 *   LOG_KERN       (0<<3)      kernel messages
 *   LOG_USER       (1<<3)      random user-level messages
 *   LOG_MAIL       (2<<3)      mail system
 *   LOG_DAEMON     (3<<3)      system daemons
 *   LOG_AUTH       (4<<3)      security/authorization messages
 *   LOG_SYSLOG     (5<<3)      messages generated internally by syslogd
 *   LOG_LPR        (6<<3)      line printer subsystem
 *   LOG_NEWS       (7<<3)      network news subsystem
 *   LOG_UUCP       (8<<3)      UUCP subsystem
 *   LOG_CRON       (9<<3)      clock daemon
 *   LOG_AUTHPRIV   (10<<3)     security/authorization messages (private)
 *   LOG_FTP        (11<<3)     ftp daemon
 *  
 * -Reserved facility
 *   LOG_LOCAL0     (16<<3)     reserved for local use
 *   LOG_LOCAL1     (17<<3)     reserved for local use
 *   LOG_LOCAL2     (18<<3)     reserved for local use
 *   LOG_LOCAL3     (19<<3)     reserved for local use
 *   LOG_LOCAL4     (20<<3)     reserved for local use
 *   LOG_LOCAL5     (21<<3)     reserved for local use
 *   LOG_LOCAL6     (22<<3)     reserved for local use
 *   LOG_LOCAL7     (23<<3)     reserved for local use
 */
struct LoglibApndCfg{
    U_32                       dispBit;
    U_32                       logLvlBit;
    union {
        struct {
            CHAR               *logPath;
            CHAR               *name;
            UINT               maxLogSize;
        }file; /* file */
        struct {
            UINT               type;
        }stdout; /* standard output */
        struct {
            UINT               fac; /* facility */
        }syslog; /* syslog */ 
        struct {
            CHAR               *locName;
            TrnlibTransAddr    trnsAddr;
        }rmt; /* remote */
        struct {
            LoglibUsrLogFunc   usrLogFunc;
        }usr; /* user function */
    }u;
};

struct LoglibCfg{
    UINT                       dfltLogLvl;
    UINT                       wrType; /* default : DIRECT */
    UINT                       dfltApndType;
    LoglibApndCfg              dfltApndCfg;
};

struct LoglibCb{
    UINT                       logLvl;
    LoglibMainCb               mainCb;
};

FT_PUBLIC RT_RESULT       loglib_apiGlobInit            (UINT bit);
FT_PUBLIC RT_RESULT       loglib_apiLoadCfg             (LoglibCb *loglibCb, CHAR *cfgFile, CHAR *name);
FT_PUBLIC RT_RESULT       loglib_apiInitLoglibCb        (LoglibCb *loglibCb, LoglibCfg *cfg);
FT_PUBLIC RT_RESULT       loglib_apiDstryLoglibCb       (LoglibCb *loglibCb);
FT_PUBLIC RT_RESULT       loglib_apiRegApnd             (LoglibCb *loglibCb, CHAR *name, UINT type, LoglibApndCfg *apndCfg);
FT_PUBLIC RT_RESULT       loglib_apiDeregApnd           (LoglibCb *loglibCb, CHAR *name);
FT_PUBLIC RT_RESULT       loglib_apiSetApndLogLvl       (LoglibCb *loglibCb, CHAR *name, U_32 logLvlBit);
FT_PUBLIC RT_RESULT       loglib_apiSetDispHdr          (LoglibCb *loglibCb, CHAR *apndName, U_32 dispBit);
FT_PUBLIC RT_RESULT       loglib_apiSetDispHdrToPrev    (LoglibCb *loglibCb, CHAR *apndName);
FT_PUBLIC RT_RESULT       loglib_apiGetDispHdr          (LoglibCb *loglibCb, CHAR *apndName, U_32 *rt_dispBit);
FT_PUBLIC RT_RESULT       loglib_apiSetAllDispHdr       (LoglibCb *loglibCb, U_32 dispBit);
FT_PUBLIC RT_RESULT       loglib_apiSetAllDispHdrToPrev (LoglibCb *loglibCb);
FT_PUBLIC U_32            loglib_apiGetDfltDispHdr      (LoglibCb *loglibCb);
FT_PUBLIC RT_RESULT       loglib_apiSetLogLvl           (LoglibCb *loglibCb, UINT lvl);
FT_PUBLIC UINT            loglib_apiGetLogLvl           (LoglibCb *loglibCb);
FT_PUBLIC RT_RESULT       loglib_apiLogWrite            (LoglibCb *loglibCb, UINT lvl, CONST CHAR *fName, UINT line, 
                                                         CONST CHAR *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _LOGLIB_X_ */

