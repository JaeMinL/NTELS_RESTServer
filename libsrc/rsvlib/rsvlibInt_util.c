#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "thrlib.h"
#include "thrlib.x"
#include "trnlib.h"
#include "trnlib.x"
#include "loglib.h"
#include "loglib.x"
#include "rrllib.h"
#include "rrllib.x"

#include "rsvlib.h"
#include "rsvlib.x"
#include "rsvlibInt.h"
#include "rsvlibInt.x"

FT_PUBLIC RT_RESULT rsvlibInt_utilReadFile(CHAR *path, CHAR **rt_buf)
{
    FILE *inFile = NULL;
    CHAR *buf = NULL;
    UINT numBytes = 0;

    inFile = fopen(path, "r");
    if(inFile == NULL){
        RSV_LOG(RSV_ERR,"Read faile error(path=%s)\n",path);
        goto goto_readFileErrRet;
    }

    /* get the number of bytes */
    fseek(inFile, 0L, SEEK_END);
    numBytes = ftell(inFile);

    fseek(inFile, 0L, SEEK_SET);

    buf = (CHAR*)comlib_memMalloc(numBytes+1);
    if(buf == NULL){
        RSV_LOG(RSV_ERR,"allocation failed(path=%s)\n", path);
        goto goto_readFileErrRet;
    }

    fread(buf, sizeof(CHAR), numBytes, inFile);
    fclose(inFile);

    buf[numBytes] = '\0';
    (*rt_buf) = buf;

    return RC_OK;

goto_readFileErrRet:
    if(inFile != NULL){
        fclose(inFile);
    }

    return RC_NOK;
}

