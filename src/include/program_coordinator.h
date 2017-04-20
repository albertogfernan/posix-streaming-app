/*
 * program_coordinator.h
 *
 *  Created on: Aug 29, 2016
 *      Author: agarcia
 */

#ifndef PROGRAM_COORDINATOR_H_
#define PROGRAM_COORDINATOR_H_

#include <sys/types.h>
#include <signal.h>
#include "ipc_communications.h"
#include "program_kernel.h"

void coordinate(int num_procs);

#endif /* PROGRAM_COORDINATOR_H_ */
