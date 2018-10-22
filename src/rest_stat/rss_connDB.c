#include <json/json.h>
#include <mysql/mysql.h>

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
#define HTTP_BAD_REQUEST 404

FT_PUBLIC RT_RESULT rss_connDBGetResult(CHAR * query, RsvlibSesCb *sesCb, CONST CHAR *who)
{
	MYSQL *conn = NULL;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	MYSQL_FIELD *fields = NULL;
	UINT field_cnt = 0;
	UINT i = 0;
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