#include <stdio.h>
#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

FT_PRIVATE RT_RESULT    parse_addFix           (WldlibBlkLst *blkLst, CHAR *str, UINT strLen, WldlibChrCb **rt_chrCb, 
		WldlibBlkLst **rt_nxtLst);
FT_PUBLIC RT_RESULT     wldlib_parseStrCont    (WldlibBlkLst *blkLst, CHAR *str, WldlibChrCb **rt_chrCb);

FT_PRIVATE RT_RESULT parse_addFix(WldlibBlkLst *blkLst, CHAR *str, UINT strLen, 
		WldlibChrCb **rt_chrCb, WldlibBlkLst **rt_nxtLst)
{
    SINT ret = RC_OK;
    CHAR *cur = NULL;
    UINT remLen = 0;
    WldlibBlkLst *curBlk = NULL;
    WldlibChrCb *chrCb = NULL;

    remLen = strLen;
    cur = str;
    curBlk = blkLst;

    (*rt_chrCb) = NULL;

    if(remLen == 0){
        (*rt_nxtLst) = blkLst;
        (*rt_chrCb) = NULL;
        return RC_OK;
    }

    while(remLen){
        if(curBlk->strLen == 0){
            ret = wldlib_blkLstAddFixed(curBlk, cur, remLen, NULL, &chrCb);
            if(ret != RC_OK){
                WLD_LOG(WLD_ERR,"Fixed control block add failed(ret=%d, strLen=%d, str=%.*s)\n", ret , remLen, 
                        remLen, cur);
                return ret;
            }

            (*rt_nxtLst) = chrCb->nxtLst;
            (*rt_chrCb) = chrCb;

            return RC_OK;
        }
        else if(curBlk->strLen <= remLen){
            ret = wldlib_blkLstAddFixed(curBlk, cur, curBlk->strLen, NULL, &chrCb);
            if(ret != RC_OK){
                WLD_LOG(WLD_ERR,"Fixed control block add failed(ret=%d, strLen=%d, str=%.*s)\n", ret , remLen, 
                        remLen, cur);
                return ret;
            }

            remLen -= curBlk->strLen;
            cur += curBlk->strLen;

            curBlk = chrCb->nxtLst;
            (*rt_chrCb) = chrCb;
        }
        else if(remLen < curBlk->strLen){
            ret = wldlib_blkLstSpilt(curBlk, remLen);
            if(ret != RC_OK){
                WLD_LOG(WLD_ERR,"Fixed char spilt failed(ret=%d, remLen=%d)\n",ret, remLen);
                return ret;
            }

            ret = wldlib_blkLstAddFixed(curBlk, cur, curBlk->strLen, NULL,  &chrCb);
            if((ret != RC_OK) &&
               (ret != WLDERR_CHR_CB_ALREADY_EXIST)){
                WLD_LOG(WLD_ERR,"Fixed char block add failed(ret=%d, cur=%s, len=%d)\n",ret, cur, curBlk->strLen);
                return ret;
            }

            (*rt_nxtLst) = chrCb->nxtLst;
            (*rt_chrCb) = chrCb;

            return RC_OK;

        }
    }/* end of while(remStrLen) */

    (*rt_nxtLst) = curBlk;

    return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_parseStrCont(WldlibBlkLst *blkLst, CHAR *str, WldlibChrCb **rt_chrCb)
{
	SINT ret = RC_OK;
	WldlibBlkLst *curBlkLst = NULL;
	WldlibChrCb *chrCb = NULL;
	WldlibChrCb *wldChrCb = NULL;
	UINT wldLen = 0;
	CHAR *cur = NULL;
	CHAR *lstCur = NULL;

	cur = str;

	curBlkLst = blkLst;

	fprintf(stderr,"\nSTRING=%s\n",str);
	if(*str == '\0'){
		return RC_OK;
	}

	/* fixed */
	ret = wldlib_blkLstFindFixed(curBlkLst, cur, &chrCb); 
	if(ret == RC_OK){
		cur += curBlkLst->strLen;

		wldlib_chrPrnt(0, chrCb, RC_FALSE);
		ret = wldlib_parseStrCont(chrCb->nxtLst, cur, &chrCb);
		if(ret == RC_OK){
			(*rt_chrCb) = chrCb;
			return RC_OK;
		}
	}

	/* wild */
	wldLen = 0;
	ret = wldlib_blkLstFindWld(curBlkLst, cur, &lstCur, &chrCb);
	if(ret == RC_OK){
		ComlibLnkNode *lnkNode = NULL;
		wldChrCb = chrCb;
		wldLen = lstCur - cur;

		/* uniWldLL */
		if(curBlkLst->uniWldLL.nodeCnt != 0){
			COM_GET_LNKLST_FIRST(&curBlkLst->uniWldLL, lnkNode);
			while(1){
				chrCb = lnkNode->data;

				if(wldLen >= chrCb->u.uni.cnt){
					if((chrCb->u.uni.cnt+1) <= wldLen){
						if((chrCb->u.uni.lstChr != '\0') &&
								(cur[chrCb->u.uni.cnt] == chrCb->u.uni.lstChr)){
							cur += (chrCb->u.uni.cnt + 1);
							wldlib_chrPrnt(0, chrCb, RC_FALSE);
							ret = wldlib_parseStrCont(chrCb->nxtLst, lstCur, &chrCb);
							if(ret == RC_OK){
								(*rt_chrCb) = chrCb;
								return RC_OK;
							}
						}
					}
					else if(chrCb->u.uni.cnt == wldLen){
						if(chrCb->u.uni.lstChr == '\0'){
							(*rt_chrCb) = chrCb;
							return RC_OK;
						}
					}
				}/* end of if(wldLen >= chrCb->u.uni.cnt) */
				else {
					break;
				}

				COM_GET_NEXT_NODE(lnkNode);
				if(lnkNode == NULL){
					break;
				}
			}/* end of while(1) */
		}/* end of if(curBlkLst->uniWldLL.nodeCnt != 0) */

		wldlib_chrPrnt(0, wldChrCb, RC_FALSE);
		fprintf(stderr,"NEXT\n");
		ret = wldlib_parseStrCont(wldChrCb->nxtLst, lstCur, &chrCb);
		if(ret != RC_OK){
			fprintf(stderr,"NEXT2\n");
			chrCb = NULL;
			if(curBlkLst->uniWldLL.nodeCnt != 0){
				fprintf(stderr,"NEXT3\n");
				if(lnkNode != NULL){
					while(1){
						chrCb = lnkNode->data;

						if(wldLen < chrCb->u.uni.cnt){
							if(cur[chrCb->u.uni.cnt] == chrCb->u.uni.lstChr){
								cur += (chrCb->u.uni.cnt + 1);
								wldlib_chrPrnt(0, chrCb, RC_FALSE);
								ret = wldlib_parseStrCont(chrCb->nxtLst, cur, &chrCb);
								if(ret == RC_OK){
									(*rt_chrCb) = chrCb;
									return RC_OK;
								}
							}
						}
						COM_GET_NEXT_NODE(lnkNode);
						if(lnkNode == NULL){
							chrCb = NULL;
							break;
						}
					}/* end of while(1) */
				}/* end of if(lnkNode ! = NULL) */
			}/* end of if(curBlkLst->uniWldLL.nodeCnt != 0) */

			if(chrCb != NULL){
				(*rt_chrCb) = chrCb;
				return RC_OK;
			}
			WLD_LOG(WLD_ERR,"String parsing failed(cur=%s\n",cur);
			return RC_NOK;
		}/* end of if(ret != RC_OK) */
		else {
			if(ret == RC_OK){
				(*rt_chrCb) = chrCb;
				return RC_OK;
			}
		}
	} /* end of if(ret == RC_OK) */

	return RC_NOK;
}

FT_PUBLIC RT_RESULT wldlib_parseStr(WldlibCb *wldlibCb, CHAR *str, VOID **rt_usrArg)
{
	SINT ret = RC_OK;
	WldlibChrCb *chrCb = NULL;

	ret = wldlib_parseStrCont(wldlibCb->blkLst, str, &chrCb);
	if(ret != RC_OK){
		return ret;
	}

	(*rt_usrArg) = chrCb->usrArg;

	return RC_OK;
}

FT_PUBLIC RT_RESULT wldlib_parseRegRule(WldlibCb *wldlibCb, CHAR *str, VOID *usrArg)
{
    SINT ret = RC_OK;
    UINT curIdx = 0;
    UINT wldCnt = 0;
    UINT uniCnt = 0;
    CHAR *cur = NULL;
    UINT remStrLen = 0;
    WldlibBlkLst *blkLst = NULL;
    WldlibChrCb *chrCb = NULL;
    WldlibChrCb *addChrCb = NULL;

    cur = str;

    blkLst = wldlibCb->blkLst;
    if(blkLst == NULL){
        ret = wldlib_blkLstMakeFirst(wldlibCb, &blkLst);
        if(ret != RC_OK){
            WLD_LOG(WLD_ERR,"First block list make failed(ret=%d)\n",ret);
            return ret;
        }
    }

    while(*cur){
        curIdx = comlib_strCSpn(cur, "*?");
        if(cur[curIdx] == '\0'){
            if(cur == str){
                /* insert fullMatch */
            }
            else { /* last fixed message */
                remStrLen = &cur[curIdx] - cur;

                ret = parse_addFix(blkLst, cur, remStrLen, &addChrCb, &blkLst);
                if(ret != RC_OK){
                    WLD_LOG(WLD_ERR,"Fix list add failed(ret=%d)\n",ret);
                    return ret;
                }

                if(addChrCb != NULL){
                    chrCb = addChrCb;
                }

                cur+= remStrLen;
                break;
            }
        }/* end of if(cur[curIdx] == '\0') */

        /* add string */
        if(curIdx != 0){
            remStrLen = &cur[curIdx] - cur;

            while(remStrLen){
                ret = wldlib_blkLstFindFixed(blkLst, cur, &chrCb);
                if(ret == RC_OK){
                    remStrLen -= blkLst->strLen;

                    cur += blkLst->strLen;
                    blkLst = chrCb->nxtLst;
                }
                else {
                    break;
                }
            }/* end of while(remStrLen) */

            if((*cur != '*')  && (*cur != '?')){
                ret = parse_addFix(blkLst, cur, remStrLen, &addChrCb, &blkLst);
                if(ret != RC_OK){
                    WLD_LOG(WLD_ERR,"Fix list add failed(ret=%d)\n",ret);
                    return ret;
                }

                if(addChrCb != NULL){
                    chrCb = addChrCb;
                }

                cur += remStrLen;
            }
        }/* end of if(curIdx != 0) */

        wldCnt = 0;
        uniCnt = 0;
        while(*cur){
            if((*cur) == '*')         { wldCnt++; }
            else if((*cur) == '?')    { uniCnt++; }
            else                      { break;    }

            cur++;
        }/* end of while(1) */

        /* add wildcard */
        if(wldCnt != 0){
            ret = wldlib_blkLstAddWld(blkLst, (*cur), uniCnt, NULL, &chrCb);
            if((ret != RC_OK) &&
               (ret != WLDERR_CHR_CB_ALREADY_EXIST)){
                WLD_LOG(WLD_ERR,"Wildcard add failed(ret=%d)\n",ret);
                return ret;
            }
            cur++;
        }
        else if(uniCnt != 0){
            ret = wldlib_blkLstAddUni(blkLst, uniCnt, (*cur), NULL, &chrCb);
            if((ret != RC_OK) &&
               (ret != WLDERR_CHR_CB_ALREADY_EXIST)){
                WLD_LOG(WLD_ERR,"Uni add failed(ret=%d)\n",ret);
                return ret;
            }
            cur++;
        }

        blkLst = chrCb->nxtLst;
    }/* end of while(*cur) */

    if(chrCb != NULL){
        chrCb->usrArg = usrArg;
    }

    return RC_OK;
}

