#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

FT_PRIVATE RT_RESULT     wldlib_blkLstInit                 (WldlibBlkLst *blkLst);
FT_PRIVATE RT_RESULT     wldlib_blkLstWldChrSetInit        (WldlibWldChrSet *wldChrSet, UINT maxLstChr);

FT_PRIVATE RT_RESULT wldlib_blkLstWldChrSetInit(WldlibWldChrSet *wldChrSet, UINT maxLstChr)
{
    wldChrSet->lstChrCnt = 0;
    if(maxLstChr == 0){
        wldChrSet->maxLstChr = WLDLIB_DFLT_CHR_LST;
    }
    else {
        wldChrSet->maxLstChr = maxLstChr;
    }

    wldChrSet->lstChrLst = comlib_memMalloc(sizeof(CHAR) * (wldChrSet->maxLstChr + 1));
    wldChrSet->chrCb = comlib_memMalloc(sizeof(WldlibChrCb*) * wldChrSet->maxLstChr);

    return RC_OK;
}

FT_PRIVATE RT_RESULT wldlib_blkLstInit(WldlibBlkLst *blkLst)
{
    SINT ret = RC_OK;

    blkLst->strLen = 0;

    ret = comlib_hashTblInit(&blkLst->fixHT, 1024, RC_FALSE, COM_HASH_TYPE_STRING ,NULL);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"full match hash table init failed(ret=%d)\n",ret);
        return WLDERR_HASHTBL_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&blkLst->fixLL,~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"full match linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    ret = comlib_lnkLstInit(&blkLst->uniWldLL,~0);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"uni wildcard linked list init failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INIT_FAILED;
    }

    blkLst->own = NULL;

    ret = wldlib_blkLstWldChrSetInit(&blkLst->wldChrSet, WLDLIB_DFLT_CHR_LST);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Wildcard char set init failed(ret=%d)\n",ret);
        return ret;
    }

    blkLst->lnkNode.data = blkLst;

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstFindFixed(WldlibBlkLst *blkLst, CHAR *str, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;
    ComlibHashKey hKey;
    ComlibHashNode *hNode = NULL;

    hKey.keyLen = blkLst->strLen;
    hKey.key = str;

    ret = comlib_hashTblFindHashNode(&blkLst->fixHT, &hKey, 0, &hNode);
    if(ret != RC_OK){
        return RC_NOK;
    }

    if(rt_chrCb != NULL){
        chrCb = hNode->data;

        (*rt_chrCb) = chrCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstInsrtFixed(WldlibBlkLst *blkLst, WldlibChrCb *chrCb)
{
    SINT ret = RC_OK;

    ret = comlib_hashTblInsertHashNode(&blkLst->fixHT, &chrCb->u.fix.hNode.key, &chrCb->u.fix.hNode);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Fixed hash table insert failed(ret=%d)\n",ret);
        comlib_memFree(chrCb);
        return WLDERR_HASHTBL_INSERT_FAILED;
    }

    ret = comlib_lnkLstInsertTail(&blkLst->fixLL, &chrCb->lnkNode);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Fixed lnk data insert failed(ret=%d)\n",ret);
        ret = comlib_hashTblDelHashNode(&blkLst->fixHT, &chrCb->u.fix.hNode);
        if(ret != RC_OK){
            WLD_LOG(WLD_ERR,"Fixed hash table delete failed(ret=%d)\n",ret);
        }

        return WLDERR_LNKLST_INSERT_FAILED;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstAddFixed(WldlibBlkLst *blkLst, CHAR *str, UINT strLen, VOID *usrArg, 
		WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;

    if((blkLst->strLen != 0) && (strLen != blkLst->strLen)){
        WLD_LOG(WLD_ERR,"Invalid string length(len=%d, blkLen=%d)\n",strLen, blkLst->strLen);
        return WLDERR_INVALID_STR_LEN;
    }

    /* find */
    ret = wldlib_blkLstFindFixed(blkLst, str, &chrCb);
    if(ret == RC_OK){
        if(rt_chrCb != NULL){
            (*rt_chrCb) = chrCb;
        }
        return WLDERR_CHR_CB_ALREADY_EXIST;
    }

    ret = wldlib_chrMakeFix(str, strLen, RC_FALSE, usrArg, &chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"fixed char control block make failed(ret=%d, str=%s, strLen=%d)\n",ret, str, strLen);
        return ret;
    }

    blkLst->strLen = strLen;

    ret = wldlib_blkLstInsrtFixed(blkLst, chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Block list insert failed(ret=%d)\n",ret);
        comlib_memFree(chrCb);
    }

    if(rt_chrCb != NULL){
        (*rt_chrCb) = chrCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstAddUni(WldlibBlkLst *blkLst, UINT chrCnt, CHAR lstChr, VOID *usrArg, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;
    ComlibLnkNode *lnkNode = NULL;

    COM_GET_LNKLST_FIRST(&blkLst->uniWldLL, lnkNode);
    if(lnkNode != NULL){
        while(1){
            chrCb = lnkNode->data;

            if((chrCb->u.uni.cnt > chrCnt)){
                break;
            }

            if((chrCb->u.uni.cnt == chrCnt) &&
               (chrCb->u.uni.lstChr == lstChr)){
                if(rt_chrCb != NULL){
                    (*rt_chrCb) = chrCb;
                }

                return WLDERR_CHR_CB_ALREADY_EXIST;
            }

            COM_GET_NEXT_NODE(lnkNode);
            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }/* end of if(lnkNode != NULL) */

    ret = wldlib_chrMakeUni(chrCnt, lstChr, usrArg, &chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Uni char control block make failed(ret=%d)\n",ret);
        return ret;
    }

    /* sorting */
    if(lnkNode == NULL){
        ret = comlib_lnkLstInsertTail(&blkLst->uniWldLL, &chrCb->lnkNode);
    }
    else {
        ret = comlib_lnkLstInsertPrevNode(&blkLst->uniWldLL, lnkNode, &chrCb->lnkNode);
    }
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Uni node insert failed(ret=%d)\n",ret);
        return WLDERR_LNKLST_INSERT_FAILED;
    }

    if(rt_chrCb != NULL){
        (*rt_chrCb) = chrCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstAddWld(WldlibBlkLst *blkLst, CHAR lstChr, UINT minChrCnt, 
		VOID *usrArg, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    WldlibWldChrSet *wldChrSet = NULL;
    WldlibChrCb *chrCb = NULL;

    wldChrSet = &blkLst->wldChrSet;

    /* find last char */
    if(lstChr != '\0'){
        CHAR *cur = NULL;

        cur = comlib_strFindChr(wldChrSet->lstChrLst, lstChr);
        if(cur != NULL){
            if(rt_chrCb != NULL){
                (*rt_chrCb) = wldChrSet->chrCb[cur - wldChrSet->lstChrLst];
            }
            return WLDERR_CHR_CB_ALREADY_EXIST;
        }
    }/* if(lstChr == '\0') */
    else {
        if(wldChrSet->lstChrCb != NULL){
            if(rt_chrCb != NULL){
                (*rt_chrCb) = wldChrSet->lstChrCb;
            }
            return WLDERR_CHR_CB_ALREADY_EXIST;
        }
    }

    ret = wldlib_chrMakeWld(lstChr, usrArg, &chrCb);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Wildcard control block make failed(ret=%d)\n",ret);
        return ret;
    }

    if(lstChr == '\0'){
        wldChrSet->lstChrCb = chrCb;

        if(rt_chrCb != NULL){
            (*rt_chrCb) = chrCb;
        }

        return RC_OK;
    }

    if(wldChrSet->lstChrCnt ==  wldChrSet->maxLstChr){
        CHAR *lstChrLst = NULL;
        WldlibChrCb **chrCbLst = NULL;

        lstChrLst = comlib_memMalloc(wldChrSet->maxLstChr + WLDLIB_DFLT_CHR_LST + 1);
        chrCbLst = comlib_memMalloc(sizeof(WldlibChrCb*) *(wldChrSet->maxLstChr + WLDLIB_DFLT_CHR_LST + 1));

        comlib_memMemcpy(lstChrLst, wldChrSet->lstChrLst, wldChrSet->maxLstChr);
        comlib_memMemcpy(chrCbLst, wldChrSet->chrCb, wldChrSet->maxLstChr);

        wldChrSet->maxLstChr = wldChrSet->maxLstChr + WLDLIB_DFLT_CHR_LST;

        comlib_memFree(wldChrSet->lstChrLst);
        comlib_memFree(wldChrSet->chrCb);

        wldChrSet->lstChrLst = lstChrLst;
    }

    wldChrSet->lstChrLst[wldChrSet->lstChrCnt] = lstChr;
    wldChrSet->chrCb[wldChrSet->lstChrCnt] = chrCb;

    wldChrSet->lstChrCnt++;
    wldChrSet->minChrCnt = minChrCnt;
    wldChrSet->lstChrLst[wldChrSet->lstChrCnt] = '\0';

    if(rt_chrCb != NULL){
        (*rt_chrCb) = chrCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstSpilt(WldlibBlkLst *blkLst, UINT strLen)
{
    SINT ret = RC_OK;
    WldlibChrCb *chrCb = NULL;
    WldlibChrCb *newChrCb = NULL;
    WldlibBlkLst *nxtBlkLst = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(strLen == 0){
        WLD_LOG(WLD_ERR,"String length is zero\n");
        return WLDERR_INVALID_STR_LEN;
    }

    /* set new length for search */
    blkLst->strLen = strLen;

    COM_GET_LNKLST_FIRST(&blkLst->fixLL, lnkNode);
    if(lnkNode != NULL){
        blkLst->fixLL.nodeCnt = 0;
        blkLst->fixLL.first = NULL;
        blkLst->fixLL.tail = NULL;

        lnkNode->prev = NULL;

        while(1){
            chrCb = lnkNode->data;

            COM_GET_NEXT_NODE(lnkNode);

            ret = comlib_hashTblDelHashNode(&blkLst->fixHT, &chrCb->u.fix.hNode);
            if(ret != RC_OK){
                WLD_LOG(WLD_ERR,"Fix hash table delete failed(ret=%d)\n",ret);
            }

            ret = wldlib_blkLstFindFixed(blkLst, chrCb->u.fix.str, &newChrCb);
            if(ret == RC_OK){
                if(newChrCb->nxtLst == NULL){
                    /* make */
                    ret = wldlib_blkLstMakeNxt(newChrCb, &nxtBlkLst);
                    if(ret != RC_OK){
                        WLD_LOG(WLD_ERR,"Make next block failed(ret=%d)\n",ret);
                        return ret;
                    }
                }
            }
            else {
                /* make new block */
                ret = wldlib_chrMakeFix(chrCb->u.fix.str,
                                        strLen, 
                                        chrCb->u.fix.dynRegFlg, NULL, &newChrCb);
                if(ret != RC_OK){
                    WLD_LOG(WLD_ERR,"Fix char make failed(ret=%d)\n",ret);
                    return ret;
                }

                ret = wldlib_blkLstAddFixed(blkLst, newChrCb->u.fix.str, strLen, NULL, &newChrCb);
                if(ret != RC_OK){
                    WLD_LOG(WLD_ERR,"Add fix char failed(ret=%d)\n",ret);
                    return ret;
                }

                ret = wldlib_blkLstMakeNxt(newChrCb, &nxtBlkLst);
                if(ret != RC_OK){
                    WLD_LOG(WLD_ERR,"Make next block failed(ret=%d)\n",ret);
                    return ret;
                }
            }

            comlib_strCpy(chrCb->u.fix.str, &chrCb->u.fix.str[strLen]);
            chrCb->u.fix.strLen = chrCb->u.fix.strLen - strLen;
            chrCb->u.fix.hNode.key.keyLen = chrCb->u.fix.strLen;
            nxtBlkLst->strLen = chrCb->u.fix.strLen;

            ret = wldlib_blkLstInsrtFixed(nxtBlkLst, chrCb);
            if(ret != RC_OK){
                WLD_LOG(WLD_ERR,"chr blcok insert failed(ret=%d)\n",ret);
                return ret;
            }

            if(lnkNode == NULL){
                break;
            }
        }/* end of while(1) */
    }/* end of if(lnkNode != NULL) */

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstMakeFirst(WldlibCb *wldlibCb, WldlibBlkLst **rt_blkLst)
{
    SINT ret = RC_OK;
    WldlibBlkLst *blkLst = NULL;

    blkLst = comlib_memMalloc(sizeof(WldlibBlkLst));

    ret = wldlib_blkLstInit(blkLst);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Block list init failed(ret=%d)\n",ret);
        comlib_memFree(blkLst);
        return ret;
    }

    wldlibCb->blkLst = blkLst;

    if(rt_blkLst != NULL){
        (*rt_blkLst) = blkLst;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstMakeNxt(WldlibChrCb *chrCb, WldlibBlkLst **rt_blkLst)
{
    SINT ret = RC_OK;
    WldlibBlkLst *blkLst = NULL;

    blkLst = comlib_memMalloc(sizeof(WldlibBlkLst));

    ret = wldlib_blkLstInit(blkLst);
    if(ret != RC_OK){
        WLD_LOG(WLD_ERR,"Block list init failed(ret=%d)\n",ret);
        comlib_memFree(blkLst);
        return ret;
    }

    blkLst->own = chrCb;

    chrCb->nxtLst = blkLst;

    if(rt_blkLst != NULL){
        (*rt_blkLst) = blkLst;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstFindWld(WldlibBlkLst *blkLst, CHAR *str, CHAR **rt_lstCur, WldlibChrCb **rt_chrCb)
{
    SINT ret = RC_OK;
    CHAR *chrPtr = NULL;
    CHAR *lstCur = NULL;
    WldlibChrCb *chrCb = NULL;
    WldlibWldChrSet *wldChrSet = NULL;

    wldChrSet = &blkLst->wldChrSet;

    ret = comlib_strCSpn(str, wldChrSet->lstChrLst);
    if(str[ret] == '\0'){
        lstCur = &str[ret];
        goto goto_findLst;
    }

    chrPtr = comlib_strFindChr(wldChrSet->lstChrLst, str[ret]);
    if(chrPtr == NULL){
        WLD_LOG(WLD_ERR,"Char not exist(%c)\n",str[ret]);
        return RC_NOK;
    }

    /* find */
    chrCb = wldChrSet->chrCb[chrPtr - wldChrSet->lstChrLst];

    if(rt_lstCur != NULL){
        (*rt_lstCur) = &str[ret+1];
    }

    if(rt_chrCb != NULL){
        (*rt_chrCb) = chrCb;
    }

    return RC_OK;

goto_findLst:
    if(wldChrSet->lstChrCb == NULL){
        return WLDERR_NOT_EXIST;
    }

    if(rt_lstCur != NULL){
        (*rt_lstCur) = lstCur;
    }
    if(rt_chrCb != NULL){
        (*rt_chrCb) = wldChrSet->lstChrCb;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_blkLstPrnt(UINT depth, WldlibBlkLst *blkLst)
{
    UINT i = 0;
    WldlibChrCb *chrCb = NULL;
    WldlibWldChrSet *wldChrSet = NULL;
    ComlibLnkNode *lnkNode = NULL;

    if(blkLst->fixLL.nodeCnt != 0){
        WLD_DISP("%*s+ [FIXED LIST]\n",depth, "");
        COM_GET_LNKLST_FIRST(&blkLst->fixLL, lnkNode);
        if(lnkNode != NULL){
            while(1){
                chrCb = lnkNode->data;

                wldlib_chrPrnt(depth+3, chrCb, RC_TRUE);

                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    break;
                }
            }/* end of while(1) */
        }/* end of if(lnkNode != NULL) */
    }/* end of if(blkLst->fixLL.nodeCnt != 0) */
 
    if(blkLst->uniWldLL.nodeCnt != 0){
        WLD_DISP("%*s+ [UNI LIST]\n",depth, "");
        COM_GET_LNKLST_FIRST(&blkLst->uniWldLL, lnkNode);
        if(lnkNode != NULL){
            while(1){
                chrCb = lnkNode->data;

                wldlib_chrPrnt(depth+3, chrCb, RC_TRUE);
                COM_GET_NEXT_NODE(lnkNode);
                if(lnkNode == NULL){
                    break;
                }
            }/* end of while(1) */
        }/* end of if(lnkNode != NULL) */
    }/* end of if(blkLst->uniWldLL.nodeCnt != 0) */

    wldChrSet = &blkLst->wldChrSet;
    if((wldChrSet->lstChrCnt != 0) ||
       (wldChrSet->lstChrCb != NULL)){
        WLD_DISP("%*s+ [WILDCHARD LIST]\n", depth, "");

        for(i=0;i<wldChrSet->lstChrCnt;i++){
            wldlib_chrPrnt(depth+3, wldChrSet->chrCb[i], RC_TRUE);
        }/* end of for(i=0;i<lstChrCnt;i++) */

        if(wldChrSet->lstChrCb != NULL){
            wldlib_chrPrnt(depth+3, wldChrSet->lstChrCb, RC_TRUE);
        }
    }

    return RC_OK;
}

