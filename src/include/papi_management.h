/*
 * papi_management.h
 *
 *  Created on: Aug 29, 2016
 *      Author: agarcia
 */

#ifndef PAPI_MANAGEMENT_H_
#define PAPI_MANAGEMENT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "papi.h"
#include "utilities.h"

extern unsigned long long int time_proc;
extern double watts;

void initPapi();
void startMeasures();
void stopMeasures();
void printMeasures(char* kernel_name);
void freePapi();

#endif /* PAPI_MANAGEMENT_H_ */
