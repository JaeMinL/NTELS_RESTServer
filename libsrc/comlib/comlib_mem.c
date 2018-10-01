#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

FT_PUBLIC VOID *comlib_memMalloc(CONST SIZET size)
{
	return malloc(size);
}

FT_PUBLIC VOID *comlib_memCalloc(CONST SIZET num, CONST SIZET size)
{
	return calloc(num, size);
}

FT_PUBLIC RT_RESULT comlib_memMemcmp(CONST VOID *s1, CONST VOID *s2, CONST SIZET n)
{
	return memcmp(s1,s2,n);
}

FT_PUBLIC VOID *comlib_memMemcpy(VOID *dst, CONST VOID *src, CONST SIZET n)
{
	return memcpy(dst,src,n);
}
FT_PUBLIC VOID *comlib_memMemmov(VOID *dst, CONST VOID *src, CONST SIZET n)
{
	return memmove(dst,src,n);
}

FT_PUBLIC VOID *comlib_memMemset(VOID *src, CONST UINT c, CONST SIZET n)
{
	return memset(src,(SINT)c,n);
}

FT_PUBLIC VOID comlib_memFree(VOID *ptr)
{
	free(ptr);
}
