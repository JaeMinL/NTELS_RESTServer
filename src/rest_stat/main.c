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
#define TABLE_LEN 32
#define CMP_LEN 1
#define DATE_LEN 20
#define HTTP_BAD_REQUEST 404

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
FT_PUBLIC RT_RESULT MakeQueryByTime(RsvlibSesCb *sesCb, CHAR *who, CHAR *term);


FT_PUBLIC RT_RESULT DbResult(CHAR * query, RsvlibSesCb *sesCb, CHAR *who)
{
	MYSQL *conn = NULL;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	MYSQL_FIELD *fields = NULL;
	UINT field_cnt = 0;
	UINT i = 0;
	UINT row_cnt=0;
	SINT ret = RC_NOK;
	
	json_object *who_obj = NULL;
	json_object *data_obj = NULL;
	json_object *array_obj = NULL;
	json_object *str_val = NULL;	

	if(who == NULL || query == NULL || sesCb == NULL)
	{
		LOGLIB_ERR("REST", "DbResult assertion failed\n");
		return RC_NOK;
	}	
	LOGLIB_NOTY("REST","%s\n", query);
	/*db connect & send qeury*/
	conn = mysql_init(NULL);
	if(conn == NULL)
	{
		LOGLIB_ERR("REST", "mysql_init failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}
	
	if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PW, DB_NAME, 0, NULL, 0) == NULL)
	{
		LOGLIB_ERR("REST", "connect db is failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		mysql_close(conn);
		return RC_NOK;
	}

	if(mysql_query(conn, query))
	{
		LOGLIB_ERR("REST", "send query is failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		mysql_close(conn);
		return RC_NOK;
	}

	res = mysql_store_result(conn);
	if(res == NULL)
	{
		LOGLIB_ERR("REST", "db data store is failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		mysql_close(conn);
		return RC_NOK;
	}
	
	row_cnt = mysql_num_rows(res);
	field_cnt = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);

	/*change query result to json*/
	who_obj = json_object_new_object();
	if(who_obj == NULL)
	{
		LOGLIB_ERR("REST", "DbResult -> who_obj create failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		mysql_close(conn);
		return RC_NOK;
	}
	
	array_obj = json_object_new_array();
	if(array_obj == NULL)
	{
		LOGLIB_ERR("REST", "DbResult -> array_obj create failed\n");
		mysql_close(conn);
		mysql_free_result(res);
		return RC_NOK;
	}

	while((row = mysql_fetch_row(res)) != NULL)
	{
		data_obj = json_object_new_object();
		if(data_obj == NULL)
		{
			LOGLIB_ERR("REST", "DbResult -> data_obj create failed\n");
			mysql_close(conn);
			mysql_free_result(res);
			return RC_NOK;
		}
		for(i=0; i<field_cnt; i++)
		{	
			if(row[i] == NULL)
			{
				str_val = json_object_new_string("NULL");
			}
			else
			{
				str_val = json_object_new_string(row[i]);
			}
			json_object_object_add(data_obj, fields[i].name, str_val);
		}
		json_object_array_add(array_obj, data_obj);
	}
	json_object_object_add(who_obj, who, array_obj);

	ret = rsvlib_apiSetRspDat(sesCb, (CHAR *)json_object_get_string(who_obj), RC_TRUE);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "DbResult -> rsvlib_apiSetRspDat() call failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		mysql_free_result(res);
		mysql_close(conn);
		return RC_NOK;
	}

	/*json object free*/
	json_object_put(who_obj);
	mysql_library_end();
	mysql_free_result(res);
	mysql_close(conn);

	return RC_OK;
}

FT_PUBLIC RT_RESULT MakeQueryByInfo(RsvlibSesCb *sesCb, CHAR *who, CHAR *term)
{
	RrllibDocArg *docArg = NULL;
	UINT strDataLen = 0;
	CONST CHAR *strData = NULL;
	CHAR query[QUERY_LEN];
	CHAR table[TABLE_LEN];
	CHAR *which = NULL;
	CHAR *argName = "name";
	CHAR *argIP = "ip";
	CHAR *who_s = who;
	UINT i = 0;

	query[0] = '\0';
	table[0] = '\0';

	SINT ret = RC_NOK;

	if(who == NULL || sesCb == NULL)
	{
		LOGLIB_ERR("REST", "MakeQueryByInfo assertion failed\n");
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	/*find arg value 'ip' or 'name' */
	ret = rsvlib_apiFindArg(sesCb, argName, &docArg);
	if(ret != RC_OK)
	{
		ret = rsvlib_apiFindArg(sesCb, argIP, &docArg);
		if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "Find srgIP argument failed(ret=%d)\n", ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
		which = "IP";
	}
	else
	{
		which = "NAME";
	}

	ret = rsvlib_apiFirstArgVal(docArg, &strData, &strDataLen);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Find strName value failed(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	/*create DBtable name*/
	if(!comlib_strCmp("SERVICE", who))
	{
		who_s = "SVC";
	}

	/*create DBtable name*/
	if(term == NULL)
	{
		i = snprintf(table, TABLE_LEN, "TSD_STAT_%s", who);
		if(i==0)
		{
			LOGLIB_ERR("REST", "snprintf() make 'table' error\n");
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
	}
	else
	{
		i = snprintf(table, TABLE_LEN, "TSD_STAT_%s_%s", who, term);
		if(i==0)
		{
			LOGLIB_ERR("REST", "snprintf() make 'table' error\n");
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}
	}
	/*create DB Query*/
	i = snprintf(query, QUERY_LEN, "SELECT b.%s_NAME, a.* FROM %s a, TSD_%s b WHERE a.%s_ID = b.%s_ID AND b.%s_%s = '%s'", who, table, who_s, who, who, who, which, strData); 
	if(i==0)
	{
		LOGLIB_ERR("REST", "snprintf() make DB Query error\n");
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

FT_PUBLIC RT_RESULT MakeQueryByTime(RsvlibSesCb *sesCb, CHAR *who, CHAR *term)
	{
	RrllibDocArg *docArg = NULL;
	UINT strSttLen = 0;
	UINT strEndLen = 0;
	CHAR CONST *strStt = NULL;
	CHAR CONST *strEnd = NULL;
	CHAR query[QUERY_LEN];
	CHAR table[TABLE_LEN];
	CHAR *arg_stt = "start";
	CHAR *arg_end = "end";
	CHAR *who_s = who;
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

	ret = rsvlib_apiFindArg(sesCb, arg_stt, &docArg);
	if(ret != RC_OK)
	{
		ret = rsvlib_apiFindArg(sesCb, arg_end, &docArg);
		if(ret != RC_OK)
		{
			/*start and end value is null*/
		}
		else
		{
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
				LOGLIB_ERR("REST", "change 'end' date type is failed(ret=%d)\n", ret);
				rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
				return RC_NOK;
			}
			/* ... ~ end*/
			i += snprintf(query+i, QUERY_LEN-i, "AND PRC_DATE <= '%s' ", strEnd_new);
			if(i<0 || i>= QUERY_LEN)
			{
				LOGLIB_ERR("REST", "snprintf() make DB Query add where error\n");
				rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
				return RC_NOK;
			}
		}

	}
	else
	{
		ret = rsvlib_apiFirstArgVal(docArg, &strStt, &strSttLen);
		if(ret != RC_OK)
		{
			LOGLIB_ERR("REST", "Insert strStt failed (ret=%d)\n", ret);
			rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
			return RC_NOK;
		}

		ret = rsvlib_apiFindArg(sesCb, arg_end, &docArg);
		if(ret != RC_OK)
		{
			/* start ~ ...*/
			ret = ChDate(strStt, strStt_new);
			if(ret != RC_OK)
			{
				LOGLIB_ERR("REST", "change 'start' date type is failed(ret=%d)\n", ret);
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
		else
		{
			ret = rsvlib_apiFirstArgVal(docArg, &strEnd, &strEndLen);
				
			if(ret != RC_OK)
			{
				LOGLIB_ERR("REST", "Insert strEnd failed(ret=%d)\n", ret);
				rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
				return RC_NOK;
			}
			else
			{
				/* start ~ end*/
				ret = ChDate(strStt, strStt_new);
				if(ret != RC_OK)
				{
					LOGLIB_ERR("REST", "change 'start' date type is failed(ret=%d)\n", ret);
					rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
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

				ret = ChDate(strEnd, strEnd_new);
				if(ret != RC_OK)
				{
					LOGLIB_ERR("REST", "change 'end' date type is failed(ret=%d)\n", ret);
					rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
					return RC_NOK;
				}
				i += snprintf(query+i , QUERY_LEN-i, "AND PRC_DATE <= '%s' ", strEnd_new);
				if(i<0 || i>= QUERY_LEN)
				{
					LOGLIB_ERR("REST", "snprintf() make DB Query add where error\n");
					rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
					return RC_NOK;
				}
			}
		}
		
	}	

	//snprintf(query, QUERY_LEN, "SELECT b.%s_NAME, a.* FROM %s a, TSD_%s b WHERE a.%s_ID = b.%s_ID %s", who, table, who_s, who, who, where);

	ret = DbResult(query, sesCb, who);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "DbResult() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}
	return RC_OK;
	}




FT_PUBLIC RT_RESULT Host_1min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", NULL);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Host_1min() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Host_5min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "5MIN");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Host_5min() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_hour(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "HOUR");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Host_hour() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_day(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "HOST", "DAY");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Host_day() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_1min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", NULL);
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "SDV_1min() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_5min(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "5MIN");
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "SDV_5min() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_hour(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "HOUR");
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "SDV_hour() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Svc_day(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByInfo(sesCb, "SERVICE", "DAY");
	if(ret != RC_OK){
		LOGLIB_ERR("REST", "SDV_day() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_1min_time(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "HOST", NULL);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "HOST_1min_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_5min_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "HOST", "5MIN");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "HOST_5min_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Host_hour_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "HOST", "HOUR");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "HOST_hour_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}
FT_PUBLIC RT_RESULT Host_day_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "HOST", "DAY");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "HOST_day_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_1min_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "SERVICE", NULL);
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "SERIVCE_1min_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_5min_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "SERVICE", "5MIN");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Svc_5min_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_hour_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;

	ret = MakeQueryByTime(sesCb, "SERVICE", "HOUR");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST", "Svc_hour_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC RT_RESULT Svc_day_time(UINT mthod, RsvlibSesCb *sesCb)
{
	SINT ret = RC_NOK;
	ret = MakeQueryByTime(sesCb, "SERVICE", "DAY");
	if(ret != RC_OK)
	{
		LOGLIB_ERR("REST","Svc_day_time() error(ret=%d)\n", ret);
		rsvlib_apiSetStaCode(sesCb, HTTP_BAD_REQUEST);
		return RC_NOK;
	}

	return RC_OK;
}

FT_PUBLIC VOID logPrnt(UINT lvl, CHAR *file, UINT line, CHAR *logStr)
{
	printf("[%d][%s:%d] %s\n",lvl, file, line, logStr);
}

int main(int argc, char **argv)
{
	SINT ret = RC_OK;
	SINT opt = 0;
#if 0
    CHAR cfgPath[MAIN_CFG_PATH_LEN];
#endif
	CHAR logCfgPath[MAIN_CFG_PATH_LEN];
	LoglibCfg logCfg;
	RsvlibGenCfg rsvCfg;

#if 0
	cfgPath[0] = '\0';
#endif
	logCfgPath[0] = '\0';

	while(1){
		opt = getopt(argc, argv,"c:l:h:");
		if(opt == -1)
		{
			break;
		}

		switch(opt){
#if 0
			case 'c':
				{
					comlib_strSNPrnt(cfgPath, MAIN_CFG_PATH_LEN, "%s", optarg);
				}
				break;
#endif
			case 'l':
				{
					comlib_strSNPrnt(logCfgPath, MAIN_CFG_PATH_LEN, "%s", optarg);
				}
				break;
			case 'h':
				{
					fprintf(stderr,"usage : rest_stat -c [config path] -l [log config path]\n");
					exit(1);
				}
				break;
			default:
				{
					fprintf(stderr,"option \"%c\" is unknown\n",opt);
					fprintf(stderr,"usage : rest_stat -c [config path] -l [log config path]\n");
					exit(1);
				}
				break;
		}/* end of switch(opt) */
	}/* end of while */

#if 0
	if(cfgPath[0] == '\0'){
		fprintf(stderr,"CONFIG NOT EXIST\n");
		exit(1);
	}
#endif

	if(logCfgPath[0] == '\0')
	{
		fprintf(stderr,"LOG CONFIG NOT EXIST\n");
		exit(1);
	}

    /* log setting */
	LOGLIB_GLOB_INIT();

	LOGLIB_INIT_CFG(&logCfg);

	ret = loglib_apiLoadToml(logCfgPath, "REST_IF");
	if(ret != RC_OK){
		fprintf(stderr,"Log config load failed(ret=%d)\n",ret);
	}


	LOGLIB_NOTY("REST","REST INTERFACE START\n");

	/* rest server setting */
	RSV_INIT_GEN_CFG(&rsvCfg, 8800);

	ret = rsvlib_apiInit(1, &rsvCfg);
	if(ret != RC_OK){
		LOGLIB_ERR("REST","REST SERVER INIT FAILED(ret=%d)\n", ret);
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
	//exit(1);
	//rsvlib_apiDstry(1);
	//loglib_apiDstryLoglibCb(&loglibCb);	

	return 0;
}
