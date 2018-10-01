#ifndef _HTPLIB_H_
#define _HTPLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* http libary log level */
#define HTP_NONE                         0
#define HTP_ERR                          1
#define HTP_NOTY                         2
#define HTP_DBG                          3

#define HTPERR_INVELID_STA_CODE          100
#define HTPERR_INVELID_STA_CODE_LEN      101

#define HTPERR_INVALID_MTHOD_LEN         200
#define HTPERR_INVALID_MTHOD_FIRST_CHR   201
#define HTPERR_MTHOD_BKT_NOT_EXIST       202
#define HTPERR_INVALID_MTHOD             203

#define HTPLIB_MAX_MTHOD_BKT_LEN         8
#define HTPLIB_MAX_ALPHABET_LEN          26
#define HTPLIB_MTHOD_CNT                 9 
#define HTPLIB_MTHOD_EMPTY_BKT           0xff

#define HTPLIB_STA_CODE_MAPPER_LEN       0x599 /* BCD (100 ~ 599) ex) 503 = [0x5, 0x0, 0x3](1283) */

#define HTPLIB_MHOTD_ID_NONE             0 /* NONE*/
#define HTPLIB_MHOTD_ID_GET              1 /* GET */
#define HTPLIB_MHOTD_ID_HEAD             2 /* HEAD */
#define HTPLIB_MHOTD_ID_POST             3 /* POST */
#define HTPLIB_MHOTD_ID_PUT              4 /* PUT */
#define HTPLIB_MHOTD_ID_DEL              5 /* DELETE */
#define HTPLIB_MHOTD_ID_CON              6 /* CONNECT */
#define HTPLIB_MHOTD_ID_OPT              7 /* OPTIONS */
#define HTPLIB_MHOTD_ID_TRC              8 /* TRACE */
#define HTPLIB_MHOTD_ID_PTH              9 /* PATCH */

/* HTP library log */
#ifdef HTPLIB_LOG
#define HTP_LOG(LEVEL,...){\
        printf(__VA_ARGS__);\
}
#else
#define HTP_LOG(LEVEL,...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* _HTPLIB_H_ */

