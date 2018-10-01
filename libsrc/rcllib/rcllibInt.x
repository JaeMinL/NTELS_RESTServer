#ifndef _RCLLIB_INT_X_
#define _RCLLIB_INT_X_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RcllibIntCb       RcllibIntCb;

struct RcllibIntCb{
    CURL               *hdlr;
    struct curl_slist  *reqHdrLst;
    ComlibMsg          *rspMsg;
    VOID               *usrArg;
};

FT_PUBLIC RT_RESULT      rcllibInt_globInit      ();

#ifdef __cplusplus
}
#endif

#endif /* _RCLLIB_INT_X_ */

