/************************************************************************

     Name:     Wildcard library

     Type:     C include file

     Desc:     Wildcard library definition and macro

     File:     wldlib.h

     Prg:      Son Lee-Suk

************************************************************************/
#ifndef _WLDLIB_H_
#define _WLDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WLDLIB_BLK_TYPE_NONE           0
#define WLDLIB_BLK_TYPE_FIXED          1
#define WLDLIB_BLK_TYPE_WLDCARD        2
#define WLDLIB_BLK_TYPE_UNI_WLDCARD    3

#define WLDLIB_DFLT_CHR_LST            256

#define WLDERR_LNKLST_INIT_FAILED      100 
#define WLDERR_HASHTBL_INIT_FAILED     101
#define WLDERR_INVALID_STR_LEN         102
#define WLDERR_INVLAID_DYNREG_FLG      103
#define WLDERR_HASHTBL_INSERT_FAILED   104
#define WLDERR_LNKLST_INSERT_FAILED    105
#define WLDERR_CHR_CB_ALREADY_EXIST    107
#define WLDERR_UNI_CHR_CNT_NOT_EXSIT   108
#define WLDERR_NOT_EXIST               109

#define WLD_ERR                        1
#define WLD_NOTY                       2
#define WLD_DBG                        3

#ifdef WLDLIB_LOG
#include <stdio.h>
#define WLD_LOG(LEVEL,...){\
    printf("[%s:%d] ",__FILE__, __LINE__);\
    printf(__VA_ARGS__);\
}
#else 
#define WLD_LOG(LEVEL,...)
#endif

#define WLD_DISP(...){\
    printf(__VA_ARGS__);\
}

#ifdef __cplusplus
}
#endif

#endif /* _WLDLIB_H_ */

