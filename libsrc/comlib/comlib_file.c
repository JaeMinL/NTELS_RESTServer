#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

FT_PUBLIC RT_RESULT comlib_fileMkdir(CONST CHAR *path, CONST U_16 mode)
{
    SINT ret = 0;

    ret = mkdir(path, mode);
    if(ret != 0){
        if(errno == EACCES){
            return COMERR_FILE_ACCES;
        }
        if(errno == EDQUOT){
            return COMERR_FILE_DQUOT;
        }
        if(errno == EEXIST){
            return COMERR_FILE_EXIST;
        }
        if(errno == EFAULT){
            return COMERR_FILE_FAULT;
        }
        if(errno == ELOOP){
            return COMERR_FILE_LOOP;
        }
        if(errno == EMLINK){
            return COMERR_FILE_MLINK;
        }
        if(errno == ENAMETOOLONG){
            return COMERR_FILE_NAMETOOLONG;
        }
        if(errno == ENOENT){
            return COMERR_FILE_NOENT;
        }
        if(errno == ENOMEM){
            return COMERR_FILE_NOMEM;
        }
        if(errno == ENOSPC){
            return COMERR_FILE_NOSPC;
        }
        if(errno == ENOTDIR){
            return COMERR_FILE_NOTDIR;
        }
        if(errno == EPERM){
            return COMERR_FILE_PERM;
        }
        if(errno == EROFS){
            return COMERR_FILE_ROFS;
        }
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_fileFrceDir(CONST CHAR *path, CONST U_16 mode)
{
    SINT ret = RC_OK;
    UINT i = 0;
    UINT bufLen = 0;
    CHAR *buf = NULL;
    CHAR *cur = NULL;

    bufLen = comlib_strGetLen((CHAR*)path);
    buf = comlib_memMalloc(bufLen+1);

    comlib_strCpy((CHAR*)buf, (CHAR*)path);
    buf[bufLen] = '\0';

    cur = buf;

    for(i=0;i<bufLen;i++){
        if(*cur == '/'){
            cur++;
        }
        else {
            break;
        }
    }/* end of for(i=0;i<bufLen;i++) */

    while(*cur){
        if ( '/' == *cur){
            *cur = '\0';

            if ( 0 != access(buf, F_OK)){
                ret = comlib_fileMkdir(buf, mode);
                if(ret != RC_OK){
                    comlib_memFree(buf);
                    return ret;
                }
            }
            *cur = '/';
        }
        cur++;
    }

    if ( 0 != access( buf, F_OK)){
        ret = comlib_fileMkdir(buf, mode);
        if(ret != RC_OK){
            comlib_memFree(buf);
            return ret;
        }
    }

    comlib_memFree(buf);

    return RC_OK;
}
