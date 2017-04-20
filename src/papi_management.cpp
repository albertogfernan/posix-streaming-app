/*
 * papi_management.cpp
 *
 *  Created on: Aug 29, 2016
 *      Author: agarcia
 */
#include "papi_management.h"

#define MAX_RAPL_EVENTS 64

int EventSet = PAPI_NULL;
int num_events = 0;
long long *values;
long long *acc_values;
unsigned long long int nowtime = 0;
unsigned long long int time_proc = 0;
unsigned long long int acctime_proc = 0;
double joules = 0;
double joules_tot = 0;
double watts = 0;

char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN];
char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN];


void initPapi()
{
	int retval, cid,rapl_cid=-1,numcmp;
	int code;
	int r;
	const PAPI_component_info_t *cmpinfo = NULL;
	PAPI_event_info_t evinfo;

	/* PAPI Initialization */
	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT) {
		printf("PAPI_library_init failed with return value %d\n", retval);
	}

	numcmp = PAPI_num_components();

	for (cid = 0; cid < numcmp; cid++) {

		if ((cmpinfo = PAPI_get_component_info(cid)) == NULL) {
			printf("PAPI_get_component_info failed\n");
		}

		if (strstr(cmpinfo->name, "rapl")) {
			rapl_cid = cid;
			if (cmpinfo->disabled) {
				printf("RAPL component disabled: %s\n", cmpinfo->disabled_reason);
				return;
			}
			break;
		}
	}

	/* Component not found */
	if (cid == numcmp) {
		printf("No rapl component found\n");
		return;
	}

	/* Create EventSet */
	retval = PAPI_create_eventset(&EventSet);
	if (retval != PAPI_OK) {
		printf("PAPI_create_eventset() failed with value %d", retval);
		return;
	}

	/* Add all events */

    code = PAPI_NATIVE_MASK;

    r = PAPI_enum_cmp_event( &code, PAPI_ENUM_FIRST, rapl_cid );

	while (r == PAPI_OK) {
		retval = PAPI_event_code_to_name(code, event_names[num_events]);
		if (retval != PAPI_OK) {
			printf("Error translating %#x\n", code);
			return;
		}

		retval = PAPI_get_event_info(code, &evinfo);
		if (retval != PAPI_OK) {
			printf("Error getting event info with value %d\n", retval);
			return;
		}

		strncpy(units[num_events], evinfo.units, sizeof(units[0]) - 1);
		// buffer must be null terminated to safely use strstr operation on it below
		units[num_events][sizeof(units[0]) - 1] = '\0';

		retval = PAPI_add_event(EventSet, code);
		if (retval != PAPI_OK) {
			break; /* We've hit an event limit */
		}
		num_events++;

		r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
	}

	values = (long long*)calloc(num_events, sizeof(long long));
	if (values == NULL) {
		printf("No memory for the tests\n");
	}

	acc_values = (long long*) calloc(num_events, sizeof(long long));
	if (acc_values == NULL) {
		printf("No memory for the tests\n");
	}
	else
	{
		memset(acc_values, 0, num_events * sizeof(long long));
	}
}

void startMeasures()
{
	nowtime = now();

	if (num_events != 0)
		PAPI_start(EventSet);

	watts = 0;
	joules = 0;
}



void stopMeasures()
{
	if(num_events != 0)
		PAPI_stop( EventSet, values);

	time_proc = (now() - nowtime);
	acctime_proc += time_proc;

	for (int i = 0; i < num_events; i++)
	{
		acc_values[i] += values[i];
		if(strstr(event_names[i], "PACKAGE_ENERGY") != NULL)
		{
			joules += (double)values[i]/1.0e9; //Joules
		}
	}

	joules_tot += joules;
	watts = joules / time_proc;
}

void printMeasures(char* kernel_name)
{
	double secs = ((double) (acctime_proc)) / 1e6;
	printf("kernel %s: accumulated cpu time %lf secs, energy %lf W\n", kernel_name, secs, secs != 0 ? joules_tot / secs : 0);
}



void freePapi()
{
	int retval;

	 /* Done, clean up */
	retval = PAPI_cleanup_eventset(EventSet);
	if (retval != PAPI_OK) {
		printf("PAPI_cleanup_eventset() failed\n");
		return;
	}

	retval = PAPI_destroy_eventset(&EventSet);
}

