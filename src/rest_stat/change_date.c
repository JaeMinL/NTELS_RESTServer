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

FT_PUBLIC RT_RESULT ChDate(CONST CHAR *date_old, CHAR **date_new){
	UINT year_len = 4;
	UINT other_len = 2;
	UINT i = 0;

	if(date_old == NULL){
		return RC_NOK;
	}	
	
//	comlib_strCpy(date_old, &date_new);
	if(comlib_strGetLen(date_old) != DATE_LEN){
		fprintf(stderr, "date type is not correct\n");
		return RC_NOK;
	}

	UINT count = 0;
	UINT stack = 0;

	count = comlib_strSpn(date_old, DATE_SCHEME_CHARS);
	if(count != year_len || date_old[count] != '-'){
		fprintf(stderr, "year type is not correct\n");
		return RC_NOK;
	}
	i = snprintf(date_new, count+2 , "%c%c%c%c-", date_old[0], date_old[1], date_old[2], date_old[3]);
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' ){
		fprintf(stderr, "month type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(&date_new+i, count+2 , "%c%c-", date_old[stack], date_old[stack+1]);
	stack += count+1;
	printf("??%s??", date_new);	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != '-' ){
		fprintf(stderr, "day type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(date_new+i, count+2 , "%c%c", date_old[stack], date_old[stack+1]);
	stack += count+1;

	if(date_old[stack-1] != '_'){
		fprintf(stderr, "date type('_') change is failed\n");
		return RC_NOK;
	}
	i += snprintf(&date_new+i, 2 , " ");
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len){
		fprintf(stderr, "hour type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(&date_new+i, count+2 , "%c%c:", date_old[stack], date_old[stack+1]);	
	stack += count+1;
	
	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' ){
		fprintf(stderr, "minute type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(&date_new+i, count+2 , "%c%c:", date_old[stack], date_old[stack+1]);
	stack += count+1;

	count = comlib_strSpn(&date_old[stack], DATE_SCHEME_CHARS);
	if(count != other_len || date_old[stack-1] != ':' ){
		fprintf(stderr, "second type is not correct\n");
		return RC_NOK;
	}
	i += snprintf(&date_new+i, count+1 , "%c%c", date_old[stack], date_old[stack+1]);

	return RC_OK;
}
