#ifndef _REST_STAT_H_
#define _REST_STAT_H_

FT_PUBLIC RT_RESULT ChDate(CONST CHAR *date_old, CHAR **date_new);

#ifdef __cplusplus
extern "C" {
#endif

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr);

#ifdef __cplusplus
}
#endif

#endif /* _REST_STAT_H_ */
