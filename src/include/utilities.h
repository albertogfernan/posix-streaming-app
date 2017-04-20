/*
 * utilities.h
 *
 *  Created on: Aug 6, 2016
 *      Author: garci
 */

#ifndef INCLUDE_UTILITIES_H_
#define INCLUDE_UTILITIES_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "debug_header.h"

#define OPT_MAX_LENGTH 100

typedef struct{
	char key[OPT_MAX_LENGTH];
	char value[OPT_MAX_LENGTH];
}program_opt_t;


unsigned int count_lines(FILE * filedesc);

int count_chars(char *s, char c);

char *trim_whitespace(char *str);

int parse_options(char* options, char** program, program_opt_t** options_found);

unsigned long long int stamp(struct timeval *t);

unsigned long long int now();

unsigned long long int then(struct timeval *t);

#endif /* INCLUDE_UTILITIES_H_ */
