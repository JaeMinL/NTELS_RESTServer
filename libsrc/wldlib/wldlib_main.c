
#include <stdio.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "wldlib.h"
#include "wldlib.x"

FT_PUBLIC RT_RESULT wldlib_mainPrnt(WldlibCb *wldlibCb)
{
    if(wldlibCb->blkLst != NULL){
        wldlib_blkLstPrnt(0, wldlibCb->blkLst);
    }

    return RC_OK;
}
