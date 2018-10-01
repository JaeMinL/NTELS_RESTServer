#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

STATIC CONST CHAR g_digitsLut[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};

FT_PUBLIC UINT comlib_utilAtoi(CONST CHAR *src, UINT len)
{
    UINT tot = 0;

    while(len != 0){
        tot = tot * 10 + *src - '0';
        src++;
        len--;
    }

    return tot;
}

/* 
 * branchlut atoi from itoa-benchmark
 * site : https://github.com/miloyip/itoa-benchmark/blob/master/src/branchlut.cpp 
 */
FT_PUBLIC CHAR* comlib_utilItoaU32(U_32 val, CHAR* buf)
{
    if (val < 10000){
        CONST U_32 d1 = (val / 100) << 1;
        CONST U_32 d2 = (val % 100) << 1;

        if (val >= 1000)
            *buf++ = g_digitsLut[d1];
        if (val >= 100)
            *buf++ = g_digitsLut[d1 + 1];
        if (val >= 10)
            *buf++ = g_digitsLut[d2];
        *buf++ = g_digitsLut[d2 + 1];
    }
    else if (val < 100000000){
        /* val = bbbbcccc */
        CONST U_32 b = val / 10000;
        CONST U_32 c = val % 10000;

        CONST U_32 d1 = (b / 100) << 1;
        CONST U_32 d2 = (b % 100) << 1;

        CONST U_32 d3 = (c / 100) << 1;
        CONST U_32 d4 = (c % 100) << 1;

        if (val >= 10000000){
            *buf++ = g_digitsLut[d1];
        }
        if (val >= 1000000){
            *buf++ = g_digitsLut[d1 + 1];
        }
        if (val >= 100000){
            *buf++ = g_digitsLut[d2];
        }
        *buf++ = g_digitsLut[d2 + 1];

        *buf++ = g_digitsLut[d3];
        *buf++ = g_digitsLut[d3 + 1];
        *buf++ = g_digitsLut[d4];
        *buf++ = g_digitsLut[d4 + 1];
    }
    else{
        /* val = aabbbbcccc in decimal */

        CONST U_32 a = val / 100000000; /* 1 to 42 */
        val %= 100000000;

        if (a >= 10){
            CONST U_16 i = a << 1;
            *buf++ = g_digitsLut[i];
            *buf++ = g_digitsLut[i + 1];
        }
        else{
            *buf++ = '0' + (CHAR)(a);
        }

        CONST U_32 b = val / 10000; /* 0 to 9999 */
        CONST U_32 c = val % 10000; /* 0 to 9999 */

        CONST U_32 d1 = (b / 100) << 1;
        CONST U_32 d2 = (b % 100) << 1;

        CONST U_32 d3 = (c / 100) << 1;
        CONST U_32 d4 = (c % 100) << 1;

        *buf++ = g_digitsLut[d1];
        *buf++ = g_digitsLut[d1 + 1];
        *buf++ = g_digitsLut[d2];
        *buf++ = g_digitsLut[d2 + 1];
        *buf++ = g_digitsLut[d3];
        *buf++ = g_digitsLut[d3 + 1];
        *buf++ = g_digitsLut[d4];
        *buf++ = g_digitsLut[d4 + 1];
    }

    *buf = '\0';

    return buf;
}

FT_PUBLIC CHAR* comlib_utilItoaS32(S_32 val, CHAR* buf)
{
    U_32 u = (U_32)(val);

    if(val < 0){
        *buf++ = '-';
        u = ~u + 1;
    }

    return comlib_utilItoaU32(u, buf);
}

FT_PUBLIC CHAR* comlib_utilItoaU64(U_64 val, CHAR* buf)
{
    if(val < 100000000){
        U_32 v = (U_32)(val);

        if(v < 10000){
            CONST U_32 d1 = (v / 100) << 1;
            CONST U_32 d2 = (v % 100) << 1;

            if(v >= 1000){
                *buf++ = g_digitsLut[d1];
            }

            if(v >= 100){
                *buf++ = g_digitsLut[d1 + 1];
            }

            if(v >= 10){
                *buf++ = g_digitsLut[d2];
            }

            *buf++ = g_digitsLut[d2 + 1];
        }
        else{
            /* val = bbbbcccc */
            CONST U_32 b = v / 10000;
            CONST U_32 c = v % 10000;

            CONST U_32 d1 = (b / 100) << 1;
            CONST U_32 d2 = (b % 100) << 1;

            CONST U_32 d3 = (c / 100) << 1;
            CONST U_32 d4 = (c % 100) << 1;

            if(val >= 10000000){
                *buf++ = g_digitsLut[d1];
            }

            if(val >= 1000000){
                *buf++ = g_digitsLut[d1 + 1];
            }

            if(val >= 100000){
                *buf++ = g_digitsLut[d2];
            }

            *buf++ = g_digitsLut[d2 + 1];

            *buf++ = g_digitsLut[d3];
            *buf++ = g_digitsLut[d3 + 1];
            *buf++ = g_digitsLut[d4];
            *buf++ = g_digitsLut[d4 + 1];
        }
    }
    else if(val < 10000000000000000) {
        CONST U_32 v0 = (U_32)(val / 100000000);
        CONST U_32 v1 = (U_32)(val % 100000000);

        CONST U_32 b0 = v0 / 10000;
        CONST U_32 c0 = v0 % 10000;

        CONST U_32 d1 = (b0 / 100) << 1;
        CONST U_32 d2 = (b0 % 100) << 1;

        CONST U_32 d3 = (c0 / 100) << 1;
        CONST U_32 d4 = (c0 % 100) << 1;

        CONST U_32 b1 = v1 / 10000;
        CONST U_32 c1 = v1 % 10000;

        CONST U_32 d5 = (b1 / 100) << 1;
        CONST U_32 d6 = (b1 % 100) << 1;

        CONST U_32 d7 = (c1 / 100) << 1;
        CONST U_32 d8 = (c1 % 100) << 1;

        if(val >= 1000000000000000){
            *buf++ = g_digitsLut[d1];
        }

        if(val >= 100000000000000){
            *buf++ = g_digitsLut[d1 + 1];
        }

        if(val >= 10000000000000){
            *buf++ = g_digitsLut[d2];
        }

        if(val >= 1000000000000){
            *buf++ = g_digitsLut[d2 + 1];
        }

        if(val >= 100000000000){
            *buf++ = g_digitsLut[d3];
        }

        if(val >= 10000000000){
            *buf++ = g_digitsLut[d3 + 1];
        }

        if(val >= 1000000000){
            *buf++ = g_digitsLut[d4];
        }

        if(val >= 100000000){
            *buf++ = g_digitsLut[d4 + 1];
        }

        *buf++ = g_digitsLut[d5];
        *buf++ = g_digitsLut[d5 + 1];
        *buf++ = g_digitsLut[d6];
        *buf++ = g_digitsLut[d6 + 1];
        *buf++ = g_digitsLut[d7];
        *buf++ = g_digitsLut[d7 + 1];
        *buf++ = g_digitsLut[d8];
        *buf++ = g_digitsLut[d8 + 1];
    }
    else{
        CONST U_32 a = (U_32)(val / 10000000000000000); /* 1 to 1844 */
        val %= 10000000000000000;

        if(a < 10){
            *buf++ = '0' + (CHAR)(a);
        }
        else if(a < 100){
            CONST U_32 i = a << 1;
            *buf++ = g_digitsLut[i];
            *buf++ = g_digitsLut[i + 1];
        }
        else if(a < 1000){
            *buf++ = '0' + (CHAR)(a / 100);

            CONST U_32 i = (a % 100) << 1;
            *buf++ = g_digitsLut[i];
            *buf++ = g_digitsLut[i + 1];
        }
        else{
            CONST U_32 i = (a / 100) << 1;
            CONST U_32 j = (a % 100) << 1;
            *buf++ = g_digitsLut[i];
            *buf++ = g_digitsLut[i + 1];
            *buf++ = g_digitsLut[j];
            *buf++ = g_digitsLut[j + 1];
        }

        CONST U_32 v0 = (U_32)(val / 100000000);
        CONST U_32 v1 = (U_32)(val % 100000000);

        CONST U_32 b0 = v0 / 10000;
        CONST U_32 c0 = v0 % 10000;

        CONST U_32 d1 = (b0 / 100) << 1;
        CONST U_32 d2 = (b0 % 100) << 1;

        CONST U_32 d3 = (c0 / 100) << 1;
        CONST U_32 d4 = (c0 % 100) << 1;

        CONST U_32 b1 = v1 / 10000;
        CONST U_32 c1 = v1 % 10000;

        CONST U_32 d5 = (b1 / 100) << 1;
        CONST U_32 d6 = (b1 % 100) << 1;

        CONST U_32 d7 = (c1 / 100) << 1;
        CONST U_32 d8 = (c1 % 100) << 1;

        *buf++ = g_digitsLut[d1];
        *buf++ = g_digitsLut[d1 + 1];
        *buf++ = g_digitsLut[d2];
        *buf++ = g_digitsLut[d2 + 1];
        *buf++ = g_digitsLut[d3];
        *buf++ = g_digitsLut[d3 + 1];
        *buf++ = g_digitsLut[d4];
        *buf++ = g_digitsLut[d4 + 1];
        *buf++ = g_digitsLut[d5];
        *buf++ = g_digitsLut[d5 + 1];
        *buf++ = g_digitsLut[d6];
        *buf++ = g_digitsLut[d6 + 1];
        *buf++ = g_digitsLut[d7];
        *buf++ = g_digitsLut[d7 + 1];
        *buf++ = g_digitsLut[d8];
        *buf++ = g_digitsLut[d8 + 1];
    }

    *buf = '\0';

    return buf;
}

FT_PUBLIC CHAR* comlib_utilItoaS64(S_64 val, CHAR* buf)
{
    U_64 u = (U_64)(val);

    if(val < 0){
        *buf++ = '-';
        u = ~u + 1;
    }

    return comlib_utilItoaU64(u, buf);
}

