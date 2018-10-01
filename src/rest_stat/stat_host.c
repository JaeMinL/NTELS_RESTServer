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

FT_PUBLIC RT_RESULT Host_1min_time(UINT mthod, RsvlibSesCb *sesCb){
	SINT ret = RC_NOK;

    ret = MakeQueryByTime(sesCb, "HOST", NULL);
    if(ret != RC_OK){
            fprintf(stderr, "HOST_1min_time() error");
            return RC_NOK;
    }

    return RC_OK;
}




