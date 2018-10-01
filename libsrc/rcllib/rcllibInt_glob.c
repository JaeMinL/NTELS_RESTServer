#include <curl/curl.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rcllib.h"
#include "rcllib.x"
#include "rcllibInt.x"

FT_PUBLIC RT_RESULT rcllibInt_globInit()
{
    curl_global_init(CURL_GLOBAL_ALL);

    return RC_OK;
}

