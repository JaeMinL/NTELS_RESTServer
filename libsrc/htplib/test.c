#include <stdio.h>
#include <unistd.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"

#include "htplib.h"
#include "htplib.x"

FT_PUBLIC VOID check_staCode()
{
    SINT ret = 0;
    UINT i = 0;
    UINT len = 0;
    U_16 staCode = 0;
    CHAR strStaCode[3];

    for(i=0;i<599;i++){
        len = sprintf(strStaCode, "%d", i);
        ret = htplib_staCodeCvtStrToNum(strStaCode, len, &staCode);
        if(ret != RC_OK){
            fprintf(stderr,"Status code convert failed(ret=%d)\n",ret);
            continue;
        }
        if(staCode != i){
            fprintf(stderr,"INVALID CODE(%d, %d)\n",staCode, i);
        }
        fprintf(stderr,"IDX=[%d], STATUS_CODE=[%d]\n",i, staCode);
    }
}

FT_PUBLIC VOID check_mthod()
{
    UINT mthodId = 0;

    htplib_mthodCvtStrToId("heAd",4, &mthodId);

    fprintf(stderr,"MTHOD_ID=%d\n",mthodId);
}

FT_PUBLIC VOID check_req()
{
    SINT ret = 0;
#if 0
    CHAR httpHdr[] = "HTTP/1.1 200 OK\n\
Date: Sat, 19 May 2007 13:49:37 GMT\n\
Server: IBM_HTTP_SERVER/1.3.26.2 Apache/1.3.26 (Unix)\n\
Set-Cookie: tracking=tI8rk7joMx44S2Uu85nSWc\n\
Pragma: no-cache\n\
Expires: Thu, 01 Jan 1970 00:00:00 GMT\n\
Content-Type: text/html;charset=ISO-8859-1\n\
Content-Language: en-US\n\
Content-Length: 24246";
#else
    CHAR httpHdr[] = "GET /books/search.asp HTTP/1.1\n\
Accept: image/gif, image/xxbitmap, image/jpeg, image/pjpeg,\n\
application/xshockwaveflash, application/vnd.msexcel,\n\
application/vnd.mspowerpoint, application/msword, */*\n\
Referer: http://wahh-app.com/books/default.asp\n\
Accept-Language: en-gb,en-us;q=0.5\n\
Accept-Encoding: gzip, deflate\n\
User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)\n\
Host: wahh-app.com\n\
Cookie: lang=en; JSESSIONID=0000tI8rk7joMx44S2Uu85nSWc_:vsnlc502";
#endif

    ret = htplib_mainHtpIsReq(httpHdr, sizeof(httpHdr));
    if(ret == RC_OK){
        fprintf(stderr,"HTTP is request\n");
        return;
    }
    else {
        fprintf(stderr,"HTTP is response\n");
        return;
    }
}

int main()
{
    htplib_initGlob();

    check_staCode();

    check_mthod();

    check_req();

    return 0;
}
