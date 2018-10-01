#include <stdio.h>

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

STATIC U_32 g_loglibDispEnv = LOGLIB_DISP_TIME_BIT | LOGLIB_DISP_LVL_BIT | LOGLIB_DISP_FILE_BIT | LOGLIB_DISP_LINE_BIT;

FT_PUBLIC U_32 loglibInt_globGetDispEnv()
{
    return g_loglibDispEnv;
}

FT_PUBLIC VOID loglibInt_globSetDispEnv(U_32 bit)
{
    g_loglibDispEnv = bit;
}
