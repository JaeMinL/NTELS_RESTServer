#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "rest_stat.h"

#define QUERY_LEN 1024
#define TABLE_LEN 32
#define CMP_LEN 1
#define DATE_LEN 20
#define HTTP_BAD_REQUEST 404

FT_PUBLIC RT_RESULT MakeQuery(RsvlibSesCb *sesCb, CHAR *who, CHAR *term)
{
    RrllibDocArg *docArg = NULL;
    UINT strSttLen = 0;
    UINT strEndLen = 0;
    UINT strDataLen = 0;
    CHAR CONST *strStt = NULL;
    CHAR CONST *strEnd = NULL;
    CONST CHAR *strData = NULL;
    CHAR query[QUERY_LEN];
    CHAR table[TABLE_LEN];
    CHAR *arg_stt = "start";
    CHAR *arg_end = "end";
    CHAR *argName = "name";
    CHAR *argIP = "ip";
    CHAR *who_s = who;
    CHAR *which = NULL;
    CHAR strStt_new[DATE_LEN];
    CHAR strEnd_new[DATE_LEN];
    UINT i=0;

    query[0] = '\0';
    table[0] = '\0';
    strStt_new[0] = '\0';
    strEnd_new[0] = '\0';

    SINT ret = RC_NOK;

    if(who == NULL || sesCb == NULL)
    {
        LOGLIB_ERR("REST", "MakeQueryByTime assertion failed\n");
        rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
        return RC_NOK;
    }

    if(!comlib_strNCmp("SERVICE", who, comlib_strGetLen(who)))
    {
        who_s = "SVC";
    }

    /*create DB table name*/
    if(term == NULL)
    {
        i = snprintf(table, TABLE_LEN, "TSD_STAT_%s", who);
        if(i<0 || i>=TABLE_LEN)
        {
            LOGLIB_ERR("REST", "snprintf() make 'table' error\n");
            rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
            return RC_NOK;
        }
    }
    else
    {
        i = snprintf(table, TABLE_LEN, "TSD_STAT_%s_%s", who, term);
        if(i<0 || i>=TABLE_LEN)
        {
            LOGLIB_ERR("REST", "snprintf() make 'table' error\n");
            rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
            return RC_NOK;
        }
    }

    /*create DB Query*/
    i = snprintf(query, QUERY_LEN, "SELECT b.%s_NAME, a.* FROM %s a, TSD_%s b WHERE a.%s_ID = b.%s_ID ", who, table, who_s, who, who);
    if(i<0 || i>= QUERY_LEN)
    {
        LOGLIB_ERR("REST", "snprintf() make DB Query error\n");
        rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
        return RC_NOK;
    }

    /*find start value*/
    ret = rsvlib_apiFindArg(sesCb, arg_stt, &docArg);
    if(ret == RC_OK){
	    ret = rsvlib_apiFirstArgVal(docArg,&strStt, &strSttLen);
	    if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "Insert strEnd failed(ret=%d)\n", ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK; 
		}
		ret = ChDate(strStt, strStt_new);
		if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "change 'start' date type is failed %s(ret=%d)\n",strStt, ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
		i += snprintf(query+i, QUERY_LEN-i, "AND PRC_DATE >= '%s' ", strStt_new);
		if(i<0 || i>= QUERY_LEN)
		{
			LOGLIB_ERR("REST", "snprintf() make DB Query add where error\n");
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
	}

    /*find end value*/
	ret = rsvlib_apiFindArg(sesCb, arg_end, &docArg);
	if(ret == RC_OK){
		ret = rsvlib_apiFirstArgVal(docArg,&strEnd, &strEndLen);
		if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "Insert strEnd failed(ret=%d)\n", ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK; 
		}
		ret = ChDate(strEnd, strEnd_new);
		if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "change 'end' date type is failed %s (ret=%d)\n", strEnd, ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
		i += snprintf(query+i, QUERY_LEN-i, "AND PRC_DATE <= '%s' ", strEnd_new);
		if(i<0 || i>= QUERY_LEN)
		{
			LOGLIB_ERR("REST", "snprintf() make DB Query add where error\n");
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
	}



    /*find arg value 'ip' or 'name' */
    ret = rsvlib_apiFindArg(sesCb, argName, &docArg);
    if(ret != RC_OK)
    {
        ret = rsvlib_apiFindArg(sesCb, argIP, &docArg);
        if( ret == RC_OK && !comlib_strCmp("SERVICE", who) )
        {
            LOGLIB_ERR("REST", "Service has no IP \n");
            rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
            return RC_NOK;
        }
        else if(ret == RC_OK)
        {
            which = "IP";
        }
    }
    else
    {   
        ret = rsvlib_apiFindArg(sesCb, argIP, &docArg);
        if(ret == RC_OK)
        {
            LOGLIB_ERR("REST", "already name for query is exit, donn't need IP arg(ret=%d)\n", ret);
            rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
            return RC_NOK;
        }

        which = "NAME";
    }

    if(which != NULL){
        ret = rsvlib_apiFirstArgVal(docArg, &strData, &strDataLen);
        if(ret == RC_OK)
        {
            i += snprintf(query+i, QUERY_LEN-i, "AND b.%s_%s = '%s' ", who, which, strData);
            if(i<0 || i>= QUERY_LEN)
            {
                LOGLIB_ERR("REST", "snprintf() make DB Query add name value or ip value error\n");
                rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
                return RC_NOK;
            }
        }
    }

    i += snprintf(query+i, QUERY_LEN-i, "LIMIT 10");
    if(i<0 || i>= QUERY_LEN)
    {
        LOGLIB_ERR("REST", "snprintf() make DB Query add 'LIMIT 10' error\n");
        rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
        return RC_NOK;
    }


    ret = DbResult(query, sesCb, who);
    if(ret != RC_OK)
    {
        LOGLIB_ERR("REST", "DbResult() error(ret=%d)\n", ret);
        rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
        return RC_NOK;
    }
    return RC_OK;
    }
