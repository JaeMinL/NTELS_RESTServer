#include <stdio.h>
#include <syslog.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "loglibInt.h"
#include "loglibInt.x"

FT_PUBLIC RT_RESULT loglibInt_loadDstryApndCfg(LoglibIntApndCfg *apndCfg)
{
    comlib_memFree(apndCfg->name);

    switch(apndCfg->type){
        case LOGLIB_APND_TYPE_FILE:
            {
                comlib_memFree(apndCfg->apndCfg.u.file.logPath);
                comlib_memFree(apndCfg->apndCfg.u.file.name);
            }
            break;
    };/* end of switch(apndCfg->type) */

    return RC_OK;
}

