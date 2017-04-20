/*
 * execute_child.h
 *
 *  Created on: Aug 6, 2016
 *      Author: garci
 */

#ifndef EXECUTE_CHILD_H_
#define EXECUTE_CHILD_H_

#include <stdio.h>
#include <string.h>

#include "debug_header.h"
#include "program_filesrc.h"
#include "program_videosink.h"
#include "program_kernel.h"

void execute_child(int id, char* conf_line);

#endif /* SRC_INCLUDE_EXECUTE_CHILD_H_ */
