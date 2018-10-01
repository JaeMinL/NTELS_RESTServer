#include <stdio.h>
#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

FT_PRIVATE RT_RESULT chr_init(WldlibChrCb *chrCb);

FT_PRIVATE RT_RESULT chr_init(WldlibChrCb *chrCb)
{
    SINT ret = RC_OK;

    chrCb->blkType = WLDLIB_BLK_TYPE_NONE;

    chrCb->usrArg = NULL;

    ret = comlib_lnkLstInit(&chrCb->regFullMatchLL, ~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Registered full match linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    chrCb->lnkNode.data = chrCb;

    ret = wldlib_blkLstMakeNxt(chrCb, NULL);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Next block make failed(ret=%d)\n",ret);
        return ret;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_chrMakeFix(CHAR *str, UINT strLen, BOOL dynRegFlg, VOID *usrArg, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;

    if(strLen == 0){
        WLD_LOG(WLD_ERR,"string length is zero\n");
        return WLDERR_INVALID_STR_LEN;
    }

    if((dynRegFlg != RC_TRUE) && 
       (dynRegFlg != RC_FALSE)){
        WLD_LOG(WLD_ERR,"Invalid dyn reg flag\n");
        return WLDERR_INVLAID_DYNREG_FLG;
    }

    chrCb = comlib_memMalloc(sizeof(WldlibChrCb));

    ret = chr_init(chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Char control block init failed(ret=%d)\n",ret);
        comlib_memFree(chrCb);
        return ret;
    }

    chrCb->blkType = WLDLIB_BLK_TYPE_FIXED;
    chrCb->u.fix.dynRegFlg = dynRegFlg;
    chrCb->u.fix.strLen = strLen;
    chrCb->u.fix.str = comlib_memMalloc(strLen + 1);
    comlib_strNCpy(chrCb->u.fix.str, str, strLen);
    chrCb->u.fix.str[strLen] = '\0';
    chrCb->u.fix.hNode.key.keyLen =  strLen;
    chrCb->u.fix.hNode.key.key = chrCb->u.fix.str;

    chrCb->u.fix.hNode.data = chrCb;

    chrCb->usrArg = usrArg;

    (*rt_chrCb) = chrCb;

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_chrMakeUni(UINT chrCnt, CHAR lstChr, VOID *usrArg, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;

    if(chrCnt == 0){
        WLD_LOG(WLD_ERR,"Uni char count not exist\n");
        return WLDERR_UNI_CHR_CNT_NOT_EXSIT;
    }

    chrCb = comlib_memMalloc(sizeof(WldlibChrCb));

    ret = chr_init(chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Char control block init failed(ret=%d)\n",ret);
        comlib_memFree(chrCb);
        return ret;
    }

    chrCb->blkType = WLDLIB_BLK_TYPE_UNI_WLDCARD;
    chrCb->u.uni.cnt = chrCnt;
    chrCb->u.uni.lstChr = lstChr;

    chrCb->usrArg = usrArg;

    (*rt_chrCb) = chrCb;

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_chrMakeWld(CHAR lstChr, VOID *usrArg, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;

    chrCb = comlib_memMalloc(sizeof(WldlibChrCb));

    ret = chr_init(chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Char control block init failed(ret=%d)\n",ret);
        comlib_memFree(chrCb);
        return ret;
    }

    chrCb->blkType = WLDLIB_BLK_TYPE_WLDCARD;
    if(lstChr == '\0'){
        chrCb->u.wld.lstChr = '\0';
    }
    else {
        chrCb->u.wld.lstChr = lstChr;
    }

    chrCb->usrArg = usrArg;

    (*rt_chrCb) = chrCb;


    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_chrPrnt(UINT depth, WldlibChrCb *chrCb, BOOL treeFlg)
{
    switch(chrCb->blkType){
        case WLDLIB_BLK_TYPE_FIXED:
            {
                if(chrCb->u.fix.dynRegFlg == RC_TRUE){
                    WLD_DISP("%*s+ %.*s(DYNAMIC)",depth,"", chrCb->u.fix.strLen, chrCb->u.fix.str); 
                }
                else {
                    WLD_DISP("%*s+ %.*s(STATIC)",depth, "",chrCb->u.fix.strLen, chrCb->u.fix.str); 
                }
            }
            break;
        case WLDLIB_BLK_TYPE_WLDCARD:
            {
                if(chrCb->u.wld.lstChr == '\0'){
                    WLD_DISP("%*s+ *",depth, "");
                }
                else {
                    WLD_DISP("%*s+ *%c", depth, "",chrCb->u.wld.lstChr);
                }
            }
            break;
        case WLDLIB_BLK_TYPE_UNI_WLDCARD:
            {
                if(chrCb->u.uni.lstChr == '\0'){
                    WLD_DISP("%*s+ %d",depth,"", chrCb->u.uni.cnt);
                }
                else {
                    WLD_DISP("%*s+ %d(%c)", depth,"", chrCb->u.uni.cnt, chrCb->u.uni.lstChr);
                }
            }
            break;
    };

    if(chrCb->usrArg != NULL){
#if 0
        WLD_DISP(" ARG:[0x%x]\n",*(UINT*)(&chrCb->usrArg));
#else
        WLD_DISP(" ARG EXIST\n");
#endif
    }
    else {
        WLD_DISP("\n");
    }

    if(treeFlg == RC_TRUE){
        if(chrCb->nxtLst != NULL){
            wldlib_blkLstPrnt(depth+3, chrCb->nxtLst);
        }
    }

    return RC_OK;
}
