#ifndef _REST_STAT_H_
#define _REST_STAT_H_

FT_PUBLIC RT_RESULT ChDate(CONST CHAR *date_old, CHAR *date_store);
FT_PUBLIC RT_RESULT DbResult(CHAR * query, RsvlibSesCb *sesCb, CONST CHAR *who);
FT_PUBLIC RT_RESULT MakeQuery(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);


#ifdef __cplusplus
extern "C" {
#endif

#define MAIN_CFG_PATH_LEN               256

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr);

#ifdef __cplusplus
}
#endif

#endif /* _REST_STAT_H_ */
