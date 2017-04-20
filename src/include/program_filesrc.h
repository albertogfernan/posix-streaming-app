/*
 * program_filesrc.h
 *
 *  Created on: Aug 7, 2016
 *      Author: garci
 */

#ifndef PROGRAM_FILESRC_H_
#define PROGRAM_FILESRC_H_

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "utilities.h"
#include "ipc_communications.h"
#include "debug_header.h"
#include "global_params.h"

void execute_filesrc(int id, int num_ops, program_opt_t* options);

#endif /* SRC_INCLUDE_PROGRAM_FILESRC_H_ */
