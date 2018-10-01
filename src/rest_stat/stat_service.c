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