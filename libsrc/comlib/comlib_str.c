#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

FT_PUBLIC UINT comlib_strGetLen(CONST CHAR *src)
{
	return (UINT)strlen(src);
}

FT_PUBLIC SINT comlib_strCmp(CONST CHAR *s1, CONST CHAR *s2)
{
	return strcmp(s1, s2);
}

FT_PUBLIC SINT comlib_strNCmp(CONST CHAR *s1, CONST CHAR *s2, UINT len)
{
    return strncmp(s1, s2, len);
}

FT_PUBLIC SINT comlib_strCaseCmp(CONST CHAR *s1, CONST CHAR *s2)
{
	return strcasecmp(s1, s2);
}


FT_PUBLIC RT_RESULT comlib_strCpy(CHAR *dst, CONST CHAR *src)
{
	strcpy(dst, src);

	return RC_OK;
}

FT_PUBLIC RT_RESULT comlib_strNCpy(CHAR *dst, CONST CHAR *src, UINT len)
{
	strncpy(dst, src, len);

	return RC_OK;
}

FT_PUBLIC SINT comlib_strCaseNCmp(CONST CHAR *s1, CONST CHAR *s2, UINT len)
{
    return strncasecmp(s1, s2, len);
}

FT_PUBLIC UINT comlib_strSNPrnt(CHAR *str, UINT size, CONST CHAR *fmt,...)
{
	UINT len = 0;
	va_list ap;

	va_start(ap, fmt);

	len = vsnprintf(str, size, fmt, ap);

	va_end(ap);

	return len;
}

FT_PUBLIC CHAR comlib_strToUpper(CONST CHAR ch)
{
    return (CHAR)toupper(ch);
}

FT_PUBLIC RT_RESULT comlib_strChgStrToUpper(CONST CHAR *src, UINT srcLen, CHAR *rt_dst, UINT maxLen)
{
    UINT i = 0;

    if(srcLen > maxLen){
        return RC_NOK;
    }

    for(i=0;i<srcLen;i++){
        rt_dst[i] = comlib_strToUpper(src[i]);
    }

    return RC_OK;
}

FT_PUBLIC CHAR comlib_strToLower(CONST CHAR ch)
{
    return (CHAR)tolower(ch);
}

FT_PUBLIC RT_RESULT comlib_strChgStrToLower(CONST CHAR *src, UINT srcLen, CHAR *rt_dst, UINT maxLen)
{
    UINT i = 0;

    if(srcLen > maxLen){
        return RC_NOK;
    }

    for(i=0;i<srcLen;i++){
        rt_dst[i] = comlib_strToLower(src[i]);
    }

    return RC_OK;
}

FT_PUBLIC VOID comlib_strHexToStr(U_8 val, CHAR *rslt)
{
    CHAR strHex[] = "0123456789abcdef";

    rslt[0] = strHex[val >> 4];
    rslt[1] = strHex[val & 0x0f];
}

FT_PUBLIC UINT comlib_strAtoi(CONST CHAR *src, UINT len)
{
    UINT tot = 0;

    while(len != 0){
        tot = tot * 10 + *src - '0';
        src++;
        len--;
    }

    return tot;
}

FT_PUBLIC CHAR* comlib_strFindChr(CONST CHAR *in, CONST CHAR c)
{
    return strchr(in, c);
}

FT_PUBLIC CHAR* comlib_strFindStr(CONST CHAR *src, CONST CHAR *str)
{
    return strstr(src, str);
}

FT_PUBLIC SIZET comlib_strCSpn(CONST CHAR *src, CONST CHAR *chrSet)
{
    return strcspn(src, chrSet);
}

FT_PUBLIC SIZET comlib_strSpn(CONST CHAR *src, CONST CHAR *chrSet)
{
    return strspn(src, chrSet);
}
