#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json/json.h>

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
#define TABLE_LEN 64

FT_PUBLIC RT_RESULT MakeQueryByInfo(RsvlibSesCb *sesCb, CHAR *who, CHAR *term){
	if(who == NULL || sesCb == NULL){
		fprintf(stderr, "MakeQueryByInfo assertion failed \n");
		return RC_NOK;
	}

    RrllibDocArg *docArg = NULL;
    UINT strDataLen = 0;
    CONST CHAR *strData = NULL;
    CHAR query[QUERY_LEN];
    CHAR table[TABLE_LEN];
    CHAR *which = NULL;
    CHAR *argName = "name";
    CHAR *argIP = "ip";
    CHAR *who_s = who;

   	query[0] = '\0';
   	table[0] = '\0';

    SINT ret = RC_NOK;
    /*find arg value 'ip' or 'name' */
    ret = rsvlib_apiFindArg(sesCb, argName, &docArg);
    if(ret != RC_OK){
	ret = rsvlib_apiFindArg(sesCb, argIP, &docArg);
    	if(ret != RC_OK){
        	fprintf(stderr, "no arg : %s & %s\n", argIP, argName);
        	return RC_NOK;
    	}
	which = "IP";
    }
    else{which = "NAME";}

    ret = rsvlib_apiFirstArgVal(docArg, &strData, &strDataLen);
    if(ret != RC_OK){
    	fprintf(stderr, "strName=%s\n", strData);
    	return RC_NOK;
    }    

    /*create DBtable name*/
    if(!comlib_strCmp("SERVICE", who)){who_s = "SVC";}

    /*create DBtable name*/
    if(term == NULL){
    	snprintf(table, TABLE_LEN, "TSD_STAT_%s", who);
    }
    else{
    	snprintf(table, TABLE_LEN, "TSD_STAT_%s_%s", who, term);
    }
    /*create DB Query*/
	snprintf(query, QUERY_LEN, "SELECT b.%s_NAME, a.* FROM %s a, TSD_%s b WHERE a.%s_ID = b.%s_ID AND b.%s_%s = '%s'", who, table, who_s, who, who, who, which, strData); 
    
    ret = DbResult(query, sesCb, who);
    if(ret != RC_OK)
    {
    	fprintf(stderr, "DbResult() error");
    	return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT MakeQueryByTime(RsvlibSesCb *sesCb, CHAR *who, CHAR *term){
    if(who == NULL || sesCb == NULL){
	fprintf(stderr, "MakeQueryByTime assertion failed \n");
	return RC_NOK;
    }

    RrllibDocArg *docArg = NULL;
    UINT strSttLen = 0;
    UINT strEndLen = 0;
    CONST CHAR *strStt = NULL;
    CONST CHAR *strEnd = NULL;
    CHAR query[QUERY_LEN];
    CHAR table[TABLE_LEN];
    CHAR where[QUERY_LEN];
    CHAR *arg_stt = "start";
    CHAR *arg_end = "end";
    CHAR *who_s = who;

    query[0] = '\0';
    table[0] = '\0';
    where[0] = '\0';
    SINT ret = RC_NOK;

    ret = rsvlib_apiFindArg(sesCb, arg_stt, &docArg);
    if(ret != RC_OK){
	ret = rsvlib_apiFindArg(sesCb, arg_end, &docArg);
	if(ret != RC_OK){
		/*start and end value is null*/
	    	snprintf(where, QUERY_LEN, " ");
	}
	else{
		ret = rsvlib_apiFirstArgVal(docArg, &strEnd, &strEndLen);
        	if(ret != RC_OK){
                	fprintf(stderr, "strEnd=%s\n", strEnd);
                	return RC_NOK; 
       		}
		strEnd = ChDate(strEnd);
		if(!strEnd){
			fprintf(stderr, "change 'end' date type is failed\n");
			return RC_NOK;
		}
		/* ... ~ end*/
		snprintf(where, QUERY_LEN, "AND PRC_DATE <= '%s' ", strEnd);
	}

    }
    else{
   		ret = rsvlib_apiFirstArgVal(docArg, &strStt, &strSttLen);
	   	if(ret != RC_OK){
	   		fprintf(stderr, "strStt=%s\n", strStt);
			return RC_NOK;
	   	}

		ret = rsvlib_apiFindArg(sesCb, arg_end, &docArg);
		if(ret != RC_OK){
			/* start ~ ...*/
			strStt = ChDate(strStt);
			if(!strStt){
				fprintf(stderr, "change 'start' date type is failed\n");
				return RC_NOK;
			}
		    snprintf(where, QUERY_LEN, "AND PRC_DATE >= '%s' ", strStt);
		}
		else{
			ret = rsvlib_apiFirstArgVal(docArg, &strEnd, &strEndLen);
			if(ret != RC_OK){
				fprintf(stderr, "strEnd=%s\n", strEnd);
				return RC_NOK;
			}
			else{
				strStt = ChDate(strStt);
				if(!strStt){
					fprintf(stderr, "change 'start' date type is failed\n");
					return RC_NOK;
				}
				strEnd = ChDate(strEnd);
				if(!strEnd){
					fprintf(stderr, "change 'end' date type is failed\n");
					return RC_NOK;
				}
				/* start ~ end*/
				snprintf(where, QUERY_LEN, "AND PRC_DATE >= '%s' AND PRC_DATE <= '%s' ", strStt, strEnd);
			}
		}
		
    }	

    if(!comlib_strNCmp("SERVICE", who, comlib_strGetLen(who))){who_s = "SVC";}
    /*create DB table name*/
    if(term == NULL){
	snprintf(table, TABLE_LEN, "TSD_STAT_%s", who);
    }
    else{
	snprintf(table, TABLE_LEN, "TSD_STAT_%s_%s", who, term);
    }

    /*create DB Query*/
    snprintf(query, QUERY_LEN, "SELECT b.%s_NAME, a.* FROM %s a, TSD_%s b WHERE a.%s_ID = b.%s_ID %s", who, table, who_s, who, who, where);
	
    ret = DbResult(query, sesCb, who);
	if(ret != RC_OK)
	{
		fprintf(stderr, "DbResult() error");
		return RC_NOK;
	}

	return RC_OK;
}
