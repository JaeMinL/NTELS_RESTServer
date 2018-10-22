#define __USE_XOPEN
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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

#define DATE_LEN 20 

FT_PUBLIC RT_RESULT rss_dateChange(CONST CHAR *date_old, CHAR *date_store){
	UINT i = 0;
	struct tm  result = {0};

	if(date_old == NULL){
		return RC_NOK;
	}	
	
	if(comlib_strGetLen(date_old) != DATE_LEN-1 || !isdigit(date_old[DATE_LEN-2]))
	{
		LOGLIB_ERR("REST", "date type is not correct\n");
		return RC_NOK;
	}

	if(strptime(date_old, "%Y-%m-%d_%H:%M:%S", &result) == NULL)
	{
		LOGLIB_ERR("REST", "date type is not correct\n");
		return RC_NOK;
	}
	//printf("%d %d %d: %d %d %d", result.tm_year+1900, result.tm_mon+1, result.tm_mday, result.tm_hour, result.tm_min, result.tm_sec);
	i = snprintf((CHAR *)date_store, DATE_LEN+1, "%d-%d-%d %d:%d:%d", result.tm_year+1900, result.tm_mon+1, result.tm_mday, result.tm_hour, result.tm_min, result.tm_sec);
	if(i == 0 || i >= DATE_LEN+1)
	{
		LOGLIB_ERR("REST", "date format change failed\n");
		return RC_NOK;
	}

/*
	UINT count = 0;
	UINT stack = 0;
	UINT year_len = 4;
	UINT other_len = 2;
	CHAR date_new[DATE_LEN];
	date_new[0] = '\0';

	count = comlib_strSpn(date_old, DATE_SCHEME_CHARS);
	if(count != year_len || date_old[count] != '-')
	{
		LOGLIB_ERR("REST", "year type is not correct\n");
		return RC_NOK;
	}

	i = snprintf(date_new, DATE_LEN, "%c%c%c%c-", date_old[0], date_old[1], date_old[2], date_old[3]);
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append year error\n");
		return RC_NOK;
	}
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' )
	{
		LOGLIB_ERR("REST", "month type is not correct\n");
		return RC_NOK;
	}
		
	i += snprintf(date_new+i, DATE_LEN-i , "%c%c-", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append month error\n");
		return RC_NOK;
	}
	stack += count+1;
	

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' )
	{
		LOGLIB_ERR("REST", "day type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i , "%c%c", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append day error\n");
		return RC_NOK;
	}
	stack += count+1;

	if(date_old[stack-1] != '_')
	{
		LOGLIB_ERR("REST",  "date type('_') change is failed\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i , " ");
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append '_' error\n");
		return RC_NOK;
	}	

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len)
	{
		LOGLIB_ERR("REST", "hour type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c:", date_old[stack], date_old[stack+1]);	
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append hour error\n");
		return RC_NOK;
	}
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' )
	{
		LOGLIB_ERR("REST", "minute type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c:", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append minute error\n");
		return RC_NOK;
	}
	stack += count+1;

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' )
	{
		LOGLIB_ERR("REST", "second type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c", date_old[stack], date_old[stack+1]);
	if(i<0 || i>DATE_LEN)
	{
		LOGLIB_ERR("REST", "snprintf() append second error\n");
		return RC_NOK;
	}	
	
	snprintf((CHAR *)date_store, DATE_LEN+1, "%s", date_new);
	*/
	return RC_OK;
}
