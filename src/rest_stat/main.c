#include <unistd.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <json/json.h>
//#include <json-c/json.h>

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

#define DB_HOST "192.168.6.84"
#define DB_USER "sdv"
#define DB_PW "sdv2016"
#define DB_NAME "SDV"
#define QUERY_LEN 1024
#define TABLE_LEN 64
#define CMP_LEN 1


FT_PUBLIC RT_RESULT Host_1min(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_5min(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_hour(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_day(UINT mthod, RsvlibSesCb *sesCb);

FT_PUBLIC RT_RESULT Svc_1min(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_5min(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_hour(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_day(UINT mthod, RsvlibSesCb *sesCb);

FT_PUBLIC RT_RESULT Host_1min_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_5min_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_hour_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Host_day_time(UINT mthod, RsvlibSesCb *sesCb);

FT_PUBLIC RT_RESULT Svc_1min_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_5min_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_hour_time(UINT mthod, RsvlibSesCb *sesCb);
FT_PUBLIC RT_RESULT Svc_day_time(UINT mthod, RsvlibSesCb *sesCb);

FT_PUBLIC RT_RESULT DbResult(CHAR * query, RsvlibSesCb *sesCb, CHAR *who);

FT_PUBLIC RT_RESULT MakeQueryByInfo(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);
FT_PUBLIC RT_RESULT MakeQueryByIP(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);
FT_PUBLIC RT_RESULT MakeQueryByTime(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);


FT_PUBLIC RT_RESULT DbResult(CHAR * query, RsvlibSesCb *sesCb, CHAR *who){
	if(who == NULL || query == NULL || sesCb == NULL){
		fprintf(stderr, "DbResult assertion failed \n");
		return RC_NOK;
	}
	
	printf("\n\n%s\n", query);
	
	MYSQL *conn = NULL;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	MYSQL_FIELD *fields = NULL;
	UINT field_cnt = 0;
	UINT i;
	UINT row_cnt = 0;

	SINT ret = RC_NOK;
	
	json_object *who_obj = NULL;
	json_object *data_obj = NULL;
	json_object *array_obj = NULL;

	/*db connect & send qeury*/
	conn = mysql_init(NULL);
	if( conn == NULL){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return RC_NOK;
	}
	
	if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PW, DB_NAME, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "connect db is failed : %s\n", mysql_error(conn));
		mysql_close(conn);
		return RC_NOK;
	}

	if(mysql_query(conn, query)){
		fprintf(stderr, "send query is failed : %s\n", mysql_error(conn));
		mysql_close(conn);
		return RC_NOK;
	}

	res = mysql_store_result(conn);
	if(res == NULL){
		fprintf(stderr, "db data store is failed : %s\n", mysql_error(conn));
		mysql_close(conn);
		return RC_NOK;
	}

	row_cnt = mysql_num_rows(res);
	field_cnt = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);

	/*change query result to json*/
	who_obj = json_object_new_object();
	if(who_obj == NULL){
		fprintf(stderr, "DbResult -> who_obj create fail!\n");
		mysql_close(conn);
		return RC_NOK;
	}
	
	array_obj = json_object_new_array();
	if(array_obj == NULL){
		fprintf(stderr, "DbResult -> array_obj create fail!\n");
		mysql_close(conn);
		mysql_free_result(res);
		return RC_NOK;
	}

	printf("row 갯수 : %d\n",row_cnt);	
	while((row = mysql_fetch_row(res)) != NULL)
	{
		data_obj = json_object_new_object();
		if(data_obj == NULL){
			fprintf(stderr, "DbResult -> data_obj create fail!\n");
			mysql_close(conn);
			mysql_free_result(res);
			return RC_NOK;
		}

		for(i=0; i<field_cnt; i++)
		{	
			json_object_object_add(data_obj, fields[i].name, json_object_new_string(row[i]));
		}
		json_object_array_add(array_obj, data_obj);
	}
	json_object_object_add(who_obj, who, array_obj);

	//ret = rsvlib_apiSetRspDat(sesCb, (CHAR *)json_object_to_json_string(who_obj), RC_TRUE);
	ret = rsvlib_apiSetRspDat(sesCb, (CHAR *)json_object_get_string(who_obj), RC_TRUE);
	if(ret != RC_OK){
		fprintf(stderr, "DbResult -> rsvlib_apiSetRspDat() call fail!\n");
		mysql_free_result(res);
		mysql_close(conn);
		return RC_NOK;
	}

	/*json object free*/
	/* for(i=0; i<row_cnt; i++)
	{	
		for(j=0; j<field_cnt; j++)
		{
			json_object_object_del(json_object_array_get_idx(array_obj, 0), fields[j].name);
			json_object_put(json_object_array_get_idx(array_obj, 0), fields[j].name);
			
		}
		json_object_put(json_object_array_get_idx(array_obj, 0));
	}
	

	json_object_put(array_obj);*/
	json_object_put(who_obj);
	//rsvlib_apiSetRspDat(sesCb, query, RC_TRUE);
	mysql_free_result(res);
	mysql_close(conn);

	return RC_OK;
}

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
    CHAR CONST *strStt = NULL;
    CHAR CONST *strEnd = NULL;
    CHAR query[QUERY_LEN];
    CHAR table[TABLE_LEN];
    CHAR where[QUERY_LEN];
    CHAR *arg_stt = "start";
    CHAR *arg_end = "end";
    CHAR *who_s = who;
    CHAR *strStt_new = NULL;
    CHAR *strEnd_new = NULL;

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
		ret = rsvlib_apiFirstArgVal(docArg,&strEnd, &strEndLen);
        	if(ret != RC_OK){
                	fprintf(stderr, "strEnd=%s\n", strEnd);
                	return RC_NOK; 
       		}
		ret = ChDate(strEnd, &strEnd_new);
		if(ret != RC_OK){
			fprintf(stderr, "change 'end' date type is failed\n");
			return RC_NOK;
		}
		/* ... ~ end*/
		snprintf(where, QUERY_LEN, "AND PRC_DATE <= '%s' ", strEnd_new);
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
			ret = ChDate(strStt, strStt_new);
			if(ret != RC_OK){
				fprintf(stderr, "change 'start' date type is failed\n");
				return RC_NOK;
			}
		    snprintf(where, QUERY_LEN, "AND PRC_DATE >= '%s' ", strStt_new);
		}
		else{
			ret = rsvlib_apiFirstArgVal(docArg, &strEnd, &strEndLen);
			if(ret != RC_OK){
				fprintf(stderr, "strEnd=%s\n", strEnd);
				return RC_NOK;
			}
			else{
				ret = ChDate(strStt, &strStt_new);
				if(ret != RC_OK){
					fprintf(stderr, "change 'start' date type is failed\n");
					return RC_NOK;
				}
				ret = ChDate(strEnd, &strEnd_new);
				if(ret != RC_OK){
					fprintf(stderr, "change 'end' date type is failed\n");
					return RC_NOK;
				}
				/* start ~ end*/
				snprintf(where, QUERY_LEN, "AND PRC_DATE >= '%s' AND PRC_DATE <= '%s' ", strStt_new, strEnd_new);
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




FT_PUBLIC RT_RESULT Host_1min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", NULL);
	if(ret != RC_OK){
		fprintf(stderr, "Host_1min() error");
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Host_5min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "5MIN");
	if(ret != RC_OK){
		fprintf(stderr, "Host_5min() error");
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_hour(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "HOUR");
	if(ret != RC_OK){
		fprintf(stderr, "Host_hour() error");
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_day(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "DAY");
	if(ret != RC_OK){
		fprintf(stderr, "Host_day() error");
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_1min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", NULL);
	if(ret != RC_OK){
		fprintf(stderr, "SDV_1min() error");
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_5min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "5MIN");
	if(ret != RC_OK){
		fprintf(stderr, "SDV_5min() error");
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_hour(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "HOUR");
	 if(ret != RC_OK){
		fprintf(stderr, "SDV_hour() error");
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_day(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "DAY");
	if(ret != RC_OK){
		fprintf(stderr, "SDV_day() error");
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_1min_time(UINT mthod, RsvlibSesCb *sesCb){
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "HOST", NULL);
    if(ret != RC_OK){
            fprintf(stderr, "HOST_1min_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Host_5min_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "HOST", "5MIN");
    if(ret != RC_OK){
            fprintf(stderr, "HOST_5min_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Host_hour_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "HOST", "HOUR");
    if(ret != RC_OK){
            fprintf(stderr, "HOST_hour_time() error");
            return RC_NOK;
    }

    return RC_OK;
}
FT_PUBLIC RT_RESULT Host_day_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "HOST", "DAY");
    if(ret != RC_OK){
            fprintf(stderr, "HOST_day_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_1min_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "SERVICE", NULL);
    if(ret != RC_OK){
            fprintf(stderr, "SERIVCE_1min_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_5min_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "SERVICE", "5MIN");
    if(ret != RC_OK){
            fprintf(stderr, "Svc_5min_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_hour_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "SERVICE", "HOUR");
    if(ret != RC_OK){
            fprintf(stderr, "Svc_hour_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_day_time(UINT mthod, RsvlibSesCb *sesCb)
{
    SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "SERVICE", "DAY");
    if(ret != RC_OK){
            fprintf(stderr, "Svc_day_time() error");
            return RC_NOK;
    }

    return RC_OK;
}

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr)
{
    printf("[%d][%s:%d] %s\n",lvl, file, line, logStr);
}

int main()
{
    SINT ret = RC_OK;
    LoglibCfg logCfg;
    RsvlibGenCfg rsvCfg;
    LoglibCb loglibCb;
    
    /* log setting */
    LOGLIB_GLOB_INIT();

    LOGLIB_INIT_CFG(&logCfg);

    loglib_apiInitLoglibCb(&loglibCb, &logCfg);

    /* rest server setting */
    RSV_INIT_GEN_CFG(&rsvCfg, 8800);

    ret = rsvlib_apiInit(1, &rsvCfg);
    if(ret != RC_OK){
        LOGLIB_ERR(&loglibCb, "REST SERVER INIT FAILED(ret=%d)\n", ret);
        return RC_NOK;
    }

    rsvlib_apiSetLogFunc(RSV_DBG, logPrnt);
    
    /* url rule setting */
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/1min", "[name][ip]", NULL, Host_1min);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/5min", "[name][ip]", NULL, Host_5min);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/hour", "[name][ip]", NULL, Host_hour);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/day", "[name][ip]", NULL, Host_day);

    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/1min", "{name}", NULL, Svc_1min);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/5min", "{name}", NULL, Svc_5min);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/hour", "{name}", NULL, Svc_hour);    
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/day", "{name}", NULL, Svc_day);  
  
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/1min/term", "[start] [end]", NULL, Host_1min_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/5min/term", "[start] [end]", NULL, Host_5min_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/hour/term", "[start] [end]", NULL, Host_hour_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/host/day/term", "[start] [end]", NULL, Host_day_time);

    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/1min/term", "[start] [end]", NULL, Svc_1min_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/5min/term", "[start] [end]", NULL, Svc_5min_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/hour/term", "[start] [end]", NULL, Svc_day_time);
    ret = rsvlib_apiSetRule(1, RSV_MTHOD_GET, "/svc/day/term", "[start] [end]", NULL, Svc_hour_time);

    /* run rest server */
    
    rsvlib_apiRun(1);

    while(1){
        sleep(1);
    }

    rsvlib_apiStop(1);	
    return 0;
}
