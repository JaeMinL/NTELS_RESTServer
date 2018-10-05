#include <unistd.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <json/json.h>
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

#define DATE_LEN 19
#define DATE_SCHEME_CHARS "1234567890"

FT_PUBLIC RT_RESULT ChDate(CONST CHAR *date_old, CHAR *date_store){
	CHAR date_new[DATE_LEN];
	UINT year_len = 4;
	UINT other_len = 2;
	UINT count = 0;
        UINT stack = 0;
	UINT i = 0;
	date_new[0] = '\0';

	if(date_old == NULL){
		return RC_NOK;
	}	
	
	if(comlib_strGetLen(date_old) != DATE_LEN){
		fprintf(stderr, "date type is not correct\n");
		return RC_NOK;
	}

	count = comlib_strSpn(date_old, DATE_SCHEME_CHARS);
	if(count != year_len || date_old[count] != '-'){
		fprintf(stderr, "year type is not correct\n");
		return RC_NOK;
	}

	i = snprintf(date_new, DATE_LEN, "%c%c%c%c-", date_old[0], date_old[1], date_old[2], date_old[3]);
	if(i<0 || i>=DATE_LEN)
	{
		fprintf(stderr, "snprintf() append year error\n");
                return RC_NOK;
	}
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' ){
		fprintf(stderr, "month type is not correct\n");
		return RC_NOK;
	}
		
	i += snprintf(date_new+i, DATE_LEN-i , "%c%c-", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN){
                fprintf(stderr, "snprintf() append month error\n");
                return RC_NOK;
        }
	stack += count+1;
	

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' ){
		fprintf(stderr, "day type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i , "%c%c", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN){
                fprintf(stderr, "snprintf() append day error\n");
                return RC_NOK;
        }
	stack += count+1;

	if(date_old[stack-1] != '_'){
		fprintf(stderr, "date type('_') change is failed\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i , " ");
	if(i<0 || i>=DATE_LEN){
                fprintf(stderr, "snprintf() append '_' error\n");
                return RC_NOK;
        }	

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len){
		fprintf(stderr, "hour type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c:", date_old[stack], date_old[stack+1]);	
	if(i<0 || i>=DATE_LEN){
                fprintf(stderr, "snprintf() append hour error\n");
                return RC_NOK;
        }
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' ){
		fprintf(stderr, "minute type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c:", date_old[stack], date_old[stack+1]);
	if(i<0 || i>=DATE_LEN){
                fprintf(stderr, "snprintf() append minute error\n");
                return RC_NOK;
        }
	stack += count+1;

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' ){
		fprintf(stderr, "second type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, DATE_LEN-i, "%c%c", date_old[stack], date_old[stack+1]);
	if(i<0 || i>DATE_LEN){
                fprintf(stderr, "snprintf() append second error\n");
                return RC_NOK;
        }	

	snprintf((CHAR *)date_store, DATE_LEN+1, "%s", date_new);
	return RC_OK;
}
