#include <stdio.h>
#include <string.h>

#include "gendef.h"
#include "comlib.h"
#include "comlib.x"
#include "rrllib.h"
#include "rrllib.x"

RrllibCb rrlCb;

char peerCmd[] = "ADD_DIAM_PEER";
char realmCmd[] = "DIS_REALM";

int regURL()
{
    SINT ret = 0;
#if 0
    CHAR *pathName = NULL;
    CHAR *queryName = NULL;
    RrllibResPathLst *resPathLst = NULL;
    RrllibResPath *resPath = NULL;
    RrllibResPath *resDiamPath = NULL;
    RrllibResPath *resPeerPath = NULL;
#endif
    CHAR *uri = NULL;
    CHAR *query = NULL;

    rrllib_init(&rrlCb);

    uri = "/diameter/peer/{host}";
    query = "{ip:3}{port} [tls]";
    ret = rrllib_parseResCfg(&rrlCb, uri, strlen(uri), RRL_MTHOD_PUT, query, strlen(query), NULL);

    //uri = "/diameter/realms/";
    //ret = rrllib_parseResCfg(&rrlCb, uri, strlen(uri));

    uri = "/diameter/realms/{realm}";
    query = "{app}";
    ret = rrllib_parseResCfg(&rrlCb, uri, strlen(uri), RRL_MTHOD_PUT, query, strlen(query),NULL);

    uri = "/diameter/realms";
    ret = rrllib_parseResCfg(&rrlCb, uri, strlen(uri), RRL_MTHOD_GET, NULL, 0, NULL);
#if 0 
    rrllib_pathAddFirstFixPathLst(&rrlCb, &resPathLst);

    pathName = "diameter";
    rrllib_pathAddFixPath(resPathLst, pathName, strlen(pathName), &resDiamPath);

    /* peer */
    rrllib_pathAddNxtFixPathLst(resDiamPath, &resPathLst);

    pathName = "peer";
    rrllib_pathAddFixPath(resPathLst, pathName, strlen(pathName), &resPeerPath);


    pathName = "host";
    rrllib_pathAddNxtDynPath(resPeerPath, pathName, strlen(pathName), &resPath);


    ret = rrllib_pathAddNxtDynPath(resPath , pathName, strlen(pathName), NULL);
    if(ret != RC_OK){
        fprintf(stderr,"DynPath add failed(ret=%d)\n",ret);
    }

    queryName= "ip";
    rrllib_queryAdd(resPath, queryName, strlen(queryName), 1);

    rrllib_mthodAdd(resPath, RRL_MTHOD_POST, peerCmd);

    /* realm */
    pathName = "realm";
    rrllib_pathAddFixPath(resPathLst, pathName, strlen(pathName), &resPath);

    rrllib_mthodAdd(resPath, RRL_MTHOD_GET, realmCmd);
#endif
    
    return 0;
}

int main()
{
    SINT ret = 0;
    CHAR *uri = NULL;
    UINT mthod = RRL_MTHOD_GET;
    RrllibDoc *doc = NULL;

    printf("TEST START\n");

    regURL();

#if 0
    uri = "/diameter/peer/hlr";
    mthod = RRL_MTHOD_POST;
    ret = rrllib_parseUri(&rrlCb, mthod, (CONST CHAR*)uri, strlen(uri), &doc);
    if(ret == RC_OK){
        rrllib_docPrnt(doc);
        rrllib_docDstry(doc);
    }

    uri = "/diameter/peer/cscf";
    mthod = RRL_MTHOD_GET;
    ret = rrllib_parseUri(&rrlCb, mthod, uri, strlen(uri), &doc);
    if(ret == RC_OK){
        rrllib_docPrnt(doc);
        rrllib_docDstry(doc);
    }

    uri = "/diameter/realm";
    ret = rrllib_parseUri(&rrlCb, mthod, uri, strlen(uri), &doc);
    if(ret == RC_OK){
        rrllib_docPrnt(doc);
        rrllib_docDstry(doc);
    }

    uri = "/diameter/realm/skt.com";
    ret = rrllib_parseUri(&rrlCb, mthod, uri, strlen(uri), &doc);
    if(ret == RC_OK){
        rrllib_docPrnt(doc);
        rrllib_docDstry(doc);
    }

#endif
#if 1
    uri = "/diameter/peer/hlr?ip=192.168.0.1&ip=127.0.0.1&test=ddd";
    uri = "/diameter/peer/hlr?ip=192.168.0.1&ip=127.0.0.1&ip=192.168.0.1&port=3868";
    mthod = RRL_MTHOD_PUT;
    ret = rrllib_parseUri(&rrlCb, mthod, uri, strlen(uri), &doc);
    if(ret == RC_OK){
        rrllib_docPrnt(doc);
        rrllib_docDstry(doc);
    }
#endif

#if 0
    CHAR *endTkn = NULL;
    uri = "/{diameter}/{peer}/host?ip=1212";
    //uri = "diameter";
    //ret = rrllib_parseResPathCfg(&rrlCb, uri, strlen(uri), &endTkn);
    ret = rrllib_parseResCfg(&rrlCb, uri, strlen(uri));
#endif


    //char *tkn;
    //rrllib_parseQuery("ip=192.168.0.1&ip=127.0.0.1&", strlen("ip=192.168.0.1&ip=127.0.0.1&"), rrllib_parseKvpToDoc, NULL);

    return 0;
}
